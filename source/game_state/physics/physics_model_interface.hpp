///
/// @file
/// @details Defines the API expected from all racecar physics models.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_PhysicsModelInterface_hpp
#define LudumDare56_PhysicsModelInterface_hpp

//#include "../../game_state/race_session_state.hpp" //for RacecarIndex etc.

#include "../../ludumdare56.hpp"
#include "../../game_state/racecar_controller_interface.hpp" //for Gear

#include <turtle_brains/math/tb_vector.hpp>
#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_typed_integer.hpp>

#include <ice/physics/ice_physical_world.hpp>
#include <ice/physics/ice_physical_vehicle.hpp>

#include <array>

namespace LudumDare56::GameState::PhysicsModels
{
	class PhysicsModelInterface;

	typedef std::unique_ptr<PhysicsModelInterface> PhysicsModelInterfacePtr;

	enum class PhysicsModel { NullModel, ExtremelyBasic, ExtremelyFast, ExtremeDrifting };

	class PhysicsModelInterface : public tbCore::Noncopyable
	{
	public:
		PhysicsModelInterface(void);
		virtual ~PhysicsModelInterface(void);

		/// @note This isn't named terribly well, it was required so the RaycastVehicle (or other physics bodies?) could
		///   be added to and removed from the PhysicalWorld when enabled/disabled. But PhysicsModel's don't actually
		///   (strictly) know about a PhysicalWorld so naming these OnAddToWorld/Remove etc, is a little odd too.
		void SetEnabled(bool isEnabled);
		void ResetRacecarForces(void);
		void Simulate(const RacecarControllerInterface& racecarController);
		void DebugRender(void);

		virtual iceMatrix4 GetVehicleToWorld(void) const = 0;
		virtual void SetVehicleToWorld(const iceMatrix4& vehicleToWorld) = 0;
		virtual iceMatrix4 GetBodyToWorld(void) const = 0;
		virtual iceMatrix4 GetWheelToWorld(const size_t wheelIndex) const = 0;

		virtual iceVector3 GetAngularVelocity(void) const = 0;
		virtual void SetAngularVelocity(const iceVector3& angularVelocity) = 0;

		virtual iceVector3 GetLinearVelocity(void) const = 0;
		virtual void SetLinearVelocity(const iceVector3& linearVelocity) = 0;

		/// @details Expected to return the revolutions-per-minute.
		virtual iceScalar GetEngineSpeed(void) const = 0;

		/// @details Expected to return the current gear the car is in.
		/// @note This is somewhat more racecar state releated, but physics models change shift points, or could even
		///   ignore the gear stick altogether.
		virtual Gear GetShifterPosition(void) const = 0;

	protected:
		virtual void OnSetEnabled(bool /*isEnabled*/) { }
		virtual void OnResetRacecarForces(void) = 0;
		virtual void OnSimulate(const RacecarControllerInterface& racecarController) = 0;
		virtual void OnDebugRender(void) const = 0;
	};

	PhysicsModelInterfacePtr Instantiate(icePhysics::World& physicalWorld, const PhysicsModel physicsModel);



	class NullPhysicsModel : public PhysicsModelInterface
	{
	public:
		NullPhysicsModel(void) : PhysicsModelInterface()
		{
		}

		inline virtual ~NullPhysicsModel(void) { }

		virtual iceMatrix4 GetVehicleToWorld(void) const override;
		virtual void SetVehicleToWorld(const iceMatrix4& vehicleToWorld) override;
		virtual iceMatrix4 GetBodyToWorld(void) const override;
		virtual iceMatrix4 GetWheelToWorld(const size_t wheelIndex) const override;

		virtual iceVector3 GetAngularVelocity(void) const override;
		virtual void SetAngularVelocity(const iceVector3& angularVelocity) override;
		virtual iceVector3 GetLinearVelocity(void) const override;
		virtual void SetLinearVelocity(const iceVector3& linearVelocity) override;

		inline virtual iceScalar GetEngineSpeed(void) const override { return iceScalar(0.0); }
		inline virtual Gear GetShifterPosition(void) const override { return Gear::Neutral; }

	protected:
		inline virtual void OnResetRacecarForces(void) override { }
		inline virtual void OnSimulate(const RacecarControllerInterface& /*racecarController*/) override { }
		inline virtual void OnDebugRender(void) const override { }
	};



	class RaycastVehiclePhysicsModelInterface : public PhysicsModelInterface
	{
	public:
		RaycastVehiclePhysicsModelInterface(icePhysics::World& physicalWorld, const icePhysics::VehicleInfo& vehicleInfo);
		virtual ~RaycastVehiclePhysicsModelInterface(void);

		virtual iceMatrix4 GetVehicleToWorld(void) const override;
		virtual void SetVehicleToWorld(const iceMatrix4& vehicleToWorld) override;
		virtual iceMatrix4 GetBodyToWorld(void) const override;
		virtual iceMatrix4 GetWheelToWorld(const size_t wheelIndex) const override;

		virtual iceVector3 GetAngularVelocity(void) const override;
		virtual void SetAngularVelocity(const iceVector3& angularVelocity) override;
		virtual iceVector3 GetLinearVelocity(void) const override;
		virtual void SetLinearVelocity(const iceVector3& linearVelocity) override;

	protected:
		virtual void OnSetEnabled(bool isEnabled) override;
		virtual void OnResetRacecarForces(void);
		virtual void OnSimulate(const RacecarControllerInterface& racecarController);
		virtual void OnDebugRender(void) const override;

		icePhysics::World& mPhysicalWorld;
		icePhysics::RaycastVehicle mPhysicalVehicle;
	};


};	//namespace LudumDare56::GameState::PhysicsModels

#endif /* LudumDare56_PhysicsModelInterface_hpp */
