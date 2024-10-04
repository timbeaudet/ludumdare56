///
/// @file
/// @details Create a racecar controller for players to use keyboard/gamepad at their will.
/// @history This file originally started in Trailing Brakes Racing Simulator.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../game_client/player_racecar_controller.hpp"

#include "../core/input/key_binder.hpp"
#include "../user_settings.hpp"

//--------------------------------------------------------------------------------------------------------------------//

using namespace TyreBytes::Core::Input;

LudumDare56::GameClient::PlayerRacecarController::PlayerRacecarController(void) :
	mSteerLeftAction(KeyBinder::ActionFromName(
		TheUserSettings().GetString(Settings::ControlSteering()),
		TheUserSettings().GetBoolean(Settings::ControlSteeringInverted()))),
	mSteerRightAction(KeyBinder::ActionFromName(
		TheUserSettings().GetString(Settings::ControlSteering()),
		TheUserSettings().GetBoolean(Settings::ControlSteeringInverted()))),
	mThrottleAction(KeyBinder::ActionFromName(
		TheUserSettings().GetString(Settings::ControlThrottle()),
		TheUserSettings().GetBoolean(Settings::ControlThrottleInverted()))),
	mBrakeAction(KeyBinder::ActionFromName(
		TheUserSettings().GetString(Settings::ControlBrake()),
		TheUserSettings().GetBoolean(Settings::ControlBrakeInverted()))),
	mShiftUpAction(KeyBinder::ActionFromName(TheUserSettings().GetString(Settings::ControlShiftUp()))),
	mShiftDownAction(KeyBinder::ActionFromName(TheUserSettings().GetString(Settings::ControlShiftDown()))),
	mHandbrakeAction(KeyBinder::ActionFromName(TheUserSettings().GetString(Settings::ControlHandbrake()))),
	mSteerLeftConverter(0.49f, 0.29f),
	mSteerRightConverter(0.51f, 0.71f),
	mThrottleConverter(),
	mBrakeConverter()
{
	{	//Hard-coded mess to allow keyboard steering with A/D or Left/Right arrows during early-access testing. This
		//  is not something we should spend significant time supporting or dealing with huge "hard to control" issues
		//  unless the decision to fully support keyboard/digital controls changes to offer that support. 2022-08-25
		const tbCore::tbString steeringName = TheUserSettings().GetString(Settings::ControlSteering());
		if (true == tbCore::StringContains(steeringName, "Left") || true == tbCore::StringContains(steeringName, "Right"))
		{
			mSteerLeftAction = tbGame::InputAction(tbApplication::tbKeyLeft);
			mSteerRightAction = tbGame::InputAction(tbApplication::tbKeyRight);
		}
		else if (steeringName == "A" || steeringName == "D")
		{
			mSteerLeftAction = tbGame::InputAction(tbApplication::tbKeyA);
			mSteerRightAction = tbGame::InputAction(tbApplication::tbKeyD);
		}
	}

	mSteerLeftAction.SetInputSignalConverter(&mSteerLeftConverter);
	mSteerRightAction.SetInputSignalConverter(&mSteerRightConverter);
	mThrottleAction.SetInputSignalConverter(&mThrottleConverter);
	mBrakeAction.SetInputSignalConverter(&mBrakeConverter);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::PlayerRacecarController::~PlayerRacecarController(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::PlayerRacecarController::OnUpdateControls(void)
{
	// @note 2023-10-25: LudumDare56: Trailing Brakes used to check if the player had a racecar. I would think that
	//   if there is no racecar for the player, the NullRacecarController should be used/set cleaning up any old PlayerController.
	const float steerLeft = mSteerLeftAction.GetAnalogValue();
	const float steerRight = mSteerRightAction.GetAnalogValue();
	//const float steering = (steerLeft + steerRight) - 1.0f; //analog case
	const float steering = ((1.0f - steerLeft) + steerRight) - 1.0f; //digital case
	SetSteeringPercentage(steering);
	SetThrottlePercentage(mThrottleAction.GetAnalogValue());
	SetBrakePercentage(mBrakeAction.GetAnalogValue());

	SetActionDown(GameState::DriverAction::ShiftUp, mShiftUpAction.IsDown());
	SetActionDown(GameState::DriverAction::ShiftDown, mShiftDownAction.IsDown());
	SetActionDown(GameState::DriverAction::Handbrake, mHandbrakeAction.IsDown());
}

//--------------------------------------------------------------------------------------------------------------------//
