///
/// @file
/// @details Provide an API to Authenticate a user with Twitch services and if they are subscribed / supporting.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef Core_ConnectorServiceInterface_hpp
#define Core_ConnectorServiceInterface_hpp

#include "../../core/services/web_server.hpp"

#include <turtle_brains/core/tb_noncopyable.hpp>
#include <turtle_brains/core/tb_string.hpp>
#include <turtle_brains/core/tb_dynamic_structure.hpp>

#include <functional>
#include <memory>

namespace TyreBytes
{
	namespace Core
	{
		namespace Services
		{
			namespace Implementation
			{
				struct ServiceData;
			};

			enum SubscriptionTier { Unsubscribed, Tier1, Tier2, Tier3 };

			struct AuthenticationResult
			{
				tbCore::tbString mDisplayName;
				tbCore::tbString mUserID;
				SubscriptionTier mSubscription;
				bool mIsVerified;
				bool mSuccessful; //true if the server response was successful, false if something went wrong.
			};

			class ConnectorServiceInterface : public WebServer::httpRequestHandlerInterface
			{
			public:
				///
				/// @details Ensures the http server is shutdown and cleans up any resources from the authentication process.
				///
				/// @note This will end any and all request. No callbacks will be called.
				///
				~ConnectorServiceInterface(void);

				///
				/// @details Returns the name of the concrete service running under the interface.
				///
				tbCore::tbString GetServiceName(void) const;

				///
				/// @details After starting the service you can check if there is a valid access key which can skip some steps. Instead
				///   of calling RequestAuthenticationCode() you can jump to VerifyUserAccessKey().
				///
				bool HasUserAccessKey(void) const;

				///
				/// @details Returns the user access key which may be an empty string if invalid.
				///
				const tbCore::tbString& GetUserAccessKey(void) const;

				/// @details Check if the user account is verified with the service, to be used after VerifyAuthenticationCode()
				///   or VerifyUserAccessKey() to eliminate the processing or error handling in game-code.
				///
				bool IsUserVerified(void) const;

				///
				/// @details Will return true if the user is subscribed to the tier or a higher tier.
				///
				bool IsUserSubscribed(SubscriptionTier tier) const;

				///
				/// @details Returns the display name of the user based on the service being connected with.
				///
				tbCore::tbString GetUserDisplayName(void) const;
				tbCore::tbString GetUserID(void) const;

				///
				/// @details Open a web-browser and start a http server to listen for authentication with Twitch. The user will
				///   be prompted to sign-in to Twitch and given the option to autheniticate or not.
				///
				/// @note It is possible the user closes the tab instead of accepting authentication, use CancelAuthenticationRequest()
				///   to time-out the server in the background.
				///
				void RequestAuthenticationCode(std::function<void(bool success)> callback);

				///
				/// @details Call this to timeout RequestAuthenticationCode() which runs an http server in the background.
				///
				void CancelAuthenticationRequest(void);

				///
				/// @details Send a UserAccessKey to the server to verify the name and subscription status of the user.
				///
				/// @note This will throw an error condition if the userAccessKey is invalid.
				///
				void VerifyUserAccessKey(std::function<void(bool success)> callback);

				///
				/// @note Sends the userAccessKey to the WebServer for verification. It is plausible that the access key had to
				///   refresh tokens and a new key was returned from the WebServer. In that situation the GameServer should be
				///   sending the new key back to user to save on disk. Theoretically this won't happen as the users game would
				///   have called verify before sending the key to GameServer.
				///
				/// @note Unlike other functions in the connector this does not start an http server/listening connection and does
				///   not store the results beyond passing them to the callback.
				///
				void GameServerVerifyUserAccessKey(const tbCore::tbString& userAccessKey, std::function<void(AuthenticationResult result)> callback);

			protected:
				///
				/// @details Creates a service to authenticate the user as a twitch viewer with the game. This will also attempt
				///   to load the UserAccessKey from disk which would allow skipping the RequestAuthenticationCode().
				///
				/// @note Call Shutdown() passing the service when completed.
				///
				explicit ConnectorServiceInterface(const tbCore::tbString& serviceName, const tbCore::tbString& applicationClientID,
					const tbCore::tbString& redirectURI, const tbCore::uint16 listeningPort, const tbCore::tbString& userKeyFilepath);

				virtual tbCore::tbString GetBrowserAuthenticationPath(void) const;
				virtual bool OnHandleRequest(const WebServer::httpRequest& request) override;

				std::unique_ptr<Implementation::ServiceData> mData;
				const tbCore::tbString mServiceName;
			};

			class TwitchConnectorService : public ConnectorServiceInterface
			{
			public:
				TwitchConnectorService(const tbCore::tbString& applicationClientID, const tbCore::tbString& userKeyFilepath);
			};

			class PatreonConnectorService : public ConnectorServiceInterface
			{
			public:
				PatreonConnectorService(const tbCore::tbString& applicationClientID, const tbCore::tbString& userKeyFilepath);
			};

			class YouTubeConnectorService : public ConnectorServiceInterface
			{
			public:
				YouTubeConnectorService(const tbCore::tbString& applicationClientID, const tbCore::tbString& userKeyFilepath);
			};

		};	//namespace Services
	};	//namespace Core
};	//namespace TyreBytes

#endif /* Core_ConnectorServiceInterface_hpp */
