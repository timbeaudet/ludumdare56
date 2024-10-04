///
/// @file
/// @details Create a racecar controller for players to use keyboard/gamepad at their will.
/// @history This file originally started in Trailing Brakes Racing Simulator.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_PlayerRacecarController_hpp
#define LudumDare56_PlayerRacecarController_hpp

#include "../game_state/racecar_controller_interface.hpp"
#include "../core/input/input_signal_converters.hpp"

//--------------------------------------------------------------------------------------------------------------------//

namespace LudumDare56
{
	namespace GameClient
	{

		class PlayerRacecarController : public GameState::RacecarControllerInterface
		{
		public:
			PlayerRacecarController(void);
			virtual ~PlayerRacecarController(void);

		protected:
			virtual void OnUpdateControls(void) override;

		private:
			tbGame::InputAction mSteerLeftAction;
			tbGame::InputAction mSteerRightAction;
			tbGame::InputAction mThrottleAction;
			tbGame::InputAction mBrakeAction;

			tbGame::InputAction mShiftUpAction;
			tbGame::InputAction mShiftDownAction;
			tbGame::InputAction mHandbrakeAction;

			TyreBytes::Core::Input::SteeringSignalConverter mSteerLeftConverter;
			TyreBytes::Core::Input::SteeringSignalConverter mSteerRightConverter;
			TyreBytes::Core::Input::SignalConverter mThrottleConverter;
			TyreBytes::Core::Input::SignalConverter mBrakeConverter;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

//--------------------------------------------------------------------------------------------------------------------//

#endif /* LudumDare56_PlayerRacecarController_hpp */
