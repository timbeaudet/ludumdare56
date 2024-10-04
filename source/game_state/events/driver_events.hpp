///
/// @file
/// @details A place for all events related to the DriverState for LudumDare56.
/// @history This file started in 2022 with Trailing Brakes Racing Simulator.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_DriverEvents_hpp
#define LudumDare56_DriverEvents_hpp

#include "../../game_state/events/game_state_events.hpp"
#include "../../game_state/driver_state.hpp"

namespace LudumDare56
{
	namespace GameState
	{
		namespace Events
		{

			enum Driver
			{
				DriverEntersCompetition = EventCategories::StartDriverEvent, //NOT YET SENT...
				DriverLeavesCompetition,
				///If any event is over StartEvent + 1000 we need to modify the EventCategories and this comment.
				LastDriverEvent
			};

			class DriverEvent : public TyreBytes::Core::Event
			{
			public:
				explicit DriverEvent(const Events::Driver driverEvent, const DriverIndex driverIndex) :
					TyreBytes::Core::Event(driverEvent),
					mDriverIndex(driverIndex)
				{
				}

				virtual ~DriverEvent(void)
				{
				}

				const DriverIndex mDriverIndex;
			};

		};	//namespace Events
	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_DriverEvents_hpp */
