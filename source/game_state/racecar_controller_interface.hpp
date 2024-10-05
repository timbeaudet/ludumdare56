///
/// @file
/// @details An entity within the TopDownRacer project.
/// @history Originally started 2017 with Playground TopDownRacer networking.
///   2023-10-17: Began combining with elements of TrailingBrakes (Fall 2022) for DriverActions.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RacecarControllerInterface_hpp
#define LudumDare56_RacecarControllerInterface_hpp

#include "ludumdare56.hpp"

namespace LudumDare56
{
	namespace GameState
	{
		enum class DriverAction { Ignition, Starter, TearOff, ShiftUp, ShiftDown, Handbrake, NumberOfActions };
		enum Gear { Neutral, First, Second, Third, Fourth, Fifth, Sixth, Reverse };

		class RacecarControllerInterface
		{
		public:
			RacecarControllerInterface(void);
			virtual ~RacecarControllerInterface(void);

			void ResetControls(void);
			void UpdateControls(void);

			inline tbCore::uint16 GetSteeringValue(void) const { return mSteeringValue; }
			inline tbCore::uint16 GetThrottleValue(void) const { return mThrottleValue; }
			inline tbCore::uint16 GetBrakeValue(void) const { return mBrakeValue; }

			float GetThrottlePercentage(void) const;
			float GetBrakePercentage(void) const;
			float GetSteeringPercentage(void) const;

			inline Gear GetShifterPosition(void) const { return mShifterPosition; }

			inline bool IsActionPressed(const DriverAction& action) const { return (true == mIsActionDown[static_cast<int>(action)] && false == mWasActionDown[static_cast<int>(action)]); }
			inline bool IsActionDown(const DriverAction& action) const { return (true == mIsActionDown[static_cast<int>(action)]); }

		protected:
			static const tbCore::uint16 kCenterSteeringValue;

			virtual void OnUpdateControls(void) = 0;

			inline void SetSteeringValue(const tbCore::uint16 steeringValue) { mSteeringValue = steeringValue; }
			inline void SetSteeringPercentage(const float steeringPercentage)
			{
				const float zeroToOne = tbMath::Clamp(steeringPercentage, -1.0f, 1.0f) * 0.5f + 0.5f;
				mSteeringValue = static_cast<tbCore::uint16>(zeroToOne * std::numeric_limits<tbCore::uint16>::max());
			}

			inline void SetThrottleValue(const tbCore::uint16 throttleValue) { mThrottleValue = throttleValue; }
			inline void SetThrottlePercentage(const float throttlePercentage)
			{
				mThrottleValue = static_cast<tbCore::uint16>(tbMath::Clamp(throttlePercentage, 0.0f, 1.0f) * std::numeric_limits<tbCore::uint16>::max());
			}


			inline void SetBrakeValue(const tbCore::uint16 brakeValue) { mBrakeValue = brakeValue; }
			inline void SetBrakePercentage(const float brakePercentage)
			{
				mBrakeValue = static_cast<tbCore::uint16>(tbMath::Clamp(brakePercentage, 0.0f, 1.0f) * std::numeric_limits<tbCore::uint16>::max());
			}

			inline void SetShifterPosition(Gear gear) { mShifterPosition = gear; }

			inline bool SetActionDown(const DriverAction& action, const bool isDown) { return mIsActionDown[static_cast<int>(action)] = isDown; }

		private:
			tbCore::uint16 mSteeringValue;
			tbCore::uint16 mThrottleValue;
			tbCore::uint16 mBrakeValue;
			Gear mShifterPosition;

			std::array<bool, static_cast<int>(DriverAction::NumberOfActions)> mIsActionDown;
			std::array<bool, static_cast<int>(DriverAction::NumberOfActions)> mWasActionDown;
		};

		class NullRacecarController : public RacecarControllerInterface
		{
		public:
			NullRacecarController(void);
			virtual ~NullRacecarController(void);

		protected:
			virtual void OnUpdateControls(void) override;
		};

		class BrakeOnlyRacecarController : public RacecarControllerInterface
		{
		public:
			BrakeOnlyRacecarController(void);
			virtual ~BrakeOnlyRacecarController(void);

		protected:
			virtual void OnUpdateControls(void) override;
		};
	};

};	//namespace LudumDare56

#endif /* LudumDare56_RacecarControllerInterface_hpp */
