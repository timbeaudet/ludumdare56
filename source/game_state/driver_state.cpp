///
/// @file
/// @details A driver is an active connection on the server, it doesn't mean they are driving the car; they could be
///   just waiting for their chance to do a driver swap into a car, or spectating the event, or even being a spotter.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../game_state/driver_state.hpp"
#include "../game_state/events/driver_events.hpp"
#include "../logging.hpp"

#include <array>

namespace
{
	std::array<LudumDare56::GameState::DriverState, LudumDare56::GameState::kNumberOfDrivers>& ArrayInstance(void)
	{
		static std::array<LudumDare56::GameState::DriverState, LudumDare56::GameState::kNumberOfDrivers> theDriverStates;
		static bool hasDoneOnce = false;
		if (false == hasDoneOnce)
		{
			LudumDare56::GameState::DriverIndex driverIndex = 0;
			for (LudumDare56::GameState::DriverState& driver : theDriverStates)
			{
				driver.SetDriverIndex(driverIndex);
				++driverIndex;
			}

			hasDoneOnce = true;
		}

		return theDriverStates;
	}
};

//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::GameState::DriverState& LudumDare56::GameState::DriverState::Get(const DriverIndex driverIndex)
{
	tb_error_if(false == IsValidDriver(driverIndex), "Error: Invalid driverIndex(%d).", +driverIndex);
	return ArrayInstance()[driverIndex];
}

LudumDare56::GameState::DriverState& LudumDare56::GameState::DriverState::GetMutable(const DriverIndex driverIndex)
{
	tb_error_if(false == IsValidDriver(driverIndex), "Error: Invalid driverIndex(%d).", +driverIndex);
	return ArrayInstance()[driverIndex];
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::DriverState::DriverState(void) :
	TyreBytes::Core::EventBroadcaster(),
	mIdentifier(DriverLicense::Invalid()),
	mDriverIndex(InvalidDriver()),
	mRacecarIndex(InvalidRacecar())
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::DriverState::~DriverState(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameState::DriverState::IsEntered(void) const
{
	return (false == mIdentifier.mIdentifier.empty() && false == mIdentifier.mName.empty());
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::DriverState::EnterCompetition(const DriverLicense& license)
{
	mIdentifier = license;
	tb_debug_log_if(license.mIsModerator, "Moderator has Joined the Competition: " << license.mIdentifier);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::DriverState::LeaveCompetition(void)
{
	mIdentifier = DriverLicense::Invalid();
}

//--------------------------------------------------------------------------------------------------------------------//

const tbCore::tbString& LudumDare56::GameState::DriverState::GetLicense(void) const
{
	return mIdentifier.mIdentifier;
}

//--------------------------------------------------------------------------------------------------------------------//

const tbCore::tbString& LudumDare56::GameState::DriverState::GetName(void) const
{
	return mIdentifier.mName;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::DriverIndex LudumDare56::GameState::DriverState::GetDriverIndex(void) const
{
	return mDriverIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::DriverState::SetDriverIndex(const DriverIndex driverIndex)
{
	mDriverIndex = driverIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarIndex LudumDare56::GameState::DriverState::GetRacecarIndex(void) const
{
	return mRacecarIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameState::DriverState::IsModerator(void) const
{
	return mIdentifier.mIsModerator;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameState::DriverState::IsDriving(void) const
{
	return IsValidRacecar(mRacecarIndex);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::DriverState::EnterRacecar(const RacecarIndex racecarIndex)
{
	mRacecarIndex = racecarIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::DriverState::LeaveRacecar(void)
{
	mRacecarIndex = InvalidRacecar();
}

//--------------------------------------------------------------------------------------------------------------------//
