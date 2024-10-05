///
/// @file
/// @details Provide a simple scene to Close, change levels, then Reopoen the RacingScene.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_NextLevelScene_hpp
#define LudumDare56_NextLevelScene_hpp

#include "../../game_client/scenes/base_3d_scene.hpp"
#include "../../game_client/user_interface/sprite_button_graphic.hpp"
#include "../../game_client/user_interface/message_box_graphic.hpp"
#include "../../game_client/entities_2d/settings_screen_entity.hpp"

#include <turtle_brains/graphics/tb_sprite.hpp>
#include <turtle_brains/game/tb_game_scene.hpp>
#include <turtle_brains/game/tb_game_timer.hpp>
#include <turtle_brains/game/tb_input_action.hpp>

namespace LudumDare56::GameClient
{

	class NextLevelScene : public Base3dScene
	{
	public:
		NextLevelScene(void);
		virtual ~NextLevelScene(void);

	protected:
		virtual void OnSimulate(void) override;
		virtual void OnUpdate(const float deltaTime) override;
		virtual void OnOrthographicRender(void) const override;
		virtual void OnInterfaceRender(void) const override;
		virtual void OnOpen(void) override;
		virtual void OnClose(void) override;

	private:

	};

};	//namespace LudumDare56::GameClient

#endif /* LudumDare56_NextLevelScene_hpp */
