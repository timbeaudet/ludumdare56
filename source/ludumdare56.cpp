///
/// @file
/// @details Entry point of the LudumDare56 project.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "ludumdare56.hpp"
#include "logging.hpp"
#include "version.hpp"

#include "game_server/game_server.hpp"
#include "core/utilities.hpp"
#include "core/services/connector_service_interface.hpp"
#include "core/development/developer_console.hpp" //where Init/Cleanup DevTools lives.

#include "network/network_handlers.hpp"

#include <turtle_brains/system/tb_system_utilities.hpp>
#include <turtle_brains/core/unit_test/tb_unit_test.hpp>
#include <turtle_brains/core/tb_version.hpp>

#include <ice/core/ice_version.hpp>

#include <track_bundler/version.hpp>

#ifndef ludumdare56_headless_build

#include "game_client/scenes/scene_manager.hpp"
#include "game_client/scenes/racing_scene.hpp"
#include "game_client/development/launcher.hpp"

#include <turtle_brains/system/tb_system_timer.hpp>

#include <ice/game/ice_game_application.hpp>
#include <ice/game/ice_game_scene.hpp>

/// Setting Up Developer Key
///
/// 0. If the project lead has not done so, create a file "run/developer_key" using uuidgen or a method to create a
///    random string unlikely to be used/guessed by others. This is almost like a password. This does not get commited
///    to source control and will need to be copied to any machines that will deploy / create a game server. Talk to Tim!
/// 1. Create a copy of the developer_key and save it in your users SaveDirectory (LocalAppData etc) while appending
///    your name. If the key was "abcdef" then you'd save "abcdefTimBeaudet" for a name of TimBeaudet.
/// 2. The GameClient will send this key to the GameServer, so when not running locally it the developer_key needs to
///    be copied to the GameServer. (in theory this is happening from the deploy / CreateGameServer scripts).
/// 3. Note that using a DeveloperKey may skip other forms of authentication like through Twitch/Patreon etc.
///

// When first starting LudumDare we will need to (at the very least, there could be additional bumps along the way)
//   1. Create a GridSpot component which will register a GridSlot. This used to be the job of "Logic Objects" which
//      we no longer use Object Definitions. Those are Legacy track builder things and Node/Components are the future.
//   2. Create a Trigger of sorts, although we might be able to reuse the built-in BoxCollider component and even
//      Track Bundler might have a helper function to get this into world/physics already, who know about tying events.
//   3. Any additional fallout from the removal of almost all assets in Trophy Brawlers to create the LD empty project.

// Create a bit wider track with longer sweeping corners as the swarms won't like sharp corners!
// Place the camera in the center of the whole swarm, or perhaps in between the swarm center and target center?
//    Ideally the target arrow will go away.
// Continue tuning and at some point around 8 or 9am hard-stop tuning, wrap up the rest of the 'game' by killing the
//   creatures that fall off the track/hit obstacles, and add time & scoring.
//   Some tuning ideas; we might want to slow the acceleration of the "target" as the creature gets closer to it.
//   Ideally when the target is not moving, niether would the swarm?? Hmm - not really how they work. Good Luck!
//
//   By 11am, noon at the latest I want to be FINISHED with the core-game loop and game-play, and begin adding content
//   and polish for the rest of the weekend.

//--------------------------------------------------------------------------------------------------------------------//

namespace LudumDare56
{
	namespace GameClient
	{
		tbGame::GameApplication* theGameApplication = nullptr;

		int Main(int argumentCount, const char* argumentValues[]);
	};

	String theQuickPlayRacetrackPath = "";
};

int LudumDare56::GameClient::Main(int argumentCount, const char* argumentValues[])
{
	const UserSettings launchSettings = ParseLaunchParameters(argumentCount, argumentValues);

	if (true == launchSettings.GetBoolean("server") && true == launchSettings.GetBoolean("headless"))
	{
		return GameServer::RunDedicatedServer(argumentCount, argumentValues);
	}

	if (true == launchSettings.GetBoolean("developer"))
	{
		const String developerKey = TyreBytes::Core::Utilities::LoadFileContentsToString(GetSaveDirectory() + "developer_key", true);
		tb_always_log(LogGame::Warning() << ((true == developerKey.empty()) ? "No DeveloperKey was found or loaded." : "[DeveloperKey] successfully loaded."));
		if (false == developerKey.empty())
		{
			Network::ClientPacketHandler::SetUserAccessKey(developerKey, Network::AuthenticationService::Developer);
		}
	}

	TheUserSettings().CreateDefaultSettings();
	TheUserSettings().LoadSettings("settings.cfg");

	const bool kFullscreenDefault = false;

	// TODO: LudumDare56: TurtleBrains does not support ensuring the application is in a viewable area. There is/was
	//   a comment talking about Windows managing that for us with CW_USEDEFAULT in TrailingBrakes (2023-10-22).
	tbApplication::WindowProperties windowProperties;
	windowProperties.mWindowMode = launchSettings.GetBoolean("", TheUserSettings().GetBoolean(Settings::Fullscreen(), kFullscreenDefault))
		? tbApplication::WindowMode::FullScreen : tbApplication::WindowMode::Windowed;

	windowProperties.mVerticalSync = launchSettings.GetBoolean(Settings::VerticalSync(), TheUserSettings().GetBoolean(Settings::VerticalSync(), true));
	windowProperties.mWindowPositionX = static_cast<tbCore::int16>(launchSettings.GetInteger(Settings::WindowPositionX(), TheUserSettings().GetInteger(Settings::WindowPositionX(), 100)));
	windowProperties.mWindowPositionY = static_cast<tbCore::int16>(launchSettings.GetInteger(Settings::WindowPositionY(), TheUserSettings().GetInteger(Settings::WindowPositionY(), 100)));
	windowProperties.mWindowWidth = static_cast<tbCore::int16>(launchSettings.GetInteger(Settings::WindowWidth(), TheUserSettings().GetInteger(Settings::WindowWidth(), 1280)));
	windowProperties.mWindowHeight = static_cast<tbCore::int16>(launchSettings.GetInteger(Settings::WindowHeight(), TheUserSettings().GetInteger(Settings::WindowHeight(), 720)));

#if defined(development_build) && defined(tb_windows)
	if (0 != launchSettings.GetInteger("multi"))
	{
		windowProperties = Development::LaunchMultipleWindows(argumentValues[0], launchSettings.GetInteger("multi"), windowProperties);
	}
#endif /* development_build */

	iceGame::GameApplication gameApplication(windowProperties, true);
	theGameApplication = &gameApplication;
	gameApplication.SetWindowTitle(tb_string("LudumDare56"));

	TyreBytes::Core::InitializeDevelopmentTools(GetSaveDirectory());
	iceGraphics::LoadInternalShaders();

	tbGraphics::theSpriteManager.LoadSpriteSheetFromFile("interface_sheet", "data/interface/interface_sheet.json");
	tbGraphics::theSpriteManager.LoadSpriteSheetFromFile("hud_sheet", "data/interface/heads_up_display.json");
	tbGraphics::TextureHandle palette64 = tbGraphics::theTextureManager.CreateTextureFromFile("data/textures/palette64.png", false);
	tbGraphics::theTextureManager.SetTextureFiltering(palette64, tbGraphics::TextureFilter::Closest);

	tbAudio::theAudioManager.LoadEventTable("audio_events", "data/audio/audio_events.json");
	tbDevelopment::ConfigObject::AddConfigurationFile("data/runtime_configuration.json");

	SceneManager::CreateSceneManager();

	if (true == launchSettings.GetBoolean("server"))
	{
		RacingScene::SetGameMode(RacingScene::GameMode::MultiplayerHost);
		gameApplication.RunGame(SceneManager::GetScene(SceneId::kRacingScene));
	}
	else if (false == launchSettings.GetString("play_track").empty())
	{
		theQuickPlayRacetrackPath = launchSettings.GetString("play_track");
		RacingScene::SetGameMode(RacingScene::GameMode::Singleplayer);
		gameApplication.RunGame(SceneManager::GetScene(SceneId::kRacingScene));
	}
	else
	{
		gameApplication.RunGame(SceneManager::GetScene(SceneId::kTitleScene));
	}

	// TODO: LudumDare56: 2023-10-22: The recent iceCore::MeshData vs iceMesh::VisualMeshes seem to be causing an
	//   issue where we don't cleanly shutdown, so nothing below this is actually happening right now. Good luck!
	//   It did seem to have saved a settings file at some point, so it might not repro 100% of the time?
	//
	// 2023-11-01: Haven't really seen or heard of any issues with the saving of user data, so it might be working now?
#if defined(development_build)
	// Do not save any settings when using the multiplayer test mode to create 4 clients quickly; we might wish to have
	//   various setting files much like we did with the client logs?
	if (0 == launchSettings.GetInteger("multi") && 0 == launchSettings.GetInteger("split", 0))
#endif
	{
		const tbApplication::WindowProperties currentWindowProperties = gameApplication.GetWindowProperties();
		TheUserSettings().SetInteger("window_position_x", currentWindowProperties.mWindowPositionX);
		TheUserSettings().SetInteger("window_position_y", currentWindowProperties.mWindowPositionY);
		TheUserSettings().SetInteger("window_width", currentWindowProperties.mWindowWidth);
		TheUserSettings().SetInteger("window_height", currentWindowProperties.mWindowHeight);
		TheUserSettings().SaveSettings("settings.cfg");
	}

	TyreBytes::Core::CleanupDevelopmentTools();

	SceneManager::DestroySceneManager();
	theGameApplication = nullptr;

	return 0;
}

#endif /* not ludumdare56_headless_build */

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::UserSettings LudumDare56::ParseLaunchParameters(int argumentCount, const char* argumentValues[])
{
	UserSettings launchSettings;

	{
		// @note 2023-10-25: Currently Track Builder will launch the game with and set the track file parameter and
		//   that is all the info we get. Future Track Builder may have per-project settings which could use a string
		//   to specify launch parameters and have a --track filepath and this could then be setup without any extra
		//   check, such that argumentCount is exactly 2 and ending in .trk.
		//
		//   That said, even if Track Builder supports custom launch parameters, this check might still be worthwhile
		//   for dragging a trackfile onto the executable in the manner Windows does it, to just launch into that track.
		if (2 == argumentCount)
		{
			const String argument = argumentValues[1];
			if (tbCore::StringContains(argument, ".trk"))
			{
				launchSettings.SetBoolean("developer", true);
				launchSettings.SetString("play_track", argument);

				//Converts any \ slashes into /
				const String executable = tbSystem::PathToNormalSlashes(argumentValues[0]);
				const String executablePath = executable.substr(0, executable.find_last_of("/"));
				tbSystem::SetCurrentWorkingDirectory(executablePath);
			}
		}
	}

	const std::map<String, String> booleanArgumentToKeys = {
		{ "--headless", "headless" },
		{ "--server", "server" },
		{ "--developer", "developer" },
	};

	const std::map<String, String> intArgumentToKeys = {
		{ "--x", "window_position_x" },
		{ "--y", "window_position_y" },
		{ "--width", "window_width" },
		{ "--height", "window_height" },
		{ "--multi", "multi" },
		{ "--split", "split" },
	};

	const std::map<String, String> stringArgumentToKeys = {
		{ "--log", "client_log" },
		{ "--track", "racetrack" },
		{ "--racetrack", "racetrack" },
	};

	for (int argumentIndex = 0; argumentIndex < argumentCount; ++argumentIndex)
	{
		for (const auto& nameKeyPair : booleanArgumentToKeys)
		{
			if (argumentValues[argumentIndex] == nameKeyPair.first)
			{
				launchSettings.SetBoolean(nameKeyPair.second, true);
			}
		}

		for (const auto& nameKeyPair : intArgumentToKeys)
		{
			if (argumentValues[argumentIndex] == nameKeyPair.first && argumentIndex + 1 < argumentCount)
			{
				++argumentIndex;
				launchSettings.SetInteger(nameKeyPair.second, tbCore::FromString<tbCore::int64>(argumentValues[argumentIndex]));
			}
		}

		for (const auto& nameKeyPair : stringArgumentToKeys)
		{
			if (argumentValues[argumentIndex] == nameKeyPair.first && argumentIndex + 1 < argumentCount)
			{
				++argumentIndex;
				launchSettings.SetString(nameKeyPair.second, argumentValues[argumentIndex]);
			}
		}
	}

	return launchSettings;
}

//--------------------------------------------------------------------------------------------------------------------//

int main(const int argumentCount, const char* argumentValues[])
{
	for (int argumentIndex = 0; argumentIndex < argumentCount; ++argumentIndex)
	{	//Run unit tests if --test is present as an argument.
		if (LudumDare56::String("--test") == argumentValues[argumentIndex])
		{
			LudumDare56::String testHeader = "Testing " + LudumDare56::Version::ProjectVersionString();
			return (true == TurtleBrains::Core::UnitTest::RunAllTests(testHeader)) ? 0 : 1;
		}
	}

	// Run --test above this so it can output the results into the nightly build emails.

	const LudumDare56::UserSettings launchSettings = LudumDare56::ParseLaunchParameters(argumentCount, argumentValues);

#if defined(ludumdare56_headless_build)
	tbCore::Debug::OpenLog(LudumDare56::GetSaveDirectory() + launchSettings.GetString("server_log", "server_log.txt"), true);
	LudumDare56::SetLoggingLevels();
	tb_always_log(LudumDare56::LogGameServer::Always() << "LudumDare56 Dedicated Server v" << LudumDare56::Version::VersionString());
	tb_always_log(LudumDare56::LogGameServer::Always() << "    " << TrackBundler::Version::ProjectVersionString());
	tb_always_log(LudumDare56::LogGameServer::Always() << "    " << iceCore::Version::ProjectVersionString());
	tb_always_log(LudumDare56::LogGameServer::Always() << "    " << tbCore::Version::ProjectVersionString());

	int returnCode = tb_debug_project_entry_point_with(LudumDare56::GameServer::RunDedicatedServer, argumentCount, argumentValues);
#else
	tbCore::Debug::OpenLog(LudumDare56::GetSaveDirectory() + launchSettings.GetString("client_log", "client_log.txt"), true);
	LudumDare56::SetLoggingLevels();
	tb_always_log(LudumDare56::LogGame::Always() << "Starting " << LudumDare56::Version::ProjectVersionString());
	tb_always_log(LudumDare56::LogGame::Always() << "    " << TrackBundler::Version::ProjectVersionString());
	tb_always_log(LudumDare56::LogGame::Always() << "    " << iceCore::Version::ProjectVersionString());
	tb_always_log(LudumDare56::LogGame::Always() << "    " << tbCore::Version::ProjectVersionString());

	int returnCode = tb_debug_project_entry_point_with(LudumDare56::GameClient::Main, argumentCount, argumentValues);
#endif

	tb_always_log(LudumDare56::LogGame::Always() << "Clean shutdown.");
	tbCore::Debug::CloseLog();
	return returnCode;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::String LudumDare56::GetSaveDirectory(void)
{
#if defined(ludumdare56_headless_build)
	return ""; //Save directory is next to the executable.
#else
	const String saveDirectory = tbSystem::UserDirectoryPath() + "TyreBytes/LudumDare56/";
	tbSystem::CreateDirectoryPath(tbSystem::PathToSystemSlashes(saveDirectory));
	return tbSystem::PathToNormalSlashes(saveDirectory);
#endif
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::String LudumDare56::GetQuickPlayRacetrackPath(void)
{
#if defined(ludumdare56_headless_build)
	return "";
#else
	return theQuickPlayRacetrackPath;
#endif
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::String LudumDare56::GetTwitchClientID(void)
{
	return "4eiyy9tb2yokwnvplgngwlmwr7tn7s";
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::String LudumDare56::GetPatreonClientID(void)
{
	return "Ylo1Kxe69Mz2glgPOUNhbhiKAYicATFc33Ik7E_7exta5Um6ox7-Nj2UbE5S3EC5";
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::String LudumDare56::GetYouTubeClientID(void)
{
	return "119120827043-5369dabc9eo7ornuvrnt9f0drtncandp.apps.googleusercontent.com";
}

//--------------------------------------------------------------------------------------------------------------------//
