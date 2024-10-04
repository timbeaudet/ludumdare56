///
/// @file
/// @details Opens a TCP connection to send data to things that attempt to connect to us.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../core/services/web_server.hpp"
#include "../../core/utilities.hpp"

#include <turtle_brains/core/tb_dynamic_structure.hpp>
#include <turtle_brains/core/tb_json_parser.hpp>
#include <turtle_brains/core/debug/tb_debug_logger.hpp>
#include <turtle_brains/network/tb_socket_connection.hpp>
#include <turtle_brains/network/tb_packet_handler_interface.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>

using tbCore::Debug::LogNetwork;

//
// Note: As of 2021-01-10 the WebSocketConnection is not a thing. It would be a nice way to communicate with the
//   web-overlay however as a WebSocket Server we would need to do some SHA1 stuff and libressl sha stuff didn't
//   just work out. It may also be preferrable to upgrade the http connection into a websocket connection on a per
//   client basis, though that would get to be a little fun.
//
// For now all communication comes through http get requests.
//

namespace TyreBytes
{
	namespace Core
	{
		namespace Services
		{
			namespace WebServer
			{
				namespace Implementation
				{

					const static tbCore::byte kInvalidClient = tbCore::byte(0xFF); //implementation detail that shouldn't be: tbNetwork::kInvalidClientIdentifier(0xFF);

					const tbCore::tbString kEmptyString("");

					const float kTimeoutDisconnectClient = 120.0f; //in seconds.
					const float kTimeToPingClients = 45.0f; //in seconds.

					class httpRequest;

					class HttpServerHandler : public tbNetwork::PacketHandlerInterface
					{
					public:
						static tbCore::byte sHandlingClientID;
						static bool sIsHandlingRequest;
						static tbNetwork::SocketConnection* sHandlingConnection;

						HttpServerHandler(void);
						~HttpServerHandler(void);

						bool IsDisconnectRequired(void) const { return mDisconnectRequired; }
						void Update(const float deltaTime);

						void SetHttpRequestHandler(WebServer::httpRequestHandlerInterface* httpHandler)
						{
							mHttpRequestHandler = httpHandler;
						}

						void SetHttpConnection(tbNetwork::SocketConnection* connection)
						{
							mConnection = connection;
						}

						struct ClientData
						{
							tbCore::tbString mIncomingMessageBuffer;

							ClientData(void) :
								mIncomingMessageBuffer()
							{
							}
						};

						typedef std::unordered_map<tbCore::byte, ClientData> ClientMap;
						ClientMap mClients;

					protected:
						virtual void OnConnect(void) override;
						virtual void OnDisconnect(void) override;
						virtual void OnConnectClient(tbCore::byte clientID) override;
						virtual void OnDisconnectClient(tbCore::byte clientID) override;
						virtual bool OnHandlePacket(const tbCore::byte* packetData, size_t packetSize, tbCore::byte from) override;

					private:
						WebServer::httpRequestHandlerInterface* mHttpRequestHandler;
						tbNetwork::SocketConnection* mConnection;

						float mLastPingTimer;
						bool mDisconnectRequired; //2021-04-16: This didn't seem to be set to true anywhere, it may or may not need to go into ClientData? or is it server disconnect req?
					};

					class HttpServerData
					{
					public:
						HttpServerHandler theHttpHandler;
						std::unique_ptr<tbNetwork::SocketConnection> theWebSocketConnection = (nullptr);
					};

					//HttpServerHandler theHttpHandler;
					//std::unique_ptr<tbNetwork::SocketConnection> theWebSocketConnection(nullptr);

					//tbCore::byte WebServerImplementation::HttpServerHandler::sHandlingClientID = 0; //Should this not be Invalid
					//bool WebServerImplementation::HttpServerHandler::sIsHandlingRequest = false;

				};	//namespace Implementation
			};	//namespace WebServer
		};	//namespace Services
	};	//namespace Core
};	//namespace TyreBytes

tbCore::byte TyreBytes::Core::Services::WebServer::Implementation::HttpServerHandler::sHandlingClientID = 0; //Should this not be Invalid
bool TyreBytes::Core::Services::WebServer::Implementation::HttpServerHandler::sIsHandlingRequest = false;
tbNetwork::SocketConnection* TyreBytes::Core::Services::WebServer::Implementation::HttpServerHandler::sHandlingConnection = nullptr;

//using namespace Implementation;

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::WebServer::httpServer::httpServer(void) :
	mData(new Implementation::HttpServerData)
{
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::WebServer::httpServer::~httpServer(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Services::WebServer::httpServer::Connect(tbCore::uint16 port, httpRequestHandlerInterface& httpHandler)
{
	mData->theHttpHandler.SetHttpRequestHandler(&httpHandler);
	mData->theWebSocketConnection.reset(new tbNetwork::SocketConnection(tbNetwork::SocketConnectionType::ServerStreamTCP));

	mData->theHttpHandler.SetHttpConnection(mData->theWebSocketConnection.get());
	if (true == mData->theWebSocketConnection->Connect("", port, mData->theHttpHandler))
	{
		tb_debug_log(LogNetwork::Info() << "[WebServer] Connected and listening for clients.");
		return true;
	}

	tb_always_log(LogNetwork::Error() << "[WebServer] Failed to connect or listen for clients.");
	return false;
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Services::WebServer::httpServer::Disconnect(void)
{
	mData->theHttpHandler.SetHttpConnection(nullptr);
	mData->theHttpHandler.SetHttpRequestHandler(nullptr);
	mData->theWebSocketConnection->Disconnect();
	mData->theWebSocketConnection = nullptr;
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Services::WebServer::httpServer::IsConnected(void)
{
	return nullptr != mData->theWebSocketConnection && mData->theWebSocketConnection->IsConnected();
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Services::WebServer::httpServer::Update(const float deltaTime)
{
	if (nullptr != mData->theWebSocketConnection)
	{
		mData->theHttpHandler.Update(deltaTime);
	}
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::WebServer::Implementation::HttpServerHandler::HttpServerHandler(void) :
	mClients(),
	mHttpRequestHandler(nullptr),
	mConnection(nullptr),
	mLastPingTimer(0.0f),
	mDisconnectRequired(false)
{
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::WebServer::Implementation::HttpServerHandler::~HttpServerHandler(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Services::WebServer::Implementation::HttpServerHandler::Update(const float /*deltaTime*/)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Services::WebServer::Implementation::HttpServerHandler::OnConnect(void)
{
	mDisconnectRequired = false;
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Services::WebServer::Implementation::HttpServerHandler::OnDisconnect(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Services::WebServer::Implementation::HttpServerHandler::OnConnectClient(tbCore::byte clientID)
{
	auto& client = mClients[clientID];
	client.mIncomingMessageBuffer = "";
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Services::WebServer::Implementation::HttpServerHandler::OnDisconnectClient(tbCore::byte clientID)
{
	mClients.erase(clientID);

	if (sHandlingClientID == clientID)
	{	//Do not handle this client any further.
		sIsHandlingRequest = false;
		sHandlingClientID = kInvalidClient;
		sHandlingConnection = nullptr;
	}
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString ToBase64(const tbCore::byte* data, const size_t size) //EncodeBase64
{	//https://tools.ietf.org/html/rfc4648#page-5
	//If needed american passed this for extra help: https://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp/index

	const tbCore::tbString alphabet64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	const char kPaddingCharacter = '=';

	tbCore::tbString base64;

	tbCore::uint8 filledBits = 0;
	tbCore::uint32 bits = 0;

	for (size_t letter = 0; letter < size; ++letter)
	{
		bits = bits << 8;
		bits |= data[letter];
		filledBits += 8;
		if (filledBits == 24)
		{
			tbCore::uint8 processed[4] = {
				//(filledBits & 0x00FC0000) >> 18
				static_cast<tbCore::uint8>(bits >> 18),
				static_cast<tbCore::uint8>((bits >> 12) & 0x3F),
				static_cast<tbCore::uint8>((bits >> 6) & 0x3F),
				static_cast<tbCore::uint8>((bits) & 0x3F),
			};

			base64 += alphabet64[processed[0]];
			base64 += alphabet64[processed[1]];
			base64 += alphabet64[processed[2]];
			base64 += alphabet64[processed[3]];

			filledBits = 0;
			bits = 0;
		}
	}

	if (filledBits != 0)
	{
		while (0 != filledBits % 6)
		{
			bits = bits << 1;
			++filledBits;
		}

		if (12 == filledBits)
		{
			tbCore::uint8 processed[4] = {
				//(filledBits & 0x00FC0000) >> 18
				static_cast<tbCore::uint8>((bits >> 6) & 0x3F),
				static_cast<tbCore::uint8>((bits) & 0x3F),
			};

			base64 += alphabet64[processed[0]];
			base64 += alphabet64[processed[1]];
			base64 += kPaddingCharacter;
			base64 += kPaddingCharacter;
		}
		else if (18 == filledBits)
		{
			tbCore::uint8 processed[4] = {
				//(filledBits & 0x00FC0000) >> 18
				static_cast<tbCore::uint8>((bits >> 12) & 0x3F),
				static_cast<tbCore::uint8>((bits >> 6) & 0x3F),
				static_cast<tbCore::uint8>((bits) & 0x3F),
			};

			base64 += alphabet64[processed[0]];
			base64 += alphabet64[processed[1]];
			base64 += alphabet64[processed[2]];
			base64 += kPaddingCharacter;
		}
	}

	return base64;
}

tbCore::tbString ToBase64(const tbCore::tbString& input) //EncodeBase64
{
	return ToBase64(reinterpret_cast<const tbCore::byte*>(input.c_str()), input.size());
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Services::WebServer::Implementation::HttpServerHandler::OnHandlePacket(const tbCore::byte* packetData, size_t packetSize, tbCore::byte from)
{
	tb_always_log(LogNetwork::Debug() << "Handling Packet of size " << packetSize << " from client(" << int(from) << ")");
	//if (tbCore::Debug::TheLogger().GetLogLevel(LogNetwork::AsString()) >= tbCore::Debug::kTrace)
	//{
	//	tb_always_hexdump("Contents: ", packetData, packetSize);
	//}

	auto& client = mClients[from];

	//Note: This may be set to false before the bottom of the function if the client gets disconnected part way through.
	sIsHandlingRequest = true;
	sHandlingClientID = from;
	sHandlingConnection = mConnection;

	for (size_t index(0); index < packetSize; ++index)
	{
		const char current = static_cast<char>(packetData[index]);
		if (current != '\0') //just skip over null-terms and \r's
		{
			client.mIncomingMessageBuffer += current;
			if (current == '\n' && tbCore::tbString::npos != client.mIncomingMessageBuffer.find("\r\n\r\n"))
			{
				httpHeader header(client.mIncomingMessageBuffer);
				client.mIncomingMessageBuffer = "";

				WebServer::httpRequest request(header.GetHeaderLine());
				if (false == mHttpRequestHandler->OnHandleRequest(request))
				{
					const tbCore::tbString fullString = "HTTP/1.1 404 NOT FOUND\r\nAccess-Control-Allow-Origin: *\r\n\r\n";
					mConnection->SendPacketTo(reinterpret_cast<const tbCore::byte*>(fullString.c_str()), fullString.size(), from);
				}

				mConnection->DisconnectClient(from);
				tb_error_if(true == sIsHandlingRequest || from == sHandlingClientID || nullptr != sHandlingConnection,
					"Expected DisconnectClient() to turn off handling request from.");
			}
		}
	}

	sIsHandlingRequest = false;
	sHandlingClientID = kInvalidClient;
	sHandlingConnection = nullptr;

	return true;
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::WebServer::httpHeader::httpHeader(const tbCore::tbString& headerString) :
	mHeaderLines(),
	mHeaderFields()
{
	//"you can split everything by \n and then by : and trimming whitespaces, to get key-value pairs of headers"
	auto modifiedHeader = tbCore::String::ReplaceAllInstancesOf(headerString, "\r", "");

	std::vector<tbCore::tbString> lines = tbCore::String::SeparateString(modifiedHeader, "\n");
	for (const auto& line : lines)
	{
		//This may fail if a header line can contain an escaped : character. SpicyCondiment
		//When we make this more universal code, we should create unit tests. Rhymu8354 supplied
		//the following: https://github.com/rhymu8354/MessageHeaders/blob/master/test/src/MessageHeadersTests.cpp

		//There is chance that we did this wrong and the loop would need to change to extract the first line
		//as a header line and the rest as key-values. This will work for current needs and so it stays.
		const size_t colonAt = line.find(":");
		if (std::string::npos == colonAt)
		{
			mHeaderLines.push_back(line);
		}
		else
		{
			const tbCore::tbString key = tbCore::String::Lowercase(tbCore::String::TrimWhitespace(line.substr(0, colonAt)));
			const tbCore::tbString value = tbCore::String::TrimWhitespace(line.substr(colonAt + 1));
			mHeaderFields[key] = value;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::WebServer::httpHeader::~httpHeader(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

const tbCore::tbString& TyreBytes::Core::Services::WebServer::httpHeader::GetHeaderLine(void) const
{
	return (mHeaderLines.empty()) ? Implementation::kEmptyString : mHeaderLines[0];
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Services::WebServer::httpHeader::HasHeaderField(const tbCore::tbString& fieldName) const
{
	return (mHeaderFields.end() == mHeaderFields.find(fieldName)) ? false : true;
}

//--------------------------------------------------------------------------------------------------------------------//

const tbCore::tbString& TyreBytes::Core::Services::WebServer::httpHeader::GetHeaderFieldValue(const tbCore::tbString& fieldName) const
{
	const auto fieldIterator = mHeaderFields.find(fieldName);
	return (mHeaderFields.end() == fieldIterator) ? Implementation::kEmptyString : fieldIterator->second;
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString UnescapeUrlString(const tbCore::tbString& urlString)
{
	return tbCore::String::ReplaceAllInstancesOf(urlString, "+", " ");
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::WebServer::httpRequest::httpRequest(const tbCore::tbString& request) :
	mPath(),
	mParameters()
{
	const size_t getIndex = request.find("GET ");
	tb_debug_log_if(0 != getIndex, "Expected to see a GET as the first part of request.");

	const size_t httpIndex = request.find("HTTP/1.1");
	tb_debug_log_if(std::string::npos == httpIndex, "Expected to find HTTP/1.1 in the request.");

	//We are assuming here that before HTTP/1.1 there is exactly 1 space character. IDK if that is the standard.
	//BEFORE: GET /str?time=1234&setting=true HTTP/1.1
	// AFTER: /str?time=1234&setting=true
	const tbCore::tbString pathAndParameters = tbCore::String::TrimWhitespace(request.substr(getIndex + 4, httpIndex - getIndex - 4));

	const size_t startParameterIndex = pathAndParameters.find("?");
	if (std::string::npos != startParameterIndex)
	{
		mPath = pathAndParameters.substr(0, startParameterIndex);
		tb_debug_log(LogNetwork::Trace() << "Path: \"" << mPath << "\"");

		tbCore::tbString parameters = pathAndParameters.substr(startParameterIndex + 1);
		std::vector<tbCore::tbString> parameterAndValues = tbCore::String::SeparateString(parameters, "&");

		tb_debug_log(LogNetwork::Trace() << "Parameters: ");
		for (const tbCore::tbString& input : parameterAndValues)
		{
			std::vector<tbCore::tbString> split = tbCore::String::SeparateString(input, "=");
			tb_debug_log_if(split.size() != 2, LogNetwork::Trace() << "The parameter does not have a value.");
			if (split.size() == 2)
			{
				//atomicnibble: You can only do space as + if content-type is "application/x-www-form-urlencoded" and
				//		the + are only allowed in the query part of the url.
				const tbCore::tbString keyName = UnescapeUrlString(split[0]); //Note: 2021-09-01 UnescapeUrl is not very robust yet.
				const tbCore::tbString keyValue = UnescapeUrlString(split[1]); //Note: 2021-09-01 UnescapeUrl is not very robust yet.
				mParameters[keyName] = keyValue;
				tb_debug_log(LogNetwork::Trace() << "\t\"" << keyName << "\" = \"" << keyValue << "\"");
			}
		}
	}
	else
	{
		mPath = pathAndParameters;
	}
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::WebServer::httpRequest::~httpRequest(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

const tbCore::tbString& TyreBytes::Core::Services::WebServer::httpRequest::GetPath(void) const
{
	return mPath;
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Services::WebServer::httpRequest::HasParameter(const tbCore::tbString& parameterName) const
{
	return (mParameters.end() == mParameters.find(parameterName)) ? false : true;
}

//--------------------------------------------------------------------------------------------------------------------//

const tbCore::tbString& TyreBytes::Core::Services::WebServer::httpRequest::GetParameterValue(const tbCore::tbString& parameterName) const
{
	const auto parameterIterator = mParameters.find(parameterName);
	return (mParameters.end() == parameterIterator) ? Implementation::kEmptyString : parameterIterator->second;
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::WebServer::httpRequestHandlerInterface::httpRequestHandlerInterface(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::WebServer::httpRequestHandlerInterface::~httpRequestHandlerInterface(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Services::WebServer::httpRequestHandlerInterface::SendData(const tbCore::tbString& data)
{
	return SendData(reinterpret_cast<const tbCore::byte*>(data.c_str()), data.size());
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Services::WebServer::httpRequestHandlerInterface::SendData(const tbCore::byte* data, size_t size)
{
	tb_error_if(nullptr == Implementation::HttpServerHandler::sHandlingConnection,
		"httpRequestHandler::SendData can only be called while handling an http request.");

	return Implementation::HttpServerHandler::sHandlingConnection->SendPacketTo(data, size, Implementation::HttpServerHandler::sHandlingClientID);
}

//--------------------------------------------------------------------------------------------------------------------//
