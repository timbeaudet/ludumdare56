///
/// @file
/// @details Provides the primary controls for the camera that is broadcasting the racing event. This might be chasing
///   another car, fixed points around the track, drones, or attached to the driver helmet. Each individual camera may
///   have controls of its own.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_CameraController_hpp
#define LudumDare56_CameraController_hpp

#include "../game_state/race_session_state.hpp"

#include <ice/express/icex_cameras.hpp>
#include <ice/express/ice_gamepad_interface.hpp>

namespace LudumDare56
{
	namespace GameClient
	{

		class CameraController
		{
		public:
			enum class CameraMode
			{
				kOrbitalCamera,
				kOverheadCamera,
				kChaseCamera,
				kCockpitCamera,
				kFlyingCamera,
				kStaticCamera,
				kDrivingCamera,
				kDomeCamera,
				kDroneCamera,
			};

			CameraController(void);
			~CameraController(void);

			void Simulate(void);
			void Update(iceGraphics::Camera& camera, const float deltaTime);

			void SetToDefaults(const tbMath::Vector3& targetPosition = tbMath::Vector3::Zero(),
				const tbMath::Vector3& cameraPosition = tbMath::Vector3(10.0f, 10.0f, 10.0f));

			inline CameraMode GetCameraMode(void) const { return mCameraMode; }
			inline void SetCameraMode(CameraMode cameraMode) { mCameraMode = cameraMode; }

			GameState::RacecarIndex GetViewedRacecarIndex(void) const;
			void SetViewedRacecarIndex(const GameState::RacecarIndex racecarIndex);

			///
			/// @details This is currently used for the FlyingCamera in meters per seconds, but it is possible other
			///   camera modes make use of the value in different ways in the future.
			///
			inline void SetMovementSpeed(const float movementSpeed) { mMovementSpeed = movementSpeed; }

		private:
			CameraController::CameraMode GetNextCameraMode(const CameraMode cameraMode) const;
			CameraController::CameraMode GetPreviousCameraMode(const CameraMode cameraMode) const;
			GameState::RacecarIndex GetNextRacecar(const GameState::RacecarIndex racecar) const;
			GameState::RacecarIndex GetPreviousRacecar(const GameState::RacecarIndex racecar) const;

			iceGraphics::Camera GetActiveCamera(void) const;
			tbMath::Matrix4 GetViewedTargetToWorld(void) const;
			tbMath::Vector3 GetViewedTargetVelocity(void) const;

			void SimulateCockpitCamera(void);
			void SimulateChaseCamera(void);
			void SimulateOverheadCamera(void);
			void UpdateOrbitalCamera(const float deltaTime);
			void UpdateStaticCamera(const float deltaTime);

			icex::GamepadController mCameraController;
			icex::FlyingCamera mFlyingCamera;
			iceGraphics::Camera mCockpitCamera;
			iceGraphics::Camera mChaseCamera;
			iceGraphics::Camera mOrbitalCamera;
			iceGraphics::Camera mStaticCamera;
			iceGraphics::Camera mOverheadCamera;
			CameraMode mCameraMode;
			GameState::RacecarIndex mViewedRacecarIndex;
			float mMovementSpeed;
			float mOrbitalTimer;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_CameraController_hpp */
