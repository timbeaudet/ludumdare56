///
/// @file
/// @details Opens a TCP connection to send data to things that attempt to connect to us.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef Core_Services_HttpServer_hpp
#define Core_Services_HttpServer_hpp

#include <turtle_brains/core/tb_noncopyable.hpp>
#include <turtle_brains/core/tb_dynamic_structure.hpp>
#include <turtle_brains/network/tb_socket_connection.hpp>

#include <memory>
#include <unordered_map>


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
					class HttpServerHandler;
					class HttpServerData;
				};

				class httpHeader
				{
				public:
					httpHeader(const tbCore::tbString& headerString);
					~httpHeader(void);

					const tbCore::tbString& GetHeaderLine(void) const;

					bool HasHeaderField(const tbCore::tbString& fieldName) const;
					const tbCore::tbString& GetHeaderFieldValue(const tbCore::tbString& fieldName) const;

				private:
					typedef std::unordered_map<tbCore::tbString, tbCore::tbString> HeaderMap;
					std::vector<tbCore::tbString> mHeaderLines;
					HeaderMap mHeaderFields;
				};

				class httpRequest
				{
				public:
					httpRequest(const tbCore::tbString& request);
					~httpRequest(void);

					const tbCore::tbString& GetPath(void) const;

					bool HasParameter(const tbCore::tbString& parameterName) const;
					const tbCore::tbString& GetParameterValue(const tbCore::tbString& parameterName) const;

				private:
					typedef std::unordered_map<tbCore::tbString, tbCore::tbString> ParameterMap;
					tbCore::tbString mPath;
					ParameterMap mParameters;
				};

				class httpRequestHandlerInterface : tbCore::Noncopyable
				{
				public:
					explicit httpRequestHandlerInterface(void);
					virtual ~httpRequestHandlerInterface(void);

				protected:
					///
					/// @details Handle the given request and return true/false to indicate if it was handled.
					///
					/// @note If false is returned, the httpServer will automatically generate a 404 response.
					///
					virtual bool OnHandleRequest(const httpRequest& request) = 0;

					///
					/// @note SendData can only be called during OnHandleRequest() otherwise it will trigger an error condition.
					///
					bool SendData(const tbCore::tbString& data);
					bool SendData(const tbCore::byte* data, size_t size);

				private:
					friend class Implementation::HttpServerHandler;
				};

				class httpServer : tbCore::Noncopyable
				{
				public:
					explicit httpServer(void);
					~httpServer(void);

					//Note: httpHandler must remain in scope until Disconnect() is called otherwise there will be hanging references.
					bool Connect(tbCore::uint16 port, httpRequestHandlerInterface& httpHandler);
					void Disconnect(void);
					bool IsConnected(void);

					void Update(float deltaTime);

				private:
					std::unique_ptr<Implementation::HttpServerData> mData;
				};
			};

		};	//namespace Services
	};	//namespace Core
};	//namespace TyreBytes

#endif /* Core_Services_HttpServer_hpp */
