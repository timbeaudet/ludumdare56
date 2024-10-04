///
/// @file
/// @details Holds and collects information about the server.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_GameServer_hpp
#define LudumDare56_GameServer_hpp

#include <turtle_brains/core/tb_string.hpp>
#include <turtle_brains/core/tb_types.hpp>

#include <functional>

namespace LudumDare56
{
	namespace GameServer
	{

		void InitializeServer(void);
		void ShutdownServer(void);

		///
		/// @details This is just like main() for a dedicated server, it will initialize and cleanup any/all resouces
		///   needed to run the dedicated server including the GameState, racetrack etc.
		///
		int RunDedicatedServer(int argumentCount, const char* argumentValues[]);

		tbCore::tbString ServerIP(void);
		tbCore::uint16 ServerPort(void);

		///
		/// @details Will ping TyreBytes server to get the IP of the GameServer if one exists. This is a blocking call and
		///   will not return until the response comes back.
		///
		void PullServerInfo(void);

		///
		/// @details Will ping TyreBytes server to get the IP of the GameServer if one exists. This is a non-blocking call
		///   which will call callback once finished.
		///
		void PullServerInfo(std::function<void()> callback);

		void LoadServerInfo(const tbCore::tbString& filepath);

	};	//namespace GameServer
};	//namespace LudumDare56

#endif /* LudumDare56_ServerInformation_hpp */
