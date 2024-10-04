///
/// @file
/// @details Manage all the managers in the simulation on both game and server side to create the racing environment.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RaceSessionState_hpp
#define LudumDare56_RaceSessionState_hpp

#include "../core/event_system.hpp"
#include "../game_state/driver_license.hpp"

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_typed_integer.hpp>

#include <ice/physics/ice_physical_types.hpp>

#include <array>

namespace LudumDare56
{
	namespace GameState
	{

		class RacecarState;

		enum class GridIndexType : tbCore::uint8 { };
		typedef tbCore::TypedInteger<GridIndexType> GridIndex;

		enum class DriverIndexType : tbCore::uint8 { };
		typedef tbCore::TypedInteger<DriverIndexType> DriverIndex;
		constexpr DriverIndex::Integer kNumberOfModerators = 2;
		constexpr DriverIndex::Integer kNumberOfDrivers = 20 + kNumberOfModerators;
		constexpr DriverIndex InvalidDriver(void) { return DriverIndex::Integer(~0); }
		constexpr bool IsValidDriver(const DriverIndex driverIndex) { return driverIndex < kNumberOfDrivers; }

		enum class RacecarIndexType : tbCore::uint8 { };
		typedef tbCore::TypedInteger<RacecarIndexType> RacecarIndex;

		constexpr RacecarIndex::Integer kNumberOfRacecars = 20;
		constexpr RacecarIndex InvalidRacecar(void) { return RacecarIndex::Integer(~0); }
		constexpr bool IsValidRacecar(const RacecarIndex racecarIndex) { return racecarIndex < kNumberOfRacecars; }

		tb_static_error_if(kNumberOfRacecars > kNumberOfDrivers - kNumberOfModerators, "There are not enough regular drivers to fill racecars.");

		bool IsTrusted(void);

		namespace RaceSessionState
		{
			enum class SessionPhase : tbCore::uint8
			{
				kPhaseWaiting,     //The server is waiting for players to connect to the server.
				kPhasePractice,    //There is someone on the GameServer, so drive around like open practice.
				kPhaseGrid,        //Cars are stuck on the grid waiting for green-lights.
				kPhaseRacing,
				//kPhaseLoadRacetrack  //Not actually supported at this time, but it might be nice support in the future.
			};

			///
			/// @details Add an EventListenter for RaceSessionState changes through events.
			///
			void AddEventListener(TyreBytes::Core::EventListener& eventListener);

			///
			/// @details Remove an EventListenter to stop listening to events from the RaceSessionState changes.
			///
			void RemoveEventListener(TyreBytes::Core::EventListener& eventListener);

			///
			/// @param isTrusted should only be true for Singleplayer games or Multiplayer Servers. Multiplayer Clients
			///   are not to be trusted.
			///
			void Create(const bool isTrusted, const tbCore::tbString& racetrackFilepath = "");
			void Destroy(void);
			void Simulate(void);

			SessionPhase GetSessionPhase(void);
			void SetSessionPhase(SessionPhase phase);
			void SetSessionPhase(SessionPhase phase, tbCore::uint32 phaseTimer);

			tbCore::uint32 GetPhaseTimer(void);
			tbCore::uint32 GetWorldTimer(void);

			GridIndex GetGridIndexFor(const RacecarIndex racecarIndex);
			//void SetGridIndexFor(const RacecarIndex racecarIndex, const GridIndex gridIndex);

			RacecarIndex GetRacecarIndexOnGrid(const GridIndex gridIndex);

			void SetStartingGrid(const std::array<GridIndex, kNumberOfRacecars>& startingGrid);

			///
			/// @details While this will give you a randomized grid, it will ensure all active racecars are at the front
			///   of the grid, where inactive racecars are not.
			///
			void RandomizeStartingGrid(void);

			void PlaceCarOnGrid(RacecarState& racecar);

			///
			/// @details This should only be called from a GameServer / Singleplayer mode, and will search through all
			///   available driver slots to find the first open spot. If none is found, or another issue occurs, then
			///   InvalidDriver() is returned and the driver did NOT enter the compeition.
			///
			DriverIndex DriverEnterCompetition(const DriverLicense& driverLicense);
			void DriverEnterCompetition(const DriverIndex driverIndex, const DriverLicense& driverLicense);

			///
			/// @note A driver leaving the competition is automatically removed from their racecar as well.
			///
			void DriverLeaveCompetition(const DriverIndex driverIndex);

			///
			/// @details This should only be called from a GameServer or Singleplayer mode, and will reserves the next
			///   available racecar for the driver. If none is available InvalidRacecar() is returned.
			///
			RacecarIndex DriverEnterRacecar(const DriverIndex driverIndex);
			void DriverEnterRacecar(const DriverIndex driverIndex, RacecarIndex racecarIndex);
			void DriverLeaveRacecar(const DriverIndex driverIndex, const RacecarIndex racecarIndex);

			void RenderDebug(void);

		};	//namespace RaceSessionState
	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_RaceSessionState_hpp */
