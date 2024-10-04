///
/// @file
/// @details Creates a small box/display area for user interface when the user is binding an analog control. Will show
///   the current values and automatically invert as necessary.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/user_interface/analog_binding_graphic.hpp"
#include "../../game_client/user_interface/user_interface_helpers.hpp"
#include "../../game_client/user_interface/user_interface_constants.hpp"
#include "../../core/input/key_binder.hpp"

#include "../../logging.hpp"

using namespace TyreBytes::Core::Input;

namespace
{
	const tbGraphics::PixelSpace kDisplayWidth = 880;
	const tbGraphics::PixelSpace kDisplayHeight = 400;

	KeyBinder::ActionBinder theActionBinder;
};

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::AnalogBindingGraphic::AnalogBindingGraphic(void) :
	BaseControlGraphic(),
	mBackdropGraphic("hud_sheet", "nine_slice_round", kDisplayWidth, kDisplayHeight),
	mOutlineGraphic("hud_sheet", "nine_slice_round_outline", kDisplayWidth, kDisplayHeight),
	mAxisSlider(),
	mInvertCheckbox(),
	mAxisLabel("---"),
	mControlBindingText("Press the button to bind to your control, escape to cancel."),
	mConfirmButton("Confirm Binding"),
	mCancelBindingButton("Cancel Binding"),
	mPossibleControlName(""),
	mPossibleControlInvert(false),
	mIsConfirmedBinding(false),
	mFirstFrame(false)
{
	mBackdropGraphic.SetColor(ui::Color::DarkBackdrop);
	mOutlineGraphic.SetColor(ui::Color::TyreBytesBlue);

	mConfirmButton.SetCallback([this]() {
		FinishBinding(true);
	});

	mCancelBindingButton.SetCallback([this]() {
		FinishBinding(false);
	});

	mInvertCheckbox.SetCallback([this]() {
		mPossibleControlInvert = mInvertCheckbox.IsChecked();
	});
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::UserInterface::AnalogBindingGraphic::~AnalogBindingGraphic(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

tbGraphics::PixelSpace LudumDare56::GameClient::UserInterface::AnalogBindingGraphic::GetPixelWidth(void) const
{
	return mBackdropGraphic.GetPixelWidth();
}

//--------------------------------------------------------------------------------------------------------------------//

tbGraphics::PixelSpace LudumDare56::GameClient::UserInterface::AnalogBindingGraphic::GetPixelHeight(void) const
{
	return mBackdropGraphic.GetPixelHeight();
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString LudumDare56::GameClient::UserInterface::AnalogBindingGraphic::GetControlDisplayName(void) const
{
	return (false == mIsConfirmedBinding) ? "" : KeyBinder::FullNameToDisplayName(mPossibleControlName);
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString LudumDare56::GameClient::UserInterface::AnalogBindingGraphic::GetControlFullName(void) const
{
	return (false == mIsConfirmedBinding) ? "" : mPossibleControlName;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameClient::UserInterface::AnalogBindingGraphic::IsControlInverted(void) const
{
	return mPossibleControlInvert;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::AnalogBindingGraphic::OnUpdate(const float deltaTime)
{
	BaseControlGraphic::OnUpdate(deltaTime);
	mBackdropGraphic.Update(deltaTime);
	mOutlineGraphic.Update(deltaTime);

	if (false == IsEnabled() || false == IsVisible())
	{
		return;
	}

	if (true == mFirstFrame)
	{
		mFirstFrame = false;
		return;
	}

	PollKeyBinder();

	if (false == mPossibleControlName.empty())
	{
		static tbGame::InputAction action(KeyBinder::ActionFromName(mPossibleControlName, mPossibleControlInvert));
		static tbCore::tbString currentActionName = mPossibleControlName;
		static bool currentInvertState = mPossibleControlInvert;
		if (mPossibleControlName != currentActionName || mPossibleControlInvert != currentInvertState)
		{
			action = KeyBinder::ActionFromName(mPossibleControlName, mPossibleControlInvert);
			currentActionName = mPossibleControlName;
			currentInvertState = mPossibleControlInvert;
		}

		mAxisSlider.SetSliderPercentage(action.GetAnalogValue());
	}

	const tbGraphics::PixelSpace kPadding(20);
	const tbGraphics::PixelSpace kHalfPadding(kPadding / 2);

	mControlBindingText.SetOrigin(tbGraphics::kAnchorTopCenter);
	mControlBindingText.SetPosition(GetWidth() / 2.0f, kPadding);

	mAxisSlider.SetOrigin(tbGraphics::kAnchorBottomCenter);
	mAxisSlider.SetPosition(GetWidth() / 2.0f, GetHeight() / 2.0f);
	mAxisLabel.SetOrigin(tbGraphics::kAnchorTopCenter);
	mAxisLabel.SetPosition(mAxisSlider.GetPosition() + tbMath::Vector2(0.0f, kPadding));

	mInvertCheckbox.SetOrigin(tbGraphics::kAnchorCenterLeft);
	mInvertCheckbox.SetPosition(mAxisSlider.GetPosition() + tbMath::Vector2(mAxisSlider.GetWidth() / 2.0f + kPadding * 4, 0.0f));

	mConfirmButton.SetEnabled(false == mPossibleControlName.empty());
	mConfirmButton.SetOrigin(tbGraphics::kAnchorBottomLeft);
	mConfirmButton.SetPosition(GetWidth() / 2.0f + kHalfPadding, GetHeight() - kPadding);

	mCancelBindingButton.SetOrigin(tbGraphics::kAnchorBottomRight);
	mCancelBindingButton.SetPosition(GetWidth() / 2.0f - kHalfPadding, GetHeight() - kPadding);

	const tbMath::Vector2 parentOffset = GetAnchorPositionOf(*this, tbGraphics::kAnchorTopLeft);
	const tbMath::Vector2 parentScale = GetScale();

	mAxisSlider.SetParentOffset(parentOffset, parentScale);
	mAxisSlider.Update(deltaTime);
	mAxisLabel.SetParentOffset(parentOffset, parentScale);
	mAxisLabel.Update(deltaTime);
	mInvertCheckbox.SetParentOffset(parentOffset, parentScale);
	mInvertCheckbox.Update(deltaTime);
	mConfirmButton.SetParentOffset(parentOffset, parentScale);
	mConfirmButton.Update(deltaTime);
	mCancelBindingButton.SetParentOffset(parentOffset, parentScale);
	mCancelBindingButton.Update(deltaTime);
	mControlBindingText.Update(deltaTime);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::AnalogBindingGraphic::PollKeyBinder(void)
{
	if (true == theActionBinder.PollForBinding())
	{
		const tbCore::tbString keyName = theActionBinder.GetBindingName();
		tb_debug_log(LogGame::Info() << "The key you pressed was: \"" << keyName << "\"");

		mPossibleControlName = keyName;

		mAxisLabel.SetText(KeyBinder::FullNameToDisplayName(mPossibleControlName));

		//if (true == KeyBinder::IsAnalogControl(keyName))
		//{	//If index is SteerLeft or SteerRight; in hard-coded poor manner.
		//	if (mSteerLeftControlIndex == mBindingControlIndex || mSteerRightControlIndex == mBindingControlIndex)
		//	{
		//		*mBindableControlNames[mSteerLeftControlIndex] = keyName;
		//		mBindableButtons[mSteerLeftControlIndex]->SetLabel(KeyBinder::FullNameToDisplayName(keyName));

		//		*mBindableControlNames[mSteerRightControlIndex] = keyName;
		//		mBindableButtons[mSteerRightControlIndex]->SetLabel(KeyBinder::FullNameToDisplayName(keyName));
		//	}
		//}

		//for (size_t index = 0; index < mInvertCheckboxes.size(); ++index)
		//{
		//	mInvertCheckboxes[index]->SetState(KeyBinder::ActionFromName(*mBindableControlNames[index]).IsAnalog() ?
		//		tbxInterface::Unstable::Element::State::Enabled : tbxInterface::Unstable::Element::State::Hidden);
		//	mInvertCheckboxes[index]->SetChecked(*mBindableControlInvertAxis[index]);
		//}
	}


	if (false == mPossibleControlName.empty() && (
		true == tbApplication::Input::IsKeyReleased(tbApplication::tbKeyEnter) ||
		true == tbApplication::Input::IsKeyReleased(tbApplication::tbKeyNumpadEnter)))
	{
		FinishBinding(true);
	}

	if (true == tbApplication::Input::IsKeyReleased(tbApplication::tbKeyEscape))
	{
		FinishBinding(false);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::AnalogBindingGraphic::OnRender(void) const
{
	BaseControlGraphic::OnRender();

	mBackdropGraphic.Render();
	mOutlineGraphic.Render();
	mAxisSlider.Render();
	mAxisLabel.Render();
	mInvertCheckbox.Render();
	mConfirmButton.Render();
	mCancelBindingButton.Render();
	mControlBindingText.Render();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::AnalogBindingGraphic::StartBinding(void)
{
	mPossibleControlName = "";
	mPossibleControlInvert = false;
	mIsConfirmedBinding = false;
	mFirstFrame = true;

	mInvertCheckbox.SetChecked(mPossibleControlInvert);

	theActionBinder.ResetAxisValues();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::UserInterface::AnalogBindingGraphic::FinishBinding(const bool isConfirmedBinding)
{
	mIsConfirmedBinding = isConfirmedBinding;

	if (nullptr != mCallback)
	{
		mCallback();
	}
}

//--------------------------------------------------------------------------------------------------------------------//
