///
/// @file
/// @details The most basic of all physics models separated out from the racecars.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_state/physics/model_extreme_drifting.hpp"
#include "../../game_state/helpers/torque_curve.hpp"
#include "../../game_state/racecar_controller_interface.hpp"

namespace
{
	using namespace LudumDare56;

	const iceAngle kMaximumTurnAngle = iceAngle::Degrees(20.0);
	const iceScalar kEngineRevLimiter = iceScalar(7200); //rpm
	const iceScalar kWheelRadius = iceScalar(0.29337);

	icePhysics::VehicleInfo DefaultVehicle(void)
	{
		icePhysics::VehicleInfo vehicle;
		vehicle.mass = 1042.809f; //kg

		//vehicle.boundingBox = Vector3(1.0f, 0.75f, 1.5f); //width, height, depth
		vehicle.boundingBox = iceVector3(iceScalar(1.1f + 0.125), iceScalar(0.75f), iceScalar(1.6f)); //width, height, depth (make it a little wider/longer than trackWidth/wheelBase)
		return vehicle;
	}

	icePhysics::Spring DefaultSpring(void)
	{
		icePhysics::Spring spring;
		spring.mStrength = 10000.0f;
		spring.mDamper = 600.0f;
		return spring;
	}

};

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::PhysicsModels::ExtremeDriftingPhysicsModel::ExtremeDriftingPhysicsModel(icePhysics::World& physicalWorld) :
	RaycastVehiclePhysicsModelInterface(physicalWorld, DefaultVehicle()),
	mEngineSpeed(0.0),

	mPreviousVelocity(iceVector3::Zero()),
	mGearBox(Gear::Third),
	mBodyTilter(),
	mDriftEndedTimer(0),
	mIsHandbrakePulled(false)
{
	const iceScalar halfWheelBase = iceScalar(0.838906);
	const iceScalar halfTrackWidth = iceScalar(0.732454);
	const iceVector3 suspensionDirection(iceScalar(0.0), iceScalar(-1.0), iceScalar(0.0));
	mPhysicalVehicle.AddWheel(iceVector3(halfTrackWidth, -0.25, -halfWheelBase), suspensionDirection, DefaultSpring(), kWheelRadius);
	mPhysicalVehicle.AddWheel(iceVector3(-halfTrackWidth, -0.25, -halfWheelBase), suspensionDirection, DefaultSpring(), kWheelRadius);
	mPhysicalVehicle.AddWheel(iceVector3(halfTrackWidth, -0.25, halfWheelBase), suspensionDirection, DefaultSpring(), kWheelRadius);
	mPhysicalVehicle.AddWheel(iceVector3(-halfTrackWidth, -0.25, halfWheelBase), suspensionDirection, DefaultSpring(), kWheelRadius);

	mPhysicalVehicle.SetFrictionModel(icePhysics::RaycastVehicle::FrictionModel::kOther);

	icePhysics::RigidBody& rigidBody = *mPhysicalVehicle.HackyAPI_GetRigidBody();
	rigidBody.SetRestitutionCoefficient(0.0f);
	//rigidBody.AddBoundingVolume(new icePhysics::BoundingSphere(icePhysics::Vector3(0.0, 0.25, 0.0), 0.6f));
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::PhysicsModels::ExtremeDriftingPhysicsModel::~ExtremeDriftingPhysicsModel(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::ExtremeDriftingPhysicsModel::OnResetRacecarForces(void)
{
	RaycastVehiclePhysicsModelInterface::OnResetRacecarForces();

	mIsHandbrakePulled = false;
	mDriftEndedTimer = 0;

	mEngineSpeed = iceScalar(0.0);
	mGearBox.Reset();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::ExtremeDriftingPhysicsModel::OnSimulate(const RacecarControllerInterface& racecarController)
{
	RaycastVehiclePhysicsModelInterface::OnSimulate(racecarController);

	mEngineSpeed = mGearBox.SimulateGearBox(mEngineSpeed, kWheelRadius, *this, racecarController);
	SimulateTireGrip(racecarController);
	SimulateFizzics(racecarController);

	mBodyTilter.SimulateBodyRoll(*this);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::ExtremeDriftingPhysicsModel::OnDebugRender(void) const
{
	RaycastVehiclePhysicsModelInterface::OnDebugRender();
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Angle LudumDare56::GameState::PhysicsModels::ExtremeDriftingPhysicsModel::GetSteeringAngle(
	const iceScalar steeringInput, const iceScalar vehicleGroundSpeed)
{	//TODO: LudumDare56: Physics: This is definitely NOT a good way to figure out the steering angle, based off Rally of Rockets.
	const iceScalar minimumSpeed(5.0); //m/s
	const iceScalar maximumSpeed(20.0); //m/s

	const iceScalar speedPercentage = tbMath::Clamp((vehicleGroundSpeed - minimumSpeed) / (maximumSpeed - minimumSpeed), iceScalar(0.0), iceScalar(1.0));
	const iceAngle maximumAngle = tbMath::Interpolation::Linear(speedPercentage, kMaximumTurnAngle, kMaximumTurnAngle / iceScalar(4.0));
	return steeringInput * maximumAngle;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Angle LudumDare56::GameState::PhysicsModels::ExtremeDriftingPhysicsModel::CalculateDriftAngle(void) const
{
	const iceVector3 vehicleForward = -mPhysicalVehicle.GetVehicleToWorld().GetBasis(2);
	return vehicleForward.AngleTo(GetLinearVelocity());
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::ExtremeDriftingPhysicsModel::SimulateFizzics(const RacecarControllerInterface& racecarController)
{
	icePhysics::RaycastVehicle& racecar = mPhysicalVehicle;

	icePhysics::RigidBody& rigidBody = *racecar.HackyAPI_GetRigidBody();
	const iceVector3 vehicleGroundVelocity(rigidBody.GetLinearVelocity().x, 0.0f, rigidBody.GetLinearVelocity().z);
	const iceScalar vehicleGroundSpeed = vehicleGroundVelocity.Magnitude();

	iceScalar throttle = racecarController.GetThrottlePercentage();
	iceScalar brake = racecarController.GetBrakePercentage();

	// 2024-09-04: Worth noting that throttle/brake may be swapped in reverse.
	mGearBox.SimulateAutomaticShifting(vehicleGroundSpeed, throttle, brake);

	const iceMatrix4 carToWorld = rigidBody.GetObjectToWorld();
	const iceVector3 carRight = carToWorld.GetBasis(0);
	const iceVector3 carUp = carToWorld.GetBasis(1);
	const iceVector3 carForward = -carToWorld.GetBasis(2);

	const bool isOnThrottle = (throttle > 0.001f);
	const bool isOnBrake = (brake > 0.001f);
	mIsHandbrakePulled = racecarController.IsActionDown(DriverAction::Handbrake);


	for (size_t wheelIndex = 0; wheelIndex < 4; ++wheelIndex)
	{
		mPhysicalVehicle.SetSteeringAngle(iceAngle::Zero(), wheelIndex);
		mPhysicalVehicle.SetEngineTorque(wheelIndex, iceScalar(0.0));
		mPhysicalVehicle.SetBrakeTorque(wheelIndex, iceScalar(0.0));
	}

	if (true == IsDrifting())
	{	//Handle rotation (and add some track) as if the car were Asteroids like controlled.
		if (CalculateDriftAngle() < iceAngle::Degrees(5.0))
		{
			mDriftEndedTimer.DecrementStep();
		}
		else
		{
			mDriftEndedTimer = 250;
		}

		const iceVector3 forwardGroundVelocity = carForward * Vector3::Dot(carForward, vehicleGroundVelocity);
		const iceVector3 lateralGroundVelocity = carRight * Vector3::Dot(carRight, vehicleGroundVelocity);
		//const iceVector3 lateralGroundVelocity = (vehicleGroundVelocity - forwardGroundVelocity);

		iceVector3 linearVelocity = mPhysicalVehicle.GetLinearVelocity();
		//linearVelocity -= forwardGroundVelocity * 0.35f * kFixedTime; //Forward drag applied elsewhere?
		linearVelocity -= lateralGroundVelocity * 1.35f * kFixedTime;

		//linearVelocity -= lateralGroundVelocity.GetNormalized() * tbMath::Minimum(0.0, lateralGroundVelocity.Magnitude() * 10.0f * kFixedTime);
		mPhysicalVehicle.SetLinearVelocity(linearVelocity);

		const iceScalar topSpeed = tbMath::Convert::MileHourToMeterSecond(65.0f);
		const iceAngle rotationAngle = -tbMath::Clamp(vehicleGroundSpeed / topSpeed, iceScalar(0.0), iceScalar(1.0)) *
			racecarController.GetSteeringPercentage() * iceAngle::Degrees(180.0) * kFixedTime;
		mPhysicalVehicle.SetVehicleToWorld(iceMatrix4::RotationY(rotationAngle) * mPhysicalVehicle.GetVehicleToWorld());
	}

	//NOTE: 2022-08-13: We negate the steering percentage because the steering wheel rotates around the positive up
	//  axis which to turn the wheel right needs a negative value (counter clockwise when looking UP the axis).
	const icePhysics::Angle steeringAngle = GetSteeringAngle(racecarController.GetSteeringPercentage(), vehicleGroundSpeed);
	racecar.SetSteeringAngle(-steeringAngle, 0);
	racecar.SetSteeringAngle(-steeringAngle, 1);

	racecar.SetDragCoefficient(iceScalar(0.0), iceScalar(0.8)); //linear air drag force is applied below.

	const iceScalar fluidDensity = iceScalar(1.225);     //kg/meters^3 (Air Density at Sea Level)
	const iceScalar dragCoefficient = iceScalar(0.37);   //Miata 1999 drag coefficient is 0.37
	const iceScalar frontalArea = iceScalar(1.7113);     //meters^2 Miata NA frontalArea
	const iceScalar speedSquared = rigidBody.GetLinearVelocity().MagnitudeSquared();
	const iceScalar dragForce = iceScalar(0.5) * fluidDensity * speedSquared * dragCoefficient * frontalArea;
	rigidBody.ApplyForce(-rigidBody.GetLinearVelocity().GetNormalized() * dragForce);

	if (true == isOnThrottle && mEngineSpeed < kEngineRevLimiter)
	{
		const iceScalar clampedEngineSpeed = tbMath::Clamp(mEngineSpeed, iceScalar(800), iceScalar(8500));
		TorqueCurve torqueCurve = TorqueCurve::MiataTorqueCurve();

		const iceScalar driftTorque = (true == IsDrifting()) ? iceScalar(2.0) : iceScalar(1.0);
		const iceScalar engineTorque = throttle * torqueCurve.GetOutputTorque(clampedEngineSpeed) * driftTorque;
		const iceScalar wheelTorque = mGearBox.CalculateWheelTorque(engineTorque, GetShifterPosition());
		const iceScalar torquePerWheel(wheelTorque / 2.0); //Nm

		racecar.SetEngineTorque(2, torquePerWheel);
		racecar.SetEngineTorque(3, torquePerWheel);
	}

	if (true == isOnBrake)
	{
		//After turning off air resistance / damping this value would stop the autocross car in ~110ft from ~60mph.
		const iceScalar kMaximumBrakeTorque(4500.0);
		const iceScalar kMaximumBrakeTorquePerWheel(kMaximumBrakeTorque / 4.0);

		racecar.SetBrakeTorque(0, kMaximumBrakeTorquePerWheel);
		racecar.SetBrakeTorque(1, kMaximumBrakeTorquePerWheel);
		racecar.SetBrakeTorque(2, kMaximumBrakeTorquePerWheel);
		racecar.SetBrakeTorque(3, kMaximumBrakeTorquePerWheel);
	}

	const int wheelsOnGround = racecar.CountWheelsOnGround();
	if (wheelsOnGround >= 2)
	{
		const iceScalar vehicleMass = rigidBody.GetMass();
		const iceScalar rollingResistanceCoefficient = 0.02f; //ordinary car tire on new-ish asphalt.
		const iceScalar weight = 10.0f * vehicleMass;
		rigidBody.ApplyForce(-vehicleGroundVelocity.GetNormalized() * weight * rollingResistanceCoefficient);
	}

	if (vehicleGroundSpeed < 0.001f && (false == isOnThrottle))
	{
		iceVector3 stoppedOnGround(iceScalar(0.0), iceScalar(rigidBody.GetLinearVelocity().y), iceScalar(0.0));
		rigidBody.SetLinearVelocity(stoppedOnGround);
	}

	//This keeps the racecar from turning very slightly forever after releasing steering.
	if (rigidBody.GetAngularVelocity().Magnitude() < iceScalar(0.1))
	{
		rigidBody.SetAngularVelocity(icePhysics::Vector3::Zero());
	}
}

//--------------------------------------------------------------------------------------------------------------------//

namespace
{
	icePhysics::FrictionCurve CreateFlatCurve(icePhysics::Scalar value)
	{
		icePhysics::FrictionCurve frictionCurve;
		frictionCurve.mStartValue = value;
		frictionCurve.mExtremeValue = value;
		frictionCurve.mSteadyValue = value;
		frictionCurve.mExtremeSlip = icePhysics::Scalar(0.0);
		frictionCurve.mSteadySlip = icePhysics::Scalar(1.0);
		frictionCurve.mMultiplier = icePhysics::Scalar(1.0);
		return frictionCurve;
	}

	icePhysics::FrictionCurve CreateDippedCurve(void)
	{
		icePhysics::FrictionCurve frictionCurve;
		frictionCurve.mStartValue = icePhysics::Scalar(0.2);
		frictionCurve.mExtremeValue = icePhysics::Scalar(0.01);
		frictionCurve.mSteadyValue = icePhysics::Scalar(0.9);
		frictionCurve.mExtremeSlip = icePhysics::Scalar(0.0005);
		frictionCurve.mSteadySlip = icePhysics::Scalar(0.04);
		frictionCurve.mMultiplier = icePhysics::Scalar(1.0);
		return frictionCurve;
	}

	icePhysics::FrictionCurve CreateFrictionlessCurve(void)
	{
		icePhysics::FrictionCurve frictionCurve;
		frictionCurve.mStartValue = icePhysics::Scalar(0.0);
		frictionCurve.mExtremeValue = icePhysics::Scalar(0.0);
		frictionCurve.mSteadyValue = icePhysics::Scalar(0.0);
		frictionCurve.mExtremeSlip = icePhysics::Scalar(0.0);
		frictionCurve.mSteadySlip = icePhysics::Scalar(0.0);
		frictionCurve.mMultiplier = icePhysics::Scalar(1.0);
		return frictionCurve;
	}
};

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::ExtremeDriftingPhysicsModel::SimulateTireGrip(const RacecarControllerInterface& racecarController)
{
	tb_unused(racecarController);

	if (true == IsDrifting())
	{
		//2024-09-06: Not called for drifty model, trying to do something ... different. Asteroid like drifting with raycast wheels.
		const icePhysics::FrictionCurve frictionlessCurve = CreateFrictionlessCurve();
		for (size_t wheelIndex = 0; wheelIndex < 4; ++wheelIndex)
		{
			mPhysicalVehicle.SetWheelFriction(wheelIndex, frictionlessCurve, frictionlessCurve);
		}
		return;
	}

	if (true)
	{
		const icePhysics::FrictionCurve defaultCurve;
		mPhysicalVehicle.SetWheelFriction(0, defaultCurve, defaultCurve);
		mPhysicalVehicle.SetWheelFriction(1, defaultCurve, defaultCurve);

		icePhysics::FrictionCurve curve = CreateFlatCurve(0.75);
		mPhysicalVehicle.SetWheelFriction(2, curve, curve);
		mPhysicalVehicle.SetWheelFriction(3, curve, curve);
		return;
	}

	//static bool firstEver = true;

	//if (true == firstEver)
	//{
	//	firstEver = false;
	//	std::ofstream frictionData("friction.data.csv");
	//	frictionData << "slipValue,   Frictional Force\n";

	//	icePhysics::FrictionCurve frictionCurve = CreateDippedCurve(); // CreateFlatCurve(0.75);
	//	//for (icePhysics::Scalar slipValue = 0.0; slipValue < 4.0; slipValue += 0.01)
	//	for (icePhysics::Scalar slipValue = 0.0; slipValue < .1 ; slipValue += 0.0001)
	//	{
	//		frictionData << slipValue << ", " << frictionCurve.GetFrictionalForce(slipValue) << "\n";
	//	}
	//}

	//If this chunk gets uncommented, we may conflict/issues with previous = current velocity, see SimulateBodyRoll()
	//const iceVector3 currentVelocity = GetLinearVelocity();
	//const iceVector3 acceleration = (currentVelocity - mPreviousVelocity) / kFixedTime;
	//const iceVector3 accelerationVehicle = GetVehicleToWorld().FastInverse().TransformNormal(acceleration);
	//mPreviousVelocity = currentVelocity;

	////tb_always_log("Acceleration is: { Lat: " << accelerationVehicle.x / 10.0 << " , Long: " << accelerationVehicle.z / 10.0 << " }");

	const iceScalar dot = icePhysics::Vector3::Dot(mPhysicalVehicle.GetGroundVelocity().GetNormalized(), -mPhysicalVehicle.GetObjectToWorld().GetBasis(2));
	//tb_always_log("1 - Dot is: " << icePhysics::Scalar(1.0) - dot << "    steer: " << racecarController.GetSteeringPosition());
	if (dot < iceScalar(0.0) || std::fabs(racecarController.GetSteeringPercentage()) < 0.025f)
	{	//Sliding backwards or in reverse, or not turning!

		icePhysics::FrictionCurve curve = CreateFlatCurve(0.75);
		mPhysicalVehicle.SetWheelFriction(2, curve, curve);
		mPhysicalVehicle.SetWheelFriction(3, curve, curve);
	}
	else
	{
		iceScalar frictionValue = CreateDippedCurve().GetFrictionalForce(iceScalar(1.0) - dot);
		frictionValue = tbMath::Clamp(frictionValue, iceScalar(0.0), iceScalar(1.0));

		icePhysics::FrictionCurve curve = CreateFlatCurve(frictionValue);
		mPhysicalVehicle.SetWheelFriction(2, curve, curve);
		mPhysicalVehicle.SetWheelFriction(3, curve, curve);
	}
}

//--------------------------------------------------------------------------------------------------------------------//
