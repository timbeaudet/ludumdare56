///
/// @file
/// @details A simple Graphic icon / button that performs a callback when it is clicked on with the mouse. This is NOT
///   part of the TurtleBrains::ExpressKit interface in anyway, it is just a sprite object.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "sprite_button_graphic.hpp"

#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"

#include <turtle_brains/game/tb_input_action.hpp>

namespace
{
	static const tbGraphics::Color ControlActive(0xFFFF7675); //This added delayed flash state on some buttons, so kept same as Hovered.
	static const tbGraphics::Color ControlHovered(0xFFFF7675);
	static const tbGraphics::Color ControlEnabled(0xFFD63031);

	std::array<tbGraphics::Color, 3> kTitlePrimaryColors{
		tbGraphics::Color(0xFFFF2E9F), tbGraphics::Color(0xFFff008a), tbGraphics::Color(0xFFff008a)
	};

	std::array<tbGraphics::Color, 3> kTitleSecondaryColors{
		tbGraphics::Color(0xFF2E9FFF), tbGraphics::Color(0xFF0076ff), tbGraphics::Color(0xFF0076ff)
	};

	std::array<tbGraphics::Color, 3> kTitleExitColors{
		tbGraphics::Color(0xFF7528d5), tbGraphics::Color(0xFF854bcd), tbGraphics::Color(0xFF7528d5)
	};


	//index = 0 for enabled, 1 for hovered, 2 for actice
	tbGraphics::Color GetButtonColor(int index, LudumDare56::GameClient::UserInterface::ButtonType buttonType)
	{
		using namespace LudumDare56::GameClient;

		switch (buttonType)
		{
		case ui::ButtonType::kTitlePrimary: return kTitlePrimaryColors[index];
		case ui::ButtonType::kTitleSecondary: return kTitleSecondaryColors[index];
		case ui::ButtonType::kTitleExit: return kTitleExitColors[index];
		default: {
			switch (index)
			{
			case 0: return ui::Color::ControlEnabled;
			case 1: return ui::Color::ControlHovered;
			case 2: return ui::Color::ControlActive;
			};
			break; }
		};

		return ui::Color::ControlDisabled;
	}

	static const tbGraphics::Color ControlDisabled(0xFF746564); //Was once; 0xFFb4b4b4
};

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::SpriteButtonGraphic::SpriteButtonGraphic(
	const tbCore::tbString& buttonLabel, const ButtonType buttonType) :

	BaseControlGraphic(),
	mBackdropGraphic("hud_sheet", "nine_slice_round", 350, (ButtonType::kTitlePrimary == buttonType) ? 75 : 60),
	mLabelText(buttonLabel, ((ButtonType::kTitlePrimary == buttonType) ? 37.0f : 25.0f)),
	mButtonType(buttonType)
{
	mLabelText.SetOrigin(tbGraphics::kAnchorCenter);
	mLabelText.SetPosition(UserInterface::GetAnchorPositionOf(mBackdropGraphic, tbGraphics::kAnchorCenter));

//	mBackdropGraphic.SetColor(ui::Color::ControlEnabled);
	mBackdropGraphic.SetColor(GetButtonColor(0, mButtonType));
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::SpriteButtonGraphic::~SpriteButtonGraphic(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::SpriteButtonGraphic::SetLabel(const tbCore::tbString& buttonLabel)
{
	mLabelText.SetText(buttonLabel);
	mLabelText.SetOrigin(tbGraphics::kAnchorCenter);
	mLabelText.SetPosition(UserInterface::GetAnchorPositionOf(mBackdropGraphic, tbGraphics::kAnchorCenter));
}

//--------------------------------------------------------------------------------------------------------------------//

tbGraphics::PixelSpace LudumDare56::GameClient::UserInterface::SpriteButtonGraphic::GetPixelWidth(void) const
{
	return mBackdropGraphic.GetPixelWidth();
}

//--------------------------------------------------------------------------------------------------------------------//

tbGraphics::PixelSpace LudumDare56::GameClient::UserInterface::SpriteButtonGraphic::GetPixelHeight(void) const
{
	return mBackdropGraphic.GetPixelHeight();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::SpriteButtonGraphic::OnUpdate(const float deltaTime)
{
	mLabelText.Update(deltaTime);
	mBackdropGraphic.Update(deltaTime);

	if (true == IsEnabled() && true == IsVisible() && true == IsMouseContained())
	{
		if (true == tbGame::Input::IsKeyPressed(tbApplication::tbMouseLeft))
		{
			if (mCallback)
			{
				mCallback();
			}

			//mBackdropGraphic.SetColor(Color::ControlActive);
			mBackdropGraphic.SetColor(GetButtonColor(2, mButtonType));
		}
		else
		{
			//mBackdropGraphic.SetColor(Color::ControlHovered);
			mBackdropGraphic.SetColor(GetButtonColor(1, mButtonType));
		}
	}
	else if (true == IsEnabled())
	{
		//mBackdropGraphic.SetColor(Color::ControlEnabled);
		mBackdropGraphic.SetColor(GetButtonColor(0, mButtonType));
	}
	else //false == mIsEnabled
	{
		mBackdropGraphic.SetColor(Color::ControlDisabled);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::SpriteButtonGraphic::OnRender(void) const
{
	mBackdropGraphic.Render();
	mLabelText.Render();
}

//--------------------------------------------------------------------------------------------------------------------//
