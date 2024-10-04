///
/// @file
/// @details Launch multiple copies of LudumDare56 game for multiplayer debugging/testing sessions.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/development/launcher.hpp"

#include <turtle_brains/core/tb_platform.hpp>
#include <turtle_brains/core/tb_string.hpp>
#include <turtle_brains/core/tb_defines.hpp>

#if defined(development_build)

//--------------------------------------------------------------------------------------------------------------------//

void StartWindowsApplication(const tbCore::tbString& applicationName, const tbCore::tbString& commandLine)
{
#if defined(tb_windows)
	STARTUPINFOA startupInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);

	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo, sizeof(processInfo));

	tb_error_if(commandLine.size() >= 512, "Not enough space in command Buffer.");
	char commandBuffer[512];
	strcpy(commandBuffer, commandLine.c_str());

	CreateProcessA(applicationName.c_str(), commandBuffer, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInfo);
	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);
#else
	tb_unused(applicationName);
	tb_unused(commandLine);
#endif /* tb_windows */
}

//--------------------------------------------------------------------------------------------------------------------//

tbApplication::WindowProperties LudumDare56::GameClient::Development::LaunchMultipleWindows(
	const tbCore::tbString& executablePath, const tbCore::int64 clientCount,
	const tbApplication::WindowProperties& originalWindow)
{
#if defined(tb_windows)
	const tbCore::uint16 titleBar = 40;
	const tbCore::uint16 screenX = 1920; //This is not 0 because Tim's primary monitor is an oddball (5760x1080, 1920x is center part)
	const tbCore::uint16 screenY = 0;
	const tbCore::uint16 windowWidth = 1920 / 2;                 //960
	const tbCore::uint16 windowHeight = (1080 / 2) - titleBar;   //500

	//tbCore::int16& windowX = windowProperties.mWindowPositionX;
	//tbCore::int16& windowY = windowProperties.mWindowPositionY;

	//windowProperties.mWindowPositionY = originalWindow.mWindowPositionY;
	//windowProperties.mWindowWidth = windowWidth;
	////The following is not changed to windowHeight like above, because I was afraid it would move the primary (1) around.
	//windowProperties.mWindowHeight = 1080 / 2;

	//tbCore::int16 windowX = 0;
	//tbCore::int16 windowY = 0;

	//if (1 == splitWindow) { windowX = screenX; windowY = screenY; }
	//if (2 == splitWindow) { windowX = screenX + windowWidth; windowY = screenY; }
	//if (3 == splitWindow) { windowX = screenX; windowY = screenY + windowHeight; }
	//if (4 == splitWindow) { windowX = screenX + windowWidth; windowY = screenY + windowHeight; }

	const tbCore::int16 x1 = screenX;
	const tbCore::int16 x2 = screenX + windowWidth;
	const tbCore::int16 x3 = screenX;
	const tbCore::int16 x4 = screenX + windowWidth;

	const tbCore::int16 y1 = screenY;
	const tbCore::int16 y2 = screenY;
	const tbCore::int16 y3 = screenY + windowHeight;
	const tbCore::int16 y4 = screenY + windowHeight;

	DWORD pauseTime = 250;

	if (2 == clientCount)
	{
		StartWindowsApplication(executablePath, "--developer --split 2 --x " + tbCore::ToString(x2) + " --y " + tbCore::ToString(y2) +
			" --width " + tbCore::ToString(windowWidth) + " --height " + tbCore::ToString(windowHeight) + " --fullscreen 0 --log client_2_log.txt");
		Sleep(pauseTime);
	}
	else if (3 == clientCount)
	{
		StartWindowsApplication(executablePath, "--developer --split 2 --x " + tbCore::ToString(x2) + " --y " + tbCore::ToString(y2) +
			" --width " + tbCore::ToString(windowWidth) + " --height " + tbCore::ToString(windowHeight) + " --fullscreen 0 --log client_2_log.txt");
		Sleep(pauseTime);

		StartWindowsApplication(executablePath, "--developer --split 3 --x " + tbCore::ToString(x3) + " --y " + tbCore::ToString(y3) +
			" --width " + tbCore::ToString(windowWidth) + " --height " + tbCore::ToString(windowHeight) + " --fullscreen 0 --log client_3_log.txt");
		Sleep(pauseTime);
	}
	else if (4 == clientCount)
	{	// @note 2023-10-25: We start with split 4 instead of 2, 3, 4 because the console log will hide the third split.
		StartWindowsApplication(executablePath, "--developer --split 4 --x " + tbCore::ToString(x4) + " --y " + tbCore::ToString(y4) +
			" --width " + tbCore::ToString(windowWidth) + " --height " + tbCore::ToString(windowHeight) + " --fullscreen 0 --log client_4_log.txt");
		Sleep(pauseTime);

		StartWindowsApplication(executablePath, "--developer --split 2 --x " + tbCore::ToString(x2) + " --y " + tbCore::ToString(y2) +
			" --width " + tbCore::ToString(windowWidth) + " --height " + tbCore::ToString(windowHeight) + " --fullscreen 0 --log client_2_log.txt");
		Sleep(pauseTime);

		StartWindowsApplication(executablePath, "--developer --split 3 --x " + tbCore::ToString(x3) + " --y " + tbCore::ToString(y3) +
			" --width " + tbCore::ToString(windowWidth) + " --height " + tbCore::ToString(windowHeight) + " --fullscreen 0 --log client_3_log.txt");
		Sleep(pauseTime);
	}

	tbApplication::WindowProperties windowProperties = originalWindow;
	windowProperties.mWindowWidth = windowWidth;
	windowProperties.mWindowHeight = windowHeight;
	windowProperties.mWindowPositionX = x1;
	windowProperties.mWindowPositionY = y1;
	return windowProperties;
#else
	tb_unused(executablePath);
	tb_unused(clientCount);
	return originalWindow;
#endif /* windows */
}

//--------------------------------------------------------------------------------------------------------------------//

#endif /* development_build */
