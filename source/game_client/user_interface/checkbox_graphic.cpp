///
/// @file
/// @details A simple user interface graphic object for the user to control a checkbox, on/off switch.
///   This is NOT part of the TurtleBrains::ExpressKit interface in anyway, it is just a sprite object.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "checkbox_graphic.hpp"

#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"

#include <turtle_brains/game/tb_input_action.hpp>

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::CheckboxGraphic::CheckboxGraphic(void) :
	BaseControlGraphic(),
	mBackdropGraphic("hud_sheet", "nine_slice_round", 60, 60),
	mOutlineGraphic("hud_sheet", "nine_slice_round_outline", 60, 60),
	mIsChecked(false)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::CheckboxGraphic::~CheckboxGraphic(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

tbGraphics::PixelSpace LudumDare56::GameClient::UserInterface::CheckboxGraphic::GetPixelWidth(void) const
{
	return mBackdropGraphic.GetPixelWidth();
}

//--------------------------------------------------------------------------------------------------------------------//

tbGraphics::PixelSpace LudumDare56::GameClient::UserInterface::CheckboxGraphic::GetPixelHeight(void) const
{
	return mBackdropGraphic.GetPixelHeight();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::CheckboxGraphic::SetChecked(const bool isChecked)
{
	mIsChecked = isChecked;

	if (true == mIsChecked)
	{
		mBackdropGraphic.SetColor(ui::Color::White);
	}
	else
	{
		mBackdropGraphic.SetColor(ui::Color::DarkBackdrop); //or Invisible?
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::CheckboxGraphic::OnUpdate(const float deltaTime)
{
	mBackdropGraphic.Update(deltaTime);
	mOutlineGraphic.Update(deltaTime);

	if (true == IsEnabled() && true == IsVisible() && true == IsMouseContained())
	{
		if (true == tbGame::Input::IsKeyPressed(tbApplication::tbMouseLeft))
		{
			SetChecked(!IsChecked());

			if (mCallback)
			{
				mCallback();
			}
		}
		else
		{
			mOutlineGraphic.SetColor(ui::Color::ControlHovered);
		}
	}
	else if (true == IsEnabled())
	{
		mOutlineGraphic.SetColor(ui::Color::TyreBytesBlue);
	}
	else //false == mIsEnabled
	{
		mOutlineGraphic.SetColor(ui::Color::ControlDisabled);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::CheckboxGraphic::OnRender(void) const
{
	mBackdropGraphic.Render();
	mOutlineGraphic.Render();
}

//--------------------------------------------------------------------------------------------------------------------//
