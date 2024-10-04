///
/// @file
/// @details Contains all of the scenes in the LudumDare56 project from splash, menus, options,
///   to gameplay to provide an  easy way for the scenes to be managed and changed between.
///   (Although TurtleBrains supplies a way to change between GameScene's it doesn't provide a great way to manage their memory.)
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/scenes/scene_manager.hpp"
#include "../../game_client/scenes/title_scene.hpp"
#include "../../game_client/scenes/racing_scene.hpp"

LudumDare56::GameClient::SceneManager* LudumDare56::GameClient::theSceneManager(nullptr);

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::SceneManager::SceneManager(void) :
	mScenes()
{
	mScenes.resize(kSceneCount, nullptr);
	mScenes[SceneId::kTitleScene] = new TitleScene();
	mScenes[SceneId::kRacingScene] = new RacingScene();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::SceneManager::~SceneManager(void)
{
	while (false == mScenes.empty())
	{
		delete mScenes.back();
		mScenes.pop_back();
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SceneManager::CreateSceneManager(void)
{
	tb_error_if(nullptr != theSceneManager, "Expected theSceneManager to be null when calling create, seems create has been called twice.");
	theSceneManager = new SceneManager();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SceneManager::DestroySceneManager(void)
{
	tb_error_if(nullptr == theSceneManager, "Expected theSceneManager to be a valid pointer when calling destroy, seems destroy has been called twice, or without create.");
	delete theSceneManager;
	theSceneManager = nullptr;
}

//--------------------------------------------------------------------------------------------------------------------//

tbGame::GameScene& LudumDare56::GameClient::SceneManager::GetScene(const SceneId& sceneIdentifier)
{
	tb_error_if(nullptr == theSceneManager, "Expected theSceneManager to be created and valid before ChangeToScene is called.");
	tb_error_if(sceneIdentifier < 0, "Expected a valid sceneIdentifier, below zero is out of range.");
	tb_error_if(static_cast<size_t>(sceneIdentifier) >= theSceneManager->mScenes.size(), "Expected a valid sceneIdentifier, out of range.  Was the scene added to manager?");
	return *theSceneManager->mScenes[sceneIdentifier];
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SceneManager::ChangeToScene(const SceneId& sceneIdentifier)
{
	tbGame::GameScene::ChangeToScene(SceneManager::GetScene(sceneIdentifier));
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SceneManager::QuitGame(void)
{
	tb_error_if(nullptr == theSceneManager, "Expected theSceneManager to be created and valid before ChangeToScene is called.");
	tbGame::GameApplication::MarkForClose();
}

//--------------------------------------------------------------------------------------------------------------------//
