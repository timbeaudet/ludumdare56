///
/// @file
/// @details A simple Graphic icon / button that takes the user to another scene when it is clicked on and displays a
///   control icon that will also take the user back to another scene if pressed.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_BackButtonGraphic_hpp
#define LudumDare56_BackButtonGraphic_hpp

#include "../../game_client/scenes/scene_manager.hpp"
#include "../../game_client/user_interface/sprite_button_graphic.hpp"

#include <functional>

namespace LudumDare56
{
	namespace GameClient
	{
		namespace UserInterface
		{

			class BackButtonGraphic : public SpriteButtonGraphic
			{
			public:
				explicit BackButtonGraphic(SceneId backToScene);
				virtual ~BackButtonGraphic(void);

				void SetBackToScene(SceneId backToScene);

			private:
				GameClient::SceneId mBackToScene;
			};

		};	//namespace UserInterface

		namespace ui = UserInterface;
	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_BackButtonGraphic_hpp */
