///
/// @file
/// @details A place for all events related to the DriverState for LudumDare56.
/// @history This file started in 2022 with Trailing Brakes Racing Simulator.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RaceSessionEvents_hpp
#define LudumDare56_RaceSessionEvents_hpp

#include "../../game_state/events/game_state_events.hpp"
#include "../../game_state/race_session_state.hpp"

namespace LudumDare56
{
	namespace GameState
	{
		namespace Events
		{

			enum RaceSession
			{
				RaceSessionPhaseChanged = EventCategories::StartRaceSessionEvent,
				StartGridChanged,
				///If any event is over StartEvent + 1000 we need to modify the EventCategories and this comment.
				LastRaceSessionEvent
			};

			class RaceSessionPhaseChangeEvent : public TyreBytes::Core::Event
			{
			public:
				explicit RaceSessionPhaseChangeEvent(const RaceSessionState::SessionPhase& sessionPhase, const tbCore::uint32 phaseTimer) :
					TyreBytes::Core::Event(RaceSession::RaceSessionPhaseChanged),
					mSessionPhase(sessionPhase),
					mPhaseTimer(phaseTimer)
				{
				}

				virtual ~RaceSessionPhaseChangeEvent(void)
				{
				}

				const RaceSessionState::SessionPhase mSessionPhase;
				const tbCore::uint32 mPhaseTimer;
			};

		};	//namespace Events
	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_RaceSessionEvents_hpp */
