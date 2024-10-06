///
/// @file
/// @details A simple graphic to display the Racetrack.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/graphics_3d/racetrack_graphic.hpp"
#include "../../game_client/graphics_3d/spectator_graphic.hpp"
#include "../../game_state/racetrack_state.hpp"
#include "../../game_state/object_state.hpp"
#include "../../game_state/events/racetrack_events.hpp"
#include "../../core/utilities.hpp"
#include "../../ludumdare56.hpp"
#include "../../logging.hpp"

#include <track_bundler/track_bundler_to_ice_graphics.hpp>


//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

namespace LudumDare56::GameClient
{
	class MeshComponent : public GameState::ComponentState
	{
	public:
		MeshComponent(GameState::ObjectState& object, TrackBundler::GraphicPtr&& graphic) :
			GameState::ComponentState(object),
			mGraphic(std::move(graphic))
		{
			mGraphic->SetVisible(IsActive());
		}

		virtual ~MeshComponent(void)
		{
		}

		virtual void OnUpdate(const float /*deltaTime*/)
		{
			mGraphic->SetObjectToWorld(mObject.GetObjectToWorld());
		}

		virtual void OnActivate(void)
		{
			mGraphic->SetVisible(true);
		}

		virtual void OnDeactivate(void)
		{
			mGraphic->SetVisible(false);
		}

	protected:
		typedef std::unique_ptr<iceGraphics::Graphic> GraphicPointer;
		GraphicPointer mGraphic;
	};


	class GraphicComponent : public GameState::ComponentState
	{
	public:
		typedef TrackBundler::GraphicPtr GraphicPointer;
		typedef TrackBundler::GraphicsContainer GraphicsContainer;

		//explicit GraphicComponent(GameState::ObjectState& object, GraphicPointer&& inputGraphic) :
		//	GameState::ComponentState(object),
		//	mGraphics({ std::move(inputGraphic) })
		//{
		//	for (auto& graphic : mGraphics)
		//	{
		//		graphic->SetVisible(IsActive());
		//	}
		//}

		explicit GraphicComponent(GameState::ObjectState& object, GraphicsContainer&& graphics) :
			GameState::ComponentState(object),
			mGraphics(std::move(graphics))
		{
			for (auto& graphic : mGraphics)
			{
				graphic->SetVisible(IsActive());
			}
		}

		virtual ~GraphicComponent(void)
		{
		}

		virtual void OnUpdate(const float /*deltaTime*/)
		{
			for (auto& graphic : mGraphics)
			{
				graphic->SetObjectToWorld(mObject.GetObjectToWorld());
			}
		}

		virtual void OnActivate(void)
		{
			for (auto& graphic : mGraphics)
			{
				graphic->SetVisible(true);
			}
		}

		virtual void OnDeactivate(void)
		{
			for (auto& graphic : mGraphics)
			{
				graphic->SetVisible(false);
			}
		}

	protected:
		GraphicsContainer mGraphics;
	};

	class DecalComponent : public GameState::ComponentState
	{
	public:
		DecalComponent(GameState::ObjectState& object, const TrackBundler::Component& decalComponent) :
			GameState::ComponentState(object),
			mDecal(TrackBundler::CreateGraphicFromDecalComponent(decalComponent, object.GetObjectToWorld()))
		{
			mDecal->SetVisible(IsActive());
		}

		virtual ~DecalComponent(void)
		{
		}

		virtual void OnUpdate(const float /*deltaTime*/)
		{
			mDecal->SetObjectToWorld(mObject.GetObjectToWorld());
		}

		virtual void OnActivate(void)
		{
			mDecal->SetVisible(true);
		}

		virtual void OnDeactivate(void)
		{
			mDecal->SetVisible(false);
		}


	protected:
		typedef std::unique_ptr<iceGraphics::Decal> DecalPointer;
		DecalPointer mDecal;
	};
};

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::RacetrackGraphic::RacetrackGraphic(void) :
	mRacetrackGraphics()
{
	GameState::RacetrackState::AddEventListener(*this);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::RacetrackGraphic::~RacetrackGraphic(void)
{
	GameState::RacetrackState::RemoveEventListener(*this);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ComponentStatePtr LudumDare56::GameClient::RacetrackGraphic::OnCreateComponent(
	GameState::ObjectState& object, const TrackBundler::Component& componentInformation)
{
	if (TrackBundler::ComponentDefinition::kMeshKey == componentInformation.mDefinitionKey)
	{
		GraphicPointer graphic = TrackBundler::CreateGraphicFromMeshComponent(componentInformation, object.GetObjectToWorld());
		if (nullptr != graphic)
		{
			return GameState::ComponentStatePtr(new MeshComponent(object, std::move(graphic)));
		}
	}
	else if (TrackBundler::ComponentDefinition::kSplineMeshKey == componentInformation.mDefinitionKey)
	{
		const TrackBundler::ResourceKey meshResourceKey = TrackBundler::ResourceKey::FromString(componentInformation.mProperties["mesh"].AsString());
		const String meshFilepath = TrackBundler::MasterResourceTable::Get().GetResource(meshResourceKey).mFilepath;

		if (true == tbCore::StringContains(object.GetName(), "_collider") || true == tbCore::StringContains(meshFilepath, "_collider"))
		{	//Don't do...
			return nullptr;
		}

		//We need to find the SplinePath component on this object.

		const TrackBundler::NodeKey nodeKey = static_cast<TrackBundler::NodeKey>(object.GetID());
		const TrackBundler::Component& splinePath = GetComponentByType(nodeKey, TrackBundler::ComponentDefinition::kSplinePathKey);
		if (TrackBundler::ComponentKey::Invalid() != splinePath.mComponentKey &&
			TrackBundler::ComponentDefinition::kSplinePathKey == splinePath.mDefinitionKey)
		{
			GraphicComponent::GraphicsContainer graphics;
			TrackBundler::CreateGraphicsFromSplineComponent(graphics, splinePath, componentInformation, object.GetObjectToWorld());
			return GameState::ComponentStatePtr(new GraphicComponent(object, std::move(graphics)));
		}
		//object.GetComponent
	}
	else if (TrackBundler::ComponentDefinition::kDecalKey == componentInformation.mDefinitionKey)
	{
		return GameState::ComponentStatePtr(new DecalComponent(object, componentInformation));
	}

	//else if (CustomComponentDefinition::kGrandStandKey == componentInformation.mDefinitionKey)
	else if (CustomComponent::GrandStand == componentInformation.mDefinitionKey)
	{
		SpectatorGraphic::SpawnSpectatorsAt(static_cast<Matrix4>(object.GetObjectToWorld()));
	}

	return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacetrackGraphic::GenerateFrom(const TrackBundler::Legacy::TrackBundle& trackBundle,
	const TrackBundler::Legacy::TrackSegmentDefinitionContainer& /*segmentDefinitions*/,
	const TrackBundler::Legacy::TrackObjectDefinitionContainer& /*objectDefinitions*/,
	const TrackBundler::Legacy::TrackSplineDefinitionContainer& splineDefinitions)
{
	// @note 2023-11-10: While we are clearing the racetrack and decals here, any object that gets created through the
	//   Events like AddObject or such need to clear/cleanup during ClearObjects() rather than here, otherwise they
	//   get cleared IMMEDIATELY after being added. Unfortunately the RacetrackState is forced to call these in a weird
	//   order due to TrackBundler and processing the objects, sure makes this annoying.
	mRacetrackGraphics.clear();
	mDecals.clear();

//	TrackBundler::CreateGraphicsFromObjects(mRacetrackGraphics, trackBundle, objectDefinitions);
	TrackBundler::Legacy::CreateGraphicsFromSplines(mRacetrackGraphics, trackBundle, splineDefinitions);

	for (const TrackBundler::Legacy::TrackDecal& trackDecal : trackBundle.mTrackDecals)
	{
		mDecals.emplace_back(new iceGraphics::Decal(trackDecal.mMaterialFile, trackDecal.mDecalToWorld));
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacetrackGraphic::Update(const float deltaTime)
{
	for (ObjectGraphicPair& pair : mRacetrackObjectGraphics)
	{
		const GameState::ObjectState& objectState = GameState::RacetrackState::GetObjectState(pair.first);
		const tbMath::Matrix4 objectToWorld = static_cast<tbMath::Matrix4>(objectState.GetObjectToWorld());
		for (GraphicPointer& graphic : pair.second)
		{
			graphic->SetObjectToWorld(objectToWorld);
		}
	}

	SpectatorGraphic::UpdateAllSpectators(deltaTime);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacetrackGraphic::OnHandleEvent(const TyreBytes::Core::Event& event)
{
	switch (event.GetID())
	{
	case GameState::Events::Racetrack::NewRacetrack: {
		const GameState::Events::CreateRacetrackEvent& createEvent = event.As<GameState::Events::CreateRacetrackEvent>();
		GenerateFrom(createEvent.mTrackBundle, createEvent.mSegmentDefinitions, createEvent.mObjectDefinitions, createEvent.mSplineDefinitions);
		break; }

	case GameState::Events::Racetrack::ClearObjects: {
		mRacetrackObjectGraphics.clear();
		SpectatorGraphic::ClearAllSpectators();

		break; }
	case GameState::Events::Racetrack::AddObject: {
		//const GameState::Events::RacetrackObjectEvent& objectEvent = event.As<GameState::Events::RacetrackObjectEvent>();
		//const GameState::ObjectState& objectState = GameState::RacetrackState::GetObjectState(objectEvent.GetObjectHandle());

		//std::vector<iceGraphics::Graphic*> graphics = TrackBundler::CreateGraphicsFrom(objectState.GetDefinition(),
		//	static_cast<tbMath::Matrix4>(objectState.GetObjectToWorld()));

		//GraphicContainer realGraphics;
		//for (iceGraphics::Graphic* graphic : graphics)
		//{
		//	realGraphics.emplace_back(graphic);
		//}

		//mRacetrackObjectGraphics.push_back(ObjectGraphicPair(objectEvent.GetObjectHandle(), std::move(realGraphics)));

		//const tbCore::tbString objectTypeName = tbCore::String::Lowercase(objectState.GetDefinition().mDisplayName);
		//if ("grand stands" == objectTypeName)
		//{
		//	SpectatorGraphic::SpawnSpectatorsAt(static_cast<Matrix4>(objectState.GetObjectToWorld()));
		//}


		////	TrackBundler::CreateGraphicsFromObjects(mRacetrackGraphics, trackBundle, objectDefinitions);

		break; }
	case GameState::Events::Racetrack::RemoveObject: {
		break; }
	};
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacetrackGraphic::SetVisible(bool visible)
{
	for (auto& graphic : mRacetrackGraphics)
	{
		graphic->SetVisible(visible);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacetrackGraphic::RenderDebug(void) const
{
	mDebugVisuals.Render();

	GameState::RacetrackState::RenderDebug();
}

//--------------------------------------------------------------------------------------------------------------------//
