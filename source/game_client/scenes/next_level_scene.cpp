///
/// @file
/// @details Provide a simple scene to Close, change levels, then Reopoen the RacingScene.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/scenes/next_level_scene.hpp"
#include "../../game_client/scenes/scene_manager.hpp"
#include "../../game_client/scenes/racing_scene.hpp"

#include "../../logging.hpp"

#include <turtle_brains/game/tb_game_application.hpp>
#include <turtle_brains/graphics/tb_text.hpp>
#include <turtle_brains/graphics/tb_sprite.hpp>

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::NextLevelScene::NextLevelScene(void) :
	Base3dScene()
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::NextLevelScene::~NextLevelScene(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::NextLevelScene::OnSimulate(void)
{
	ludumdare56_start_timer(TimingChannel::kSimulate);

	Base3dScene::OnSimulate();

	ludumdare56_stop_timer(TimingChannel::kSimulate);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::NextLevelScene::OnUpdate(const float deltaTime)
{
	ludumdare56_start_timer(TimingChannel::kUpdate);

	Base3dScene::OnUpdate(deltaTime);

	tb_debug_log(LogClient::Info() << "Updating NextLevelScene.");
	GameState::RaceSessionState::AdvanceToNextLevel();

	tb_debug_log(LogClient::Info() << "Changing Scene to RacingScene.");
	theSceneManager->ChangeToScene(SceneId::kRacingScene);

	ludumdare56_stop_timer(TimingChannel::kUpdate);
	ludumdare56_start_timer(TimingChannel::kRender);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::NextLevelScene::OnOrthographicRender(void) const
{
	Base3dScene::OnOrthographicRender();
	ludumdare56_stop_timer(TimingChannel::kRender);
	DisplayDeveloperConsole();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::NextLevelScene::OnInterfaceRender(void) const
{
	//TitleScene::RenderTitleBackdrop();
	Base3dScene::OnInterfaceRender();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::NextLevelScene::OnOpen(void)
{
	tb_debug_log(LogClient::Info() << "Opening NextLevelScene.");
	Base3dScene::OnOpen();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::NextLevelScene::OnClose(void)
{
	Base3dScene::OnClose();
	tb_debug_log(LogClient::Info() << "Closing NextLevelScene.");
}

//--------------------------------------------------------------------------------------------------------------------//
