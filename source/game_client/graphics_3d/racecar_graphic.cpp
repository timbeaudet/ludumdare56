///
/// @file
/// @details A simple graphic to display the Racecars.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/graphics_3d/racecar_graphic.hpp"
#include "../../game_state/racecar_state.hpp"
#include "../../network/network_manager.hpp"
#include "../../network/networked_racecar_controller.hpp"
#include "../../logging.hpp"

bool LudumDare56::GameClient::RacecarGraphic::sDisplayCarNumbers = true;

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::RacecarGraphic::RacecarGraphic(void) :
	mRacecarIndex(0),
	mRacecarMeshID(0),
	mRacecarGraphic(),
	mWheelGraphics(),
	mCreatureGraphics(),
	mLagText("LAG", 15.0f),
	mCarText("", 20.0f)
{
	//mRacecarGraphic.SetMesh(GameState::RacecarState::GetCarFilepath(0));
	//mRacecarGraphic.SetMesh("data/meshes/racecars/indicator.msh");

	mRacecarGraphic.SetMaterial("data/materials/palette256.mat");
	for (iceGraphics::Graphic& wheelGraphic : mWheelGraphics)
	{
		//wheelGraphic.SetMesh("data/meshes/racecars/wheel_fancy.msh");
		mRacecarGraphic.AddGraphic(wheelGraphic);
	}

	for (iceGraphics::Graphic& creatureGraphic : mCreatureGraphics)
	{
		creatureGraphic.SetMesh(GameState::RacecarState::GetCarFilepath(
			tbCore::RangedCast<tbCore::uint8>(rand() % GameState::RacecarState::GetAvailableCars(false, false).size())
		));
		creatureGraphic.SetMaterial("data/materials/palette256.mat");
	}
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::RacecarGraphic::~RacecarGraphic(void)
{
	for (iceGraphics::Graphic& wheelGraphic : mWheelGraphics)
	{
		mRacecarGraphic.RemoveGraphic(&wheelGraphic);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacecarGraphic::SetRacecarIndex(tbCore::uint8 racecarIndex)
{
	mRacecarIndex = racecarIndex;
	mCarText.SetText(tb_string(static_cast<int>(mRacecarIndex) + 1));
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacecarGraphic::Update(const float /*deltaTime*/)
{
	const GameState::RacecarState& racecar = GameState::RacecarState::Get(mRacecarIndex);

	const Matrix4 vehicleToWorld = static_cast<Matrix4>(racecar.GetBodyToWorld());
	mRacecarGraphic.SetObjectToWorld(vehicleToWorld);
	mRacecarGraphic.SetVisible(racecar.IsRacecarInUse());

	{
		if (mRacecarMeshID != racecar.GetRacecarMeshID())
		{
			mRacecarMeshID = racecar.GetRacecarMeshID();
			SetRacecarMesh(GameState::RacecarState::GetCarFilepath(mRacecarMeshID));
		}

		if (true == racecar.IsRacecarInUse())
		{
			mLagText.SetVisible(false);

			const Network::NetworkedRacecarController* networkController = dynamic_cast<const Network::NetworkedRacecarController*>(&racecar.GetRacecarController());
			if (nullptr != networkController)
			{
				const tbCore::uint32 timeSinceLastUpdate = networkController->GetLastUpdateTimer();

				if (timeSinceLastUpdate > 250)
				{
					mLagText.SetText(tb_string(timeSinceLastUpdate) + "ms");
					//TODO: LudumDare56: 2023-09-08: 2D-3D need to updates this here.
					//mLagText.SetPosition(racecar.mRacecar.GetPosition() - tbMath::Vector3(mLagText.GetWidth() / 2.0f, 40.0f));

					tb_debug_log(LogGame::Always() << DebugInfo(racecar) << " is lagging: " << mLagText.GetText());
					mLagText.SetVisible(true);
				}
			}
		}
		else
		{
			mLagText.SetVisible(false);
		}
	}

	CreatureIndex creatureIndex = 0;
	for (iceGraphics::Graphic& creatureGraphic : mCreatureGraphics)
	{
		const tbMath::Matrix4 creatureToWorld = static_cast<tbMath::Matrix4>(racecar.GetCreatureToWorld(creatureIndex));
		creatureGraphic.SetObjectToWorld(creatureToWorld);
		++creatureIndex;
	}

	size_t wheelIndex = 0;
	for (iceGraphics::Graphic& wheelGraphic : mWheelGraphics)
	{
		const tbMath::Matrix4 wheelToWorld = static_cast<tbMath::Matrix4>(racecar.GetWheelToWorld(wheelIndex));
		wheelGraphic.SetObjectToWorld(wheelToWorld);
		++wheelIndex;
	}

	////TODO: TurtleBrains: This didn't seem to place the text at the center of the sprite, when using sprite.GetPosition().
	////mCarText.SetOrigin(tbGraphics::AnchorLocation::kAnchorCenter);
	////mCarText.SetPosition(mSprite.GetPosition());
	//mCarText.SetPosition(mSprite.GetPosition() - tbMath::Vector2(mCarText.GetWidth() / 2.0f, 20.0f));
	//mCarText.SetVisible(mSprite.IsVisible() && sDisplayCarNumbers);
}

//--------------------------------------------------------------------------------------------------------------------//
