///
/// @file
/// @details A single lined text label for some of the user interface parts in LudumDare56.
/// @history This file was original started in Trailing Brakes Racing Simulator in 2022.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "label_graphic.hpp"

#include "../../game_client/user_interface/user_interface_constants.hpp"

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::LabelGraphic::LabelGraphic(const tbCore::tbString& labelText) :
	BaseControlGraphic(),
	mLabelText(labelText)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::LabelGraphic::~LabelGraphic(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::LabelGraphic::SetText(const tbCore::tbString& labelText)
{
	mLabelText.SetText(labelText);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::LabelGraphic::OnRender(void) const
{
	mLabelText.Render();
}

//--------------------------------------------------------------------------------------------------------------------//
