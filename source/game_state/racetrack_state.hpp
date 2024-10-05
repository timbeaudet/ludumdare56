///
/// @file
/// @details Manage the racetrack of the game!
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RacetrackManager_hpp
#define LudumDare56_RacetrackManager_hpp

#include "../game_state/race_session_state.hpp"
#include "../core/event_system.hpp"
#include "../core/typed_range.hpp"
#include "../ludumdare56.hpp"

#include <track_bundler/track_bundler.hpp>

#include <turtle_brains/math/tb_vector.hpp>
#include <turtle_brains/math/tb_curve.hpp>
#include <turtle_brains/core/tb_string.hpp>

#include <ice/physics/ice_physical_world.hpp>

class TrackEditorScene;

namespace LudumDare56
{
	namespace GameState
	{
		class ObjectState;

		namespace RacetrackState
		{
			enum class ObjectHandleType : tbCore::uint32 { };
			typedef tbCore::TypedInteger<ObjectHandleType> ObjectHandle;
			constexpr ObjectHandle InvalidObject(void) { return ObjectHandle::Integer(~0); }

			enum class TrackNodeIndexType : tbCore::uint16 { };
			typedef tbCore::TypedInteger<TrackNodeIndexType> TrackNodeIndex;
			constexpr TrackNodeIndex InvalidTrackNode(void) { return TrackNodeIndex::Integer(~0); }

			///
			/// @details Add an EventListenter for RacetrackEvents defined in ...
			///
			void AddEventListener(TyreBytes::Core::EventListener& eventListener);

			///
			/// @details Remove an EventListenter to stop listening to events from the RacetrackState changing.
			///
			void RemoveEventListener(TyreBytes::Core::EventListener& eventListener);


			void InvalidateRacetrack(void);
			bool IsValidRacetrack(void);
			const String& GetCurrentRacetrack(void);
			const iceCore::MeshHandle& GetCurrentRacetrackMesh(void);

			void Create(icePhysics::World& physicalWorld);
			void Destroy(icePhysics::World& physicalWorld);

			void Simulate(void);
			void RenderDebug(void);

			icePhysics::Matrix4 GetGridToWorld(const GridIndex gridIndex);

			TrackBundler::Legacy::TrackBundle& GetTrackBundle(void);
			void LoadRacetrack(const String& racetrackFilepath);

			const ObjectState& GetObjectState(const ObjectHandle objectHandle);
			ObjectState& GetMutableObjectState(const ObjectHandle objectHandle);

			/// @note 2023-10-19: Some of the following may want to move into a Timing and Scoring area that the Racectrack
			///   can manage/update etc. But for sake of focusing on Artificial Drivers First Lap, I'm just winging it.

			enum TrackEdge
			{
				kFarLeft,
				kLeft,
				kCenter,
				kRight,
				kFarRight,

				kNumberOfEdges
			};

			///
			/// @details This is for providing the track nodes that will be used in position timing & scoring as well as
			///   aritificial drivers.
			///
			typedef std::array<Vector3, TrackEdge::kNumberOfEdges> TrackNodeEdge;

			TrackNodeIndex GetNumberOfTrackNodes(void);

			const Vector3& GetTrackNodeLeadingEdge(const TrackNodeIndex trackNodeIndex, const TrackEdge trackEdge);
			const TrackNodeEdge& GetTrackNodeLeadingEdge(const TrackNodeIndex trackNodeIndex);
			const Vector3& GetTrackNodeTrailingEdge(const TrackNodeIndex trackNodeIndex, const TrackEdge trackEdge);
			const TrackNodeEdge& GetTrackNodeTrailingEdge(const TrackNodeIndex trackNodeIndex);

			inline bool IsValidTrackNode(const TrackNodeIndex trackNodeIndex) { return trackNodeIndex <= GetNumberOfTrackNodes(); }

			bool IsOnTrack(const iceVector3& positionInWorld);

		};	//namespace RacetrackState

		typedef RacetrackState::TrackEdge TrackEdge;

	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_RacetrackManager_hpp */
