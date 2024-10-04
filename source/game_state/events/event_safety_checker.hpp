///
/// @file
/// @details A place for all events related to the RacetrackState.
/// @history This file started in 2022 with Trailing Brakes Racing Simulator.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_EventSafetyChecker_hpp
#define LudumDare56_EventSafetyChecker_hpp

#include "../../game_state/events/game_state_events.hpp"
#include "../../game_state/events/driver_events.hpp"
#include "../../game_state/events/racecar_events.hpp"
#include "../../game_state/events/racetrack_events.hpp"
#include "../../game_state/events/race_session_events.hpp"
#include "../../game_state/events/timing_events.hpp"

#include <turtle_brains/core/tb_error.hpp>

namespace LudumDare56
{
	namespace GameState
	{
		namespace Events
		{

			inline void SafetyCheck(void)
			{
//#if defined(__GNUC__) || defined(__clang__)
#if defined(tb_linux)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wenum-compare"
#endif
				tb_static_error_if(static_cast<int>(EventCategories::StartRacetrackEvent) < static_cast<int>(Events::RaceSession::LastRaceSessionEvent), "Event ID collision, too many RaceSessionEvents.");
				tb_static_error_if(static_cast<int>(EventCategories::StartDriverEvent) < static_cast<int>(Events::Racetrack::LastRacetrackEvent), "Event ID collision, too many RacetrackEvents.");
				tb_static_error_if(static_cast<int>(EventCategories::StartRacecarEvent) < static_cast<int>(Events::Driver::LastDriverEvent), "Event ID collision, too many DriverEvents.");
				tb_static_error_if(static_cast<int>(EventCategories::StartTimingEvent) < static_cast<int>(Events::Racecar::LastRacecarEvent), "Event ID collision, too many RacecarEvents.");
				//tb_static_error_if(EventCategories::StartNextUnknownEvent < Events::Timing::LastTimingEvent, "Event ID collision, too many TimingEvents.");

//#if defined(__GNUC__) || defined(__clang__)
#if defined(tb_linux)
  #pragma GCC diagnostic push
#endif
			}

		};	//namespace Events
	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_EventSafetyChecker_hpp */
