///
/// @file
/// @details A simple helper to take a vehicle and some acceleration paramters to tilt the body from the vehicle.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_VehicleBodyTilter_hpp
#define LudumDare56_VehicleBodyTilter_hpp

#include "../../ludumdare56.hpp"

namespace LudumDare56::GameState::PhysicsModels
{

	struct VehicleBodyTilter
	{
		iceVector3 mPreviousVelocity = iceVector3::Zero();
		iceMatrix4 mBodyToVehicle = iceMatrix4::Identity();
		iceAngle mBodyRoll = iceAngle::Zero();
		iceAngle mBodyPitch = iceAngle::Zero();

		inline iceMatrix4 GetBodyToVehicle(void) const { return mBodyToVehicle; }

		inline void SimulateBodyRoll(const PhysicsModelInterface& physicsModel)
		{
			const iceVector3 vehicleVelocity = physicsModel.GetLinearVelocity();
			const iceVector3 acceleration = (vehicleVelocity - mPreviousVelocity) / kFixedTime;
			const iceVector3 accelerationVehicle = physicsModel.GetVehicleToWorld().FastInverse().TransformNormal(acceleration);
			mPreviousVelocity = vehicleVelocity;

			const iceScalar maximumGee = 2.0;
			const iceScalar clampedLateralGees = tbMath::Clamp(accelerationVehicle.x / 10.0f / 15.0f, -maximumGee, maximumGee);
			const iceScalar clampedLongitudinalGees = tbMath::Clamp(accelerationVehicle.z / 10.0f / 2.0f, -maximumGee, maximumGee);

			mBodyRoll = tbMath::Interpolation::Linear(0.2f, mBodyRoll, iceAngle::Degrees(10.0) * clampedLateralGees);
			mBodyPitch = tbMath::Interpolation::Linear(0.2f, mBodyPitch, iceAngle::Degrees(-10.0) * clampedLongitudinalGees);

			mBodyToVehicle = iceMatrix4::RotationZ(mBodyRoll) * iceMatrix4::RotationX(mBodyPitch);
			//tb_always_log("Acceleration is: { Lat: " << accelerationVehicle.x / 10.0 << " , Long: " << accelerationVehicle.z / 10.0 << " }");
		}

	};

};	//namespace LudumDare56::GameState::PhysicsModels

#endif /* LudumDare56_VehicleBodyTilter_hpp */
