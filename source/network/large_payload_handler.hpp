///
/// @file
/// @details Combines multiple packets into a single large payload.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_LargePayloadHandler_hpp
#define LudumDare56_LargePayloadHandler_hpp

#include "network_packets.hpp"

#include <turtle_brains/core/tb_types.hpp>

#include <vector>

namespace LudumDare56
{
	namespace Network
	{

		typedef tbCore::byte byte;

		class LargePayloadHandler
		{
		public:
			explicit LargePayloadHandler(void);

			bool AppendData(const LargePayloadPacket& payloadPacket);

			void Clear(void);

			tbCore::byte GetPacketType(void) const;
			size_t GetPacketSize(void) const;
			const tbCore::byte* GetPacketData(void) const;

		private:
			std::vector<tbCore::byte> mPayload;
			tbCore::byte mPacketType;
		};

	};	//namespace Network
};	//namespace LudumDare56

#endif /* LudumDare56_LargePayloadHandler_hpp */
