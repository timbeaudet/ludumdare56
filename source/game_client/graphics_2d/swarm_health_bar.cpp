///
/// @file
/// @details Displays a health bar for the swarm of ant drivers.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/graphics_2d/swarm_health_bar.hpp"

#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"

#include "../../game_state/racecar_state.hpp"

using icePhysics::Scalar;

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::SwarmHealthBar::SwarmHealthBar(const GameState::RacecarIndex racecarIndex) :
	tbGraphics::Graphic(),
	mRacecarIndex(racecarIndex),
	mHealthBar()
{
	mHealthBar.SetTotal(GameState::RacecarState::kNumberOfCreatures);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::SwarmHealthBar::~SwarmHealthBar(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SwarmHealthBar::SetRacecarIndex(const GameState::RacecarIndex racecarIndex)
{
	mRacecarIndex = racecarIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SwarmHealthBar::OnUpdate(const float deltaTime)
{
	if (true == GameState::IsValidRacecar(mRacecarIndex))
	{
		SetVisible(true);

		const GameState::RacecarState::CreatureIndex total = GameState::RacecarState::kNumberOfCreatures;
		const GameState::RacecarState::CreatureIndex health = GameState::RacecarState::Get(mRacecarIndex).GetSwarmHealth();
		const GameState::RacecarState::CreatureIndex min = GameState::RacecarState::kMinimumCreatures;;

		mHealthBar.SetTotal(total - min);
		mHealthBar.SetCount(health - min);

		const float interfaceScale = ui::InterfaceScale();
		mHealthBar.SetScale(interfaceScale);
		mHealthBar.SetOrigin(tbGraphics::kAnchorTopCenter);
		//mHealthBar.SetPosition(ui::GetAnchorPositionOfInterface(tbGraphics::kAnchorBottomCenter, Vector2(0.0f, -50.0f) * interfaceScale));
		mHealthBar.SetPosition(ui::GetAnchorPositionOfInterface(tbGraphics::kAnchorTopCenter, Vector2(0.0f, 50.0f) * interfaceScale));
		mHealthBar.SetSize(800, 80);

		if (health > (total - min) / 2)
		{
			mHealthBar.SetFillColor(tbGraphics::ColorPalette::Green);
		}
		else if (health > (total - min) / 4)
		{
			mHealthBar.SetFillColor(tbGraphics::ColorPalette::Yellow);
		}
		else if (health > (total - min) / 6)
		{
			mHealthBar.SetFillColor(tbGraphics::ColorPalette::MonkyOrange);
		}
		else
		{
			mHealthBar.SetFillColor(tbGraphics::ColorPalette::Red);
		}

		mHealthBar.Update(deltaTime);
	}
	else
	{
		SetVisible(false);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SwarmHealthBar::OnRender(void) const
{
	//const GameState::RacecarState& racecar = GameState::RacecarState::Get(mRacecarIndex);

	mHealthBar.Render();

	//tbMath::Vector3 flatVelocity = tbMath::Vector3(racecar.GetLinearVelocity());
	//flatVelocity.y = 0.0f;

	//const float interfaceScale = ui::InterfaceScale();

	//const bool isMetric(false);
	//const Scalar vehicleSpeed = flatVelocity.Magnitude();
	//const Scalar convertedSpeed = (isMetric) ? tbMath::Convert::MeterSecondToKilometerHour(vehicleSpeed) : tbMath::Convert::MeterSecondToMileHour(vehicleSpeed);
	//const tbCore::tbString speedyUnits = (isMetric) ? "km/h" : "mph";

	//const tbMath::Vector2 tachometerPosition(250.0f * interfaceScale, ui::TargetHeight() - 250.0f * interfaceScale);
	//const tbMath::Vector2 speedOffset(350.0f, 150.0f);

	////tbxGraphics::ShadowedText speedText(tbCore::ToString(convertedSpeed) + " " + speedyUnits, 70.0f);
	//tbGraphics::Text speedText(tbCore::ToString(convertedSpeed) + " " + speedyUnits, 70.0f * interfaceScale);
	//speedText.SetOrigin(tbGraphics::kAnchorBottomCenter);
	//speedText.SetPosition(tachometerPosition + speedOffset * interfaceScale);
	//speedText.SetScale(interfaceScale);
	//speedText.Render();
}

//--------------------------------------------------------------------------------------------------------------------//
