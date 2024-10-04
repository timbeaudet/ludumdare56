///
/// @file
/// @details A simple user interface graphic object for the user to control a checkbox, on/off switch.
///   This is NOT part of the TurtleBrains::ExpressKit interface in anyway, it is just a sprite object.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_CheckboxGraphic_hpp
#define LudumDare56_CheckboxGraphic_hpp

#include "../../game_client/user_interface/base_control_graphic.hpp"

#include <turtle_brains/express/graphics/tbx_nine_slice.hpp>

namespace LudumDare56
{
	namespace GameClient
	{
		namespace UserInterface
		{

			class CheckboxGraphic : public BaseControlGraphic
			{
			public:
				explicit CheckboxGraphic(void);
				virtual ~CheckboxGraphic(void);

				inline bool IsChecked(void) const { return mIsChecked; }
				void SetChecked(const bool isChecked);

				virtual tbGraphics::PixelSpace GetPixelWidth(void) const override;
				virtual tbGraphics::PixelSpace GetPixelHeight(void) const override;

			protected:
				virtual void OnUpdate(const float deltaTime) override;
				virtual void OnRender(void) const override;

				tbxGraphics::NineSlice mBackdropGraphic;
				tbxGraphics::NineSlice mOutlineGraphic;

			private:
				bool mIsChecked;
			};

		};	//namespace UserInterface

		namespace ui = UserInterface;
	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_CheckboxGraphic_hpp */
