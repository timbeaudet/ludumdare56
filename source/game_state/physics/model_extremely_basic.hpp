///
/// @file
/// @details The most basic of all physics models separated out from the racecars.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_PhysicsModelExtremelyBasic_hpp
#define LudumDare56_PhysicsModelExtremelyBasic_hpp

#include "../../game_state/physics/physics_model_interface.hpp"
#include "../../game_state/physics/vehicle_gear_box.hpp"

#include <array>

namespace LudumDare56::GameState::PhysicsModels
{

	class ExtremelyBasicsPhysicsModel : public RaycastVehiclePhysicsModelInterface
	{
	public:
		ExtremelyBasicsPhysicsModel(icePhysics::World& physicalWorld);
		virtual ~ExtremelyBasicsPhysicsModel(void);

		inline virtual icePhysics::Scalar GetEngineSpeed(void) const override { return mEngineSpeed; }
		inline virtual Gear GetShifterPosition(void) const override { return mGearBox.mCurrentGear; }

	protected:
		virtual void OnResetRacecarForces(void) override;
		virtual void OnSimulate(const RacecarControllerInterface& racecarController) override;
		virtual void OnDebugRender(void) const override;

	private:
		static iceAngle GetSteeringAngle(const iceScalar steeringInput, const iceScalar vehicleGroundSpeed);

		void SimulateFizzics(const RacecarControllerInterface& racecarController);
		void SimulateGearBox(const RacecarControllerInterface& racecarController);
		void SimulateTireGrip(const RacecarControllerInterface& racecarController);

		icePhysics::Scalar mEngineSpeed;
		icePhysics::Vector3 mPreviousVelocity;
		VehicleGearBox mGearBox;
	};

};	//namespace LudumDare56::GameState::PhysicsModels

#endif /* LudumDare56_PhysicsModelExtremelyBasic_hpp */
