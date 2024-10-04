///
/// @file
/// @details Combines multiple packets into a single large payload.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "large_payload_handler.hpp"
#include "network_packets.hpp"

#include <turtle_brains/core/tb_error.hpp>

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::LargePayloadHandler::LargePayloadHandler(void) :
	mPayload(),
	mPacketType(0)
{
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::Network::LargePayloadHandler::AppendData(const LargePayloadPacket& payloadPacket)
{
	if (true == mPayload.empty())
	{
		mPacketType = payloadPacket.subtype;

		//This was initially added when I hacked a string for testing userKey, it should never be used when sending
		//  an actual large packet, though perhaps we may one day wish to add a flag to send large strings?
		//
		////This 'hack' was added to ensure the payload contains a type as the first byte. It might be desired to
		////keep that inside actual packed structs still, but this made it far easier to send string of unknown size.
		////The first byte being a 0 is because most of those packets expect size to be a single first byte...
		//mPayload.push_back(0);
		//mPayload.push_back(mPacketType);
	}

	tb_error_if(mPacketType != payloadPacket.subtype, "Handling an unexpected packet, communications are lost.");

	for (size_t payloadIndex = 0; payloadIndex < static_cast<size_t>(payloadPacket.size) - 4; ++payloadIndex)
	{
		mPayload.push_back(payloadPacket.payload[payloadIndex]);
	}

	return (1 == payloadPacket.finished);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::LargePayloadHandler::Clear(void)
{
	mPayload.clear();
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::byte LudumDare56::Network::LargePayloadHandler::GetPacketType(void) const
{
	return mPacketType;
}

//--------------------------------------------------------------------------------------------------------------------//

size_t LudumDare56::Network::LargePayloadHandler::GetPacketSize(void) const
{
	return mPayload.size();
}

//--------------------------------------------------------------------------------------------------------------------//

const tbCore::byte* LudumDare56::Network::LargePayloadHandler::GetPacketData(void) const
{
	return mPayload.data();
}

//--------------------------------------------------------------------------------------------------------------------//
