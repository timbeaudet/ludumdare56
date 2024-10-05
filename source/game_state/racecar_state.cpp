///
/// @file
/// @details Manage all the racecars in the simulation on both game and server side.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../game_state/racecar_state.hpp"
#include "../game_state/racetrack_state.hpp"
#include "../game_state/driver_state.hpp"
#include "../game_state/helpers/torque_curve.hpp"
#include "../game_state/events/racecar_events.hpp"
#include "../game_state/racecar_controller_interface.hpp"

#include "../logging.hpp"

#include <array>

namespace
{
	using LudumDare56::GameState::PhysicsModels::PhysicsModel;

	const PhysicsModel kGenericPhysicsModel = PhysicsModel::ExtremelyBasic;
	const PhysicsModel kTheBestPhysicsModel = PhysicsModel::ExtremeDrifting; //This is intended for "Tim's Awesome Car" / Tier3 support.

	std::vector<PhysicsModel> thePhysicsModels = {
		PhysicsModel::ExtremelyFast, PhysicsModel::ExtremelyBasic, PhysicsModel::ExtremeDrifting, PhysicsModel::ExtremelyBasic, PhysicsModel::NullModel
	};

	typedef std::array<LudumDare56::GameState::RacecarState, LudumDare56::GameState::kNumberOfRacecars> RacecarArray;
	RacecarArray& TheRacecarArray(void)
	{
		static RacecarArray theRacecarArray;
		return theRacecarArray;
	}
};

PhysicsModel GetRacecarPhysicsModel(tbCore::uint8 carID);

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::GameState::RacecarState& LudumDare56::GameState::RacecarState::Get(const RacecarIndex racecarIndex)
{
	return TheRacecarArray()[racecarIndex];
}

LudumDare56::GameState::RacecarState& LudumDare56::GameState::RacecarState::GetMutable(const RacecarIndex racecarIndex)
{
	return TheRacecarArray()[racecarIndex];
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarState::RacecarState(void) :
	mCreatures(),
	mPhysicsModel(new PhysicsModels::NullPhysicsModel()),
	mController(new NullRacecarController()),
	mPhysicalWorld(nullptr),
	mOnTrackCounter(0),
	mRacecarIndex(InvalidRacecar()),
	mDriverIndex(InvalidDriver()),
	mRacecarMeshID(0),
	mIsOnTrack(false),
	mIsVisible(false)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarState::~RacecarState(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::Create(icePhysics::World& physicalWorld)
{
	tb_error_if(false == IsValidRacecar(mRacecarIndex), "Expected the RacecarIndex to be valid by Create().");

	mPhysicalWorld = &physicalWorld;
	mPhysicsModel = PhysicsModels::Instantiate(physicalWorld, GetRacecarPhysicsModel(mRacecarMeshID));

	RaceSessionState::PlaceCarOnGrid(*this);

	const iceScalar range = 5.0f;

	const iceMatrix4 vehicleToWorld = GetVehicleToWorld();
	for (Creature& creature : mCreatures)
	{
		const iceMatrix4 creatureToVehicle = iceMatrix4::Translation(tbMath::RandomFloat(-range, range),
			0.0f, tbMath::RandomFloat(-range, range));

		creature.mCreatureToWorld = creatureToVehicle * vehicleToWorld;
		creature.mCreatureToWorld.SetPosition(creature.mCreatureToWorld.GetPosition().x, 0.0f, creature.mCreatureToWorld.GetPosition().z);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::Destroy(icePhysics::World& /*physicalWorld*/)
{
	mPhysicalWorld = nullptr;
	mPhysicsModel.reset(new PhysicsModels::NullPhysicsModel());
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::ResetRacecar(const iceMatrix4& vehicleToWorld)
{
	SetVehicleToWorld(vehicleToWorld);
	mPhysicsModel->ResetRacecarForces();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetRacecarController(RacecarControllerInterface* controller)
{
	mController.reset((nullptr == controller) ? new NullRacecarController() : controller);
}

//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::GameState::RacecarControllerInterface& LudumDare56::GameState::RacecarState::GetRacecarController(void) const
{
	tb_error_if(nullptr == mController, "Expected the controller to always be valid, even if NullController.");
	return *mController;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarControllerInterface& LudumDare56::GameState::RacecarState::GetMutableRacecarController(void)
{
	tb_error_if(nullptr == mController, "Expected the controller to always be valid, even if NullController.");
	return *mController;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetRacecarMeshID(const tbCore::uint8 racecarMeshID)
{
	if (racecarMeshID != mRacecarMeshID)
	{
		mRacecarMeshID = racecarMeshID;

		if (nullptr != mPhysicalWorld)
		{
			if (true == IsRacecarInUse())
			{
				mPhysicsModel->SetEnabled(false);
			}

			mPhysicsModel = PhysicsModels::Instantiate(*mPhysicalWorld, GetRacecarPhysicsModel(mRacecarMeshID));

			if (true == IsRacecarInUse())
			{
				mPhysicsModel->SetEnabled(true);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetRacecarIndex(const RacecarIndex& racecarIndex)
{
	mRacecarIndex = racecarIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetRacecarDriver(const DriverIndex driverIndex)
{
	const DriverIndex previousDriverIndex = mDriverIndex;
	if (true == IsValidDriver(previousDriverIndex))
	{
		SendEvent(Events::RacecarSeatEvent(Events::Racecar::DriverLeavesRacecar, mDriverIndex, mRacecarIndex));
	}

	mDriverIndex = driverIndex;

	if (true == IsValidDriver(mDriverIndex))
	{
		RaceSessionState::PlaceCarOnGrid(*this);

		if (false == IsValidDriver(previousDriverIndex))
		{
			mPhysicsModel->SetEnabled(true);
		}

		RaceSessionState::PlaceCarOnGrid(*this);

		SendEvent(Events::RacecarSeatEvent(Events::Racecar::DriverEntersRacecar, mDriverIndex, mRacecarIndex));
	}
	else if (true == IsValidDriver(previousDriverIndex))
	{
		mPhysicsModel->SetEnabled(false);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetBodyToWorld(void) const
{
	return mPhysicsModel->GetBodyToWorld();
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetWheelToWorld(const size_t wheelIndex) const
{
	return mPhysicsModel->GetWheelToWorld(wheelIndex);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetCreatureToWorld(const CreatureIndex& creatureIndex) const
{
	return mCreatures[creatureIndex].mCreatureToWorld;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetVehicleToWorld(void) const
{
	return mPhysicsModel->GetVehicleToWorld();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetVehicleToWorld(const iceMatrix4& vehicleToWorld)
{
	mPhysicsModel->SetVehicleToWorld(vehicleToWorld);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::GetAngularVelocity(void) const
{
	return mPhysicsModel->GetAngularVelocity();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetAngularVelocity(const iceVector3& angularVelocity)
{
	mPhysicsModel->SetAngularVelocity(angularVelocity);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::GetLinearVelocity(void) const
{
	return mPhysicsModel->GetLinearVelocity();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetLinearVelocity(const iceVector3& linearVelocity)
{
	mPhysicsModel->SetLinearVelocity(linearVelocity);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Scalar LudumDare56::GameState::RacecarState::GetEngineSpeed(void) const
{
	return mPhysicsModel->GetEngineSpeed();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::Gear LudumDare56::GameState::RacecarState::GetShifterPosition(void) const
{
	return mPhysicsModel->GetShifterPosition();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::Simulate(void)
{
	mController->UpdateControls();

	mPhysicsModel->Simulate(*mController);

	SimulateCreatureSwarm();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::RenderDebug(void) const
{
#if !defined(ludumdare56_headless_build)
	if (true == IsRacecarInUse())
	{
		mPhysicsModel->DebugRender();
	}
#endif /* ludumdare56_headless_build */
}

//--------------------------------------------------------------------------------------------------------------------//

std::vector<tbCore::uint8> LudumDare56::GameState::RacecarState::GetAvailableCars(bool /*isSubbed*/, bool /*isTier3*/)
{
	std::vector<tbCore::uint8> cars;
	cars.push_back(0);
	cars.push_back(1);
	return cars;
}

//--------------------------------------------------------------------------------------------------------------------//

PhysicsModel GetRacecarPhysicsModel(tbCore::uint8 /*carID*/)
{
	//return PhysicsModel::ExtremeDrifting;
	return PhysicsModel::ExtremelyBasic;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString LudumDare56::GameState::RacecarState::GetCarFilepath(tbCore::uint8 carID)
{
	const tbCore::tbString pathToRacecars = "data/meshes/racecars/";
	std::vector<String> availableRacecars = {
		pathToRacecars + "indicator.msh",
		pathToRacecars + "formula_blue.msh",
		pathToRacecars + "formula_red.msh",
	};

	if (carID >= availableRacecars.size())
	{	//Return generic racecar.
		return availableRacecars[0];
	}

	return availableRacecars[carID];
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

//namespace
//{
	//icePhysics::Scalar kCohesionDistance = 7.0f;   //more like a visible range.
	//icePhysics::Scalar kSeparationDistance = 3.0f; //more like a in my personal space.

	//icePhysics::Scalar kAvoidFactor = 0.2f;    //separation
	//icePhysics::Scalar kMatchingFactor = 0.2f; //alignment
	//icePhysics::Scalar kCenteringFactor = 0.2f; //cohesion
	//icePhysics::Scalar kTargetFactor = 2.0;

	//icePhysics::Scalar kMaximumVelocity = 2.0;


icePhysics::Scalar kCohesionDistance = 0.0f;   //more like a visible range.
icePhysics::Scalar kSeparationDistance = 1.664f; //more like a in my personal space.

icePhysics::Scalar kAvoidFactor = 5.0f;    //separation
icePhysics::Scalar kMatchingFactor = 1.449f; //alignment
icePhysics::Scalar kCenteringFactor = 0.913f; //cohesion
icePhysics::Scalar kTargetFactor = 40.0;

icePhysics::Scalar kMaximumVelocity = 80.0;
icePhysics::Scalar kVelocityDrag = 0.89;

//};

void LudumDare56::GameState::RacecarState::SimulateCreatureSwarm(void)
{
	iceVector3 targetPosition = GetVehicleToWorld().GetPosition();
	targetPosition.y = 0.0f;

	tb_debug_log(LogState::Info() << "Target position: " << targetPosition);

	bool first = true;
	for (Creature& creature : mCreatures)
	{
		const iceVector3 alignment = CalculateAlignment(creature);
		const iceVector3 cohesion = CalculateCohesion(creature);
		const iceVector3 separation = CalculateSeparation(creature);
		creature.Move(targetPosition, alignment, cohesion, separation);

		if (first)
		{
			first = false;
			tb_debug_log(LogState::Info() << "   alignment: " << alignment);
			tb_debug_log(LogState::Info() << "   cohesion: " << cohesion);
			tb_debug_log(LogState::Info() << "   separation: " << separation);

		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::Creature::Move(const iceVector3& target, const iceVector3& alignment,
	const iceVector3& cohesion, const iceVector3& separation)
{
	iceVector3 position = mCreatureToWorld.GetPosition();

	const iceVector3 directionToTarget = (target - position).GetNormalized();
	//const iceVector3 directionToTarget = (target - position);

	mVelocity -= mVelocity * kVelocityDrag * kFixedTime;

	mVelocity += ((cohesion * kCenteringFactor) + (separation * kAvoidFactor) + (alignment * kMatchingFactor) +
		directionToTarget * kTargetFactor) * kFixedTime;

	const iceScalar speed = mVelocity.Magnitude();
	if (speed > kMaximumVelocity)
	{
		mVelocity = mVelocity.GetNormalized() * kMaximumVelocity;
	}

	position += mVelocity * kFixedTime;

	mCreatureToWorld.SetPosition(position);

	//TODO: Turn the creature
	const iceVector3 direction = (speed > 0.4) ? mVelocity.GetNormalized() : iceVector3::Forward();
	const iceVector3 right = Vector3::Cross(direction, Vector3::Up());
	mCreatureToWorld.SetBasis(0, right);
	mCreatureToWorld.SetBasis(1, Vector3::Up());
	mCreatureToWorld.SetBasis(2, -direction);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::CalculateCohesion(const Creature& creature) const
{
	int count = 0;
	iceVector3 averagePosition = iceVector3::Zero();

	for (const Creature& otherCreature : mCreatures)
	{
		if (&otherCreature == &creature)
		{	//Don't look at ourself!
			continue;
		}

		const iceScalar distance = creature.mCreatureToWorld.GetPosition().DistanceTo(otherCreature.mCreatureToWorld.GetPosition());
		if (distance < kCohesionDistance)
		{
			averagePosition += otherCreature.mCreatureToWorld.GetPosition();
			if (false == tbMath::IsZero(averagePosition.y))
			{
				rand();
			}
			++count;
		}
	}

	if (count > 0)
	{
		averagePosition = averagePosition / count;
		return averagePosition - creature.mCreatureToWorld.GetPosition();
	}

	return iceVector3::Zero();
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::CalculateSeparation(const Creature& creature) const
{
	iceVector3 separation = iceVector3::Zero();
	for (const Creature& otherCreature : mCreatures)
	{
		if (&otherCreature == &creature)
		{	//Don't look at ourself!
			continue;
		}

		const iceScalar distance = creature.mCreatureToWorld.GetPosition().DistanceTo(otherCreature.mCreatureToWorld.GetPosition());
		if (distance < kSeparationDistance)
		{
			separation += creature.mCreatureToWorld.GetPosition() - otherCreature.mCreatureToWorld.GetPosition();
			if (false == tbMath::IsZero(separation.y))
			{
				rand();
			}
		}
	}

	return separation;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::CalculateAlignment(const Creature& creature) const
{
	int count = 0;
	iceVector3 averageVelocity;

	for (const Creature& otherCreature : mCreatures)
	{
		if (&otherCreature == &creature)
		{	//Don't look at ourself!
			continue;
		}

		const iceScalar distance = creature.mCreatureToWorld.GetPosition().DistanceTo(otherCreature.mCreatureToWorld.GetPosition());
		if (distance < kCohesionDistance)	//used as 'visual range' in https://vanhunteradams.com/Pico/Animal_Movement/Boids-algorithm.html
		{
			if (false == tbMath::IsZero(averageVelocity.y))
			{
				rand();
			}

			averageVelocity += otherCreature.mVelocity;
			++count;
		}
	}

	if (count > 0)
	{
		averageVelocity = averageVelocity / count;
		return averageVelocity - creature.mVelocity;
	}

	return iceVector3::Zero();
}

//--------------------------------------------------------------------------------------------------------------------//
