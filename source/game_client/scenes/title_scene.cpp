///
/// @file
/// @details Provide a simple title scene for the LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/scenes/title_scene.hpp"
#include "../../game_client/scenes/scene_manager.hpp"
#include "../../game_client/scenes/racing_scene.hpp"
#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"

#include "../../network/network_handlers.hpp"
#include "../../version.hpp"
#include "../../logging.hpp"

#include <turtle_brains/game/tb_game_application.hpp>
#include <turtle_brains/graphics/tb_text.hpp>
#include <turtle_brains/graphics/tb_sprite.hpp>

namespace
{
	const tbGame::GameTimer kMaximumFadeInTime(500);
	const tbGame::GameTimer kMaximumFadeOutTime(500);
};

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::TitleScene::GotoTitleWithMessage(const tbCore::tbString& message)
{
	TitleScene& titleScene = theSceneManager->GetSceneAs<TitleScene>(SceneId::kTitleScene);
	titleScene.mMessageBox.SetMessage(message);
	titleScene.mMessageBox.SetVisible(true);

	titleScene.mMessageBox.SetOkayCallback([&titleScene](){
		titleScene.mMessageBox.SetVisible(false);
	});

	tb_always_log(LogGame::Info() << message);
	theSceneManager->ChangeToScene(SceneId::kTitleScene);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::TitleScene::TitleScene(void) :
	Base3dScene(),
	mMessageBox(""),
	mPracticeButton("Practice", ui::ButtonType::kTitleSecondary),
	mSettingsButton("Settings", ui::ButtonType::kTitleSecondary),
	mExitButton("Exit", ui::ButtonType::kTitleExit),
	mTitleSprite("data/interface/logo_game.png"),
	mSettingsScreenEntity(),
	mFadeInTimer(kMaximumFadeInTime),
	mFadeOutTimer(0),
	mStartGameAction(),
	mQuitGameAction()
{
	AddGraphic(mPracticeButton);
	mPracticeButton.SetCallback([]() {
		RacingScene::SetGameMode(RacingScene::GameMode::Singleplayer);
		theSceneManager->ChangeToScene(SceneId::kRacingScene);
	});

	AddGraphic(mSettingsButton);
	mSettingsButton.SetCallback([this]() {
		AddEntity(mSettingsScreenEntity);
	});

	AddGraphic(mExitButton);
	mExitButton.SetCallback([]() {
		tbGame::GameApplication::MarkForClose();
	});

	mQuitGameAction.AddBinding(tbApplication::tbKeyEscape);
	mStartGameAction.AddBinding(tbApplication::tbKeySpace);
	mStartGameAction.AddBinding(tbApplication::tbMouseLeft);

	AddGraphic(mMessageBox);
	mMessageBox.SetVisible(false);

	AddGraphic(mTitleSprite);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::TitleScene::~TitleScene(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::TitleScene::OnSimulate(void)
{
	ludumdare56_start_timer(TimingChannel::kSimulate);

	if (false == mFadeInTimer.IsZero())
	{
		float percentage = mFadeInTimer.GetPercentageOf(kMaximumFadeInTime);
		tbGraphics::Color fadingColor;
		fadingColor.SetColor(percentage, tbGraphics::ColorPalette::Opaque, tbGraphics::ColorPalette::Transparent);
		SetColor(fadingColor);

		if (true == mFadeInTimer.DecrementStep())
		{
			SetColor(tbGraphics::ColorPalette::Opaque);
		}
	}

	//if (false == mFadeOutTimer.IsZero())
	//{
	//	float percentage = mFadeOutTimer.GetPercentageOf(kMaximumFadeOutTime);
	//	tbGraphics::Color fadingColor;
	//	fadingColor.SetColor(percentage, tbGraphics::ColorPalette::Transparent, tbGraphics::ColorPalette::Opaque);
	//	SetColor(fadingColor);

	//	if (true == mFadeOutTimer.DecrementStep())
	//	{
	//		SetColor(tbGraphics::ColorPalette::Transparent);
	//		LudumDare56::theSceneManager->ChangeToScene(SceneId::kAuthenicationScene);
	//	}
	//}
	//else
	//{
	//	if (true == mStartGameAction.IsPressed())
	//	{
	//		mFadeOutTimer = kMaximumFadeOutTime;
	//	}
	//}

	Base3dScene::OnSimulate();

	ludumdare56_stop_timer(TimingChannel::kSimulate);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::TitleScene::OnUpdate(const float deltaTime)
{
	ludumdare56_start_timer(TimingChannel::kUpdate);

	if (true == mQuitGameAction.IsReleased() && false == mSettingsScreenEntity.IsDisplayingSettings())
	{
		theSceneManager->QuitGame();
	}

	const bool enableButtons = (false == mSettingsScreenEntity.IsDisplayingSettings() && false == mMessageBox.IsVisible());

	mPracticeButton.SetVisible(true);
	mPracticeButton.SetEnabled(true && enableButtons);
	mSettingsButton.SetVisible(true);
	mSettingsButton.SetEnabled(true && enableButtons);
	mExitButton.SetEnabled(enableButtons);

	Base3dScene::OnUpdate(deltaTime);

	{
		const float interfaceScale = UserInterface::InterfaceScale();

		//in css, padding is internal spacing and margin is external. Button to edge screen is external.
		const Vector2 margin(-60.0f, -60.0f);

		//const float buttonOffset = 100.0f;
		const Vector2 buttonOffset(0.0f, -75.0f);
		//const Vector2 settingsOffset(0.0f, margin.y);
		const Vector2 settingsOffset(0.0f, 0.0f);


		mMessageBox.SetOrigin(tbGraphics::kAnchorCenter);
		mMessageBox.SetPosition(tbGraphics::ScreenCenter());
		mMessageBox.SetScale(interfaceScale);

		mTitleSprite.SetOrigin(tbGraphics::kAnchorCenter);
		mTitleSprite.SetPosition(UserInterface::GetAnchorPositionOfInterface(tbGraphics::kAnchorCenter,
			tbMath::Vector2(0.0f, 50.0f) * interfaceScale));
		mTitleSprite.SetScale(interfaceScale);

		mPracticeButton.SetOrigin(tbGraphics::kAnchorBottomRight);
		mPracticeButton.SetPosition(UserInterface::GetAnchorPositionOfInterface(tbGraphics::kAnchorBottomRight,
			(margin + settingsOffset + (buttonOffset * 3)) * interfaceScale));
		mPracticeButton.SetScale(interfaceScale);

		mSettingsButton.SetOrigin(tbGraphics::kAnchorBottomRight);
		mSettingsButton.SetPosition(UserInterface::GetAnchorPositionOfInterface(tbGraphics::kAnchorBottomRight,
			(margin + settingsOffset + (buttonOffset * 2)) * interfaceScale));
		mSettingsButton.SetScale(interfaceScale);

		mExitButton.SetOrigin(tbGraphics::kAnchorBottomRight);
		mExitButton.SetPosition(UserInterface::GetAnchorPositionOfInterface(tbGraphics::kAnchorBottomRight,
			margin * interfaceScale));
		mExitButton.SetScale(interfaceScale);
	}

	ludumdare56_stop_timer(TimingChannel::kUpdate);
	ludumdare56_start_timer(TimingChannel::kRender);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::TitleScene::OnOrthographicRender(void) const
{
	Base3dScene::OnOrthographicRender();
	ludumdare56_stop_timer(TimingChannel::kRender);
	DisplayDeveloperConsole();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::TitleScene::OnInterfaceRender(void) const
{
	RenderTitleBackdrop();
	Base3dScene::OnInterfaceRender();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::TitleScene::RenderTitleBackdrop(void)
{
	//TODO: LudumDare56: 2023-10-24: The UserInterface does not yet follow the screen sizing etc.
	const Vector2 left(0.0f, 600.0f);
	const Vector2 right(1920.0f, 300.0f);
	const Vector2 overOffset(0.0f, 75.0f);
	const float topY = 0.0f;
	const float bottomY = 1080.0f;

	tbGraphics::Quad topQuad(960.0f, 540.0f);
	topQuad.SetPosition(tbGraphics::Quad::kTopLeft, tbMath::Vector2(left.x, topY));
	topQuad.SetPosition(tbGraphics::Quad::kBottomLeft, left);
	topQuad.SetPosition(tbGraphics::Quad::kTopRight, tbMath::Vector2(right.x, topY));
	topQuad.SetPosition(tbGraphics::Quad::kBottomRight, right);
	topQuad.SetColor(tbGraphics::Color(0xFF48198f), tbGraphics::Color(0xFF412e98), tbGraphics::Color(0xFF5771d0), tbGraphics::Color(0xFF5670d0));
	topQuad.Render();

	tbGraphics::Quad bottomQuad(960.0f, 540.0f);
	bottomQuad.SetPosition(tbGraphics::Quad::kTopLeft, left);
	bottomQuad.SetPosition(tbGraphics::Quad::kTopRight, right);
	bottomQuad.SetPosition(tbGraphics::Quad::kBottomLeft, tbMath::Vector2(left.x, bottomY));
	bottomQuad.SetPosition(tbGraphics::Quad::kBottomRight, tbMath::Vector2(right.x, bottomY));
	bottomQuad.SetColor(tbGraphics::Color(0xFF892d80), tbGraphics::Color(0xFFa23492), tbGraphics::Color(0xFF6f256c), tbGraphics::Color(0xFF6f246c));
	bottomQuad.Render();

	tbGraphics::Quad overQuad(960.0f, 540.0f);
	overQuad.SetPosition(tbGraphics::Quad::kTopLeft, left - overOffset);
	overQuad.SetPosition(tbGraphics::Quad::kTopRight, right - overOffset);
	overQuad.SetPosition(tbGraphics::Quad::kBottomLeft, left + overOffset);
	overQuad.SetPosition(tbGraphics::Quad::kBottomRight, right + overOffset);
	overQuad.SetColor(tbGraphics::Color(0xFFff2e9f));
	overQuad.Render();

	// tbGraphics::Sprite boomCar("data/interface/temp_turbo_boom_car.png");
	// boomCar.SetOrigin(tbGraphics::kAnchorCenter);
	// boomCar.SetPosition(tbGraphics::ScreenCenter());
	// boomCar.SetScale(0.75f);
	// boomCar.Render();

	tbGraphics::Sprite timbeaudetLogo("data/interface/logo_blackbird.png");
	timbeaudetLogo.SetOrigin(tbGraphics::kAnchorTopLeft);
	timbeaudetLogo.SetPosition(Vector2(50.0f, 50.0f));
	timbeaudetLogo.SetScale(0.25f);
	timbeaudetLogo.Render();

	tbGraphics::Sprite allovLogo("data/interface/logo_allov.png");
	allovLogo.SetOrigin(tbGraphics::kAnchorTopLeft);
	allovLogo.SetPosition(Vector2(50.0f + 128.0f + 50.0f, 50.0f));
	allovLogo.SetScale(0.25f);
	allovLogo.Render();

	tbGraphics::Sprite studioLogo("data/interface/logo_tyre_bytes.png");
	studioLogo.SetOrigin(tbGraphics::kAnchorTopRight);
	studioLogo.SetPosition(UserInterface::GetAnchorPositionOfInterface(tbGraphics::kAnchorTopRight,
		tbMath::Vector2(-ui::Padding::ScreenEdge, ui::Padding::ScreenEdge)));
	studioLogo.SetScale(0.0625f * ui::InterfaceScale());
	studioLogo.Render();

	tbGraphics::Text versionText(Version::VersionString(), 20.0f);
	versionText.SetColor(tbGraphics::Color(0xFFadadad));
	versionText.SetOrigin(tbGraphics::kAnchorBottomLeft);
	versionText.SetPosition(UserInterface::GetAnchorPositionOfInterface(tbGraphics::kAnchorBottomLeft,
		tbMath::Vector2(ui::Padding::ScreenEdge, -ui::Padding::ScreenEdge)));
	versionText.Render();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::TitleScene::OnOpen(void)
{
	Base3dScene::OnOpen();

	mFadeInTimer = kMaximumFadeInTime;
	mFadeOutTimer = 0;
	SetColor(tbGraphics::ColorPalette::Transparent);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::TitleScene::OnClose(void)
{
	Base3dScene::OnClose();
}

//--------------------------------------------------------------------------------------------------------------------//
