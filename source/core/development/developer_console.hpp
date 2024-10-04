///
/// @file
/// @details This is a small terminal/console emulator using ImGui for developer modes.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef Core_DeveloperConsole_hpp
#define Core_DeveloperConsole_hpp

#include <turtle_brains/core/tb_string.hpp>

namespace TurtleBrains
{
	namespace Development
	{

		void ToggleDeveloperConsole(void);

		void DisplayTerminal(void);

		void AddLog(const tbCore::tbString& message);
		void AddLog(const char* format, ...);

	}; //namespace Development
}; //namespace TurtleBrains

// @note The following fuctions will only do anything if development_build is defined, but can be called safely from
//   any build type, which is why it remains outside of the Development namespace.
namespace TyreBytes
{
	namespace Core
	{
		void InitializeDevelopmentTools(const tbCore::tbString& saveDirectory);
		void CleanupDevelopmentTools(void);
	};	//namespace Core
};	//namespace TyreBytes

namespace tbDevelopment = TurtleBrains::Development;

#endif /* Core_DeveloperConsole_hpp */
