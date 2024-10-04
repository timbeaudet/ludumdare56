///
/// @file
/// @details Provides some utility functions for performing keybinding with TurtleBrains.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#if !defined(ludumdare56_headless_build)

#include "../../core/input/key_binder.hpp"
#include "../../logging.hpp"

//--------------------------------------------------------------------------------------------------------------------//

namespace
{

	struct KeyInformation
	{
		tbApplication::Key mKey;
		tbCore::tbString mDisplayName;
		tbCore::tbString mFullName;
	};

	std::array<KeyInformation, tbApplication::tbKeyMax> theKeyTable{
		KeyInformation{ tbApplication::tbKey0, "0", "0" },
		KeyInformation{ tbApplication::tbKey1, "1", "1" },
		KeyInformation{ tbApplication::tbKey2, "2", "2" },
		KeyInformation{ tbApplication::tbKey3, "3", "3" },
		KeyInformation{ tbApplication::tbKey4, "4", "4" },
		KeyInformation{ tbApplication::tbKey5, "5", "5" },
		KeyInformation{ tbApplication::tbKey6, "6", "6" },
		KeyInformation{ tbApplication::tbKey7, "7", "7" },
		KeyInformation{ tbApplication::tbKey8, "8", "8" },
		KeyInformation{ tbApplication::tbKey9, "9", "9" },

		KeyInformation{ tbApplication::tbKeyNumpad0, "NP0", "Numpad0" },
		KeyInformation{ tbApplication::tbKeyNumpad1, "NP1", "Numpad1" },
		KeyInformation{ tbApplication::tbKeyNumpad2, "NP2", "Numpad2" },
		KeyInformation{ tbApplication::tbKeyNumpad3, "NP3", "Numpad3" },
		KeyInformation{ tbApplication::tbKeyNumpad4, "NP4", "Numpad4" },
		KeyInformation{ tbApplication::tbKeyNumpad5, "NP5", "Numpad5" },
		KeyInformation{ tbApplication::tbKeyNumpad6, "NP6", "Numpad6" },
		KeyInformation{ tbApplication::tbKeyNumpad7, "NP7", "Numpad7" },
		KeyInformation{ tbApplication::tbKeyNumpad8, "NP8", "Numpad8" },
		KeyInformation{ tbApplication::tbKeyNumpad9, "NP9", "Numpad9" },

		KeyInformation{ tbApplication::tbKeyNumpadDivide, "NP/", "NumpadDivide" },
		KeyInformation{ tbApplication::tbKeyNumpadMultiply, "NP*", "NumpadMultiply" },
		KeyInformation{ tbApplication::tbKeyNumpadSubtract, "NP-", "NumpadSubtract" },
		KeyInformation{ tbApplication::tbKeyNumpadAdd, "NP+", "NumpadAdd" },
		KeyInformation{ tbApplication::tbKeyNumpadEnter, "NPEnter", "NumpadEnter" },
		KeyInformation{ tbApplication::tbKeyNumpadPeriod, "NP.", "NumpadPeriod" },

		KeyInformation{ tbApplication::tbKeyA, "A", "A" },
		KeyInformation{ tbApplication::tbKeyB, "B", "B" },
		KeyInformation{ tbApplication::tbKeyC, "C", "C" },
		KeyInformation{ tbApplication::tbKeyD, "D", "D" },
		KeyInformation{ tbApplication::tbKeyE, "E", "E" },
		KeyInformation{ tbApplication::tbKeyF, "F", "F" },
		KeyInformation{ tbApplication::tbKeyG, "G", "G" },
		KeyInformation{ tbApplication::tbKeyH, "H", "H" },
		KeyInformation{ tbApplication::tbKeyI, "I", "I" },
		KeyInformation{ tbApplication::tbKeyJ, "J", "J" },
		KeyInformation{ tbApplication::tbKeyK, "K", "K" },
		KeyInformation{ tbApplication::tbKeyL, "L", "L" },
		KeyInformation{ tbApplication::tbKeyM, "M", "M" },
		KeyInformation{ tbApplication::tbKeyN, "N", "N" },
		KeyInformation{ tbApplication::tbKeyO, "O", "O" },
		KeyInformation{ tbApplication::tbKeyP, "P", "P" },
		KeyInformation{ tbApplication::tbKeyQ, "Q", "Q" },
		KeyInformation{ tbApplication::tbKeyR, "R", "R" },
		KeyInformation{ tbApplication::tbKeyS, "S", "S" },
		KeyInformation{ tbApplication::tbKeyT, "T", "T" },
		KeyInformation{ tbApplication::tbKeyU, "U", "U" },
		KeyInformation{ tbApplication::tbKeyV, "V", "V" },
		KeyInformation{ tbApplication::tbKeyW, "W", "W" },
		KeyInformation{ tbApplication::tbKeyX, "X", "X" },
		KeyInformation{ tbApplication::tbKeyY, "Y", "Y" },
		KeyInformation{ tbApplication::tbKeyZ, "Z", "Z" },

		KeyInformation{ tbApplication::tbKeyF1, "F1", "F1" },
		KeyInformation{ tbApplication::tbKeyF2, "F2", "F2" },
		KeyInformation{ tbApplication::tbKeyF3, "F3", "F3" },
		KeyInformation{ tbApplication::tbKeyF4, "F4", "F4" },
		KeyInformation{ tbApplication::tbKeyF5, "F5", "F5" },
		KeyInformation{ tbApplication::tbKeyF6, "F6", "F6" },
		KeyInformation{ tbApplication::tbKeyF7, "F7", "F7" },
		KeyInformation{ tbApplication::tbKeyF8, "F8", "F8" },
		KeyInformation{ tbApplication::tbKeyF9, "F9", "F9" },
		KeyInformation{ tbApplication::tbKeyF10, "F10", "F10" },
		KeyInformation{ tbApplication::tbKeyF11, "F11", "F11" },
		KeyInformation{ tbApplication::tbKeyF12, "F12", "F12" },

		KeyInformation{ tbApplication::tbKeyUp, "Up", "UpArrow" },
		KeyInformation{ tbApplication::tbKeyDown, "Down", "DownArrow" },
		KeyInformation{ tbApplication::tbKeyLeft, "Left", "LeftArrow" },
		KeyInformation{ tbApplication::tbKeyRight, "Right", "RightArrow" },

		KeyInformation{ tbApplication::tbKeySpace, "Space", "Space" },
		KeyInformation{ tbApplication::tbKeyBackspace, "Backspace", "Backspace" },
		KeyInformation{ tbApplication::tbKeyEscape, "Esc", "Escape" },
		KeyInformation{ tbApplication::tbKeyEnter, "Enter", "Enter" },
		KeyInformation{ tbApplication::tbKeyTab, "Tab", "Tab" },

		KeyInformation{ tbApplication::tbKeyInsert, "Insert", "Insert" },
		KeyInformation{ tbApplication::tbKeyDelete, "Delete", "Delete" },
		KeyInformation{ tbApplication::tbKeyPageUp, "PageUp", "PageUp" },
		KeyInformation{ tbApplication::tbKeyPageDown, "PageDown", "PageDown" },
		KeyInformation{ tbApplication::tbKeyHome, "Home", "Home" },
		KeyInformation{ tbApplication::tbKeyEnd, "End", "End" },

		KeyInformation{ tbApplication::tbKeyLeftShift, "LeftShift", "LeftShift" },
		KeyInformation{ tbApplication::tbKeyRightShift, "RightShift", "RightShift" },
		KeyInformation{ tbApplication::tbKeyShiftModifier, "Shift", "Shift" },
		KeyInformation{ tbApplication::tbKeyLeftControl, "LeftControl", "LeftControl" },
		KeyInformation{ tbApplication::tbKeyRightControl, "RightControl", "RightControl" },
		KeyInformation{ tbApplication::tbKeyControlModifier, "Control", "Control" },
		KeyInformation{ tbApplication::tbKeyLeftAlt, "LeftAlt", "LeftAlt" },
		KeyInformation{ tbApplication::tbKeyRightAlt, "RightAlt", "RightAlt" },
		KeyInformation{ tbApplication::tbKeyAltModifier, "Alt", "Alt" },

		KeyInformation{ tbApplication::tbKeyCapsLock, "CapsLock", "CapsLock" },
		KeyInformation{ tbApplication::tbKeyCapsLockModifier, "CapsLockModifier", "CapsLockModifier" },
		KeyInformation{ tbApplication::tbKeyNumLock, "NumLock", "NumberLock" },
		KeyInformation{ tbApplication::tbKeyNumLockModifier, "NumLockModifier", "NumberLockModifier" },
		KeyInformation{ tbApplication::tbKeyScrollLock, "ScrollLock", "ScrollLock" },
		KeyInformation{ tbApplication::tbKeyScrollLockModifier, "ScrollLockModifier", "ScrollLockModifier" },

		KeyInformation{ tbApplication::tbKeyPrintScreen, "PrintScreen", "PrintScreen" },
		KeyInformation{ tbApplication::tbKeyPause, "Pause", "Pause" },

		KeyInformation{ tbApplication::tbKeyTilde, "~", "Tilde" },
		KeyInformation{ tbApplication::tbKeyComma, ",", "Comma" },
		KeyInformation{ tbApplication::tbKeyPeriod, ".", "Period" },
		KeyInformation{ tbApplication::tbKeyPeriod, "/", "Slash" },
		KeyInformation{ tbApplication::tbKeyPeriod, "\\", "Backslash" },
		KeyInformation{ tbApplication::tbKeyMinus, "-", "Minus" },
		KeyInformation{ tbApplication::tbKeyPlus, "+", "Plus" },
		KeyInformation{ tbApplication::tbKeyColon, ":", "Colon" },
		KeyInformation{ tbApplication::tbKeyLeftBracket, "[", "LeftBracket" },
		KeyInformation{ tbApplication::tbKeyRightBracket, "]", "RightBracket" },
		KeyInformation{ tbApplication::tbKeyQuote, "'", "Quote" },

		KeyInformation{ tbApplication::tbMouseLeft, "LMB", "LeftMouse" },
		KeyInformation{ tbApplication::tbMouseRight, "RMB", "RightMouse" },
		KeyInformation{ tbApplication::tbMouseMiddle, "Mouse3", "Mouse3" },
	};
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString TyreBytes::Core::Input::KeyBinder::KeyDisplayName(const tbApplication::Key& key)
{
	for (const KeyInformation& keyInfo : theKeyTable)
	{
		if (key == keyInfo.mKey)
		{
			return keyInfo.mDisplayName;
		}
	}

	return "---";
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString TyreBytes::Core::Input::KeyBinder::KeyFullName(const tbApplication::Key& key)
{
	for (const KeyInformation& keyInfo : theKeyTable)
	{
		if (key == keyInfo.mKey)
		{
			return keyInfo.mFullName;
		}
	}

	return "InvalidKey";
}

//--------------------------------------------------------------------------------------------------------------------//

TurtleBrains::Application::Key TyreBytes::Core::Input::KeyBinder::KeyFromName(const tbCore::tbString& keyName)
{
	for (const KeyInformation& keyInfo : theKeyTable)
	{
		if (keyName == keyInfo.mFullName)
		{
			return keyInfo.mKey;
		}
	}

	return tbApplication::tbKeyInvalid;
}

//--------------------------------------------------------------------------------------------------------------------//

tbGame::InputAction TyreBytes::Core::Input::KeyBinder::ActionFromName(const tbCore::tbString& fullName, bool invertAxis)
{
	for (const KeyInformation& keyInfo : theKeyTable)
	{
		if (fullName == keyInfo.mFullName)
		{
			return tbGame::InputAction(keyInfo.mKey);
		}
	}

	if (tbCore::StringStartsWith(fullName, "axis") && fullName.size() > 6)
	{	//axisX_YY
		tbSystem::Unstable::DeviceHandle device = tbCore::RangedCast<tbSystem::Unstable::DeviceHandle>(tbCore::FromString<int>(&fullName[4]));
		tbSystem::Unstable::AxisIdentifier axis = tbCore::RangedCast<tbSystem::Unstable::DeviceHandle>(tbCore::FromString<int>(&fullName[6]));

		tb_always_log(LudumDare56::LogGame::Debug() << "Action from name: " << fullName << " is dev " << +device << " with axis " << +axis);

		tbGame::InputAction action;
		action.AddAnalogBinding(device, axis, invertAxis);
		return action;
	}

	if (tbCore::StringStartsWith(fullName, "j") && fullName.size() > 3)
	{	//jX_YY
		tbSystem::Unstable::DeviceHandle device = tbCore::RangedCast<tbSystem::Unstable::DeviceHandle>(tbCore::FromString<int>(&fullName[1]));
		tbSystem::Unstable::ButtonIdentifier button = tbCore::RangedCast<tbSystem::Unstable::DeviceHandle>(tbCore::FromString<int>(&fullName[3]));

		tb_always_log("Action from name: " << fullName << " is dev " << +device << " with button " << +button);

		tbGame::InputAction action;
		action.AddBinding(device, button);
		return action;
	}

	return tbGame::InputAction();
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString TyreBytes::Core::Input::KeyBinder::FullNameToDisplayName(const tbCore::tbString& fullName)
{
	for (const KeyInformation& keyInfo : theKeyTable)
	{
		if (fullName == keyInfo.mFullName)
		{
			return keyInfo.mDisplayName;
		}
	}

	if (tbCore::StringStartsWith(fullName, "j") || tbCore::StringStartsWith(fullName, "axis"))
	{
		return fullName;
	}

	return "---";
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Input::KeyBinder::IsAnalogControl(const tbCore::tbString& fullName)
{
	return tbCore::StringStartsWith(fullName, "axis");
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString TyreBytes::Core::Input::KeyBinder::ButtonDisplayName(tbSystem::Unstable::DeviceHandle device, tbSystem::Unstable::ButtonIdentifier button)
{
	return tb_string("j") + tbCore::ToString<size_t>(device) + tb_string("_") + tbCore::ToString<size_t>(button);
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString TyreBytes::Core::Input::KeyBinder::AxisDisplayName(tbSystem::Unstable::DeviceHandle device, tbSystem::Unstable::AxisIdentifier axis)
{
	return tb_string("axis") + tbCore::ToString<size_t>(device) + tb_string("_") + tbCore::ToString<size_t>(axis);
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Input::KeyBinder::ActionBinder::ResetAxisValues(void)
{
	for (float& value : mCurrentAxisValues)
	{
		value = -10000.0f;
	}

	tbSystem::Unstable::InputDeviceManager& inputManager = tbSystem::Unstable::theInputDeviceManager;
	for (tbSystem::Unstable::DeviceHandle deviceIndex = 0; deviceIndex < tbSystem::Unstable::kMaximumDevices; ++deviceIndex)
	{
		const size_t axisCount = inputManager.GetAxisCount(deviceIndex);
		tb_error_if(axisCount > 8, "KeyBinder is hardcoded to a maximum of 8 axes at present time.");
		for (tbSystem::Unstable::AxisIdentifier axisIndex = 0; axisIndex < axisCount; ++axisIndex)
		{
			mCurrentAxisValues[deviceIndex * 8 + axisIndex] = inputManager.GetAxisPercentage(deviceIndex, axisIndex);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Input::KeyBinder::ActionBinder::PollForBinding(void)
{
	mDevice = tbSystem::Unstable::InvalidDeviceHandle();
	mButton = tbSystem::Unstable::InvalidButton();
	mAxis = tbSystem::Unstable::InvalidAxis();
	mKeyboardKey = tbApplication::tbKeyInvalid;

	std::array<tbApplication::Key, 64> bindableKeys{
		tbApplication::tbKeyA,
		tbApplication::tbKeyB,
		tbApplication::tbKeyC,
		tbApplication::tbKeyD,
		tbApplication::tbKeyE,
		tbApplication::tbKeyF,
		tbApplication::tbKeyG,
		tbApplication::tbKeyH,
		tbApplication::tbKeyI,
		tbApplication::tbKeyJ,
		tbApplication::tbKeyK,
		tbApplication::tbKeyL,
		tbApplication::tbKeyM,
		tbApplication::tbKeyN,
		tbApplication::tbKeyO,
		tbApplication::tbKeyP,
		tbApplication::tbKeyQ,
		tbApplication::tbKeyR,
		tbApplication::tbKeyS,
		tbApplication::tbKeyT,
		tbApplication::tbKeyU,
		tbApplication::tbKeyV,
		tbApplication::tbKeyW,
		tbApplication::tbKeyX,
		tbApplication::tbKeyY,
		tbApplication::tbKeyZ,

		tbApplication::tbKey1,
		tbApplication::tbKey2,
		tbApplication::tbKey3,
		tbApplication::tbKey4,
		tbApplication::tbKey5,
		tbApplication::tbKey6,
		tbApplication::tbKey7,
		tbApplication::tbKey8,
		tbApplication::tbKey9,
		tbApplication::tbKey0,

		tbApplication::tbKeyNumpad1,
		tbApplication::tbKeyNumpad2,
		tbApplication::tbKeyNumpad3,
		tbApplication::tbKeyNumpad4,
		tbApplication::tbKeyNumpad5,
		tbApplication::tbKeyNumpad6,
		tbApplication::tbKeyNumpad7,
		tbApplication::tbKeyNumpad8,
		tbApplication::tbKeyNumpad9,
		tbApplication::tbKeyNumpad0,

		tbApplication::tbKeyLeft,
		tbApplication::tbKeyRight,
		tbApplication::tbKeyUp,
		tbApplication::tbKeyDown,
		tbApplication::tbKeySpace,

		tbApplication::tbKeyComma,
		tbApplication::tbKeyPeriod,
		tbApplication::tbKeySlash,
		tbApplication::tbKeyColon,
		tbApplication::tbKeyQuote,
		tbApplication::tbKeyLeftBracket,
		tbApplication::tbKeyRightBracket,
	};

	//Check for keyboard and mouse buttons.
	for (const tbApplication::Key& key : bindableKeys)
	{
		if (tbApplication::Input::IsKeyPressed(key))
		{
			mKeyboardKey = key;
			return true;
		}
	}

	//
	// Joystick Buttons and Axes
	tbSystem::Unstable::InputDeviceManager& inputManager = tbSystem::Unstable::theInputDeviceManager;
	for (tbSystem::Unstable::DeviceHandle deviceIndex = 0; deviceIndex < tbSystem::Unstable::kMaximumDevices; ++deviceIndex)
	{
		const size_t buttonCount = inputManager.GetButtonCount(deviceIndex);
		const size_t axisCount = inputManager.GetAxisCount(deviceIndex);

		for (tbSystem::Unstable::ButtonIdentifier buttonIndex = 0; buttonIndex < buttonCount; ++buttonIndex)
		{
			if (true == inputManager.IsButtonPressed(deviceIndex, buttonIndex))
			{
				tb_always_log("You pressed a controller button: Device " << +deviceIndex << ", Button " << +buttonIndex);
				mDevice = deviceIndex;
				mButton = buttonIndex;
				return true;
			}
		}

		for (tbSystem::Unstable::AxisIdentifier axisIndex = 0; axisIndex < axisCount; ++axisIndex)
		{
			float& previous = mCurrentAxisValues[deviceIndex * 8 + axisIndex];
			const float current = inputManager.GetAxisPercentage(deviceIndex, axisIndex);
			if (fabsf(previous - current) > 0.1f) //BAD IMPLEMENTATION
			{
				ResetAxisValues();

				mDevice = deviceIndex;
				mAxis = axisIndex;
				return true;
			}
		}
	}

	//.
	mDevice = tbSystem::Unstable::InvalidDeviceHandle();
	mButton = tbSystem::Unstable::InvalidButton();
	mAxis = tbSystem::Unstable::InvalidAxis();
	mKeyboardKey = tbApplication::tbKeyInvalid;
	return false;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString TyreBytes::Core::Input::KeyBinder::ActionBinder::GetBindingName(void) const
{
	if (IsKeyboardBinding())
	{
		return KeyBinder::KeyFullName(mKeyboardKey);
	}

	if (tbSystem::Unstable::InvalidDeviceHandle() != mDevice)
	{
		if (tbSystem::Unstable::InvalidButton() != mButton)
		{
			return KeyBinder::ButtonDisplayName(mDevice, mButton);
		}

		if (tbSystem::Unstable::InvalidAxis() != mAxis)
		{
			return KeyBinder::AxisDisplayName(mDevice, mAxis);
		}
	}

	return "InvalidKey";
}

//--------------------------------------------------------------------------------------------------------------------//

#endif /* not ludumdare56_headless_build */
