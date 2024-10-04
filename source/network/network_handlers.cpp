///
/// @file
/// @details Defines the packet handlers for LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../network/network_handlers.hpp"
#include "../network/network_packets.hpp"
#include "../network/network_manager.hpp"
#include "../network/networked_racecar_controller.hpp"

#include "../core/services/connector_service_interface.hpp"
#include "../game_state/race_session_state.hpp"
#include "../game_state/racecar_state.hpp"
#include "../game_state/driver_state.hpp"
#include "../game_state/racetrack_state.hpp"
#include "../game_state/timing_and_scoring_state.hpp"
#include "../game_state/events/race_session_events.hpp"
#include "../game_state/events/timing_events.hpp"
#include "../game_state/events/racecar_events.hpp"

#include "../version.hpp"
#include "../logging.hpp"
#include "../ludumdare56.hpp"

#include "../core/utilities.hpp"

//#include "../game_client/development/console_command_system.hpp"

#include <turtle_brains/tb_core_kit.hpp>
#include <turtle_brains/math/tb_random_numbers.hpp>
#include <turtle_brains/math/tb_quaternion.hpp>
#include <turtle_brains/math/tb_matrix_quaternion.hpp>
#include <turtle_brains/network/tb_socket_connection.hpp>

namespace
{
	const tbCore::uint32 kInvalidRegistrationCode = 0;
	const tbCore::byte kInvalidConnection = tbNetwork::InvalidClientID();

	tbCore::tbString theUserAccessKey = "";
	LudumDare56::Network::AuthenticationService theAuthenticationService = LudumDare56::Network::AuthenticationService::Unknown;
};

namespace LudumDare56
{
	namespace Network
	{
		namespace Implementation

		{	//This is found in network_manager.cpp and ... while it shouldn't be used here, it gets stuff done.
			extern tbNetwork::SocketConnection* theSafeConnection;
			extern tbNetwork::SocketConnection* theFastConnection;
		};
	};
};

using LudumDare56::Network::Implementation::theSafeConnection;
using LudumDare56::Network::Implementation::theFastConnection;

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::LudumDare56PacketHandlerInterface::LudumDare56PacketHandlerInterface(void) :
	tbNetwork::PacketHandlerInterface(),
	mHandlingSafeConnection(false)
{
	for (GameState::RacecarState& racecar : GameState::RacecarState::AllMutableRacecars())
	{
		racecar.AddEventListener(*this);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::LudumDare56PacketHandlerInterface::~LudumDare56PacketHandlerInterface(void)
{
	for (GameState::RacecarState& racecar : GameState::RacecarState::AllMutableRacecars())
	{
		racecar.RemoveEventListener(*this);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::LudumDare56PacketHandlerInterface::FixedUpdate(tbCore::uint32 /*deltaTimeMS*/)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::PacketType LudumDare56::Network::LudumDare56PacketHandlerInterface::GetActualPacketType(
	const tbCore::byte* packetData, size_t packetSize) const
{
	const PacketType packetType = static_cast<PacketType>(packetData[1]);
	if (PacketType::Tiny == packetType)
	{
		return static_cast<PacketType>(ToPacket<TinyPacket>(packetData, packetSize).subtype);
	}
	else if (PacketType::Small == packetType)
	{
		return static_cast<PacketType>(ToPacket<SmallPacket>(packetData, packetSize).subtype);
	}
	else if (PacketType::LargePayload == packetType)
	{
		return static_cast<PacketType>(ToPacket<LargePayloadPacket>(packetData, packetSize).subtype);
	}

	return packetType;
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ClientPacketHandler::SetUserAccessKey(const tbCore::tbString& userAccessKey, const AuthenticationService service)
{
	tb_always_log(LogNetwork::Info() << "ClientPacketHandler setting UserAccessKey to " << service << " service.");
	theUserAccessKey = userAccessKey;
	theAuthenticationService = service;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::ClientPacketHandler::HasUserAccessKey(void)
{
	return (AuthenticationService::Unknown != theAuthenticationService);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::ClientPacketHandler::ClientPacketHandler(void) :
	LudumDare56PacketHandlerInterface(),
	mPingMonitor(false),
	mLargePayload(),
	mLastUpdateTimes(),
	mRegistrationTimer(0),
	mRegistrationCode(kInvalidRegistrationCode),
	mPlayerDriverIndex(GameState::InvalidDriver()),
	mIsAuthenticated(false),
	mIsRegistered(false),
	mIsReadyToPlay(false)
{
	for (tbCore::uint32& updateTime : mLastUpdateTimes)
	{
		updateTime = 0;
	}
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::ClientPacketHandler::~ClientPacketHandler(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ClientPacketHandler::FixedUpdate(tbCore::uint32 deltaTimeMS)
{
	if (true == mIsAuthenticated && (nullptr == theFastConnection || true == mIsRegistered))
	{
		mPingMonitor.Update(deltaTimeMS);
	}

	if (true == mIsAuthenticated && false == mIsRegistered && kInvalidRegistrationCode != mRegistrationCode)
	{
		tb_error_if(deltaTimeMS != tbGame::GameTimer::GetMillisecondsPerStep(), "Expected the fixedTime step to match.");
		if (true == mRegistrationTimer.DecrementStep())
		{
			//TODO: RallyOfRockets: 2022-04-20: Disconnect for time-out and go to the error scene with a message.
			//  This todo is different from calling the DestroyConnectionSoon() below as it should be more user-friendly
			//  to send them to an error screen with message.
			//static bool logMessageOnce = true;
			//tb_always_log_if(true == logMessageOnce, LogNetwork::Error() << "Timed out while trying to register the fast connection.");
			//logMessageOnce = false;

			tb_always_log(LogClient::Error() << "Timed out while trying to register the fast connection.");
			Network::DestroyConnectionSoon(DisconnectReason::UnregisteredTimeout);
		}
		else if (0 == mRegistrationTimer.GetRemainingTime() % 100)
		{	//Send a new request every tenth of a second or so until registered, some packets may have got lost.
			Network::SendFastPacket(CreateSmallPacket(PacketType::RegistrationRequest, mRegistrationCode, GetDriverIndexForPlayer()));
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarIndex LudumDare56::Network::ClientPacketHandler::GetRacecarIndexForPlayer(void) const
{
	if (true == IsValidDriver(GetDriverIndexForPlayer()))
	{
		return GameState::DriverState::Get(GetDriverIndexForPlayer()).GetRacecarIndex();
	}

	return GameState::InvalidRacecar();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ClientPacketHandler::OnConnect(void)
{
	if (true == IsHandlingSafeConnection())
	{
		SendSafePacket(CreateJoinRequestPacket());
		mPingMonitor.Reset();
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ClientPacketHandler::OnDisconnect(void)
{
	if (true == IsHandlingSafeConnection())
	{
		mRegistrationCode = kInvalidRegistrationCode;
		mIsAuthenticated = false;
		mIsRegistered = false;
		mIsReadyToPlay = false;
		mPingMonitor.SetRegisteredFastConnection(false);

		for (const GameState::DriverState& driver : GameState::DriverState::AllDrivers())
		{
			if (true == driver.IsEntered())
			{
				GameState::RaceSessionState::DriverLeaveCompetition(driver.GetDriverIndex());
			}
		}

		GameState::RaceSessionState::Destroy();
		mPlayerDriverIndex = GameState::InvalidDriver();

		for (const GameState::RacecarState& racecar : GameState::RacecarState::AllRacecars())
		{
			if (true == racecar.IsRacecarInUse())
			{
				GameState::RaceSessionState::DriverLeaveRacecar(racecar.GetDriverIndex(), racecar.GetRacecarIndex());
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::ClientPacketHandler::CanHandlePacket(const tbCore::byte* packetData, size_t packetSize) const
{
	const PacketType actualPacketType = GetActualPacketType(packetData, packetSize);

	if (false == mIsAuthenticated &&
		PacketType::JoinResponse != actualPacketType &&
		PacketType::AuthenticateResponse != actualPacketType &&
		PacketType::Disconnect != actualPacketType)
	{
		return false;
	}

	if (false == GameState::RacetrackState::IsValidRacetrack())
	{
		const std::vector<PacketType> unsafePacketsWithoutRacetrack = { //aka Packets that require a valid racetrack state.
			PacketType::DriverEntersRacecar, PacketType::DriverLeavesRacecar, PacketType::RacecarReset, PacketType::RacecarRequest,
			PacketType::RacecarUpdate, PacketType::MultiCarUpdate
		};

		for (const PacketType& type : unsafePacketsWithoutRacetrack)
		{
			if (actualPacketType == type)
			{
				return false;
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::ClientPacketHandler::OnHandlePacket(const tbCore::byte* packetData, size_t packetSize, tbCore::byte fromConnection)
{
	TracePacket("Client Receiving", packetData, packetSize);

	const PacketType packetType = static_cast<PacketType>(packetData[1]);

	if (false == CanHandlePacket(packetData, packetSize))
	{
		TracePacket("Client is not handling", packetData, packetSize);
		return false;
	}

	switch (packetType)
	{
	case PacketType::Tiny: {
		const TinyPacket& tinyPacket = ToPacket<TinyPacket>(packetData, packetSize);
		const PacketType packetSubType = static_cast<PacketType>(tinyPacket.subtype);

		switch (packetSubType)
		{
		case PacketType::PingSyncReady: {
			mIsReadyToPlay = true;
			break; }

		case PacketType::AuthenticateResponse: {
			//The server will not send this if the authentication failed, instead Disconnect will be sent.
			mIsAuthenticated = true;
			mPlayerDriverIndex = tinyPacket.data;
			SendSafePacket(Network::CreateTinyPacket(PacketType::RacetrackRequest, mPlayerDriverIndex));
			break; }

		case PacketType::JoinResponse: {
			tb_always_log_if(true == theUserAccessKey.empty(), LogClient::Info() << "UserAccessKey is invalid, must authenticate.");
			const AuthenticationPacket authPacket = CreateAuthenticationRequest(theUserAccessKey, theAuthenticationService);
			SendSafePacket(authPacket, sizeof(authPacket));
			break; }
		case PacketType::NetworkSettings: {
			SetPacketsPerSecond(tinyPacket.data);
			break; }
		case PacketType::Disconnect: {
			const DisconnectReason disconnectReason = static_cast<DisconnectReason>(tinyPacket.data);
			tb_always_log(LogClient::Info() << "\tDisconnecting because " << ToString(disconnectReason));
			mPlayerDriverIndex = GameState::InvalidDriver();
			DestroyConnectionSoon(disconnectReason);
			break; }
		case PacketType::RegistrationResponse: {
			mPingMonitor.SetRegisteredFastConnection(true);
			mIsRegistered = true;

			//TODO: LudumDare56: 2023-10-21: Once a driver is registered they need to get the results/situation report.
			//SendSafePacket(CreateTinyPacket(PacketType::AutocrossResult));
			break; }

		case PacketType::DriverLeft: {
			const DriverIndex driverIndex = static_cast<DriverIndex>(tinyPacket.data);
			if (true == IsValidDriver(driverIndex))
			{	//When a driver leaves the competition they also leave the racecar if they were in one.
				GameState::RaceSessionState::DriverLeaveCompetition(driverIndex);
			}
			break; }
		case PacketType::DriverLeavesRacecar: {
			const DriverIndex driverIndex = static_cast<DriverIndex>(tinyPacket.data);
			const RacecarIndex racecarIndex = GameState::DriverState::Get(driverIndex).GetRacecarIndex();
			if (true == GameState::IsValidRacecar(racecarIndex))
			{
				GameState::RaceSessionState::DriverLeaveRacecar(driverIndex, racecarIndex);
			}
			break; }

		case PacketType::RacecarReset: {
			const RacecarIndex racecarIndex = tinyPacket.data;
			if (racecarIndex < GameState::kNumberOfRacecars)
			{
				GameState::RaceSessionState::PlaceCarOnGrid(GameState::RacecarState::GetMutable(racecarIndex));
			}
			break; }

		case PacketType::TimingReset: {
			GameState::TimingState::ResetCompetition();
			break; }

		default:
			tb_debug_log(LogClient::Warning() << "Warning: Unhandled tiny message of type: " << packetSubType);
			break;
		}
		break; }

	case PacketType::Small: {
		const SmallPacket& smallPacket = ToPacket<SmallPacket>(packetData, packetSize);
		const PacketType packetSubType = static_cast<PacketType>(smallPacket.subtype);

		switch (packetSubType)
		{
		case PacketType::RegistrationStartResponse: {
			//Send the first registration request to the GameServer over fast connection, and then continue to send the
			//  registration requests in FixedUpdate() until we actually get registered, or time-out and disconnect all.
			mRegistrationTimer = 5000;
			mRegistrationCode = smallPacket.payload;
			Network::SendFastPacket(CreateSmallPacket(PacketType::RegistrationRequest, mRegistrationCode, GetDriverIndexForPlayer()));
			break; }

		case PacketType::PhaseChanged: {
			GameState::RaceSessionState::SetSessionPhase(static_cast<GameState::RaceSessionState::SessionPhase>(smallPacket.data), smallPacket.payload);
			break; }

		case PacketType::RaceSessionTimer: {
			for (tbCore::uint32& updateTime : mLastUpdateTimes)
			{
				updateTime = smallPacket.payload;
			}
			break; }

		default:
			tb_debug_log(LogClient::Warning() << "Warning: Unhandled small message of type: " << packetSubType);
			break;
		};

		break; }

	case PacketType::LargePayload: {
		const LargePayloadPacket& payloadPacket = ToPacket<LargePayloadPacket>(packetData);

		if (true == mLargePayload.AppendData(payloadPacket))
		{
			OnHandlePacket(mLargePayload.GetPacketData(), mLargePayload.GetPacketSize(), fromConnection);
			mLargePayload.Clear();
		}
		break; }

	case PacketType::PingRequest:
	case PacketType::PingResponse: {
		const PingPacket& pingPacket = ToPacket<PingPacket>(packetData, packetSize);
		mPingMonitor.HandlePacket(pingPacket, fromConnection);
		break; }

	case PacketType::StartGrid: {
		const StartGridPacket& gridPacket = ToPacket<StartGridPacket>(packetData, packetSize);
		std::array<GameState::GridIndex, GameState::kNumberOfRacecars> startingGrid;
		for (RacecarIndex racecarIndex = 0; racecarIndex < GameState::kNumberOfRacecars; ++racecarIndex)
		{
			startingGrid[racecarIndex] = gridPacket.grid[racecarIndex];
		}

		GameState::RaceSessionState::SetStartingGrid(startingGrid);
		break; }

	case PacketType::DriverJoined: {
		const DriverJoinedPacket& driverJoined = ToPacket<DriverJoinedPacket>(packetData, packetSize);

		GameState::DriverLicense driverLicense;
		driverLicense.mIdentifier = driverJoined.license;
		driverLicense.mName = driverJoined.name;
		driverLicense.mIsModerator = driverJoined.isModerator;
		GameState::RaceSessionState::DriverEnterCompetition(driverLicense);
		break; }
	case PacketType::DriverEntersRacecar: {
		const DriverEntersRacecarPacket& packet = ToPacket<DriverEntersRacecarPacket>(packetData, packetSize);
		const tbMath::Matrix4 transform = tbMath::Matrix4::FromQuaternion(
			static_cast<tbMath::Quaternion>(packet.rotation),
			static_cast<tbMath::Vector3>(packet.position));

		GameState::RacecarState::GetMutable(packet.racecarIndex).SetVehicleToWorld(static_cast<icePhysics::Matrix4>(transform));
		GameState::RacecarState::GetMutable(packet.racecarIndex).SetRacecarMeshID(packet.carID);
		GameState::RaceSessionState::DriverEnterRacecar(packet.driverIndex, packet.racecarIndex);

		break; }

	case PacketType::RacetrackResponse: {
		const RacetrackResponsePacket& packet = ToPacket<RacetrackResponsePacket>(packetData, packetSize);
		if (true == packet.racetrack.empty())
		{
			GameState::RacetrackState::InvalidateRacetrack();
		}
		else
		{
			const tbCore::tbString racetrackFilepath = "data/racetracks/" + tbCore::tbString(packet.racetrack.c_str()) + ".trk";
			tb_debug_log(LogClient::Always() << "RacetrackResponse from GameServer, loading racetrack: \"" << packet.racetrack << "\"");

			//Note: To support changing the racetrack Rally of Rockets use to have a special case for loadingTag == 0
			//  which was used the very first call, and would then call Track::LoadRacetrack(), Session::Create() while
			//  having a non-zero loading tag would call RaceSession::RestartAndLoadRacetrack(). Ideally this could all
			//  be handled as one thing, but RestartAndLoad seems to make some assumptions; what if we called Destroy()
			//  here?
			//
			//  Not actually supporting the changing the racetrack mid-session at this particular moment, but see above.

			GameState::RaceSessionState::Create(false, racetrackFilepath);

			//Note: Because of single-threading we know the racetrack has been loaded and fully created at this point,
			//  so we can tell the server the track has been loaded and we are ready to know about the racecars.
			SendSafePacket(CreateTinyPacket(PacketType::RacetrackLoaded, packet.loadingTag));
			SendSafePacket(CreateTinyPacket(PacketType::RegistrationStartRequest));
		}
		break; }

	case PacketType::RacecarUpdate: {
		const RacecarUpdatePacket& carUpdate = ToPacket<RacecarUpdatePacket>(packetData, packetSize);

		if (carUpdate.carInfo.racecarIndex != GetRacecarIndexForPlayer() && carUpdate.time > mLastUpdateTimes[carUpdate.carInfo.racecarIndex])
		{
			mLastUpdateTimes[carUpdate.carInfo.racecarIndex] = carUpdate.time;
			HandleUpdatePacket(carUpdate.carInfo, carUpdate.time);
		}
		break; }

	//case PacketType::AutocrossUpdate: {
	//	const AutocrossUpdatePacket& autocrossUpdate = ToPacket<AutocrossUpdatePacket>(packetData, packetSize);
	//	GameState::AutocrossState::HandleAutocrossUpdatePacket(autocrossUpdate);
	//	break; }
	case PacketType::TimingResult: {
		const TimingResultPacket& timingResult = ToPacket<TimingResultPacket>(packetData, packetSize);
		const GameState::Events::TimingEvent lapResultEvent(GameState::Events::Timing::CompletedLapResult,
			timingResult.driverLicense, timingResult.driverName, timingResult.lapTime, timingResult.lapNumber);
		GameState::TimingState::AddCompletedLapResult(lapResultEvent);
		//GameState::AutocrossState::AutocrossRunFinished(autocrossResult);
		break; }

	default:
		tb_debug_log(LogClient::Warning() << "Warning: Unhandled message of type: " << packetType);
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ClientPacketHandler::OnHandleEvent(const TyreBytes::Core::Event& event)
{
	switch (event.GetID())
	{
	case GameState::Events::Racecar::DriverEntersRacecar: {
		const auto& racecarSeatEvent = event.As<GameState::Events::RacecarSeatEvent>();
		tb_error_if(false == GameState::IsValidRacecar(racecarSeatEvent.mRacecarIndex), "Expected a valid racecar for seat changing!");
		tb_debug_log(LogClient::Info() << "Setting a racecar controller to NetworkController.");
		GameState::RacecarState::GetMutable(racecarSeatEvent.mRacecarIndex).SetRacecarController(new NetworkedRacecarController(racecarSeatEvent.mRacecarIndex));
		break; }
	};
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::ServerPacketHandler::ServerPacketHandler(void) :
	LudumDare56PacketHandlerInterface(),
	mLargePayloads(),
	mSafeDriverTable(),
	mFastDriverTable(),
	mConnectorServices(),
	mConnectedClients(),
	mUnregisteredClients(),
	mBannedDrivers(),
	mNumberOfConnections(0)
{
	for (DriverIndex& driverIndex : mSafeDriverTable) { driverIndex = GameState::InvalidDriver(); }
	for (DriverIndex& driverIndex : mFastDriverTable) { driverIndex = GameState::InvalidDriver(); }

	tb_always_log(LogServer::Info() << "Server resetting all client PingMonitors.");

	DriverIndex driverIndex = 0;
	for (ConnectedClient& client : mConnectedClients)
	{
		tb_error_if(false == GameState::IsValidDriver(driverIndex), "Expected each connected client to have a valid driver index.");
		client.mDriverIndex = driverIndex;
		client.mSafeConnection = tbNetwork::InvalidClientID();
		client.mFastConnection = tbNetwork::InvalidClientID();
		client.mPingMonitor.Reset();
		client.mLastUpdateTime = 0;

		++driverIndex;
	}

	GameState::RaceSessionState::AddEventListener(*this);
	GameState::TimingState::AddEventListener(*this);

	for (GameState::RacecarState& racecar : GameState::RacecarState::AllMutableRacecars())
	{
		racecar.AddEventListener(*this);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::ServerPacketHandler::~ServerPacketHandler(void)
{
	for (GameState::RacecarState& racecar : GameState::RacecarState::AllMutableRacecars())
	{
		racecar.RemoveEventListener(*this);
	}

	GameState::TimingState::RemoveEventListener(*this);
	GameState::RaceSessionState::RemoveEventListener(*this);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ServerPacketHandler::FixedUpdate(tbCore::uint32 deltaTimeMS)
{
	for (DriverIndex driverIndex : mSafeDriverTable)
	{	//This isn't exactly, required, but is a good safety check...
		if (true == GameState::IsValidDriver(driverIndex))
		{
			tb_error_if(kInvalidConnection == GetSafeConnection(driverIndex),
				"Error: Expected all connected drivers/racecars to have a valid connection...");
		}
	}

	for (ConnectedClient& client : mConnectedClients)
	{
		if (kInvalidConnection != client.mSafeConnection)
		{
			client.mPingMonitor.Update(deltaTimeMS);
			if (client.mPingMonitor.GetTimeSinceLastPingResponse() >= Network::MaximumPingAllowed())
			{
				DisconnectClient(client.mSafeConnection, client.mFastConnection, DisconnectReason::PingTimeout);
			}
		}
	}

	//2022-04-26: We must keep the connections to kill in a seperate container because DisconnectClient() will modify
	//  the mUnregisteredClients and that must not happen while iterating over the container.
	//
	//  Note: This is necessary to have a double loop because we are changing the timer directly in the clients, and
	//  cannot modify the container while iterating through.
	std::vector<tbCore::uint8> fastConnectionsToKill;
	for (UnregisteredClient& client : mUnregisteredClients)
	{
		client.mRegistrationTimer += deltaTimeMS;
		if (client.mRegistrationTimer > Network::MaximumPingAllowed())
		{
			fastConnectionsToKill.push_back(client.mFastConnection);
		}
	}

	for (tbCore::uint8 fastConnection : fastConnectionsToKill)
	{
		DisconnectClient(kInvalidConnection, fastConnection, DisconnectReason::UnregisteredTimeout);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::uint32 LudumDare56::Network::ServerPacketHandler::FindWorstCaseLatency(const ConnectionType connectionType) const
{
	tbCore::uint32 numberOfConnections = 0;
	tbCore::uint32 worstLatency = 0;

	for (const ConnectedClient& client : mConnectedClients)
	{
		const tbCore::uint8 connectionIndex = (ConnectionType::Safe == connectionType) ?
			static_cast<tbCore::uint8>(client.mSafeConnection) : static_cast<tbCore::uint8>(client.mFastConnection);

		if (tbNetwork::InvalidClientID() != connectionIndex)
		{
			const tbCore::uint32 syncedLatency = client.mPingMonitor.GetSyncedLatency(16, connectionType);
			if (InvalidLatency() == syncedLatency)
			{
				return InvalidLatency();
			}
			else if (syncedLatency > worstLatency)
			{
				worstLatency = syncedLatency;
			}

			++numberOfConnections;
		}
	}

	if (0 == numberOfConnections)
	{
		return InvalidLatency();
	}

	return worstLatency;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ServerPacketHandler::BanDriver(const DriverIndex driverIndex)
{
	mBannedDrivers.push_back(GameState::DriverState::Get(driverIndex).GetLicense());
	DisconnectDriver(driverIndex, DisconnectReason::Banned);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ServerPacketHandler::OnConnect(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ServerPacketHandler::OnDisconnect(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ServerPacketHandler::OnConnectClient(tbCore::byte connection)
{
	tb_debug_log(LogServer::Info() << "New " << ((IsHandlingSafeConnection()) ? "SAFE" : "FAST") << " client connecting with id( "
		<< static_cast<int>(connection) << " ) and port " << Implementation::theSafeConnection->UnstableApi_GetClientPort(connection));

	if (true == IsHandlingSafeConnection())
	{
		if (0 == mNumberOfConnections)
		{
			tb_always_log_if(GameState::RaceSessionState::SessionPhase::kPhaseWaiting != GameState::RaceSessionState::GetSessionPhase(),
				LogServer::Error() << "Expected the GameServer to be in the waiting state when there are no connections.");

			GameState::RaceSessionState::SetSessionPhase(GameState::RaceSessionState::SessionPhase::kPhasePractice);
		}

		++mNumberOfConnections;
		tb_always_log(LogServer::Info() << "Number of connections: " << mNumberOfConnections);
	}
	else
	{
		AddUnregisteredClient(connection);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ServerPacketHandler::OnDisconnectClient(tbCore::byte connection)
{
	if (true == IsHandlingSafeConnection())
	{
		const DriverIndex driverIndex = GetDriverIndexFromSafeConnection(connection);
		if (true == IsValidDriver(driverIndex))
		{
			const FastConnection fastConnection = GetFastConnection(driverIndex);
			DisconnectClient(kInvalidConnection, fastConnection, DisconnectReason::Graceful);

			const GameState::DriverState& driver = GameState::DriverState::Get(driverIndex);
			tb_debug_log(LogServer::Info() << "Disconnecting " << DebugInfo(driverIndex));

			const RacecarIndex racecarIndex = driver.GetRacecarIndex();
			ConnectedClient& client = mConnectedClients[driverIndex];

			//This must happen before clearing out the client information.
			mSafeDriverTable[client.mSafeConnection] = GameState::InvalidDriver();
			mFastDriverTable[client.mFastConnection] = GameState::InvalidDriver();

			client.mPingMonitor.Reset();
			client.mPingMonitor.SetSafeConnection(kInvalidConnection);
			client.mPingMonitor.SetFastConnection(kInvalidConnection);
			client.mSafeConnection = kInvalidConnection;
			client.mFastConnection = kInvalidConnection;
			client.mRegistrationCode = kInvalidRegistrationCode;
			client.mLastUpdateTime = 0;

			if (true == IsValidRacecar(racecarIndex))
			{
				GameState::RaceSessionState::DriverLeaveRacecar(driverIndex, racecarIndex);
			}

			GameState::RaceSessionState::DriverLeaveCompetition(driverIndex);

			SendSafePacket(CreateTinyPacket(PacketType::DriverLeft, driverIndex));
		}

		--mNumberOfConnections;
		tb_always_log(LogServer::Info() << "Number of connections: " << mNumberOfConnections);

		if (0 == mNumberOfConnections)
		{
			GameState::RaceSessionState::SetSessionPhase(GameState::RaceSessionState::SessionPhase::kPhaseWaiting);
		}
	}
	else
	{
		RemoveUnregisteredClient(connection);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

/// @details To run multiple copies from the same machine for testing, while having identifyable names, if the developer
///   name is already being used, grab a random name from a hat.
tbCore::tbString GetDeveloperOrRandomName(const tbCore::tbString& developerName)
{
	using namespace LudumDare56::GameState;

	bool isNameUsed = false;
	for (const DriverState& driver : DriverState::AllDrivers())
	{
		if (developerName == driver.GetName())
		{
			isNameUsed = true;
			break;
		}
	}

	if (false == isNameUsed)
	{
		return developerName;
	}

	static std::vector<tbCore::tbString> theAvailableRacerNames;
	if (true == theAvailableRacerNames.empty())
	{
		theAvailableRacerNames = {
			"S-Car Go", "Lollipop", "Diesel", "Honker", "Swifty",
			"Spunk", "T-Bone", "Rubber", "Hammer", "Reflex",
			"Lightspeed", "Tank", "Shakey", "Slingshot", "Apex",
			"Wings", "Nitro", "Turbo", "Chaser", "Lightning",
			"Runner", "Comet", "Zoomer", "Bunny", "Speedy"
		};
	}

	const tbCore::tbString racerName = theAvailableRacerNames.back();
	theAvailableRacerNames.pop_back();
	return racerName;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::ServerPacketHandler::OnHandlePacket(const tbCore::byte* packetData, size_t packetSize, tbCore::byte fromConnection)
{
	TracePacket("Receiving", packetData, packetSize, "from " + tbCore::ToString(static_cast<int>(fromConnection)));

	//TODO: LudumDare56: 2022-04-19: Check to ensure the packet should be handled, do we have a packet that is
	//  establishing the connection or do we know the fromConnection is who we think it is?

	const PacketType packetType = static_cast<PacketType>(packetData[1]);

	switch (packetType)
	{
	case PacketType::Tiny: {
		const TinyPacket& tinyPacket = ToPacket<TinyPacket>(packetData, packetSize);
		const PacketType packetSubType = static_cast<PacketType>(tinyPacket.subtype);

		switch (packetSubType)
		{
		case PacketType::Disconnect: {
			if (true == IsHandlingSafeConnection())
			{
				//Note: I don't suspect this will happen, but there may be a slight chance of a race condition where two
				//  disconnect packets arrive, one on FastConnection and the other on SafeConnection and they get handled
				//  in an order that would have had the driver already disconnected.
				const DriverIndex driverIndex = GetDriverIndexFromSafeConnection(fromConnection);
				tb_error_if(false == GameState::IsValidDriver(driverIndex), "Error: The unexpected happened, disconnect race condition.");
				DisconnectClient(fromConnection, GetFastConnection(driverIndex), DisconnectReason::Graceful);
			}
			else
			{
				bool disconnectedRegisteredClient = false;
				for (ConnectedClient& client : mConnectedClients)
				{
					if (client.mFastConnection == fromConnection)
					{
						disconnectedRegisteredClient = true;
						DisconnectClient(client.mSafeConnection, client.mFastConnection, DisconnectReason::Graceful);
					}
				}

				if (false == disconnectedRegisteredClient)
				{
					DisconnectClient(kInvalidConnection, fromConnection, DisconnectReason::Graceful);
				}
			}
			break; }

		case PacketType::RegistrationStartRequest: {
			if (true == IsHandlingSafeConnection())
			{
				const DriverIndex driverIndex = GetDriverIndexFromSafeConnection(fromConnection);
				const ConnectedClient& client = mConnectedClients[driverIndex];
				SendSafePacketTo(CreateSmallPacket(PacketType::RegistrationStartResponse,
					client.mRegistrationCode, driverIndex), fromConnection);
			}
			else
			{	//TODO: LudumDare56: Network: Will need to figure out how to disconnect the client in a much better way.
				DisconnectClient(kInvalidConnection, fromConnection, DisconnectReason::ConnectionMismatch);
			}
			break; }

		case PacketType::RacetrackRequest: {
			if (true == IsHandlingSafeConnection())
			{
				const DriverIndex driverIndex = GetDriverIndexFromSafeConnection(fromConnection);
				if (driverIndex == tinyPacket.data)
				{
					SendSafePacketTo(CreateRacetrackResponse(0), fromConnection);
				}
				else
				{
					DisconnectClient(fromConnection, GetFastConnection(driverIndex), DisconnectReason::InvalidInformation);
				}
			}
			else
			{	//TODO: LudumDare56: Network: Will need to figure out how to disconnect the client in a much better way.
				DisconnectClient(kInvalidConnection, fromConnection, DisconnectReason::ConnectionMismatch);
			}
			break; }
		case PacketType::RacetrackLoaded: {
			if (true == IsHandlingSafeConnection())
			{
				for (const GameState::RacecarState& racecar : GameState::RacecarState::AllRacecars())
				{
					if (true == IsValidDriver(racecar.GetDriverIndex()))
					{
						SendSafePacketTo(CreateDriverEntersRacecarPacket(racecar), fromConnection);
					}
				}
			}
			else
			{	//TODO: LudumDare56: Network: Will need to figure out how to disconnect the client in a much better way.
				DisconnectClient(kInvalidConnection, fromConnection, DisconnectReason::ConnectionMismatch);
			}
			break; }

		case PacketType::DriverLeavesRacecar: {
			if (true == IsHandlingSafeConnection())
			{
				const DriverIndex driverIndex = GetDriverIndexFromSafeConnection(fromConnection);
				const DriverIndex claimedDriverIndex = static_cast<DriverIndex>(tinyPacket.data);
				if (driverIndex == claimedDriverIndex)
				{
					const RacecarIndex racecarIndex = GameState::DriverState::Get(driverIndex).GetRacecarIndex();
					if (true == GameState::IsValidRacecar(racecarIndex))
					{
						GameState::RaceSessionState::DriverLeaveRacecar(driverIndex, racecarIndex);
						SendSafePacket(CreateTinyPacket(PacketType::DriverLeavesRacecar, driverIndex));
					}
				}
				else
				{
					DisconnectClient(kInvalidConnection, fromConnection, DisconnectReason::InvalidInformation);
				}
			}
			else
			{	//TODO: LudumDare56: Network: Will need to figure out how to disconnect the client in a much better way.
				DisconnectClient(kInvalidConnection, fromConnection, DisconnectReason::ConnectionMismatch);
			}
			break; }

		case PacketType::RacecarReset: {
			const DriverIndex driverIndex = GetDriverIndexFromSafeConnection(fromConnection);
			const RacecarIndex racecarIndex = GameState::DriverState::Get(driverIndex).GetRacecarIndex();
			if (racecarIndex == tinyPacket.data && racecarIndex < GameState::kNumberOfRacecars)
			{
				GameState::RaceSessionState::PlaceCarOnGrid(GameState::RacecarState::GetMutable(racecarIndex));

				const TinyPacket resetPacket = CreateTinyPacket(PacketType::RacecarReset, racecarIndex);
				SendSafePacket(resetPacket);
			}

			break; }

		//case PacketType::AutocrossResult: {
		//	if (true == IsHandlingSafeConnection())
		//	{
		//		GameState::AutocrossState::SendCompetitionResultsTo(fromConnection);
		//	}
		//	else
		//	{	// TODO: LudumDare56: Network: Will need to figure out how to disconnect the client in a much better way.
		//		//   Results shouldn't be sent from a fast connection.
		//		DisconnectClient(kInvalidConnection, fromConnection, DisconnectReason::ConnectionMismatch);
		//	}
		//	break; }

		default:
			tb_debug_log(LogServer::Warning() << "Warning: Unhandled tiny message of type: " << packetSubType);
			break;
		};
		break; }

	case PacketType::Small: {
		const SmallPacket& smallPacket = ToPacket<SmallPacket>(packetData, packetSize);
		const PacketType packetSubType = static_cast<PacketType>(smallPacket.subtype);

		switch (packetSubType)
		{
		case PacketType::RegistrationRequest: {	//This packet is sent over the unsafe connection.
			if (true == IsHandlingSafeConnection())
			{
				DisconnectClient(fromConnection, kInvalidConnection, DisconnectReason::ConnectionMismatch);
				break;
			}

			const DriverIndex claimedDriverIndex = smallPacket.data;
			const tbCore::uint32 claimedRegistrationCode = smallPacket.payload;
			bool wasConnectionRegistered = false;

			int connectionState = 0;
			if (0 == connectionState && claimedDriverIndex >= mConnectedClients.size()) { connectionState = 1; }
			if (0 == connectionState && mConnectedClients[claimedDriverIndex].mRegistrationCode != claimedRegistrationCode) { connectionState = 2; }
			if (0 == connectionState && mConnectedClients[claimedDriverIndex].mFastConnection != tbNetwork::InvalidClientID())
			{
				connectionState = 3;

				if (mConnectedClients[claimedDriverIndex].mFastConnection == fromConnection)
				{	//Another packet might arrive after we already registered, so if this connection is already set as
					//  the fastConnection for the connected client, then we safely ignore it.
					wasConnectionRegistered = true;
				}
			}

			if (0 == connectionState)
			{
				ConnectedClient& client = mConnectedClients[claimedDriverIndex]; //At this point we are good to go.
				client.mFastConnection = fromConnection;
				mFastDriverTable[fromConnection] = client.mDriverIndex;

				//2022-04-20: It seemed this potentially not needed, but if there are any issues with ping monitoring
				//  of the Fast/UDP connection this is the place to begin, uncomment and ensure that they are set for
				//  Slow/TCP connection as well. It is possible the server broadcasting to all just works...
				client.mPingMonitor.SetRegisteredFastConnection(true);
				client.mPingMonitor.SetFastConnection(fromConnection);

				RemoveUnregisteredClient(fromConnection);

				SendSafePacketTo(CreateTinyPacket(PacketType::RegistrationResponse, 0), client.mSafeConnection);
				wasConnectionRegistered = true;
			}

			if (false == wasConnectionRegistered)
			{	//This is an unregistered FastConnection, which we can disconnect from, but it won't disconnect the
				//  end client entirely until their FastConnection times out.
				tb_debug_log(LogServer::Warning() << "Registration for fastConnection( " << static_cast<int>(fromConnection) << " ) failed state: " << connectionState);
				DisconnectClient(kInvalidConnection, fromConnection, DisconnectReason::InvalidInformation);
			}
			break; }

		default:
			tb_debug_log(LogServer::Warning() << "Warning: Unhandled small message of type: " << packetSubType);
			break;
		};

		break; }

	case PacketType::LargePayload: {
		const LargePayloadPacket& payloadPacket = ToPacket<LargePayloadPacket>(packetData);
		LargePayloadHandler& payloadHandler = mLargePayloads[fromConnection];
		if (true == payloadHandler.AppendData(payloadPacket))
		{
			OnHandlePacket(payloadHandler.GetPacketData(), payloadHandler.GetPacketSize(), fromConnection);
			payloadHandler.Clear();
		}

		break; }

	case PacketType::PingRequest:
	case PacketType::PingResponse: {
		// TODO: LudumDare56: UpgradeToUDP: 2022-08-21: The following comment came from Rally of Rockets and seems to
		//   indicate a need to hand UDP / FastConnections pings differently. Since LudumDare56 is still using only
		//   the SafeConnection (TCP) this can't be tested yet, but I am 99% certain it will just work as is. If not
		//   we can lean on the IsHandlingSafePacket() to know Safe or Fast connection. Another comment in the code was
		//   indicating we need to ensure client is truthful (about their driverIndex) but that probably doesn't apply
		//   anymore since PingPackets do not contain a driver index and we can know who it came from by the connection.
		//
		//   UpgradeToUDP: 2022-04-19: This is still assuming only a single connection exists and
		//      we need to separate the Safe vs Fast pings to the correct PingMonitor... After refactoring come back to this.
		const PingPacket& pingPacket = ToPacket<PingPacket>(packetData, packetSize);

		const DriverIndex driverIndex = (true == IsHandlingSafeConnection()) ? GetDriverIndexFromSafeConnection(fromConnection) :
			GetDriverIndexFromFastConnection(fromConnection);

		if (true == IsValidDriver(driverIndex))
		{
			const bool wasSyncFinished = mConnectedClients[driverIndex].mPingMonitor.IsSyncedLatencyReady();
			mConnectedClients[driverIndex].mPingMonitor.HandlePacket(pingPacket, fromConnection);

			if (true == IsHandlingSafeConnection() && false == wasSyncFinished &&
				mConnectedClients[driverIndex].mPingMonitor.IsSyncedLatencyReady())
			{
				tb_always_log(LogServer::Debug() << "GameServer is sending a PingReady to " << DebugInfo(driverIndex));
				SendSafePacketTo(CreateTinyPacket(PacketType::PingSyncReady, 0), fromConnection);
			}
		}

		break; }


	case PacketType::JoinRequest: {
		const JoinRequestPacket& packet = ToPacket<JoinRequestPacket>(packetData, packetSize);
		if (Version::Major() != packet.major || Version::Minor() != packet.minor || Version::Patch() != packet.patch ||
			PacketVersion() != packet.packetVersion)
		{
			DisconnectClient(fromConnection, kInvalidConnection, DisconnectReason::VersionMismatch);
		}
		else if (false /* == Available Slot on Server */)
		{
			DisconnectClient(fromConnection, kInvalidConnection, DisconnectReason::ServerFull);
		}
		else
		{
			const TinyPacket joinPacket = CreateTinyPacket(PacketType::JoinResponse);
			SendSafePacketTo(joinPacket, fromConnection);
		}

		break; }

	case PacketType::AuthenticateRequest: {
		const AuthenticationPacket& authenticatePacket = ToPacket<AuthenticationPacket>(packetData);
		tb_always_log(LogAuth::Info() << "Authentication using a " << ToString(authenticatePacket.service) << " userKey.");
		tb_debug_log(LogSecret::Debug() << "Authenticating User Key: " << authenticatePacket.userKey);

		TyreBytes::Core::Services::ConnectorServiceInterface* connectorService = nullptr;

		// Note: This is perhaps very unlikely, but, if a connection (X) requests authentication, then disconnects for any
		//   reason; and another client connects quite immediately after- it could replace the connection (X') before such
		//   time that the WebServer verified the userAccessKey then the fromConnection here may respond to X' as the
		//   initial connection is gone... Good luck!   (This is because the Twitch/Patreon verification is async and
		//   another network call).
		//
		// Note: It might not be that unlikely... If the client disconnects while the GameServer is stuck authenticating
		//   the user, then once the callback is reached the server will spam the following message:
		//
		//   tbInternal: tbNetwork Warning: Attempting to send packet to client where connection does not exist.
		//
		// And the server gets stuck in that state; it appears this happens when TurtleBrains has disconnected / killed
		//   a connection, and almost certainly already called DisconnectClient() on our handler, but then this delayed
		//   callback to authenticate overwrites the connected client as if it were connected and we begin trying to
		//   send data.  (To reproduce; kill the client immediately after the server sends DriverJoined or receives
		//   AuthenticateRequest; it is a race-condition and works better with Twitch/Patreon services to allow time.
		//
		//   This message "Server: SendPacketTo() is broadcasting to all clients, use SendPacket() instead?" hints that
		//   we might be able to figure it out!
		switch (authenticatePacket.service)
		{
		case AuthenticationService::Developer: {
			const tbCore::tbString expectedKey = TyreBytes::Core::Utilities::LoadFileContentsToString("developer_key", true);
			const bool isVerified = tbCore::StringContains(authenticatePacket.userKey, expectedKey);
			GameState::DriverLicense driverLicense;

			const tbCore::tbString developerName = tbCore::tbString(authenticatePacket.userKey).substr(expectedKey.size());
			driverLicense.mIdentifier = "DEVKEY" + developerName;
			driverLicense.mName = GetDeveloperOrRandomName(developerName);
			driverLicense.mIsModerator = (developerName == driverLicense.mName);
			OnAuthenticateConnection(fromConnection, isVerified, driverLicense);
			return true;
			break; }
		case AuthenticationService::Twitch: {
			connectorService = new TyreBytes::Core::Services::TwitchConnectorService(LudumDare56::GetTwitchClientID(), "");
			break; }
		case AuthenticationService::Patreon: {
			connectorService = new TyreBytes::Core::Services::PatreonConnectorService(LudumDare56::GetPatreonClientID(), "");
			break; }
		case AuthenticationService::YouTube: {
			connectorService = new TyreBytes::Core::Services::YouTubeConnectorService(LudumDare56::GetYouTubeClientID(), "");
			break; }

		case AuthenticationService::Unknown: {
			break; }
		}

		if (nullptr != connectorService)
		{
			mConnectorServices.push_back(connectorService);
			mConnectorServices.back()->GameServerVerifyUserAccessKey(authenticatePacket.userKey,
				[this,  connectorService, authenticatePacket, fromConnection](TyreBytes::Core::Services::AuthenticationResult result) {
				if (true == result.mIsVerified)
				{	//What if the client disconnected for any reason, see above notes. fromConnection may be bad.
					GameState::DriverLicense driverLicense;

					const tbCore::tbString serviceName = ToString(authenticatePacket.service);
					driverLicense.mIdentifier = result.mUserID + "@" + serviceName;
					driverLicense.mName = result.mDisplayName;
					driverLicense.mIsModerator = (AuthenticationService::Twitch == authenticatePacket.service && "30693918" == result.mUserID); //TimBeaudet on Twitch
					OnAuthenticateConnection(fromConnection, true, driverLicense);
				}
				else
				{
					tb_always_log(LogAuth::Error() << "Failed to authenticate connection id( " << +fromConnection << " ).");
					DisconnectClient(fromConnection, kInvalidConnection, DisconnectReason::InvalidInformation);
				}

				mConnectorServices.remove(connectorService);
				delete connectorService;
			});
		}
		else
		{
			tb_always_log(LogAuth::Error() << "Failed to authenticate connection id( " << +fromConnection << " ).");
			DisconnectClient(fromConnection, kInvalidConnection, DisconnectReason::InvalidInformation);
		}

		break; }

	case PacketType::RacecarRequest: {
		const RacecarRequestPacket& packet = ToPacket<RacecarRequestPacket>(packetData, packetSize);
		if (true == IsHandlingSafeConnection() && GetDriverIndexFromSafeConnection(fromConnection) == packet.driverIndex)
		{
			const GameState::DriverState& driver = GameState::DriverState::Get(packet.driverIndex);
			if (false == IsValidRacecar(driver.GetRacecarIndex()))
			{
				const RacecarIndex racecarIndex = GameState::RaceSessionState::DriverEnterRacecar(packet.driverIndex);
				if (true == IsValidRacecar(driver.GetRacecarIndex()))
				{
					//TODO: LudumDare56: 2023-10-27: The GameServer should verify that the driver actually has access
					//  to the racecar they are requesting. If not, set the generic racecar.
					GameState::RacecarState::GetMutable(racecarIndex).SetRacecarMeshID(packet.carID);

					SendSafePacket(CreateDriverEntersRacecarPacket(GameState::RacecarState::Get(racecarIndex)));
				}
			}
		}
		break; }
	case PacketType::RacecarUpdate: {
		// TODO: LudumDare56: Network: This is currently trusting the packet for the racecarIndex. We need to grab the
		//   driverIndex from the connection and verify they are using the racecar they are telling us about.

		// TODO: LudumDare56: Cleanup: It might actually be better to remove any racecarIndex/driverIndex from all
		//   packets that go from client to GameServer since the GameServer already knows this information based on the
		//   connection;  Note: any packets from GameServer to the clients will still need this information!
		const RacecarUpdatePacket& carUpdate = ToPacket<RacecarUpdatePacket>(packetData, packetSize);
		if (true == GameState::IsValidRacecar(carUpdate.carInfo.racecarIndex))
		{
			const DriverIndex driverIndex = GameState::RacecarState::Get(carUpdate.carInfo.racecarIndex).GetDriverIndex();
			if (true == GameState::IsValidDriver(driverIndex) && carUpdate.time > mConnectedClients[driverIndex].mLastUpdateTime)
			{
				mConnectedClients[driverIndex].mLastUpdateTime = carUpdate.time;
				HandleUpdatePacket(carUpdate.carInfo, carUpdate.time);
			}
		}
		break; }

	//case PacketType::DeveloperCommand: {
	//	const DriverIndex driverIndex = GetDriverIndexFromSafeConnection(fromConnection);
	//	const DeveloperCommandPacket& developerPacket = ToPacket<DeveloperCommandPacket>(packetData, packetSize);

	//	const tbCore::tbString& driverName = GameState::DriverState::Get(driverIndex).GetDriverName();
	//	if ("TimBeaudet" == driverName)
	//	{
	//		tb_debug_log(LogServer::Info() << "Executing command for admin/mod: " << developerPacket.command.c_str());
	//		tbDevelopment::TheCommandManager().ExecuteCommand(developerPacket.command.c_str());
	//	}
	//	else
	//	{
	//		tb_debug_log(LogServer::Info() << driverName <<  " sent command: " << developerPacket.command.c_str());
	//	}
	//	break; }

	default:
		tb_debug_log(LogServer::Warning() << "Warning: Unhandled message of type: " << packetType);
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ServerPacketHandler::OnHandleEvent(const TyreBytes::Core::Event& event)
{
	switch (event.GetID())
	{
	case GameState::Events::RaceSession::RaceSessionPhaseChanged: {
		const auto& phaseChangeEvent = event.As<GameState::Events::RaceSessionPhaseChangeEvent>();

		switch (phaseChangeEvent.mSessionPhase)
		{
		case GameState::RaceSessionState::SessionPhase::kPhaseGrid: {
			if (0 == phaseChangeEvent.mPhaseTimer)
			{
				const tbCore::uint32 worstLatency = FindWorstCaseLatency(ConnectionType::Safe);
				tb_error_if(Network::InvalidLatency() == worstLatency, "This was unexpected. worstLatency is Invalid...");
				GameState::RaceSessionState::SetSessionPhase(GameState::RaceSessionState::SessionPhase::kPhaseGrid, worstLatency + 250);
			}
			else
			{
				const Network::byte phase = static_cast<Network::byte>(phaseChangeEvent.mSessionPhase);
				SendSafePacket(CreateSmallPacket(PacketType::PhaseChanged, phaseChangeEvent.mPhaseTimer, phase));
			}
			break; }

		case GameState::RaceSessionState::SessionPhase::kPhaseRacing: {
			// @note 2023-11-03: This might wish to be a more generic thing the GameServer detects from GameState. Whenever
			//   the WorldTimer is less than an older value everything needs to be reset like the following. It may happen
			//   in other states or specific situations.
			SendSafePacket(CreateSmallPacket(PacketType::RaceSessionTimer, GameState::RaceSessionState::GetWorldTimer(), 0));

			for (ConnectedClient& client : mConnectedClients)
			{
				client.mLastUpdateTime = GameState::RaceSessionState::GetWorldTimer();
			}

			break; }
		default: {
			break; }
		};

		break; }
	case GameState::Events::RaceSession::StartGridChanged: {
		StartGridPacket gridPacket;
		gridPacket.size = sizeof(StartGridPacket);
		gridPacket.type = PacketType::StartGrid;
		for (RacecarIndex racecarIndex = 0; racecarIndex < GameState::kNumberOfRacecars; ++racecarIndex)
		{
			gridPacket.grid[racecarIndex] = GameState::RaceSessionState::GetGridIndexFor(racecarIndex);
		}

		SendSafePacket(gridPacket);

		break; }

	case GameState::Events::Timing::ResetTimingResults: {
		tb_always_log(LogServer::Info() << "Timing and Scoring Reset Competition!");
		SendSafePacket(CreateTinyPacket(PacketType::TimingReset));
		break; }
	case GameState::Events::Timing::CompletedLapResult: {
		const auto& lapResultEvent = event.As<GameState::Events::TimingEvent>();
		SendSafePacket(CreateTimingResult(lapResultEvent));
		break; }

	case GameState::Events::Racecar::DriverEntersRacecar: {
		const auto& racecarSeatEvent = event.As<GameState::Events::RacecarSeatEvent>();
		tb_error_if(false == GameState::IsValidRacecar(racecarSeatEvent.mRacecarIndex), "Expected a valid racecar for seat changing!");
		tb_error_if(false == GameState::IsValidDriver(racecarSeatEvent.mDriverIndex), "Expected a valid driver for seat changing!");
		GameState::RacecarState::GetMutable(racecarSeatEvent.mRacecarIndex).SetRacecarController(new NetworkedRacecarController(racecarSeatEvent.mRacecarIndex));

		tb_always_log(LogServer::Info() << "Racecar seat change for: " << DebugInfo(racecarSeatEvent.mRacecarIndex) <<
			" is now driven by: " << DebugInfo(racecarSeatEvent.mDriverIndex));

		break; }
	};
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ServerPacketHandler::OnAuthenticateConnection(const SafeConnection safeConnection,
	bool isAuthenticated, const GameState::DriverLicense& driverLicense)
{
	for (const tbCore::tbString& bannedLicense : mBannedDrivers)
	{
		if (driverLicense.mIdentifier == bannedLicense)
		{
			isAuthenticated = false;
			break;
		}
	}

	if (true == isAuthenticated)
	{
		const DriverIndex driverIndex = GameState::RaceSessionState::DriverEnterCompetition(driverLicense);
		tb_debug_log(LogServer::Debug() << DebugInfo(driverIndex) << " is authenticated with GameServer and now entering competition.");

		if (false == IsValidDriver(driverIndex))
		{
			tb_always_log(LogServer::Always() << "There are no open spots for " << driverLicense.mName << " to join.");
			DisconnectClient(safeConnection, kInvalidConnection, DisconnectReason::ServerFull);
		}
		else
		{
			ValidateDriverIndex(safeConnection, driverIndex);

			//Tell the new driver they got authenticated and are connected to the server.
			SendSafePacketTo(CreateTinyPacket(PacketType::AuthenticateResponse, driverIndex), safeConnection);
			SendSafePacketTo(CreateTinyPacket(PacketType::NetworkSettings, GetPacketsPerSecond()), safeConnection);

			//TODO: LudumDare56: Cleanup: Network could probably remove the phase information, at least this is sent
			//  during a RacetrackResponse and doesn't need to be here; The phases were once used for Lobby vs Rounds
			//  in Rally of Rockets and that isn't really a thing in Trailing Brakes Racing Simulator, although the
			//  GameServer probably wants a Waiting vs OpenTrack type thing?
			//
			//  2022-08-26: Lobby was removed from the phases, but overall they still exist.
			{	//Tell the new connection what state the server is current in, and for how long.
				const tbCore::uint32 phaseTimer = GameState::RaceSessionState::GetPhaseTimer();
				const GameState::RaceSessionState::SessionPhase phase = GameState::RaceSessionState::GetSessionPhase();
				SmallPacket phasePacket = CreateSmallPacket(PacketType::PhaseChanged, phaseTimer, static_cast<byte>(phase));
				SendSafePacketTo(phasePacket, safeConnection);
			}

			for (const GameState::DriverState& otherDriver : GameState::DriverState::AllDrivers())
			{
				const DriverIndex otherDriverIndex = otherDriver.GetDriverIndex();
				if (true == otherDriver.IsEntered() && driverIndex != otherDriverIndex)
				{
					SendSafePacketTo(CreateDriverJoinedPacket(otherDriverIndex), safeConnection);
				}
			}

			SendSafePacket(CreateDriverJoinedPacket(driverIndex));
		}
	}
	else
	{
		tb_debug_log(LogServer::Warning() << "Authentication failed for safeConnection( " << static_cast<int>(safeConnection) << " )");
		DisconnectClient(safeConnection, kInvalidConnection, DisconnectReason::InvalidInformation);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::uint32 LudumDare56::Network::ServerPacketHandler::CreateRegistrationCode(void) const
{
	bool codeExisted = false;
	tbCore::uint32 registrationCode = 0;
	do
	{
		registrationCode = tbMath::RandomInt();

		codeExisted = false;
		for (const ConnectedClient& client : mConnectedClients)
		{
			if (registrationCode == client.mRegistrationCode)
			{
				codeExisted = true;
				break;
			}
		}
	} while (kInvalidRegistrationCode != registrationCode && true == codeExisted);

	return registrationCode;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ServerPacketHandler::AddUnregisteredClient(const FastConnection fastConnection)
{
	UnregisteredClient newClient;
	newClient.mRegistrationTimer = 0;
	newClient.mFastConnection = fastConnection;

	mUnregisteredClients.push_back(newClient);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ServerPacketHandler::RemoveUnregisteredClient(const FastConnection fastConnection)
{
	tb_debug_log(LogServer::Info() << "Removing an unregistered client from fast connection.");
	for (size_t clientIndex = 0; clientIndex < mUnregisteredClients.size(); ++clientIndex)
	{
		if (fastConnection == mUnregisteredClients[clientIndex].mFastConnection)
		{
			mUnregisteredClients[clientIndex] = mUnregisteredClients.back();
			mUnregisteredClients.pop_back();
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::ServerPacketHandler::ValidateDriverIndex(const SafeConnection safeConnection, const DriverIndex driverIndex)
{
	mSafeDriverTable[safeConnection] = driverIndex;

	ConnectedClient& client = mConnectedClients[driverIndex];
	client.mPingMonitor.SetSafeConnection(safeConnection);
	client.mPingMonitor.ReceivedKeepAlive();
	client.mRegistrationCode = CreateRegistrationCode();
	client.mSafeConnection = safeConnection;

	tb_always_log(LogServer::Info() << "Validating " << DebugInfo(driverIndex) << " with SafeConnection: " << +safeConnection);
}

//--------------------------------------------------------------------------------------------------------------------//
