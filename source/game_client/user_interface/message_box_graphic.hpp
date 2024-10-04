///
/// @file
/// @details Displays a message to the user with a little box surrounding it.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_MessageBoxGraphic_hpp
#define LudumDare56_MessageBoxGraphic_hpp

#include "../../game_client/user_interface/text_area_graphic.hpp"
#include "../../game_client/user_interface/sprite_button_graphic.hpp"

#include <turtle_brains/express/graphics/tbx_nine_slice.hpp>

#include <functional>

namespace LudumDare56
{
	namespace GameClient
	{
		namespace UserInterface
		{

			class MessageBoxGraphic : public tbGraphics::Graphic
			{
			public:
				explicit MessageBoxGraphic(const tbCore::tbString& message);
				virtual ~MessageBoxGraphic(void);

				//This will automatically split on \n for new lines.
				void SetMessage(const tbCore::tbString& message);

				void SetOkayCallback(std::function<void()> callbackFunction);

				virtual tbGraphics::PixelSpace GetPixelWidth(void) const override;
				virtual tbGraphics::PixelSpace GetPixelHeight(void) const override;

			protected:
				virtual void OnUpdate(const float deltaTime) override;
				virtual void OnRender(void) const override;

			private:
				tbxGraphics::NineSlice mBackdrop;
				TextAreaGraphic mTextArea;
				SpriteButtonGraphic mOkayButton;
				std::function<void(void)> mOkayCallback;
			};

		};	//namespace UserInterface

		namespace ui = UserInterface;
	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_MessageBoxGraphic_hpp */
