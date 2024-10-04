///
/// @file
/// @details A place for all events related to the RacetrackState.
/// @history This file started in 2022 with Trailing Brakes Racing Simulator.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RacetrackEvents_hpp
#define LudumDare56_RacetrackEvents_hpp

#include "../../game_state/events/game_state_events.hpp"
#include "../../game_state/racetrack_state.hpp"

#include <track_bundler/track_bundler.hpp>

namespace LudumDare56
{
	namespace GameState
	{
		namespace Events
		{

			enum Racetrack
			{
				// @note 2023-11-04: The NewRacetrack event may come after AddObject, or similar events due to the way
				//   the racetrack loading process works. AddObject event is sent during TrackBundler::LoadTrackBundle
				//   as it processes each new object, but the NewRacetrack event is sent after that loads.
				NewRacetrack = EventCategories::StartRacetrackEvent,
				ClearObjects,
				AddObject,
				RemoveObject,
				AddAutocrossObject,
				LoadSky,
				///If any event is over StartEvent + 1000 we need to modify the EventCategories and this comment.
				LastRacetrackEvent
			};

			class CreateRacetrackEvent : public TyreBytes::Core::Event
			{
			public:
				CreateRacetrackEvent(const TrackBundler::Legacy::TrackBundle& trackBundle,
					const TrackBundler::Legacy::TrackSegmentDefinitionContainer& segmentDefinitions,
					const TrackBundler::Legacy::TrackObjectDefinitionContainer& objectDefinitions,
					const TrackBundler::Legacy::TrackSplineDefinitionContainer& splineDefinitions) :
					TyreBytes::Core::Event(Racetrack::NewRacetrack),
					mTrackBundle(trackBundle),
					mSegmentDefinitions(segmentDefinitions),
					mObjectDefinitions(objectDefinitions),
					mSplineDefinitions(splineDefinitions)
				{
				}

				virtual ~CreateRacetrackEvent(void)
				{
				}

				const TrackBundler::Legacy::TrackBundle& mTrackBundle;
				const TrackBundler::Legacy::TrackSegmentDefinitionContainer& mSegmentDefinitions;
				const TrackBundler::Legacy::TrackObjectDefinitionContainer& mObjectDefinitions;
				const TrackBundler::Legacy::TrackSplineDefinitionContainer& mSplineDefinitions;
			};

			class RacetrackObjectEvent : public TyreBytes::Core::Event
			{
			public:
				explicit RacetrackObjectEvent(const Events::Racetrack racetrackEvent, const RacetrackState::ObjectHandle objectHandle) :
					TyreBytes::Core::Event(racetrackEvent),
					mObjectHandle(objectHandle)
				{
				}

				virtual ~RacetrackObjectEvent(void)
				{
				}

				inline RacetrackState::ObjectHandle GetObjectHandle(void) const { return mObjectHandle; }

			private:
				const RacetrackState::ObjectHandle mObjectHandle;
			};

			//class RacetrackSkyEvent : public TyreBytes::Core::Event
			//{
			//public:
			//	explicit RacetrackSkyEvent(const Racetrack event, const tbCore::tbString& skyFile) :
			//		TyreBytes::Core::Event(event),
			//		mSkyFile(skyFile)
			//	{
			//	}

			//	virtual ~RacetrackSkyEvent(void)
			//	{
			//	}

			//	inline const tbCore::tbString& GetSkyFile(void) const { return mSkyFile; }

			//private:
			//	const tbCore::tbString mSkyFile;
			//};

		};	//namespace Events
	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_RacetrackEvents_hpp */
