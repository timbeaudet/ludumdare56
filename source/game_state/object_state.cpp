///
/// @file
/// @details Contains the basic state of an object at or on the racetrack in LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "object_state.hpp"

#include "logging.hpp"

#include <track_bundler/track_bundler.hpp>

//--------------------------------------------------------------------------------------------------------------------//


// Much of this is summarized at the bottom, start with the portion dated 2024-03-01: Summary:
//
// 2024-02-28: The plan of attack is to make an ObjectState a node that has a hierarchy like we built in TrackBuilder
//   as well as some super basic Node-Component system; unlikely to have the performance benefits of the whole complex
//   system right now, but rather each Node will update each of the Components it contains. This will be rather simple
//   and quick to build and is specific to Snailed It!.
//
//   We will need some form of a ComponentCreatorFactory to actually create the various "ComponentState" objects/instances
//   and the ObjectState will need that in order to build up its components from TrackBundler::ComponentContainer. This
//   factory will need to be extended, or somehow chained together in order for the GameClient to create client specific
//   Components like the MeshComponent.
//
//   The ObjectState previously contained a RigidBody which is now becoming a RigidBodyComponent, this will be a bit of
//   a challenge since the icePhysics::RigidBody has a transform and the ObjectState node (from tbCore) has a transform
//   and those will need stay in sync.
//
//   Potential problem: The RigidBody component would appear to dig through all other components, including any children
//   components, searching for Collider components, BoxCollider, SphereCollider etc... This seems to create a bit of a
//   race condition as it requires the whole hierarcy to be setup, and ALL the components to be setup, before initializing?
//   Perhaps this is the object/point of Start() in Unity? I've created an OnAdd() but that wouldn't have the right timing.
//      I think we will need some form of Awake/Start function that gets called after the whole chain/components were
//      were setup instead of having an OnAdd()/OnRemove(). It seems the Awake()/OnDestroy() is Unity's version of Add/Remove
//      but get called after(Awake)/before(Destroy) the Object chain is built, including hierarchy and components...
//
//		2024-03-01: Went in the direction of Awake() being called after the hierarch & components were loaded/created.
//
// 2024-02-28: There might be use in creating a separation between LogicalComponent's and VisualComponent's where the
//   LogicalComponent would hold-onto a visual component for the GameClient to create and render. With this thought, I
//   am thinking a "Snail" component would have a LogicalSnail inside the GameState and a VisualSnail inside GameClient
//   where both would be using the same component info, or even the Visual knows the Logics...
//
//   2024-03-01: I actually think that was bad example. The Snail object would have a SnailLogic component and also a
//     separate component for visuals, mostly MeshComponent. That is how the separation should work, rather than trying
//     to use one component definition/key and have two components created from it.
//
// 2024-02-28: I left off with one ComponentCreator creating a StaticPhysicsObject with a rigidbody for the static objects/
//   However we haven't yet created the Colliders (Box, Sphere, Mesh etc) as components and the RigidBody component will
//   require looking through all components on the object and it's children for Colliders that are not triggers in order
//   to add them to the RigidBody correctly. This will need to use an OnAwake() type of call that is not yet implemented.
//
//     Instead of creating BoxColliderComponent, SphereColliderComponent, etc - we might just want ColliderVolumesComponent
//        that collects all the volumes, Box, Sphere, Mesh etc into container(s) separating Trigger vs non-Trigger. This
//        has an advantage of writing ONE component for ALL colliders, but may be more complicated than desired, and less
//        flexible since you can't adjust the box collider OnAwake through code.
//
//     2024-03-01: We went with each separate collider as a separate component. This represents Components a bit better
//		  and prevents oddities that would arise from One code-Component matching several different editor-Component types.
//
//
// 2024-02-29: ComponentState will need a function for OnActivate() on OnUnactivate() (or some better naming) for the
//   components to handle situations correctly. Like if you SetActive(false) on a RigidBody it will need to remove itself
//   from the World, and then readd as necessary. That said this gets a little more complicated when the ObjectState will
//   also need a separate OnActivate()/OnUnactivate etc, and when the Object changes activation state it will need to
//   call the attached Components activation notices appropriately without changing the components active state; as in
//   if the RigidBodyComponent was enabled, and the object just disabled, the RigidBodyComponent needs OnUnactivate() to
//   be invoked, but to also remain active so when the object is active things still work...
//
//   2024-03-01: There will certainly be more Activate/Deactive bugs to work out, but Nodes/Components have these two
//     callbacks now and SetActive() will call them. It got a little more complicated thinking about parent/child and
//     activating/deactivating the components vs object, but it seems to be a good start.
//
// 2024-03-01: Summary: This concludes the first pass of this major refactor into Node/Components. The node hierarchy
//   hasn't really been used or tested very much, and Track Builder needs work done before it could be more heavily
//   played with. As of right now the ObjectHandle, GetObjectState() and Racetrack::AddObject, Racetrack::RemoveObject
//   events are not used and could be removed. It felt too useful to keep AddObject/RemoveObject for events unknown at
//   the present time to just throw the code away. If it hasn't been used mid-way through alpha, remove it.
//
//   The bit above about RigidBodies and Colliders is still issue, though will only be an issue once node hierarchies
//   are tested and used more thoroughly, also we would need some actual RigidBodies (as of right now all are static).
//
//   A couple other things, OnDestroy() is not yet called, and I'm not convinced cleanup is amazing (no level reset or
//   loading into other tracks to see if that works or not). Another major future problem will be cloning / duplicating
//   the Node/Components programmatically. Say I made a Cloud object and wanted to duplicate it several times, that is
//   not close to possible right now; We might need TrackBundler::InstantiatePrefab type thing to exist in game code api
//   as well as TrackBuilder...

namespace
{
	std::list<LudumDare56::GameState::ComponentCreatorInterface*>& TheComponentCreators(void)
	{
		static std::list<LudumDare56::GameState::ComponentCreatorInterface*> theCreators;
		return theCreators;
	}

	const TrackBundler::Component kInvalidComponent;
};


//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ComponentCreatorInterface::ComponentCreatorInterface(void) :
	mTrackBundle(nullptr)
{
	TheComponentCreators().push_back(this);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ComponentCreatorInterface::~ComponentCreatorInterface(void)
{
	TheComponentCreators().remove(this);
}

//--------------------------------------------------------------------------------------------------------------------//

const TrackBundler::Component& LudumDare56::GameState::ComponentCreatorInterface::GetComponent(
	const TrackBundler::NodeKey& nodeKey, const TrackBundler::ComponentKey& componentKey)
{
	tb_error_if(nullptr == mTrackBundle, "Expected a valid track bundle to GetComponent from.");

	// 2024-10-03: Okay, so this laughable, or will be. We don't have a NodeCluster because ... we are silly? So we
	//   can't just use TrackBundler::NodeFramework:: like was planned... We have a total mess due to legacy decisions.
	//
	//   The TrackBundle format holds the nodes in a flat vector to iterate until we find a matching NodeKey. Then we
	//   can use that nodeIndex to grab the ComponentContainer for that Node according to the bundle. Finally we can
	//   search the ComponentContainer for the matching componentKey.
	size_t nodeIndex = 0;
	for (const TrackBundler::Node& node : mTrackBundle->mNodeHierarchy)
	{
		if (nodeKey == node.mNodeKey)
		{
			for (const TrackBundler::Component& component : mTrackBundle->mNodeComponents[nodeIndex])
			{
				if (componentKey == component.mComponentKey)
				{
					return component;
				}
			}

			return kInvalidComponent;
		}
		++nodeIndex;
	}

	return kInvalidComponent;
}

//--------------------------------------------------------------------------------------------------------------------//

const TrackBundler::Component& LudumDare56::GameState::ComponentCreatorInterface::GetComponentByType(
	const TrackBundler::NodeKey& nodeKey, const TrackBundler::ComponentDefinitionKey& definitionKey)
{
	tb_error_if(nullptr == mTrackBundle, "Expected a valid track bundle to GetComponent from.");

	// 2024-10-03: Okay, so this laughable, or will be. We don't have a NodeCluster because ... we are silly? So we
	//   can't just use TrackBundler::NodeFramework:: like was planned... We have a total mess due to legacy decisions.
	//
	//   The TrackBundle format holds the nodes in a flat vector to iterate until we find a matching NodeKey. Then we
	//   can use that nodeIndex to grab the ComponentContainer for that Node according to the bundle. Finally we can
	//   search the ComponentContainer for the matching componentKey.
	size_t nodeIndex = 0;
	for (const TrackBundler::Node& node : mTrackBundle->mNodeHierarchy)
	{
		if (nodeKey == node.mNodeKey)
		{
			for (const TrackBundler::Component& component : mTrackBundle->mNodeComponents[nodeIndex])
			{
				if (definitionKey == component.mDefinitionKey)
				{
					return component;
				}
			}

			return kInvalidComponent;
		}
		++nodeIndex;
	}

	return kInvalidComponent;
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ComponentStatePtr LudumDare56::GameState::ComponentState::CreateComponent(
	ObjectState& object, const TrackBundler::Component& componentInformation, const TrackBundler::ImprovedTrackBundle& trackBundle)
{
	for (ComponentCreatorInterface* componentCreator : TheComponentCreators())
	{
		componentCreator->mTrackBundle = &trackBundle;
		ComponentStatePtr component = componentCreator->OnCreateComponent(object, componentInformation);
		componentCreator->mTrackBundle = nullptr;

		if (nullptr != component)
		{
			component->mDefinitionKey = componentInformation.mDefinitionKey;
			component->mIsActive = componentInformation.mIsActive;
			return component;
		}
	}

	return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameState::ComponentState::IsActive(void) const
{
	return mIsActive && mObject.IsActive();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::ComponentState::SetActive(const bool isActive)
{
	const bool wasActiveSelf = mIsActive;
	mIsActive = isActive;

	if (true == wasActiveSelf && false == isActive)
	{
		if (true == mObject.IsActive())
		{	//If the object was already inactive, then the component didn't just deactivate...
			OnDeactivate();
		}
	}
	else if (false == wasActiveSelf && true == isActive)
	{
		if (true == IsActive())
		{	//The object might not be active, and if not we shouldn't call Activate.
			OnActivate();
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ComponentState::ComponentState(ObjectState& object) :
	mObject(object),
	mDefinitionKey(TrackBundler::ComponentDefinitionKey::Invalid()),
	mIsActive(true)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ComponentState::~ComponentState(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ObjectState::ObjectState(const TrackBundler::Node& objectNode) :
	tbCore::Node(static_cast<tbCore::uuid>(objectNode.mNodeKey)),
	mComponents()
{
	SetActive(objectNode.IsActive());
	SetName(objectNode.GetName());
	SetObjectToWorld(objectNode.GetNodeToWorld());
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ObjectState::~ObjectState(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::ObjectState::OnActivate(void)
{
	for (ComponentState& component : AllComponents())
	{
		if (true == component.IsActive())
		{
			component.OnActivate();
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::ObjectState::OnDeactivate(void)
{
	for (ComponentState& component : AllComponents())
	{
		if (true == component.IsActive())
		{
			component.OnDeactivate();
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::ObjectState::AddComponent(ComponentStatePtr&& component)
{
	if (nullptr == component || &component->GetObject() != this)
	{
		tb_error("Expected a valid component with a matching object.");
		return;
	}

	mComponents.emplace_back(std::move(component));
}

//--------------------------------------------------------------------------------------------------------------------//
