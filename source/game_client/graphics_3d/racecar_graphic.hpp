///
/// @file
/// @details A simple graphic to display the Racecars.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RacecarEntity_hpp
#define LudumDare56_RacecarEntity_hpp

#include "../../game_state/racecar_state.hpp"

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/graphics/tb_graphic_list.hpp>
#include <turtle_brains/graphics/tb_sprite.hpp>
#include <turtle_brains/graphics/tb_text.hpp>

#include <ice/graphics/ice_graphic.hpp>

namespace LudumDare56
{
	namespace GameClient
	{

		class RacecarGraphic
		{
		public:
			static bool sDisplayCarNumbers;

			explicit RacecarGraphic(void);
			~RacecarGraphic(void);

			tbCore::uint8 GetRacecarIndex(void) const { return mRacecarIndex; }
			void SetRacecarIndex(tbCore::uint8 racecarIndex);
			void SetRacecarMesh(const tbCore::tbString& meshFilepath) { mRacecarGraphic.SetMesh(meshFilepath); }

			inline tbMath::Matrix4 GetRacecarToWorld(void) const { return mRacecarGraphic.GetObjectToWorld(); }

			void Update(const float deltaTime);
			void SetVisible(bool visible) { mRacecarGraphic.SetVisible(visible); }

		private:
			typedef GameState::RacecarState::CreatureIndex CreatureIndex;
			static constexpr CreatureIndex kNumberOfCreatures = GameState::RacecarState::kNumberOfCreatures;

			tbCore::uint8 mRacecarIndex;
			tbCore::uint8 mRacecarMeshID;
			iceGraphics::Graphic mRacecarGraphic;
			std::array<iceGraphics::Graphic, 4> mWheelGraphics;
			std::array<iceGraphics::Graphic, kNumberOfCreatures> mCreatureGraphics;

			tbGraphics::Text mLagText;
			tbGraphics::Text mCarText;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_RacecarEntity_hpp */
