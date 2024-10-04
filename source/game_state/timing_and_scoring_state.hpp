///
/// @file
/// @details A Timing and Scoring system for LudumDare56 to know what position each racecar is in, as well as thier
///   laps and such.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_TimingState_hpp
#define LudumDare56_TimingState_hpp

#include "../game_state/race_session_state.hpp"
#include "../game_state/racetrack_state.hpp"
#include "../game_state/events/timing_events.hpp"

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_typed_integer.hpp>

#include <ice/physics/ice_physical_types.hpp>

namespace LudumDare56
{
	namespace GameState
	{
		enum class LapCounterType : tbCore::uint8 { };
		typedef tbCore::TypedInteger<LapCounterType> LapCounter;
		constexpr LapCounter InvalidLapCount(void) { return LapCounter::Integer(~0); }

		namespace TimingState
		{
			enum class CheckpointIndexType : tbCore::uint8 { };
			typedef tbCore::TypedInteger<CheckpointIndexType> CheckpointIndex;
			constexpr CheckpointIndex InvalidCheckpoint(void) { return CheckpointIndex::Integer(~0); }

			typedef RacetrackState::TrackNodeIndex TrackNodeIndex;
			constexpr TrackNodeIndex InvalidTrackNode(void) { return RacetrackState::InvalidTrackNode(); }
			//inline bool IsValidTrackNode(const TrackNodeIndex trackNodeIndex) { return RacetrackState::IsValidTrackNode(trackNodeIndex); }

			///
			/// @details Add an EventListenter for TimingEvents.
			///
			void AddEventListener(TyreBytes::Core::EventListener& eventListener);

			///
			/// @details Remove an EventListenter to stop listening to events from the TimingState changes.
			///
			void RemoveEventListener(TyreBytes::Core::EventListener& eventListener);

			///
			/// @details Clears out any old state from a previous racetrack. There will be no-checkpoints or valid laps
			///   once this is called. You will need to readd the checkpoint data and reinitialize any transponders.
			///
			void Invalidate(void);

			///
			/// @details Clears all the time entries that have been tracked, any lap status, etc for each driver/racecar
			///   in the event/competition.
			///
			void ResetCompetition(void);

			///
			/// @details Adds a checkpoint for the racecars to pass through.
			///
			void AddCheckpoint(const icePhysics::Matrix4& checkpointToWorld, const CheckpointIndex checkpointIndex, bool withCutPenalty);

			///
			///
			///
			void Simulate(void);

			///
			/// @details This is so the GameClient can add the results the GameServer sends.
			///
			void AddCompletedLapResult(const Events::TimingEvent& lapResultEvent);

			int GetRaceStandingsFor(const RacecarIndex racecarIndex);

			LapCounter GetCurrentLapFor(const RacecarIndex racecarIndex);
			bool IsRacecarFinished(const RacecarIndex racecarIndex);

			void RenderDebug(void);

		};	//namespace TimingState
	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_TimingState_hpp */
