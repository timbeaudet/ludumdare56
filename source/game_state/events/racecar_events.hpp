///
/// @file
/// @details A place for all events related to the RacecarState.
/// @history This file started in 2022 with Trailing Brakes Racing Simulator.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RacecarEvents_hpp
#define LudumDare56_RacecarEvents_hpp

#include "../../game_state/race_session_state.hpp"
#include "../../game_state/events/game_state_events.hpp"

namespace LudumDare56
{
	namespace GameState
	{
		namespace Events
		{

			enum Racecar
			{
				DriverEntersRacecar = EventCategories::StartRacecarEvent,
				DriverLeavesRacecar,
				///If any event is over StartEvent + 1000 we need to modify the EventCategories and this comment.
				LastRacecarEvent
			};

			class RacecarSeatEvent : public TyreBytes::Core::Event
			{
			public:
				explicit RacecarSeatEvent(const Events::Racecar racecarEvent, const DriverIndex driverIndex, const RacecarIndex racecarIndex) :
					TyreBytes::Core::Event(racecarEvent),
					mDriverIndex(driverIndex),
					mRacecarIndex(racecarIndex)
				{
				}

				const DriverIndex mDriverIndex;
				const RacecarIndex mRacecarIndex;
			};

		};	//namespace Events
	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_RacecarEvents_hpp */
