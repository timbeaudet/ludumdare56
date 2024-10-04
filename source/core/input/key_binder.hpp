///
/// @file
/// @details Provides some utility functions for performing keybinding with TurtleBrains.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef Core_KeyBinder_hpp
#define Core_KeyBinder_hpp
#if !defined(ludumdare56_headless_build)

#include <turtle_brains/core/tb_string.hpp>
#include <turtle_brains/application/tb_application_input.hpp>
#include <turtle_brains/game/tb_input_action.hpp>

#include <array>

namespace TyreBytes
{
	namespace Core
	{
		namespace Input
		{
			namespace KeyBinder
			{

				tbCore::tbString KeyDisplayName(const tbApplication::Key& key);
				tbCore::tbString KeyFullName(const tbApplication::Key& key);
				tbApplication::Key KeyFromName(const tbCore::tbString& keyName);

				tbGame::InputAction ActionFromName(const tbCore::tbString& fullName, bool invertAxis = false);

				tbCore::tbString ButtonDisplayName(tbSystem::Unstable::DeviceHandle device, tbSystem::Unstable::ButtonIdentifier button);
				tbCore::tbString AxisDisplayName(tbSystem::Unstable::DeviceHandle device, tbSystem::Unstable::AxisIdentifier axis);

				tbCore::tbString FullNameToDisplayName(const tbCore::tbString& fullName);
				bool IsAnalogControl(const tbCore::tbString& fullName);

				class ActionBinder
				{
				public:
					void ResetAxisValues(void);

					bool PollForBinding(void);

					/// @note only valid when PollForBinding returned true.
					bool IsKeyboardBinding(void) const { return tbApplication::tbKeyInvalid != mKeyboardKey; }

					/// @note only valid when PollForBinding returned true.
					tbApplication::Key GetKeyboardKey(void) const { return mKeyboardKey; }

					tbCore::tbString GetBindingName(void) const;

				private:
					tbApplication::Key mKeyboardKey;
					tbSystem::Unstable::DeviceHandle mDevice;
					tbSystem::Unstable::ButtonIdentifier mButton;
					tbSystem::Unstable::AxisIdentifier mAxis;

					std::array<float, tbSystem::Unstable::kMaximumDevices * 8> mCurrentAxisValues;
				};

			};	//namespace KeyBinder
		};	//namespace Input
	};	//namespace Core
};	//namespace TyreBytes

#endif /* not ludumdare56_headless_build */
#endif /* Core_KeyBinder_hpp */
