///
/// @file
/// @details Displays the most recent laptime for a few moments for the player to see what they achieved.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/entities_2d/start_procedure_lights_entity.hpp"
#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"
#include "../../game_state/race_session_state.hpp"

#include <turtle_brains/game/tb_entity_manager.hpp>
#include <turtle_brains/express/behaviors/tbx_basic_behaviors.hpp>

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::StartProcedureLightsEntity::StartProcedureLightsEntity(void) :
	tbGame::Entity("StartProcedureLights"),
	mCountdownText("", 130.0f),
	mAliveTimer(0)
{
	AddGraphic(mCountdownText);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::StartProcedureLightsEntity::~StartProcedureLightsEntity(void)
{
}


//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::StartProcedureLightsEntity::OnSimulate(void)
{
	if (GameState::RaceSessionState::SessionPhase::kPhaseGrid != GameState::RaceSessionState::GetSessionPhase())
	{
		if (true == mAliveTimer.DecrementStep())
		{
			GetEntityManager()->RemoveEntity(this);
		}
	}

	const tbCore::uint32 startTimer = GameState::RaceSessionState::GetPhaseTimer();
	mCountdownText.SetVisible(startTimer < 3000);

	if (startTimer > 2000) { mCountdownText.SetText("3"); }
	else if (startTimer > 1000) { mCountdownText.SetText("2"); }
	else if (startTimer > 50) { mCountdownText.SetText("1"); }
	else
	{
		mCountdownText.SetText("GO!");

		if (true == mAliveTimer.IsZero())
		{
			mAliveTimer = 1000;
		}
	}

	const float interfaceScale = ui::InterfaceScale();

	mCountdownText.SetOrigin(tbGraphics::kAnchorCenter);
	mCountdownText.SetPosition(ui::GetAnchorPositionOfInterface(tbGraphics::kAnchorCenter));
	mCountdownText.SetScale(interfaceScale);
}

//--------------------------------------------------------------------------------------------------------------------//
