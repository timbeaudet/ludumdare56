///
/// @file
/// @details A single lined text label for some of the user interface parts in LudumDare56.
/// @history This file was original started in Trailing Brakes Racing Simulator in 2022.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_LabelGraphic_hpp
#define LudumDare56_LabelGraphic_hpp

#include "../../game_client/user_interface/base_control_graphic.hpp"

#include <turtle_brains/graphics/tb_text.hpp>

namespace LudumDare56
{
	namespace GameClient
	{
		namespace UserInterface
		{

			class LabelGraphic : public BaseControlGraphic
			{
			public:
				explicit LabelGraphic(const tbCore::tbString& labelText);
				virtual ~LabelGraphic(void);

				void SetText(const tbCore::tbString& labelText);

				inline virtual tbGraphics::PixelSpace GetPixelWidth(void) const { return mLabelText.GetPixelWidth(); }
				inline virtual tbGraphics::PixelSpace GetPixelHeight(void) const { return mLabelText.GetPixelHeight(); }

			protected:
				virtual void OnRender(void) const;

			private:
				tbGraphics::Text mLabelText;
			};

		};	//namespace UserInterface

		namespace ui = UserInterface;
	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_LabelGraphic_hpp */
