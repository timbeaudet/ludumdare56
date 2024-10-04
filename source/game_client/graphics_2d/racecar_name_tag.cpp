///
/// @file
/// @details A name tag that floats over each of the racecars in the simulator to identify who is who.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "racecar_name_tag.hpp"

#include "../../game_state/racecar_state.hpp"
#include "../../game_state/driver_state.hpp"
#include "../../game_client/scenes/racing_scene.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"

#include <turtle_brains/graphics/tb_basic_shapes.hpp>

//--------------------------------------------------------------------------------------------------------------------//

namespace
{
	tbMath::Vector3 WorldPositionToScreenPosition(const tbMath::Vector3& worldPosition, const tbMath::Matrix4& mvp)
	{
		const tbMath::Vector4 world(worldPosition.x, worldPosition.y, worldPosition.z, 1.0f);
		tbMath::Vector4 screen = world * mvp;
		screen.x /= screen.w;
		screen.y /= screen.w;
		screen.x = screen.x * 0.5f + 0.5f;
		screen.y = -screen.y * 0.5f + 0.5f;
		//Screen x and y are 0 to 1

		return tbMath::Vector3(
			screen.x * tbGraphics::ScreenWidth(),
			screen.y * tbGraphics::ScreenHeight(),
			screen.z
		);
	}
};

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::RacecarNameTag::RacecarNameTag(const GameState::RacecarIndex& racecarIndex) :
	tbGraphics::Graphic(),
	mRacecarIndex(racecarIndex),
	mNameText(""),
	mHitConeTimer(-1.0f),
	mIsOnTrack(false)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::RacecarNameTag::~RacecarNameTag(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacecarNameTag::OnUpdate(const float deltaTime)
{
	const GameState::RacecarState& racecar = GameState::RacecarState::Get(mRacecarIndex);
	if (false == racecar.IsRacecarInUse())
	{
		mHitConeTimer = -1.0f;
		mIsOnTrack = false;
		SetVisible(false);
		return;
	}

	if (mRacecarIndex == RacingScene::GetPlayerRacecarIndex())
	{
		SetVisible(false);
		return;
	}

	const GameState::DriverState& driver = GameState::DriverState::Get(racecar.GetDriverIndex());

	SetVisible(true);

	bool activelyHitCone = false;
	if (mHitConeTimer >= 0.0f)
	{
		mHitConeTimer -= deltaTime;
		activelyHitCone = true;
	}

	const tbMath::Vector3 racecarPosition = tbMath::Vector3(racecar.GetVehicleToWorld().GetPosition());
	const tbMath::Vector3 aboveCar = racecarPosition + tbMath::Vector3(0.0f, 1.5f, 0.0f);
	const tbMath::Vector3 namePosition = WorldPositionToScreenPosition(aboveCar, RacingScene::GetWorldToProjection());

	const float textSize = 30.0f - (namePosition.z / 4.0f);
	if (namePosition.z >= 0.0f && textSize > 5.0f)
	{
		tbGraphics::Color nameColor = ui::Color::White;
		if (true == activelyHitCone)
		{
			nameColor = ui::Color::PenaltyText;
		}
		else if (true == mIsOnTrack)
		{
			nameColor = ui::Color::DriverOnTrack;
		}

		const tbCore::tbString id = tb_string(static_cast<int>(racecar.GetRacecarIndex())) + ". ";
		mNameText.SetText(id + driver.GetName(), 30.0f, "");
		mNameText.SetColor(nameColor);
		mNameText.SetScale(textSize / 30.0f);
		mNameText.SetOrigin(tbGraphics::kAnchorCenter);
		mNameText.SetPosition(namePosition.x, namePosition.y);
		mNameText.SetVisible(true);
	}
	else
	{
		mNameText.SetVisible(false);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacecarNameTag::OnRender(void) const
{
	mNameText.Render();
}

//--------------------------------------------------------------------------------------------------------------------//
