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
	mPreviousPosition(iceVector3::Zero()),
	mSwarmToWorld(iceMatrix4::Identity()),
	mOnTrackCounter(0),
	mSwarmHealth(kNumberOfCreatures),
	mRacecarIndex(InvalidRacecar()),
	mDriverIndex(InvalidDriver()),
	mRacecarMeshID(0),
	mIsOnTrack(false),
	mIsVisible(false),
	mRacecarFinished(false),
	mCreatureFinished(false)
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

	ResetRacecar(GetVehicleToWorld());
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

	const iceScalar range = 5.0f;

	for (Creature& creature : mCreatures)
	{
		const iceMatrix4 creatureToVehicle = iceMatrix4::Translation(tbMath::RandomFloat(-range, range),
			0.0f, tbMath::RandomFloat(-range, range));

		creature.mCreatureToWorld = creatureToVehicle * vehicleToWorld;
		creature.mCreatureToWorld.SetPosition(creature.mCreatureToWorld.GetPosition().x, 0.0f, creature.mCreatureToWorld.GetPosition().z);
		creature.mPreviousPosition = creature.mCreatureToWorld.GetPosition();
		creature.mVelocity = iceVector3::Zero();
		creature.mIsOnTrack = true;
		creature.mIsAlive = true;
		creature.mIsRacing = true;
	}

	mPreviousPosition = vehicleToWorld.GetPosition();

	mRacecarFinished = false;
	mCreatureFinished = false;
	mSwarmHealth = kNumberOfCreatures;
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

void LudumDare56::GameState::RacecarState::OnRacecarFinished(void)
{
	mRacecarFinished = true;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::OnCreatureFinished(const CreatureIndex& creatureIndex)
{
	mCreatureFinished = true;
	mCreatures[creatureIndex].mIsRacing = false;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetCreatureToWorld(const CreatureIndex& creatureIndex) const
{
	return mCreatures[creatureIndex].mCreatureToWorld;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetSwarmToWorld(void) const
{
	return mSwarmToWorld;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::GetSwarmVelocity(void) const
{
	return mSwarmVelocity;
};

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
	mPreviousPosition = GetVehicleToWorld().GetPosition();

	if (false == mRacecarFinished && false == HasLost())
	{
		mController->UpdateControls();
		mPhysicsModel->Simulate(*mController);
	}
	else
	{
		BrakeOnlyRacecarController brakesController;
		mPhysicsModel->Simulate(brakesController);

		mPhysicsModel->SetLinearVelocity(mPhysicsModel->GetLinearVelocity() * 0.5f * kFixedTime);
	}

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
icePhysics::Scalar kTargetRange = -3.5;
icePhysics::Scalar kTargetSpeed = 0.5;

//When target is stationary / in range.
//icePhysics::Scalar kCohesionDistance = 1.0f;   //more like a visible range.
//icePhysics::Scalar kSeparationDistance = 0.5f; //more like a in my personal space.
//
//icePhysics::Scalar kAvoidFactor = 2.0f;    //separation
//icePhysics::Scalar kMatchingFactor = 1.25f; //alignment
//icePhysics::Scalar kCenteringFactor = 0.913f; //cohesion
//icePhysics::Scalar kTargetFactor = 0.25;
//
//icePhysics::Scalar kMaximumVelocity = 80.0;
//icePhysics::Scalar kVelocityDrag = 0.89;
//};

void LudumDare56::GameState::RacecarState::SimulateCreatureSwarm(void)
{
	//if (true == HasLost())
	//{
	//	for (Creature& creature : mCreatures)
	//	{
	//		if (true == creature.mIsAlive)
	//		{
	//  Ideally we would add drag to any alive creatures, and stop them from moving around. But then I realized
	//  if they are alive, but under gravity, or on/off track etc... yuck. Also at time of writing we are testing
	//  all creature dead to lose, so nobody is alive anyway.
	//		}
	//	}
	//}

#if defined(tb_debug_build)
	static int skipFrames = 5;
#else
	//1 disables, since all indices will be mod == 0. 2 = skip 1 frame, even/odds...
	static int skipFrames = 1;
#endif
	static int dumdumFrameCounter = 0;
	++dumdumFrameCounter;
	dumdumFrameCounter = dumdumFrameCounter % skipFrames;

	iceVector3 targetPosition = GetVehicleToWorld().GetPosition();
	targetPosition.y = 0.0f;

	const iceScalar targetSpeed = GetLinearVelocity().Magnitude();

	//tb_debug_log(LogState::Info() << "Target position: " << targetPosition);

	int creatureCount = 0;
	iceVector3 swarmPosition = iceVector3::Zero();
	mSwarmVelocity = iceVector3::Zero();

	//iceCore::MeshHandle racetrackMesh = RacetrackState::GetCurrentRacetrackMesh();

	//bool first = true;
	CreatureIndex creatureIndex = 0;
	mSwarmHealth = 0;
	for (Creature& creature : mCreatures)
	{
		creature.mPreviousPosition = creature.mCreatureToWorld.GetPosition();
		if (false == creature.mIsAlive)
		{
			++creatureIndex;
			continue;
		}

		if (false == creature.mIsRacing)
		{
			++mSwarmHealth;
			++creatureIndex;
			continue;
		}

		// TODO: LudumDare56: 2024-10-05: We might want to go implement the Spline Collider to take in a specific collider
		//   mesh instead of forcing visuals.
		iceScalar fraction;
		iceVector3 intersectionPoint;
		if (creatureIndex % skipFrames == dumdumFrameCounter)
		{
			if (true == mPhysicalWorld->HackyAPI_CastRayToGlobalCollider(creature.mCreatureToWorld.GetPosition() +
				Vector3::Up() * 2.0f, Vector3::Down(), intersectionPoint, fraction) && fraction < 2.1)
			{
				creature.mIsOnTrack = true;
				//creature.mVelocity.y = 0.0;

				const iceVector3 oldPosition = creature.mCreatureToWorld.GetPosition();
				iceVector3 position = creature.mCreatureToWorld.GetPosition();
				position.y = intersectionPoint.y + 0.1f;
				creature.mCreatureToWorld.SetPosition(position);

				creature.mVelocity.y = position.y - oldPosition.y;
			}
			else
			{
				//creature.mIsAlive = false;
				creature.mIsOnTrack = false;
			}
		}

		if (false == mIsOnTrack)
		{
			creature.mVelocity.y += -10.0f * kFixedTime;
			if (creature.mCreatureToWorld.GetPosition().y < - 5.0f)
			{
				creature.mIsAlive = false;
				++creatureIndex;
				continue;
			}
		}
		else if (creature.mVelocity.y < 0.0f)
		{
			creature.mVelocity.y = 0.0;
		}

		icePhysics::Scalar visibleDistance = kCohesionDistance;
		icePhysics::Scalar bubbleDistance = kSeparationDistance;

		if (creature.mCreatureToWorld.GetPosition().DistanceTo(targetPosition) < kTargetRange &&
			targetSpeed < kTargetSpeed)
		{
			visibleDistance = 1.0;
			bubbleDistance = 0.5;
		}

		const iceVector3 alignment = CalculateAlignment(creature, visibleDistance);
		const iceVector3 cohesion = CalculateCohesion(creature, visibleDistance);
		const iceVector3 separation = CalculateSeparation(creature, bubbleDistance);
		const iceVector3 closeSeparation = CalculateSeparation(creature, bubbleDistance / 4.0f);

		creature.Move(targetPosition, targetSpeed, alignment, cohesion, separation);

		//if (first)
		//{
		//	first = false;
		//	tb_debug_log(LogState::Info() << "   alignment: " << alignment);
		//	tb_debug_log(LogState::Info() << "   cohesion: " << cohesion);
		//	tb_debug_log(LogState::Info() << "   separation: " << separation);
		//}

		swarmPosition += creature.mCreatureToWorld.GetPosition();
		mSwarmVelocity += creature.mVelocity;
		mSwarmVelocity.y = 0.0f;

		++mSwarmHealth;
		++creatureCount;
		++creatureIndex;
	}

	if (creatureCount == 0)
	{	//Should never happen if we have minimum health be 20 or 40 creatures...
		mSwarmToWorld = GetVehicleToWorld();
	}
	else
	{
		swarmPosition /= creatureCount;
		mSwarmVelocity /= creatureCount;

		mSwarmToWorld.SetPosition(swarmPosition);

		iceScalar swarmSpeed = 0.0;
		iceVector3 direction = iceVector3::Normalize(mSwarmVelocity, swarmSpeed);
		if (swarmSpeed < 0.4)
		{
			direction = iceVector3::Forward();
		}

		const iceVector3 right = Vector3::Cross(direction, Vector3::Up());
		mSwarmToWorld.SetBasis(0, right);
		mSwarmToWorld.SetBasis(1, Vector3::Up());
		mSwarmToWorld.SetBasis(2, -direction);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::Creature::Move(const iceVector3& targetPosition, const iceScalar targetSpeed,
	const iceVector3& alignment, const iceVector3& cohesion, const iceVector3& separation)
{
	iceVector3 position = mCreatureToWorld.GetPosition();

	iceScalar distanceToTarget = 0.0;
	const iceVector3 directionToTarget = iceVector3::Normalize(targetPosition - position, distanceToTarget);
	//const iceVector3 directionToTarget = (targetPosition - position);

	icePhysics::Scalar avoidFactor = kAvoidFactor;
	icePhysics::Scalar centerFactor = kCenteringFactor;
	icePhysics::Scalar matchFactor = kMatchingFactor;
	icePhysics::Scalar targetFactor = kTargetFactor;

	if (distanceToTarget < kTargetRange && targetSpeed < kTargetSpeed)
	{
		avoidFactor = 2.0f;    //separation
		matchFactor = 1.25f; //alignment
		centerFactor = 0.913f; //cohesion
		targetFactor = 0.25;

		//icePhysics::Scalar kMaximumVelocity = 80.0;
		//icePhysics::Scalar kVelocityDrag = 0.89;
	}

	iceScalar speed = 0.0;
	if (true == mIsOnTrack)
	{
		mVelocity -= mVelocity * kVelocityDrag * kFixedTime;

		// Ignore any Y from swarm behavior.
		iceVector3 flatVelocity = mVelocity;
		flatVelocity += ((cohesion * centerFactor) + (separation * avoidFactor) + (alignment * matchFactor) +
			directionToTarget * targetFactor) * kFixedTime;
		flatVelocity.y = 0.0;

		speed = flatVelocity.Magnitude();
		if (speed > kMaximumVelocity)
		{
			flatVelocity = flatVelocity.GetNormalized() * kMaximumVelocity;
		}

		mVelocity.x = flatVelocity.x;
		mVelocity.z = flatVelocity.z;
	}

	position += mVelocity * kFixedTime;

	mCreatureToWorld.SetPosition(position);

	const iceVector3 direction = (speed > 0.4) ? mVelocity.GetNormalized() : iceVector3::Forward();
	const iceVector3 right = Vector3::Cross(direction, Vector3::Up());
	mCreatureToWorld.SetBasis(0, right);
	mCreatureToWorld.SetBasis(1, Vector3::Up());
	mCreatureToWorld.SetBasis(2, -direction);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::CalculateCohesion(const Creature& creature, const iceScalar visibleDistance) const
{
	int count = 0;
	iceVector3 averagePosition = iceVector3::Zero();

	for (const Creature& otherCreature : mCreatures)
	{
		if (&otherCreature == &creature || false == otherCreature.mIsAlive)
		{	//Don't look at ourself or unalived creatures!
			continue;
		}

		const iceScalar distance = creature.mCreatureToWorld.GetPosition().DistanceTo(otherCreature.mCreatureToWorld.GetPosition());
		if (distance < visibleDistance)
		{
			averagePosition += otherCreature.mCreatureToWorld.GetPosition();
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

icePhysics::Vector3 LudumDare56::GameState::RacecarState::CalculateSeparation(const Creature& creature, const iceScalar separationDistance) const
{
	iceVector3 separation = iceVector3::Zero();
	for (const Creature& otherCreature : mCreatures)
	{
		if (&otherCreature == &creature || false == otherCreature.mIsAlive)
		{	//Don't look at ourself or unalived creatures!
			continue;
		}

		iceScalar distance = 0.0;
		iceVector3 separationDirection = iceVector3::Normalize(creature.mCreatureToWorld.GetPosition() - otherCreature.mCreatureToWorld.GetPosition(), distance);
		if (distance < separationDistance)
		{
			// The source article said this; but we actually need to invert it so that we separate more strongly from
			//   the creatures that are closer than the creatures that are near the separation 'border'.
			//	separation += creature.mCreatureToWorld.GetPosition() - otherCreature.mCreatureToWorld.GetPosition();
			separation += separationDirection * (separationDistance - distance);
		}
	}

	return separation;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::CalculateAlignment(const Creature& creature, const iceScalar visibleDistance) const
{
	int count = 0;
	iceVector3 averageVelocity;

	for (const Creature& otherCreature : mCreatures)
	{
		if (&otherCreature == &creature || false == otherCreature.mIsAlive)
		{	//Don't look at ourself or unalived creatures!
			continue;
		}

		const iceScalar distance = creature.mCreatureToWorld.GetPosition().DistanceTo(otherCreature.mCreatureToWorld.GetPosition());
		if (distance < visibleDistance)	//used as 'visual range' in https://vanhunteradams.com/Pico/Animal_Movement/Boids-algorithm.html
		{
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
