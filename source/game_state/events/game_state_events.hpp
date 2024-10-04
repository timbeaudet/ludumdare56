///
/// @file
/// @details Separating the events into different categories to identify the start points and such.
/// @history This file started in 2022 with Trailing Brakes Racing Simulator.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_GameStateEvents_hpp
#define LudumDare56_GameStateEvents_hpp

#include "../../core/event_system.hpp"

#include <turtle_brains/core/tb_types.hpp>

namespace LudumDare56
{
	namespace GameState
	{
		namespace Events
		{

			//Canceled: 2022-08-25: Due to potential performance concerns I don't think the ease of use is worthwhile
			//  as of today; Registering to multiple broadcasters had not been widely used yet.
			//
			//Note: 2022-07-27: Consider rebroadcasting events through global/all GameState events.
			//  With this cleanup it would be good to also make a single access point in case something want to listen to all
			//  sorts of SimulatorEvents while not registering with each and every broadcaster. There is an image in the
			//  scratchpad "sending_events"
			//
			//  Since there are no places in code that listen to two or more broadcasters I think this is premature
			//  cleanup as of 2022-08-23.  On further inspection/use, there are some listeners that are added to all
			//  the racecars or drivers which could instead just listen to all; but that does have a performance hit
			//  since all drivers, racecars etc etc would send all events to the listener...
			//
			//  This will also require a place to initialize the simulator to ensure it is listening to all the components
			//  correctly. The SimulatorBroadcaster needs to listen to EVERYTHING in the simulator that broadcasts events in-order to
			//    rebroadcast from the single listening point.
			//
			//  Note: If this gets changed for AddEventListener() then be sure to deal with RemoveEventListeners() too!
			//
			//Note: 2023-09-21: When adding a new category, there is an Events::SafetyCheck that will ensure all events
			//  are within proper ranges. Create a LastCategoryEvent entry as the last entry in the category enum, and
			//  the first entry should be a start point here. Add a proper check in the SafetyCheck for new category.
			//
			//  Also, lets not save these in anyway that depends on these values/ranges! In the event that a category
			//  needs more events we could just change numbers...

			enum EventCategories : tbCore::uint32
			{
				StartRaceSessionEvent = 0,
				StartRacetrackEvent = 1000,
				StartDriverEvent = 2000,
				StartRacecarEvent = 3000,
				StartTimingEvent = 4000,
			};

		};	//namespace Events
	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_GameStateEvents_hpp */
