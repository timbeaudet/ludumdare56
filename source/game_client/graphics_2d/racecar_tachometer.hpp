///
/// @file
/// @details Displays a tachometer for the engine speed of a racecar in LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RacecarTachometer_hpp
#define LudumDare56_RacecarTachometer_hpp

#include "../../game_state/race_session_state.hpp"

#include "../../core/event_system.hpp"

#include <turtle_brains/graphics/tb_graphic.hpp>
#include <turtle_brains/graphics/tb_sprite.hpp>

namespace LudumDare56
{
	namespace GameClient
	{

		class RacecarTachometer : public tbGraphics::Graphic
		{
		public:
			RacecarTachometer(const GameState::RacecarIndex racecarIndex);
			virtual ~RacecarTachometer(void);

			void SetRacecarIndex(const GameState::RacecarIndex racecarIndex);

		protected:
			virtual void OnUpdate(const float deltaTime) override;
			virtual void OnRender(void) const override;

			tbMath::Angle ComputeNeedleRotationForRPM(const float revolutionsPerMinute) const;

		private:
			tbGraphics::Sprite mTachometerSprite;
			tbGraphics::Sprite mNeedleSprite;
			GameState::RacecarIndex mRacecarIndex;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_RacecarTachometer_hpp */
