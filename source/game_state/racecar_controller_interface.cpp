///
/// @file
/// @details An entity within the TopDownRacer project.
/// @history Originally started 2017 with Playground TopDownRacer networking.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../game_state/racecar_controller_interface.hpp"

const tbCore::uint16 LudumDare56::GameState::RacecarControllerInterface::kCenterSteeringValue(0x7FFF);

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarControllerInterface::RacecarControllerInterface(void) :
	mSteeringValue(kCenterSteeringValue),
	mThrottleValue(0),
	mBrakeValue(0),
	mShifterPosition(Gear::Neutral),
	mIsActionDown(),
	mWasActionDown()
{
	ResetControls();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarControllerInterface::~RacecarControllerInterface(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarControllerInterface::ResetControls(void)
{
	mSteeringValue = kCenterSteeringValue;
	mThrottleValue = 0;
	mBrakeValue = 0;
	mShifterPosition = Gear::Neutral;

	for (size_t actionIndex = 0; actionIndex < static_cast<size_t>(DriverAction::NumberOfActions); ++actionIndex)
	{
		mIsActionDown[actionIndex] = false;
		mWasActionDown[actionIndex] = false;
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarControllerInterface::UpdateControls(void)
{
	OnUpdateControls();
}


//--------------------------------------------------------------------------------------------------------------------//

float LudumDare56::GameState::RacecarControllerInterface::GetSteeringPercentage(void) const
{
	const float zeroToOne = static_cast<float>(mSteeringValue) / static_cast<float>(std::numeric_limits<tbCore::uint16>::max());
	const float value = (zeroToOne * 2.0f) - 1.0f; //-1 to 1
	if (fabs(value) < 0.001)
	{
		return 0.0f;
	}

	return value;
}

//--------------------------------------------------------------------------------------------------------------------//

float LudumDare56::GameState::RacecarControllerInterface::GetThrottlePercentage(void) const
{
	return static_cast<float>(mThrottleValue) / static_cast<float>(std::numeric_limits<tbCore::uint16>::max());
}

//--------------------------------------------------------------------------------------------------------------------//

float LudumDare56::GameState::RacecarControllerInterface::GetBrakePercentage(void) const
{
	return static_cast<float>(mBrakeValue) / static_cast<float>(std::numeric_limits<tbCore::uint16>::max());
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::NullRacecarController::NullRacecarController(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::NullRacecarController::~NullRacecarController(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::NullRacecarController::OnUpdateControls(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::BrakeOnlyRacecarController::BrakeOnlyRacecarController(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::BrakeOnlyRacecarController::~BrakeOnlyRacecarController(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::BrakeOnlyRacecarController::OnUpdateControls(void)
{
	SetSteeringPercentage(0.0);
	SetThrottlePercentage(0.0f);
	SetBrakePercentage(1.0f);
}

//--------------------------------------------------------------------------------------------------------------------//
