///
/// @file
/// @details Displays a health bar for the swarm of ant drivers.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/graphics_2d/win_lose_screen_graphic.hpp"

#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"
#include "../../game_client/scenes/scene_manager.hpp"

#include "../../game_state/racecar_state.hpp"
#include "../../game_state/racetrack_state.hpp"

#include "../../logging.hpp"

using icePhysics::Scalar;

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::WinLoseScreenGraphic::WinLoseScreenGraphic(const GameState::RacecarIndex racecarIndex) :
	tbGraphics::Graphic(),
	mYouWinText("You Won!", 120.0f),
	mYouLoseText("You Lost!", 120.0f),
	mRetryButton("Retry (space)"),
	mNextButton("Next (enter)"),
	mRacecarIndex(racecarIndex),
	mState(State::None)
{
	mRetryButton.SetCallback([this](){
		//This seemed to PlaceCarOnGrid and Reset, so I used it instead of digging for grid position...
		GameState::RaceSessionState::PlaceCarOnGrid(GameState::RacecarState::GetMutable(mRacecarIndex));
		//const GridIndex gridIndex = GetGridIndexFor(racecar.GetRacecarIndex());
		//GameState::RacecarState::GetMutable(mRacecarIndex).ResetRacecar(GameState::RacetrackState::GetGridToWorld(
	});

	mNextButton.SetCallback([this](){
		GotoNextLevel();
	});
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::WinLoseScreenGraphic::~WinLoseScreenGraphic(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::WinLoseScreenGraphic::GotoNextLevel(void)
{
	tb_debug_log(LogClient::Info() << "Changing to NextLevelScene.");
	theSceneManager->ChangeToScene(SceneId::kNextLevelScene);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::WinLoseScreenGraphic::SetRacecarIndex(const GameState::RacecarIndex racecarIndex)
{
	mRacecarIndex = racecarIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::WinLoseScreenGraphic::OnUpdate(const float deltaTime)
{
	if (true == GameState::IsValidRacecar(mRacecarIndex))
	{
		const GameState::RacecarState& racecar = GameState::RacecarState::Get(mRacecarIndex);
		const float interfaceScale = ui::InterfaceScale();

		mNextButton.SetOrigin(tbGraphics::kAnchorCenterRight);
		mNextButton.SetPosition(ui::GetAnchorPositionOfInterface(tbGraphics::kAnchorCenter, Vector2(-20.0f, 0.0f) * interfaceScale));
		mNextButton.SetScale(interfaceScale);

		mRetryButton.SetOrigin(tbGraphics::kAnchorCenterLeft);
		mRetryButton.SetPosition(ui::GetAnchorPositionOfInterface(tbGraphics::kAnchorCenter, Vector2(20.0f, 0.0f) * interfaceScale));
		mRetryButton.SetScale(interfaceScale);

		switch (mState)
		{
		case State::None: {
			SetVisible(false);
			mRetryButton.SetVisible(false);
			mNextButton.SetVisible(false);

			if (true == racecar.HasWon())
			{
				mState = State::Win;
			}
			else if (true == racecar.HasLost())
			{
				mState = State::Lose;
			}
			break; }

		case State::Win: {
			SetVisible(true);
			mRetryButton.SetVisible(true);
			mRetryButton.Update(deltaTime);

			mNextButton.SetVisible(true);
			mNextButton.Update(deltaTime);

			mYouWinText.SetOrigin(tbGraphics::kAnchorBottomCenter);
			mYouWinText.SetPosition(ui::GetAnchorPositionOfInterface(tbGraphics::kAnchorCenter, Vector2(0.0f, -50.0f) * interfaceScale));
			mYouWinText.SetScale(interfaceScale);

			if (true == tbApplication::Input::IsKeyPressed(tbApplication::tbKeyEnter) ||
				true == tbApplication::Input::IsKeyPressed(tbApplication::tbKeyNumpadEnter))
			{
				GotoNextLevel();
			}

			break; }

		case State::Lose: {
			SetVisible(true);
			mRetryButton.SetVisible(true);
			mRetryButton.Update(deltaTime);

			mNextButton.SetVisible(false);

			mYouLoseText.SetOrigin(tbGraphics::kAnchorBottomCenter);
			mYouLoseText.SetPosition(ui::GetAnchorPositionOfInterface(tbGraphics::kAnchorCenter, Vector2(0.0f, -50.0f) * interfaceScale));
			mYouLoseText.SetScale(interfaceScale);

			break; }
		};

		if (false == racecar.HasWon() && false == racecar.HasLost())
		{
			mState = State::None;
		}
	}
	else
	{
		SetVisible(false);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::WinLoseScreenGraphic::OnRender(void) const
{
	switch (mState)
	{
	case State::Win: {
		mYouWinText.Render();
		mNextButton.Render();
		mRetryButton.Render();
		break; }

	case State::Lose: {
		mYouLoseText.Render();
		mNextButton.Render();
		mRetryButton.Render();
		break; }
	};
}

//--------------------------------------------------------------------------------------------------------------------//
