///
/// @file
/// @details Allow the user to adjust settings and configure LudumDare56 to meet their needs.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/entities_2d/settings_screen_entity.hpp"
#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/scenes/racing_scene.hpp"
#include "../../game_client/scenes/scene_manager.hpp"
#include "../../core/input/key_binder.hpp"
#include "../../logging.hpp"

#include <turtle_brains/application/tb_application_window.hpp>
#include <turtle_brains/audio/tb_audio_manager.hpp>
#include <turtle_brains/game/tb_entity_manager.hpp>
#include <turtle_brains/game/tb_game_application.hpp>
#include <turtle_brains/express/behaviors/tbx_basic_behaviors.hpp>
#include <turtle_brains/system/unstable/tbu_input_device_manager.hpp>

namespace
{
	//std::vector<std::pair<tbCore::uint16, tbCore::uint16>> screenResolutions{
	//	//{ 1024, 768 }, { 1280, 720 }, { 1280, 1024}, { 1440, 900 }, { 1680, 1050 }, { 1920, 1080 }
	//	std::pair<tbCore::uint16, tbCore::uint16>{ static_cast<tbCore::uint16>(1920), static_cast<tbCore::uint16>(1080) },
	//	std::pair<tbCore::uint16, tbCore::uint16>{ static_cast<tbCore::uint16>(1680), static_cast<tbCore::uint16>(1050) },
	//	std::pair<tbCore::uint16, tbCore::uint16>{ static_cast<tbCore::uint16>(1440), static_cast<tbCore::uint16>(900) },
	//	std::pair<tbCore::uint16, tbCore::uint16>{ static_cast<tbCore::uint16>(1280), static_cast<tbCore::uint16>(1024) },
	//	std::pair<tbCore::uint16, tbCore::uint16>{ static_cast<tbCore::uint16>(1280), static_cast<tbCore::uint16>(720) },
	//	std::pair<tbCore::uint16, tbCore::uint16>{ static_cast<tbCore::uint16>(1024), static_cast<tbCore::uint16>(768) }
	//};
};

using namespace TyreBytes::Core::Input::KeyBinder;

namespace LudumDare56
{
	namespace GameClient
	{
		extern tbGame::GameApplication* theGameApplication;
	};
};

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::SettingsScreenEntity::SettingsScreenEntity(void) :
	tbGame::Entity("SettingsScreenEntity"),
	mSettings(),
	mTabbedDisplayGraphic(new ui::TabbedDisplayGraphic()),
	mBindingDisplayGraphic(new ui::AnalogBindingGraphic()),
	mBackButtonGraphic("Press Escape"), //The scene should never be used as we are using the callback instead.
	mEscapeAction(tbApplication::tbKeyEscape),
	mBindingControlSettingKey(""),
	mBindableControlLabel(nullptr),
	mIsBindingInvertableControl(false),
	mEscapePressedWithoutBinding(false)
{
	//TODO: LudumDare56: 2023-10-24: Probably in Core::Input put some definitions to help XBox-like controllers. Note,
	//   that will almost certainly be true for Windows only, but so is how this line of code is already...
	//8 is left bumper, 9 is right bumper and 11 is the B button on xbox-like controllers.
	mEscapeAction.AddBinding(0, 11);

	ResetInterfaceControls();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::SettingsScreenEntity::~SettingsScreenEntity(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameClient::SettingsScreenEntity::IsDisplayingSettings(void) const
{
	return (nullptr != GetEntityManager());
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameClient::SettingsScreenEntity::IsBindingButton(void) const
{
	return (false == mBindingControlSettingKey.empty());
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SettingsScreenEntity::OnAdd(void)
{
	tbGame::Entity::OnAdd();

	ResetInterfaceControls();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SettingsScreenEntity::OnUpdate(const float deltaTime)
{
	const float interfaceScale = UserInterface::InterfaceScale();

	{
		mTabbedDisplayGraphic->SetScale(interfaceScale);
		mTabbedDisplayGraphic->SetOrigin(tbGraphics::kAnchorCenter);
		mTabbedDisplayGraphic->SetPosition(ui::GetAnchorPositionOfInterface(tbGraphics::kAnchorCenter));
		mTabbedDisplayGraphic->SetEnabled(false == IsBindingButton());
	}

	//This wants to be below the tabbedDisplay disabling itself when IsBindingButton() is true.
	tbGame::Entity::OnUpdate(deltaTime);

	//It is possible that IsBindingButton became true in the Entity::OnUpdate() because the callback would have hit, and
	//  we need to ensure the display is setup properly before getting to render. This must be below OnUpdate().
	if (true == IsBindingButton())
	{
		mBindingDisplayGraphic->SetScale(interfaceScale);
		mBindingDisplayGraphic->SetOrigin(tbGraphics::kAnchorCenter);
		mBindingDisplayGraphic->SetPosition(ui::GetAnchorPositionOfInterface(tbGraphics::kAnchorCenter));
		mBindingDisplayGraphic->Update(deltaTime);
	}

	mBackButtonGraphic.SetScale(interfaceScale);
	mBackButtonGraphic.SetOrigin(tbGraphics::kAnchorBottomLeft);
	mBackButtonGraphic.SetPosition(ui::GetAnchorPositionOf(*mTabbedDisplayGraphic, tbGraphics::kAnchorBottomLeft, tbMath::Vector2(20.0f, -20.0f)));
	mBackButtonGraphic.SetVisible(false == IsBindingButton());

	if (false == IsBindingButton())
	{
		if (true == mEscapeAction.IsPressed())
		{
			mEscapePressedWithoutBinding = true;
		}
		if (true == mEscapePressedWithoutBinding && true == mEscapeAction.IsReleased())
		{
			OnAcceptSettings();
			PushBehavior(new tbxBehaviors::KillBehavior(*this));
		}
	}


	const float musicVolume = mSettings.GetFloat("volume_music");
	const float soundVolume = mSettings.GetFloat("volume_sound");
	tbAudio::theAudioManager.SetGlobalMusicVolume(musicVolume * musicVolume);
	tbAudio::theAudioManager.SetGlobalEffectVolume(soundVolume * soundVolume);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SettingsScreenEntity::OnRender(void) const
{
	tbGame::Entity::OnRender();

	if (true == IsBindingButton())
	{
		mBindingDisplayGraphic->Render();
	}
	else
	{
		const float interfaceScale = ui::InterfaceScale();
		ui::TextAreaGraphic restartWarning;
		restartWarning.AddLineOfText(tbGraphics::Text("Some settings apply after leaving this screen.", 20.0f));
		restartWarning.SetScale(interfaceScale);
		restartWarning.SetOrigin(tbGraphics::kAnchorBottomCenter);
		restartWarning.SetPosition(ui::GetAnchorPositionOfInterface(tbGraphics::kAnchorBottomCenter, tbMath::Vector2(0.0f, -170.0f * interfaceScale)));
		restartWarning.Render();
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SettingsScreenEntity::OnAcceptSettings(void)
{
	if (mSettings.GetBoolean(Settings::VerticalSync()) != TheUserSettings().GetBoolean(Settings::VerticalSync()))
	{
		tbApplication::WindowProperties window = theGameApplication->GetWindowProperties();
		window.mVerticalSync = mSettings.GetBoolean(Settings::VerticalSync());
		theGameApplication->SetWindowProperties(window);
	}

	TheUserSettings() = mSettings;

	//This needs to be below SetSettings() as the PlayerRacecarController() will grab bindings from GameSettings.
	if (true /* == mUserChangedControlBindings */)
	{
		 theSceneManager->GetSceneAs<RacingScene>(SceneId::kRacingScene).UpdateControllerBindings();
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SettingsScreenEntity::ResetInterfaceControls(void)
{
	using namespace TyreBytes::Core::Input;

	mSettings = TheUserSettings();

	RemoveGraphic(mTabbedDisplayGraphic.get());
	mTabbedDisplayGraphic.reset(new ui::TabbedDisplayGraphic());
	AddGraphic(*mTabbedDisplayGraphic);

	mBackButtonGraphic.SetCallback([this]() {
		OnAcceptSettings();
		GetEntityManager()->RemoveEntity(this);
	});
	AddGraphic(mBackButtonGraphic);

	///////  GRAPHICS TAB
	mTabbedDisplayGraphic->AddTab("Graphics");

	ui::CheckboxGraphic* vsyncCheckbox = new ui::CheckboxGraphic();
	mTabbedDisplayGraphic->AddControlLine(new ui::LabelGraphic("VSync"), vsyncCheckbox);
	vsyncCheckbox->SetChecked(mSettings.GetBoolean(Settings::VerticalSync()));
	vsyncCheckbox->SetCallback([this, vsyncCheckbox]() {
		mSettings.SetBoolean(Settings::VerticalSync(), vsyncCheckbox->IsChecked());
	});

	ui::CheckboxGraphic* shadowCheckbox = new ui::CheckboxGraphic();
	mTabbedDisplayGraphic->AddControlLine(new ui::LabelGraphic("Shadows"), shadowCheckbox);
	shadowCheckbox->SetChecked(mSettings.GetBoolean(Settings::ShowShadows()));
	shadowCheckbox->SetCallback([this, shadowCheckbox]() {
		mSettings.SetBoolean(Settings::ShowShadows(), shadowCheckbox->IsChecked());
	});

	ui::CheckboxGraphic* reflectionsCheckbox = new ui::CheckboxGraphic();
	mTabbedDisplayGraphic->AddControlLine(new ui::LabelGraphic("Reflections"), reflectionsCheckbox);
	reflectionsCheckbox->SetChecked(mSettings.GetBoolean(Settings::ShowReflections()));
	reflectionsCheckbox->SetCallback([this, reflectionsCheckbox]() {
		mSettings.SetBoolean(Settings::ShowReflections(), reflectionsCheckbox->IsChecked());
	});

	ui::CheckboxGraphic* particlesCheckbox = new ui::CheckboxGraphic();
	mTabbedDisplayGraphic->AddControlLine(new ui::LabelGraphic("Particles"), particlesCheckbox);
	particlesCheckbox->SetChecked(mSettings.GetBoolean(Settings::ShowParticles()));
	particlesCheckbox->SetCallback([this, particlesCheckbox]() {
		mSettings.SetBoolean(Settings::ShowParticles(), particlesCheckbox->IsChecked());
	});

	///////  AUDIO TAB
	mTabbedDisplayGraphic->AddTab("Audio");

	ui::SliderBarGraphic* musicVolumeSlider = new ui::SliderBarGraphic();
	mTabbedDisplayGraphic->AddControlLine(new ui::LabelGraphic("Music Volume"), musicVolumeSlider);
	musicVolumeSlider->SetSliderPercentage(mSettings.GetFloat(Settings::MusicVolume()));
	musicVolumeSlider->SetCallback([this, musicVolumeSlider]() {
		mSettings.SetFloat(Settings::MusicVolume(), musicVolumeSlider->GetSliderPercentage());
	});

	ui::SliderBarGraphic* soundVolumeSlider = new ui::SliderBarGraphic();
	mTabbedDisplayGraphic->AddControlLine(new ui::LabelGraphic("Sound Volume"), soundVolumeSlider);
	soundVolumeSlider->SetSliderPercentage(mSettings.GetFloat(Settings::SoundVolume()));
	soundVolumeSlider->SetCallback([this, soundVolumeSlider]() {
		mSettings.SetFloat(Settings::SoundVolume(), soundVolumeSlider->GetSliderPercentage());
	});

	///////  INPUT TAB
	mTabbedDisplayGraphic->AddTab("Input");

	ui::SpriteButtonGraphic* bindSteeringButton = new ui::SpriteButtonGraphic("Steering");
	ui::LabelGraphic* steeringControlLabel = new ui::LabelGraphic(KeyBinder::FullNameToDisplayName(mSettings.GetString(Settings::ControlSteering())));
	mTabbedDisplayGraphic->AddControlLine(bindSteeringButton, steeringControlLabel);
	bindSteeringButton->SetCallback([this, steeringControlLabel]() {
		StartBinding(Settings::ControlSteering(), steeringControlLabel, true);
	});

	ui::SpriteButtonGraphic* bindThrottleButton = new ui::SpriteButtonGraphic("Throttle");
	ui::LabelGraphic* throttleControlLabel = new ui::LabelGraphic(KeyBinder::FullNameToDisplayName(mSettings.GetString(Settings::ControlThrottle())));
	mTabbedDisplayGraphic->AddControlLine(bindThrottleButton, throttleControlLabel);
	bindThrottleButton->SetCallback([this, throttleControlLabel]() {
		StartBinding(Settings::ControlThrottle(), throttleControlLabel, true);
	});

	ui::SpriteButtonGraphic* bindBrakesButton = new ui::SpriteButtonGraphic("Brake");
	ui::LabelGraphic* brakeControlLabel = new ui::LabelGraphic(KeyBinder::FullNameToDisplayName(mSettings.GetString(Settings::ControlBrake())));
	mTabbedDisplayGraphic->AddControlLine(bindBrakesButton, brakeControlLabel);
	bindBrakesButton->SetCallback([this, brakeControlLabel]() {
		StartBinding(Settings::ControlBrake(), brakeControlLabel, true);
	});

	ui::SpriteButtonGraphic* bindShiftUpButton = new ui::SpriteButtonGraphic("Shift Up");
	ui::LabelGraphic* shiftUpControlLabel = new ui::LabelGraphic(KeyBinder::FullNameToDisplayName(mSettings.GetString(Settings::ControlShiftUp())));
	mTabbedDisplayGraphic->AddControlLine(bindShiftUpButton, shiftUpControlLabel);
	bindShiftUpButton->SetCallback([this, shiftUpControlLabel]() {
		StartBinding(Settings::ControlShiftUp(), shiftUpControlLabel);
	});

	ui::SpriteButtonGraphic* bindShiftDownButton = new ui::SpriteButtonGraphic("Shift Down");
	ui::LabelGraphic* shiftDownControlLabel = new ui::LabelGraphic(KeyBinder::FullNameToDisplayName(mSettings.GetString(Settings::ControlShiftDown())));
	mTabbedDisplayGraphic->AddControlLine(bindShiftDownButton, shiftDownControlLabel);
	bindShiftDownButton->SetCallback([this, shiftDownControlLabel]() {
		StartBinding(Settings::ControlShiftDown(), shiftDownControlLabel);
	});

	ui::SpriteButtonGraphic* bindHandbrakeButton = new ui::SpriteButtonGraphic("Handbrake");
	ui::LabelGraphic* handbrakeControlLabel = new ui::LabelGraphic(KeyBinder::FullNameToDisplayName(mSettings.GetString(Settings::ControlHandbrake())));
	mTabbedDisplayGraphic->AddControlLine(bindHandbrakeButton, handbrakeControlLabel);
	bindHandbrakeButton->SetCallback([this, handbrakeControlLabel]() {
		StartBinding(Settings::ControlHandbrake(), handbrakeControlLabel);
	});
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SettingsScreenEntity::StartBinding(const String& controlSettingKey,
	ui::LabelGraphic* controlLabel, const bool allowInvert)
{
	mBindingControlSettingKey = controlSettingKey;
	mIsBindingInvertableControl = allowInvert;

	mBindableControlLabel = controlLabel;
	mEscapePressedWithoutBinding = false;

	mBindingDisplayGraphic.reset(new ui::AnalogBindingGraphic());
	mBindingDisplayGraphic->SetCallback([this]() {
		FinishBinding(mBindingDisplayGraphic->IsConfirmedBinding());
	});
	mBindingDisplayGraphic->StartBinding();

	mTabbedDisplayGraphic->SetVisible(false);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SettingsScreenEntity::FinishBinding(const bool keepTheNewBinding)
{
	tb_error_if(true == mBindingControlSettingKey.empty(), "Expected to be binding a control when finishing...");

	if (true == keepTheNewBinding)
	{	//Only put the newly bound control into the settings if we should keep the binding!
		if (nullptr != mBindableControlLabel)
		{
			mBindableControlLabel->SetText(mBindingDisplayGraphic->GetControlDisplayName());
		}

		mSettings.SetString(mBindingControlSettingKey, mBindingDisplayGraphic->GetControlFullName());
		if (true == mIsBindingInvertableControl)
		{
			mSettings.SetBoolean(mBindingControlSettingKey + "_inverted", mBindingDisplayGraphic->IsControlInverted());
		}
	}

	mBindableControlLabel = nullptr;
	mEscapePressedWithoutBinding = false;

	mIsBindingInvertableControl = false;
	mBindingControlSettingKey = "";

	mBindingDisplayGraphic->SetVisible(false);
	mTabbedDisplayGraphic->SetVisible(true);
}

//--------------------------------------------------------------------------------------------------------------------//
