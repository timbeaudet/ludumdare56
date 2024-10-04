///
/// @file
/// @details A Timing and Scoring system for LudumDare56 to know what position each racecar is in, as well as thier
///   laps and such.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../game_state/timing_and_scoring_state.hpp"
#include "../game_state/implementation/racetrack_implementation.hpp"
#include "../core/utilities.hpp"
#include "../logging.hpp"

#include <array>
#include <algorithm>
#include <vector>

typedef icePhysics::Scalar Scalar;

namespace LudumDare56
{
	namespace GameState
	{
		namespace Implementation
		{

			struct BoxTrigger
			{
				icePhysics::Matrix3 mOrientation;
				icePhysics::Vector3 mPosition;
				icePhysics::Vector3 mHalfDimensions;
			};

			struct Checkpoint
			{
				BoxTrigger mBoxTrigger;
				TimingState::CheckpointIndex mCheckpointIndex = TimingState::InvalidCheckpoint();
				bool mCutPenalty = false;
			};

			struct Transponder
			{
				icePhysics::Vector3 mPosition = icePhysics::Vector3::Zero();
				icePhysics::Vector2 mPositionOnTrack = icePhysics::Vector2::Zero();
				TimingState::CheckpointIndex mCheckpointIndex = TimingState::InvalidCheckpoint();
				TimingState::TrackNodeIndex mTrackNodeIndex = TimingState::InvalidTrackNode();
				TimingState::TrackNodeIndex mLastValidNode = TimingState::InvalidTrackNode();
				tbGame::GameTimer mElapsedLapTime = 0;
				int mRaceStanding = 0; //0 is out-of-race, 1, 2, 3 etc.
				LapCounter mCurrentLap = 0;
				bool mIsActive = false;

				static Transponder Invalid(void) { return Transponder(); }
			};

			struct LapResult
			{	//It is important to remember the driver name, etc that has left the competition, so we can't just
				//  hold the driver index and get the information whenever, we must store it.
				tbCore::tbString mDriverLicense;
				tbCore::tbString mDriverName;
				tbGame::GameTimer mElapsedTime;
				LapCounter mLapNumber;
			};

			std::vector<LapResult> theLapResults;
			//std::unordered_map<tbCore::tbString, LapResult> result;

			/// @param triggerToWorld will contain scaling which will describe the dimensions of the trigger.
			BoxTrigger CreateBoxTrigger(const icePhysics::Matrix4& triggerToWorld);

			bool IsCrossingTrigger(const icePhysics::Vector3& oldTransponderPosition,
				const icePhysics::Vector3& newTransponderPosition, const BoxTrigger& trigger, Scalar& outTeeFraction);
			bool IsRacecarInTrigger(const RacecarState& racecar, const BoxTrigger& trigger);
			bool IsPointInTrigger(const icePhysics::Vector3& point, const BoxTrigger& trigger);

			bool IsTransponderInTrackNode(const icePhysics::Vector3& transponderPosition, const TimingState::TrackNodeIndex trackNodeIndex);
			TimingState::TrackNodeIndex FindTransponder(const icePhysics::Vector3& transponderPosition);
			TimingState::TrackNodeIndex FindTransponderNearTrackNode(const icePhysics::Vector3& transponderPosition,
				const TimingState::TrackNodeIndex trackNodeIndex);

			icePhysics::Vector2 ComputeNodeSpace(const icePhysics::Vector3& transponderPosition,
				const TimingState::TrackNodeIndex trackNodeIndex, icePhysics::Scalar& boundarySpace);

			std::vector<Checkpoint> theCheckpoints;
			TimingState::CheckpointIndex theHighestCheckpointIndex = TimingState::InvalidCheckpoint();
			std::array<Transponder, kNumberOfRacecars> theTransponders;

			LapCounter theTotalLapsInRace = 3;

			TyreBytes::Core::EventBroadcaster theTimingBroadcaster;

		};	//namespace Implementation
	};	//namespace GameState
};	//LudumDare56

using namespace LudumDare56::GameState::Implementation;

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::TimingState::AddEventListener(TyreBytes::Core::EventListener& eventListener)
{
	theTimingBroadcaster.AddEventListener(eventListener);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::TimingState::RemoveEventListener(TyreBytes::Core::EventListener& eventListener)
{
	theTimingBroadcaster.RemoveEventListener(eventListener);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::TimingState::Invalidate(void)
{
	theCheckpoints.clear();
	theHighestCheckpointIndex = InvalidCheckpoint();

	ResetCompetition();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::TimingState::ResetCompetition(void)
{
	theTimingBroadcaster.SendEvent(GameState::Events::Timing::ResetTimingResults);
	theLapResults.clear();

	for (Transponder& transponder : theTransponders)
	{
		transponder = Transponder::Invalid();
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::TimingState::AddCheckpoint(const icePhysics::Matrix4& checkpointToWorld,
	const CheckpointIndex checkpointIndex, bool withCutPenalty)
{
	tb_error_if(InvalidCheckpoint() == checkpointIndex, "Error: Expected a valid checkpoint index.");

	Checkpoint checkpoint;
	checkpoint.mBoxTrigger = CreateBoxTrigger(checkpointToWorld);
	checkpoint.mCheckpointIndex = checkpointIndex;
	checkpoint.mCutPenalty = withCutPenalty;
	theCheckpoints.push_back(checkpoint);

	if (InvalidCheckpoint() == theHighestCheckpointIndex || checkpointIndex > theHighestCheckpointIndex)
	{
		theHighestCheckpointIndex = checkpointIndex;
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::TimingState::Simulate(void)
{
	for (const RacecarState& racecar : RacecarState::AllRacecars())
	{
		Transponder& transponder = theTransponders[racecar.GetRacecarIndex()];

		if (false == racecar.IsRacecarInUse())
		{
			transponder = Transponder::Invalid();
			continue;
		}

		const icePhysics::Vector3 racecarPosition = racecar.GetVehicleToWorld().GetPosition();
		if (false == transponder.mIsActive)
		{
			transponder.mIsActive = true;
			transponder.mPosition = racecarPosition;
			transponder.mElapsedLapTime = 0;
			transponder.mCurrentLap = 0;
		}

		transponder.mElapsedLapTime.IncrementStep();

		if (true)
		{
			for (const Checkpoint& checkpoint : theCheckpoints)
			{
				Scalar teeFraction = Scalar(0.0);
				if (true == IsCrossingTrigger(transponder.mPosition, racecarPosition, checkpoint.mBoxTrigger, teeFraction))
				{
					if (checkpoint.mCheckpointIndex == transponder.mCheckpointIndex + static_cast<CheckpointIndex>(1) ||
						(InvalidCheckpoint() == transponder.mCheckpointIndex && 1 == checkpoint.mCheckpointIndex))
					{
						transponder.mCheckpointIndex = checkpoint.mCheckpointIndex;

						tb_debug_log(LogState::Info() << DebugInfo(racecar) << " has crossed checkpoint: " << static_cast<int>(checkpoint.mCheckpointIndex));
					}

					if (0 == checkpoint.mCheckpointIndex && 0 == transponder.mCurrentLap)
					{	//Starting the first lap. This is duplicated for Transponder to Checkpoints as well as Transponder to TrackNodes.
						//  This is duplicated to prevent the Standings jumping about on the very first lap whether the checkpoint
						//  is slightly ahead or behind where the TrackNode circuit loops from finish to start.
						transponder.mCheckpointIndex = 0;
						transponder.mElapsedLapTime = 0;
						transponder.mCurrentLap = 1;
					}
					else if (0 == checkpoint.mCheckpointIndex && theHighestCheckpointIndex == transponder.mCheckpointIndex)
					{
						transponder.mElapsedLapTime += static_cast<tbCore::uint32>(teeFraction * 1000.0 + 0.5);

						tb_debug_log(LogState::Info() << DebugInfo(racecar) << " has finished lap " << +transponder.mCurrentLap <<
							" with a time of: " << tbCore::String::TimeToString(transponder.mElapsedLapTime.GetElapsedTime()));

						const DriverState& driver = DriverState::Get(racecar.GetDriverIndex());

						if (true == IsTrusted())
						{
							const Events::TimingEvent lapResultEvent(Events::Timing::CompletedLapResult, driver.GetLicense(),
								driver.GetName(), transponder.mElapsedLapTime.GetElapsedTime(), transponder.mCurrentLap);

							AddCompletedLapResult(lapResultEvent);
						}

						//Setup and start the next lap... (this is assuming lap-based racing)
						transponder.mCheckpointIndex = checkpoint.mCheckpointIndex;
						transponder.mElapsedLapTime = 0;
						++transponder.mCurrentLap;
					}
				}
			}
		}

		if (true == RacetrackState::IsValidTrackNode(transponder.mTrackNodeIndex))
		{
			if (false == IsTransponderInTrackNode(transponder.mPosition, transponder.mTrackNodeIndex))
			{	//Look for new tracknode, probably forward or backwards a little.
				transponder.mTrackNodeIndex = FindTransponderNearTrackNode(transponder.mPosition, transponder.mTrackNodeIndex);
				if (false == RacetrackState::IsValidTrackNode(transponder.mTrackNodeIndex))
				{
					transponder.mTrackNodeIndex = FindTransponder(transponder.mPosition);
				}
			}
		}
		else
		{
			transponder.mTrackNodeIndex = FindTransponder(transponder.mPosition);
		}

		if (true == RacetrackState::IsValidTrackNode(transponder.mTrackNodeIndex))
		{
			if (0 == transponder.mTrackNodeIndex && 0 == transponder.mCurrentLap)
			{	//Starting the first lap. This is duplicated for Transponder to Checkpoints as well as Transponder to TrackNodes.
				//  This is duplicated to prevent the Standings jumping about on the very first lap whether the checkpoint
				//  is slightly ahead or behind where the TrackNode circuit loops from finish to start.
				transponder.mCheckpointIndex = 0;
				transponder.mElapsedLapTime = 0;
				transponder.mCurrentLap = 1;
			}

			Scalar boundarySpace = Scalar(0.0);
			transponder.mPositionOnTrack = Implementation::ComputeNodeSpace(transponder.mPosition, transponder.mTrackNodeIndex, boundarySpace);
			transponder.mPositionOnTrack.y += static_cast<Scalar>(transponder.mTrackNodeIndex);
			transponder.mLastValidNode = transponder.mTrackNodeIndex;
		}

		transponder.mPosition = racecarPosition;
	}

	///
	/// Sort the current racecar standings based on what transponders are where, and update the transponder standings.
	///
	std::array<RacecarIndex, kNumberOfRacecars> racecarStandings;
	for (RacecarIndex racecarIndex = 0; racecarIndex < kNumberOfRacecars; ++racecarIndex)
	{
		racecarStandings[racecarIndex] = racecarIndex;
	}

	std::sort(racecarStandings.begin(), racecarStandings.end(), [](RacecarIndex racecarA, RacecarIndex racecarB) {
		const Transponder& transponderA = theTransponders[racecarA];
		const Transponder& transponderB = theTransponders[racecarB];

		if (true == RacecarState::Get(racecarA).IsRacecarInUse() && false == RacecarState::Get(racecarB).IsRacecarInUse()) { return true; }
		if (false == RacecarState::Get(racecarA).IsRacecarInUse() && true == RacecarState::Get(racecarB).IsRacecarInUse()) { return false; }
		if (transponderA.mCurrentLap > transponderB.mCurrentLap) { return true; }
		if (transponderA.mCurrentLap < transponderB.mCurrentLap) { return false; }

		if (true == RacetrackState::IsValidTrackNode(transponderA.mTrackNodeIndex) && true == RacetrackState::IsValidTrackNode(transponderB.mTrackNodeIndex))
		{
			return transponderA.mPositionOnTrack.y > transponderB.mPositionOnTrack.y;
		}
		else if (true == RacetrackState::IsValidTrackNode(transponderA.mTrackNodeIndex))
		{
			return transponderA.mPositionOnTrack.y > static_cast<Scalar>(transponderB.mLastValidNode);
		}
		else if (true == RacetrackState::IsValidTrackNode(transponderB.mTrackNodeIndex))
		{
			return static_cast<Scalar>(transponderA.mLastValidNode) > transponderB.mPositionOnTrack.y;
		}

		return transponderA.mLastValidNode > transponderB.mLastValidNode;
	});

	for (size_t standingIndex = 0; standingIndex < racecarStandings.size(); ++standingIndex)
	{
		const RacecarIndex racecarIndex = racecarStandings[standingIndex];
		const bool isInUse = RacecarState::Get(racecarIndex).IsRacecarInUse();
		theTransponders[racecarIndex].mRaceStanding = (true == isInUse) ? tbCore::RangedCast<int>(standingIndex) + 1 : 0;
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::TimingState::AddCompletedLapResult(const Events::TimingEvent& lapResultEvent)
{
	LapResult lapResult;
	lapResult.mDriverLicense = lapResultEvent.mDriverLicense;
	lapResult.mDriverName = lapResultEvent.mDriverName;
	lapResult.mElapsedTime = lapResultEvent.mLapTime;
	lapResult.mLapNumber = lapResultEvent.mLapNumber;
	theLapResults.push_back(lapResult);

	theTimingBroadcaster.SendEvent(lapResultEvent);
}

//--------------------------------------------------------------------------------------------------------------------//

int LudumDare56::GameState::TimingState::GetRaceStandingsFor(const RacecarIndex racecarIndex)
{
	return theTransponders[racecarIndex].mRaceStanding;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::LapCounter LudumDare56::GameState::TimingState::GetCurrentLapFor(const RacecarIndex racecarIndex)
{
	return (false == theTransponders[racecarIndex].mIsActive) ? InvalidLapCount() : theTransponders[racecarIndex].mCurrentLap;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameState::TimingState::IsRacecarFinished(const RacecarIndex racecarIndex)
{
	return (false == theTransponders[racecarIndex].mIsActive || theTransponders[racecarIndex].mCurrentLap > theTotalLapsInRace);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::TimingState::RenderDebug(void)
{
#if !defined(ludumdare56_headless_build)
	//iceGraphics::Visualization debugVisuals;

	//for (const Checkpoint& checkpoint : theCheckpoints)
	//{
	//}

	//debugVisuals.Render();
#endif /* ludumdare56_headless_build */
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::Implementation::BoxTrigger LudumDare56::GameState::Implementation::CreateBoxTrigger(
	const icePhysics::Matrix4& triggerToWorld)
{
	const icePhysics::Vector3 triggerScale(
		triggerToWorld.GetBasis(0).Magnitude(),
		triggerToWorld.GetBasis(1).Magnitude(),
		triggerToWorld.GetBasis(2).Magnitude());

	BoxTrigger trigger;
	trigger.mHalfDimensions = triggerScale / 2.0f;
	trigger.mOrientation.SetBasis(0, triggerToWorld.GetBasis(0) / triggerScale.x);
	trigger.mOrientation.SetBasis(1, triggerToWorld.GetBasis(1) / triggerScale.y);
	trigger.mOrientation.SetBasis(2, triggerToWorld.GetBasis(2) / triggerScale.z);
	trigger.mPosition = triggerToWorld.GetPosition();

	return trigger;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameState::Implementation::IsCrossingTrigger(
	const icePhysics::Vector3& oldTransponderPosition, const icePhysics::Vector3& newTransponderPosition,
	const BoxTrigger& trigger, Scalar& outTeeFraction)
{
	icePhysics::Vector3 collideAt;

	if (true == icePhysics::LineSegmentToPlaneCollision(oldTransponderPosition, newTransponderPosition,
			trigger.mPosition, -trigger.mOrientation.GetBasis(2), outTeeFraction, collideAt) &&
		true == icePhysics::LineSegmentToOOBBCollision(oldTransponderPosition, newTransponderPosition,
			trigger.mPosition, trigger.mHalfDimensions, trigger.mOrientation))
	{
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameState::Implementation::IsRacecarInTrigger(const RacecarState& racecar, const BoxTrigger& trigger)
{
	return IsPointInTrigger(racecar.GetVehicleToWorld().GetPosition(), trigger);
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameState::Implementation::IsPointInTrigger(const icePhysics::Vector3& point, const BoxTrigger& trigger)
{
	return icePhysics::Collision::SphereToOrientedBoxIntersect(point, icePhysics::Scalar(0.001),
		trigger.mPosition, trigger.mHalfDimensions, trigger.mOrientation, nullptr);
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameState::Implementation::IsTransponderInTrackNode(
	const icePhysics::Vector3& transponderPosition, const TimingState::TrackNodeIndex trackNodeIndex)
{
	// @note 2023-11-04: The normals for the trackNode planes are pointing outward, away from the center, so here we
	//   are negating all the normals because we are trying to test if the transponder is INSIDE. One could argue the
	//   normals should point inwards already, but it also makes some sense for the leading edge plane to point forward,
	//   left edge plane to point left and so on, which is how it works today.
	const TrackNode& trackNode = TheTrackNodes()[trackNodeIndex];
	if (true == icePhysics::PointHalfspaceTest(transponderPosition, trackNode.mLeadingPlane.Position(), trackNode.mLeadingPlane.Normal()) &&
		true == icePhysics::PointHalfspaceTest(transponderPosition, trackNode.mTrailingPlane.Position(), trackNode.mTrailingPlane.Normal()) &&
		true == icePhysics::PointHalfspaceTest(transponderPosition, trackNode.mLeftPlane.Position(), trackNode.mLeftPlane.Normal()) &&
		true == icePhysics::PointHalfspaceTest(transponderPosition, trackNode.mRightPlane.Position(), trackNode.mRightPlane.Normal()))
	{
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::TimingState::TrackNodeIndex LudumDare56::GameState::Implementation::FindTransponder(
	const icePhysics::Vector3& transponderPosition)
{
	const TimingState::TrackNodeIndex totalNodes = RacetrackState::GetNumberOfTrackNodes();
	for (TimingState::TrackNodeIndex searchNodeIndex = 0; searchNodeIndex < totalNodes; ++searchNodeIndex)
	{
		if (true == IsTransponderInTrackNode(transponderPosition, searchNodeIndex))
		{
			return searchNodeIndex;
		}
	}

	return TimingState::InvalidTrackNode();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::TimingState::TrackNodeIndex LudumDare56::GameState::Implementation::FindTransponderNearTrackNode(
	const icePhysics::Vector3& transponderPosition, const TimingState::TrackNodeIndex trackNodeIndex)
{
	const TimingState::TrackNodeIndex totalNodes = RacetrackState::GetNumberOfTrackNodes();

	const std::vector<TimingState::TrackNodeIndex> searchTrackNodeIndices{
		tbCore::RangedCast<TimingState::TrackNodeIndex::Integer>((trackNodeIndex + static_cast<TimingState::TrackNodeIndex::Integer>(1)) % totalNodes),
		tbCore::RangedCast<TimingState::TrackNodeIndex::Integer>((trackNodeIndex + static_cast<TimingState::TrackNodeIndex::Integer>(2)) % totalNodes),
		tbCore::RangedCast<TimingState::TrackNodeIndex::Integer>((trackNodeIndex + static_cast<TimingState::TrackNodeIndex::Integer>(3)) % totalNodes),
		tbCore::RangedCast<TimingState::TrackNodeIndex::Integer>((trackNodeIndex + static_cast<TimingState::TrackNodeIndex::Integer>(4)) % totalNodes),
		tbCore::RangedCast<TimingState::TrackNodeIndex::Integer>((trackNodeIndex + static_cast<TimingState::TrackNodeIndex::Integer>(5)) % totalNodes),
		tbCore::RangedCast<TimingState::TrackNodeIndex::Integer>((trackNodeIndex + totalNodes - static_cast<TimingState::TrackNodeIndex::Integer>(1)) % totalNodes),
		tbCore::RangedCast<TimingState::TrackNodeIndex::Integer>((trackNodeIndex + totalNodes - static_cast<TimingState::TrackNodeIndex::Integer>(2)) % totalNodes),
		tbCore::RangedCast<TimingState::TrackNodeIndex::Integer>((trackNodeIndex + totalNodes - static_cast<TimingState::TrackNodeIndex::Integer>(3)) % totalNodes),
	};

	for (TimingState::TrackNodeIndex searchNodeIndex : searchTrackNodeIndices)
	{
		if (true == IsTransponderInTrackNode(transponderPosition, searchNodeIndex))
		{
			return searchNodeIndex;
		}
	}

	return TimingState::InvalidTrackNode();
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 Flatten3(const icePhysics::Vector3& input) { return icePhysics::Vector3(input.x, Scalar(0.0), input.z); }

icePhysics::Vector2 LudumDare56::GameState::Implementation::ComputeNodeSpace(const icePhysics::Vector3& transponderPosition,
	const TimingState::TrackNodeIndex trackNodeIndex, icePhysics::Scalar& boundarySpace)
{
	const RacetrackState::TrackNodeEdge& trailing = RacetrackState::GetTrackNodeTrailingEdge(trackNodeIndex);
	const RacetrackState::TrackNodeEdge& leading = RacetrackState::GetTrackNodeLeadingEdge(trackNodeIndex);

	const icePhysics::Vector3 planeNormal = icePhysics::Vector3::Cross(Flatten3(leading[TrackEdge::kLeft]) - Flatten3(trailing[TrackEdge::kLeft]),
		Flatten3(trailing[TrackEdge::kRight]) - Flatten3(trailing[TrackEdge::kLeft])).GetNormalized();
	const icePhysics::Vector3 positionOnPlane = Flatten3(icePhysics::RayToPlaneIntersect(transponderPosition, -Up(),
		Flatten3(trailing[TrackEdge::kLeft]), planeNormal));

	const icePhysics::Vector3 farLeft = Flatten3(icePhysics::ClosestPointOnLine(positionOnPlane, Flatten3(leading[TrackEdge::kFarLeft]), Flatten3(trailing[TrackEdge::kFarLeft])));
	const icePhysics::Vector3 farRight = Flatten3(icePhysics::ClosestPointOnLine(positionOnPlane, Flatten3(leading[TrackEdge::kFarRight]), Flatten3(trailing[TrackEdge::kFarRight])));
	const icePhysics::Vector3 left = Flatten3(icePhysics::ClosestPointOnLine(positionOnPlane, Flatten3(leading[TrackEdge::kLeft]), Flatten3(trailing[TrackEdge::kLeft])));
	const icePhysics::Vector3 right = Flatten3(icePhysics::ClosestPointOnLine(positionOnPlane, Flatten3(leading[TrackEdge::kRight]), Flatten3(trailing[TrackEdge::kRight])));
	const icePhysics::Vector3 previous = Flatten3(icePhysics::ClosestPointOnLine(positionOnPlane, Flatten3(trailing[TrackEdge::kLeft]), Flatten3(trailing[TrackEdge::kRight])));
	const icePhysics::Vector3 next = Flatten3(icePhysics::ClosestPointOnLine(positionOnPlane, Flatten3(leading[TrackEdge::kLeft]), Flatten3(leading[TrackEdge::kRight])));

	icePhysics::Vector2 nodeSpace;
	nodeSpace.x = ((positionOnPlane - left) * (right - left)) / (right - left).MagnitudeSquared();
	nodeSpace.y = ((positionOnPlane - previous) * (next - previous)) / (next - previous).MagnitudeSquared();

	const icePhysics::Vector3 leadingEdgeNormal = icePhysics::Vector3::Cross(leading[TrackEdge::kLeft] - leading[TrackEdge::kRight], Up());
	const icePhysics::Vector3 trailingEdgeNormal = icePhysics::Vector3::Cross(trailing[TrackEdge::kLeft] - trailing[TrackEdge::kRight], Up());
	if ((positionOnPlane - Flatten3(leading[TrackEdge::kLeft])) * leadingEdgeNormal > 0.0f)
	{	//Transponder has passed the leading edge already, just used distance to leading edge.
		nodeSpace.y = 1.0f + (next - positionOnPlane).Magnitude();
	}
	else if ((positionOnPlane - Flatten3(trailing[TrackEdge::kLeft])) * trailingEdgeNormal < 0.0f)
	{	//Transponder has not yet entered the trailing edge, use - distance.
		nodeSpace.y = -(previous - positionOnPlane).Magnitude();
	}

	boundarySpace = ((positionOnPlane - farLeft) * (farRight - farLeft)) / (farRight - farLeft).MagnitudeSquared();
	return nodeSpace;
}

//--------------------------------------------------------------------------------------------------------------------//
