///
/// @file
/// @details Displays the most recent laptime for a few moments for the player to see what they achieved.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/entities_2d/lap_time_popup_entity.hpp"
#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"
#include "../../game_state/racecar_state.hpp"
#include "../../core/utilities.hpp"

#include <turtle_brains/express/behaviors/tbx_basic_behaviors.hpp>

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::LapTimePopupEntity::LapTimePopupEntity(const tbCore::uint32 lapTime) :
	tbGame::Entity("LapTimePopup"),
	mLapTimeText(tbCore::String::TimeToString(lapTime), 70.0f)
{
	AddGraphic(mLapTimeText);

	PushBehavior(new tbxBehaviors::KillBehavior(*this));
	PushBehavior(new tbxBehaviors::DelayBehavior(*this, 4000));
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::LapTimePopupEntity::~LapTimePopupEntity(void)
{
}


//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::LapTimePopupEntity::OnUpdate(const float /*deltaTime*/)
{
	const float interfaceScale = ui::InterfaceScale();

	mLapTimeText.SetOrigin(tbGraphics::kAnchorTopCenter);
	mLapTimeText.SetPosition(ui::GetAnchorPositionOfInterface(tbGraphics::kAnchorTopCenter, 0.0f, 50.0f * interfaceScale));
	mLapTimeText.SetScale(interfaceScale);
}

//--------------------------------------------------------------------------------------------------------------------//
