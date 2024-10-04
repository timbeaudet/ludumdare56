///
/// @file
/// @details Provide an API to Authenticate a user with Twitch services and if they are subscribed / supporting.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "connector_service_interface.hpp"

#include "../../core/services/web_server.hpp"
#include "../../core/utilities.hpp"
#include "../../version.hpp"

#include <turtle_brains/core/tb_defines.hpp>
#include <turtle_brains/core/debug/tb_debug_logger.hpp>
#include <turtle_brains/core/tb_dynamic_structure.hpp>
#include <turtle_brains/core/tb_json_parser.hpp>
#include <turtle_brains/system/tb_system_utilities.hpp>

#include <turtle_brains/network/tb_network.hpp>
#include <turtle_brains/network/tb_http_request.hpp>
#include <turtle_brains/network/tb_socket_connection.hpp>
#include <turtle_brains/network/tb_packet_handler_interface.hpp>

#include <turtle_brains/core/debug/tb_debug_logger.hpp>

struct AuthenticationChannel { static tbCore::tbString AsString(void) { return ("Auth"); } };
typedef TurtleBrains::Core::Debug::LogChannelLevel<AuthenticationChannel> LogAuth;

//Do not log anything secret on Error or Always, otherwise it will be displayed!
struct SecretChannel { static tbCore::tbString AsString(void) { return ("Secret"); } };
typedef TurtleBrains::Core::Debug::LogChannelLevel<SecretChannel> LogSecret;

namespace TyreBytes
{
	namespace Core
	{
		namespace Services
		{
			namespace Implementation
			{
				extern const unsigned char theAuthenticationSuccessPageData[];
				extern const size_t theAuthenticationSuccessPageSize;
				extern const unsigned char theAuthenticationFailurePageData[];
				extern const size_t theAuthenticationFailurePageSize;

				const tbCore::tbString kTyreBytesURI = "https://dev.tyrebytes.com/";
				//const tbCore::tbString kTyreBytesURI = "https://www.tyrebytes.com/";
				const tbCore::tbString kTyreBytesAPI = kTyreBytesURI + "api/1/";

				struct ServiceData
				{
					WebServer::httpServer mHttpServer;
					std::function<void(bool)> mAuthenticationCallback;

					tbCore::tbString mUserKeyFilepath = "";
					tbCore::tbString mApplicationClientID = "";
					tbCore::tbString mRedirectURI = "";
					tbCore::uint16 mListeningPort = 45045;

					//These are more "State Based" variables.
					tbCore::tbString mUserAccessKey = "";

					tbCore::tbString mDisplayName = "";
					tbCore::tbString mUserID = "";
					SubscriptionTier mSubscribedTier = SubscriptionTier::Unsubscribed;
					bool mIsVerified = false;
					bool mIsWaitingForRequest = false;
					bool mIsServiceStarted = false;
				};

				bool CheckForApiErrors(const tbCore::DynamicStructure& apiResult)
				{
					const tbCore::DynamicStructure& apiError = apiResult["error"];
					if (true == apiError.IsStructure())
					{
						const tbCore::tbString title = apiError["error"].AsStringWithDefault("UnknownError");
						const tbCore::tbString message = apiError["message"].AsStringWithDefault("Without a description.");
						tb_always_log(LogAuth::Error() << "API Error: " << title << ": " << message);
						return true;
					}

					return false;
				}
			};	//namespace Implementation
		};	//namespace Services
	};	//namespace Core
};	//namespace TyreBytes

//TODO: LudumDare56: Crash: If a request is cancelled that later uses a ConnectorService (this) when the request
//  returns, it could crash because the object no longer exists. One solution may be to use shared_ptr's and a weak_ptr
//  within the request llambda, but that currently would require trusting the client of the API to use a shared_ptr,
//  we may need to adjust the API to ensure this is forced, or hidding from the API user.
//
//  Note if we go with shared_ptr/weak_ptr make sure to go with shared_from_this or such. Bad example below used a shared_ptr
//    from this incorrectly: https://en.cppreference.com/w/cpp/memory/enable_shared_from_this
//
//  Another option might be to use some booleans in here that will protect access, but that could be rather complicated
//  and hacky. The final solution would be a global shared_ptr-like object that invalidates itself. Perhaps a table by
//  service name and only one of each service name can exist at any point in time. This last option could be all in the
//  implementation details and not need to concern the API user with exception of restriction each ConnectorService to
//  exactly one instance at a given time.

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::ConnectorServiceInterface::ConnectorServiceInterface(const tbCore::tbString& serviceName,
	const tbCore::tbString& applicationClientID, const tbCore::tbString& redirectURI,
	const tbCore::uint16 listeningPort, const tbCore::tbString& userKeyFilepath) :
	mData(new Implementation::ServiceData),
	mServiceName(serviceName)
{
	mData->mUserKeyFilepath = userKeyFilepath;
	mData->mApplicationClientID = applicationClientID;
	mData->mRedirectURI = redirectURI;
	mData->mListeningPort = listeningPort;

	mData->mUserAccessKey = Utilities::LoadFileContentsToString(mData->mUserKeyFilepath, true);
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::ConnectorServiceInterface::~ConnectorServiceInterface(void)
{
	CancelAuthenticationRequest();
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString TyreBytes::Core::Services::ConnectorServiceInterface::GetServiceName(void) const
{
	return mServiceName;
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Services::ConnectorServiceInterface::HasUserAccessKey(void) const
{
	return (false == mData->mUserAccessKey.empty());
}

//--------------------------------------------------------------------------------------------------------------------//

const tbCore::tbString& TyreBytes::Core::Services::ConnectorServiceInterface::GetUserAccessKey(void) const
{
	return mData->mUserAccessKey;
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Services::ConnectorServiceInterface::IsUserVerified(void) const
{
	return mData->mIsVerified;
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Services::ConnectorServiceInterface::IsUserSubscribed(SubscriptionTier tier) const
{
	return mData->mSubscribedTier >= tier;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString TyreBytes::Core::Services::ConnectorServiceInterface::GetUserDisplayName(void) const
{
	return mData->mDisplayName;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString TyreBytes::Core::Services::ConnectorServiceInterface::GetUserID(void) const
{
	return mData->mUserID;
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Services::ConnectorServiceInterface::RequestAuthenticationCode(std::function<void(bool success)> callback)
{
	tb_error_if(true == mData->mIsWaitingForRequest, "Cannot call RequestAuthenticationCode when awaiting a response already.");
	tb_debug_log(LogAuth::Trace() << "Starting request to " << mServiceName << " for user authentication.");

	mData->mIsWaitingForRequest = true;
	mData->mAuthenticationCallback = callback;

	if (false == mData->mHttpServer.IsConnected())
	{
		mData->mHttpServer.Connect(mData->mListeningPort, *this);
	}

	tbSystem::OpenBrowserTo(GetBrowserAuthenticationPath());
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Services::ConnectorServiceInterface::CancelAuthenticationRequest(void)
{
	tb_debug_log_if(true == mData->mIsWaitingForRequest, LogAuth::Trace() << "Cancelling " << mServiceName << " authentication requests...");
	tb_debug_log_if(false == mData->mIsWaitingForRequest, LogAuth::Trace() << "No " << mServiceName << " authentication request to cancel, already finished.");

	mData->mIsWaitingForRequest = false;

	if (true == mData->mHttpServer.IsConnected())
	{
		mData->mHttpServer.Disconnect();
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Services::ConnectorServiceInterface::VerifyUserAccessKey(std::function<void(bool success)> callback)
{
	tb_error_if(true == mData->mUserAccessKey.empty(), "Expected the service to have a valid userAccessKey, call VerifyAuthenticationCode() first.");
	tb_debug_log(LogAuth::Trace() << "Trying to VerifyUserAccessKey through " << mServiceName << ".");

	GameServerVerifyUserAccessKey(mData->mUserAccessKey, [this, callback](AuthenticationResult result) {
		tb_debug_log(LogAuth::Trace() << "VerifyUserAccessKey result for " << mServiceName << " is: " << std::boolalpha << result.mIsVerified);

		mData->mIsVerified = result.mIsVerified;
		mData->mUserID = result.mUserID;
		mData->mDisplayName = result.mDisplayName;
		mData->mSubscribedTier = result.mSubscription;

		callback(result.mSuccessful);
	});
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Services::ConnectorServiceInterface::GameServerVerifyUserAccessKey(const tbCore::tbString& userAccessKey,
	std::function<void(AuthenticationResult result)> callback)
{
	tb_error_if(true == userAccessKey.empty(), "Expected the service to have a valid userAccessKey.");

	const tbCore::tbString verifyApi = tbCore::String::Lowercase(mServiceName) + "/verify.php";
	tbNetwork::HTTP::Request request(Implementation::kTyreBytesAPI + verifyApi);
	request.AddParameter("access_key", userAccessKey);
	request.AddParameter("game", "ludumdare56");
	request.AddParameter("game_version", LudumDare56::Version::VersionString());
	request.AddParameter("info", "true"); //Does nothing at this time.
	const tbCore::tbString serviceName = mServiceName;
	request.GetResponseAsync([callback, serviceName, this](tbNetwork::HTTP::Response response) {
		tb_debug_log(LogAuth::Debug() << serviceName << ": ServerSideVerifyAuthCode ResponseCode: " << response.GetResponseCode());
		tb_debug_log(LogSecret::Debug() << serviceName << ": with a body of: " << response.GetResponseBody());

		const tbCore::DynamicStructure verificationResults = tbCore::ParseJson(response.GetResponseBody());
		if (false == Implementation::CheckForApiErrors(verificationResults))
		{	//NOTE: The following collection of user_info is duplicated code... VerifyAuthenticationCode() and GameServerVerifyUserAccessKey()
			const tbCore::DynamicStructure& userInfo = verificationResults["user_info"];

			AuthenticationResult result;
			result.mDisplayName = userInfo["display_name"].AsString();
			result.mUserID = userInfo["id"].AsString();
			result.mIsVerified = true;
			result.mSuccessful = true;

			const tbCore::uint32 subscriptionAmount = userInfo["support_amount"].AsRangedIntegerWithDefault<tbCore::uint32>(0, "out of range");
			if (subscriptionAmount > 1000) { result.mSubscription = SubscriptionTier::Tier3; }
			else if (subscriptionAmount > 500) { result.mSubscription = SubscriptionTier::Tier2; }
			else if (subscriptionAmount > 0) { result.mSubscription = SubscriptionTier::Tier1; }
			else { result.mSubscription = SubscriptionTier::Unsubscribed; }

			//Note: It would be better if the (this) object was not allowed in the llambda, but the UserAccessKey might
			//   be updated when verifying a user is still authenticated due to refreshing of the tokens. This might
			//   get a little bumpy for the GameServer as the GameServer doesn't need to save the access keys to disk,
			//   although it should send the access key back to the client. (before 2022-07-29)
			mData->mUserAccessKey = verificationResults["access_key"].AsStringWithDefault("");
			if (false == mData->mUserAccessKey.empty())
			{
				tb_debug_log(LogAuth::Trace() << "Saving the user_access_key to file: " << mData->mUserKeyFilepath);
				Core::Utilities::SaveStringContentToFile(mData->mUserKeyFilepath, mData->mUserAccessKey);
			}
			callback(result);
		}
		else
		{
			AuthenticationResult result;
			result.mDisplayName = "";
			result.mUserID = "";
			result.mSubscription = SubscriptionTier::Unsubscribed;
			result.mIsVerified = false;
			result.mSuccessful = false;

			callback(result);
		}
	});
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString TyreBytes::Core::Services::ConnectorServiceInterface::GetBrowserAuthenticationPath(void) const
{
	return Implementation::kTyreBytesURI + "auth/" + tbCore::String::Lowercase(mServiceName) +
		"?game=ludumdare56&game_version=" + LudumDare56::Version::VersionString() +
		"&redirect_uri=" + mData->mRedirectURI;
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Services::ConnectorServiceInterface::OnHandleRequest(const WebServer::httpRequest& request)
{
	tb_debug_log(LogAuth::Info() << "Handling an httpRequest for " << mServiceName << " authentication: " << request.GetPath());

	if (request.GetPath() == "/")
	{
		const tbCore::tbString errorTitle = request.GetParameterValue("error");
		const tbCore::tbString errorDescription = request.GetParameterValue("error_description");
		const tbCore::tbString userAccessKey = request.GetParameterValue("access_key");
		const tbCore::tbString userID = request.GetParameterValue("id");
		const tbCore::tbString displayName = request.GetParameterValue("display_name");
		const tbCore::tbString supportAmountString = request.GetParameterValue("support_amount");
		const tbCore::uint32 supportAmount = (true == supportAmountString.empty()) ? 0 : tbCore::FromString<tbCore::uint32>(supportAmountString);

		SendData("HTTP/1.1 200 OK\nContent-Type: text/html\n\n");
		if (false == errorTitle.empty() || true == userAccessKey.empty() || true == userID.empty())
		{
			tb_always_log(LogAuth::Info() << "Failed to authenticate with " << mServiceName << ":\n\tError: " << errorTitle << "\n\tDescription: " << errorDescription);
			SendData(Implementation::theAuthenticationFailurePageData, Implementation::theAuthenticationFailurePageSize);
			mData->mAuthenticationCallback(false);
		}
		else
		{
			SendData(Implementation::theAuthenticationSuccessPageData, Implementation::theAuthenticationSuccessPageSize);

			mData->mIsVerified = true;
			mData->mUserID = userID;
			mData->mDisplayName = displayName;
			mData->mUserAccessKey = userAccessKey; //Must set this before calling theAuthenticationCallback().

			if (supportAmount > 1000) { mData->mSubscribedTier = SubscriptionTier::Tier3; }
			else if (supportAmount > 500) { mData->mSubscribedTier = SubscriptionTier::Tier2; }
			else if (supportAmount > 0) { mData->mSubscribedTier = SubscriptionTier::Tier1; }
			else { mData->mSubscribedTier = SubscriptionTier::Unsubscribed; }

			if (false == mData->mUserAccessKey.empty())
			{
				tb_debug_log(LogAuth::Trace() << "Saving the user_access_key to file: " << mData->mUserKeyFilepath);
				Core::Utilities::SaveStringContentToFile(mData->mUserKeyFilepath, userAccessKey);
			}
			tb_debug_log(LogAuth::Trace() << "Calling the " << mServiceName << " authentication callback for game-code to handle.");
			mData->mAuthenticationCallback(true);
		}

		mData->mIsWaitingForRequest = false;
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::TwitchConnectorService::TwitchConnectorService(const tbCore::tbString& applicationClientID,
	const tbCore::tbString& userKeyFilepath) :
	ConnectorServiceInterface("Twitch", applicationClientID, "http://localhost:45045", 45045, userKeyFilepath)
{
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::PatreonConnectorService::PatreonConnectorService(const tbCore::tbString& applicationClientID,
	const tbCore::tbString& userKeyFilepath) :
	ConnectorServiceInterface("Patreon", applicationClientID, "http://localhost:45045", 45045, userKeyFilepath)
{
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Services::YouTubeConnectorService::YouTubeConnectorService(const tbCore::tbString& applicationClientID,
	const tbCore::tbString& userKeyFilepath) :
	ConnectorServiceInterface("YouTube", applicationClientID, "http://localhost:45045", 45045, userKeyFilepath)
{
}

//--------------------------------------------------------------------------------------------------------------------//

