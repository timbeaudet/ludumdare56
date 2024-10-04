///
/// @file
/// @details Manages the connections, handlers and other network things for LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_NetworkManager_hpp
#define LudumDare56_NetworkManager_hpp

#include "../network/network_packets.hpp"
#include "../network/network_connection_types.hpp"

#include "../game_state/driver_state.hpp"

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_string.hpp>
#include <turtle_brains/network/tb_socket_connection.hpp>

namespace LudumDare56
{
	namespace Network
	{
		namespace Implementation
		{
			//SendPacket expects the first byte of packetData to be the size of the packet, and the second byte to be the type.
			//The size limitation come from TurtleBrains::Network API and the type is expected for the handlers and used to
			//Will log the packet type being sent as well as create the LargePayload packet with proper subtype when size is large.
			void SendPacket(const tbCore::byte* packetData, const size_t packetSize, const ConnectionType connectionType);
			void SendPacketTo(const tbCore::byte* packetData, const size_t packetSize, const tbCore::byte connection, const ConnectionType connectionType);
		};
	};
};

namespace LudumDare56
{
	namespace Network
	{
		class ClientPacketHandler;
		class ServerPacketHandler;


		bool IsConnected(void);
		bool IsRegistered(void);
		bool IsReadyToPlay(void);
		bool HasReceivedData(void);

		bool IsAttemptingToConnect(void);
		bool IsServerConnection(void);

		bool CreateServerConnection(void);
		bool CreateServerConnection(const tbCore::uint16 serverPort);
		bool CreateClientConnection(void);
		bool CreateClientConnection(const tbCore::tbString& serverIP, const tbCore::uint16 serverPort);

		//Cannot be called from the Handler::OnHandlePacket because TurtleBrains is iterating through list of connections
		//and removing while iterating is bad. Instead use DestroyConnectionSoon().
		void DestroyConnection(const DisconnectReason reason);
		void DestroyConnectionSoon(const DisconnectReason reason);

		tbCore::uint32 GetMillisecondsPerPacket(void);
		tbCore::uint8 GetPacketsPerSecond(void);

		///
		/// @note This only has 10ms (or FixedTime steps) due to using Simulate() for polling things. Perhaps using
		///   an update could be better, possibly even a separate thread in the future etc...
		///
		void SetPacketsPerSecond(const tbCore::uint8 packetsPerSecond);

		void Simulate(void);

		template <typename Type> void SendSafePacket(const Type& packet)
		{
			Implementation::SendPacket(ToData(packet), packet.size, ConnectionType::Safe);
		}

		template <typename Type> void SendSafePacket(const Type& packet, const size_t packetSize)
		{
			Implementation::SendPacket(ToData(packet), packetSize, ConnectionType::Safe);
		}

		template <typename Type> void SendSafePacketTo(const Type& packet, const SafeConnection safeConnection)
		{
			Implementation::SendPacketTo(ToData(packet), packet.size, safeConnection, ConnectionType::Safe);
		}

		template <typename Type> void SendFastPacket(const Type& packet)
		{
			Implementation::SendPacket(ToData(packet), packet.size, ConnectionType::Fast);
		}

		template <typename Type> void SendFastPacket(const Type& packet, const size_t packetSize)
		{
			Implementation::SendPacket(ToData(packet), packetSize, ConnectionType::Fast);
		}

		template <typename Type> void SendFastPacketTo(const Type& packet, const FastConnection fastConnection)
		{
			Implementation::SendPacketTo(ToData(packet), packet.size, fastConnection, ConnectionType::Fast);
		}

		//The following code is not exactly the desired or well thought out API...
		//This should be called only from the GameServer and not from the client.
		void DisconnectDriver(const GameState::DriverIndex driverIndex, const DisconnectReason reason);
		void DisconnectClient(const SafeConnection safeConnection, const FastConnection fastConnection, const DisconnectReason reason);

		//This will trigger an error condition if the IsServerConnection() return false. Check for it yourself before calling!
		ServerPacketHandler& GetMutableServerHandler(void);
		const ServerPacketHandler& GetServerHandler(void);

		//This will trigger an error condition if the IsServerConnection() returns true. Check for it yourself before calling!
		ClientPacketHandler& GetMutableClientHandler(void);
		const ClientPacketHandler& GetClientHandler(void);

		namespace Development
		{
			void ImGuiShowNetworkHistory(void);
		};	//namespace Development
	};	//namespace Network
};	//namespace LudumDare56

#endif /* LudumDare56_NetworkManager_hpp */
