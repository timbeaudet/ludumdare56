///
/// @file
/// @details A simple Graphic icon / button that takes the user to another scene when it is clicked on and displays a
///   control icon that will also take the user back to another scene if pressed.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/user_interface/back_button_graphic.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"
#include "../../game_client/scenes/scene_manager.hpp"

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::BackButtonGraphic::BackButtonGraphic(SceneId backToScene) :
	SpriteButtonGraphic(""),
	mBackToScene(backToScene)
{
	SetCallback([this]() {
		theSceneManager->ChangeToScene(mBackToScene);
	});
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::BackButtonGraphic::~BackButtonGraphic(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::BackButtonGraphic::SetBackToScene(SceneId backToScene)
{
	mBackToScene = backToScene;
}

//--------------------------------------------------------------------------------------------------------------------//
