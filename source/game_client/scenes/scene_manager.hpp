///
/// @file
/// @details Contains all of the scenes in the LudumDare56 project from splash, menus, options,
///   to gameplay to provide an  easy way for the scenes to be managed and changed between.
///   (Although TurtleBrains supplies a way to change between GameScene's it doesn't provide a great way to manage their memory.)
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_SceneManager_hpp
#define LudumDare56_SceneManager_hpp

#include "../../ludumdare56.hpp"

#include <vector>

namespace LudumDare56
{
	namespace GameClient
	{

		enum SceneId
		{
			kTitleScene,
			kRacingScene,
			kNextLevelScene,
			kSceneCount,
		};

		class SceneManager : public tbCore::Noncopyable
		{
		public:
			///
			/// @details Creates the SceneManager instance for theSceneManager which is accessed as a global singleton object.
			///   Each scene of the game APRIL is then added it to the SceneManager so it can be changed to the active scene
			///   with ChangeToScene().
			///
			static void CreateSceneManager(void);

			///
			/// @details Destroys the SceneManager instance for theSceneManager and sets it to nullptr, destroying all scenes
			///   in the process.
			///
			static void DestroySceneManager(void);

			///
			/// @details Returns a reference to the game scene object that corresponds with the sceneIdentifier supplied. An
			///   error condition will be triggered if the SceneManager has not been created with CreateSceneManager or if the
			///   sceneIdentifier is out of range.
			///
			static tbGame::GameScene& GetScene(const SceneId& sceneIdentifier);

			///
			/// @details Returns a reference to the game scene object that corresponds with the sceneIdentifier supplied. An
			///   error condition will be triggered if the SceneManager has not been created with CreateSceneManager or if the
			///   sceneIdentifier is out of range.
			///
			template<typename Type> static Type& GetSceneAs(const SceneId& sceneIdentifier)
			{
				Type* sceneType = dynamic_cast<Type*>(&GetScene(sceneIdentifier));
				tb_error_if(nullptr == sceneType, "Expected the scene from identifier to match Type.");
				return *sceneType;
			}

			///
			/// @details Changes the TurtleBrains Scene to the desired scene which will then be the active scene.  An error
			///   condition will be triggered if the SceneManager has not been created with CreateSceneManager() or if the
			///   sceneIdentifier is out of range.
			///
			static void ChangeToScene(const SceneId& sceneIdentifier);

			///
			/// @details Signals TurtleBrains to close the game.
			///
			static void QuitGame(void);

		private:
			SceneManager(void);
			~SceneManager(void);

			std::vector<tbGame::GameScene*> mScenes;
		};

		extern SceneManager* theSceneManager;

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_SceneManager_hpp */
