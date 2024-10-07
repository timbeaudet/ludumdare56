///
/// @file
/// @details Manage all the managers in the simulation on both game and server side to create the racing environment.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../game_state/race_session_state.hpp"
#include "../game_state/racecar_state.hpp"
#include "../game_state/racetrack_state.hpp"
#include "../game_state/driver_state.hpp"
#include "../game_state/timing_and_scoring_state.hpp"

#include "../logging.hpp"

#include "../game_state/events/event_safety_checker.hpp"

#include <ice/physics/ice_physical_world.hpp>
#include <ice/physics/ice_rigid_body.hpp>

#include <unordered_map>
#include <map>
#include <algorithm>

namespace
{
	std::unique_ptr<icePhysics::World> thePhysicalWorld;
	std::unique_ptr<icePhysics::RigidBody> theTemporaryParklotBody;

	LudumDare56::GameState::RaceSessionState::SessionPhase theSessionPhase = LudumDare56::GameState::RaceSessionState::SessionPhase::kPhaseWaiting;
	tbGame::GameTimer thePhaseTimer = 0;
	tbGame::GameTimer theWorldTimer = 0;
	bool theTrustedMode = true;

	tbCore::tbString theCurrentTrackDisplayName = "";
	tbCore::tbString theNextRacetrackName = "";

	class RacetrackLoader : public TrackBundler::BundleProcessorInterface
	{
	private:
		virtual void OnCreateTrackSegment(const TrackBundler::Legacy::TrackSegment& trackSegment, const TrackBundler::Legacy::TrackBundle& trackBundle);
		virtual void OnCreateTrackObject(const TrackBundler::Legacy::TrackObject& trackObject, const TrackBundler::Legacy::TrackBundle& trackBundle);
		virtual void OnCreateTrackSpline(const TrackBundler::Legacy::TrackSpline& trackSpline, const TrackBundler::Legacy::TrackBundle& trackBundle);
	};

	TyreBytes::Core::EventBroadcaster theRaceSessionBroadcaster;

	typedef std::map<LudumDare56::GameState::RacecarIndex, LudumDare56::GameState::GridIndex> StartingGrid;
	StartingGrid theStartingGrid;
};

//Accessed by GameServer launch parameters.
const tbCore::tbString theOriginalDefaultRacetrackName = "default";
tbCore::tbString theDefaultRacetrackName = theOriginalDefaultRacetrackName;

tbCore::tbString RacetrackNameToFilepath(const tbCore::tbString& racetrackName)
{
	return "data/racetracks/" + racetrackName + ".trk";
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameState::IsTrusted(void)
{
	return theTrustedMode;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString ToString(const LudumDare56::GameState::RaceSessionState::SessionPhase& gamePhase)
{
	switch (gamePhase)
	{
	case LudumDare56::GameState::RaceSessionState::SessionPhase::kPhaseWaiting: return "Waiting";
	case LudumDare56::GameState::RaceSessionState::SessionPhase::kPhasePractice: return "Practice";
	case LudumDare56::GameState::RaceSessionState::SessionPhase::kPhaseGrid: return "Grid";
	case LudumDare56::GameState::RaceSessionState::SessionPhase::kPhaseRacing: return "Racing";
	//case LudumDare56::GameState::RaceSessionState::SessionPhase::kPhaseLoadRacetrack: return "LoadRacetrack";
	};

	return "Unknown";
}

// @note 2023-10-21: Unsure why, but tb_always_log(LogGame::Info() << "The phase: " << phase); will fail to compile with
//   ambiguity or other templated issues unless manually using ToString(), I'm  not entirely sure why, but I believe
//   this is also why I don't often use "Vector: " << vec3; in the logs despite it also having an overload.
std::ostream& operator<<(std::ostream& output, const LudumDare56::GameState::RaceSessionState::SessionPhase& sessionPhase)
{
	output << ToString(sessionPhase);
	return output;
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::AddEventListener(TyreBytes::Core::EventListener& eventListener)
{
	theRaceSessionBroadcaster.AddEventListener(eventListener);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::RemoveEventListener(TyreBytes::Core::EventListener& eventListener)
{
	theRaceSessionBroadcaster.RemoveEventListener(eventListener);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::Create(const bool isTrusted, const tbCore::tbString& racetrackFilepath)
{
	theTrustedMode = isTrusted;

	tb_debug_log(LogState::Info() << "RaceSessionState is Creating the Physical World!");

	/// Note: This function doesn't actually do anything at runtime, but will ensure all the Event IDs are safe.
	Events::SafetyCheck();

	if (nullptr != thePhysicalWorld)
	{
		Destroy();
	}

	if (false == GetQuickPlayRacetrackPath().empty())
	{
		RacetrackState::LoadRacetrack(GetQuickPlayRacetrackPath());
	}
	else if (false == racetrackFilepath.empty())
	{
		RacetrackState::LoadRacetrack(racetrackFilepath);
	}
	else
	{
		RacetrackState::LoadRacetrack(RacetrackNameToFilepath(theDefaultRacetrackName));
	}

	thePhysicalWorld.reset(new icePhysics::PhysicalWorld());
	thePhysicalWorld->SetGravity(icePhysics::Vector3(0.0f, -10.0f, 0.0f));

	///
	/// Note: (2023-09-20) Due to the (early) state of icePhysics and testing/API changed between Trailing Brakes and Terrible Brakes
	///   the internal workings of the RaycastVehicle cast rays differently. In Trailing Brakes it actually uses the
	///   rigid bodies in the world, which is probably the best option, but has a potential (future?) issue of the ray
	///   colliding with the car bodywork. The Terrible Brakes project used a custom mesh to cast a ray against known
	///   as the GlobalMeshCollider because we were testing the creation of physics similar to that of the LudumDare 50
	///   project in Unity.
	///
	/// The actual solution to this is to have BoundingVolumes for Convex/Concave Hulls and MeshColliders and apply those
	///   to a RigidBody in the world much like adding a plane. Then the whole system can work. This is actually our
	///   current goal, to get Capsules, ConvexHulls and MeshColliders working in the physics engine.
	///
	/// 2023-09-21: The above applies to the RaycastVehicle, but when trying to add other physics objects, we still
	///   needed to add the plane volume to the physical world for the other objects (cones in testing).

	//theTemporaryParklotBody.reset(new icePhysics::RigidBody(-1.0f));
	//theTemporaryParklotBody->AddBoundingVolume(new icePhysics::BoundingPlane(icePhysics::Vector3::Zero(), icePhysics::Vector3(0.0f, 1.0f, 0.0f)));
	//thePhysicalWorld->AddBody(*theTemporaryParklotBody);

	/// End Note.

	RacetrackState::Create(*thePhysicalWorld);

	theWorldTimer = 0;

	GridIndex gridIndex = 0;
	RacecarIndex racecarIndex = 0;
	for (RacecarState& racecar : RacecarState::AllMutableRacecars())
	{
		racecar.SetRacecarIndex(racecarIndex);
		racecar.Create(*thePhysicalWorld);

		theStartingGrid[racecarIndex] = gridIndex;

		++racecarIndex;
		++gridIndex;
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::Destroy(void)
{
	tb_debug_log(LogState::Info() << "RaceSessionState is Destroying the Physical World, oooh no!");

	for (const DriverState& driver : DriverState::AllDrivers())
	{
		if (true == driver.IsEntered())
		{
			DriverLeaveCompetition(driver.GetDriverIndex());
		}
	}

	if (nullptr != thePhysicalWorld)
	{
		for (RacecarState& racecar : RacecarState::AllMutableRacecars())
		{
			racecar.Destroy(*thePhysicalWorld);
		}

		RacetrackState::Destroy(*thePhysicalWorld);
	}

	TimingState::Invalidate();
	RacetrackState::InvalidateRacetrack();
	thePhysicalWorld = nullptr;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::Simulate(void)
{
	theWorldTimer += kFixedTimeMS;

	if (SessionPhase::kPhaseWaiting == theSessionPhase)
	{
		theWorldTimer = 0;
		thePhaseTimer = 0;
	}
	else if (SessionPhase::kPhaseGrid == theSessionPhase)
	{
		for (RacecarState& racecar : RacecarState::AllMutableRacecars())
		{
			GameState::RaceSessionState::PlaceCarOnGrid(racecar);
		}

		tb_error_if(true == thePhaseTimer.IsZero(), "This timer should not be zero in a Simulate step without decrementing below.");

		if (true == thePhaseTimer.DecrementStep())
		{	//Let the GameClient "predict" that the session phase jumped into racing here, server will still send it.
			SetSessionPhase(SessionPhase::kPhaseRacing);
		}
	}
	else if (SessionPhase::kPhasePractice == theSessionPhase && true == IsTrusted())
	{
		bool atLeastOneDriverEntered = false;
		bool allDriversWithCar = true;
		for (const DriverState& driver : DriverState::AllDrivers())
		{
			if (true == driver.IsEntered())
			{
				if (false == driver.IsDriving())
				{
					allDriversWithCar = false;
					break;
				}

				atLeastOneDriverEntered = true;
			}
		}

		if (true == atLeastOneDriverEntered && (true == allDriversWithCar || true == thePhaseTimer.IncrementStep(1000 * 60 * 3)))
		{
			SetSessionPhase(SessionPhase::kPhaseGrid);
		}
	}
	else if (SessionPhase::kPhaseRacing == theSessionPhase && true == IsTrusted())
	{
		bool atLeastOneRacecarFinished = false;
		for (const RacecarState& racecar : RacecarState::AllRacecars())
		{
			if (true == racecar.IsRacecarInUse() && true == TimingState::IsRacecarFinished(racecar.GetRacecarIndex()))
			{
				atLeastOneRacecarFinished = true;
				break;
			}
		}

		if (true == atLeastOneRacecarFinished || false == thePhaseTimer.IsZero())
		{
			if (true == thePhaseTimer.IncrementStep(1000 * 30))
			{
				SetSessionPhase(SessionPhase::kPhasePractice);
			}
		}
	}

	thePhysicalWorld->Simulate(kFixedTime);

	RacetrackState::Simulate();
	for (RacecarState& racecar : RacecarState::AllMutableRacecars())
	{
		if (false == racecar.IsRacecarInUse())
		{
			//Then lock car to grid and done.
			//racecar.SetVehicleToWorld(RacetrackState::GetSpawnToWorld(racecar.GetRacecarIndex()));
			continue;
		}

		racecar.Simulate();
	}

	TimingState::Simulate();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::PlaceCarOnGrid(RacecarState& racecar)
{
	const GridIndex gridIndex = GetGridIndexFor(racecar.GetRacecarIndex());
	racecar.ResetRacecar(RacetrackState::GetGridToWorld(gridIndex));
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RaceSessionState::SessionPhase LudumDare56::GameState::RaceSessionState::GetSessionPhase(void)
{
	return theSessionPhase;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::SetSessionPhase(SessionPhase phase)
{
	SetSessionPhase(phase, 0);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::SetSessionPhase(SessionPhase phase, tbCore::uint32 phaseTimer)
{
	const SessionPhase oldPhase = theSessionPhase;

	theSessionPhase = phase;
	thePhaseTimer = phaseTimer;

	switch (phase)
	{
	case SessionPhase::kPhasePractice: {
		TimingState::ResetCompetition();
		break; }

	case SessionPhase::kPhaseGrid: {
		if (true == IsTrusted() && 0 == phaseTimer)
		{
			RandomizeStartingGrid();
		}

		if (0 != phaseTimer)
		{	//GameServer or Singleplayer mode is expected to call SetSessionPhase(Grid, nonZero + worstLatency) from
			//  within the SendEvent for the phase change.
			thePhaseTimer += 3000; //Add 3 seconds to whatever time we have prior to getting here.
		}
		break; }
	case SessionPhase::kPhaseRacing: {
		theWorldTimer = 0;
		break; }

	default:
		break;
	};

	theRaceSessionBroadcaster.SendEvent(GameState::Events::RaceSessionPhaseChangeEvent(phase, phaseTimer));

	// @note 2023-10-21: Unsure why, but tb_always_log(LogGame::Info() << "The phase: " << phase); will fail to compile with
	//   ambiguity or other templated issues unless manually using ToString(), I'm  not entirely sure why, but I believe
	//   this is also why I don't often use "Vector: " << vec3; in the logs despite it also having an overload.
	tb_always_log(LogState::Info() << "The RaceSession has changed from " << ToString(oldPhase) << " to: " << ToString(phase));
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::uint32 LudumDare56::GameState::RaceSessionState::GetPhaseTimer(void)
{
	return thePhaseTimer.GetElapsedTime();
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::uint32 LudumDare56::GameState::RaceSessionState::GetWorldTimer(void)
{
	return theWorldTimer.GetElapsedTime();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::GridIndex LudumDare56::GameState::RaceSessionState::GetGridIndexFor(const RacecarIndex racecarIndex)
{
	return theStartingGrid[racecarIndex];
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarIndex LudumDare56::GameState::RaceSessionState::GetRacecarIndexOnGrid(const GridIndex gridIndex)
{
	for (const RacecarState& racecar : RacecarState::AllRacecars())
	{
		if (gridIndex == theStartingGrid[racecar.GetRacecarIndex()])
		{
			return racecar.GetRacecarIndex();
		}
	}

	return InvalidRacecar();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::SetStartingGrid(const std::array<GridIndex, kNumberOfRacecars>& startingGrid)
{
	for (const RacecarState& racecar : RacecarState::AllRacecars())
	{
		const RacecarIndex racecarIndex = racecar.GetRacecarIndex();
		theStartingGrid[racecarIndex] = startingGrid[racecarIndex];
	}

	//tb_debug_log(LogState::Info() << "Randomizing the starting grid: " << tbCore::Debug::ContinueEntry());
	//for (const RacecarState& racecar : RacecarState::AllRacecars())
	//{
	//	const GridIndex gridIndex = GetGridIndexFor(racecar.GetRacecarIndex());
	//	tb_debug_log("\n\t" << +gridIndex << ". " << DebugInfo(racecar) << tbCore::Debug::ContinueEntry());
	//}
	//tb_debug_log("");

	tb_debug_log(LogState::Info() << "Randomizing the starting grid: " << tbCore::Debug::ContinueEntry());
	for (GridIndex gridIndex = 0; gridIndex < kNumberOfRacecars; ++gridIndex)
	{
		const RacecarIndex racecarIndex = GetRacecarIndexOnGrid(gridIndex);
		tb_debug_log("\n\t" << +gridIndex << ". " << DebugInfo(racecarIndex) << tbCore::Debug::ContinueEntry());
	}
	tb_debug_log("");

	theRaceSessionBroadcaster.SendEvent(TyreBytes::Core::Event(Events::RaceSession::StartGridChanged));
}

//--------------------------------------------------------------------------------------------------------------------//

const tbCore::tbString& LudumDare56::GameState::RaceSessionState::GetCurrentTrackDisplayName(void)
{
	return theCurrentTrackDisplayName;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::SetCurrentTrackDisplayName(const String& trackName)
{
	theCurrentTrackDisplayName = trackName;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::SetNextLevel(const String& trackName)
{
	theNextRacetrackName = trackName;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::AdvanceToNextLevel(void)
{
	//const String racetrackName = (true == theNextRacetrackName.empty()) ? theDefaultRacetrackName : theNextRacetrackName;
	//const String racetrackFilepath = RacetrackNameToFilepath(racetrackName);

	////What all we need to do to reload to a new track? That is new!
	//Destroy();
	//Create(true, racetrackFilepath);

	theDefaultRacetrackName = (true == theNextRacetrackName.empty()) ? theOriginalDefaultRacetrackName : theNextRacetrackName;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::RandomizeStartingGrid(void)
{
	std::vector<RacecarIndex> readyGridIndices;
	readyGridIndices.reserve(kNumberOfRacecars);

	std::vector<RacecarIndex> inactiveGridIndices;
	inactiveGridIndices.reserve(kNumberOfRacecars);

	for (const RacecarState& racecar : RacecarState::AllRacecars())
	{
		std::vector<RacecarIndex>& gridContainer = (true == racecar.IsRacecarInUse()) ? readyGridIndices : inactiveGridIndices;
		gridContainer.push_back(racecar.GetRacecarIndex());
	}

	std::random_device randomDevice;
	std::mt19937 randomAlgorithm(randomDevice());
	std::shuffle(readyGridIndices.begin(), readyGridIndices.end(), randomAlgorithm);
	std::shuffle(inactiveGridIndices.begin(), inactiveGridIndices.end(), randomAlgorithm);

	std::array<GridIndex, kNumberOfRacecars> startingGrid;

	for (const RacecarState& racecar : RacecarState::AllRacecars())
	{
		const GridIndex totalReadyIndex = (true == racecar.IsRacecarInUse()) ? 0 : static_cast<GridIndex::Integer>(readyGridIndices.size());
		const std::vector<RacecarIndex>& gridContainer = (true == racecar.IsRacecarInUse()) ? readyGridIndices : inactiveGridIndices;

		for (GridIndex gridIndex = 0; gridIndex < static_cast<GridIndex::Integer>(gridContainer.size()); ++gridIndex)
		{
			if (gridContainer[gridIndex] == racecar.GetRacecarIndex())
			{
				startingGrid[racecar.GetRacecarIndex()] = gridIndex + totalReadyIndex;
				break;
			}
		}
	}

	SetStartingGrid(startingGrid);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::DriverIndex LudumDare56::GameState::RaceSessionState::DriverEnterCompetition(
	const DriverLicense& driverLicense)
{
	if (true == IsTrusted() && false == driverLicense.mIsModerator)
	{
		// @note 2023-11-07: We need to know if the moderator driver slots are still opened, that takes precedence over all
		//   other things.
		int moderatorCount = 0;
		int availableCount = 0;
		for (const DriverState& driver : DriverState::AllDrivers())
		{
			if (false == driver.IsEntered())
			{
				++availableCount;
			}
			else if (true == driver.IsModerator())
			{
				++moderatorCount;
			}
		}

		const int reservedForModerators = kNumberOfModerators - moderatorCount;
		if (reservedForModerators >= availableCount)
		{
			return InvalidDriver();
		}
	}

	for (const DriverState& driver : DriverState::AllDrivers())
	{
		if (false == driver.IsEntered())
		{
			DriverEnterCompetition(driver.GetDriverIndex(), driverLicense);
			return driver.GetDriverIndex();
		}
	}

	return InvalidDriver();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::DriverEnterCompetition(const DriverIndex driverIndex, const DriverLicense& driverLicense)
{
	tb_error_if(false == IsValidDriver(driverIndex), "Error: A driver cannot enter the competition with an invalid driverIndex.");

	const DriverState& driver = DriverState::Get(driverIndex);
	tb_always_log_if(true == driver.IsEntered(), LogState::Error() << "Driver already entered in competition.");

	DriverState::GetMutable(driverIndex).EnterCompetition(driverLicense);
	tb_always_log(LogState::Info() << DebugInfo(driverIndex) << " has ENTERED the competition.");
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::DriverLeaveCompetition(const DriverIndex driverIndex)
{
	tb_error_if(false == IsValidDriver(driverIndex), "Error: A driver cannot leave the competition if they never entered, invalid driverIndex.");

	const DriverState& driver = DriverState::Get(driverIndex);
	tb_always_log(LogState::Info() << DebugInfo(driverIndex) << " has left the competition.");

	if (true == driver.IsDriving())
	{
		DriverLeaveRacecar(driverIndex, driver.GetRacecarIndex());
	}
	DriverState::GetMutable(driverIndex).LeaveCompetition();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarIndex LudumDare56::GameState::RaceSessionState::DriverEnterRacecar(const DriverIndex driverIndex)
{
	tb_error_if(false == IsValidDriver(driverIndex), "Error: An invalid driver cannot reserve or use a racecar.");

	RacecarIndex racecarIndex = 0;
	for (const RacecarState& racecar : RacecarState::AllRacecars())
	{
		if (false == racecar.IsRacecarInUse())
		{
			DriverEnterRacecar(driverIndex, racecarIndex);
			return racecarIndex;
		}

		++racecarIndex;
	}

	tb_always_log(LogState::Warning() << "There was no racecar for the driver to enter.");
	return kNumberOfRacecars;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::DriverEnterRacecar(const DriverIndex driverIndex, RacecarIndex racecarIndex)
{
	tb_error_if(false == IsValidRacecar(racecarIndex), "Error: Cannot reserve a racecar with an invalid racecarIndex.");
	tb_error_if(false == IsValidDriver(driverIndex), "Error: An invalid driver cannot reserve or use a racecar.");

	RacecarState& racecar = RacecarState::GetMutable(racecarIndex);
	tb_always_log_if(true == racecar.IsRacecarInUse(), LogState::Error() << "Reserving a Racecar that is already in use.");

	DriverState::GetMutable(driverIndex).EnterRacecar(racecarIndex);

	racecar.SetRacecarController(new NullRacecarController());
	racecar.SetRacecarDriver(driverIndex);

	const DriverState& driver = DriverState::Get(driverIndex);
	tb_error_if(false == driver.IsDriving(), "Error: Expected the driver to be driving a racecar.");
	tb_error_if(false == racecar.IsRacecarInUse(), "Error: Expected the racecar to be in use by a driver.");
	tb_error_if(driver.GetRacecarIndex() != racecarIndex, "Error: Driver should have entered the racecar.");
	tb_error_if(racecar.GetDriverIndex() != driverIndex, "Error: The racecar should belong to the driver trying to enter it.");

	tb_always_log(LogState::Info() << DebugInfo(driver) << " has entered a " << DebugInfo(racecar) << ").");
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::DriverLeaveRacecar(const DriverIndex driverIndex, const RacecarIndex racecarIndex)
{
	tb_error_if(false == IsValidRacecar(racecarIndex), "Error: Cannot leave a racecar with invalid racecarIndex.");
	tb_error_if(false == IsValidDriver(driverIndex), "Error: An invalid driver cannot leave a racecar.");
	const DriverState& driver = DriverState::Get(driverIndex);
	RacecarState& racecar = RacecarState::GetMutable(racecarIndex);

	tb_error_if(driver.GetRacecarIndex() != racecarIndex, "Error: Driver cannot leave a racecar that they are not in.");
	tb_error_if(racecar.GetDriverIndex() != driverIndex, "Error: Racecar does not belong to the driver trying to leave it.");

	tb_always_log(LogState::Info() << DebugInfo(driver) << " has left the " << DebugInfo(racecar) << ").");

	DriverState::GetMutable(driverIndex).LeaveRacecar();
	racecar.SetRacecarController(nullptr);
	racecar.SetRacecarDriver(InvalidDriver());
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RaceSessionState::RenderDebug(void)
{
#if !defined(ludumdare56_headless_build)
	if (nullptr != thePhysicalWorld)
	{
		icePhysics::PhysicalVisualizer visualizer;
		thePhysicalWorld->DebugRender(visualizer);
		visualizer.Render();
	}

	TimingState::RenderDebug();
	RacetrackState::RenderDebug();

	for (const RacecarState& racecar : RacecarState::AllRacecars())
	{
		racecar.RenderDebug();
	}
#endif /* ludumdare56_headless_build */
}

//--------------------------------------------------------------------------------------------------------------------//
