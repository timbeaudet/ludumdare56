///
/// @file
/// @details Displays a speedometer to show how fast a racecar is moving in LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RacecarSpeedometer_hpp
#define LudumDare56_RacecarSpeedometer_hpp

#include "../../game_state/race_session_state.hpp"

#include "../../core/event_system.hpp"

#include <turtle_brains/graphics/tb_graphic.hpp>
#include <turtle_brains/graphics/tb_sprite.hpp>

namespace LudumDare56
{
	namespace GameClient
	{

		class RacecarSpeedometer : public tbGraphics::Graphic
		{
		public:
			RacecarSpeedometer(const GameState::RacecarIndex racecarIndex);
			virtual ~RacecarSpeedometer(void);

			void SetRacecarIndex(const GameState::RacecarIndex racecarIndex);

		protected:
			virtual void OnUpdate(const float deltaTime) override;
			virtual void OnRender(void) const override;

		private:
			GameState::RacecarIndex mRacecarIndex;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_RacecarSpeedometer_hpp */
