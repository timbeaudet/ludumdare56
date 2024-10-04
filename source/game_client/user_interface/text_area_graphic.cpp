///
/// @file
/// @details A multilined text area that is really not so smart, but should get a job done.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/user_interface/text_area_graphic.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::TextAreaGraphic::TextAreaGraphic(void) :
	TurtleBrains::Graphics::Graphic(),
	mLinesOfText(),
	mTextAreaWidth(0),
	mTextAreaHeight(0),
	mHideTextTimer(0)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::TextAreaGraphic::~TextAreaGraphic(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::TextAreaGraphic::ClearText(void)
{
	mLinesOfText.clear();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::TextAreaGraphic::AddLineOfText(const tbGraphics::Text& text)
{
	mLinesOfText.emplace_back(new tbGraphics::Text(text.GetText(), text.GetPointSize(), text.GetFont()));

	mTextAreaWidth = 0;
	mTextAreaHeight = 0;

	const float kLineSpacing = 10.0f;

	float y = 0.0f;
	for (std::unique_ptr<tbGraphics::Text>& line : mLinesOfText)
	{
		line->SetOrigin(tbGraphics::kAnchorTopLeft); //what about center or right align? Not needed today 2022-07-10
		line->SetPosition(0.0f, y);
		y += line->GetHeight() + kLineSpacing;

		if (line->GetPixelWidth() > mTextAreaWidth)
		{
			mTextAreaWidth = line->GetPixelWidth();
		}
	}

	mTextAreaHeight = static_cast<tbGraphics::PixelSpace>(y);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::TextAreaGraphic::Simulate(void)
{
	mHideTextTimer.DecrementStep();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::TextAreaGraphic::OnRender(void) const
{
	if (true == mHideTextTimer.IsZero())
	{
		for (const std::unique_ptr<tbGraphics::Text>& line : mLinesOfText)
		{
			line->Render();
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//
