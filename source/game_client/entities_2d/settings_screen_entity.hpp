///
/// @file
/// @details Allow the user to adjust settings and configure LudumDare56 to meet their needs.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_SettingsScreenEntity_hpp
#define LudumDare56_SettingsScreenEntity_hpp

#include "../../game_client/user_interface/tabbed_display_graphic.hpp"
#include "../../game_client/user_interface/sprite_button_graphic.hpp"
#include "../../game_client/user_interface/checkbox_graphic.hpp"
#include "../../game_client/user_interface/slider_bar_graphic.hpp"
#include "../../game_client/user_interface/text_area_graphic.hpp"
#include "../../game_client/user_interface/label_graphic.hpp"
#include "../../game_client/user_interface/analog_binding_graphic.hpp"

#include "../../user_settings.hpp"

#include <turtle_brains/game/tb_entity.hpp>
#include <turtle_brains/game/tb_input_action.hpp>
#include <turtle_brains/graphics/tb_render_target.hpp>
#include <turtle_brains/express/graphics/tbx_nine_slice.hpp>
#include <turtle_brains/express/interface/tbx_interface_screen.hpp>
#include <turtle_brains/express/interface/tbx_interface_handler_interface.hpp>

#include <memory>
#include <array>

namespace LudumDare56
{
	namespace GameClient
	{

		class SettingsScreenEntity : public tbGame::Entity
		{
		public:
			SettingsScreenEntity(void);
			virtual ~SettingsScreenEntity(void);

			bool IsDisplayingSettings(void) const;
			bool IsBindingButton(void) const;

		protected:
			virtual void OnAdd(void) override;
			virtual void OnUpdate(const float deltaTime) override;
			virtual void OnRender(void) const override;

			void OnAcceptSettings(void);
			void ResetInterfaceControls(void);

		private:
			void StartBinding(const tbCore::tbString& controlSettingKey, ui::LabelGraphic* controlLabel, const bool allowInvert = false);
			void FinishBinding(const bool keepTheNewBinding);

			UserSettings mSettings;
			std::unique_ptr<ui::TabbedDisplayGraphic> mTabbedDisplayGraphic;
			std::unique_ptr<ui::AnalogBindingGraphic> mBindingDisplayGraphic;
			ui::SpriteButtonGraphic mBackButtonGraphic;

			tbGame::InputAction mEscapeAction;

			//std::array<tbCore::tbString*, kBindMaximumValue> mBindableControlNames;
			//std::array<bool*, kBindMaximumValue> mBindableControlInvertAxis; //Must be below the mSettings.

			tbCore::tbString mBindingControlSettingKey;
			ui::LabelGraphic* mBindableControlLabel; //A pointer to the label that takes the control name!

			bool mIsBindingInvertableControl;
			bool mEscapePressedWithoutBinding;
		};


	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_SettingsScreenEntity_hpp */
