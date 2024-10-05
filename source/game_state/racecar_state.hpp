///
/// @file
/// @details Manage all the racecars in the simulation on both game and server side.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RacecarManager_hpp
#define LudumDare56_RacecarManager_hpp

#include "../game_state/physics/physics_model_interface.hpp"
#include "../game_state/racecar_controller_interface.hpp"
#include "../game_state/race_session_state.hpp" //for RacecarIndex etc.

#include "../core/typed_range.hpp"
#include "../core/event_system.hpp"

#include <turtle_brains/math/tb_vector.hpp>
#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_typed_integer.hpp>

#include <ice/physics/ice_physical_world.hpp>
#include <ice/physics/ice_physical_vehicle.hpp>

#include <array>

class RacecarControllerInterface;

namespace LudumDare56::GameState
{
	namespace PhysicsModels
	{
		class PhysicsModelInterface;
	};

	class RacecarState : public TyreBytes::Core::EventBroadcaster
	{
	public:
		enum class CreatureIndexType : tbCore::uint8 { };
		typedef tbCore::TypedInteger<CreatureIndexType> CreatureIndex;

		static constexpr CreatureIndex::Integer kNumberOfCreatures = 200;
		static constexpr CreatureIndex::Integer kMinimumCreatures = 0;

		constexpr CreatureIndex InvalidCreature(void) { return CreatureIndex::Integer(~0); }
		constexpr bool IsValidCreature(const CreatureIndex creatureIndex) { return creatureIndex < kNumberOfCreatures; }

		class Creature
		{
		public:
			iceMatrix4 mCreatureToWorld;
			iceVector3 mPreviousPosition;
			iceVector3 mVelocity;
			bool mIsAlive;
			bool mIsOnTrack;
			bool mIsRacing;

			explicit Creature(const iceMatrix4& creatureToWorld = iceMatrix4::Identity()) :
				mCreatureToWorld(creatureToWorld),
				mPreviousPosition(iceVector3::Zero()),
				mVelocity(iceVector3::Zero()),
				mIsAlive(true),
				mIsOnTrack(true),
				mIsRacing(true)
			{
			}

			void Move(const iceVector3& targetPosition, const iceScalar targetSpeed, const iceVector3& alignment,
				const iceVector3& cohesion, const iceVector3& separation);
		};

		static const RacecarState& Get(const RacecarIndex racecarIndex);
		static RacecarState& GetMutable(const RacecarIndex racecarIndex);

		const Creature& GetCreature(const CreatureIndex creatureIndex) { return mCreatures[creatureIndex]; }
		Creature& GetMutableCreature(const CreatureIndex creatureIndex) { return mCreatures[creatureIndex]; }

		///
		/// @note This is purely for some syntactical sugars of using ranged for-loops;
		///
		typedef TyreBytes::Core::TypedRange<RacecarIndex, const RacecarState, kNumberOfRacecars, RacecarState::Get> RacecarContainerAccessor;
		typedef TyreBytes::Core::TypedRange<RacecarIndex, RacecarState, kNumberOfRacecars, RacecarState::GetMutable> MutableRacecarContainerAccessor;
		static inline constexpr RacecarContainerAccessor AllRacecars(void) { return RacecarContainerAccessor(); }
		static inline constexpr MutableRacecarContainerAccessor AllMutableRacecars(void) { return MutableRacecarContainerAccessor(); }

		static std::vector<tbCore::uint8> GetAvailableCars(bool isSubbed, bool isTier3);
		static String GetCarFilepath(tbCore::uint8 carID);

		RacecarState(void);
		virtual ~RacecarState(void);

		void Create(icePhysics::World& physicalWorld);
		void Destroy(icePhysics::World& physicalWorld);

		void ResetRacecar(const iceMatrix4& vehicleToWorld);

		bool HasWon(void) const { return mCreatureFinished; }
		bool HasLost(void) const { return mSwarmHealth <= kMinimumCreatures; }
		CreatureIndex GetSwarmHealth(void) const { return mSwarmHealth; }
		bool IsCreatureAlive(const CreatureIndex& creatureIndex) const { return mCreatures[creatureIndex].mIsAlive; }

		void OnRacecarFinished(void);
		void OnCreatureFinished(const CreatureIndex& creatureIndex);

		iceMatrix4 GetBodyToWorld(void) const;
		iceMatrix4 GetWheelToWorld(const size_t wheelIndex) const;
		iceMatrix4 GetCreatureToWorld(const CreatureIndex& creatureIndex) const;

		///
		/// @details Returns roughly the center of the swarm.
		///
		iceMatrix4 GetSwarmToWorld(void) const;
		iceVector3 GetSwarmVelocity(void) const;

		iceMatrix4 GetVehicleToWorld(void) const;
		void SetVehicleToWorld(const iceMatrix4& vehicleToWorld);
		iceVector3 GetPreviousPosition(void) const { return mPreviousPosition; }

		iceVector3 GetAngularVelocity(void) const;
		void SetAngularVelocity(const iceVector3& angularVelocity);
		iceVector3 GetLinearVelocity(void) const;
		void SetLinearVelocity(const iceVector3& linearVelocity);

		iceScalar GetEngineSpeed(void) const;
		Gear GetShifterPosition(void) const;

		void Simulate(void);
		void RenderDebug(void) const;

		void SetRacecarController(RacecarControllerInterface* controller);
		const RacecarControllerInterface& GetRacecarController(void) const;
		RacecarControllerInterface& GetMutableRacecarController(void);

		const PhysicsModels::PhysicsModelInterface& GetPhysicsModel(void) { return *mPhysicsModel; }
		PhysicsModels::PhysicsModelInterface& GetMutablePhysicsModel(void) { return *mPhysicsModel; }

		inline RacecarIndex GetRacecarIndex(void) const { return mRacecarIndex; }
		inline DriverIndex GetDriverIndex(void) const { return mDriverIndex; }
		inline bool IsRacecarInUse(void) const { return IsValidDriver(mDriverIndex); }

		inline tbCore::uint8 GetRacecarMeshID(void) const { return mRacecarMeshID; }
		void SetRacecarMeshID(const tbCore::uint8 racecarMeshID);

		//For RaceSession/Manager or DriverState only.
		void SetRacecarIndex(const RacecarIndex& racecarIndex);
		void SetRacecarDriver(const DriverIndex driverIndex);

	private:
		void SimulateCreatureSwarm(void);
		iceVector3 CalculateCohesion(const Creature& creature, const iceScalar distance) const;
		iceVector3 CalculateSeparation(const Creature& creature, const iceScalar distance) const;
		iceVector3 CalculateAlignment(const Creature& creature, const iceScalar distance) const;

		std::array<Creature, kNumberOfCreatures> mCreatures;

		PhysicsModels::PhysicsModelInterfacePtr mPhysicsModel;
		std::unique_ptr<RacecarControllerInterface> mController;
		icePhysics::World* mPhysicalWorld;

		iceVector3 mPreviousPosition;
		iceMatrix4 mSwarmToWorld;
		iceVector3 mSwarmVelocity;
		int mOnTrackCounter;
		CreatureIndex mSwarmHealth;

		RacecarIndex mRacecarIndex;
		DriverIndex mDriverIndex;

		tbCore::uint8 mRacecarMeshID;
	private:
		bool mIsOnTrack;
		bool mIsVisible;
		bool mRacecarFinished;
		bool mCreatureFinished;
	};

};	//namespace LudumDare56::GameState

#endif /* LudumDare56_RacecarManager_hpp */
