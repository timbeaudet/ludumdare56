///
/// @file
/// @details A racecar controller for an artifical driver to control the car.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_state/ai/artificial_driver_controller.hpp"
#include "../../game_state/racecar_state.hpp"
#include "../../game_state/driver_state.hpp"
#include "../../game_state/racetrack_state.hpp"

namespace
{
#if !defined(ludumdare56_headless_build)
	iceGraphics::Visualization* theVisualizer = nullptr;
	iceGraphics::Visualization::Color kDebugColorTarget = 0xFFffd966;
	iceGraphics::Visualization::Color kDebugSteering = 0xFFFFFFFF;
	iceGraphics::Visualization::Color kDebugThrottle = 0xFF00FF00;
	iceGraphics::Visualization::Color kDebugBrake = 0xFFFF0000;
#endif /* !ludumdare56_headless_build */

	LudumDare56::Vector2 Flatten(const LudumDare56::Vector3& input)
	{
		return LudumDare56::Vector2(input.x, input.z);
	}
};

#if !defined(ludumdare56_headless_build)
void LudumDare56::GameState::ArtificialDriverController::SetDebugVisualizer(iceGraphics::Visualization* visualizer)
{
	theVisualizer = visualizer;
}
#endif /* !ludumdare56_headless_build */

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ArtificialDriverController::ArtificialDriverController(const DriverIndex& driverIndex, const RacecarIndex& racecarIndex) :
	RacecarControllerInterface(),
	mDriver(DriverState::Get(driverIndex)),
	mRacecar(RacecarState::Get(racecarIndex))
{
	ResetControls();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ArtificialDriverController::~ArtificialDriverController(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::ArtificialDriverController::OnUpdateControls(void)
{
	/// @note 2023-10-19: targetNodeIndex is assuming the racetrack is a looping circuit and won't work terrible well
	///   for a point-to-point type track, unless the race were to finish before it would possible loop around.
	const TrackNodeIndex closestNodeIndex = FindClosestTrackNode();
	//const TrackNodeIndex targetNodeIndex = (closestNodeIndex + 3) % RacetrackState::GetNumberOfTrackNodes();
	const TrackNodeIndex targetNodeIndex = (closestNodeIndex + static_cast<TrackNodeIndex>(1)) % RacetrackState::GetNumberOfTrackNodes();

	const Vector3 targetPosition = RacetrackState::GetTrackNodeLeadingEdge(targetNodeIndex, TrackEdge::kCenter);

	const Vector2 flatTargetPosition = Flatten(targetPosition);
	const Vector2 flatRacecarPosition = Flatten(mRacecar.GetVehicleToWorld().GetPosition());
	const Vector2 flatRacecarRight = Flatten(mRacecar.GetVehicleToWorld().GetBasis(0));
	const Vector2 flatRacecarForward = Flatten(-mRacecar.GetVehicleToWorld().GetBasis(2));

	const Vector2 directionToTarget = (flatTargetPosition - flatRacecarPosition).GetNormalized();
	const float steeringDot = Vector2::Dot(flatRacecarRight, directionToTarget);
	SetSteeringPercentage(steeringDot * 2.0f);

	SetThrottlePercentage(1.0f * (1.0f - std::fabs(steeringDot * 2.0f)));
	SetBrakePercentage(0.0f);

	if (mRacecar.GetLinearVelocity().Magnitude() > 10.0f)
	{
		SetBrakePercentage(std::fabs(steeringDot * 2.0f) - 1.0f);
	}
	else
	{	//Apply minimum throttle to ensure we get up to speed.
		SetThrottlePercentage(0.5f);
		SetBrakePercentage(0.0f);
	}


	if (Vector2::Dot(flatRacecarForward, directionToTarget) < 0.0f)
	{
		const float sign = std::signbit(Vector2::Dot(flatRacecarRight, directionToTarget)) ? -1.0f : 1.0f;
		SetSteeringPercentage(sign);

		if (mRacecar.GetLinearVelocity().Magnitude() > 20.0f)
		{
			SetBrakePercentage(0.8f);
		}
	}

#if !defined(ludumdare56_headless_build)
	if (nullptr != theVisualizer)
	{
		const Vector3 racecarPosition = mRacecar.GetVehicleToWorld().GetPosition();
		const Vector3 racecarRight = mRacecar.GetVehicleToWorld().GetBasis(0);
		const Vector3 racecarForward = -mRacecar.GetVehicleToWorld().GetBasis(2);

		theVisualizer->VisualizeLine(racecarPosition, targetPosition, kDebugColorTarget);

		const Vector3 frontOfCar = racecarPosition + racecarForward * 2.0f;
		theVisualizer->VisualizeLine(frontOfCar - racecarRight, frontOfCar + racecarRight, kDebugSteering);

		theVisualizer->VisualizeCircle(frontOfCar + racecarRight * GetSteeringPercentage(), 0.05f, kDebugSteering, 0);
		theVisualizer->VisualizeCircle(frontOfCar + racecarRight * GetSteeringPercentage(), 0.05f, kDebugSteering, 1);
		theVisualizer->VisualizeCircle(frontOfCar + racecarRight * GetSteeringPercentage(), 0.05f, kDebugSteering, 2);
		theVisualizer->VisualizeCircle(frontOfCar + racecarRight * GetSteeringPercentage(), 0.075f, kDebugSteering, 0);
		theVisualizer->VisualizeCircle(frontOfCar + racecarRight * GetSteeringPercentage(), 0.075f, kDebugSteering, 1);
		theVisualizer->VisualizeCircle(frontOfCar + racecarRight * GetSteeringPercentage(), 0.075f, kDebugSteering, 2);

		theVisualizer->VisualizeLine(frontOfCar + racecarRight, frontOfCar + racecarRight + Up() * GetThrottlePercentage(), kDebugThrottle);
		theVisualizer->VisualizeLine(frontOfCar - racecarRight, frontOfCar - racecarRight + Up() * GetBrakePercentage(), kDebugBrake);
	}
#endif /* ludumdare56_headless_build */
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ArtificialDriverController::TrackNodeIndex LudumDare56::GameState::ArtificialDriverController::FindClosestTrackNode(void)
{
	const Vector3 racecarPosition = static_cast<Vector3>(mRacecar.GetVehicleToWorld().GetPosition());

	TrackNodeIndex closestNodeIndex = 0;
	float closestDistanceSquared = -1.0f;

	for (TrackNodeIndex trackNodeIndex = 0; trackNodeIndex < RacetrackState::GetNumberOfTrackNodes(); ++trackNodeIndex)
	{
		const Vector3& nodeEdgeCenter = RacetrackState::GetTrackNodeLeadingEdge(trackNodeIndex, TrackEdge::kCenter);
		const float distanceSquared = (racecarPosition - nodeEdgeCenter).MagnitudeSquared();
		if (closestDistanceSquared < 0.0f || distanceSquared < closestDistanceSquared)
		{
			closestNodeIndex = trackNodeIndex;
			closestDistanceSquared = distanceSquared;
		}
	}

	return closestNodeIndex;
}

//--------------------------------------------------------------------------------------------------------------------//
