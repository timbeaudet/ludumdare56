///
/// @file
/// @details A simple graphic to give the drivers some fans to watch them.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/graphics_3d/spectator_graphic.hpp"

#include <vector>

namespace
{
	using namespace LudumDare56::GameClient;

	std::vector<std::unique_ptr<SpectatorGraphic>> theSpectators;
};

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SpectatorGraphic::SpawnSpectatorsAt(const Matrix4& bleacherToWorld)
{
	const Vector3 kColumnOffset(2.0f, 0.0f, 0.0f);
	const Vector3 kRowOffset(0.0f, 1.0f, 2.0f);
	const Vector3 kBottomRightSpot(3.98f - 10.0f, 2.76f - 0.0f, -(-18.29f - -20.0f));

	for (int row = 0; row < 3; ++row)
	{
		for (int column = 0; column < 7; ++column)
		{
			const Matrix4 spectatorToWorld = Matrix4::Translation(kBottomRightSpot +
				kColumnOffset * static_cast<float>(column) + kRowOffset * static_cast<float>(row)) * bleacherToWorld;

			theSpectators.emplace_back(new SpectatorGraphic(spectatorToWorld));
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SpectatorGraphic::ClearAllSpectators(void)
{
	theSpectators.clear();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::SpectatorGraphic::UpdateAllSpectators(const float deltaTime)
{
	for (auto& spectator : theSpectators)
	{
		spectator->mBobTimer += deltaTime * 4.0f;
		Matrix4 spectatorToWorld = spectator->mSpectatorGraphic.GetObjectToWorld();
		spectatorToWorld.SetPosition(spectator->mOriginalPosition + Vector3::Up() * std::sin(spectator->mBobTimer) * 0.25f);
		spectator->mSpectatorGraphic.SetObjectToWorld(spectatorToWorld);
	}
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::SpectatorGraphic::SpectatorGraphic(const Matrix4& spectatorToWorld) :
	mSpectatorGraphic(),
	mOriginalPosition(spectatorToWorld.GetPosition()),
	mBobTimer(0.0f + spectatorToWorld.GetPosition().x * spectatorToWorld.GetPosition().z)
{
	const std::vector<tbCore::tbString> spectators{
		"purple", "pink", "green", "blue", "orange"
	};

	const tbCore::tbString randomSpectator = spectators[tbMath::RandomInt() % spectators.size()];

	mSpectatorGraphic.SetObjectToWorld(spectatorToWorld);
	mSpectatorGraphic.SetMesh("data/meshes/spectator_" + randomSpectator + ".msh");
	mSpectatorGraphic.SetMaterial("data/materials/palette64.mat");
	mSpectatorGraphic.SetVisible(true);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::SpectatorGraphic::~SpectatorGraphic(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//
