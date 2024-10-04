///
/// @file
/// @details A simple graphic to display the Racetrack.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RacetrackGraphic_hpp
#define LudumDare56_RacetrackGraphic_hpp

#include "core/event_system.hpp"
#include "game_state/racetrack_state.hpp"
#include "game_state/components/component_creator.hpp"

#include <track_bundler/track_bundler.hpp>

#include <turtle_brains/math/tb_curve.hpp>

#include <ice/graphics/ice_graphic.hpp>
#include <ice/graphics/ice_decal.hpp>
#include <ice/graphics/ice_visualization.hpp>

namespace LudumDare56
{

	namespace CustomComponent
	{
		static const TrackBundler::ComponentDefinitionKey GrandStand("f541890d-8f33-4926-bc20-5a4e615e23b7");
	};

	namespace GameClient
	{

		class RacetrackGraphic : public TyreBytes::Core::EventListener, public GameState::ComponentCreatorInterface
		{
		public:
			explicit RacetrackGraphic(void);
			virtual ~RacetrackGraphic(void);

			void Update(const float deltaTime);

			virtual void OnHandleEvent(const TyreBytes::Core::Event& event) override;

			void SetVisible(bool visible);

			void RenderDebug(void) const;

		protected:
			virtual GameState::ComponentStatePtr OnCreateComponent(GameState::ObjectState& object, const TrackBundler::Component& componentInformation) override;

		private:
			void GenerateFrom(const TrackBundler::Legacy::TrackBundle& trackBundle, const TrackBundler::Legacy::TrackSegmentDefinitionContainer& segmentDefinitions,
				const TrackBundler::Legacy::TrackObjectDefinitionContainer& objectDefinitions, const TrackBundler::Legacy::TrackSplineDefinitionContainer& splineDefinitions);

			typedef std::unique_ptr<iceGraphics::Graphic> GraphicPointer;
			typedef std::vector<GraphicPointer> GraphicContainer;
			typedef std::pair<GameState::RacetrackState::ObjectHandle, GraphicContainer> ObjectGraphicPair;
			std::vector<ObjectGraphicPair> mRacetrackObjectGraphics;

			std::vector<GraphicPointer> mRacetrackGraphics;
			std::vector<std::unique_ptr<iceGraphics::Decal>> mDecals;

			iceGraphics::Visualization mDebugVisuals;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_RacetrackGraphic_hpp */
