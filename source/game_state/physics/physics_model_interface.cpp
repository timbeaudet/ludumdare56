///
/// @file
/// @details The most basic of all physics models separated out from the racecars.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_state/physics/physics_model_interface.hpp"
#include "../../game_state/physics/model_extremely_basic.hpp"
#include "../../game_state/physics/model_extremely_fast.hpp"
#include "../../game_state/physics/model_extreme_drifting.hpp"

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::PhysicsModels::PhysicsModelInterface::PhysicsModelInterface(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::PhysicsModels::PhysicsModelInterface::~PhysicsModelInterface(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::PhysicsModelInterface::SetEnabled(bool isEnabled)
{
	OnSetEnabled(isEnabled);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::PhysicsModelInterface::ResetRacecarForces(void)
{
	OnResetRacecarForces();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::PhysicsModelInterface::Simulate(const RacecarControllerInterface& racecarController)
{
	OnSimulate(racecarController);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::PhysicsModelInterface::DebugRender(void)
{
	OnDebugRender();
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::PhysicsModels::PhysicsModelInterfacePtr LudumDare56::GameState::PhysicsModels::Instantiate(
	icePhysics::World& physicalWorld, const PhysicsModel physicsModel)
{
	switch (physicsModel)
	{
	case PhysicsModel::NullModel:             return std::make_unique<NullPhysicsModel>();
	case PhysicsModel::ExtremelyBasic:        return std::make_unique<ExtremelyBasicsPhysicsModel>(physicalWorld);
	case PhysicsModel::ExtremelyFast:         return std::make_unique<ExtremelyFastPhysicsModel>(physicalWorld);
	case PhysicsModel::ExtremeDrifting:       return std::make_unique<ExtremeDriftingPhysicsModel>(physicalWorld);
	};

	tb_error("Unknown physics model, did you add a case to Instantiate?");
	return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::PhysicsModels::NullPhysicsModel::GetVehicleToWorld(void) const
{
	return iceMatrix4::Identity();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::NullPhysicsModel::SetVehicleToWorld(const iceMatrix4& /*vehicleToWorld*/)
{
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::PhysicsModels::NullPhysicsModel::GetBodyToWorld(void) const
{
	return iceMatrix4::Identity();
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::PhysicsModels::NullPhysicsModel::GetWheelToWorld(const size_t /*wheelIndex*/) const
{
	return iceMatrix4::Identity();
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::PhysicsModels::NullPhysicsModel::GetAngularVelocity(void) const
{
	return iceVector3::Zero();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::NullPhysicsModel::SetAngularVelocity(const iceVector3& /*angularVelocity*/)
{
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::PhysicsModels::NullPhysicsModel::GetLinearVelocity(void) const
{
	return iceVector3::Zero();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::NullPhysicsModel::SetLinearVelocity(const iceVector3& /*linearVelocity*/)
{
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::RaycastVehiclePhysicsModelInterface(
	icePhysics::World& physicalWorld, const icePhysics::VehicleInfo& vehicleInfo) :

	PhysicsModelInterface(),
	mPhysicalWorld(physicalWorld),
	mPhysicalVehicle(vehicleInfo)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::~RaycastVehiclePhysicsModelInterface(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::GetVehicleToWorld(void) const
{
	return mPhysicalVehicle.GetVehicleToWorld();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::SetVehicleToWorld(const iceMatrix4& vehicleToWorld)
{
	mPhysicalVehicle.SetVehicleToWorld(vehicleToWorld);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::GetBodyToWorld(void) const
{
	return mPhysicalVehicle.GetVehicleToWorld();
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::GetWheelToWorld(const size_t wheelIndex) const
{
	return mPhysicalVehicle.GetWheelToWorld(wheelIndex);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::GetAngularVelocity(void) const
{
	return mPhysicalVehicle.GetAngularVelocity();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::SetAngularVelocity(const iceVector3& angularVelocity)
{
	mPhysicalVehicle.SetAngularVelocity(angularVelocity);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::GetLinearVelocity(void) const
{
	return mPhysicalVehicle.GetLinearVelocity();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::SetLinearVelocity(const iceVector3& linearVelocity)
{
	mPhysicalVehicle.SetLinearVelocity(linearVelocity);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::OnSetEnabled(bool isEnabled)
{
	if (true == isEnabled)
	{
		mPhysicalWorld.AddBody(*mPhysicalVehicle.HackyAPI_GetRigidBody());
	}
	else
	{
		mPhysicalWorld.RemoveBody(mPhysicalVehicle.HackyAPI_GetRigidBody());

	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::OnResetRacecarForces(void)
{
	SetAngularVelocity(iceVector3::Zero());
	SetLinearVelocity(iceVector3::Zero());
	mPhysicalVehicle.ClearForcesAndTorque();
	mPhysicalVehicle.ClearWheelForces();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::OnSimulate(const RacecarControllerInterface& /*racecarController*/)
{
	mPhysicalVehicle.Simulate(mPhysicalWorld, kFixedTime);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::RaycastVehiclePhysicsModelInterface::OnDebugRender(void) const
{
	mPhysicalVehicle.DebugRender();
}

//--------------------------------------------------------------------------------------------------------------------//
