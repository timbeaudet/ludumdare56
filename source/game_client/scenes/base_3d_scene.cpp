///
/// @file
/// @details This is the base for all scenes that will be requiring online connection with the GameServer. If the
///   connection is lost the game will return to Title with a message.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "base_3d_scene.hpp"
#include "title_scene.hpp"
#include "scene_manager.hpp"

#include "../../network/network_manager.hpp"
#include "../../game_server/game_server.hpp"

#if defined(development_build)
#include "../../game_state/racecar_state.hpp"
#include "../../core/development/tb_imgui_implementation.hpp"
#include "../../core/development/developer_console.hpp"
#endif /* development_build */

namespace
{
	bool theGameIsDebugServer = false;
}

bool LudumDare56::GameClient::Base3dScene::sIsDebugging = false;
bool LudumDare56::GameClient::Base3dScene::sIsDebuggingPhysics = false;

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::Base3dScene::StartAsDebugServer(void)
{
	//Commented out as was in TrailingBrakes
	tb_error("Not Yet Implemented");

//	theGameIsDebugServer = true;
//	GameServer::InitializeServer();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::Base3dScene::DestroyConnection(void)
{
	if (true == theGameIsDebugServer)
	{
		//Commented out as was in TrailingBrakes
		tb_error("Not Yet Implemented");

		//GameServer::ShutdownServer();
	}
	else
	{
		GameState::RaceSessionState::Destroy();
		Network::DestroyConnection(Network::DisconnectReason::Graceful);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::Base3dScene::Base3dScene(void) :
	iceGame::GameScene(90.0_degrees, 1.0f, 5000.0f, false)
#if defined(development_build)
	, mProfiler()
#endif /* development_build */
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::Base3dScene::~Base3dScene(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::Base3dScene::OnSimulate(void)
{
	iceGame::GameScene::OnSimulate();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::Base3dScene::OnUpdate(const float deltaTime)
{
	iceGame::GameScene::OnUpdate(deltaTime);
	UpdateDeveloperStuffs(deltaTime);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::Base3dScene::SimulateGameState(void)
{
	GameState::RaceSessionState::Simulate();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::Base3dScene::SimulateNetworkAndGameState(void)
{
	if (true == Network::IsConnected())
	{
		Network::Simulate();
	}

	//It is possible the NetworkManager::Simulate() just disconnected from the server, so we check a second time and
	//  jump back to the TitleScene when disconnected...
	if (true == Network::IsConnected())
	{
		SimulateGameState();
	}
	else
	{
		TitleScene::GotoTitleWithMessage("Lost connection with the GameServer.");
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::Base3dScene::UpdateDeveloperStuffs(const float deltaTime)
{
#if defined(development_build)
	if (true == tbApplication::Input::IsKeyReleased(tbApplication::tbKeyTilde) || true == tbApplication::Input::IsKeyReleased(tbApplication::tbKeyF1))
	{
		sIsDebugging = !sIsDebugging;
		TurtleBrains::Development::ToggleDeveloperConsole();
	}
	if (true == tbApplication::Input::IsKeyReleased(tbApplication::tbKeyF2))
	{
		sIsDebuggingPhysics = !sIsDebuggingPhysics;
	}

	if (true == sIsDebugging)
	{
		TyreBytes::Core::Development::tbImGui::UpdateFrame(deltaTime);
	}
#else
	tb_unused(deltaTime);
#endif /* development_build */
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::Base3dScene::OnPerspectiveRender(void) const
{
	iceGame::GameScene::OnPerspectiveRender();
}

//--------------------------------------------------------------------------------------------------------------------//


void LudumDare56::GameClient::Base3dScene::OnOrthographicRender(void) const
{
	iceGame::GameScene::OnOrthographicRender();

	/// @note 2023-10-25: We can't call DisplayDeveloperConsole here without losing the performance timers. The Render
	///   timer should account for any time spent in this function too. A child objects OnOrthographicRender() should
	///   have the following contents to end the performance and display stuffs.
	///   {
	///       Base3dScene::OnOrthographicRender();
	///       { Any custom code/behavior }
	///       ludumdare56_stop_timer(TimingChannel::kRender);
	///       DisplayDeveloperConsole();
	///   }

	//DisplayDeveloperConsole();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::Base3dScene::OnInterfaceRender(void) const
{
	iceGame::GameScene::OnInterfaceRender();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::Base3dScene::OnOpen(void)
{
	iceGame::GameScene::OnOpen();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::Base3dScene::OnClose(void)
{
	iceGame::GameScene::OnClose();
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameClient::Base3dScene::IsDeveloperConsoleOpen(void) const
{
	return sIsDebugging;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::Base3dScene::DisplayDeveloperConsole(void) const
{
#if defined(development_build)
	if (true == sIsDebugging)
	{
		TurtleBrains::Development::DisplayTerminal();

		ImGui::SetNextWindowSize(ImVec2(480, 340), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(794, 10), ImGuiCond_FirstUseEver);
		if (true == ImGui::Begin("Profiler"))
		{
			TyreBytes::Core::Development::FrameProfiler::ImGuiShowPerformance(mProfiler);
			Network::Development::ImGuiShowNetworkHistory();
		}
		ImGui::End();

		TyreBytes::Core::Development::tbImGui::RenderFrame();
	}
#endif /* development_build */
}

//--------------------------------------------------------------------------------------------------------------------//
