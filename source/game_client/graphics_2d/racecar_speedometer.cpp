///
/// @file
/// @details Displays a speedometer to show how fast a racecar is moving in LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "racecar_speedometer.hpp"

#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"

#include "../../game_state/racecar_state.hpp"

//#include "../../game_client/scenes/driving_scene.hpp"
//#include "../../game_client/user_settings.hpp"

#include <turtle_brains/graphics/tb_basic_shapes.hpp>

using icePhysics::Scalar;

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::RacecarSpeedometer::RacecarSpeedometer(const GameState::RacecarIndex racecarIndex) :
	tbGraphics::Graphic(),
	mRacecarIndex(racecarIndex)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::RacecarSpeedometer::~RacecarSpeedometer(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacecarSpeedometer::SetRacecarIndex(const GameState::RacecarIndex racecarIndex)
{
	mRacecarIndex = racecarIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacecarSpeedometer::OnUpdate(const float /*deltaTime*/)
{
	if (true == GameState::IsValidRacecar(mRacecarIndex))
	{
		SetVisible(true);
	}
	else
	{
		SetVisible(false);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacecarSpeedometer::OnRender(void) const
{
	const GameState::RacecarState& racecar = GameState::RacecarState::Get(mRacecarIndex);

	tbMath::Vector3 flatVelocity = tbMath::Vector3(racecar.GetLinearVelocity());
	flatVelocity.y = 0.0f;

	const float interfaceScale = ui::InterfaceScale();

	const bool isMetric(false);
	const Scalar vehicleSpeed = flatVelocity.Magnitude();
	const Scalar convertedSpeed = (isMetric) ? tbMath::Convert::MeterSecondToKilometerHour(vehicleSpeed) : tbMath::Convert::MeterSecondToMileHour(vehicleSpeed);
	const tbCore::tbString speedyUnits = (isMetric) ? "km/h" : "mph";

	const tbMath::Vector2 tachometerPosition(250.0f * interfaceScale, ui::TargetHeight() - 250.0f * interfaceScale);
	const tbMath::Vector2 speedOffset(350.0f, 150.0f);

	//tbxGraphics::ShadowedText speedText(tbCore::ToString(convertedSpeed) + " " + speedyUnits, 70.0f);
	tbGraphics::Text speedText(tbCore::ToString(convertedSpeed) + " " + speedyUnits, 70.0f * interfaceScale);
	speedText.SetOrigin(tbGraphics::kAnchorBottomCenter);
	speedText.SetPosition(tachometerPosition + speedOffset * interfaceScale);
	speedText.SetScale(interfaceScale);
	speedText.Render();
}

//--------------------------------------------------------------------------------------------------------------------//
