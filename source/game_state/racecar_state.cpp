///
/// @file
/// @details Manage all the racecars in the simulation on both game and server side.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../game_state/racecar_state.hpp"
#include "../game_state/racetrack_state.hpp"
#include "../game_state/driver_state.hpp"
#include "../game_state/helpers/torque_curve.hpp"
#include "../game_state/events/racecar_events.hpp"
#include "../game_state/racecar_controller_interface.hpp"

#include "../logging.hpp"

#include <array>

namespace
{
	using LudumDare56::GameState::PhysicsModels::PhysicsModel;

	const PhysicsModel kGenericPhysicsModel = PhysicsModel::ExtremelyBasic;
	const PhysicsModel kTheBestPhysicsModel = PhysicsModel::ExtremeDrifting; //This is intended for "Tim's Awesome Car" / Tier3 support.

	std::vector<PhysicsModel> thePhysicsModels = {
		PhysicsModel::ExtremelyFast, PhysicsModel::ExtremelyBasic, PhysicsModel::ExtremeDrifting, PhysicsModel::ExtremelyBasic, PhysicsModel::NullModel
	};

	typedef std::array<LudumDare56::GameState::RacecarState, LudumDare56::GameState::kNumberOfRacecars> RacecarArray;
	RacecarArray& TheRacecarArray(void)
	{
		static RacecarArray theRacecarArray;
		return theRacecarArray;
	}
};

PhysicsModel GetRacecarPhysicsModel(tbCore::uint8 carID);

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::GameState::RacecarState& LudumDare56::GameState::RacecarState::Get(const RacecarIndex racecarIndex)
{
	return TheRacecarArray()[racecarIndex];
}

LudumDare56::GameState::RacecarState& LudumDare56::GameState::RacecarState::GetMutable(const RacecarIndex racecarIndex)
{
	return TheRacecarArray()[racecarIndex];
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarState::RacecarState(void) :
	mPhysicsModel(new PhysicsModels::NullPhysicsModel()),
	mController(new NullRacecarController()),
	mPhysicalWorld(nullptr),
	mOnTrackCounter(0),
	mRacecarIndex(InvalidRacecar()),
	mDriverIndex(InvalidDriver()),
	mRacecarMeshID(0),
	mIsOnTrack(false),
	mIsVisible(false)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarState::~RacecarState(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::Create(icePhysics::World& physicalWorld)
{
	tb_error_if(false == IsValidRacecar(mRacecarIndex), "Expected the RacecarIndex to be valid by Create().");

	mPhysicalWorld = &physicalWorld;
	mPhysicsModel = PhysicsModels::Instantiate(physicalWorld, GetRacecarPhysicsModel(mRacecarMeshID));

	RaceSessionState::PlaceCarOnGrid(*this);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::Destroy(icePhysics::World& /*physicalWorld*/)
{
	mPhysicalWorld = nullptr;
	mPhysicsModel.reset(new PhysicsModels::NullPhysicsModel());
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::ResetRacecar(const iceMatrix4& vehicleToWorld)
{
	SetVehicleToWorld(vehicleToWorld);
	mPhysicsModel->ResetRacecarForces();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetRacecarController(RacecarControllerInterface* controller)
{
	mController.reset((nullptr == controller) ? new NullRacecarController() : controller);
}

//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::GameState::RacecarControllerInterface& LudumDare56::GameState::RacecarState::GetRacecarController(void) const
{
	tb_error_if(nullptr == mController, "Expected the controller to always be valid, even if NullController.");
	return *mController;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarControllerInterface& LudumDare56::GameState::RacecarState::GetMutableRacecarController(void)
{
	tb_error_if(nullptr == mController, "Expected the controller to always be valid, even if NullController.");
	return *mController;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetRacecarMeshID(const tbCore::uint8 racecarMeshID)
{
	if (racecarMeshID != mRacecarMeshID)
	{
		mRacecarMeshID = racecarMeshID;

		if (nullptr != mPhysicalWorld)
		{
			if (true == IsRacecarInUse())
			{
				mPhysicsModel->SetEnabled(false);
			}

			mPhysicsModel = PhysicsModels::Instantiate(*mPhysicalWorld, GetRacecarPhysicsModel(mRacecarMeshID));

			if (true == IsRacecarInUse())
			{
				mPhysicsModel->SetEnabled(true);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetRacecarIndex(const RacecarIndex& racecarIndex)
{
	mRacecarIndex = racecarIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetRacecarDriver(const DriverIndex driverIndex)
{
	const DriverIndex previousDriverIndex = mDriverIndex;
	if (true == IsValidDriver(previousDriverIndex))
	{
		SendEvent(Events::RacecarSeatEvent(Events::Racecar::DriverLeavesRacecar, mDriverIndex, mRacecarIndex));
	}

	mDriverIndex = driverIndex;

	if (true == IsValidDriver(mDriverIndex))
	{
		RaceSessionState::PlaceCarOnGrid(*this);

		if (false == IsValidDriver(previousDriverIndex))
		{
			mPhysicsModel->SetEnabled(true);
		}

		RaceSessionState::PlaceCarOnGrid(*this);

		SendEvent(Events::RacecarSeatEvent(Events::Racecar::DriverEntersRacecar, mDriverIndex, mRacecarIndex));
	}
	else if (true == IsValidDriver(previousDriverIndex))
	{
		mPhysicsModel->SetEnabled(false);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetBodyToWorld(void) const
{
	return mPhysicsModel->GetBodyToWorld();
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetWheelToWorld(const size_t wheelIndex) const
{
	return mPhysicsModel->GetWheelToWorld(wheelIndex);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetVehicleToWorld(void) const
{
	return mPhysicsModel->GetVehicleToWorld();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetVehicleToWorld(const iceMatrix4& vehicleToWorld)
{
	mPhysicsModel->SetVehicleToWorld(vehicleToWorld);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::GetAngularVelocity(void) const
{
	return mPhysicsModel->GetAngularVelocity();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetAngularVelocity(const iceVector3& angularVelocity)
{
	mPhysicsModel->SetAngularVelocity(angularVelocity);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::GetLinearVelocity(void) const
{
	return mPhysicsModel->GetLinearVelocity();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetLinearVelocity(const iceVector3& linearVelocity)
{
	mPhysicsModel->SetLinearVelocity(linearVelocity);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Scalar LudumDare56::GameState::RacecarState::GetEngineSpeed(void) const
{
	return mPhysicsModel->GetEngineSpeed();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::Gear LudumDare56::GameState::RacecarState::GetShifterPosition(void) const
{
	return mPhysicsModel->GetShifterPosition();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::Simulate(void)
{
	mController->UpdateControls();

	mPhysicsModel->Simulate(*mController);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::RenderDebug(void) const
{
#if !defined(ludumdare56_headless_build)
	if (true == IsRacecarInUse())
	{
		mPhysicsModel->DebugRender();
	}
#endif /* ludumdare56_headless_build */
}

//--------------------------------------------------------------------------------------------------------------------//

std::vector<tbCore::uint8> LudumDare56::GameState::RacecarState::GetAvailableCars(bool /*isSubbed*/, bool /*isTier3*/)
{
	std::vector<tbCore::uint8> cars;
	cars.push_back(0);
	cars.push_back(1);
	return cars;
}

//--------------------------------------------------------------------------------------------------------------------//

PhysicsModel GetRacecarPhysicsModel(tbCore::uint8 /*carID*/)
{
	return PhysicsModel::ExtremeDrifting;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString LudumDare56::GameState::RacecarState::GetCarFilepath(tbCore::uint8 carID)
{
	const tbCore::tbString pathToRacecars = "data/meshes/racecars/";
	std::vector<String> availableRacecars = {
		pathToRacecars + "indicator.msh",
		pathToRacecars + "formula_blue.msh",
		pathToRacecars + "formula_red.msh",
	};

	if (carID >= availableRacecars.size())
	{	//Return generic racecar.
		return availableRacecars[0];
	}

	return availableRacecars[carID];
}

//--------------------------------------------------------------------------------------------------------------------//
