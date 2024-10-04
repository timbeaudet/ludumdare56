///
/// @file
/// @details A simple helper to take simulate a basic gear box for a vehicle.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_VehicleGearBox_hpp
#define LudumDare56_VehicleGearBox_hpp

#include "../../ludumdare56.hpp"
#include "../../game_state/racecar_controller_interface.hpp"

#include <utility>

namespace LudumDare56::GameState::PhysicsModels
{
	namespace HardcodedValues
	{
		static const iceScalar kFinalRatio = iceScalar(4.30); //final drive ratio
		static const std::array<iceScalar, 8> kGearRatios = {	//Gear Ratios for ... you guessed it, the 1999 Apex5
			//iceScalar(0.0), iceScalar(3.163), iceScalar(1.888), iceScalar(1.333), iceScalar(1.000), iceScalar(0.814), iceScalar(0.0), iceScalar(-3.163),
			iceScalar(0.0), iceScalar(4.163), iceScalar(2.38), iceScalar(1.8), iceScalar(1.0), iceScalar(0.9), iceScalar(0.75), iceScalar(-3.163),
		};
	}

	struct VehicleGearBox
	{
		Gear mCurrentGear = Gear::Neutral;
		bool mCanShift = true;
		bool mIsAutomatic = true;

		const Gear kMaximumGear = Gear::Sixth;

		explicit VehicleGearBox(const Gear maximumGear) :
			kMaximumGear(maximumGear)
		{
		}

		inline void Reset(void)
		{
			mCurrentGear = Gear::Neutral;
			mCanShift = true;
		}

		inline iceScalar CalculateWheelTorque(const iceScalar& engineTorque, const Gear& currentGear)
		{
			const icePhysics::Scalar gearRatio = HardcodedValues::kGearRatios[currentGear];
			return engineTorque * gearRatio * HardcodedValues::kFinalRatio;
		}

		///
		/// @note 2024-09-04: Not saying this is right, but, it was how OG auto shifting worked :D
		///
		inline Gear SimulateAutomaticShifting(const iceScalar vehicleGroundSpeed, iceScalar& throttle, iceScalar& brake)
		{
			if (true == mIsAutomatic)
			{
				if (vehicleGroundSpeed < 0.01f)
				{
					mCurrentGear = Gear::Neutral;
				}

				if (Gear::Neutral == mCurrentGear)
				{
					if (brake > iceScalar(0.1) && brake > throttle)
					{
						mCurrentGear = Gear::Reverse;
					}
					else if (throttle > iceScalar(0.1))
					{
						mCurrentGear = Gear::First;
					}
				}

				if (Gear::Reverse == mCurrentGear)
				{
					std::swap(throttle, brake);
				}
			}

			return mCurrentGear;
		}

		///
		/// @note engineSpeed is in and out!
		///
		inline iceScalar SimulateGearBox(const iceScalar engineSpeed, const iceScalar wheelRadius,
			const PhysicsModelInterface& physicsModel, const RacecarControllerInterface& racecarController)
		{
			bool automaticShiftUp = false;
			bool automaticShiftDown = false;

			if (true == mIsAutomatic && Gear::Reverse != mCurrentGear)
			{
				//icePhysics::RaycastVehicle& racecar = mPhysicalVehicle;

				//icePhysics::RigidBody& rigidBody = *racecar.HackyAPI_GetRigidBody();
				//const icePhysics::Vector3 vehicleGroundVelocity(rigidBody.GetLinearVelocity().x, 0.0f, rigidBody.GetLinearVelocity().z);

				//const Scalar vehicleGroundSpeed = vehicleGroundVelocity.Magnitude();

				//icePhysics::Scalar throttle = racecarController.GetThrottlePosition();
				//icePhysics::Scalar brake = racecarController.GetBrakePosition();

				//if (Gear::Reverse == mGearbox.mCurrentGear)
				//{
				//	vehicleGroundSpeed
				//}
				//if (Gear::Neutral == mGearbox.mCurrentGear)
				//{

				//}

				if (engineSpeed > 5000)
				{
					automaticShiftUp = true;
				}
				if (engineSpeed < 3000)
				{
					if (Gear::First != mCurrentGear && Gear::Neutral != mCurrentGear && Gear::Reverse != mCurrentGear)
					{
						automaticShiftDown = true;
					}
				}
			}

			if (true == mCanShift)
			{
				if (true == automaticShiftUp || true == racecarController.IsActionPressed(DriverAction::ShiftUp))
				{
					if (mCurrentGear < kMaximumGear)
					{
						mCurrentGear = static_cast<Gear>(static_cast<int>(mCurrentGear) + 1);
					}
					else if (mCurrentGear == Gear::Reverse)
					{
						mCurrentGear = Gear::Neutral;
					}

					mCanShift = false;
				}
				if (true == automaticShiftDown || true == racecarController.IsActionPressed(DriverAction::ShiftDown))
				{
					if (mCurrentGear == Gear::Neutral)
					{
						mCurrentGear = Gear::Reverse;
					}
					else if (mCurrentGear > Gear::Neutral && mCurrentGear != Gear::Reverse)
					{
						mCurrentGear = static_cast<Gear>(static_cast<int>(mCurrentGear) - 1);
					}

					mCanShift = false;
				}
			}
			else
			{
				if (false == racecarController.IsActionDown(DriverAction::ShiftUp) &&
					false == racecarController.IsActionDown(DriverAction::ShiftDown))
				{
					mCanShift = true;
				}
			}

			//Set the engine speed based on vehicle speed and current gear, it isn't perfect, but we don't have engine inertia yet.
			if (Gear::Neutral == mCurrentGear)
			{	//This isn't correct, but for a faux gearbox it will work for today.
				return iceScalar(0.0);
			}

			const iceScalar reverseGearRatio = iceScalar(3.163);
			const iceVector3 vehicleForwardDirection = -physicsModel.GetVehicleToWorld().GetBasis(2);
			const iceScalar forwardSpeed = Vector3::Dot(vehicleForwardDirection, physicsModel.GetLinearVelocity());
			const iceScalar gearRatio = (forwardSpeed < 0.0f) ? -reverseGearRatio : HardcodedValues::kGearRatios[mCurrentGear];
			const iceScalar wheelSpeed = forwardSpeed / wheelRadius;
			return tbMath::Convert::RadiansSecondToRevolutionsMinute(wheelSpeed * HardcodedValues::kFinalRatio * gearRatio);
		}

	};

};	//namespace LudumDare56::GameState::PhysicsModels

#endif /* LudumDare56_VehicleGearBox_hpp */
