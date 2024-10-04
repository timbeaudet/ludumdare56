///
/// @file
/// @details Defines the packets that will get sent for the multiplayer component of LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../network/network_packets.hpp"
#include "../network/networked_racecar_controller.hpp"
#include "../game_state/race_session_state.hpp"
#include "../game_state/racecar_state.hpp"
#include "../game_state/driver_state.hpp"
#include "../game_state/racetrack_state.hpp"
#include "../game_state/events/timing_events.hpp"

#include "../logging.hpp"
#include "../version.hpp"

#include <turtle_brains/math/tb_quaternion.hpp>
#include <turtle_brains/math/tb_matrix_quaternion.hpp>

#include <algorithm>

//--------------------------------------------------------------------------------------------------------------------//

using LudumDare56::Network::PacketType;
using LudumDare56::Network::DisconnectReason;

tbCore::tbString LudumDare56::Network::ToString(const PacketType& packetType)
{
	switch (packetType)
	{
	case PacketType::Tiny: return "TinyPacket";
	case PacketType::Small: return "SmallPacket";
	case PacketType::LargePayload: return "LargePayload";

	case PacketType::JoinRequest: return "JoinRequest";
	case PacketType::JoinResponse: return "JoinResponse";
	case PacketType::NetworkSettings: return "NetworkSettings";
	case PacketType::Disconnect: return "Disconnect";

	case PacketType::AuthenticateRequest: return "AuthenticateRequest";
	case PacketType::AuthenticateResponse: return "AuthenticateResponse";

	case PacketType::RegistrationStartRequest: return "RegistrationStartRequest";
	case PacketType::RegistrationStartResponse: return "RegistrationStartResponse";
	case PacketType::RegistrationRequest: return "RegistrationRequest";
	case PacketType::RegistrationResponse: return "RegistrationResponse";

	case PacketType::PhaseChanged: return "PhaseChanged";

	case PacketType::RaceSessionTimer: return "RaceSessionTimer";
	case PacketType::StartGrid: return "StartGrid";

	case PacketType::RacetrackRequest: return "RacetrackRequest";
	case PacketType::RacetrackResponse: return "RacetrackResponse";
	case PacketType::RacetrackLoaded: return "RacetrackLoaded";

	case PacketType::DriverJoined: return "DriverJoined";
	case PacketType::DriverLeft: return "DriverLeft";
	case PacketType::DriverEntersRacecar: return "DriverEntersRacecar";
	case PacketType::DriverLeavesRacecar: return "DriverLeavesRacecar";

	//case PacketType::RacecarSelect: return "RacecarSelect";
	case PacketType::RacecarRequest: return "RacecarRequest";
	case PacketType::RacecarReset: return "RacecarReset";
	case PacketType::RacecarUpdate: return "RacecarUpdate";
	case PacketType::MultiCarUpdate: return "MultiRacecarUpdate";

	case PacketType::TimingReset: return "TimingReset";
	case PacketType::TimingResult: return "TimingResult";
	//case PacketType::AutocrossReset: return "AutocrossReset";
	//case PacketType::AutocrossUpdate: return "AutocrossUpdate";
	//case PacketType::AutocrossResult: return "AutocrossResult";

	case PacketType::GameUpdate: return "GameUpdate";

	case PacketType::PingRequest: return "PingRequest";
	case PacketType::PingResponse: return "PingResponse";
	case PacketType::PingSyncReady: return "PingSyncReady";

	//case PacketType::DeveloperCommand: return "DeveloperCommand";

	case PacketType::UnknownPacket: break;
	};

	return "Unknown PacketType";
}

std::ostream& LudumDare56::Network::operator<<(std::ostream& output, const PacketType& packetType)
{
	output << ToString(packetType);
	return output;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString LudumDare56::Network::ToString(const DisconnectReason& reason)
{
	switch (reason)
	{
	case DisconnectReason::Graceful: return "Graceful";
	case DisconnectReason::VersionMismatch: return "VersionMismatch";
	case DisconnectReason::ConnectionMismatch: return "ConnectionMismatch";
	case DisconnectReason::ServerFull: return "ServerFull";
	case DisconnectReason::Timeout: return "Timeout";
	case DisconnectReason::PingTimeout: return "PingTimeout";
	case DisconnectReason::UnregisteredTimeout: return "UnregisteredTimeout";
	case DisconnectReason::Kicked: return "Kicked";
	case DisconnectReason::Banned: return "Banned";
	case DisconnectReason::ServerShutdown: return "ServerShutdown";
	case DisconnectReason::UnknownPacket: return "UnknownPacket";
	case DisconnectReason::InvalidInformation: return "InvalidInformation";
	};

	return "Unknown";
}

std::ostream& LudumDare56::Network::operator<<(std::ostream& output, const DisconnectReason& reason)
{
	output << ToString(reason);
	return output;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString LudumDare56::Network::ToString(const AuthenticationService& authService)
{
	switch (authService)
	{
	case AuthenticationService::Unknown: return "Unknown";
	case AuthenticationService::Developer: return "Developer";
	case AuthenticationService::Patreon: return "Patreon";
	case AuthenticationService::Twitch: return "Twitch";
	case AuthenticationService::YouTube: return "YouTube";
	};

	return "ERROR: Unknown";
}

std::ostream& LudumDare56::Network::operator<<(std::ostream& output, const AuthenticationService& authService)
{
	output << ToString(authService);
	return output;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::TracePacket(const tbCore::tbString& reason, const byte* packetData, const size_t packetSize,
	const tbCore::tbString& appendString)
{
	PacketType packetType = (packetSize >= 2) ? static_cast<PacketType>(packetData[1]) : PacketType::UnknownPacket;
	switch (packetType)
	{
	case PacketType::PingRequest:
	case PacketType::PingResponse:
	case PacketType::RacecarUpdate:
		return;
	default: break;
	};

	//The above will return early if the Packet should be untracable.

	const tbCore::tbString spaceOrNot = (false == appendString.empty()) ? " " : "";

	switch (packetType)
	{
	case PacketType::Tiny: {
		const TinyPacket& tinyPacket = ToPacket<TinyPacket>(packetData, packetSize);
		const PacketType packetSubType = static_cast<PacketType>(tinyPacket.subtype);
		tb_debug_log(LogNetwork::Trace() << reason << " " << packetType << "( " << packetSubType << " )" << spaceOrNot << appendString);
		break; }
	case PacketType::Small: {
		const SmallPacket& smallPacket = ToPacket<SmallPacket>(packetData, packetSize);
		const PacketType packetSubType = static_cast<PacketType>(smallPacket.subtype);
		tb_debug_log(LogNetwork::Trace() << reason << " " << packetType << "( " << packetSubType << " )" << spaceOrNot << appendString);
		break; }
	case PacketType::LargePayload: {
		const LargePayloadPacket& largePacket = ToPacket<LargePayloadPacket>(packetData);
		const PacketType packetSubType = static_cast<PacketType>(largePacket.subtype);
		tb_debug_log(LogNetwork::Trace() << reason << " " << packetType << "( " << packetSubType << " )" << spaceOrNot << appendString);
		break; }
	default:
		tb_debug_log(LogNetwork::Trace() << reason << " " << packetType << spaceOrNot << appendString);
		break;
	};
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

PacketType LudumDare56::Network::GetPacketTypeFrom(const tbCore::byte* packetData, const size_t packetSize)
{
	PacketType packetType = (packetSize >= 2) ? static_cast<PacketType>(packetData[1]) : PacketType::UnknownPacket;
	if (PacketType::Tiny == packetType || PacketType::Small == packetType || PacketType::LargePayload == packetType)
	{
		packetType = (packetSize >= 3) ? static_cast<PacketType>(packetData[2]) : packetType;
	}

	return packetType;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::TinyPacket LudumDare56::Network::CreateTinyPacket(PacketType subtype, byte data)
{
	TinyPacket packet;
	packet.size = sizeof(TinyPacket);
	packet.type = PacketType::Tiny;
	packet.subtype = static_cast<tbCore::byte>(subtype);
	packet.data = data;
	return packet;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::SmallPacket LudumDare56::Network::CreateSmallPacket(PacketType subtype, tbCore::uint32 payload, byte data)
{
	SmallPacket packet;
	packet.size = sizeof(SmallPacket);
	packet.type = PacketType::Small;
	packet.subtype = static_cast<tbCore::byte>(subtype);
	packet.data = data;
	packet.payload = payload;
	return packet;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::PingPacket LudumDare56::Network::CreatePingPacket(const tbCore::uint32& time,
	const tbCore::byte& pingid, const ConnectionType connectionType)
{
	tb_error_if(pingid >= 32, "Invalid pingid specified for PingPacket.");

	PingPacket ping;
	ping.size = sizeof(ping);
	ping.type = PacketType::PingRequest;
	ping.pingid = pingid;
	ping.flags = (ConnectionType::Fast == connectionType) ? PingFlags::ConnectionUDP : PingFlags::ConnectionTCP;
	ping.time = time;
	ping.padding = 0;
	return ping;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::JoinRequestPacket LudumDare56::Network::CreateJoinRequestPacket(void)
{
	static_assert(Version::Major() < std::numeric_limits<byte>::max(), "Version major is too large to fit in a byte.");
	static_assert(Version::Minor() < std::numeric_limits<byte>::max(), "Version minor is too large to fit in a byte.");
	static_assert(Version::Patch() < std::numeric_limits<byte>::max(), "Version patch is too large to fit in a byte.");

	JoinRequestPacket packet;
	packet.size = sizeof(packet);
	packet.type = PacketType::JoinRequest;
	packet.major = tbCore::RangedCast<tbCore::byte>(Version::Major());
	packet.minor = tbCore::RangedCast<tbCore::byte>(Version::Minor());
	packet.patch = tbCore::RangedCast<tbCore::byte>(Version::Patch());
	packet.packetVersion = PacketVersion();
	packet.padding[0] = packet.padding[1] = 0;
	return packet;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::AuthenticationPacket LudumDare56::Network::CreateAuthenticationRequest(
	const tbCore::tbString& userKey, AuthenticationService service)
{
	AuthenticationPacket packet;
	tb_error_if(userKey.size() >= packet.userKey.FixedSize(), "The userKey is too large to fit in the authentication packet.");

	packet.service = service;
	packet.type = static_cast<byte>(PacketType::AuthenticateRequest);
	packet.pad1 = packet.pad2 = 0;
	packet.userKey = userKey;
	return packet;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::DriverJoinedPacket LudumDare56::Network::CreateDriverJoinedPacket(const DriverIndex driverIndex)
{
	const GameState::DriverState& driver = GameState::DriverState::Get(driverIndex);
	const tbCore::tbString& driverLicense = driver.GetLicense();
	const tbCore::tbString& driverName = driver.GetName();

	DriverJoinedPacket packet;
	tb_error_if(driverLicense.size() >= packet.license.FixedSize(), "The driverLicense is too large for the join packet.");
	tb_error_if(driverName.size() >= packet.name.FixedSize(), "The driverName is too large for the join packet.");

	packet.size = sizeof(packet);
	packet.type = PacketType::DriverJoined;
	packet.driverIndex = driverIndex;
	packet.service = AuthenticationService::Unknown;
	packet.license = driverLicense;
	packet.name = driverName;
	packet.isModerator = driver.IsModerator();
	packet.pad1 = packet.pad2 = packet.pad3 = 0;
	return packet;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::DriverEntersRacecarPacket LudumDare56::Network::CreateDriverEntersRacecarPacket(const GameState::RacecarState& racecar)
{
	DriverEntersRacecarPacket packet;
	packet.size = sizeof(packet);
	packet.type = PacketType::DriverEntersRacecar;
	packet.driverIndex = racecar.GetDriverIndex();
	packet.racecarIndex = racecar.GetRacecarIndex();
	packet.carID = racecar.GetRacecarMeshID();

	//I believe we could send quat X, Y, Z under certain conditions... But for now (2021-09-18) I am going with what
	//worked in old Rally of Rockets.
	const tbMath::Matrix4 vehicleToWorld = static_cast<tbMath::Matrix4>(racecar.GetVehicleToWorld());
	const tbMath::Vector3& position = vehicleToWorld.GetPosition();
	const tbMath::Quaternion rotation = tbMath::Quaternion::FromMatrix(vehicleToWorld);
	std::copy_n(position.mComponents, 3, packet.position);
	std::copy_n(rotation.mComponents, 4, packet.rotation);

	return packet;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::RacecarRequestPacket LudumDare56::Network::CreateRacecarRequest(const DriverIndex driverIndex, byte carID)
{
	RacecarRequestPacket packet;
	packet.size = sizeof(packet);
	packet.type = PacketType::RacecarRequest;
	packet.driverIndex = driverIndex;
	packet.carID = carID;
	return packet;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::RacetrackResponsePacket LudumDare56::Network::CreateRacetrackResponse(tbCore::byte loadingTag)
{
	RacetrackResponsePacket packet;
	packet.size = sizeof(RacetrackResponsePacket);
	packet.type = PacketType::RacetrackResponse;
	packet.phase = static_cast<byte>(GameState::RaceSessionState::GetSessionPhase());
	packet.phaseTimer = GameState::RaceSessionState::GetPhaseTimer();
	packet.loadingTag = loadingTag;

	tbCore::tbString racetrackName = GameState::RacetrackState::GetCurrentRacetrack();
	{	//Remove the path, and extension from the racetrack name.
		const size_t lastSlashIndex = racetrackName.find_last_of('/');
		if (tbCore::tbString::npos != lastSlashIndex)
		{
			racetrackName = racetrackName.substr(lastSlashIndex + 1);
		}
		racetrackName = racetrackName.substr(0, racetrackName.find('.'));
	}

	packet.racetrack = racetrackName;
	return packet;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::ControllerInfo ControllerToInfo(const LudumDare56::GameState::RacecarControllerInterface& controller)
{
	LudumDare56::Network::ControllerInfo controllerInfo;
	controllerInfo.steering = controller.GetSteeringValue();
	controllerInfo.throttle = controller.GetThrottleValue();
	controllerInfo.braking = controller.GetBrakeValue();
	controllerInfo.buttons = 0;

	//if (controller.IsHandbrakeDown()) { controllerInfo.buttons |= 1; }
	//if (controller.IsLaunchDown()) { controllerInfo.buttons |= 2; }
	//if (controller.IsResetDown()) { controllerInfo.buttons |= 4; }
	//if (controller.IsBoostDown()) { controllerInfo.buttons |= 8; }

	controllerInfo.padding = 0;
	return controllerInfo;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::RacecarInfo RacecarToInfo(const LudumDare56::Network::RacecarIndex racecarIndex)
{
	const LudumDare56::GameState::RacecarState& racecar = LudumDare56::GameState::RacecarState::Get(racecarIndex);

	LudumDare56::Network::RacecarInfo info;
	info.racecarIndex = racecarIndex;
	info.controller = ControllerToInfo(racecar.GetRacecarController());

	const tbMath::Matrix4 vehicleToWorld = static_cast<tbMath::Matrix4>(racecar.GetVehicleToWorld());
	const tbMath::Quaternion rotation = tbMath::Quaternion::FromMatrix(vehicleToWorld);
	const tbMath::Vector3& position = racecar.GetVehicleToWorld().GetPosition();
	const tbMath::Vector3 linearVelocity = tbMath::Vector3(racecar.GetLinearVelocity());
	const tbMath::Vector3 angulateVelocity = tbMath::Vector3(racecar.GetAngularVelocity());

	//Source, ElementCount, Destination...
	std::copy_n(rotation.mComponents, 4, info.rotation);
	std::copy_n(position.mComponents, 3, info.position);
	std::copy_n(linearVelocity.mComponents, 3, info.linearVelocity);
	std::copy_n(angulateVelocity.mComponents, 3, info.angularVelocity);

	return info;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::RacecarUpdatePacket LudumDare56::Network::CreateRacecarUpdatePacket(
	const RacecarIndex racecarIndex, tbCore::uint32 worldTime)
{
	RacecarUpdatePacket packet;
	packet.size = sizeof(RacecarUpdatePacket);
	packet.type = PacketType::RacecarUpdate;
	packet.time = worldTime;
	packet.carInfo = RacecarToInfo(racecarIndex);
	return packet;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::Network::TimingResultPacket LudumDare56::Network::CreateTimingResult(const GameState::Events::TimingEvent& lapResultEvent)
{
	TimingResultPacket packet;
	packet.size = sizeof(TimingResultPacket);
	packet.type = PacketType::TimingResult;
	packet.driverLicense = lapResultEvent.mDriverLicense;
	packet.driverName = lapResultEvent.mDriverName;
	packet.lapTime = lapResultEvent.mLapTime;
	packet.lapNumber = lapResultEvent.mLapNumber;
	return packet;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::Network::HandleUpdatePacket(const RacecarInfo& racecarInfo, tbCore::uint32 /*worldTime*/)
{
	GameState::RacecarState& racecar = GameState::RacecarState::GetMutable(racecarInfo.racecarIndex);
	racecar.GetMutablePhysicsModel().ResetRacecarForces();

	const tbMath::Quaternion rotation = static_cast<tbMath::Quaternion>(racecarInfo.rotation);
	const tbMath::Vector3 position = static_cast<tbMath::Vector3>(racecarInfo.position);

	//racecar.ResetRacecar(icePhysics::Matrix4(tbMath::Matrix4::FromQuaternion(rotation, position)));
	racecar.SetVehicleToWorld(icePhysics::Matrix4(tbMath::Matrix4::FromQuaternion(rotation, position)));
	racecar.SetLinearVelocity(icePhysics::Vector3(
		icePhysics::Scalar(racecarInfo.linearVelocity[0]),
		icePhysics::Scalar(racecarInfo.linearVelocity[1]),
		icePhysics::Scalar(racecarInfo.linearVelocity[2])));
	racecar.SetAngularVelocity(icePhysics::Vector3(
		icePhysics::Scalar(racecarInfo.angularVelocity[0]),
		icePhysics::Scalar(racecarInfo.angularVelocity[1]),
		icePhysics::Scalar(racecarInfo.angularVelocity[2])));

	NetworkedRacecarController* racecarController = dynamic_cast<NetworkedRacecarController*>(&racecar.GetMutableRacecarController());
	if (nullptr != racecarController)
	{
		racecarController->SetControllerInformation(racecarInfo.controller);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

//LudumDare56::Network::DeveloperCommandPacket LudumDare56::Network::CreateDeveloperCommandPacket(const tbCore::tbString& fullCommandString)
//{
//	DeveloperCommandPacket developerPacket;
//	developerPacket.type = static_cast<byte>(PacketType::DeveloperCommand);
//	developerPacket.command = fullCommandString;
//	developerPacket.pad1 = 0;
//	developerPacket.pad2 = 0;
//	developerPacket.unused = 0;
//	return developerPacket;
//}

//--------------------------------------------------------------------------------------------------------------------//
