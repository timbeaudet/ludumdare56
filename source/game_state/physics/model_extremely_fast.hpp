///
/// @file
/// @details The most basic of all physics models separated out from the racecars.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_PhysicsModelExtremelyFast_hpp
#define LudumDare56_PhysicsModelExtremelyFast_hpp

#include "../../game_state/physics/physics_model_interface.hpp"
#include "../../game_state/physics/vehicle_gear_box.hpp"
#include "../../game_state/physics/vehicle_body_tilter.hpp"

#include <array>

namespace LudumDare56::GameState::PhysicsModels
{

	class ExtremelyFastPhysicsModel : public RaycastVehiclePhysicsModelInterface
	{
	public:
		ExtremelyFastPhysicsModel(icePhysics::World& physicalWorld);
		virtual ~ExtremelyFastPhysicsModel(void);

		inline virtual iceMatrix4 GetBodyToWorld(void) const override { return mBodyTilter.GetBodyToVehicle() * mPhysicalVehicle.GetVehicleToWorld(); }

		inline virtual iceScalar GetEngineSpeed(void) const override { return mEngineSpeed; }
		inline virtual Gear GetShifterPosition(void) const override { return mGearBox.mCurrentGear; }

	protected:
		virtual void OnResetRacecarForces(void) override;
		virtual void OnSimulate(const RacecarControllerInterface& racecarController) override;
		virtual void OnDebugRender(void) const override;

	private:
		static iceAngle GetSteeringAngle(const iceScalar steeringInput, const iceScalar vehicleGroundSpeed);

		void SimulateFizzics(const RacecarControllerInterface& racecarController);
		void SimulateTireGrip(const RacecarControllerInterface& racecarController);

		iceScalar mEngineSpeed;

		iceVector3 mPreviousVelocity;

		VehicleGearBox mGearBox;
		VehicleBodyTilter mBodyTilter;
	};

};	//namespace LudumDare56::GameState::PhysicsModels

#endif /* LudumDare56_PhysicsModelExtremelyFast_hpp */
