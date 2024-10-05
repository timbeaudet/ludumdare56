///
/// @file
/// @details Contains component / logic for the finish area where the player will finish a level/track and get a time.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_state/components/zone_finish_component.hpp"
#include "../../game_state/racetrack_state.hpp"
#include "../../game_state/timing_and_scoring_state.hpp"

#include "../../logging.hpp"

#include <track_bundler/track_bundler_to_ice_physics.hpp>

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ZoneFinishComponent::ZoneFinishComponent(ObjectState& object, const TrackBundler::Component& /*component*/) :
	ComponentState(object)
{
	//TrackBundler::CreateBoundingVolumesFrom(static_cast<TrackBundler::NodeKey>(object.GetID()),

	//object.Com
	//TrackBundler::CreateBoundingBoxFrom(component, object.GetObjectToWorld(), true);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::ZoneFinishComponent::~ZoneFinishComponent(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::ZoneFinishComponent::OnAwake(void)
{
	const Matrix4 objectToWorld = mObject.GetObjectToWorld();
	//const icePhysics::BoundingSphere finishSphere(icePhysics::Vector3::Zero(), 0.25f);
	//TimingState::AddCheckpoint(static_cast<iceMatrix4>(objectToWorld), finishSphere, 0, false);

	TimingState::AddCheckpoint(static_cast<iceMatrix4>(objectToWorld), 0, false);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::ZoneFinishComponent::OnDestroy(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::ZoneFinishComponent::OnSimulate(void)
{
	for (RacecarState& racecar : RacecarState::AllMutableRacecars())
	{
		for (RacecarState::CreatureIndex creatureIndex = 0; creatureIndex < RacecarState::kNumberOfCreatures; ++creatureIndex)
		{
			RacecarState::Creature& creature = racecar.GetMutableCreature(creatureIndex);
			if (false == creature.mIsAlive && false == creature.mIsRacing)
			{
				continue;
			}

			const iceVector3 finishPosition = mObject.GetObjectToWorld().GetPosition();
			const iceVector3 finishDirection = -mObject.GetObjectToWorld().GetBasis(2).GetNormalized();
			const iceVector3 creatureStartPosition = creature.mPreviousPosition;
			const iceVector3 creatureFinalPosition = creature.mCreatureToWorld.GetPosition();
			const iceVector3 creatureDirection = creatureStartPosition.DirectionTo(creatureFinalPosition);
			icePhysics::BoundingPlane plane(finishPosition, finishDirection, icePhysics::Performant::Normalized);
			icePhysics::BoundingRay ray(creature.mPreviousPosition, creatureDirection, icePhysics::Performant::Normalized);

			icePhysics::Vector3 at;
			if (true == icePhysics::LineSegmentToPlaneCollision(creatureStartPosition, creatureFinalPosition,
				finishPosition, finishDirection, at))
			{
				if (at.DistanceTo(finishPosition) < 10.0f && Vector3::Dot(creatureDirection, finishDirection) > 0.0f)
				{	//Creature finished!
					creature.mIsRacing = false;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//
