///
/// @file
/// @details Holds and collects information about the server.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../game_server/game_server.hpp"

#include "../game_state/race_session_state.hpp"

#include "../network/network_manager.hpp"

#include "../logging.hpp"

#include <turtle_brains/core/tb_dynamic_structure.hpp>
#include <turtle_brains/core/tb_json_parser.hpp>
#include <turtle_brains/core/debug/tb_debug_logger.hpp>
#include <turtle_brains/system/tb_system_timer.hpp>
#include <turtle_brains/system/tb_system_utilities.hpp>
#include <turtle_brains/network/tb_http_request.hpp>
#include <turtle_brains/network/tb_http_response.hpp>
#include <turtle_brains/network/tb_network.hpp>

#include <thread>
#include <chrono>

namespace LudumDare56
{
	namespace GameServer
	{

		// Order of precedence on what IP/Port is actually used:
		//   1. Information provided by tyrebytes master server.
		//   2. server_info.json in the SaveDirectory or as a file next to the executable.
		//   3. GAME_ADDRESS and GAME_PORT environment variables.
		//   4. The hard-coded constant values set at the top of this file.
		const tbCore::tbString kHardcodedServerIP = "127.0.0.1";
		const tbCore::uint16 kHardcodedServerPort = 45001;
		const tbCore::tbString kHardcodedServerFilename = "server_info.json";

		tbCore::tbString theServerIP = "";
		tbCore::uint16 theServerPort = 0;
		bool theServerIsRunning = false;

		namespace Implementation
		{

			void HandleMasterServerResponse(const tbNetwork::HTTP::Response& response);
			void SetupServerInfoWithoutMasterServer(void);

		};	//namespace Implementation
	};	//namespace GameServer
};	//namespace LudumDare56

//Exists in race_session_state.cpp for starting track.
extern tbCore::tbString theDefaultRacetrackName;

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameServer::InitializeServer(void)
{
	PullServerInfo();

	theServerIsRunning = true;
	Network::CreateServerConnection(theServerPort);

	GameState::RaceSessionState::Create(true);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameServer::ShutdownServer(void)
{
	//TODO: Cleanly disconnect all the players on the server, as the server is getting shutdown.
	GameState::RaceSessionState::Destroy();
	Network::DestroyConnection(Network::DisconnectReason::ServerShutdown);
	theServerIsRunning = false;
}

//--------------------------------------------------------------------------------------------------------------------//

int LudumDare56::GameServer::RunDedicatedServer(int argumentCount, const char* argumentValues[])
{
	const UserSettings launchSettings = ParseLaunchParameters(argumentCount, argumentValues);
	const tbCore::tbString startRacetrack = launchSettings.GetString("racetrack");
	if (false == startRacetrack.empty())
	{
		theDefaultRacetrackName = startRacetrack;
	}

	tbSystem::Timer::Timer timer;
	float accumulatedSimulationTime = 0.0f;
	const float kSecondsPerStep(0.01f);

	InitializeServer();

	while (true == theServerIsRunning)
	{
		std::chrono::high_resolution_clock::time_point timeAtStart = std::chrono::high_resolution_clock::now();

		timer.Update();

		const float kMaxDeltaTimePerFrame(1.0f / 10.0f); //Slower than 10fps. Make a warning?
		const float realDeltaTime = static_cast<float>(timer.GetDeltaTime());
		const float deltaTime((realDeltaTime > kMaxDeltaTimePerFrame) ? kMaxDeltaTimePerFrame : realDeltaTime);

		tbNetwork::UpdateNetworking(deltaTime);

		accumulatedSimulationTime += deltaTime;
		int numberOfSimulateCalls(0);
		while (accumulatedSimulationTime > kSecondsPerStep && numberOfSimulateCalls < 5)
		{
			if (false == Network::IsConnected() && false == Network::IsAttemptingToConnect())
			{	//Return an "error"
				tb_always_log(LogServer::Error() << "Failed to create the server connection, timed-out");
				ShutdownServer();
				return 1;
			}
			else
			{
				Network::Simulate();
				GameState::RaceSessionState::Simulate();
			}

			++numberOfSimulateCalls;
			accumulatedSimulationTime -= kSecondsPerStep;
		}

		tb_debug_log_if(accumulatedSimulationTime > kSecondsPerStep, LogServer::Warning() << "Warning, simulation time falling behind wall-timer.");

		std::chrono::high_resolution_clock::duration totalFrameTime = std::chrono::high_resolution_clock::now() - timeAtStart;
		auto sleepTime = std::chrono::milliseconds(10) - std::chrono::duration_cast<std::chrono::milliseconds>(totalFrameTime);
		if (sleepTime > std::chrono::milliseconds(1))
		{	//Could technically sleep a little, this doesn't need to run faster than ~100hz
			//std::this_thread::sleep_for(std::chrono::milliseconds(2));
			std::this_thread::sleep_for(sleepTime);
		}
	}

	ShutdownServer();
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString LudumDare56::GameServer::ServerIP(void)
{
	return theServerIP;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::uint16 LudumDare56::GameServer::ServerPort(void)
{
	return theServerPort;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameServer::PullServerInfo(void)
{
	tbNetwork::HTTP::Request request("https://www.tyrebytes.com/t/ludumdare56_status.json");
	tbNetwork::HTTP::Response response = request.GetResponse();
	Implementation::HandleMasterServerResponse(response);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameServer::PullServerInfo(std::function<void()> callback)
{
	tbNetwork::HTTP::Request request("https://www.tyrebytes.com/t/ludumdare56_status.json");
	request.GetResponseAsync([callback](tbNetwork::HTTP::Response response) {
		Implementation::HandleMasterServerResponse(response);
		callback();
	});
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameServer::Implementation::HandleMasterServerResponse(const tbNetwork::HTTP::Response& response)
{
	bool usingServer(false);
	if (true == response.IsValid())
	{
		const tbCore::DynamicStructure responseData = tbCore::ParseJson(response.GetResponseBody());
		const tbCore::DynamicStructure& serverRunning(responseData["server_running"]);

		if (true == serverRunning.AsBoolean())
		{
			const tbCore::DynamicStructure& serverIP = responseData["server_ip"];
			const tbCore::DynamicStructure& serverPort = responseData["server_port"];
			if (true == serverIP.IsString() && true == serverPort.IsInteger())
			{
				tb_always_log(LogServer::Always() << "Found a running GameServer, using connection info from MasterServer.");
				theServerIP = serverIP.AsString();
				theServerPort = serverPort.AsRangedInteger<tbCore::uint16>("Port out of range.");
				usingServer = true;
			}
		}
	}

	if (false == usingServer)
	{
		SetupServerInfoWithoutMasterServer();
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameServer::Implementation::SetupServerInfoWithoutMasterServer(void)
{
	if (true == tbSystem::DoesFileExist(kHardcodedServerFilename))
	{
		tb_debug_log(LogServer::Always() << "No GameServer running, using server info from \"" << kHardcodedServerFilename << "\".");
		LoadServerInfo(kHardcodedServerFilename);
	}
	else if (true == tbSystem::DoesFileExist(LudumDare56::GetSaveDirectory() + kHardcodedServerFilename))
	{
		tb_debug_log(LogServer::Always() << "No GameServer running, using server info from \"" <<
			LudumDare56::GetSaveDirectory() + kHardcodedServerFilename << "\".");
		LoadServerInfo(LudumDare56::GetSaveDirectory() + kHardcodedServerFilename);
	}
	else
	{
		theServerIP = kHardcodedServerIP;
		theServerPort = kHardcodedServerPort;

		const char* environmentAddress = getenv("GAME_ADDRESS");
		if (nullptr != environmentAddress)
		{
			theServerIP = environmentAddress;
		}

		const char* environmentPort = getenv("GAME_PORT");
		if (nullptr != environmentPort)
		{
			theServerPort = tbCore::FromString<tbCore::uint16>(environmentPort);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameServer::LoadServerInfo(const tbCore::tbString& filepath)
{
	const tbCore::DynamicStructure& connectionData = tbCore::LoadJsonFile(filepath);
	const tbCore::DynamicStructure& addressData = connectionData["ip"];
	const tbCore::DynamicStructure& portData = connectionData["port"];

	tb_error_if(false == addressData.IsString(), "Expected ip to be a string containing an IP address of server.");
	tb_error_if(false == portData.IsInteger(), "Expected port to be a number for the connection port of server.");
	theServerIP = addressData.AsString();
	theServerPort = static_cast<tbCore::uint16>(portData.AsInteger());
}

//--------------------------------------------------------------------------------------------------------------------//
