///
/// @file
/// @details An object that does all the magic of handling the Ping Messaging from client to GameServer.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "ping_monitor.hpp"
#include "network_manager.hpp"
#include "../logging.hpp"

#include <turtle_brains/core/tb_types.hpp>

namespace
{
	//
	// If we keep sending ping packets every ~200 milliseconds we can use the PingMonitors to get the synced latency
	// pretty much at any time we want, assuming there is enough responses for a good average. If this makes the lines
	// too noisy we can add another PingArray to hold times for the synchronization pings that would happen faster.
	// That could also reuse the PingPacket by grabbing one of the unused bits to mark as a SyncPing and that could
	// run on a faster timer until all the sync pings came in, or are no-long required.
	//
	// For now, we just blast a lot of extra pings!
	//
	const tbCore::uint32 kMaximumPingRateTimer = 200;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::PingMonitor::PingMonitor(bool isServer) :
	mLastReceivedTime(InvalidLatency()),
	mPingArrayTCP(),
	mPingArrayUDP(),
	mSyncedPings(),
	mWallClockTimer(0),
	mLastPingSentTimer(0),
	mPingIndex(0),
	mSafeConnection(tbNetwork::InvalidClientID()),
	mFastConnection(tbNetwork::InvalidClientID()),
	mIsServer(isServer),
	mIsRegisteredFastConnection(false)
{
	Reset();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::PingMonitor::~PingMonitor(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::PingMonitor::Reset(void)
{
	ResetPingArray(mPingArrayTCP);
	ResetPingArray(mPingArrayUDP);

	mLastReceivedTime = InvalidLatency();
	mSyncedPings.Clear();

	mWallClockTimer = 0;
	mLastPingSentTimer = 0;
	mPingIndex = 0;
	mSafeConnection = tbNetwork::InvalidClientID();
	mFastConnection = tbNetwork::InvalidClientID();
	mIsRegisteredFastConnection = false;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::PingMonitor::Update(const tbCore::uint32& deltaTimeMS)
{
	mWallClockTimer += deltaTimeMS;

	if (mLastPingSentTimer > deltaTimeMS)
	{
		mLastPingSentTimer -= deltaTimeMS;
	}
	else //subtracting deltaTime would underflow mLastPingSentTimer, so the timer is triggering now.
	{
		mLastPingSentTimer = (true == IsSyncedLatencyReady()) ? kMaximumPingRateTimer : 80;

		SendPingTo(mSafeConnection, mPingArrayTCP, ConnectionType::Safe);

		if (true == mIsRegisteredFastConnection)
		{	//Don't broadcast the ping message to everyone.
			SendPingTo(mFastConnection, mPingArrayUDP, ConnectionType::Fast);
		}

		++mPingIndex;
		if (mPingIndex >= kNumberOfPings)
		{
			mPingIndex = 0;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::PingMonitor::HandlePacket(const PingPacket& pingPacket, tbCore::byte fromConnection)
{
	const ConnectionType connectionType((PingFlags::ConnectionUDP == (PingFlags::ConnectionUDP & pingPacket.flags)) ?
		ConnectionType::Fast : ConnectionType::Safe);

	const PacketType packetType = static_cast<PacketType>(pingPacket.type);

	if (PacketType::PingRequest == packetType)
	{
		if (false == mIsServer)
		{
			if (ConnectionType::Fast == connectionType && false == mIsRegisteredFastConnection)
			{
				tb_always_log(LogNetwork::Info() << "PingMonitor: Received ping request packet; has unregistered udp socket.");
				return false;
			}
		}

		PingPacket pingResponse = pingPacket;
		pingResponse.type = PacketType::PingResponse;

		if (true == mIsServer)
		{
			if (ConnectionType::Fast == connectionType)
			{
				Network::SendFastPacketTo(pingResponse, fromConnection);
			}
			else
			{
				Network::SendSafePacketTo(pingResponse, fromConnection);
			}
		}
		else
		{
			if (ConnectionType::Fast == connectionType)
			{
				Network::SendFastPacket(pingResponse);
			}
			else
			{
				Network::SendSafePacket(pingResponse);
			}
		}

		return true;
	}

	if (PacketType::PingResponse == packetType)
	{
		PingArray& pingArray((ConnectionType::Fast == connectionType) ? mPingArrayUDP : mPingArrayTCP);
		PingInfo& pingInfo = pingArray[pingPacket.pingid];
		if (pingPacket.time != pingInfo.mSentAtTime)
		{
			tb_debug_log(LogNetwork::Warning() << "PingMonitor: Received " << ((ConnectionType::Fast == connectionType) ? "UDP" : "TCP") <<
				" ping response from mismatched time: " << pingPacket.time << " expected " << pingInfo.mSentAtTime <<
				" with pingid" << static_cast<int>(pingPacket.pingid));
			return false;
		}

		mLastReceivedTime = mWallClockTimer;
		if (pingInfo.mSentAtTime <= mWallClockTimer)
		{
			const tbCore::uint32 roundTripTime = mWallClockTimer - pingInfo.mSentAtTime;
			pingInfo.mLatency = roundTripTime;
			pingArray[kNumberOfPings].mLatency = roundTripTime;
			if (ConnectionType::Safe == connectionType)
			{
				mSyncedPings.Push(roundTripTime);
			}
		}
		else
		{
			pingInfo.mLatency = InvalidLatency();
			pingArray[kNumberOfPings].mLatency = InvalidLatency();
			tb_debug_log(LogNetwork::Error() << "PingMonitor: Ignored ping response as it would cause overflow.");
		}

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::PingMonitor::SetRegisteredFastConnection(bool isRegistered)
{
	tb_debug_log(LogNetwork::Info() << "PingMonitor UDP Socket is now " << ((isRegistered) ? "registered" : "unregistered") << ".");
	mIsRegisteredFastConnection = isRegistered;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString LudumDare56::Network::PingMonitor::InformationAsString(void) const
{
	const tbCore::uint32 safePing = GetCurrentPing(ConnectionType::Safe);
	const tbCore::uint32 fastPing = GetCurrentPing(ConnectionType::Fast);
	const tbCore::uint32 safeAverage = GetAveragePing(ConnectionType::Safe);
	const tbCore::uint32 fastAverage = GetAveragePing(ConnectionType::Fast);

	return "tcp(" + tb_string(static_cast<int>(mSafeConnection)) + ") " + tb_string(safePing) + "ms  " +
		tb_string(safeAverage) + "avg    udp(" + tb_string(static_cast<int>(mFastConnection)) + ") " + tb_string(fastPing) +
		"ms  " + tb_string(fastAverage) + "avg";
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::uint32 LudumDare56::Network::PingMonitor::GetPingCount(const ConnectionType connectionType) const
{
	const PingArray& pingArray((ConnectionType::Fast == connectionType) ? mPingArrayUDP : mPingArrayTCP);

	//NOTE: The following does index 0 to < kNumberOfPings because the size of the pingArray
	//   is kNumberOfPings + 1 to hold the current latency, but we do not want that in average.
	tbCore::uint32 count = 0;
	for (size_t pingIndex = 0; pingIndex < kNumberOfPings; ++pingIndex)
	{
		const tbCore::uint32 pingValue = pingArray[pingIndex].mLatency;
		if (InvalidLatency() != pingValue)
		{
			++count;
		}
	}
	return count;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::uint32 LudumDare56::Network::PingMonitor::GetCurrentPing(const ConnectionType connectionType) const
{
	const PingArray& pingArray((ConnectionType::Fast == connectionType) ? mPingArrayUDP : mPingArrayTCP);
	return pingArray[kNumberOfPings].mLatency;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::uint32 LudumDare56::Network::PingMonitor::GetAveragePing(const ConnectionType connectionType) const
{
	const PingArray& pingArray((ConnectionType::Fast == connectionType) ? mPingArrayUDP : mPingArrayTCP);

	//NOTE: The following does index 0 to < kNumberOfPings because the size of the pingArray
	//   is kNumberOfPings + 1 to hold the current latency, but we do not want that in average.
	int count = 0;
	int total = 0;
	for (size_t pingIndex = 0; pingIndex < kNumberOfPings; ++pingIndex)
	{
		const tbCore::uint32 pingValue = pingArray[pingIndex].mLatency;
		if (InvalidLatency() != pingValue)
		{
			total += pingValue;
			++count;
		}
	}

	if (0 == count)
	{
		return 0;
	}

	return total / count;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::uint32 LudumDare56::Network::PingMonitor::GetSyncedLatency(int minimumCount, const ConnectionType connectionType) const
{
	tb_error_if(ConnectionType::Fast == connectionType, "Not yet supported as starting on single socket TCP only...");

	if (0 == mSyncedPings.size() || mSyncedPings.size() < static_cast<size_t>(minimumCount))
	{
		return InvalidLatency();
	}

	tbCore::uint32 totalLatency = 0;
	for (size_t pingIndex = 0; pingIndex < mSyncedPings.size(); ++pingIndex)
	{
		totalLatency += mSyncedPings[pingIndex];
	}

	return tbCore::size(totalLatency / mSyncedPings.size());
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::PingMonitor::IsSyncedLatencyReady(const tbCore::byte minimumCount) const
{
	return (mSyncedPings.size() >= minimumCount);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::PingMonitor::ResetPingArray(PingArray& pingArray)
{
	for (PingInfo& ping : pingArray)
	{
		ping.mSentAtTime = 0;
		ping.mLatency = InvalidLatency();
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::PingMonitor::SendPingTo(const tbCore::byte connection, PingArray& pingArray, const ConnectionType connectionType)
{
	const PingPacket pingPacket = CreatePingPacket(mWallClockTimer, mPingIndex, connectionType);
	if (true == mIsServer)
	{
		if (ConnectionType::Safe == connectionType)
		{
			Network::SendSafePacketTo(pingPacket, connection);
		}
		else
		{
			Network::SendFastPacketTo(pingPacket, connection);
		}
	}
	else
	{
		if (ConnectionType::Safe == connectionType)
		{
			Network::SendSafePacket(pingPacket);
		}
		else
		{
			Network::SendFastPacket(pingPacket);
		}
	}

	pingArray[mPingIndex].mSentAtTime = mWallClockTimer;
	pingArray[mPingIndex].mLatency = InvalidLatency();
}

//--------------------------------------------------------------------------------------------------------------------//
