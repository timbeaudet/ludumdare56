///
/// @file
/// @details Displays the most recent laptime for a few moments for the player to see what they achieved.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/entities_2d/player_standings_entity.hpp"
#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"
#include "../../game_state/timing_and_scoring_state.hpp"
#include "../../game_state/racecar_state.hpp"
#include "../../core/utilities.hpp"

#include <turtle_brains/express/behaviors/tbx_basic_behaviors.hpp>

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::PlayerStandingsEntity::PlayerStandingsEntity(const GameState::RacecarIndex racecarIndex) :
	tbGame::Entity("LapTimePopup"),
	mStandingsText("", 70.0f),
	mRacecarIndex(racecarIndex)
{
	AddGraphic(mStandingsText);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::PlayerStandingsEntity::~PlayerStandingsEntity(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::PlayerStandingsEntity::SetRacecarIndex(const GameState::RacecarIndex racecarIndex)
{
	mRacecarIndex = racecarIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::PlayerStandingsEntity::OnSimulate(void)
{
	if (false == GameState::IsValidRacecar(mRacecarIndex))
	{
		mStandingsText.SetVisible(false);
		return;
	}

	int runningCars = 0;
	for (const GameState::RacecarState& racecar : GameState::RacecarState::AllRacecars())
	{
		if (true == racecar.IsRacecarInUse())
		{
			++runningCars;
		}
	}

	const int standings = GameState::TimingState::GetRaceStandingsFor(mRacecarIndex);
	mStandingsText.SetVisible(0 != standings);
	mStandingsText.SetText(tb_string(standings) + "/" + tb_string(runningCars));
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::PlayerStandingsEntity::OnUpdate(const float /*deltaTime*/)
{
	const float interfaceScale = ui::InterfaceScale();

	mStandingsText.SetOrigin(tbGraphics::kAnchorTopRight);
	mStandingsText.SetPosition(ui::GetAnchorPositionOfInterface(tbGraphics::kAnchorTopRight, -50.0f * interfaceScale, 50.0f * interfaceScale));
	mStandingsText.SetScale(interfaceScale);
}

//--------------------------------------------------------------------------------------------------------------------//
