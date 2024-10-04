///
/// @file
/// @details Creates a small box/display area for user interface when the user is binding an analog control. Will show
///   the current values and automatically invert as necessary.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_AnalogBindingGraphic_hpp
#define LudumDare56_AnalogBindingGraphic_hpp

#include "../../game_client/user_interface/base_control_graphic.hpp"
#include "../../game_client/user_interface/label_graphic.hpp"
#include "../../game_client/user_interface/checkbox_graphic.hpp"
#include "../../game_client/user_interface/slider_bar_graphic.hpp"
#include "../../game_client/user_interface/sprite_button_graphic.hpp"

#include <turtle_brains/express/graphics/tbx_nine_slice.hpp>

#include <vector>
#include <functional>
#include <memory>

namespace LudumDare56
{
	namespace GameClient
	{
		namespace UserInterface
		{

			class AnalogBindingGraphic : public BaseControlGraphic
			{
			public:
				AnalogBindingGraphic(void);
				virtual ~AnalogBindingGraphic(void);

				void StartBinding(void);

				virtual void OnUpdate(const float deltaTime) override;
				virtual void OnRender(void) const override;

				virtual tbGraphics::PixelSpace GetPixelWidth(void) const override;
				virtual tbGraphics::PixelSpace GetPixelHeight(void) const override;

				inline bool IsConfirmedBinding(void) const { return mIsConfirmedBinding; }

				//The following are only valid if IsConfirmedBinding() is true.
				tbCore::tbString GetControlDisplayName(void) const;
				tbCore::tbString GetControlFullName(void) const;
				bool IsControlInverted(void) const;

			private:
				void PollKeyBinder(void);
				void FinishBinding(bool isConfirmedBinding);

				tbxGraphics::NineSlice mBackdropGraphic;
				tbxGraphics::NineSlice mOutlineGraphic;

				SliderBarGraphic mAxisSlider;
				CheckboxGraphic mInvertCheckbox;
				LabelGraphic mAxisLabel;
				LabelGraphic mControlBindingText;
				SpriteButtonGraphic mConfirmButton;
				SpriteButtonGraphic mCancelBindingButton;

				tbCore::tbString mPossibleControlName;
				bool mPossibleControlInvert;
				bool mIsConfirmedBinding;
				bool mFirstFrame;
			};

		};	//namespace UserInterface

		namespace ui = UserInterface;
	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_AnalogBindingGraphic_hpp */
