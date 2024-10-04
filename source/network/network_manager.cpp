///
/// @file
/// @details Manages the connections, handlers and other network things for LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../network/network_manager.hpp"
#include "../network/network_handlers.hpp"
#include "../network/network_packets.hpp"
#include "../game_state/racecar_state.hpp"
#include "../game_server/game_server.hpp"
#include "../logging.hpp"

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_defines.hpp>
#include <turtle_brains/network/tb_socket_connection.hpp>

#include <map>

namespace LudumDare56
{
	namespace Network
	{
		namespace Implementation
		{
			//All timers are in milliseconds, it is assumed that Simulate() is called at 10ms steps.
			const tbCore::uint32 kStepTime = 10;
			const tbCore::uint32 kMaximumTimeout = 5000;

			tbCore::uint8 theUpdatePacketsPerSecond = 5;
			tbCore::uint32 theMaximiumTimeToSendUpdate = 200;

			//2022-04-26: These handlers allow the sharing of a single handler instance while knowing whether the incoming
			//  packet was from the safe or fast connection. This was initially added because OnDisconnectClient() required the
			//  information; that or split the server handler into two instances for safe/fast connections which was not ideal.
			tbNetwork::PacketHandlerInterface* theSafePacketHandler = nullptr;
			tbNetwork::PacketHandlerInterface* theFastPacketHandler = nullptr;

			Network::LudumDare56PacketHandlerInterface* thePacketHandler = nullptr;
			tbNetwork::SocketConnection* theSafeConnection = nullptr;
			tbNetwork::SocketConnection* theFastConnection = nullptr;

			bool theConnectionIsServer = false;
			tbCore::uint32 theConnectingTimer = 0;
			tbCore::uint32 theSendUpdateTimer = 0;

			bool theConnectionNeedsToBeDestroyed = false;
			DisconnectReason theReasonToDestoyTheConnection = DisconnectReason::Graceful;
			void SendUpdatePackets(void);

			std::vector<float> theSafeConnectionLatency;
			std::vector<float> theFastConnectionLatency;

			void SendLargePayload(PacketType packetType, const tbCore::byte* packetData, const size_t packetSize, bool setFirstBytes = false);
			void SendLargePayloadTo(PacketType packetType, const tbCore::byte* packetData, const size_t packetSize,
				const SafeConnection toConnection, bool setFirstBytes = false);
		};
	};
};

using namespace LudumDare56::Network::Implementation;

//--------------------------------------------------------------------------------------------------------------------//

namespace
{
	void PushBackOrShiftForward(std::vector<float>& container, float value, size_t maximumSize = 100)
	{
		if (container.size() < maximumSize)
		{
			container.push_back(value);
		}
		else
		{
			for (size_t index = 0; index < container.size() - 1; ++index)
			{
				container[index] = container[index + 1];
			}
			container.back() = value;
		}
	}
};

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::IsConnected(void)
{
	return (nullptr != theSafeConnection && true == theSafeConnection->IsConnected() && false == theConnectionNeedsToBeDestroyed);
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::IsRegistered(void)
{
	return (true == IsConnected() && false == IsServerConnection() && true == GetClientHandler().IsRegistered());
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::IsReadyToPlay(void)
{
	return (true == IsConnected() && false == IsServerConnection() && true == GetClientHandler().IsReadyToPlay());
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::HasReceivedData(void)
{
	return (theSafeConnection->GetTotalBytesReceived() > 0);
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::IsAttemptingToConnect(void)
{
	return (nullptr != theSafeConnection && theConnectingTimer < kMaximumTimeout && false == theConnectionNeedsToBeDestroyed);
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::IsServerConnection(void)
{
	return theConnectionIsServer && nullptr != thePacketHandler && nullptr != theSafeConnection;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::CreateServerConnection(void)
{
	return CreateServerConnection(GameServer::ServerPort());
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::CreateServerConnection(const tbCore::uint16 serverPort)
{
	tb_debug_log(LogServer::Always() << "Creating a connection on port: " << serverPort);

	theConnectingTimer = 0;
	theConnectionIsServer = true;
	theConnectionNeedsToBeDestroyed = false;

	thePacketHandler = new ServerPacketHandler(); //actually owns/manages the server handler.
	theSafePacketHandler = new SafeOrFastConnectionProxyHandler(*thePacketHandler, true);
	theSafeConnection = new tbNetwork::SocketConnection(tbNetwork::SocketConnectionType::ServerPacketTCP);
	if (nullptr != theSafeConnection && true == theSafeConnection->Connect("", serverPort, *theSafePacketHandler))
	{
		theFastPacketHandler = new SafeOrFastConnectionProxyHandler(*thePacketHandler, false);
		theFastConnection = new tbNetwork::SocketConnection(tbNetwork::SocketConnectionType::ServerPacketUDP);
		if (nullptr != theFastConnection && true == theFastConnection->Connect("", serverPort, *theFastPacketHandler))
		{
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::CreateClientConnection(void)
{
	return CreateClientConnection(GameServer::ServerIP(), GameServer::ServerPort());
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::CreateClientConnection(const tbCore::tbString& serverIP, const tbCore::uint16 serverPort)
{
	tb_debug_log(LogClient::Always() << "Attempting to connect to server at " << serverIP << ":" << serverPort);

	theConnectingTimer = 0;
	theConnectionIsServer = false;
	theConnectionNeedsToBeDestroyed = false;

	thePacketHandler = new ClientPacketHandler();
	theSafePacketHandler = new SafeOrFastConnectionProxyHandler(*thePacketHandler, true);
	theSafeConnection = new tbNetwork::SocketConnection(tbNetwork::SocketConnectionType::ClientPacketTCP);
	if (nullptr != theSafeConnection && true == theSafeConnection->Connect(serverIP, serverPort, *theSafePacketHandler))
	{	//Note: 2022-08-19: Remember due to TCP and non-blocking sockets, Connect() doesn't mean a connection is fully
		//  established until ClientPacketHandler::OnConnect() is called, but if nothing major went wrong we can at
		//  least start preparing to register the FastConnection.
		theFastPacketHandler = new SafeOrFastConnectionProxyHandler(*thePacketHandler, false);
		theFastConnection = new tbNetwork::SocketConnection(tbNetwork::SocketConnectionType::ClientPacketUDP);
		if (nullptr != theFastConnection && true == theFastConnection->Connect(serverIP, serverPort, *theFastPacketHandler))
		{
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::DestroyConnection(const DisconnectReason reason)
{
	if (false == IsServerConnection())
	{
		TinyPacket disconnectPacket = CreateTinyPacket(PacketType::Disconnect, static_cast<tbCore::byte>(reason));
		SendSafePacket(disconnectPacket);
		SendFastPacket(disconnectPacket);
	}
	else
	{
	}

	tbCore::SafeDelete(theSafeConnection);
	tbCore::SafeDelete(theFastConnection);
	tbCore::SafeDelete(thePacketHandler);

	theConnectionIsServer = false;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::DestroyConnectionSoon(const DisconnectReason reason)
{
	theConnectionNeedsToBeDestroyed = true;
	theReasonToDestoyTheConnection = reason;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::DisconnectDriver(const DriverIndex driverIndex, const DisconnectReason reason)
{
	ServerPacketHandler& serverHandler = GetMutableServerHandler();
	DisconnectClient(serverHandler.GetSafeConnection(driverIndex), serverHandler.GetFastConnection(driverIndex), reason);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::DisconnectClient(const SafeConnection safeConnection,
	const FastConnection fastConnection, const DisconnectReason reason)
{
	if (true == IsConnected() && true == IsServerConnection())
	{
		tb_always_log(LogServer::Always() << "Disconnecting connection safe( " << +safeConnection << " ) fast( " <<
			+fastConnection << " ) because " << ToString(reason));

		TinyPacket disconnectPacket = CreateTinyPacket(PacketType::Disconnect, static_cast<byte>(reason));
		if (tbNetwork::InvalidClientID() != safeConnection && theSafeConnection->IsClientConnected(safeConnection))
		{
			SendSafePacketTo(disconnectPacket, safeConnection);
			theSafeConnection->DisconnectClient(safeConnection);
		}

		if (tbNetwork::InvalidClientID() != fastConnection && theFastConnection->IsClientConnected(fastConnection))
		{
			SendFastPacketTo(disconnectPacket, fastConnection);
			theFastConnection->DisconnectClient(fastConnection);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::ServerPacketHandler& LudumDare56::Network::GetMutableServerHandler(void)
{
	ServerPacketHandler* serverHandler = dynamic_cast<ServerPacketHandler*>(thePacketHandler);
	tb_error_if(nullptr == serverHandler, "Expected the serverHandler to be non-null, connection must be a server.");
	return *serverHandler;
}

//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::Network::ServerPacketHandler& LudumDare56::Network::GetServerHandler(void)
{
	ServerPacketHandler* serverHandler = dynamic_cast<ServerPacketHandler*>(thePacketHandler);
	tb_error_if(nullptr == serverHandler, "Expected the serverHandler to be non-null, connection must be a server.");
	return *serverHandler;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::ClientPacketHandler& LudumDare56::Network::GetMutableClientHandler(void)
{
	ClientPacketHandler* clientHandler = dynamic_cast<ClientPacketHandler*>(thePacketHandler);
	tb_error_if(nullptr == clientHandler, "Expected the clientHandler to be non-null, connection must be a client.");
	return *clientHandler;
}

//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::Network::ClientPacketHandler& LudumDare56::Network::GetClientHandler(void)
{
	ClientPacketHandler* clientHandler = dynamic_cast<ClientPacketHandler*>(thePacketHandler);
	tb_error_if(nullptr == clientHandler, "Expected the clientHandler to be non-null, connection must be a client.");
	return *clientHandler;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::Implementation::SendPacket(const tbCore::byte* packetData, const size_t packetSize,
	const ConnectionType connectionType)
{
	if (true == IsConnected())
	{
		if (packetSize < 256)
		{
			tbNetwork::SocketConnection* connection = (ConnectionType::Fast == connectionType) ? theFastConnection : theSafeConnection;

			//TODO: LudumDare56: Cleanup: It would be a lot better for debugging to trace Tiny/Small/Large packets
			//  and show the sub-type. PacketType is getting the sub-type for those packets which made me spend 30 minutes
			//  looking into an "issue" with AuthenticateRequest being sent 5 times when it was actually sent once as a
			//  large packet that had 5 parts.  Showing this as LargePacket( AuthenticateRequest ) would be better.
			const PacketType packetType = GetPacketTypeFrom(packetData, packetSize);
			TracePacket("Sending", packetData, packetSize);

			const bool wasPacketSent = connection->SendPacket(packetData, packetSize);
			tb_debug_log_if(false == wasPacketSent, LogNetwork::Warning() << "Packet " << packetType << " was not sent.");
		}
		else //if (packetSize >= 256)
		{
			tb_error_if(ConnectionType::Safe != connectionType, "LargePayload packets can only be sent over safe connection.");
			SendLargePayload(static_cast<PacketType>(packetData[1]), packetData, packetSize);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::Implementation::SendLargePayload(PacketType packetType, const tbCore::byte* packetData,
	const size_t packetSize, bool setFirstBytes)
{
	LargePayloadPacket payloadPacket;
	payloadPacket.type = PacketType::LargePayload;
	payloadPacket.size = 4;
	payloadPacket.subtype = static_cast<tbCore::byte>(packetType);
	payloadPacket.finished = 0;

	if (true == setFirstBytes)
	{
		payloadPacket.payload[0] = 0; //old size, which is too small for large payloads.
		payloadPacket.payload[1] = static_cast<tbCore::byte>(packetType); //always byte 2 for packet type.
		(*(reinterpret_cast<tbCore::uint16*>(&payloadPacket.payload[2]))) = tbCore::RangedCast<tbCore::uint16>(packetSize);
		payloadPacket.size += 4;
	}

	bool firstPacket = true;
	for (size_t packetIndex = 0, payloadIndex = 0; packetIndex < packetSize; ++packetIndex, ++payloadIndex)
	{
		const size_t firstBytesOffset = (true == firstPacket && true == setFirstBytes) ? 4 : 0;

		if (Network::LargePayloadPacket::kPayloadSize == payloadIndex + firstBytesOffset)
		{
			firstPacket = false;
			SendSafePacket(payloadPacket);
			payloadPacket.size = 4;
			payloadIndex = 0;
		}

		payloadPacket.payload[payloadIndex + firstBytesOffset] = packetData[packetIndex];
		++payloadPacket.size;
	}

	payloadPacket.finished = 1;
	SendSafePacket(payloadPacket);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::Implementation::SendPacketTo(const tbCore::byte* packetData, const size_t packetSize,
	const tbCore::byte toConnection, const ConnectionType connectionType)
{
	tb_always_log_if(tbNetwork::InvalidClientID() == toConnection, LogServer::Warning() <<
		"SendPacketTo() is broadcasting: " << Network::GetPacketTypeFrom(packetData, packetSize) << " to all clients, use " <<
		((Network::ConnectionType::Safe == connectionType) ? "SendSafePacket()" : "SendFastPacket()") << " instead?");

	if (true == IsConnected())
	{
		if (packetSize < 256)
		{
			tbNetwork::SocketConnection* connection = (ConnectionType::Fast == connectionType) ? theFastConnection : theSafeConnection;

			TracePacket("Sending", packetData, packetSize, "to " + tbCore::ToString(static_cast<int>(toConnection)));
			connection->SendPacketTo(packetData, packetSize, toConnection);
		}
		else //(packetSize >= 256)
		{
			tb_error_if(ConnectionType::Safe != connectionType, "LargePayload packets can only be sent over safe connection.");
			SendLargePayloadTo(static_cast<Network::PacketType>(packetData[1]), packetData, packetSize, toConnection);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::Implementation::SendLargePayloadTo(PacketType packetType, const tbCore::byte* packetData,
	const size_t packetSize, const SafeConnection toConnection, bool setFirstBytes)
{
	LargePayloadPacket payloadPacket;
	payloadPacket.type = PacketType::LargePayload;
	payloadPacket.size = 4;
	payloadPacket.subtype = static_cast<tbCore::byte>(packetType);
	payloadPacket.finished = 0;
	memset(payloadPacket.payload, 0, LargePayloadPacket::kPayloadSize);

	size_t firstBytesOffset = 0;

	if (true == setFirstBytes)
	{
		payloadPacket.payload[0] = 0; //old size, which is too small for large payloads.
		payloadPacket.payload[1] = static_cast<tbCore::byte>(packetType); //always byte 2 for packet type.
		(*(reinterpret_cast<tbCore::uint16*>(&payloadPacket.payload[2]))) = tbCore::RangedCast<tbCore::uint16>(packetSize);
		payloadPacket.size += 4;
		firstBytesOffset = 4;
	}

	for (size_t packetIndex = 0, payloadIndex = 0; packetIndex < packetSize; ++packetIndex, ++payloadIndex)
	{
		//If the payload packet is already full (we know there is more data to send as the byte at packetIndex has not
		//  been put into the payload yet), then fire off this partial packet and continue inserting data. 2022-05-25
		//Note: The check is above pushing the data because unsigned index type, and incrementing at top of loop would
		//  shift the inserted data a byte off where it should have been placed.
		if (Network::LargePayloadPacket::kPayloadSize == payloadIndex + firstBytesOffset)
		{
			firstBytesOffset = 0; //no-longer the first packet, so the first bytes are gone.

			SendSafePacketTo(payloadPacket, toConnection);
			payloadPacket.size = 4;
			payloadIndex = 0;
			memset(payloadPacket.payload, 0, LargePayloadPacket::kPayloadSize);
		}

		payloadPacket.payload[payloadIndex + firstBytesOffset] = packetData[packetIndex];
		++payloadPacket.size;
	}

	payloadPacket.finished = 1;
	SendSafePacketTo(payloadPacket, toConnection);
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::uint32 LudumDare56::Network::GetMillisecondsPerPacket(void)
{
	return theMaximiumTimeToSendUpdate;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::uint8 LudumDare56::Network::GetPacketsPerSecond(void)
{
	return theUpdatePacketsPerSecond;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::SetPacketsPerSecond(const tbCore::uint8 packetsPerSecond)
{
	theUpdatePacketsPerSecond = tbMath::Clamp<tbCore::uint8>(packetsPerSecond, 1, 50);
	theMaximiumTimeToSendUpdate = 1000 / theUpdatePacketsPerSecond;

	tb_always_log(LogNetwork::Info() << "Network is now sending " << +theUpdatePacketsPerSecond <<
		" packets per second which is " << theMaximiumTimeToSendUpdate << " ms per packet.");

	if (true == IsServerConnection())
	{
		SendSafePacket(CreateTinyPacket(PacketType::NetworkSettings, theUpdatePacketsPerSecond));
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::Simulate(void)
{
	tb_error_if(nullptr == theSafeConnection, "Expected theConnection to be VALID in multiplayer modes!");

	//GameClient needs to receive data before it can be fully established, Server does not need to...
	if (false == IsConnected() || (nullptr != thePacketHandler && false == HasReceivedData()))
	{
		theConnectingTimer += kStepTime;
	}
	else if(nullptr != thePacketHandler)
	{
		thePacketHandler->FixedUpdate(10);

		theSendUpdateTimer += kStepTime;
		if (theSendUpdateTimer >= theMaximiumTimeToSendUpdate)
		{
			theSendUpdateTimer = 0;
			SendUpdatePackets();
		}

		static tbCore::uint32 timer = 0;
		if (timer >= 1000)
		{
			static tbCore::uint64 totalSent = theSafeConnection->GetTotalBytesSent();
			static tbCore::uint64 totalReceived = theSafeConnection->GetTotalBytesReceived();
			if (totalSent != theSafeConnection->GetTotalBytesSent() || totalReceived != theSafeConnection->GetTotalBytesReceived())
			{
				totalSent = theSafeConnection->GetTotalBytesSent();
				totalReceived = theSafeConnection->GetTotalBytesReceived();

				tb_always_log(LogNetwork::Trace() << "Status Update:\n\tOut: " << totalSent << "\n\tIn: " << totalReceived);
				timer -= 1000;
			}
		}
		else
		{
			timer += kStepTime;
		}

		if (false == IsServerConnection())
		{
			PushBackOrShiftForward(theSafeConnectionLatency, static_cast<float>(thePacketHandler->GetSafeConnectionLatency()));
			PushBackOrShiftForward(theFastConnectionLatency, static_cast<float>(thePacketHandler->GetFastConnectionLatency()));
		}
	}

	if (true == theConnectionNeedsToBeDestroyed)
	{
		DestroyConnection(theReasonToDestoyTheConnection);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::Implementation::SendUpdatePackets(void)
{
	if (true == IsServerConnection())
	{
		if (0 != GameState::RaceSessionState::GetWorldTimer())
		{
			//TODO: LudumDare56: Optimization: It might be better to send multiple cars in a single update to make
			//   larger packets. (cannot use large payload packet on fast udp connection)
			for (RacecarIndex racecarIndex = 0; racecarIndex < GameState::kNumberOfRacecars; ++racecarIndex)
			{
				if (true == GameState::RacecarState::Get(racecarIndex).IsRacecarInUse())
				{
					//SendSafePacket(CreateUpdatePacket(racecarIndex, GameState::RaceSession::GetWorldTimer()));
					SendFastPacket(CreateRacecarUpdatePacket(racecarIndex, GameState::RaceSessionState::GetWorldTimer()));
				}
			}
		}
	}
	else //ClientConnection
	{
#if !defined(ludumdare56_headless_build)
		const RacecarIndex playerRacecarIndex = GetClientHandler().GetRacecarIndexForPlayer();

		if (true == GameState::IsValidRacecar(playerRacecarIndex) && 0 != GameState::RaceSessionState::GetWorldTimer())
		{
			SendFastPacket(Network::CreateRacecarUpdatePacket(playerRacecarIndex, GameState::RaceSessionState::GetWorldTimer()));
		}
#endif /* ludumdare56_headless_build */
	}
}

//--------------------------------------------------------------------------------------------------------------------//

#if defined(development_build) && !defined(ludumdare56_headless_build)

#include "../core/development/tb_imgui_implementation.hpp"

void ImGuiShowNetworkHistoryFor(const tbCore::tbString& label, std::vector<float> latencyValues)
{
	const float kMinimumLatency = 0.0f;
	static float sMaximumEverLatency = 10.0f;

	float maximumValue = 0.0f;
	for (const float value : latencyValues)
	{
		if (value > maximumValue) { maximumValue = value; }
		if (value > sMaximumEverLatency) { sMaximumEverLatency = value; }
	}

	if (maximumValue < 100.0f)
	{
		sMaximumEverLatency = 100.0f;
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(121, 210, 70)));
		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(ImColor(121, 210, 70)));
	}
	else if (maximumValue < 250.0f)
	{
		sMaximumEverLatency = 250.0f;
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(210, 210, 70)));
		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(ImColor(210, 210, 70)));
	}
	else if (maximumValue < 350.0f)
	{
		sMaximumEverLatency = 250.0f;
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(230, 130, 50)));
		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(ImColor(230, 130, 50)));
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(230, 50, 50)));
		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(ImColor(230, 50, 50)));
	}

	ImGui::PlotLines((label + ": " + tbCore::ToString<float>(maximumValue) + " ms").c_str(), latencyValues.data(),
		tbCore::size(theSafeConnectionLatency.size()), 0, nullptr, kMinimumLatency, sMaximumEverLatency, ImVec2(0, 40.0f));

	ImGui::PopStyleColor(2);
}

#endif /* defined(development_build) && !defined(ludumdare56_headless_build) */

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::Development::ImGuiShowNetworkHistory(void)
{
#if defined(development_build) && !defined(ludumdare56_headless_build)
	if (true == ImGui::CollapsingHeader("Network", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGuiShowNetworkHistoryFor("TCP Latency", theSafeConnectionLatency);
		ImGuiShowNetworkHistoryFor("UDP Latency", theFastConnectionLatency);
	}
#endif
}

//--------------------------------------------------------------------------------------------------------------------//
