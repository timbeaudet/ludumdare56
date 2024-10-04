///
/// @file
/// @details An object that does all the magic of handling the Ping Messaging from client to GameServer.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_PingMonitor_hpp
#define LudumDare56_PingMonitor_hpp

#include "../game_state/race_session_state.hpp"
#include "../network/network_packets.hpp"
#include "../network/network_connection_types.hpp"

#include "../core/ring_buffer.hpp"

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/network/tb_socket_connection.hpp>

#include <array>

//--------------------------------------------------------------------------------------------------------------------//

namespace LudumDare56
{
	namespace Network
	{
		constexpr tbCore::uint32 MaximumPingAllowed(void) { return 5000; }
		constexpr tbCore::uint32 InvalidLatency(void) { return ~tbCore::uint32(0); }

		class PingMonitor
		{
		public:
			PingMonitor(bool isServer = true);
			~PingMonitor(void);

			inline void SetSafeConnection(SafeConnection safeConnection) { mSafeConnection = safeConnection; }
			inline void SetFastConnection(FastConnection fastConnection) { mFastConnection = fastConnection; }

			void Reset(void);

			///
			/// @details Updates the PingMonitor to send out ping messages if necessary,
			///
			void Update(const tbCore::uint32& deltaTimeMS);

			bool HandlePacket(const PingPacket& pingPacket, tbCore::byte fromConnection);

			void SetRegisteredFastConnection(bool isRegistered);

			inline tbCore::uint32 GetTimeSinceLastPingResponse(void) const { return mWallClockTimer - mLastReceivedTime; }
			inline void ReceivedKeepAlive(void) { mLastReceivedTime = mWallClockTimer; }

			tbCore::tbString InformationAsString(void) const;

			tbCore::uint32 GetPingCount(const ConnectionType connectionType) const;
			tbCore::uint32 GetCurrentPing(const ConnectionType connectionType) const;
			tbCore::uint32 GetAveragePing(const ConnectionType connectionType) const;

			/// Return InvalidLatency if the available information does not reach minimumCount.
			tbCore::uint32 GetSyncedLatency(int minimumCount, const ConnectionType connectionType) const;

			/// @details Returns true if the PingMonitor has enough history to GetSyncedLatency.
			bool IsSyncedLatencyReady(const tbCore::byte minimumCount = kNumberOfPings) const;

		private:
			struct PingInfo
			{
				tbCore::uint32 mSentAtTime;
				tbCore::uint32 mLatency;
			};

			static const tbCore::byte kNumberOfPings = 32; //Must be 32 or less, see sizeof pingid in the PingPacket struct, 5bits at time of writing.
			typedef std::array<PingInfo, kNumberOfPings + 1> PingArray; //[kNumberPings] = last/current latency

			void ResetPingArray(PingArray& pingArray);
			void SendPingTo(const tbCore::byte connection, PingArray& pingArray, const ConnectionType connectionType);

			tbCore::uint32 mLastReceivedTime;
			PingArray mPingArrayTCP;
			PingArray mPingArrayUDP;
			TyreBytes::Core::RingBuffer<tbCore::uint32, kNumberOfPings> mSyncedPings;
			tbCore::uint32 mWallClockTimer;
			tbCore::uint32 mLastPingSentTimer;
			tbCore::byte mPingIndex;
			SafeConnection mSafeConnection;
			FastConnection mFastConnection;
			bool mIsServer;
			bool mIsRegisteredFastConnection;
		};

	};	//namespace Network
};	//namespace LudumDare56

//--------------------------------------------------------------------------------------------------------------------//

#endif /* LudumDare56_PingMonitor_hpp */
