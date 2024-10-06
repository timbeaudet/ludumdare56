///
/// @file
/// @details Displays a health bar for the swarm of ant drivers.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_WinLoseScreenGraphic_hpp
#define LudumDare56_WinLoseScreenGraphic_hpp

#include "../../game_state/race_session_state.hpp"
#include "../../game_client/user_interface/sprite_button_graphic.hpp"

#include "../../core/event_system.hpp"

#include <turtle_brains/graphics/tb_graphic.hpp>
#include <turtle_brains/graphics/tb_sprite.hpp>

#include <turtle_brains/express/graphics/tbx_shadowed_text.hpp>
#include <turtle_brains/express/graphics/tbx_web_sprite.hpp>

namespace LudumDare56::GameClient
{

	class WinLoseScreenGraphic : public tbGraphics::Graphic
	{
	public:
		WinLoseScreenGraphic(const GameState::RacecarIndex racecarIndex);
		virtual ~WinLoseScreenGraphic(void);

		void SetRacecarIndex(const GameState::RacecarIndex racecarIndex);

	protected:
		virtual void OnUpdate(const float deltaTime) override;
		virtual void OnRender(void) const override;

	private:
		void GotoNextLevel(void);

		enum class State { None, Win, Lose };

		tbxGraphics::ShadowedText mYouWinText;
		tbxGraphics::ShadowedText mWinStatusText;
		tbxGraphics::ShadowedText mYouLoseText;
		GameState::RacecarIndex mRacecarIndex;

		UserInterface::SpriteButtonGraphic mRetryButton;
		UserInterface::SpriteButtonGraphic mNextButton;
		State mState;
	};

};	//namespace LudumDare56::GameClient

#endif /* LudumDare56_WinLoseScreenGraphic_hpp */
