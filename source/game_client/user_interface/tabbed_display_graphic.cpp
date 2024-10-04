///
/// @file
/// @details Creates a tabbed display area for user interface controls to exist and swap between tabs.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/user_interface/tabbed_display_graphic.hpp"
#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"

namespace
{
	const tbGraphics::PixelSpace kTabbedDisplayWidth = 1240;
	const tbGraphics::PixelSpace kTabbedDisplayHeight = 760;

	const tbGraphics::PixelSpace kControlLineHeight = 80;
	const tbGraphics::PixelSpace kEdgePadding = 20;

	const tbGraphics::PixelSpace kTabbedButtonWidth = 200;
	const tbGraphics::PixelSpace kTabbedButtonHeight = 60;
};

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::TabbedButtonGraphic::TabbedButtonGraphic(const tbCore::tbString& tabName) :
	SpriteButtonGraphic(tabName),
	mOutlineGraphic(tbxGraphics::NineSlice(std::array<tbGraphics::Sprite, 9>{ {
		{ "hud_sheet", "nine_slice_round_outline0" }, { "hud_sheet", "nine_slice_round_outline1" }, { "hud_sheet", "nine_slice_round_outline2" },
		{ "hud_sheet", "nine_slice_square_outline3" }, { "hud_sheet", "nine_slice_square_outline4" }, { "hud_sheet", "nine_slice_square_outline5" },
		{ "hud_sheet", "nine_slice_square_outline6" }, { "hud_sheet", "nine_slice_square_outline7" }, { "hud_sheet", "nine_slice_square_outline8" },
		}}, kTabbedButtonWidth, kTabbedButtonHeight)),
	mIsOpened(false)
{
	mBackdropGraphic = tbxGraphics::NineSlice(std::array<tbGraphics::Sprite, 9>{ {
			{ "hud_sheet", "nine_slice_round0" }, { "hud_sheet", "nine_slice_round1" }, { "hud_sheet", "nine_slice_round2" },
			{ "hud_sheet", "nine_slice_square3" }, { "hud_sheet", "nine_slice_square4" }, { "hud_sheet", "nine_slice_square5" },
			{ "hud_sheet", "nine_slice_square6" }, { "hud_sheet", "nine_slice_square7" }, { "hud_sheet", "nine_slice_square8" },
		}}, kTabbedButtonWidth, kTabbedButtonHeight);

	//We just resized the button, and calling SetLabel() is the easiest/fastest way to reposition the text to center.
	SetLabel(tabName);

	mBackdropGraphic.SetColor(ui::Color::DarkBackdrop);
	mOutlineGraphic.SetColor(ui::Color::TyreBytesBlue);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::TabbedButtonGraphic::~TabbedButtonGraphic(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::TabbedButtonGraphic::SetOpened(bool opened)
{
	mIsOpened = opened;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::TabbedButtonGraphic::OnUpdate(const float deltaTime)
{
	SpriteButtonGraphic::OnUpdate(deltaTime);

	if (true == mIsOpened)
	{
		mBackdropGraphic.SetColor(ui::Color::TyreBytesBlue);
		mOutlineGraphic.SetColor(ui::Color::TyreBytesBlue);
	}
	else
	{
		mBackdropGraphic.SetColor(ui::Color::DarkBackdrop);
		mOutlineGraphic.SetColor(ui::Color::TyreBytesBlue);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::TabbedButtonGraphic::OnRender(void) const
{
	mBackdropGraphic.Render();
	mOutlineGraphic.Render();
	mLabelText.Render();
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::TabbedDisplayGraphic::TabbedDisplayGraphic(void) :
	BaseControlGraphic(),
	mBackdropGraphic({{
		{ "hud_sheet", "nine_slice_square0" }, { "hud_sheet", "nine_slice_square1" }, { "hud_sheet", "nine_slice_square2" },
		{ "hud_sheet", "nine_slice_square3" }, { "hud_sheet", "nine_slice_square4" }, { "hud_sheet", "nine_slice_square5" },
		{ "hud_sheet", "nine_slice_round6" }, { "hud_sheet", "nine_slice_round7" }, { "hud_sheet", "nine_slice_round8" } }},
		kTabbedDisplayWidth, kTabbedDisplayHeight),
	mOutlineGraphic({{
		{ "hud_sheet", "nine_slice_square_outline0" }, { "hud_sheet", "nine_slice_square_outline1" }, { "hud_sheet", "nine_slice_square_outline2" },
		{ "hud_sheet", "nine_slice_square_outline3" }, { "hud_sheet", "nine_slice_square_outline4" }, { "hud_sheet", "nine_slice_square_outline5" },
		{ "hud_sheet", "nine_slice_round_outline6" }, { "hud_sheet", "nine_slice_round_outline7" }, { "hud_sheet", "nine_slice_round_outline8" } }},
		kTabbedDisplayWidth, kTabbedDisplayHeight),
	mTabs(),
	mSelectedTabIndex(0),
	mLineCount(0)
{
	mBackdropGraphic.SetColor(ui::Color::DarkBackdrop);
	mOutlineGraphic.SetColor(ui::Color::TyreBytesBlue);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::TabbedDisplayGraphic::~TabbedDisplayGraphic(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::TabbedDisplayGraphic::AddTab(const tbCore::tbString& tabName)
{
	const size_t tabIndex = mTabs.size();

	mTabs.emplace_back(new Tab(tabName));
	mTabs.back()->mTabButton.SetCallback([tabIndex, this]() {
		mSelectedTabIndex = tabIndex;

		for (TabPtr& tab : mTabs)
		{
			tab->mTabButton.SetOpened(false);
		}

		mTabs[mSelectedTabIndex]->mTabButton.SetOpened(true);
	});

	if (0 == tabIndex)
	{
		mTabs.back()->mTabButton.SetOpened(true);
	}

	mLineCount = 0;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::TabbedDisplayGraphic::AddControlLine(BaseControlGraphic* graphicLeft)
{
	tb_error_if(true == mTabs.empty(), "Error: Can't add a control before adding at least a single tab...");

	const float lineOffsetY = static_cast<float>(kControlLineHeight * mLineCount) + (kControlLineHeight * 0.5f) + 50.0f;

	graphicLeft->SetOrigin(tbGraphics::kAnchorCenterRight);
	graphicLeft->SetPosition(GetAnchorPositionOf(*this, tbGraphics::kAnchorTopCenter, tbMath::Vector2(-kEdgePadding, lineOffsetY)));
	mTabs.back()->mControls.push_back(graphicLeft);
	mTabs.back()->mGraphics.AddGraphic(graphicLeft);

	++mLineCount;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::TabbedDisplayGraphic::AddControlLine(BaseControlGraphic* graphicLeft, BaseControlGraphic* graphicRight)
{
	tb_error_if(true == mTabs.empty(), "Error: Can't add a control before adding at least a single tab...");

	const float lineOffsetY = static_cast<float>(kControlLineHeight * mLineCount) + (kControlLineHeight * 0.5f) + 50.0f;

	graphicLeft->SetOrigin(tbGraphics::kAnchorCenterRight);
	graphicLeft->SetPosition(GetAnchorPositionOf(*this, tbGraphics::kAnchorTopCenter, tbMath::Vector2(-kEdgePadding, lineOffsetY)));
	mTabs.back()->mControls.push_back(graphicLeft);
	mTabs.back()->mGraphics.AddGraphic(graphicLeft);

	graphicRight->SetOrigin(tbGraphics::kAnchorCenterLeft);
	graphicRight->SetPosition(GetAnchorPositionOf(*this, tbGraphics::kAnchorTopCenter, tbMath::Vector2(kEdgePadding, lineOffsetY)));
	mTabs.back()->mControls.push_back(graphicRight);
	mTabs.back()->mGraphics.AddGraphic(graphicRight);

	++mLineCount;
}

//--------------------------------------------------------------------------------------------------------------------//

tbGraphics::PixelSpace LudumDare56::GameClient::UserInterface::TabbedDisplayGraphic::GetPixelWidth(void) const
{
	return mBackdropGraphic.GetPixelWidth();
}

//--------------------------------------------------------------------------------------------------------------------//

tbGraphics::PixelSpace LudumDare56::GameClient::UserInterface::TabbedDisplayGraphic::GetPixelHeight(void) const
{
	return mBackdropGraphic.GetPixelHeight();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::TabbedDisplayGraphic::OnUpdate(const float deltaTime)
{
	mBackdropGraphic.Update(deltaTime);
	mOutlineGraphic.Update(deltaTime);

	if (true == mTabs.empty() || false == IsEnabled() || false == IsVisible())
	{
		return;
	}

	float leftOfTab = 0.0f;
	for (TabPtr& tab : mTabs)
	{
		tab->mTabButton.SetOrigin(tbGraphics::kAnchorBottomLeft);
		tab->mTabButton.SetPosition(tbMath::Vector2(leftOfTab, 6.0f));
		tab->mTabButton.SetParentOffset(GetAnchorPositionOf(*this, tbGraphics::kAnchorTopLeft), GetScale());
		tab->mTabButton.Update(deltaTime);
		leftOfTab += tab->mTabButton.GetWidth();

		for (BaseControlGraphic* control : tab->mControls)
		{
			control->SetParentOffset(GetAnchorPositionOf(*this, tbGraphics::kAnchorTopLeft), GetScale());
		}
	}

	mTabs[mSelectedTabIndex]->mGraphics.Update(deltaTime);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::TabbedDisplayGraphic::OnRender(void) const
{
	mBackdropGraphic.Render();
	mOutlineGraphic.Render();

	if (true == mTabs.empty())
	{
		return;
	}

	for (const TabPtr& tab : mTabs)
	{
		tab->mTabButton.Render();
	}

	mTabs[mSelectedTabIndex]->mGraphics.Render();
}

//--------------------------------------------------------------------------------------------------------------------//
