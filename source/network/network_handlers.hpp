///
/// @file
/// @details Defines the packet handlers for LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_NetworkHandlers_hpp
#define LudumDare56_NetworkHandlers_hpp

#include "../network/large_payload_handler.hpp"
#include "../network/network_packets.hpp" //for AuthenticationService enum
#include "../network/network_connection_types.hpp"
#include "../network/ping_monitor.hpp"
#include "../core/event_system.hpp"

#include <turtle_brains/core/tb_typed_integer.hpp>
#include <turtle_brains/network/tb_packet_handler_interface.hpp>
#include <turtle_brains/game/tb_game_timer.hpp>

#include <array>
#include <list>

namespace TyreBytes
{
	namespace Core
	{
		namespace Services { class ConnectorServiceInterface; }
	}
};

namespace LudumDare56
{
	namespace Network
	{

		class SafeOrFastConnectionProxyHandler;

		class LudumDare56PacketHandlerInterface : public tbNetwork::PacketHandlerInterface, public TyreBytes::Core::EventListener
		{
		public:
			LudumDare56PacketHandlerInterface(void);
			virtual ~LudumDare56PacketHandlerInterface(void);

			virtual void FixedUpdate(tbCore::uint32 deltaTimeMS);
			inline virtual tbCore::uint32 GetSafeConnectionLatency(void) const { return 0; }
			inline virtual tbCore::uint32 GetFastConnectionLatency(void) const { return 0; }

		protected:
			bool IsHandlingSafeConnection(void) const { return mHandlingSafeConnection; }

			///
			/// @details Returns the actual packet type of packet, for a Tiny, Small or LargePayload packet this will
			///   return the SubType of that payload rather than Tiny, Small or LargePayload.
			///
			PacketType GetActualPacketType(const tbCore::byte* packetData, size_t packetSize) const;

		private:
			friend SafeOrFastConnectionProxyHandler;
			bool mHandlingSafeConnection;
		};

		class ClientPacketHandler : public LudumDare56PacketHandlerInterface
		{
		public:
			static void SetUserAccessKey(const tbCore::tbString& userAccessKey, const AuthenticationService service);
			static bool HasUserAccessKey(void);

			ClientPacketHandler(void);
			virtual ~ClientPacketHandler(void);

			virtual void FixedUpdate(tbCore::uint32 deltaTimeMS) override;

			inline virtual tbCore::uint32 GetSafeConnectionLatency(void) const override { return mPingMonitor.GetCurrentPing(ConnectionType::Safe); }
			inline virtual tbCore::uint32 GetFastConnectionLatency(void) const override { return mPingMonitor.GetCurrentPing(ConnectionType::Fast); }

			inline bool IsAuthenticated(void) const { return mIsAuthenticated; }
			inline bool IsRegistered(void) const { return mIsRegistered; }
			inline bool IsReadyToPlay(void) const { return mIsReadyToPlay; }
			inline DriverIndex GetDriverIndexForPlayer(void) const { return mPlayerDriverIndex; }
			RacecarIndex GetRacecarIndexForPlayer(void) const;

		protected:
			virtual void OnConnect(void) override;
			virtual void OnDisconnect(void) override;
			bool CanHandlePacket(const tbCore::byte* packetData, size_t packetSize) const;
			virtual bool OnHandlePacket(const tbCore::byte* packetData, size_t packetSize, tbCore::byte fromConnection) override;
			virtual void OnHandleEvent(const TyreBytes::Core::Event& event) override;

		private:
			PingMonitor mPingMonitor;
			LargePayloadHandler mLargePayload;
			std::array<tbCore::uint32, GameState::kNumberOfRacecars> mLastUpdateTimes;
			tbGame::GameTimer mRegistrationTimer;
			tbCore::uint32 mRegistrationCode;
			DriverIndex mPlayerDriverIndex;
			bool mIsAuthenticated;
			bool mIsRegistered;
			bool mIsReadyToPlay;
		};

		class ServerPacketHandler : public LudumDare56PacketHandlerInterface
		{
		public:
			ServerPacketHandler(void);
			virtual ~ServerPacketHandler(void);

			virtual void FixedUpdate(tbCore::uint32 deltaTimeMS) override;

			tbCore::uint32 FindWorstCaseLatency(const ConnectionType connectionType) const;

			void BanDriver(const DriverIndex driverIndex);

			inline SafeConnection GetSafeConnection(const DriverIndex driverIndex) const { return mConnectedClients[driverIndex].mSafeConnection; }
			inline FastConnection GetFastConnection(const DriverIndex driverIndex) const { return mConnectedClients[driverIndex].mFastConnection; }

		protected:
			virtual void OnConnect(void) override;
			virtual void OnDisconnect(void) override;

			virtual void OnConnectClient(tbCore::byte clientID) override;
			virtual void OnDisconnectClient(tbCore::byte clientID) override;

			virtual bool OnHandlePacket(const tbCore::byte* packetData, size_t packetSize, tbCore::byte fromConnection) override;
			virtual void OnHandleEvent(const TyreBytes::Core::Event& event) override;

		private:
			inline DriverIndex GetDriverIndexFromSafeConnection(const SafeConnection safeConnection) const { return mSafeDriverTable[safeConnection]; }
			inline DriverIndex GetDriverIndexFromFastConnection(const FastConnection fastConnection) const { return mFastDriverTable[fastConnection]; }

			void ValidateDriverIndex(const SafeConnection safeConnection, const DriverIndex driverIndex);

			void OnAuthenticateConnection(const SafeConnection safeConnection, bool isAuthenticated, const GameState::DriverLicense& driverLicense);

			tbCore::uint32 CreateRegistrationCode(void) const;
			void AddUnregisteredClient(const FastConnection fastConnection);
			void RemoveUnregisteredClient(const FastConnection fastConnection);

			std::array<LargePayloadHandler, 256> mLargePayloads;

			/// @note While this effectively duplicates the information found in ConnectedClients, it allows
			///   faster lookup of Racecar index from a SafeConnection which is something that probably happens often enough
			///   to keep. Consider RacecarClient (which should actually be a Driver or ConnectedClient or something else)
			///   to be the primary / truthful source, and this should always reflect what it has to say.
			std::array<DriverIndex, 256> mSafeDriverTable;
			std::array<DriverIndex, 256> mFastDriverTable;

			std::list<TyreBytes::Core::Services::ConnectorServiceInterface*> mConnectorServices;

			///
			/// @details The UnregisteredClient is only the fast connection before fully registering with the SafeConnection
			///   to be setup and used within the ConnectedClient of the driver.
			///
			struct UnregisteredClient
			{
				FastConnection mFastConnection;
				tbCore::uint32 mRegistrationTimer;
			};

			struct ConnectedClient
			{
				PingMonitor mPingMonitor;
				tbCore::uint32 mLastUpdateTime;
				tbCore::uint32 mRegistrationCode;
				DriverIndex mDriverIndex;
				SafeConnection mSafeConnection;
				FastConnection mFastConnection;
			};

			template<typename ElementType, typename AccessType, size_t Size> class TypedArray : public std::array<ElementType, Size>
			{
			public:
				typedef std::array<ElementType, Size> Array;
				const ElementType& operator[](const AccessType index) const { return Array::operator[](index) ; }
				ElementType& operator[](const AccessType index) { return Array::operator[](index); }

				const ElementType& operator[](const size_t index) const = delete;
				ElementType& operator[](const size_t index) = delete;
			};

			TypedArray<ConnectedClient, DriverIndex, GameState::kNumberOfDrivers> mConnectedClients;
			std::vector<UnregisteredClient> mUnregisteredClients;
			std::vector<tbCore::tbString> mBannedDrivers;

			int mNumberOfConnections;
		};

		///
		/// @details This is a proxy handler that will pass the packets to be handled to the actualHandler only after
		///   Changing the mode in the LudumDare56PacketHandlerInterface so it will know what type of connection the packet
		///   being handled came from.
		///
		class SafeOrFastConnectionProxyHandler : public tbNetwork::PacketHandlerInterface
		{
		public:
			SafeOrFastConnectionProxyHandler(LudumDare56PacketHandlerInterface& actualHandler, bool isSafeConnection) :
				mActualHandler(actualHandler),
				mIsSafeConnection(isSafeConnection)
			{
			}

			virtual ~SafeOrFastConnectionProxyHandler(void)
			{
			}

		protected:
			virtual void OnConnect(void) override { SetMode(); mActualHandler.OnConnect(); }
			virtual void OnDisconnect(void) override { SetMode(); mActualHandler.OnDisconnect(); }

			virtual void OnConnectClient(tbCore::byte clientID) override { SetMode(); mActualHandler.OnConnectClient(clientID); }
			virtual void OnDisconnectClient(tbCore::byte clientID) override { SetMode(); mActualHandler.OnDisconnectClient(clientID); }

			virtual bool OnHandlePacket(const tbCore::byte* packetData, size_t packetSize, tbCore::byte fromConnection) override
			{
				SetMode(); return mActualHandler.OnHandlePacket(packetData, packetSize, fromConnection);
			}

		private:
			void SetMode(void) { mActualHandler.mHandlingSafeConnection = mIsSafeConnection; }

			LudumDare56PacketHandlerInterface& mActualHandler;
			const bool mIsSafeConnection;
		};

	};	//namespace Network
};	//namespace LudumDare56

#endif /* LudumDare56_NetworkHandlers_hpp */
