///
/// @file
/// @details Implements the ComponentCreatorInterface to create the components for LudumDare56 state/logic.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_state/components/component_creator.hpp"
//#include "../../game_state/components/box_collider_component.hpp"
//#include "../../game_state/components/sphere_collider_component.hpp"
//#include "../../game_state/components/mesh_collider_component.hpp"
//#include "../../game_state/components/rigid_body_component.hpp"
//#include "../../game_state/components/spawn_point_component.hpp"
//#include "../../game_state/components/finish_area_component.hpp"
//#include "../../game_state/components/boost_pickup_component.hpp"

#include "../../logging.hpp"

#include <track_bundler/track_bundler.hpp>

namespace ComponentDefinitions
{
	typedef TrackBundler::ComponentDefinitionKey ComponentDefinitionKey;

	//const ComponentDefinitionKey kRigidBody = TrackBundler::ComponentDefinition::kRio

	//const ComponentDefinitionKey kStaticPhysicsObject = ComponentDefinitionKey::FromString("762dab40-461e-4923-bce1-93fecfe81954");
	//const ComponentDefinitionKey kSpawnPointComponent = ComponentDefinitionKey::FromString("bc5fd8fd-332a-4102-af0e-8ab87f4c57c1");
	//const ComponentDefinitionKey kFinishAreaComponent = ComponentDefinitionKey::FromString("4620568c-583a-4194-aa64-de3ccba8ddac");
	//const ComponentDefinitionKey kBoostPickupComponent = ComponentDefinitionKey::FromString("1eaf62de-da8a-4b97-b7e5-2dbba5a2a4c4");

};

//--------------------------------------------------------------------------------------------------------------------//

namespace LudumDare56::GameState
{
	ComponentCreator theComponentCreator;
};

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ComponentCreator::~ComponentCreator(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ComponentStatePtr LudumDare56::GameState::ComponentCreator::OnCreateComponent(
	ObjectState& object, const TrackBundler::Component& componentInformation)
{
	tb_unused(object);
	tb_unused(componentInformation);

	//const tbCore::DynamicStructure& componentProperties = componentInformation.mProperties;

	//if (TrackBundler::ComponentDefinition::kColliderBoxKey == componentInformation.mDefinitionKey)
	//{
	//	const iceMatrix4 objectToWorld = static_cast<iceMatrix4>(object.GetObjectToWorld());

	//	const iceVector3 scale(
	//		static_cast<iceScalar>(objectToWorld.GetBasis(0).Magnitude()),
	//		static_cast<iceScalar>(objectToWorld.GetBasis(1).Magnitude()),
	//		static_cast<iceScalar>(objectToWorld.GetBasis(2).Magnitude())
	//	);

	//	icePhysics::Vector3 center;
	//	center.x = componentProperties["center"][0].AsFloat();
	//	center.y = componentProperties["center"][1].AsFloat();
	//	center.z = componentProperties["center"][2].AsFloat();

	//	icePhysics::Vector3 size;
	//	size.x = componentProperties["size"][0].AsFloat() * scale.x;
	//	size.y = componentProperties["size"][1].AsFloat() * scale.y;
	//	size.z = componentProperties["size"][2].AsFloat() * scale.z;

	//	const bool isTrigger = componentProperties["trigger"].AsBoolean();

	//	return ComponentStatePtr(new BoxColliderComponent(object, center, size, isTrigger));
	//}
	//else if (TrackBundler::ComponentDefinition::kColliderSphereKey == componentInformation.mDefinitionKey)
	//{
	//	icePhysics::Vector3 center;
	//	center.x = componentProperties["center"][0].AsFloat();
	//	center.y = componentProperties["center"][1].AsFloat();
	//	center.z = componentProperties["center"][2].AsFloat();

	//	const icePhysics::Scalar radius = componentProperties["radius"].AsFloat();
	//	const bool isTrigger = componentProperties["trigger"].AsBoolean();

	//	return ComponentStatePtr(new SphereColliderComponent(object, center, radius, isTrigger));
	//}
	//else if (TrackBundler::ComponentDefinition::kColliderMeshKey == componentInformation.mDefinitionKey)
	//{
	//	ResourceKey meshResourceKey = ResourceKey::FromString(componentProperties["mesh"].AsString());
	//	//const tbCore::tbString meshFile = trackBundle.mImprovedBundle.mResourceTable.GetResource(meshResourceKey).mFilepath;
	//	const tbCore::tbString meshFile = TrackBundler::MasterResourceTable::GetResource(meshResourceKey).mFilepath;
	//	const iceCore::MeshHandle meshHandle = iceCore::theMeshManager.CreateMeshFromFile(meshFile);
	//	const bool isTrigger = componentProperties["trigger"].AsBoolean();

	//	tb_always_log("NUMBER OF MESH COLLIDER FACES: " << icePhysics::MeshCollider(meshFile).GetIndices().size() / 3);

	//	return ComponentStatePtr(new MeshColliderComponent(object, meshHandle, isTrigger));
	//}
	//// Above is supposedly "built-into" TrackBundler / Engine. (or _should_ be)
	//// Below is supposedly game-specific for Snailed It!
	//else if (ComponentDefinitions::kStaticPhysicsObject == componentInformation.mDefinitionKey)
	//{
	//	return ComponentStatePtr(new RigidBodyComponent(object, -1.0));
	//}
	//else if (ComponentDefinitions::kSpawnPointComponent == componentInformation.mDefinitionKey)
	//{
	//	const GridIndex spawnIndex = componentProperties["index"].AsRangedInteger<GridIndex::Integer>();
	//	return ComponentStatePtr(new SpawnPointComponent(object, spawnIndex));
	//}
	//else if (ComponentDefinitions::kFinishAreaComponent == componentInformation.mDefinitionKey)
	//{
	//	return ComponentStatePtr(new FinishAreaComponent(object));
	//}
	//else if (ComponentDefinitions::kBoostPickupComponent == componentInformation.mDefinitionKey)
	//{
	//	return ComponentStatePtr(new BoostPickupComponent(object));
	//}

	return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------//
