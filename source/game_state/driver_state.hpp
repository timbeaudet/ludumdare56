///
/// @file
/// @details A driver is an active connection on the server, it doesn't mean they are driving the car; they could be
///   just waiting for their chance to do a driver swap into a car, or spectating the event, or even being a spotter.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_DriverState_hpp
#define LudumDare56_DriverState_hpp

#include "race_session_state.hpp"
#include "driver_license.hpp"

#include "../core/typed_range.hpp"
#include "../core/event_system.hpp"

namespace LudumDare56
{
	namespace GameState
	{

		class DriverState : public TyreBytes::Core::EventBroadcaster
		{
		public:
			static const DriverState& Get(const DriverIndex driverIndex);
			static DriverState& GetMutable(const DriverIndex driverIndex);

			///
			/// @note This is purely for some syntactical sugars of using ranged for-loops;
			///
			typedef TyreBytes::Core::TypedRange<DriverIndex, const DriverState, kNumberOfDrivers, DriverState::Get> DriverContainerAccessor;
			typedef TyreBytes::Core::TypedRange<DriverIndex, DriverState, kNumberOfDrivers, DriverState::GetMutable> MutableDriverContainerAccessor;
			static inline constexpr DriverContainerAccessor AllDrivers(void) { return DriverContainerAccessor(); }
			static inline constexpr MutableDriverContainerAccessor AllMutableDrivers(void) { return MutableDriverContainerAccessor(); }

			///
			/// @note This constructor should be considered private and the DriverState::Get() or GetMutable() should be the
			///   only ways to access the driver state. Tim attempted to use a friend of creator object, but that did not
			///   allow use with a std::array which is used internally... 2022-07-14
			///
			explicit DriverState(void);
			~DriverState(void);

			///
			/// @details Checks if a driver is active / entered into a competition; This is different from what will become
			///   IsRegistered() which keeps the driver information of anyone that joined a competition as long as it remains
			///   relativant. You would not access a registered driver by driver index, it would only be by unique id where
			///   as the active/entered drivers are accessible by index.
			///
			bool IsEntered(void) const;

			const tbCore::tbString& GetLicense(void) const;

			const tbCore::tbString& GetName(void) const;

			///
			/// @details Returns the index of the driver, which technically should always be valid...
			///
			DriverIndex GetDriverIndex(void) const;

			///
			/// @details Retrieves the index of the racecar that the driver is belonging to.
			///
			/// @note It is possible for the driver to not be in a racecar and this will return InvalidRacecar() in that
			///   type of situation.
			///
			RacecarIndex GetRacecarIndex(void) const;

			bool IsModerator(void) const;

			bool IsDriving(void) const;

			//-----------------------        BELOW SHOULD ONLY BE USED BY RACE SESSION        ------------------------//

			void SetDriverIndex(const DriverIndex driverIndex);

			void EnterCompetition(const DriverLicense& license);
			void LeaveCompetition(void);

			void EnterRacecar(const RacecarIndex racecarIndex);
			void LeaveRacecar(void);

		private:
			DriverLicense mIdentifier;
			DriverIndex mDriverIndex;
			RacecarIndex mRacecarIndex;
		};

	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_DriverState_hpp */
