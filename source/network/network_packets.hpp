///
/// @file
/// @details Defines the packets that will get sent for the multiplayer component of LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_NetworkPackets_hpp
#define LudumDare56_NetworkPackets_hpp

#include "../network/network_connection_types.hpp"

#include "../game_state/race_session_state.hpp"
#include "../game_state/events/timing_events.hpp"

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_string.hpp>
#include <turtle_brains/core/tb_fixed_string.hpp>

#include <turtle_brains/math/tb_vector.hpp>

#include <array>

namespace LudumDare56
{
	namespace Network
	{
		typedef tbCore::byte byte;
		typedef GameState::DriverIndex DriverIndex;
		typedef GameState::RacecarIndex RacecarIndex;

		constexpr tbCore::uint8 PacketVersion(void) { return 1; }

		enum class PacketSizeType : tbCore::uint8 { };
		typedef tbCore::TypedInteger<PacketSizeType> PacketSize;

		enum class PacketType : tbCore::uint8
		{
			Tiny,
			Small,
			LargePayload,                //Packet of unknown size that is or could be larger than 256 bytes limited by TurtleBrains...

			JoinRequest,                 //Can be sent without userid (needs to).
			JoinResponse,                //Send by server only.
			NetworkSettings,             //Sent via a TinyPacket from GameServer to clients to update the network settings.
			Disconnect,                  //Sent via a TinyPacket to indicate a graceful disconnection.

			AuthenticateRequest,         //Sent from client to GameServer with a UserAccessKey to become authenticated/verified.
			AuthenticateResponse,        //Sent from GameServer to client if authentication of SafeConnection was successful, and server had space, etc.
			RegistrationStartRequest,    //Sent via a TinyPacket from client to GameServer over SafeConnection to begin the FastConnection registration process.
			RegistrationStartResponse,   //Sent from GameServer to client with a RegistrationCode for the process...
			RegistrationRequest,         //Sent from client to GameServer over FastConnection with a RegistrationCode provided from RegistrationStartResponse.
			RegistrationResponse,        //Sent from GameServer to client over SafeConnection once the FastConnection was registered correctly.

			PhaseChanged,                //Sent from GameServer to client when the phase of the game changes Lobby/Round.
			RaceSessionTimer,            //Sent via a SmallPacket from GameServer to client when the WorldTimer gets reset/changed.
			StartGrid,                   //Send via a GameServer to client with information on the start grid.

			RacetrackRequest,            //Sent via a TinyPacket from client to GameServer to request current racetrack.
			RacetrackResponse,           //Sent from GameServer to client to share information about current racetrack.
			RacetrackLoaded,             //Sent from client to GameServer over SafeConnection when the client finished loading the racetrack.

			DriverJoined,
			DriverLeft,
			DriverEntersRacecar,         //Sent to indicate the driver has taken control of a racecar.
			DriverLeavesRacecar,         //Sent via a TinyPacket from the GameServer to clients when a driver leaves their racecar.

			RacecarReset,
			RacecarRequest,              //Sent from client to GameServer with DriverIndex to request a racecar to use.
			RacecarUpdate,
			MultiCarUpdate,

			TimingReset,                 //Sent via a TinyPacket from GameServer to client over SafeConnection to indicate the competition is being restarted.
			//AutocrossUpdate,           //Sent from GameServer to client to update the staging queue, onDeck status etc.
			TimingResult,                //Sent via a TinyPacket from client or ResultPacket from GameServer over SafeConnection.

			GameUpdate,

			PingRequest,                 //Can be sent without userid.
			PingResponse,                //Send by client with valid userid, or freely by GameServer.
			PingSyncReady,               //Sent via a TinyPacket by the GameServer to tell the client that latency check is ready to play.

			//DeveloperCommand,

			UnknownPacket = 0xFF,
		};

		enum class DisconnectReason : byte
		{
			Graceful,
			VersionMismatch,
			ConnectionMismatch,
			ServerFull,
			Timeout,
			PingTimeout,
			UnregisteredTimeout,
			Kicked,
			Banned,
			ServerShutdown,
			UnknownPacket,
			InvalidInformation,
		};

		enum PingFlags : tbCore::byte
		{
			ConnectionTCP = 0,
			ConnectionUDP = 0x01,
			UnusedBit = 0x02,
			UnusedBit2 = 0x04
			//No other bits allowed without changing PingPacket
		};

		enum class AuthenticationService : byte
		{
			Unknown,
			Twitch,
			Patreon,
			YouTube,
			Developer,
		};

		tbCore::tbString ToString(const PacketType& packetType);
		std::ostream& operator<<(std::ostream& output, const PacketType& packetType);

		tbCore::tbString ToString(const DisconnectReason& reason);
		std::ostream& operator<<(std::ostream& output, const DisconnectReason& reason);

		tbCore::tbString ToString(const AuthenticationService& authService);
		std::ostream& operator<<(std::ostream& output, const AuthenticationService& authService);

		template <typename Type> const Type& ToPacket(const byte* packetData)
		{
			return *(reinterpret_cast<const Type*>(packetData));
		}

		template <typename Type> const Type& ToPacket(const byte* packetData, const size_t packetSize)
		{
			const PacketType packetType = (packetSize >= 2) ? static_cast<PacketType>(packetData[1]) : PacketType::UnknownPacket;
			tb_error_if(packetSize != sizeof(Type), "Packet size(%d) is too small to cast into Type: %s",
				packetSize, ToString(packetType).c_str());
			return *(reinterpret_cast<const Type*>(packetData));
		}

		template <typename Type> const tbCore::byte* ToData(const Type& packet)
		{
			return reinterpret_cast<const tbCore::byte*>(&packet);
		}

		/// @param reason Would typically be a string of "Recieving" or "Sending" or "Handling" etc. A string like
		///   " a packet of type" will be appended along with the type and subtype.
		void TracePacket(const tbCore::tbString& reason, const byte* packetData, const size_t packetSize, const tbCore::tbString& appendString = "");

		tb_static_error_if(sizeof(PacketSize) != 1, "Error: Expected PacketSize to be 1 byte.");
		tb_static_error_if(sizeof(PacketType) != 1, "Error: Expected PacketType size to be 1 byte.");

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

		//All packets in this should be packed tightly, without padding.
		//From StackOverflow the following works on vc++ and gcc...
		//https://stackoverflow.com/questions/21092415/force-c-structure-to-pack-tightly
#pragma pack(push, 1)

		struct TinyPacket
		{
			PacketSize size;
			PacketType type; //Tiny
			byte subtype;
			byte data;
		};

		struct SmallPacket
		{
			PacketSize size;
			PacketType type; //Small
			byte subtype;
			byte data;
			tbCore::uint32 payload;
		};

		struct LargePayloadPacket
		{
			static const size_t kPayloadSize = 248;

			PacketSize size;
			PacketType type; //LargeDataPayload
			byte subtype; //the actual large packet to handle.
			byte finished; //0 not finished, 1 finished
			//If there are ever more than 4 bytes above payload, need to handle that in the PayloadHandler::Append
			byte payload[kPayloadSize];
		};

		struct PingPacket
		{
			PacketSize size;
			PacketType type;
			byte pingid : 5;
			byte flags : 3; //See PingFlags
			byte padding;
			tbCore::uint32 time;
		};

		struct JoinRequestPacket
		{
			PacketSize size;
			PacketType type; //JoinRequest
			byte major;
			byte minor;
			byte patch;
			byte packetVersion;
			byte padding[2];
		};

		struct AuthenticationPacket
		{	//NOTE: MUST be sent as a LargePayloadPacket, the size is too large to fit in the typical size byte.
			AuthenticationService service;
			byte type; //AuthenticateRequest
			byte pad1; //pad could become u16 size of userKey.
			byte pad2;
			//tbCore::FixedString<1024> userKey;
			tbCore::FixedString<4096> userKey; //YouTube key was bigboi
		};

		struct StartGridPacket
		{
			PacketSize size;
			PacketType type;
			GameState::GridIndex grid[GameState::kNumberOfRacecars];
		};

		struct DriverJoinedPacket
		{
			PacketSize size;
			PacketType type;
			byte driverIndex;
			AuthenticationService service;
			tbCore::FixedString<128> license; // @note 2023-11-07: Probably overkill, but we can reduce once Tyre Bytes accounts are a thing.
			tbCore::FixedString<20> name;
			bool isModerator;
			byte pad1;
			byte pad2;
			byte pad3;

		};

		struct DriverEntersRacecarPacket
		{
			PacketSize size;
			PacketType type; //DriverEntersRacecar
			DriverIndex driverIndex;
			RacecarIndex racecarIndex;

			float rotation[4]; //quat(x,y,z,w)
			float position[3]; //pos(x, y, z)
			byte carID; //car, skin, etc all in one
		};

		struct RacetrackResponsePacket
		{
			PacketSize size;
			PacketType type;
			byte phase;
			byte loadingTag;
			tbCore::FixedString<32> racetrack;
			tbCore::uint32 phaseTimer;
		};

		struct ControllerInfo
		{	//Size = 8bytes, 1 of which are padding
			tbCore::uint16 steering;
			tbCore::uint16 throttle;
			tbCore::uint16 braking;
			byte buttons;
			byte padding;
		};

		struct RacecarInfo
		{
			//TODO: LudumDare56: Optimization: There are more bits being transfered here than are strictly necessary. We
			//   could send only X, Y, Z of the quaternion, and get away with 16-bit x,y,z. With smaller racetrack sizes and limited
			//   speeds we could also reduce the other vectors as well.
			//
			//   atomicnibble: one thing to keep in mind if you do drop the W you need to handle negatives, I just invert the
			//      whole quat if w < 0 on the sending side.
			float rotation[4]; //quat(x,y,z,w)
			//float rotation[3]; //quat(x,y,z) compute w on other side
			float position[3]; //pos(x, y, z)
			float linearVelocity[3];
			float angularVelocity[3];
			ControllerInfo controller;
			RacecarIndex racecarIndex;
		};

		struct RacecarUpdatePacket
		{
			PacketSize size;
			PacketType type; //RacecarUpdate
			tbCore::uint32 time;
			RacecarInfo carInfo;
		};

		struct RacecarRequestPacket
		{
			PacketSize size;
			PacketType type; //RacecarRequest
			DriverIndex driverIndex;
			byte carID; //car, skin, etc all in one
		};

		struct DeveloperCommandPacket
		{	//NOTE: MUST be sent as a LargePayloadPacket, the size is too large to fit in the typical size byte.
			byte unused;
			byte type; //DeveloperCommand
			byte pad1; //pad could become u16 size of command
			byte pad2;
			tbCore::FixedString<1024> command;
		};

		struct TimingUpdatePacket
		{
			PacketSize size;
			PacketType type; //AutocrossUpdate
			RacecarIndex onDeckRacecar;
			byte onDeckState;
			tbCore::uint32 time;
			std::array<RacecarIndex, GameState::kNumberOfRacecars> stagingQueue;
		};

		struct TimingResultPacket
		{
			PacketSize size;
			PacketType type; //TimingResult
			tbCore::FixedString<36> driverLicense;
			tbCore::FixedString<20> driverName;
			tbCore::uint32 lapTime;
			tbCore::uint8 lapNumber;
		};
#pragma pack(pop)

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

		///
		/// @details Returns the PacketType, or SubPacketType from the packetData supplied, or unknown if not enough data.
		///
		PacketType GetPacketTypeFrom(const tbCore::byte* packetData, const size_t packetSize);

		TinyPacket CreateTinyPacket(PacketType subtype, byte data = 0);
		SmallPacket CreateSmallPacket(PacketType subtype, tbCore::uint32 payload, byte data = 0);
		PingPacket CreatePingPacket(const tbCore::uint32& time, const tbCore::byte& pingid, const ConnectionType connectionType);
		JoinRequestPacket CreateJoinRequestPacket(void);
		AuthenticationPacket CreateAuthenticationRequest(const tbCore::tbString& userKey, AuthenticationService service);
		DriverJoinedPacket CreateDriverJoinedPacket(const DriverIndex driverIndex);
		DriverEntersRacecarPacket CreateDriverEntersRacecarPacket(const GameState::RacecarState& racecar);

		RacecarRequestPacket CreateRacecarRequest(const DriverIndex driverIndex, byte carID);
		RacecarUpdatePacket CreateRacecarUpdatePacket(const RacecarIndex racecarIndex, tbCore::uint32 worldTime);
		void HandleUpdatePacket(const RacecarInfo& racecarInfo, tbCore::uint32 worldTime);

		RacetrackResponsePacket CreateRacetrackResponse(tbCore::byte loadingTag);

		TimingResultPacket CreateTimingResult(const GameState::Events::TimingEvent& lapResultEvent);

		//DeveloperCommandPacket CreateDeveloperCommandPacket(const tbCore::tbString& fullCommandString);

	};	//namespace Network
};	//namespace LudumDare56

//--------------------------------------------------------------------------------------------------------------------//

#endif /* LudumDare56_NetworkPackets_hpp */
