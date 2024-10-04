///
/// @file
/// @details A simple user interface graphic object for the user to control a sliding bar, like music volume slider.
///   This is NOT part of the TurtleBrains::ExpressKit interface in anyway, it is just a sprite object.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/user_interface/slider_bar_graphic.hpp"
#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"

#include <turtle_brains/game/tb_input_action.hpp>

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::SliderBarGraphic::SliderBarGraphic(void) :
	BaseControlGraphic(),
	mSliderTrackGraphic(256.0f, 8.0f),
	mSliderHandleGraphic(16.0f, 32.0f, tbGraphics::ColorPalette::MonkyBlue),
	mSliderPercentage(0.5f)
{
	mSliderTrackGraphic.SetOrigin(tbGraphics::kAnchorCenterLeft);
	mSliderHandleGraphic.SetOrigin(tbGraphics::kAnchorTopCenter);

	SetSliderPercentage(0.5f);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::SliderBarGraphic::~SliderBarGraphic(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

tbGraphics::PixelSpace LudumDare56::GameClient::UserInterface::SliderBarGraphic::GetPixelWidth(void) const
{
	return mSliderTrackGraphic.GetPixelWidth();
}

//--------------------------------------------------------------------------------------------------------------------//

tbGraphics::PixelSpace LudumDare56::GameClient::UserInterface::SliderBarGraphic::GetPixelHeight(void) const
{
	return mSliderHandleGraphic.GetPixelHeight();
}

//--------------------------------------------------------------------------------------------------------------------//

float LudumDare56::GameClient::UserInterface::SliderBarGraphic::GetSliderPercentage(void) const
{
	return mSliderPercentage;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::SliderBarGraphic::SetSliderPercentage(const float percentage)
{
	mSliderPercentage = tbMath::Clamp(percentage, 0.0f, 1.0f);
	mSliderTrackGraphic.SetPosition(0.0f, GetHeight() / 2.0f);
	mSliderHandleGraphic.SetPosition(mSliderPercentage * GetPixelWidth(), 0.0f);
}

//--------------------------------------------------------------------------------------------------------------------//

tbMath::Vector2 LudumDare56::GameClient::UserInterface::SliderBarGraphic::PositionFromTopLeft(const tbMath::Vector2& screenPoint) const
{
	return screenPoint - GetPosition() + tbMath::Vector2(GetOrigin().x * GetScale().x, GetOrigin().y * GetScale().y);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::SliderBarGraphic::OnUpdate(const float deltaTime)
{
	mSliderTrackGraphic.Update(deltaTime);
	mSliderHandleGraphic.Update(deltaTime);

	if (true == IsEnabled() && true == IsVisible() && true == IsMouseContained())
	{
		if (true == tbGame::Input::IsKeyDown(tbApplication::tbMouseLeft))
		{
			mSliderHandleGraphic.SetColor(ui::Color::ControlActive);

			const tbMath::Vector2 fromTopLeft = PositionFromTopLeft(PointInParentSpace(tbGame::Input::GetMousePosition()));
			SetSliderPercentage(fromTopLeft.x / GetScaledWidth());

			if (mCallback)
			{
				mCallback();
			}
		}
		else
		{
			mSliderHandleGraphic.SetColor(ui::Color::ControlHovered);
		}
	}
	else if (true == IsEnabled())
	{
		mSliderHandleGraphic.SetColor(ui::Color::TyreBytesBlue);
	}
	else //false == mIsEnabled
	{
		mSliderHandleGraphic.SetColor(ui::Color::ControlDisabled);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::SliderBarGraphic::OnRender(void) const
{
	mSliderTrackGraphic.Render();
	mSliderHandleGraphic.Render();
}

//--------------------------------------------------------------------------------------------------------------------//
