///
/// @file
/// @details Provides base functionality for an element / control that the user can interact with.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "base_control_graphic.hpp"

#include "../../game_client/user_interface/user_interface_constants.hpp"

#include <turtle_brains/game/tb_input_action.hpp>

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::BaseControlGraphic::BaseControlGraphic(void) :
	TurtleBrains::Graphics::Graphic(),
	mCallback(),
	mParentOffset(),
	mParentScale(1.0f, 1.0f),
	mIsEnabled(true)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::BaseControlGraphic::~BaseControlGraphic(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::BaseControlGraphic::SetCallback(std::function<void()> callbackFunction)
{
	mCallback = callbackFunction;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::BaseControlGraphic::SetParentOffset(const tbMath::Vector2& parentOffset,
	const tbMath::Vector2& parentScale)
{
	mParentOffset = parentOffset;
	mParentScale = parentScale;
}

//--------------------------------------------------------------------------------------------------------------------//

tbMath::Vector2 LudumDare56::GameClient::UserInterface::BaseControlGraphic::PointInParentSpace(const tbMath::Vector2& pointInGrandparentSpace) const
{
	tbMath::Vector2 pointInParentSpace = pointInGrandparentSpace - mParentOffset;
	pointInParentSpace.x /= mParentScale.x;
	pointInParentSpace.y /= mParentScale.y;
	return pointInParentSpace;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameClient::UserInterface::BaseControlGraphic::IsPointContained(const tbMath::Vector2& point) const
{
	return UnstableIsPointContained(PointInParentSpace(point));
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameClient::UserInterface::BaseControlGraphic::IsMouseContained(void) const
{
	const tbMath::Vector2 mousePosition = tbGame::Input::GetMousePosition();
	return IsPointContained(mousePosition);
}

//--------------------------------------------------------------------------------------------------------------------//
