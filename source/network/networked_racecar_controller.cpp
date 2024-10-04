///
/// @file
/// @details Grab the most recent controller information from a networked racecar in the LudumDare56 project.
/// @history This file originally started in Trailing Brakes Racing Simulator.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../network/networked_racecar_controller.hpp"
#include "../logging.hpp"

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::NetworkedRacecarController::NetworkedRacecarController(const GameState::RacecarIndex racecarIndex) :
	RacecarControllerInterface(),
	mControllerInfo(),
	mRacecarIndex(racecarIndex),
	mLastUpdateTimer(tbCore::uint32(~0))
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::NetworkedRacecarController::~NetworkedRacecarController(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::NetworkedRacecarController::OnUpdateControls(void)
{
	mLastUpdateTimer += 10;

	SetSteeringValue(mControllerInfo.steering);
	SetThrottleValue(mControllerInfo.throttle);
	SetBrakeValue(mControllerInfo.braking);

	//SetHandbrakeDown(1 == (mControllerInfo.buttons & 1));
	//SetLaunchDown(2 == (mControllerInfo.buttons & 2));
	//SetResetDown(4 == (mControllerInfo.buttons & 4));
	//SetBoostDown(8 == (mControllerInfo.buttons & 8));
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::NetworkedRacecarController::SetControllerInformation(const Network::ControllerInfo& controllerInfo)
{
	mLastUpdateTimer = 0;
	mControllerInfo = controllerInfo;
}

//--------------------------------------------------------------------------------------------------------------------//
