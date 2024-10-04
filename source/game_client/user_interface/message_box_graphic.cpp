///
/// @file
/// @details Displays a message to the user with a little box surrounding it.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/user_interface/message_box_graphic.hpp"
#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"
#include "../../core/utilities.hpp"

#include <turtle_brains/game/tb_input_action.hpp>

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::MessageBoxGraphic::MessageBoxGraphic(const tbCore::tbString& message) :
	tbGraphics::Graphic(),
	mBackdrop("hud_sheet", "nine_slice_flat", 88 * 14, 88 * 4),
	mTextArea(),
	mOkayButton("Okay"),
	mOkayCallback()
{
	SetMessage(message);

	mBackdrop.SetColor(ui::Color::DarkBackdrop);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::MessageBoxGraphic::~MessageBoxGraphic(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::MessageBoxGraphic::SetMessage(const tbCore::tbString& message)
{
	std::vector<tbCore::tbString> messageLines = tbCore::String::SeparateString(message, "\n");

	mTextArea.ClearText();
	for (const tbCore::tbString& messageLine : messageLines)
	{
		mTextArea.AddLineOfText(tbGraphics::Text(messageLine, 30.0f));
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::MessageBoxGraphic::SetOkayCallback(std::function<void()> callbackFunction)
{
	mOkayCallback = callbackFunction;
}

//--------------------------------------------------------------------------------------------------------------------//

tbGraphics::PixelSpace LudumDare56::GameClient::UserInterface::MessageBoxGraphic::GetPixelWidth(void) const
{
	return mBackdrop.GetPixelWidth();
}

//--------------------------------------------------------------------------------------------------------------------//

tbGraphics::PixelSpace LudumDare56::GameClient::UserInterface::MessageBoxGraphic::GetPixelHeight(void) const
{
	return mBackdrop.GetPixelHeight();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::MessageBoxGraphic::OnUpdate(const float deltaTime)
{
	const tbMath::Vector2 textAreaOffset(30.0f, 30.0f);
	mTextArea.SetOrigin(tbGraphics::kAnchorTopLeft);
	mTextArea.SetPosition(textAreaOffset);

	const tbMath::Vector2 okayButtonOffset(0.0f, -30.0f);
	mOkayButton.SetOrigin(tbGraphics::kAnchorBottomCenter);
	mOkayButton.SetPosition(tbMath::Vector2(GetWidth() * 0.5f, GetHeight()) + okayButtonOffset);
	mOkayButton.SetParentOffset(GetAnchorPositionOf(*this, tbGraphics::kAnchorTopLeft), GetScale());

	if (true == IsVisible() && true == mOkayButton.IsMouseContained())
	{
		if (true == tbGame::Input::IsKeyPressed(tbApplication::tbMouseLeft))
		{
			if (mOkayCallback)
			{
				mOkayCallback();
			}
		}
	}

	mBackdrop.Update(deltaTime);
	mTextArea.Update(deltaTime);
	mOkayButton.Update(deltaTime);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::MessageBoxGraphic::OnRender(void) const
{
	mBackdrop.Render();
	mTextArea.Render();
	mOkayButton.Render();
}

//--------------------------------------------------------------------------------------------------------------------//
