///
/// @file
/// @details Provides the primary controls for the camera that is broadcasting the racing event. This might be chasing
///   another car, fixed points around the track, drones, or attached to the driver helmet. Each individual camera may
///   have controls of its own.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../game_client/camera_controller.hpp"
#include "../game_state/racecar_state.hpp"
#include "../user_settings.hpp"
#include "../logging.hpp"

#include <turtle_brains/math/tb_interpolation.hpp>

//--------------------------------------------------------------------------------------------------------------------//

namespace
{
	struct CameraModifiers
	{
		tbMath::Vector3 offsetFromCar = tbMath::Vector3(0.0f, 1.286f, 3.571f);//CarSpace
		tbMath::Vector3 lookAtOffset = tbMath::Vector3(0.0f, 0.357f, 0.0f);//WorldSpace
		float springEffect = 0.070f;
		float staticCameraDistance = 20.404f;
		float staticCameraHeight = 9.242f;
		float staticCameraTimeMod = 0.189f;
	};

	CameraModifiers theCameraSettings;
};

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::CameraController::CameraController(void) :
	mCameraController(),
	mFlyingCamera(),
	mCockpitCamera(tbMath::Angle::Degrees(TheUserSettings().GetFloat(Settings::FieldOfView()))),
	mChaseCamera(),
	mOrbitalCamera(),
	mStaticCamera(),
	mCameraMode(CameraMode::kOrbitalCamera),
	mViewedRacecarIndex(GameState::InvalidRacecar()),
	mMovementSpeed(100.0f),
	mOrbitalTimer(0.0f)
{
	SetToDefaults();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::CameraController::~CameraController(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::CameraController::Simulate(void)
{
	if (false == GameState::RacecarState::Get(mViewedRacecarIndex).IsRacecarInUse())
	{
		if (CameraMode::kDrivingCamera == mCameraMode)
		{
			mCameraMode = CameraMode::kOverheadCamera;
		}

		mViewedRacecarIndex = GetNextRacecar(mViewedRacecarIndex);
	}

	if (CameraMode::kDrivingCamera == mCameraMode)
	{
		SimulateCockpitCamera();
		SimulateChaseCamera();
	}
	else if (CameraMode::kCockpitCamera == mCameraMode)
	{
		SimulateCockpitCamera();
	}
	else if (CameraMode::kChaseCamera == mCameraMode)
	{
		SimulateChaseCamera();
	}

	SimulateOverheadCamera();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::CameraController::Update(iceGraphics::Camera& camera, const float deltaTime)
{
	const bool isShiftDown = tbGame::Input::IsKeyDown(tbApplication::tbKeyShiftModifier);
	const bool isMiddleMouseDown = tbApplication::Input::IsKeyDown(tbApplication::tbMouseMiddle);

	if (true == tbGame::Input::IsKeyPressed(tbApplication::tbKeyV))
	{
		mCameraMode = (true == isShiftDown) ? GetPreviousCameraMode(mCameraMode) : GetNextCameraMode(mCameraMode);
	}

	if (true == tbGame::Input::IsKeyPressed(tbApplication::tbKeyLeftBracket))
	{
		mViewedRacecarIndex = GetPreviousRacecar(mViewedRacecarIndex);
	}
	if (true == tbGame::Input::IsKeyPressed(tbApplication::tbKeyRightBracket))
	{
		mViewedRacecarIndex = GetNextRacecar(mViewedRacecarIndex);
	}

	if (CameraMode::kOrbitalCamera == mCameraMode || false == GameState::IsValidRacecar(mViewedRacecarIndex))
	{
		UpdateOrbitalCamera(deltaTime);
	}

	if (CameraMode::kStaticCamera == mCameraMode)
	{
		UpdateStaticCamera(deltaTime);
	}

	if (CameraMode::kFlyingCamera == mCameraMode || true == isMiddleMouseDown)
	{
		mCameraController.Update(deltaTime);
		mFlyingCamera.SetMovementSpeed(isShiftDown ? mMovementSpeed : mMovementSpeed * 0.1f);
		mFlyingCamera.Update(deltaTime, mCameraController);
	}

#if defined(development_build)
	camera = (true == isMiddleMouseDown) ? mFlyingCamera : GetActiveCamera();
#else
	camera = GetActiveCamera();
#endif /* development_build */
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::CameraController::SetToDefaults(const tbMath::Vector3& targetPosition, const tbMath::Vector3& cameraPosition)
{
	mOrbitalTimer = 0.0f;
	mFlyingCamera.LookAt(targetPosition, cameraPosition);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarIndex LudumDare56::GameClient::CameraController::GetViewedRacecarIndex(void) const
{
	return mViewedRacecarIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::CameraController::SetViewedRacecarIndex(const GameState::RacecarIndex racecarIndex)
{
	mViewedRacecarIndex = racecarIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::CameraController::CameraMode LudumDare56::GameClient::CameraController::GetNextCameraMode(const CameraMode cameraMode) const
{
	switch (cameraMode)
	{
	case CameraMode::kDrivingCamera: return CameraMode::kDrivingCamera;

	case CameraMode::kOrbitalCamera: return CameraMode::kOverheadCamera;
	case CameraMode::kOverheadCamera: return CameraMode::kChaseCamera;
	case CameraMode::kChaseCamera: return CameraMode::kFlyingCamera;
	case CameraMode::kFlyingCamera: return CameraMode::kOrbitalCamera;
	default: break;
	};

	return CameraMode::kOrbitalCamera;
}


//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::CameraController::CameraMode LudumDare56::GameClient::CameraController::GetPreviousCameraMode(const CameraMode cameraMode) const
{
	switch (cameraMode)
	{
	case CameraMode::kDrivingCamera: return CameraMode::kDrivingCamera;

	case CameraMode::kOrbitalCamera: return CameraMode::kFlyingCamera;
	case CameraMode::kOverheadCamera: return CameraMode::kOrbitalCamera;
	case CameraMode::kChaseCamera: return CameraMode::kOverheadCamera;
	case CameraMode::kFlyingCamera: return CameraMode::kChaseCamera;
	default: break;
	};

	return CameraMode::kOrbitalCamera;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarIndex LudumDare56::GameClient::CameraController::GetNextRacecar(const GameState::RacecarIndex racecarIndex) const
{
	if (CameraMode::kDrivingCamera == mCameraMode) { return racecarIndex; }

	if (false == GameState::IsValidRacecar(racecarIndex))
	{
		for (const GameState::RacecarState& racecar : GameState::RacecarState::AllRacecars())
		{
			if (true == racecar.IsRacecarInUse())
			{
				return racecar.GetRacecarIndex();
			}
		}
	}

	for (size_t nextIndex = size_t(racecarIndex) + 1; nextIndex < racecarIndex + GameState::kNumberOfRacecars; ++nextIndex)
	{
		const GameState::RacecarIndex nextRacecarIndex = tbCore::RangedCast<GameState::RacecarIndex::Integer>(nextIndex % GameState::kNumberOfRacecars);
		if (true == GameState::RacecarState::Get(nextRacecarIndex).IsRacecarInUse())
		{
			return nextRacecarIndex;
		}
	}

	if (true == GameState::IsValidRacecar(racecarIndex) && true == GameState::RacecarState::Get(racecarIndex).IsRacecarInUse())
	{
		return racecarIndex;
	}

	return GameState::InvalidRacecar();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarIndex LudumDare56::GameClient::CameraController::GetPreviousRacecar(const GameState::RacecarIndex racecarIndex) const
{
	if (CameraMode::kDrivingCamera == mCameraMode) { return racecarIndex; }

	for (size_t previousIndex = size_t(racecarIndex) + GameState::kNumberOfRacecars - 1; previousIndex > racecarIndex; --previousIndex)
	{
		const GameState::RacecarIndex previousRacecarIndex = tbCore::RangedCast<GameState::RacecarIndex::Integer>(previousIndex % GameState::kNumberOfRacecars);
		if (true == GameState::RacecarState::Get(previousRacecarIndex).IsRacecarInUse())
		{
			return previousRacecarIndex;
		}
	}

	if (true == GameState::IsValidRacecar(racecarIndex) && true == GameState::RacecarState::Get(racecarIndex).IsRacecarInUse())
	{
		return racecarIndex;
	}

	return GameState::InvalidRacecar();
}

//--------------------------------------------------------------------------------------------------------------------//

iceGraphics::Camera LudumDare56::GameClient::CameraController::GetActiveCamera(void) const
{
	if (false == GameState::IsValidRacecar(mViewedRacecarIndex) && CameraMode::kFlyingCamera != mCameraMode)
	{
		return mOrbitalCamera;
	}

	// Warning: There is technically a bit of slicing that happens here, as the active camera is likely a subclass with
	//   its own properties and that will get sliced. But the important bits should remain to allow the base camera to
	//   work as expected. At least there has been no problems with this since ~2018 and should be no problems as long as:
	//       - no virtual functions are added to iceCamera().
	//       - each sub camera has transformed the base members as needed for viewing.
	//       - you don't feed the gremlins, definitely not after midnight!
	switch (mCameraMode)
	{
	default: break;
	case CameraMode::kCockpitCamera: return mCockpitCamera;
	case CameraMode::kOrbitalCamera: return mOrbitalCamera;
	case CameraMode::kFlyingCamera: return mFlyingCamera;
	case CameraMode::kChaseCamera: return mChaseCamera;
	case CameraMode::kOverheadCamera: return mOverheadCamera;

	//case CameraMode::kDrivingCamera: return mCockpitCamera;
	//case CameraMode::kDrivingCamera: return mOverheadCamera;
	case CameraMode::kDrivingCamera: return mChaseCamera;

	case CameraMode::kStaticCamera: return mStaticCamera;
	};

	tb_error("Expected to find a camera from the camera modes...");
	return mOrbitalCamera;
}

//--------------------------------------------------------------------------------------------------------------------//

tbMath::Matrix4 LudumDare56::GameClient::CameraController::GetViewedTargetToWorld(void) const
{
	const GameState::RacecarIndex viewedRacecarIndex = GetViewedRacecarIndex();
	if (GameState::InvalidRacecar() == viewedRacecarIndex)
	{
		return tbMath::Matrix4::Identity();
	}

	return tbMath::Matrix4(GameState::RacecarState::Get(viewedRacecarIndex).GetVehicleToWorld());
}

//--------------------------------------------------------------------------------------------------------------------//

tbMath::Vector3 LudumDare56::GameClient::CameraController::GetViewedTargetVelocity(void) const
{
	const GameState::RacecarIndex viewedRacecarIndex = GetViewedRacecarIndex();
	if (GameState::InvalidRacecar() == viewedRacecarIndex)
	{
		return tbMath::Vector3::Zero();
	}

	return GameState::RacecarState::Get(viewedRacecarIndex).GetLinearVelocity();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::CameraController::SimulateCockpitCamera(void)
{
	const tbMath::Matrix4 vehicleToWorld = GetViewedTargetToWorld();
	const tbMath::Matrix4 headToVehicle = tbMath::Matrix4::Translation(-0.3736f, 0.41179f, 0.49682f);
	const tbMath::Matrix4 headToWorld = headToVehicle * vehicleToWorld;

	mCockpitCamera.LookAt(headToWorld.GetPosition() - headToWorld.GetBasis(2), headToWorld.GetPosition());
	mCockpitCamera.SetFieldOfView(tbMath::Angle::Degrees(TheUserSettings().GetFloat(Settings::FieldOfView())));
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::CameraController::SimulateChaseCamera(void)
{
	const tbMath::Matrix4 targetToWorld = GetViewedTargetToWorld();
	const tbMath::Vector3 targetLinearVelocity = GetViewedTargetVelocity();
	const float speedMPH = tbMath::Convert::MeterSecondToMileHour(tbMath::Vector2(targetLinearVelocity.x, targetLinearVelocity.z).Magnitude());
	const float speedPercentage = tbMath::Clamp(speedMPH / 80.0f, 0.0f, 1.0f);

	const tbMath::Vector3 flattenedForward(tbMath::Vector3(-targetToWorld.GetBasis(2).x, 0.0f, -targetToWorld.GetBasis(2).z).GetNormalized());
	const tbMath::Vector3 flattenedVelocity(tbMath::Vector3(targetLinearVelocity.x, 0.0f, targetLinearVelocity.z).GetNormalized());
	const tbMath::Vector3 flattenedDirection((speedMPH < 2.0f) ? flattenedForward : flattenedVelocity);
	const tbMath::Vector3 targetPosition = targetToWorld.GetPosition() - flattenedDirection * theCameraSettings.offsetFromCar.z;
	const tbMath::Vector3 worldBehindAboveKart = targetPosition + tbMath::Vector3(0.0f, theCameraSettings.offsetFromCar.y, 0.0f);

	const tbMath::Vector3 cameraPosition = ((worldBehindAboveKart - mChaseCamera.GetPosition()) * theCameraSettings.springEffect) + mChaseCamera.GetPosition();
	mChaseCamera.LookAt(targetToWorld.GetPosition() + theCameraSettings.lookAtOffset, cameraPosition);
	mChaseCamera.SetFieldOfView(90.0_degrees * (1.0f + 0.2222f * tbMath::Interpolation::SmoothStep(speedPercentage)));
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::CameraController::SimulateOverheadCamera(void)
{
	const tbMath::Vector3 cameraOffsetFromCar = tbMath::Vector3(0.0f, 12.0f, -10.0f);//CarSpace

	const tbMath::Matrix4 targetToWorld = GetViewedTargetToWorld();
	const tbMath::Vector3 targetLinearVelocity = GetViewedTargetVelocity();
	const float speedMPH = tbMath::Convert::MeterSecondToMileHour(tbMath::Vector2(targetLinearVelocity.x, targetLinearVelocity.z).Magnitude());
	const float speedPercentage = tbMath::Clamp(speedMPH / 80.0f, 0.0f, 1.0f);

	const tbMath::Vector3 targetPosition = targetToWorld.GetPosition() + cameraOffsetFromCar * (1.0f + speedPercentage);
	//const tbMath::Vector3 cameraPosition = (targetPosition - mOverheadCamera.GetPosition()) * cameraSpringEffect + mOverheadCamera.GetPosition();
	const tbMath::Vector3 cameraPosition = targetPosition;

	//const float cameraSpringEffect = 0.070f;
	//const tbMath::Vector3 flattenedVelocity(tbMath::Vector3(targetLinearVelocity.x, 0.0f, targetLinearVelocity.z).GetNormalized());
	//mOverheadCamera.LookAt(targetToWorld.GetPosition() + flattenedVelocity * 5.0f * speedPercentage, cameraPosition);

	mOverheadCamera.LookAt(targetToWorld.GetPosition(), cameraPosition);
	mOverheadCamera.SetFieldOfView(90.0_degrees * (1.0f + 0.2222f * tbMath::Interpolation::SmoothStep(speedPercentage)));
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::CameraController::UpdateOrbitalCamera(const float deltaTime)
{
	mOrbitalTimer += tbMath::Convert::DegreesToRadians(4.0f) * deltaTime;

	const float orbitRadius = 70.0f;
	const tbMath::Vector3 cameraPosition(sin(mOrbitalTimer) * orbitRadius, 45.0f, cos(mOrbitalTimer) * orbitRadius);
	mOrbitalCamera.LookAt(tbMath::Vector3::Zero(), cameraPosition);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::CameraController::UpdateStaticCamera(const float /*deltaTime*/)
{
	const tbMath::Matrix4 targetToWorld = GetViewedTargetToWorld();
	const tbMath::Vector3 cameraPosition(0.0f, 45.0f, 0.0f);
	mStaticCamera.LookAt(targetToWorld.GetPosition(), cameraPosition);
}

//--------------------------------------------------------------------------------------------------------------------//
