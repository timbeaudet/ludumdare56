///
/// @file
/// @details Provide definitions for the debug channels in LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_Logging_hpp
#define LudumDare56_Logging_hpp

#include <turtle_brains/core/debug/tb_debug_logger.hpp>

//Unsure, this could make circular includes a thing. Perhaps we want to make a DebugObject.hpp/cpp
#include "game_state/race_session_state.hpp"
#include "game_state/racecar_state.hpp"
#include "game_state/driver_state.hpp"
//End of DebugInfo object includes.

namespace LudumDare56
{

	typedef TurtleBrains::Core::Debug::LogWithColor LogWithColor;
	typedef TurtleBrains::Core::Debug::LogGameplay LogGame;
	typedef TurtleBrains::Core::Debug::LogGraphics LogGraphics;
	typedef TurtleBrains::Core::Debug::LogPhysics LogPhysics;
	typedef TurtleBrains::Core::Debug::LogNetwork LogNetwork;

	struct GameServerChannel { static tbCore::tbString AsString(void) { return ("GameServer"); } };
	typedef TurtleBrains::Core::Debug::LogChannelLevel<GameServerChannel> LogGameServer;

	struct GameStateChannel { static tbCore::tbString AsString(void) { return ("GameState"); } };
	typedef TurtleBrains::Core::Debug::LogChannelLevel<GameStateChannel> LogState;

	struct ServerChannel { static tbCore::tbString AsString(void) { return ("Server"); } };
	typedef TurtleBrains::Core::Debug::LogChannelLevel<ServerChannel> LogServer;

	struct ClientChannel { static tbCore::tbString AsString(void) { return ("Client"); } };
	typedef TurtleBrains::Core::Debug::LogChannelLevel<ClientChannel> LogClient;

	struct AudioChannel { static tbCore::tbString AsString(void) { return ("Audio"); } };
	typedef TurtleBrains::Core::Debug::LogChannelLevel<AudioChannel> LogAudio;

	struct AuthenticationChannel { static tbCore::tbString AsString(void) { return ("Auth"); } };
	typedef TurtleBrains::Core::Debug::LogChannelLevel<AuthenticationChannel> LogAuth;

	//Do not log anything secret on Error or Always, otherwise it will be displayed!
	struct SecretChannel { static tbCore::tbString AsString(void) { return ("Secret"); } };
	typedef TurtleBrains::Core::Debug::LogChannelLevel<SecretChannel> LogSecret;

	///
	/// @details Puts quotes in front of, and behind, the string of data when logging, which can be somewhat cleaner
	///   than writing log << "Hello \"" << data << "\" Turtles!";
	///
	class QuotedString
	{
	public:
		explicit QuotedString(const tbCore::tbString& data) :
			mContents(data)
		{
		}

		friend tbCore::Debug::StreamLogger& operator<<(tbCore::Debug::StreamLogger& stream, const QuotedString& quotedString)
		{
			stream << "\"" << quotedString.mContents << "\"";
			return stream;
		}

	private:
		const tbCore::tbString mContents;
	};



	class DebugInfo
	{
	public:
		DebugInfo(const GameState::RacecarState& racecar)
		{
			std::stringstream ss;
			const tbCore::tbString driverName = (GameState::InvalidDriver() == racecar.GetDriverIndex()) ? "no driver" :
				GameState::DriverState::Get(racecar.GetDriverIndex()).GetName();
			ss << "Racecar(" << static_cast<int>(racecar.GetRacecarIndex()) << ", \"" << driverName << "\")";
			mInfo = ss.str();
		}

		DebugInfo(const GameState::RacecarIndex racecarIndex) :
			mInfo("InvalidRacecar")
		{
			if (true == GameState::IsValidRacecar(racecarIndex))
			{
				std::stringstream ss;
				const GameState::RacecarState& racecar = GameState::RacecarState::Get(racecarIndex);
				const tbCore::tbString driverName = (GameState::InvalidDriver() == racecar.GetDriverIndex()) ? "no driver" :
					GameState::DriverState::Get(racecar.GetDriverIndex()).GetName();
				ss << "Racecar(" << static_cast<int>(racecar.GetDriverIndex()) << ", \"" << driverName << "\")";
				mInfo = ss.str();
			}
		}

		DebugInfo(const GameState::DriverState& driver)
		{
			std::stringstream ss;
			ss << "Driver(" << static_cast<int>(driver.GetDriverIndex()) << ", \"" << driver.GetName() << "\")";
			mInfo = ss.str();
		}

		DebugInfo(const GameState::DriverIndex driverIndex) :
			mInfo("InvalidDriver")
		{
			if (true == GameState::IsValidDriver(driverIndex))
			{
				const GameState::DriverState& driver = GameState::DriverState::Get(driverIndex);
				std::stringstream ss;
				ss << "Driver(" << static_cast<int>(driver.GetDriverIndex()) << ", \"" << driver.GetName() << "\")";
				mInfo = ss.str();
			}
		}

		friend std::ostream& operator<<(std::ostream& output, const DebugInfo& debugInfo)
		{
			output << debugInfo.mInfo;
			return output;
		}

	private:
		tbCore::tbString mInfo;
	};

	void SetLoggingLevels(void);

};	//namespace LudumDare56

#endif /* LudumDare56_Logging_hpp */
