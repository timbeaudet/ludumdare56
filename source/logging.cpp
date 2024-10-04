///
/// @file
/// @details Provide definitions for the debug channels in LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "logging.hpp"

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::SetLoggingLevels(void)
{
	tbCore::Debug::TheLogger().SetLogLevel("tbInternal", tbCore::Debug::LogLevel::kWarning);

	//If ever changed from kError, do NOT show the logs on stream!!    SECRET SECRET SECRET
	//  also never log anything secret on Error or Always, otherwise it will be displayed!
	tbCore::Debug::TheLogger().SetLogLevel(LogSecret::AsString(), tbCore::Debug::LogLevel::kError);

	tbCore::Debug::TheLogger().SetLogLevel(LogNetwork::AsString(), tbCore::Debug::LogLevel::kInfo);
	tbCore::Debug::TheLogger().SetLogLevel(LogServer::AsString(), tbCore::Debug::LogLevel::kInfo);
	tbCore::Debug::TheLogger().SetLogLevel(LogClient::AsString(), tbCore::Debug::LogLevel::kInfo);
	tbCore::Debug::TheLogger().SetLogLevel(LogAuth::AsString(), tbCore::Debug::LogLevel::kInfo);

	tbCore::Debug::TheLogger().SetLogLevel(LogGameServer::AsString(), tbCore::Debug::LogLevel::kInfo);

	tbCore::Debug::TheLogger().SetLogLevel(LogState::AsString(), tbCore::Debug::LogLevel::kInfo);
	tbCore::Debug::TheLogger().SetLogLevel(LogGame::AsString(), tbCore::Debug::LogLevel::kInfo);
	tbCore::Debug::TheLogger().SetLogLevel(LogPhysics::AsString(), tbCore::Debug::LogLevel::kInfo);
	tbCore::Debug::TheLogger().SetLogLevel(LogGraphics::AsString(), tbCore::Debug::LogLevel::kInfo);
}

//--------------------------------------------------------------------------------------------------------------------//
