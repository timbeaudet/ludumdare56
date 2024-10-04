///
/// @file
/// @details A simple Graphic icon / button that performs a callback when it is clicked on with the mouse. This is NOT
///   part of the TurtleBrains::ExpressKit interface in anyway, it is just a sprite object.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_SpriteButtonGraphic_hpp
#define LudumDare56_SpriteButtonGraphic_hpp

#include "../../game_client/user_interface/base_control_graphic.hpp"

#include <turtle_brains/graphics/tb_text.hpp>
#include <turtle_brains/express/graphics/tbx_nine_slice.hpp>

#include <functional>

namespace LudumDare56
{
	namespace GameClient
	{
		namespace UserInterface
		{

			enum class ButtonType { kTitlePrimary, kTitleSecondary, kTitleExit };

			class SpriteButtonGraphic : public BaseControlGraphic
			{
			public:
				explicit SpriteButtonGraphic(const tbCore::tbString& buttonLabel, const ButtonType buttonType = ButtonType::kTitleSecondary);
				virtual ~SpriteButtonGraphic(void);

				void SetLabel(const tbCore::tbString& buttonLabel);

				virtual tbGraphics::PixelSpace GetPixelWidth(void) const override;
				virtual tbGraphics::PixelSpace GetPixelHeight(void) const override;

			protected:
				virtual void OnUpdate(const float deltaTime) override;
				virtual void OnRender(void) const override;

				tbxGraphics::NineSlice mBackdropGraphic;
				tbGraphics::Text mLabelText;

			private:
				const ButtonType mButtonType;
			};

		};	//namespace UserInterface

		namespace ui = UserInterface;
	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_SpriteButtonGraphic_hpp */
