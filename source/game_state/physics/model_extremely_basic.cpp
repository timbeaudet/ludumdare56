///
/// @file
/// @details The most basic of all physics models separated out from the racecars.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_state/physics/model_extremely_basic.hpp"
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
		// This use to be in the VehicleInfo but now moved to spring, I don't quite trust the comments but also don't
		//   recall making changes to springStrength either...
		//vehicle.springStrength = 80.0f; //200.0f;  //Gets multipled by car mass for ease of tuning, not exactly springRate/Force.
		//vehicle.springDamper = 400.0f; //2.0f; //Damping is proportional to the spring strength in this model.

		icePhysics::Spring spring;
		spring.mStrength = 10000.0f;
		spring.mDamper = 600.0f;
		return spring;
	}

};

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::PhysicsModels::ExtremelyBasicsPhysicsModel::ExtremelyBasicsPhysicsModel(icePhysics::World& physicalWorld) :
	RaycastVehiclePhysicsModelInterface(physicalWorld, DefaultVehicle()),
	mEngineSpeed(0.0),
	mGearBox(Gear::Third)
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

LudumDare56::GameState::PhysicsModels::ExtremelyBasicsPhysicsModel::~ExtremelyBasicsPhysicsModel(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::ExtremelyBasicsPhysicsModel::OnResetRacecarForces(void)
{
	RaycastVehiclePhysicsModelInterface::OnResetRacecarForces();

	mEngineSpeed = iceScalar(0.0);
	mGearBox.Reset();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::ExtremelyBasicsPhysicsModel::OnSimulate(const RacecarControllerInterface& racecarController)
{
	RaycastVehiclePhysicsModelInterface::OnSimulate(racecarController);

	mEngineSpeed = mGearBox.SimulateGearBox(mEngineSpeed, kWheelRadius, *this, racecarController);
	SimulateTireGrip(racecarController);
	SimulateFizzics(racecarController);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::ExtremelyBasicsPhysicsModel::OnDebugRender(void) const
{
	RaycastVehiclePhysicsModelInterface::OnDebugRender();
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Angle LudumDare56::GameState::PhysicsModels::ExtremelyBasicsPhysicsModel::GetSteeringAngle(
	const iceScalar steeringInput, const iceScalar vehicleGroundSpeed)
{	//TODO: LudumDare56: Physics: This is definitely NOT a good way to figure out the steering angle, based off Rally of Rockets.
	const iceScalar minimumSpeed(5.0); //m/s
	const iceScalar maximumSpeed(20.0); //m/s

	const iceScalar speedPercentage = tbMath::Clamp((vehicleGroundSpeed - minimumSpeed) / (maximumSpeed - minimumSpeed), iceScalar(0.0), iceScalar(1.0));
	const iceAngle maximumAngle = tbMath::Interpolation::Linear(speedPercentage, kMaximumTurnAngle, kMaximumTurnAngle / iceScalar(4.0));
	return steeringInput * maximumAngle;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::PhysicsModels::ExtremelyBasicsPhysicsModel::SimulateFizzics(const RacecarControllerInterface& racecarController)
{
	icePhysics::RaycastVehicle& racecar = mPhysicalVehicle;

	//const Vector3 vehicleForwardDirection = -mPhysicalVehicle.GetObjectToWorld().GetBasis(2);
	//const Scalar forwardSpeed = Vector3::Dot(vehicleForwardDirection, GetLinearVelocity());

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

	const iceScalar vehicleMass = rigidBody.GetMass();

	const bool isOnThrottle = (throttle > 0.001f);
	const bool isOnBrake = (brake > 0.001f);
	//const bool handbrake = racecarController.GetHandbrakeValue() > 10;


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

	//Attempting to setup more slippery physics... at least on the rear end - absolutely uncontrollable; use CreateRearTyreCurve().
	for (size_t wheelIndex = 0; wheelIndex < 4; ++wheelIndex)
	{
		//racecar.SetWheelFriction(wheelIndex, (wheelIndex < 2) ? theFrontTyreCurve : theRearTyreCurve, icePhysics::FrictionCurve());
		//racecar.SetWheelFriction(wheelIndex, icePhysics::FrictionCurve(), icePhysics::FrictionCurve());

		racecar.SetEngineTorque(wheelIndex, iceScalar(0.0));
		racecar.SetBrakeTorque(wheelIndex, iceScalar(0.0));
	}

	const int wheelsOnGround = racecar.CountWheelsOnGround();

	if (wheelsOnGround >= 2 && true == isOnThrottle)
	{
		if (mEngineSpeed < kEngineRevLimiter)
		{
			const iceScalar clampedEngineSpeed = tbMath::Clamp(mEngineSpeed, iceScalar(800), iceScalar(8500));
			TorqueCurve torqueCurve = TorqueCurve::MiataTorqueCurve();

			const iceScalar engineTorque = throttle * torqueCurve.GetOutputTorque(clampedEngineSpeed);
			const iceScalar wheelTorque = mGearBox.CalculateWheelTorque(engineTorque, GetShifterPosition());
			const iceScalar torquePerWheel(wheelTorque / 2.0); //Nm

			racecar.SetEngineTorque(2, torquePerWheel);
			racecar.SetEngineTorque(3, torquePerWheel);
		}
	}

	if (wheelsOnGround >= 2 && true == isOnBrake)
	{
		//After turning off air resistance / damping this value would stop the autocross car in ~110ft from ~60mph.
		const iceScalar kMaximumBrakeTorque(4500.0);
		const iceScalar kMaximumBrakeTorquePerWheel(kMaximumBrakeTorque / 4.0);

		racecar.SetBrakeTorque(0, kMaximumBrakeTorquePerWheel);
		racecar.SetBrakeTorque(1, kMaximumBrakeTorquePerWheel);
		racecar.SetBrakeTorque(2, kMaximumBrakeTorquePerWheel);
		racecar.SetBrakeTorque(3, kMaximumBrakeTorquePerWheel);
	}

	if (wheelsOnGround >= 2)
	{
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

void LudumDare56::GameState::PhysicsModels::ExtremelyBasicsPhysicsModel::SimulateTireGrip(const RacecarControllerInterface& racecarController)
{
	if (true)
	{
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


	const iceVector3 currentVelocity = GetLinearVelocity();
	const iceVector3 acceleration = (currentVelocity - mPreviousVelocity) / kFixedTime;
	const iceVector3 accelerationVehicle = GetVehicleToWorld().FastInverse().TransformNormal(acceleration);
	mPreviousVelocity = currentVelocity;

	//tb_always_log("Acceleration is: { Lat: " << accelerationVehicle.x / 10.0 << " , Long: " << accelerationVehicle.z / 10.0 << " }");

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
