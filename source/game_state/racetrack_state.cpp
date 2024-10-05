///
/// @file
/// @details Manage the racetrack of the game!
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../game_state/racetrack_state.hpp"
#include "../game_state/object_state.hpp"
#include "../game_state/timing_and_scoring_state.hpp"
#include "../game_state/events/racetrack_events.hpp"
#include "../game_state/implementation/racetrack_implementation.hpp"
#include "../core/utilities.hpp"
#include "../core/event_system.hpp"
#include "../ludumdare56.hpp"
#include "../custom_components.hpp"
#include "../logging.hpp"

#include <turtle_brains/core/tb_types.hpp>

#include <ice/physics/ice_physical_world.hpp>

#include <track_bundler/track_bundler.hpp>
#include <track_bundler/track_bundler_to_ice_physics.hpp>
#include <track_bundler/track_bundler_to_ice_graphics.hpp> //required for mesh atm...

#include <array>
#include <fstream>

namespace LudumDare56
{
	namespace GameState
	{
		namespace Implementation
		{

			// TODO: LudumDare56: 2024-10-03: We should be focused on the TrackBundler of the future in this project
			//   since it is a template for future projects. Therefore we need to get rid of the TrackBundler::Legacy
			//   stuff. Ideally it isn't actually being much use these days anyway, so should be 'easy'.

			//class RacetrackLoader : public TrackBundler::Legacy::BundleProcessorInterface
			//{
			//private:
			//	virtual void OnCreateTrackNode(const TrackBundler::Node&, const TrackBundler::ImprovedTrackBundle&);
			//	virtual void OnCreateComponent(const TrackBundler::Node&, const TrackBundler::Component&, const TrackBundler::ImprovedTrackBundle&);
			//};

			class RacetrackLoader : public TrackBundler::Legacy::BundleProcessorInterface
			{
			private:
				virtual void OnCreateTrackNode(const TrackBundler::Node&, const TrackBundler::Legacy::TrackBundle&) override;
				virtual void OnCreateComponent(const TrackBundler::Node&, const TrackBundler::Component&, const TrackBundler::Legacy::TrackBundle&) override;

				virtual void OnCreateTrackSegment(const TrackBundler::Legacy::TrackSegment& trackSegment, const TrackBundler::Legacy::TrackBundle& trackBundle) override;
				virtual void OnCreateTrackObject(const TrackBundler::Legacy::TrackObject& trackObject, const TrackBundler::Legacy::TrackBundle& trackBundle) override;
				virtual void OnCreateTrackSpline(const TrackBundler::Legacy::TrackSpline& trackSpline, const TrackBundler::Legacy::TrackBundle& trackBundle) override;

				/// @details Checks through the different objects for logic only objects and returns true if one was created.
				bool TryCreateLogicObject(const TrackBundler::Legacy::TrackObject& trackObject, const TrackBundler::Legacy::TrackBundle& trackBundle);
			};

		};	//namespace Implementation
	};	//namespace GameState
};	//namespace LudumDare56

namespace
{
	TrackBundler::Legacy::TrackSegmentDefinitionContainer theTrackSegmentDefinitions;
	TrackBundler::Legacy::TrackObjectDefinitionContainer theTrackObjectDefinitions;
	TrackBundler::Legacy::TrackSplineDefinitionContainer theTrackSplineDefinitions;

	tbCore::tbString theCurrentRacetrack = "";
	TrackBundler::Legacy::TrackBundle theRacetrackBundle;

	std::array<icePhysics::Matrix4, 256> theGridSpotsToWorld; //16kb!

	LudumDare56::GameState::Implementation::RacetrackLoader theRacetrackLoader;
	typedef std::unique_ptr<LudumDare56::GameState::ObjectState> ObjectStatePtr;

	tbCore::Node theRootObject;
	std::vector<ObjectStatePtr> theRacetrackObjects;

	TyreBytes::Core::EventBroadcaster theRacetrackBroadcaster;

	std::vector<LudumDare56::GameState::RacetrackState::TrackNodeEdge> theTrackNodeEdges;

	tbMath::BezierCurve theRacetrackCurve;
	iceCore::MeshHandle theRacetrackMesh;
};

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacetrackState::AddEventListener(TyreBytes::Core::EventListener& eventListener)
{
	theRacetrackBroadcaster.AddEventListener(eventListener);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacetrackState::RemoveEventListener(TyreBytes::Core::EventListener& eventListener)
{
	theRacetrackBroadcaster.RemoveEventListener(eventListener);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacetrackState::InvalidateRacetrack(void)
{
	theTrackSegmentDefinitions = TrackBundler::Legacy::LoadTrackSegmentDefinitionsFromFile("data/track_segments_list.json");
	theTrackObjectDefinitions = TrackBundler::Legacy::LoadTrackObjectDefinitionsFromFile("data/track_objects_list.json");
	theTrackSplineDefinitions = TrackBundler::Legacy::LoadTrackSplineDefinitionsFromFile("data/track_splines_list.json");

	theCurrentRacetrack = "";

	theRacetrackBundle = TrackBundler::Legacy::TrackBundle();

	theRootObject.SetName("root");
	theRootObject.ClearChildren();

	for (ObjectStatePtr& objectState : theRacetrackObjects)
	{	// This is NOT ideal, but theRootObject is already managing the children/hierarchy. So we really should be holding
		//   onto them as raw pointers, or not maybe even holding onto them? I had tried making an AddChild() to Node
		//   that does not manage the added child but that started failing at startup for some reason like theRootObject
		//   was expected to be an ObjectStatePtr and it wasn't (or it was returning a child that was nullptr, etc). This
		//   code is in InvalidateRacetrack() and DestroyRacetrack();
		objectState.release();
	}
	theRacetrackObjects.clear();

	if (iceCore::InvalidMesh() != theRacetrackMesh)
	{
		iceCore::theMeshManager.DestroyMesh(theRacetrackMesh);
		theRacetrackMesh = iceCore::InvalidMesh();
	}

	Implementation::TheMutableTrackNodes().clear();
	theTrackNodeEdges.clear();

	theRacetrackBroadcaster.SendEvent(TyreBytes::Core::Event(Events::Racetrack::ClearObjects));
	theRacetrackBroadcaster.SendEvent(Events::CreateRacetrackEvent(theRacetrackBundle, theTrackSegmentDefinitions,
		theTrackObjectDefinitions, theTrackSplineDefinitions));

	TimingState::Invalidate();
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameState::RacetrackState::IsValidRacetrack(void)
{
	return (false == theCurrentRacetrack.empty());
}

//--------------------------------------------------------------------------------------------------------------------//

const tbCore::tbString& LudumDare56::GameState::RacetrackState::GetCurrentRacetrack(void)
{
	return theCurrentRacetrack;
}

//--------------------------------------------------------------------------------------------------------------------//

const iceCore::MeshHandle& LudumDare56::GameState::RacetrackState::GetCurrentRacetrackMesh(void)
{
	return theRacetrackMesh;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacetrackState::Create(icePhysics::World& physicalWorld)
{
	physicalWorld.HackyAPI_SetGlobalMeshCollider(icePhysics::MeshCollider(theRacetrackMesh));

	for (ObjectStatePtr& objectState : theRacetrackObjects)
	{
		//objectState->OnCreate(physicalWorld);
		objectState->OnAwake();
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacetrackState::Destroy(icePhysics::World& physicalWorld)
{
	physicalWorld.HackyAPI_SetGlobalMeshCollider(icePhysics::MeshCollider(iceCore::InvalidMesh()));

	for (ObjectStatePtr& objectState : theRacetrackObjects)
	{
		for (ComponentState& component : objectState->AllComponents())
		{
			component.OnDestroy();
		}

		//objectState->OnDestroy(physicalWorld);
		objectState->OnDestroy();
	}

	// 2024-09-03: Can't delete theRootObject of the track/scene before clearing the children because they have a reference
	//   to the Node data loaded from TrackBundler. It probably shouldn't but it does, so, cleanup in the right order.

	for (ObjectStatePtr& objectState : theRacetrackObjects)
	{	// 2024-10-04: Looks like past-Tim had some idea that this might happen without completing it all?
		// This is NOT ideal, but theRootObject is already managing the children/hierarchy. So we really should be holding
		//   onto them as raw pointers, or not maybe even holding onto them? I had tried making an AddChild() to Node
		//   that does not manage the added child but that started failing at startup for some reason like theRootObject
		//   was expected to be an ObjectStatePtr and it wasn't (or it was returning a child that was nullptr, etc). This
		//   code is in InvalidateRacetrack() and DestroyRacetrack();
		objectState.release();
	}
	theRacetrackObjects.clear();
	theRootObject.ClearChildren();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacetrackState::Simulate(void)
{
	for (ObjectStatePtr& objectState : theRacetrackObjects)
	{
		if (true == objectState->IsActive())
		{
			objectState->OnSimulate();

			for (ComponentState& component : objectState->AllComponents())
			{
				if (true == component.IsActiveSelf())
				{
					component.OnSimulate();
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacetrackState::RenderDebug(void)
{
#if !defined(ludumdare56_headless_build)
	if (true == theTrackNodeEdges.empty())
	{
		return;
	}

	iceGraphics::Visualization debugVisuals;

	debugVisuals.ClearPermanentVisuals();
	debugVisuals.PermanentLine(tbMath::Vector3::Zero(), tbMath::Vector3(2.0f, 0.0f, 0.0f), 0xFFFF0000);
	debugVisuals.PermanentLine(tbMath::Vector3::Zero(), tbMath::Vector3(0.0f, 0.0f, -2.0f), 0xFF0000FF);

	//tbMath::Vector3 previousCenter = GetTrackNodeTrailingEdge(GetNumberOfTrackNodes() - static_cast<TrackNodeIndex>(1), TrackEdge::kCenter);
	tbMath::Vector3 previousCenter = GetTrackNodeTrailingEdge(0, TrackEdge::kCenter);

	for (TrackNodeIndex trackNodeIndex = 0; trackNodeIndex < GetNumberOfTrackNodes(); ++trackNodeIndex)
	{
		const TrackNodeEdge& nodeLeadingEdge = GetTrackNodeLeadingEdge(trackNodeIndex);
		debugVisuals.PermanentLine(nodeLeadingEdge[TrackEdge::kLeft], nodeLeadingEdge[TrackEdge::kRight], 0xFFFF00FF);

		const tbMath::Vector3 currentCenter = nodeLeadingEdge[TrackEdge::kCenter];
		debugVisuals.PermanentLine(previousCenter, currentCenter, 0xFFFFFFFF);
		previousCenter = currentCenter;
	}

	{	//The first and last track node edges are overlapping with a closed-circuit racetrack, this may flicker a little.
		const TrackNodeEdge& lastNodeLeadingEdge = GetTrackNodeLeadingEdge(GetNumberOfTrackNodes() - static_cast<TrackNodeIndex>(1));
		debugVisuals.PermanentLine(lastNodeLeadingEdge[TrackEdge::kLeft], lastNodeLeadingEdge[TrackEdge::kRight], 0xFFFF0000);
		const TrackNodeEdge& firstNodeTrailingEdge = GetTrackNodeTrailingEdge(0);
		debugVisuals.PermanentLine(firstNodeTrailingEdge[TrackEdge::kLeft], firstNodeTrailingEdge[TrackEdge::kRight], 0xFF00FF00);
	}

	iceGraphics::PushMatrix();
	iceGraphics::SetObjectToWorld(Matrix4::Translation(Up() * 1.0f));
	debugVisuals.Render();
	iceGraphics::PopMatrix();
#endif /* ludumdare56_headless_build */
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacetrackState::GetGridToWorld(const GridIndex gridIndex)
{
	return icePhysics::Matrix4::Translation(0.0f, 2.0f, 0.0f) * theGridSpotsToWorld[gridIndex];
}

//--------------------------------------------------------------------------------------------------------------------//

TrackBundler::Legacy::TrackBundle& LudumDare56::GameState::RacetrackState::GetTrackBundle(void)
{
	return theRacetrackBundle;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacetrackState::LoadRacetrack(const String& racetrackFilepath)
{
	if (theCurrentRacetrack == racetrackFilepath)
	{
		return;
	}

	tb_always_log(LogState::Info() << "Loading racetrack \"" << racetrackFilepath << "\"");

	InvalidateRacetrack();

	if (false == TrackBundler::Legacy::LoadTrackBundle(racetrackFilepath, theRacetrackBundle, &theRacetrackLoader))
	{
		InvalidateRacetrack();
		tb_error("Failed to load track from file: %s", racetrackFilepath.c_str());
		return;
	}

	theCurrentRacetrack = racetrackFilepath;

	theRacetrackBroadcaster.SendEvent(Events::CreateRacetrackEvent(theRacetrackBundle, theTrackSegmentDefinitions,
		theTrackObjectDefinitions, theTrackSplineDefinitions));
}

//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::GameState::ObjectState& LudumDare56::GameState::RacetrackState::GetObjectState(const ObjectHandle objectHandle)
{
	tb_error_if(objectHandle >= theRacetrackObjects.size(), "Error: objectHandle is out of range getting transform.");
	tb_error_if(nullptr == theRacetrackObjects[objectHandle], "Error: invalid objectHandle, object is null getting transform.");
	return *theRacetrackObjects[objectHandle];
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ObjectState& LudumDare56::GameState::RacetrackState::GetMutableObjectState(const ObjectHandle objectHandle)
{
	tb_error_if(objectHandle >= theRacetrackObjects.size(), "Error: objectHandle is out of range getting transform.");
	tb_error_if(nullptr == theRacetrackObjects[objectHandle], "Error: invalid objectHandle, object is null getting transform.");
	return *theRacetrackObjects[objectHandle];
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

/// @note 2023-10-19: Some of the following may want to move into a Timing and Scoring area that the Racectrack
///   can manage/update etc. But for sake of focusing on Artificial Drivers First Lap, I'm just winging it.

LudumDare56::GameState::RacetrackState::TrackNodeIndex LudumDare56::GameState::RacetrackState::GetNumberOfTrackNodes(void)
{
	return tbCore::RangedCast<TrackNodeIndex::Integer>(theTrackNodeEdges.size() - 1);
}

//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::Vector3& LudumDare56::GameState::RacetrackState::GetTrackNodeLeadingEdge(
	const TrackNodeIndex trackNodeIndex, const TrackEdge trackEdge)
{
	tb_error_if(true == theTrackNodeEdges.empty(), "Error: The track does not contain any nodes or node edges.");
	tb_error_if(trackNodeIndex + static_cast<TrackNodeIndex>(1) >= theTrackNodeEdges.size(), "Error: trackNodeIndex is out of range.");
	return theTrackNodeEdges[trackNodeIndex + static_cast<TrackNodeIndex>(1)][trackEdge];
}

//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::GameState::RacetrackState::TrackNodeEdge& LudumDare56::GameState::RacetrackState::GetTrackNodeLeadingEdge(
	const TrackNodeIndex trackNodeIndex)
{
	tb_error_if(true == theTrackNodeEdges.empty(), "Error: The track does not contain any nodes or node edges.");
	tb_error_if(trackNodeIndex + static_cast<TrackNodeIndex>(1) >= theTrackNodeEdges.size(), "Error: trackNodeIndex is out of range.");
	return theTrackNodeEdges[trackNodeIndex + static_cast<TrackNodeIndex>(1)];
}

//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::Vector3& LudumDare56::GameState::RacetrackState::GetTrackNodeTrailingEdge(
	const TrackNodeIndex trackNodeIndex, const TrackEdge trackEdge)
{
	tb_error_if(true == theTrackNodeEdges.empty(), "Error: The track does not contain any nodes or node edges.");
	tb_error_if(trackNodeIndex >= theTrackNodeEdges.size(), "Error: trackNodeIndex is out of range.");
	return theTrackNodeEdges[trackNodeIndex][trackEdge];
}

//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::GameState::RacetrackState::TrackNodeEdge& LudumDare56::GameState::RacetrackState::GetTrackNodeTrailingEdge(
	const TrackNodeIndex trackNodeIndex)
{
	tb_error_if(true == theTrackNodeEdges.empty(), "Error: The track does not contain any nodes or node edges.");
	tb_error_if(trackNodeIndex >= theTrackNodeEdges.size(), "Error: trackNodeIndex is out of range.");
	return theTrackNodeEdges[trackNodeIndex];
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameState::RacetrackState::IsOnTrack(const iceVector3& positionInWorld)
{
	//float distance = 0.0;
	//theRacetrackCurve.GetClosestPoint(positionInWorld, distance);
	//return (distance > 9.5f);


//	theRacetrackMesh

	tb_unused(positionInWorld);
	return true;
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::Implementation::RacetrackLoader::OnCreateTrackNode(
	const TrackBundler::Node& node, const TrackBundler::Legacy::TrackBundle& trackBundle)
{
	tb_always_log(LogState::Info() << "Creating node: " << node.GetName() << ".");

	String parentName = "";
	size_t nodeIndex = 0;
	for (const TrackBundler::Node& existingNode : trackBundle.mImprovedBundle.mNodeHierarchy)
	{
		if (existingNode.mNodeKey == node.mNodeKey)
		{
			break;
		}

		++nodeIndex;
	}

	for (const TrackBundler::Node& parentNode : trackBundle.mImprovedBundle.mNodeHierarchy)
	{
		if (parentNode.mNodeKey == node.mParentNodeKey)
		{
			parentName = parentNode.GetName();
			break;
		}
	}

	ObjectStatePtr object;
	object.reset(new ObjectState(node));

	RacetrackState::ObjectHandle objectHandle = tbCore::RangedCast<RacetrackState::ObjectHandle::Integer>(theRacetrackObjects.size());
	theRacetrackObjects.emplace_back(object.get());
	theRacetrackBroadcaster.SendEvent(Events::RacetrackObjectEvent(GameState::Events::Racetrack::AddObject, objectHandle));

	const TrackBundler::NodeKey rootNodeKey = trackBundle.mImprovedBundle.mNodeHierarchy[0].mNodeKey;
	if (rootNodeKey == node.mParentNodeKey || TrackBundler::NodeKey::Invalid() == node.mParentNodeKey)
	{
		theRootObject.AddChild(std::move(object));
		//theRootObject.AddChild(*object);
	}
	else
	{
		const tbCore::uuid parentNodeID = static_cast<tbCore::uuid>(node.mParentNodeKey);
		tbCore::Node* parentNode = theRootObject.FindChildByID(parentNodeID, tbCore::Recursive::Yes);
		if (nullptr == parentNode)
		{
			tb_always_log(LogState::Error() << "Expected to find parentNode(" << parentName << ") in the root object already. childNode: " << node.GetName());
			tb_error("Expected to find parent node in the root object already.");
		}
		else
		{
			const tbCore::Node::ChildIndex childIndex = parentNode->AddChild(std::move(object));
			//const tbCore::Node::ChildIndex childIndex = parentNode->AddChild(*object);
			tb_always_log_if(childIndex != node.mChildIndex, LogState::Error() << "Expected the childIndex to match what was added to the parent.");
			tb_error_if(childIndex != node.mChildIndex, "Expected the childIndex to match what was added to the parent.");
		}
	}

	tb_always_log(LogState::Info() << "Created a node: " << node.GetName() << ".");
}

//--------------------------------------------------------------------------------------------------------------------//

//Dumb helper that needs to be in TrackBundler, or something.

const TrackBundler::Component* GetComponentOn(const TrackBundler::NodeKey& nodeKey, const TrackBundler::ImprovedTrackBundle& trackBundle,
	const TrackBundler::ComponentDefinitionKey& definitionKey)
{
	size_t nodeIndex = 0;
	for (const TrackBundler::Node& node : trackBundle.mNodeHierarchy)
	{
		if (nodeKey == node.mNodeKey)
		{
			for (const TrackBundler::Component& component : trackBundle.mNodeComponents[nodeIndex])
			{
				if (definitionKey == component.mDefinitionKey)
				{
					return &component;
				}
			}

			return nullptr;
		}
		++nodeIndex;
	}

	return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::Implementation::RacetrackLoader::OnCreateComponent(const TrackBundler::Node& node,
	const TrackBundler::Component& component, const TrackBundler::Legacy::TrackBundle& trackBundle)
{
	tbCore::Node* actualNode = theRootObject.FindChildByID(static_cast<tbCore::uuid>(node.mNodeKey), tbCore::Recursive::Yes);
	tb_error_if(nullptr == actualNode, "Expected the node(%s) to exist in the root object.", node.GetName().c_str());
	ObjectState* object = dynamic_cast<ObjectState*>(actualNode);
	tb_error_if(nullptr == object, "Expected the node(%s) to be an ObjectState type!", node.GetName().c_str());

	ComponentStatePtr componentState = ComponentState::CreateComponent(*object, component, trackBundle.mImprovedBundle);
	if (nullptr != componentState)
	{
		object->AddComponent(std::move(componentState));
	}

	if (ComponentDefinition::kSpawnPointKey == component.mDefinitionKey)
	{
		// TODO: LudumDare56: PainPoint: 2024-10-05: I was trying to use AsRangedIntegerWithDefault() but it didn't want
		//   to compile because 'const' if statement... AsRangedInteger works and I think we need some cleanup in the
		//   DynamicStructure regarding this mess.
		int gridIndex = component.mProperties["index"].AsRangedInteger<int>();
		tb_always_log(LogState::Always() << "Setting GridSpot[" << gridIndex << "] to: ( " << node.GetNodeToWorld().GetPosition().x << ", " <<
			node.GetNodeToWorld().GetPosition().z << " ).");

		theGridSpotsToWorld[gridIndex] = static_cast<icePhysics::Matrix4>(node.GetNodeToWorld());
	}
	else if (ComponentDefinition::kZoneForbiddenKey == component.mDefinitionKey)
	{
	}
	else if (component.mDefinitionKey == TrackBundler::ComponentDefinition::kSplinePathKey)
	{
		if ("racetrack_collider" == node.GetName())
		{
			iceGraphics::Visualization unusedDebug;

			const TrackBundler::Component* splineMeshComponent = GetComponentOn(node.mNodeKey, trackBundle.mImprovedBundle,
				TrackBundler::ComponentDefinition::kSplineMeshKey);

			tb_error_if(nullptr == splineMeshComponent, "Error: Expected 'racetrack_collider' node to have a Spline Mesh component.");
			theRacetrackMesh = TrackBundler::CreateMeshFromSplineComponent(component, *splineMeshComponent, unusedDebug);
		}
		if ("racetrack" == node.GetName())
		{
			tb_error_if(false == TheTrackNodes().empty(), "Error: Expected TheTrackNodes container to be empty, is there more than one 'racetrack'?");
			tb_error_if(false == theTrackNodeEdges.empty(), "Error: Expected TrackNodeEdges to be empty, is there more than one 'racetrack'?");

			const TrackBundler::Component* trackInfo = GetComponentOn(node.mNodeKey, trackBundle.mImprovedBundle, ComponentDefinition::kTrackInformationKey);
			tb_error_if(nullptr == trackInfo, "Error: Expected 'racetrack' node to have a Track Information component.");
			const tbCore::DynamicStructure& trackProperties = (nullptr == trackInfo) ? tbCore::DynamicStructure::kNullValue : trackInfo->mProperties;

			iceGraphics::Visualization unusedDebug;
			const TrackBundler::Component* splineMeshComponent = GetComponentOn(node.mNodeKey, trackBundle.mImprovedBundle,
				TrackBundler::ComponentDefinition::kSplineMeshKey);
			tb_error_if(nullptr == splineMeshComponent, "Error: Expected 'racetrack' node to have a Spline Mesh component.");

			if (iceCore::InvalidMesh() == theRacetrackMesh)
			{
				theRacetrackMesh = TrackBundler::CreateMeshFromSplineComponent(component, *splineMeshComponent, unusedDebug);
			}

			std::vector<tbMath::BezierCurve> curves;
			TrackBundler::CreateCurveFromSplineComponent(curves, component, node.GetNodeToWorld());
			tb_error_if(1 != curves.size(), "Error: Expected 'racetrack' to have a SINGLE spline path.");

			const tbMath::BezierCurve& trackCurve = curves[0];
			theRacetrackCurve = trackCurve;

			{	//Build all the TrackNodes / TrackNodeEdges from the TrackCurve.
				// @note 2023-11-04: Technically TrackBundler should be creating the trackCurve for us. We are assuming CatMullRomBeau
				//   where future possibilities may open up...

				std::vector<tbMath::Vector3> centerPoints;
				std::vector<tbMath::Vector3> centerTangents;
				std::vector<float> teeValues;
				trackCurve.GetInformationByDistance(centerPoints, centerTangents, teeValues, 10.0f, 1000);
				tb_error_if(centerPoints.size() != centerTangents.size(), "Expected both center points and tangents to have the same size.");

				GameState::RacetrackState::TrackNodeEdge nodeEdge;
				tbMath::Vector3 trackRightHalfWidth;
				const float halfTrackWidth = trackProperties.GetMember("width").AsFloatWithDefault(4.75f);

				for (size_t index = 0; index < centerPoints.size(); ++index)
				{
					trackRightHalfWidth = tbMath::Vector3::Cross(centerTangents[index], WorldUp()).GetNormalized() * halfTrackWidth;

					nodeEdge[TrackEdge::kCenter] = centerPoints[index];
					nodeEdge[TrackEdge::kRight] = centerPoints[index] + trackRightHalfWidth;
					nodeEdge[TrackEdge::kLeft] = centerPoints[index] - trackRightHalfWidth;
					theTrackNodeEdges.push_back(nodeEdge);

					if (index > 0)
					{
						const RacetrackState::TrackNodeEdge& leadingEdge = theTrackNodeEdges[theTrackNodeEdges.size() - 1];
						const RacetrackState::TrackNodeEdge& trailingEdge = theTrackNodeEdges[theTrackNodeEdges.size() - 2];

						TheMutableTrackNodes().emplace_back(TrackNode{ //leading, trailing, left, right
							icePhysics::BoundingPlane(leadingEdge[TrackEdge::kCenter], icePhysics::Vector3::Cross(
								Up(), leadingEdge[TrackEdge::kRight] - leadingEdge[TrackEdge::kLeft])),
							icePhysics::BoundingPlane(trailingEdge[TrackEdge::kCenter], icePhysics::Vector3::Cross(
								trailingEdge[TrackEdge::kRight] - trailingEdge[TrackEdge::kLeft], Up())),
							icePhysics::BoundingPlane(leadingEdge[TrackEdge::kLeft], icePhysics::Vector3::Cross(
								Up(), leadingEdge[TrackEdge::kLeft] - trailingEdge[TrackEdge::kLeft])),
							icePhysics::BoundingPlane(leadingEdge[TrackEdge::kRight], icePhysics::Vector3::Cross(
								leadingEdge[TrackEdge::kRight] - trailingEdge[TrackEdge::kRight], Up())),
						});
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::Implementation::RacetrackLoader::OnCreateTrackSegment(
	const TrackBundler::Legacy::TrackSegment& /*trackSegment*/, const TrackBundler::Legacy::TrackBundle& /*trackBundle*/)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::Implementation::RacetrackLoader::OnCreateTrackObject(
	const TrackBundler::Legacy::TrackObject& trackObject, const TrackBundler::Legacy::TrackBundle& trackBundle)
{
	if (true == TryCreateLogicObject(trackObject, trackBundle))
	{
		return;
	}

	const TrackBundler::Legacy::TrackObjectDefinition& objectDefinition = theTrackObjectDefinitions[trackObject.mDefinitionIndex];
	const tbCore::tbString objectTypeName = tbCore::String::Lowercase(objectDefinition.mDisplayName);
	tb_debug_log(LogState::Warning() << "NOT creating track object: " << objectTypeName << " since it is an old objectDefition type of object.");

	//GameState::RacetrackState::ObjectHandle objectHandle = tbCore::RangedCast<GameState::RacetrackState::ObjectHandle::Integer>(theRacetrackObjects.size());
	//theRacetrackObjects.emplace_back(new ObjectState(objectDefinition));
	//theRacetrackObjects.back()->SetObjectToWorld(trackObject.mObjectToWorld);
	//GameState::ObjectState& objectState = *theRacetrackObjects.back();

	//// 2024-08-17: TODO: LudumDare56: Enable the cone and test ramp again, probably need to do so with components?
	////   IDK, this whole Old/New object stuff with prefabs is a giant mess, and there isn't much point in trying to
	////   make it all work today since the TrackBuilder prefab stuff needs significant work. Lets finish that project
	////   then come back and find a cleaner way to integrate into the projects.
	////
	////if ("cone" == objectTypeName)
	////{
	////	//Create a cone object an put it in the Racetrack?

	////	const icePhysics::Scalar radius(0.45 / 2.0);
	////	//const icePhysics::Scalar radius(0.5);
	////	objectState.AddBoundingVolume(new icePhysics::BoundingSphere(icePhysics::Vector3(0.0, 0.0, 0.0), radius));
	////	//objectState.AddBoundingVolume(new icePhysics::BoundingSphere(icePhysics::Vector3(0.0, 1.0, 0.0), radius));
	////	objectState.SetMass(10.0f);
	////	objectState.SetCenterOfMass(icePhysics::Vector3(0.0, 0.45 / 2.0, 0.0));
	////	//objectState.SetCenterOfMass(icePhysics::Vector3(0.0, 0.45, 0.0));
	////	objectState.SetObjectToWorld(icePhysics::Matrix4(trackObject.mObjectToWorld));
	////}
	////else if ("test ramp" == objectTypeName)
	////{
	////	objectState.AddBoundingVolume(new icePhysics::MeshCollider("data/meshes/test_ramp.msh"));
	////	objectState.SetMass(-1.0f);
	////	objectState.SetObjectToWorld(icePhysics::Matrix4(trackObject.mObjectToWorld));
	////}

	//theRacetrackBroadcaster.SendEvent(GameState::Events::RacetrackObjectEvent(GameState::Events::Racetrack::AddObject, objectHandle));
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameState::Implementation::RacetrackLoader::TryCreateLogicObject(
	const TrackBundler::Legacy::TrackObject& trackObject, const TrackBundler::Legacy::TrackBundle& /*trackBundle*/)
{
	const TrackBundler::Legacy::TrackObjectDefinition& objectDefinition = theTrackObjectDefinitions[trackObject.mDefinitionIndex];

	const tbCore::tbString objectTypeName = tbCore::String::Lowercase(objectDefinition.mDisplayName);
	if ("zone spawn point" == objectTypeName)
	{
		int gridIndex = trackObject.mProperties["index"].AsRangedInteger<int>();
		tb_always_log(LogState::Always() << "Setting GridSpot[" << gridIndex << "] to: ( " << trackObject.mObjectToWorld.GetPosition().x << ", " <<
			trackObject.mObjectToWorld.GetPosition().z << " ).");

		theGridSpotsToWorld[gridIndex] = static_cast<icePhysics::Matrix4>(trackObject.mObjectToWorld);
	}
	else if (true == tbCore::StringContains(objectTypeName, "trigger box"))
	{
		const tbCore::tbString triggerType = trackObject.mProperties["type"].AsString();
		if ("gate" == triggerType)
		{
			const TimingState::CheckpointIndex checkpointIndex = trackObject.mProperties["index"].AsRangedInteger<TimingState::CheckpointIndex::Integer>();
			const bool withCutPenalty = trackObject.mProperties["cut_penalty"].AsBooleanWithDefault(false);
			TimingState::AddCheckpoint(static_cast<icePhysics::Matrix4>(trackObject.mObjectToWorld), checkpointIndex, withCutPenalty);
		}
	}
	else
	{	//Did not find a logic object to create.
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::Implementation::RacetrackLoader::OnCreateTrackSpline(
	const TrackBundler::Legacy::TrackSpline& trackSpline, const TrackBundler::Legacy::TrackBundle& /*trackBundle*/)
{
	const TrackBundler::Legacy::TrackSplineDefinition& splineDefinition = theTrackSplineDefinitions[trackSpline.mDefinitionIndex];
	if ("Simple Road" == splineDefinition.mDisplayName)
	{
		tb_error_if(false == TheTrackNodes().empty(), "Error: Expected TheTrackNodes container to be empty, is there more than one racetrack?");
		tb_error_if(false == theTrackNodeEdges.empty(), "Error: Expected TrackNodeEdges to be empty, is there more than one racetrack?");

		{	//Build all the TrackNodes / TrackNodeEdges from the TrackCurve.
			std::vector<tbMath::Vector3> pointsOnSpline;

			pointsOnSpline.clear();
			pointsOnSpline.reserve(trackSpline.mNodes.size());
			for (size_t nodeIndex = 0; nodeIndex < trackSpline.mNodes.size(); ++nodeIndex)
			{
				pointsOnSpline.push_back(trackSpline.mNodes[nodeIndex].mNodeToSpline.GetPosition());
			}

			// @note 2023-11-04: Technically TrackBundler should be creating the trackCurve for us. We are assuming CatMullRomBeau
			//   where future possibilities may open up...
			const tbMath::BezierCurve trackCurve = tbMath::BezierCurve::FromCatMullRomBeau(pointsOnSpline, trackSpline.mIsClosedLoop);

			std::vector<tbMath::Vector3> centerPoints;
			std::vector<tbMath::Vector3> centerTangents;
			std::vector<float> teeValues;
			trackCurve.GetInformationByDistance(centerPoints, centerTangents, teeValues, 10.0f, 1000);
			tb_error_if(centerPoints.size() != centerTangents.size(), "Expected both center points and tangents to have the same size.");

			GameState::RacetrackState::TrackNodeEdge nodeEdge;
			tbMath::Vector3 trackRightHalfWidth;
			const float halfTrackWidth = 4.75f;

			for (size_t index = 0; index < centerPoints.size(); ++index)
			{
				trackRightHalfWidth = tbMath::Vector3::Cross(centerTangents[index], WorldUp()).GetNormalized() * halfTrackWidth;

				nodeEdge[TrackEdge::kCenter] = centerPoints[index];
				nodeEdge[TrackEdge::kRight] = centerPoints[index] + trackRightHalfWidth;
				nodeEdge[TrackEdge::kLeft] = centerPoints[index] - trackRightHalfWidth;
				theTrackNodeEdges.push_back(nodeEdge);

				if (index > 0)
				{
					const RacetrackState::TrackNodeEdge& leadingEdge = theTrackNodeEdges[theTrackNodeEdges.size() - 1];
					const RacetrackState::TrackNodeEdge& trailingEdge = theTrackNodeEdges[theTrackNodeEdges.size() - 2];

					TheMutableTrackNodes().emplace_back(TrackNode{ //leading, trailing, left, right
						icePhysics::BoundingPlane(leadingEdge[TrackEdge::kCenter], icePhysics::Vector3::Cross(
							Up(), leadingEdge[TrackEdge::kRight] - leadingEdge[TrackEdge::kLeft])),
						icePhysics::BoundingPlane(trailingEdge[TrackEdge::kCenter], icePhysics::Vector3::Cross(
							trailingEdge[TrackEdge::kRight] - trailingEdge[TrackEdge::kLeft], Up())),
						icePhysics::BoundingPlane(leadingEdge[TrackEdge::kLeft], icePhysics::Vector3::Cross(
							Up(), leadingEdge[TrackEdge::kLeft] - trailingEdge[TrackEdge::kLeft])),
						icePhysics::BoundingPlane(leadingEdge[TrackEdge::kRight], icePhysics::Vector3::Cross(
							leadingEdge[TrackEdge::kRight] - trailingEdge[TrackEdge::kRight], Up())),
					});
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//
