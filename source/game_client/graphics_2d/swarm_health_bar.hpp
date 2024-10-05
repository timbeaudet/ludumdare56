///
/// @file
/// @details Displays a health bar for the swarm of ant drivers.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_SwarmHealthBar_hpp
#define LudumDare56_SwarmHealthBar_hpp

#include "../../game_state/race_session_state.hpp"

#include "../../core/event_system.hpp"

#include <turtle_brains/graphics/tb_graphic.hpp>
#include <turtle_brains/graphics/tb_sprite.hpp>

#include <turtle_brains/express/counters/tbx_health_bar.hpp>

namespace LudumDare56::GameClient
{

	class SwarmHealthBar : public tbGraphics::Graphic
	{
	public:
		SwarmHealthBar(const GameState::RacecarIndex racecarIndex);
		virtual ~SwarmHealthBar(void);

		void SetRacecarIndex(const GameState::RacecarIndex racecarIndex);

	protected:
		virtual void OnUpdate(const float deltaTime) override;
		virtual void OnRender(void) const override;

	private:
		GameState::RacecarIndex mRacecarIndex;
		tbxCounters::HealthBar mHealthBar;
	};

};	//namespace LudumDare56::GameClient

#endif /* LudumDare56_SwarmHealthBarp */
