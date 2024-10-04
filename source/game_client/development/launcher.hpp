///
/// @file
/// @details Launch multiple copies of LudumDare56 game for multiplayer debugging/testing sessions.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_MultipleClientLauncher_hpp
#define LudumDare56_MultipleClientLauncher_hpp
#if defined(development_build)

#include "../../user_settings.hpp"

#include <turtle_brains/application/tb_application_window.hpp>

namespace LudumDare56
{
	namespace GameClient
	{
		namespace Development
		{

			tbApplication::WindowProperties LaunchMultipleWindows(const tbCore::tbString& executablePath,
				const tbCore::int64 count2or4, const tbApplication::WindowProperties& originalWindow);

		};	//namespace Development
	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* development_build */
#endif /* LudumDare56_MultipleClientLauncher_hpp */
