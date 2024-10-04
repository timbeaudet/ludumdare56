///
/// @file
/// @details Grab the most recent controller information from a networked racecar in the LudumDare56 project.
/// @history This file originally started in Trailing Brakes Racing Simulator.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_NetworkedRacecarController_hpp
#define LudumDare56_NetworkedRacecarController_hpp

#include "../game_state/racecar_controller_interface.hpp"
#include "../network/network_packets.hpp"

//--------------------------------------------------------------------------------------------------------------------//

namespace LudumDare56
{
	namespace Network
	{

		class NetworkedRacecarController : public GameState::RacecarControllerInterface
		{
		public:
			NetworkedRacecarController(const GameState::RacecarIndex racecarIndex);
			virtual ~NetworkedRacecarController(void);

			void SetControllerInformation(const ControllerInfo& controllerInfo);

			tbCore::uint32 GetLastUpdateTimer(void) const { return mLastUpdateTimer; }

		protected:
			virtual void OnUpdateControls(void) override;

		private:
			Network::ControllerInfo mControllerInfo;
			const GameState::RacecarIndex mRacecarIndex;
			tbCore::uint32 mLastUpdateTimer;
		};

	};	//namespace Network
};	//namespace LudumDare56

//--------------------------------------------------------------------------------------------------------------------//

#endif /* LudumDare56_NetworkedRacecarControllerStap */
