///
/// @file
/// @details Provide a simple title scene for the LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_TitleScene_hpp
#define LudumDare56_TitleScene_hpp

#include "../../game_client/scenes/base_3d_scene.hpp"
#include "../../game_client/user_interface/sprite_button_graphic.hpp"
#include "../../game_client/user_interface/message_box_graphic.hpp"
#include "../../game_client/entities_2d/settings_screen_entity.hpp"

#include <turtle_brains/graphics/tb_sprite.hpp>
#include <turtle_brains/game/tb_game_scene.hpp>
#include <turtle_brains/game/tb_game_timer.hpp>
#include <turtle_brains/game/tb_input_action.hpp>

namespace LudumDare56
{
	namespace GameClient
	{

		class TitleScene : public Base3dScene
		{
		public:
			TitleScene(void);
			virtual ~TitleScene(void);

			static void GotoTitleWithMessage(const tbCore::tbString& message);

			static void RenderTitleBackdrop(void);

		protected:
			virtual void OnSimulate(void) override;
			virtual void OnUpdate(const float deltaTime) override;
			virtual void OnOrthographicRender(void) const override;
			virtual void OnInterfaceRender(void) const override;
			virtual void OnOpen(void) override;
			virtual void OnClose(void) override;

		private:
			ui::MessageBoxGraphic mMessageBox;
			ui::SpriteButtonGraphic mPracticeButton;
			ui::SpriteButtonGraphic mSettingsButton;
			ui::SpriteButtonGraphic mExitButton;

			SettingsScreenEntity mSettingsScreenEntity;

			tbGame::GameTimer mFadeInTimer;
			tbGame::GameTimer mFadeOutTimer;
			tbGame::InputAction mStartGameAction;
			tbGame::InputAction mQuitGameAction;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_TitleScene_hpp */
