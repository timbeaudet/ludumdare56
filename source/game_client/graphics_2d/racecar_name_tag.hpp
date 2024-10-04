///
/// @file
/// @details A name tag that floats over each of the racecars in the simulator to identify who is who.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RacecarNameTag_hpp
#define LudumDare56_RacecarNameTag_hpp

#include "../../game_state/race_session_state.hpp"
#include "../../core/event_system.hpp"

#include <turtle_brains/graphics/tb_graphic.hpp>
#include <turtle_brains/express/graphics/tbx_shadowed_text.hpp>

namespace LudumDare56
{
	namespace GameClient
	{

		class RacecarNameTag : public tbGraphics::Graphic
		{
		public:
			RacecarNameTag(const GameState::RacecarIndex& racecarIndex);
			virtual ~RacecarNameTag(void);

		protected:
			virtual void OnUpdate(const float deltaTime) override;
			virtual void OnRender(void) const override;

		private:
			const GameState::RacecarIndex mRacecarIndex;
			tbxGraphics::ShadowedText mNameText;
			float mHitConeTimer;
			bool mIsOnTrack;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_RacecarNameTag_hpp */
