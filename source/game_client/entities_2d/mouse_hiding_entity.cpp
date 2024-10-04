///
/// @file
/// @details A simple entity/object to automatically hide the mouse when not used.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "mouse_hiding_entity.hpp"

#include <turtle_brains/application/tb_application_input.hpp>
#include <turtle_brains/game/tb_game_application.hpp>

namespace
{
	const float kTimeToHideMouse = 2.0f;
};

namespace LudumDare56
{
	namespace GameClient
	{
		//This is found in ludumdare56.cpp
		extern tbGame::GameApplication* theGameApplication;
	};
};

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::MouseHidingEntity::MouseHidingEntity(void) :
	tbGame::Entity("MouseHidingEntity"),
	mAutoHideTimer(kTimeToHideMouse)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::MouseHidingEntity::~MouseHidingEntity(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::MouseHidingEntity::OnAdd(void)
{
	tbGame::Entity::OnAdd();

	theGameApplication->ShowMouseCursor(false);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::MouseHidingEntity::OnRemove(void)
{
	tbGame::Entity::OnRemove();

	theGameApplication->ShowMouseCursor(true);

}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::MouseHidingEntity::OnUpdate(const float deltaTime)
{
	tbGame::Entity::OnUpdate(deltaTime);

	int mouseMoveX = 0;
	int mouseMoveY = 0;
	if (true == tbApplication::Input::GetMouseMovement(&mouseMoveX, &mouseMoveY))
	{
		theGameApplication->ShowMouseCursor(true);
		mAutoHideTimer = kTimeToHideMouse;
	}

	if (mAutoHideTimer >= 0.0f)
	{
		mAutoHideTimer -= deltaTime;
		if (mAutoHideTimer < 0.0f)
		{
			theGameApplication->ShowMouseCursor(false);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//
