///
/// @file
/// @details A place for all events related to the Timing and Scoring / Results for LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_TimingEvents_hpp
#define LudumDare56_TimingEvents_hpp

#include "../../game_state/events/game_state_events.hpp"
#include "../../game_state/driver_state.hpp"

namespace LudumDare56
{
	namespace GameState
	{
		namespace Events
		{

			enum Timing
			{
				ResetTimingResults = EventCategories::StartTimingEvent,
				CompletedLapResult,

				///If any event is over StartEvent + 1000 we need to modify the EventCategories and this comment.
				LastTimingEvent
			};

			class TimingEvent : public TyreBytes::Core::Event
			{
			public:
				explicit TimingEvent(const Events::Timing timingEvent, const tbCore::tbString& driverLicense,
					const tbCore::tbString& driverName, tbCore::uint32 lapTime, tbCore::uint8 lapNumber) :

					TyreBytes::Core::Event(timingEvent),
					mDriverLicense(driverLicense),
					mDriverName(driverName),
					mLapTime(lapTime),
					mLapNumber(lapNumber)
				{
				}

				virtual ~TimingEvent(void)
				{
				}

				const tbCore::tbString mDriverLicense;
				const tbCore::tbString mDriverName;
				const tbCore::uint32 mLapTime;
				const tbCore::uint8 mLapNumber;
			};

		};	//namespace Events
	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_TimingEvents_hpp */
