///
/// @file
/// @details The most basic of all physics models separated out from the racecars.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_PhysicsModelExtremeDrifting_hpp
#define LudumDare56_PhysicsModelExtremeDrifting_hpp

#include "../../game_state/physics/physics_model_interface.hpp"
#include "../../game_state/physics/vehicle_gear_box.hpp"
#include "../../game_state/physics/vehicle_body_tilter.hpp"

#include <array>

namespace LudumDare56::GameState::PhysicsModels
{

	class ExtremeDriftingPhysicsModel : public RaycastVehiclePhysicsModelInterface
	{
	public:
		ExtremeDriftingPhysicsModel(icePhysics::World& physicalWorld);
		virtual ~ExtremeDriftingPhysicsModel(void);

		inline virtual iceMatrix4 GetBodyToWorld(void) const override { return mBodyTilter.GetBodyToVehicle() * mPhysicalVehicle.GetVehicleToWorld(); }

		inline virtual iceScalar GetEngineSpeed(void) const override { return mEngineSpeed; }
		inline virtual Gear GetShifterPosition(void) const override { return mGearBox.mCurrentGear; }

	protected:
		bool IsDrifting(void) const { return (true == mIsHandbrakePulled || false == mDriftEndedTimer.IsZero()); }
		virtual void OnResetRacecarForces(void) override;
		virtual void OnSimulate(const RacecarControllerInterface& racecarController) override;
		virtual void OnDebugRender(void) const override;

	private:
		static iceAngle GetSteeringAngle(const iceScalar steeringInput, const iceScalar vehicleGroundSpeed);
		iceAngle CalculateDriftAngle(void) const;

		void SimulateFizzics(const RacecarControllerInterface& racecarController);
		void SimulateTireGrip(const RacecarControllerInterface& racecarController);

		iceScalar mEngineSpeed;

		iceVector3 mPreviousVelocity;

		VehicleGearBox mGearBox;
		VehicleBodyTilter mBodyTilter;

		tbGame::GameTimer mDriftEndedTimer;
		bool mIsHandbrakePulled;
	};

};	//namespace LudumDare56::GameState::PhysicsModels

#endif /* LudumDare56_PhysicsModelExtremeDrifting_hpp */
