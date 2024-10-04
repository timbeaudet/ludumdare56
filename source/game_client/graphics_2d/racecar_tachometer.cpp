///
/// @file
/// @details Displays a tachometer for the engine speed of a racecar in LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "racecar_tachometer.hpp"

#include "../../game_state/racecar_state.hpp"

#include "../../game_client/user_interface/user_interface_constants.hpp"
#include "../../game_client/user_interface/user_interface_helpers.hpp"

//#include "../../game_client/scenes/driving_scene.hpp"
//#include "../../game_client/user_settings.hpp"

#include <turtle_brains/graphics/tb_basic_shapes.hpp>

using icePhysics::Scalar;

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::RacecarTachometer::RacecarTachometer(const GameState::RacecarIndex racecarIndex) :
	tbGraphics::Graphic(),
	mTachometerSprite("data/interface/basic_tachometer.png"),
	mNeedleSprite("data/interface/basic_tachometer_needle.png"),
	mRacecarIndex(racecarIndex)
{
	mTachometerSprite.SetOrigin(tbGraphics::kAnchorCenter);
	mNeedleSprite.SetOrigin(14.5f, 240 - 14.5f);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::RacecarTachometer::~RacecarTachometer(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacecarTachometer::SetRacecarIndex(const GameState::RacecarIndex racecarIndex)
{
	mRacecarIndex = racecarIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacecarTachometer::OnUpdate(const float /*deltaTime*/)
{
	if (true == GameState::IsValidRacecar(mRacecarIndex))
	{
		mTachometerSprite.SetPosition(250.0f * ui::InterfaceScale(), ui::TargetHeight() - 250.0f * ui::InterfaceScale());
		mTachometerSprite.SetScale(0.75f * ui::InterfaceScale());
		//mTachometerSprite.SetPosition(512.0f - 50.0f, 256.0f);

		const float engineSpeed = static_cast<float>(GameState::RacecarState::Get(mRacecarIndex).GetEngineSpeed());
		mNeedleSprite.SetRotation(ComputeNeedleRotationForRPM(engineSpeed).AsDegrees());
		mNeedleSprite.SetPosition(mTachometerSprite.GetPosition());
		mNeedleSprite.SetScale(0.75f * ui::InterfaceScale());

		SetVisible(true);
	}
	else
	{
		SetVisible(false);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacecarTachometer::OnRender(void) const
{
	const GameState::RacecarState& racecar = GameState::RacecarState::Get(mRacecarIndex);

	mTachometerSprite.Render();
	mNeedleSprite.Render();

	const GameState::Gear currentGear = racecar.GetShifterPosition();
	tbCore::tbString gearName = "N";
	if (currentGear > GameState::Gear::Sixth)
	{
		gearName = "R";
	}
	else if (currentGear > GameState::Gear::Neutral)
	{
		gearName = tbCore::ToString(static_cast<int>(currentGear));
	}

	tbMath::Vector2 gearOffset(80.0f, 80.0f);
	//tbxGraphics::ShadowedText gearText(gearName, 60.0f);
	tbGraphics::Text gearText(gearName, 60.0f * ui::InterfaceScale());
	gearText.SetColor(tbGraphics::ColorPalette::Black);
	gearText.SetOrigin(tbGraphics::kAnchorBottomCenter);
	gearText.SetPosition(mTachometerSprite.GetPosition() + gearOffset);
	gearText.Render();
}

//--------------------------------------------------------------------------------------------------------------------//

tbMath::Angle LudumDare56::GameClient::RacecarTachometer::ComputeNeedleRotationForRPM(const float revolutionsPerMinute) const
{
	return 180.0_degrees - ((revolutionsPerMinute / 1000.0f) * 30.0_degrees);
}

//--------------------------------------------------------------------------------------------------------------------//
