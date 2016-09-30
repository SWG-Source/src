// SessionApiClient.cpp
// copyright 2002 Sony Online Entertainment


#include "FirstConnectionServer.h"
#include "SessionApiClient.h"

#include "ClientConnection.h"
#include "ConfigConnectionServer.h"

#include "serverNetworkMessages/AccountFeatureIdRequest.h"
#include "serverNetworkMessages/AccountFeatureIdResponse.h"
#include "serverNetworkMessages/AdjustAccountFeatureIdRequest.h"
#include "serverNetworkMessages/AdjustAccountFeatureIdResponse.h"
#include "serverNetworkMessages/ClaimRewardsMessage.h"
#include "serverNetworkMessages/ClaimRewardsReplyMessage.h"
#include "Session/CommonAPI/CommonAPIStrings.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedLog/Log.h"

#include "sharedFoundation/CrcConstexpr.hpp"

#include <algorithm>
#include <map>
#include <vector>

//------------------------------------------------------------------------------------------

namespace SessionApiClientNamespace
{
	std::map<std::string, ClientConnection*> ms_sessionIdMap;
	std::map<apiTrackingNumber, GameNetworkMessage *> ms_getFeaturesTrackingNumberMap;
	std::map<apiTrackingNumber, AdjustAccountFeatureIdResponse *> ms_modifyFeatureTrackingNumberMap;
	std::map<apiTrackingNumber, AdjustAccountFeatureIdResponse *> ms_grantFeatureTrackingNumberMap;
}

using namespace SessionApiClientNamespace;


//------------------------------------------------------------------------------------------

std::map<apiTrackingNumber, ClientConnection *> SessionApiClient::m_validationMap;

//------------------------------------------------------------------------------------------

namespace SessionApiClientNamespace
{

};

using namespace SessionApiClientNamespace;

//------------------------------------------------------------

SessionApiClient::SessionApiClient(const char ** serverList, int serverCount) :
		Client(serverList, static_cast<unsigned int>(serverCount), "Starwars Connection Server"),
		m_sessionTimer(ConfigConnectionServer::getTimeBetweenSessionUpdates())

{
}

//------------------------------------------------------------

SessionApiClient::~SessionApiClient()
{
	std::map<std::string, ClientConnection*>::iterator j = ms_sessionIdMap.begin();
	std::vector<const char*> sessionList;
	sessionList.reserve(ms_sessionIdMap.size());
	for (; j != ms_sessionIdMap.end(); ++j)
	{
		sessionList.push_back(j->first.c_str());
	}

	if (!ConfigConnectionServer::getDisableSessionLogout())
	{
		//SessionLogout all clients
		SessionLogout(&sessionList[0], sessionList.size());
	}
}

//---------------------------------------------------------------------

void SessionApiClient::OnSessionLogin(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const LoginAPI::UsageLimit & usageLimit,
                                            const LoginAPI::Entitlement & entitlement,
                                            void * userData)
{
	UNREF(trackingNumber);
	UNREF(result);
	UNREF(account);
	UNREF(subscription);
	UNREF(session);
	UNREF(usageLimit);
	UNREF(entitlement);
	UNREF(userData);
//	DEBUG_REPORT_LOG(true, ("Session login.\n"));
}

//---------------------------------------------------------------------

void SessionApiClient::OnSessionLoginInternal(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const LoginAPI::UsageLimit & usageLimit,
                                            const LoginAPI::Entitlement & entitlement,
                                            void * userData)
{
	UNREF(trackingNumber);
	UNREF(result);
	UNREF(account);
	UNREF(subscription);
	UNREF(session);
	UNREF(usageLimit);
	UNREF(entitlement);
	UNREF(userData);
//	DEBUG_REPORT_LOG(true, ("Session login internal.\n"));
}

//---------------------------------------------------------------------

void SessionApiClient::OnSessionValidate(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const LoginAPI::UsageLimit & usageLimit,
                                            const LoginAPI::Entitlement & entitlement,
                                            void * userData)
{
	UNREF(userData);
	UNREF(usageLimit);

	std::map<apiTrackingNumber, ClientConnection*>::iterator i = m_validationMap.find(trackingNumber);
	DEBUG_REPORT_LOG(true, ("OnSessionValidate result: %d for suid: %d (entitlement total: %u/%u, since last login: %u/%u, reason unentitled: %d)\n", result, account.GetId(), entitlement.GetEntitledTime(), entitlement.GetTotalTime(), entitlement.GetEntitledTimeSinceLastLogin(), entitlement.GetTotalTimeSinceLastLogin(), static_cast<int>(entitlement.GetReasonUnentitled())));
	if (i != m_validationMap.end())
	{
		if (result == RESULT_SUCCESS)
		{
			if (i->second)
			{
				i->second->onValidateClient(account.GetId(), account.GetName(), session.GetIsSecure(), session.GetId(), subscription.GetGameFeatures(), subscription.GetSubscriptionFeatures(), entitlement.GetTotalTime(), entitlement.GetEntitledTime(), entitlement.GetTotalTimeSinceLastLogin(), entitlement.GetEntitledTimeSinceLastLogin(), ConfigConnectionServer::getFakeBuddyPoints()); //TODO:  get buddy points from station
				std::map<std::string, ClientConnection*>::iterator j = ms_sessionIdMap.find(session.GetId());
				if (j != ms_sessionIdMap.end())
				{
					LOG("ClientDisconnect", ("Client %s Disconnected by duplicate sessionId login.", j->second->getSessionId().c_str()));
					LOG("CustomerService", ("Login:%s SessionId %s Disconnected by duplicate sessionId login.", ClientConnection::describeAccount(j->second).c_str(), j->second->getSessionId().c_str()));
					j->second->disconnect();
				}

				ms_sessionIdMap[session.GetId()] = i->second;
			}
			m_validationMap.erase(i);
		}
		else
		{
//			ErrorMessage err("VALIDATION FAILED", "Your station Id was not valid. Wrong password? Account closed?");
//			i->second->send(err, true);
			LOG("ClientDisconnect", ("Suid %s by session denial reason %d.", account.GetId(), result));
			LOG("CustomerService", ("Login:%s by session denial reason %d.", ClientConnection::describeAccount(i->second).c_str(), result));
			i->second->disconnect();
		}
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("Could not find client in the validation map\n."));
	}
}

//---------------------------------------------------------------------

void SessionApiClient::OnSessionConsume(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const LoginAPI::UsageLimit & usageLimit,
                                            const LoginAPI::Entitlement & entitlement,
                                            void * userData)
{
	UNREF(trackingNumber);
	UNREF(result);
	UNREF(account);
	UNREF(subscription);
	UNREF(session);
	UNREF(usageLimit);
	UNREF(entitlement);
	UNREF(userData);
//	DEBUG_REPORT_LOG(true, ("Session consume.\n"));
}

//---------------------------------------------------------------------

void SessionApiClient::OnSessionStartPlay(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData)
{
	UNREF(trackingNumber);
	UNREF(result);
	UNREF(userData);
//	DEBUG_REPORT_LOG(true, ("Session start play.\n"));
}

//---------------------------------------------------------------------

void SessionApiClient::OnSessionStopPlay(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData)
{
	UNREF(trackingNumber);
	UNREF(result);
	UNREF(userData);
//	DEBUG_REPORT_LOG(true, ("Session stop play.\n"));
}

//---------------------------------------------------------------------

void SessionApiClient::OnSessionKick(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData)
{
	UNREF(trackingNumber);
	UNREF(result);
	UNREF(userData);
//	DEBUG_REPORT_LOG(true, ("Session kick.\n"));
}

//---------------------------------------------------------------------

void SessionApiClient::OnGetSessions(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const unsigned count, 
                                            const apiSession session[], 
                                            const apiSubscription subscription[], 
                                            const LoginAPI::UsageLimit usageLimit[],
                                            const unsigned timeCreated[],
                                            const unsigned timeTouched[],
                                            void * userData)
{
	UNREF(trackingNumber);
	UNREF(result);
	UNREF(count);
	UNREF(session);
	UNREF(subscription);
	UNREF(usageLimit);
	UNREF(timeCreated);
	UNREF(timeTouched);
	UNREF(userData);
//	DEBUG_REPORT_LOG(true, ("Get sessions.\n"));
}

//------------------------------------------------------------

void SessionApiClient::OnGetFeatures(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const unsigned featureCount,
                                            const LoginAPI::Feature featureArray[],
                                            const void * userData)
{
	UNREF(userData);
	UNREF(AccountStatusString);
	UNREF(SessionTypeString);
	UNREF(GamecodeString);
	UNREF(SubscriptionStatusString);

	const char * const resultString = ResultString[result];
	std::string sResultString;
	if (resultString)
	{
		sResultString = resultString;
	}
	else
	{
		char buffer[32];
		snprintf(buffer, sizeof(buffer)-1, "%u", result);
		buffer[sizeof(buffer)-1] = '\0';

		sResultString = buffer;
	}

	const char * const resultText = ResultText[result];
	std::string sResultText;
	if (resultText)
	{
		sResultText = resultText;
	}
	else
	{
		char buffer[32];
		snprintf(buffer, sizeof(buffer)-1, "%u", result);
		buffer[sizeof(buffer)-1] = '\0';

		sResultText = buffer;
	}

	DEBUG_REPORT_LOG(true, ("SessionApiClient::OnGetFeatures() - [%u][%u][%u][%s][%s]\n", trackingNumber, featureCount, result, sResultString.c_str(), sResultText.c_str()));

	std::map<apiTrackingNumber, GameNetworkMessage *>::iterator i = ms_getFeaturesTrackingNumberMap.find(trackingNumber);
	if (i != ms_getFeaturesTrackingNumberMap.end())
	{
		bool reuseMessage = false;
		AccountFeatureIdRequest const * accountFeatureIdRequest = nullptr;
		AdjustAccountFeatureIdRequest const * adjustAccountFeatureIdRequest = nullptr;
		AdjustAccountFeatureIdResponse * adjustAccountFeatureIdResponse = nullptr;
		ClaimRewardsMessage * claimRewardsMessage = nullptr;

		const uint32 msgType = i->second->getType();
		
		switch(msgType) {
			case constcrc("AccountFeatureIdRequest") : {
				accountFeatureIdRequest = dynamic_cast<AccountFeatureIdRequest const *>(i->second);
				break;
			}
			case constcrc("AdjustAccountFeatureIdRequest") : {
				adjustAccountFeatureIdRequest = dynamic_cast<AdjustAccountFeatureIdRequest const *>(i->second);
				break;
			}
			case constcrc("AdjustAccountFeatureIdResponse") : {
				adjustAccountFeatureIdResponse = dynamic_cast<AdjustAccountFeatureIdResponse *>(i->second);
				break;
			}
			case constcrc("ClaimRewardsMessage") : {
				claimRewardsMessage = dynamic_cast<ClaimRewardsMessage *>(i->second);
				break;
			}
		}

		if (result == RESULT_SUCCESS)
		{
			ClientConnection * clientConnection = nullptr;
			if (accountFeatureIdRequest)
				clientConnection = ConnectionServer::getClientConnection(accountFeatureIdRequest->getTargetStationId());
			else if (adjustAccountFeatureIdRequest)
				clientConnection = ConnectionServer::getClientConnection(adjustAccountFeatureIdRequest->getTargetStationId());
			else if (adjustAccountFeatureIdResponse)
				clientConnection = ConnectionServer::getClientConnection(adjustAccountFeatureIdResponse->getTargetStationId());
			else if (claimRewardsMessage)
				clientConnection = ConnectionServer::getClientConnection(claimRewardsMessage->getStationId());

			if (clientConnection && !clientConnection->getHasCSLoggedAccountFeatureIds())
			{
				clientConnection->setHasCSLoggedAccountFeatureIds(true);

				std::string featureCodes;
				for (unsigned k = 0; k < featureCount; ++k)
				{
					if (!featureCodes.empty())
						featureCodes += ", ";

					featureCodes += FormattedString<512>().sprintf("%u (%s)", featureArray[k].GetID(), featureArray[k].GetData().c_str());
				}

				LOG("CustomerService", ("Login:%s has feature count %u (%s:%s) {%s}",
					ClientConnection::describeAccount(clientConnection).c_str(), featureCount, sResultString.c_str(), sResultText.c_str(), featureCodes.c_str()));
			}
		}

		if (accountFeatureIdRequest)
		{
			GameConnection * const gc = ConnectionServer::getGameConnection(accountFeatureIdRequest->getGameServer());
			if (gc)
			{
				std::map<uint32, int> featureIds;
				std::map<uint32, std::string> featureIdsData;

				if (result == RESULT_SUCCESS)
				{
					for (unsigned k = 0; k < featureCount; ++k)
					{
						featureIdsData[featureArray[k].GetID()] = featureArray[k].GetData();
						featureIds[featureArray[k].GetID()] = featureArray[k].GetConsumeCount();
					}
				}

				AccountFeatureIdResponse const rsp(accountFeatureIdRequest->getRequester(), accountFeatureIdRequest->getGameServer(), accountFeatureIdRequest->getTarget(), accountFeatureIdRequest->getTargetStationId(), accountFeatureIdRequest->getGameCode(), accountFeatureIdRequest->getRequestReason(), result, true, featureIds, featureIdsData, sResultString.c_str(), sResultText.c_str());
				gc->send(rsp,true);
			}
		}
		else if (adjustAccountFeatureIdRequest)
		{
			if (result != RESULT_SUCCESS)
			{
				// CS log SWG TCG or reward trade in account feature grant failure
				if (!adjustAccountFeatureIdRequest->getTargetPlayerDescription().empty() && adjustAccountFeatureIdRequest->getTargetItem().isValid() && !adjustAccountFeatureIdRequest->getTargetItemDescription().empty())
				{
					if (adjustAccountFeatureIdRequest->getGameCode() == PlatformGameCode::SWGTCG)
						LOG("CustomerService",("TcgRedemption: %s ***FAILED TO REDEEM*** %s for SWGTCG account feature Id %lu with OnGetFeatures() error code (%u, %s:%s)", adjustAccountFeatureIdRequest->getTargetPlayerDescription().c_str(), adjustAccountFeatureIdRequest->getTargetItemDescription().c_str(), adjustAccountFeatureIdRequest->getFeatureId(), result, sResultString.c_str(), sResultText.c_str()));
					else if (adjustAccountFeatureIdRequest->getGameCode() == PlatformGameCode::SWG)
						LOG("CustomerService",("VeteranRewards: %s ***FAILED TO TRADE IN*** %s for SWG account feature Id %lu with OnGetFeatures() error code (%u, %s:%s)", adjustAccountFeatureIdRequest->getTargetPlayerDescription().c_str(), adjustAccountFeatureIdRequest->getTargetItemDescription().c_str(), adjustAccountFeatureIdRequest->getFeatureId(), result, sResultString.c_str(), sResultText.c_str()));
				}

				GameConnection * const gc = ConnectionServer::getGameConnection(adjustAccountFeatureIdRequest->getGameServer());
				if (gc)
				{
					AdjustAccountFeatureIdResponse const rsp(adjustAccountFeatureIdRequest->getRequestingPlayer(), adjustAccountFeatureIdRequest->getGameServer(), adjustAccountFeatureIdRequest->getTargetPlayer(), adjustAccountFeatureIdRequest->getTargetPlayerDescription(), adjustAccountFeatureIdRequest->getTargetStationId(), adjustAccountFeatureIdRequest->getTargetItem(), adjustAccountFeatureIdRequest->getTargetItemDescription(), adjustAccountFeatureIdRequest->getGameCode(), adjustAccountFeatureIdRequest->getFeatureId(), 0, 0, result, true, sResultString.c_str(), sResultText.c_str());
					gc->send(rsp,true);
				}
			}
			else
			{
				// if account already has the feature, adjust it, otherwise add the feature
				LoginAPI::Feature const * existingFeature = nullptr;

				for (unsigned k = 0; k < featureCount; ++k)
				{
					if (featureArray[k].GetID() == adjustAccountFeatureIdRequest->getFeatureId())
					{
						existingFeature = &(featureArray[k]);
						break;
					}
				}

				if (existingFeature)
				{
					int const currentCount = existingFeature->GetConsumeCount();
					int const updatedCount = std::max(0, currentCount + adjustAccountFeatureIdRequest->getAdjustment());

					LoginAPI::Feature updatedFeature;
					updatedFeature.SetID(existingFeature->GetID());
					updatedFeature.SetData(existingFeature->GetData());
					updatedFeature.SetParameter("count", updatedCount);

					apiTrackingNumber const tn = ModifyFeature_v2(adjustAccountFeatureIdRequest->getTargetStationId(), PlatformGameCode::getGamecodeName(adjustAccountFeatureIdRequest->getGameCode()).c_str(), *existingFeature, updatedFeature);
					ms_modifyFeatureTrackingNumberMap[tn] = new AdjustAccountFeatureIdResponse(adjustAccountFeatureIdRequest->getRequestingPlayer(), adjustAccountFeatureIdRequest->getGameServer(), adjustAccountFeatureIdRequest->getTargetPlayer(), adjustAccountFeatureIdRequest->getTargetPlayerDescription(), adjustAccountFeatureIdRequest->getTargetStationId(), adjustAccountFeatureIdRequest->getTargetItem(), adjustAccountFeatureIdRequest->getTargetItemDescription(), adjustAccountFeatureIdRequest->getGameCode(), adjustAccountFeatureIdRequest->getFeatureId(), currentCount, updatedCount, RESULT_SUCCESS, true);
				}
				else
				{
					apiTrackingNumber const tn = GrantFeatureByStationID(adjustAccountFeatureIdRequest->getTargetStationId(), adjustAccountFeatureIdRequest->getFeatureId(), PlatformGameCode::getGamecodeName(adjustAccountFeatureIdRequest->getGameCode()).c_str());
					ms_grantFeatureTrackingNumberMap[tn] = new AdjustAccountFeatureIdResponse(adjustAccountFeatureIdRequest->getRequestingPlayer(), adjustAccountFeatureIdRequest->getGameServer(), adjustAccountFeatureIdRequest->getTargetPlayer(), adjustAccountFeatureIdRequest->getTargetPlayerDescription(), adjustAccountFeatureIdRequest->getTargetStationId(), adjustAccountFeatureIdRequest->getTargetItem(), adjustAccountFeatureIdRequest->getTargetItemDescription(), adjustAccountFeatureIdRequest->getGameCode(), adjustAccountFeatureIdRequest->getFeatureId(), 0, std::max(0, adjustAccountFeatureIdRequest->getAdjustment()), RESULT_SUCCESS, true);
				}
			}
		}
		else if (adjustAccountFeatureIdResponse)
		{
			if (result != RESULT_SUCCESS)
			{
				// CS log SWG TCG or reward trade in account feature grant failure
				if (!adjustAccountFeatureIdResponse->getTargetPlayerDescription().empty() && adjustAccountFeatureIdResponse->getTargetItem().isValid() && !adjustAccountFeatureIdResponse->getTargetItemDescription().empty())
				{
					if (adjustAccountFeatureIdResponse->getGameCode() == PlatformGameCode::SWGTCG)
						LOG("CustomerService",("TcgRedemption: %s ***FAILED TO REDEEM*** %s for SWGTCG account feature Id %lu with OnGetFeatures() error code (%u, %s:%s)", adjustAccountFeatureIdResponse->getTargetPlayerDescription().c_str(), adjustAccountFeatureIdResponse->getTargetItemDescription().c_str(), adjustAccountFeatureIdResponse->getFeatureId(), result, sResultString.c_str(), sResultText.c_str()));
					else if (adjustAccountFeatureIdResponse->getGameCode() == PlatformGameCode::SWG)
						LOG("CustomerService",("VeteranRewards: %s ***FAILED TO TRADE IN*** %s for SWG account feature Id %lu with OnGetFeatures() error code (%u, %s:%s)", adjustAccountFeatureIdResponse->getTargetPlayerDescription().c_str(), adjustAccountFeatureIdResponse->getTargetItemDescription().c_str(), adjustAccountFeatureIdResponse->getFeatureId(), result, sResultString.c_str(), sResultText.c_str()));
				}

				GameConnection * const gc = ConnectionServer::getGameConnection(adjustAccountFeatureIdResponse->getGameServer());
				if (gc)
				{
					adjustAccountFeatureIdResponse->setSessionResultCode(result, sResultString.c_str(), sResultText.c_str());
					gc->send(*adjustAccountFeatureIdResponse,true);
				}
			}
			else
			{
				LoginAPI::Feature const * newlyAddedFeature = nullptr;

				for (unsigned k = 0; k < featureCount; ++k)
				{
					if (featureArray[k].GetID() == adjustAccountFeatureIdResponse->getFeatureId())
					{
						newlyAddedFeature = &(featureArray[k]);
						break;
					}
				}

				if (newlyAddedFeature)
				{
					LoginAPI::Feature updatedFeature;
					updatedFeature.SetID(newlyAddedFeature->GetID());
					updatedFeature.SetData(newlyAddedFeature->GetData());
					updatedFeature.SetParameter("count", adjustAccountFeatureIdResponse->getNewValue());

					if (adjustAccountFeatureIdResponse->getGameCode() == PlatformGameCode::SWGTCG)
						updatedFeature.SetActive(true);

					apiTrackingNumber const tn = ModifyFeature_v2(adjustAccountFeatureIdResponse->getTargetStationId(), PlatformGameCode::getGamecodeName(adjustAccountFeatureIdResponse->getGameCode()).c_str(), *newlyAddedFeature, updatedFeature);
					ms_modifyFeatureTrackingNumberMap[tn] = adjustAccountFeatureIdResponse;
					reuseMessage = true;
				}
				else
				{
					// CS log SWG TCG or reward trade in account feature grant failure
					if (!adjustAccountFeatureIdResponse->getTargetPlayerDescription().empty() && adjustAccountFeatureIdResponse->getTargetItem().isValid() && !adjustAccountFeatureIdResponse->getTargetItemDescription().empty())
					{
						if (adjustAccountFeatureIdResponse->getGameCode() == PlatformGameCode::SWGTCG)
							LOG("CustomerService",("TcgRedemption: %s ***FAILED TO REDEEM*** %s for SWGTCG account feature Id %lu with OnGetFeatures() error code (MISSING_NEWLY_ADDED_FEATURE_ID:The feature Id was just successfully added, but now it's gone)", adjustAccountFeatureIdResponse->getTargetPlayerDescription().c_str(), adjustAccountFeatureIdResponse->getTargetItemDescription().c_str(), adjustAccountFeatureIdResponse->getFeatureId()));
						else if (adjustAccountFeatureIdResponse->getGameCode() == PlatformGameCode::SWG)
							LOG("CustomerService",("VeteranRewards: %s ***FAILED TO TRADE IN*** %s for SWG account feature Id %lu with OnGetFeatures() error code (MISSING_NEWLY_ADDED_FEATURE_ID:The feature Id was just successfully added, but now it's gone)", adjustAccountFeatureIdResponse->getTargetPlayerDescription().c_str(), adjustAccountFeatureIdResponse->getTargetItemDescription().c_str(), adjustAccountFeatureIdResponse->getFeatureId()));
					}

					// this situation shouldn't happen, as we just successfully added the feature Id,
					// and now it's not there anymore when we retrieve it;
					// just pick some generic session error message to indicate this situation
					GameConnection * const gc = ConnectionServer::getGameConnection(adjustAccountFeatureIdResponse->getGameServer());
					if (gc)
					{
						adjustAccountFeatureIdResponse->setSessionResultCode(RESULT_CANCELLED, "MISSING_NEWLY_ADDED_FEATURE_ID", "The feature Id was just successfully added, but now it's gone");
						gc->send(*adjustAccountFeatureIdResponse,true);
					}
				}
			}
		}
		else if (claimRewardsMessage)
		{
			if (result != RESULT_SUCCESS)
			{
				GameConnection * const gc = ConnectionServer::getGameConnection(claimRewardsMessage->getGameServer());
				if (gc)
				{
					ClaimRewardsReplyMessage const rsp(claimRewardsMessage->getGameServer(), claimRewardsMessage->getStationId(), claimRewardsMessage->getPlayer(), claimRewardsMessage->getRewardEvent(), claimRewardsMessage->getRewardItem(), claimRewardsMessage->getAccountFeatureId(), claimRewardsMessage->getConsumeAccountFeatureId(), 0, 0, false);
					gc->send(rsp,true);
				}
			}
			else
			{
				// see if account already has the required feature
				LoginAPI::Feature const * existingFeature = nullptr;

				for (unsigned k = 0; k < featureCount; ++k)
				{
					if (featureArray[k].GetID() == claimRewardsMessage->getAccountFeatureId())
					{
						existingFeature = &(featureArray[k]);
						break;
					}
				}

				if (existingFeature)
				{
					int const currentCount = existingFeature->GetConsumeCount();
					if (currentCount > 0)
					{
						// if the reward consumes the account feature Id, or
						// is "once per account", then forward message to
						// LoginServer to complete the transaction; otherwise,
						// verification is complete, so send success response
						if (claimRewardsMessage->getConsumeAccountFeatureId() || claimRewardsMessage->getConsumeEvent() || claimRewardsMessage->getConsumeItem())
						{
							int const updatedCount = (claimRewardsMessage->getConsumeAccountFeatureId() ? (currentCount - 1) : currentCount);

							LoginAPI::Feature updatedFeature;
							updatedFeature.SetID(existingFeature->GetID());
							updatedFeature.SetData(existingFeature->GetData());
							updatedFeature.SetParameter("count", updatedCount);

							// forward message to LoginServer to complete the transaction
							claimRewardsMessage->setAccountFeatureIdOldValue(existingFeature->GetData());
							claimRewardsMessage->setAccountFeatureIdNewValue(updatedFeature.GetData());

							CentralConnection * const cc = ConnectionServer::getCentralConnection();
							if (cc)
								cc->send(*claimRewardsMessage, true);
						}
						else
						{
							GameConnection * const gc = ConnectionServer::getGameConnection(claimRewardsMessage->getGameServer());
							if (gc)
							{
								ClaimRewardsReplyMessage const rsp(claimRewardsMessage->getGameServer(), claimRewardsMessage->getStationId(), claimRewardsMessage->getPlayer(), claimRewardsMessage->getRewardEvent(), claimRewardsMessage->getRewardItem(), claimRewardsMessage->getAccountFeatureId(), claimRewardsMessage->getConsumeAccountFeatureId(), currentCount, currentCount, true);
								gc->send(rsp,true);
							}
						}
					}
					else
					{
						GameConnection * const gc = ConnectionServer::getGameConnection(claimRewardsMessage->getGameServer());
						if (gc)
						{
							// if the account feature id indicates the account no longer
							// qualifies for the reward, then there must be a mismatch between
							// the account feature id cache in the game server, and what's
							// really on the account, so send this fresh set of the account
							// feature id to the game server
							std::map<uint32, int> featureIds;
							std::map<uint32, std::string> featureIdsData;

							for (unsigned k = 0; k < featureCount; ++k)
							{
								featureIdsData[featureArray[k].GetID()] = featureArray[k].GetData();
								featureIds[featureArray[k].GetID()] = featureArray[k].GetConsumeCount();
							}

							AccountFeatureIdResponse const updatedFeatureIds(NetworkId::cms_invalid, claimRewardsMessage->getGameServer(), claimRewardsMessage->getPlayer(), claimRewardsMessage->getStationId(), PlatformGameCode::SWG, AccountFeatureIdRequest::RR_Reload, result, true, featureIds, featureIdsData, sResultString.c_str(), sResultText.c_str());
							gc->send(updatedFeatureIds,true);

							ClaimRewardsReplyMessage const rsp(claimRewardsMessage->getGameServer(), claimRewardsMessage->getStationId(), claimRewardsMessage->getPlayer(), claimRewardsMessage->getRewardEvent(), claimRewardsMessage->getRewardItem(), claimRewardsMessage->getAccountFeatureId(), claimRewardsMessage->getConsumeAccountFeatureId(), 0, 0, false);
							gc->send(rsp,true);
						}
					}
				}
				else
				{
					GameConnection * const gc = ConnectionServer::getGameConnection(claimRewardsMessage->getGameServer());
					if (gc)
					{
						// if the account feature id indicates the account no longer
						// qualifies for the reward, then there must be a mismatch between
						// the account feature id cache in the game server, and what's
						// really on the account, so send this fresh set of the account
						// feature id to the game server
						std::map<uint32, int> featureIds;
						std::map<uint32, std::string> featureIdsData;

						for (unsigned k = 0; k < featureCount; ++k)
						{
							featureIdsData[featureArray[k].GetID()] = featureArray[k].GetData();
							featureIds[featureArray[k].GetID()] = featureArray[k].GetConsumeCount();
						}

						AccountFeatureIdResponse const updatedFeatureIds(NetworkId::cms_invalid, claimRewardsMessage->getGameServer(), claimRewardsMessage->getPlayer(), claimRewardsMessage->getStationId(), PlatformGameCode::SWG, AccountFeatureIdRequest::RR_Reload, result, true, featureIds, featureIdsData, sResultString.c_str(), sResultText.c_str());
						gc->send(updatedFeatureIds,true);

						ClaimRewardsReplyMessage const rsp(claimRewardsMessage->getGameServer(), claimRewardsMessage->getStationId(), claimRewardsMessage->getPlayer(), claimRewardsMessage->getRewardEvent(), claimRewardsMessage->getRewardItem(), claimRewardsMessage->getAccountFeatureId(), claimRewardsMessage->getConsumeAccountFeatureId(), 0, 0, false);
						gc->send(rsp,true);
					}
				}
			}
		}

		if (!reuseMessage)
			delete i->second;

		ms_getFeaturesTrackingNumberMap.erase(i);
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("SessionApiClient::OnGetFeatures() - could not find session id for tracking number [%u]\n", trackingNumber));
	}
}

//------------------------------------------------------------

void SessionApiClient::OnGrantFeature(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData)
{
	UNREF(trackingNumber);
	UNREF(result);
	UNREF(userData);
//	DEBUG_REPORT_LOG(true, ("Grant feature.\n"));
}

//------------------------------------------------------------

void SessionApiClient::OnGrantFeatureByStationID(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData)
{
	UNREF(userData);

	const char * const resultString = ResultString[result];
	std::string sResultString;
	if (resultString)
	{
		sResultString = resultString;
	}
	else
	{
		char buffer[32];
		snprintf(buffer, sizeof(buffer)-1, "%u", result);
		buffer[sizeof(buffer)-1] = '\0';

		sResultString = buffer;
	}

	const char * const resultText = ResultText[result];
	std::string sResultText;
	if (resultText)
	{
		sResultText = resultText;
	}
	else
	{
		char buffer[32];
		snprintf(buffer, sizeof(buffer)-1, "%u", result);
		buffer[sizeof(buffer)-1] = '\0';

		sResultText = buffer;
	}

	DEBUG_REPORT_LOG(true, ("SessionApiClient::OnGrantFeatureByStationID() - [%u][%u][%s][%s]\n", trackingNumber, result, sResultString.c_str(), sResultText.c_str()));

	std::map<apiTrackingNumber, AdjustAccountFeatureIdResponse *>::iterator i = ms_grantFeatureTrackingNumberMap.find(trackingNumber);
	if (i != ms_grantFeatureTrackingNumberMap.end())
	{
		if (result != RESULT_SUCCESS)
		{
			// CS log SWG TCG or reward trade in account feature grant failure
			if (!i->second->getTargetPlayerDescription().empty() && i->second->getTargetItem().isValid() && !i->second->getTargetItemDescription().empty())
			{
				if (i->second->getGameCode() == PlatformGameCode::SWGTCG)
					LOG("CustomerService",("TcgRedemption: %s ***FAILED TO REDEEM*** %s for SWGTCG account feature Id %lu with OnGrantFeatureByStationID() error code (%u, %s:%s)", i->second->getTargetPlayerDescription().c_str(), i->second->getTargetItemDescription().c_str(), i->second->getFeatureId(), result, sResultString.c_str(), sResultText.c_str()));
				else if (i->second->getGameCode() == PlatformGameCode::SWG)
					LOG("CustomerService",("VeteranRewards: %s ***FAILED TO TRADE IN*** %s for SWG account feature Id %lu with OnGrantFeatureByStationID() error code (%u, %s:%s)", i->second->getTargetPlayerDescription().c_str(), i->second->getTargetItemDescription().c_str(), i->second->getFeatureId(), result, sResultString.c_str(), sResultText.c_str()));
			}

			GameConnection * const gc = ConnectionServer::getGameConnection(i->second->getGameServer());
			if (gc)
			{
				i->second->setSessionResultCode(result, sResultString.c_str(), sResultText.c_str());
				gc->send(*(i->second),true);
			}

			delete i->second;
		}
		else
		{
			apiTrackingNumber const tn = GetFeatures(i->second->getTargetStationId(), i->second->getGameCode());
			ms_getFeaturesTrackingNumberMap[tn] = i->second;
		}

		ms_grantFeatureTrackingNumberMap.erase(i);
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("SessionApiClient::OnGrantFeatureByStationID() - could not find session id for tracking number [%u]\n", trackingNumber));
	}
}

//------------------------------------------------------------

void SessionApiClient::OnModifyFeature(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData)
{
	UNREF(trackingNumber);
	UNREF(result);
	UNREF(userData);
//	DEBUG_REPORT_LOG(true, ("Modify feature.\n"));
}

//------------------------------------------------------------

void SessionApiClient::OnModifyFeature_v2(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const LoginAPI::Feature & currentFeature,
                                            void * userData)
{
	UNREF(currentFeature);
	UNREF(userData);

	const char * const resultString = ResultString[result];
	std::string sResultString;
	if (resultString)
	{
		sResultString = resultString;
	}
	else
	{
		char buffer[32];
		snprintf(buffer, sizeof(buffer)-1, "%u", result);
		buffer[sizeof(buffer)-1] = '\0';

		sResultString = buffer;
	}

	const char * const resultText = ResultText[result];
	std::string sResultText;
	if (resultText)
	{
		sResultText = resultText;
	}
	else
	{
		char buffer[32];
		snprintf(buffer, sizeof(buffer)-1, "%u", result);
		buffer[sizeof(buffer)-1] = '\0';

		sResultText = buffer;
	}

	DEBUG_REPORT_LOG(true, ("SessionApiClient::OnModifyFeature_v2() - [%u][%u][%s][%s]\n", trackingNumber, result, sResultString.c_str(), sResultText.c_str()));

	std::map<apiTrackingNumber, AdjustAccountFeatureIdResponse *>::iterator i = ms_modifyFeatureTrackingNumberMap.find(trackingNumber);
	if (i != ms_modifyFeatureTrackingNumberMap.end())
	{
		if (result != RESULT_SUCCESS)
		{
			// CS log SWG TCG or reward trade in account feature grant failure
			if (!i->second->getTargetPlayerDescription().empty() && i->second->getTargetItem().isValid() && !i->second->getTargetItemDescription().empty())
			{
				if (i->second->getGameCode() == PlatformGameCode::SWGTCG)
					LOG("CustomerService",("TcgRedemption: %s ***FAILED TO REDEEM*** %s for SWGTCG account feature Id %lu with OnModifyFeature_v2() error code (%u, %s:%s)", i->second->getTargetPlayerDescription().c_str(), i->second->getTargetItemDescription().c_str(), i->second->getFeatureId(), result, sResultString.c_str(), sResultText.c_str()));
				else if (i->second->getGameCode() == PlatformGameCode::SWG)
					LOG("CustomerService",("VeteranRewards: %s ***FAILED TO TRADE IN*** %s for SWG account feature Id %lu with OnModifyFeature_v2() error code (%u, %s:%s)", i->second->getTargetPlayerDescription().c_str(), i->second->getTargetItemDescription().c_str(), i->second->getFeatureId(), result, sResultString.c_str(), sResultText.c_str()));
			}
		}
		else
		{
			// CS log SWG TCG or reward trade in account feature grant
			if (!i->second->getTargetPlayerDescription().empty() && i->second->getTargetItem().isValid() && !i->second->getTargetItemDescription().empty())
			{
				if (i->second->getGameCode() == PlatformGameCode::SWGTCG)
					LOG("CustomerService",("TcgRedemption: %s redeemed %s for SWGTCG account feature Id %lu (%d -> %d)", i->second->getTargetPlayerDescription().c_str(), i->second->getTargetItemDescription().c_str(), i->second->getFeatureId(), i->second->getOldValue(), i->second->getNewValue()));
				else if (i->second->getGameCode() == PlatformGameCode::SWG)
					LOG("CustomerService",("VeteranRewards: %s traded in %s for SWG account feature Id %lu (%d -> %d)", i->second->getTargetPlayerDescription().c_str(), i->second->getTargetItemDescription().c_str(), i->second->getFeatureId(), i->second->getOldValue(), i->second->getNewValue()));
			}
		}

		GameConnection * const gc = ConnectionServer::getGameConnection(i->second->getGameServer());
		if (gc)
		{
			i->second->setSessionResultCode(result, sResultString.c_str(), sResultText.c_str());
			gc->send(*(i->second),true);
		}

		delete i->second;
		ms_modifyFeatureTrackingNumberMap.erase(i);
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("SessionApiClient::OnModifyFeature_v2() - could not find session id for tracking number [%u]\n", trackingNumber));
	}
}

//------------------------------------------------------------

void SessionApiClient::OnRevokeFeature(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData)
{
	UNREF(trackingNumber);
	UNREF(result);
	UNREF(userData);
//	DEBUG_REPORT_LOG(true, ("Revoke feature.\n"));
}

//------------------------------------------------------------

void SessionApiClient::OnEnumerateFeatures(const apiTrackingNumber trackingNumber, 
                                            const apiResult result,
                                            const unsigned featureCount,
                                            const LoginAPI::FeatureDescription featureArray[],
                                            const void * userData)
{
	UNREF(trackingNumber);
	UNREF(result);
	UNREF(featureCount);
	UNREF(featureArray);
	UNREF(userData);
//	DEBUG_REPORT_LOG(true, ("Enumerate features.\n"));
}

//------------------------------------------------------------

void SessionApiClient::getFeatures(apiAccountId accountId, apiGamecode gameCode, GameNetworkMessage* gnm)
{
	if (!gnm)
		return;

	apiTrackingNumber const tn = GetFeatures(accountId, gameCode);
	ms_getFeaturesTrackingNumberMap[tn] = gnm;
}

//------------------------------------------------------------

void SessionApiClient::dropClient(const ClientConnection* client, bool forceSessionLogout)
{
	std::map<apiTrackingNumber, ClientConnection*>::iterator i = m_validationMap.begin();
	for (;i != m_validationMap.end(); ++i)
	{
		if (i->second == client)
		{
			m_validationMap.erase(i);
			break;
		}
	}

	const std::string& sessionId = client->getSessionId();
	if (!sessionId.empty())
	{
		std::map<std::string, ClientConnection *>::iterator j = ms_sessionIdMap.find(sessionId);
		if (j != ms_sessionIdMap.end())
			ms_sessionIdMap.erase(j);
		
		if (!ConfigConnectionServer::getDisableSessionLogout() || forceSessionLogout)
		{
			SessionLogout(sessionId.c_str());
		}
	}
}


//------------------------------------------------------------

void SessionApiClient::validateClient (ClientConnection* client, const std::string & key)
{

		//Key will be the real session key
		//We will only use id if we aren't validating.
		//The key will provide a username and an id from the station.

		//We call SessionValidate(key, type)  on the Session client
		//It will callback with OnSessionValidate(trackingNumber, result, account, subscription, userdata);

		//type will be ConfigConnectionServer::getSessionType()
		//result is hopefully RESULT_SUCCESS
		//acount is struct (name, id, status) where status is ACCOUNT_STATUS_ACTIVE and id is unsigned
		//subscription is hopefully SUBSCRIPTION_STATUS_ACTIVE could be trial
		//user data is a void* and I think un-used.

		//Then store the client in a map based on the key.

	apiTrackingNumber track = SessionValidate(key.c_str(), static_cast<apiSessionType>(ConfigConnectionServer::getSessionType()));
	//apiTrackingNumber track = SessionConsume(key.c_str(), static_cast<apiSessionType>(ConfigConnectionServer::getSessionType()));

	//Ok to overwrite old or add new here.
	m_validationMap[track] = client;
}

//------------------------------------------------------------

void SessionApiClient::startPlay(const ClientConnection& client)
{
	IGNORE_RETURN(SessionStartPlay(client.getSessionId().c_str(), ConfigConnectionServer::getClusterName(), client.getCharacterName().c_str(), nullptr));
}

//------------------------------------------------------------

void SessionApiClient::stopPlay(const ClientConnection& client)
{
	IGNORE_RETURN(SessionStopPlay(client.getSessionId().c_str(), ConfigConnectionServer::getClusterName(), client.getCharacterName().c_str()));
}

//------------------------------------------------------------
void SessionApiClient::OnConnectionOpened(const char * address, unsigned port)
{
	UNREF(address);
	UNREF(port);
//	DEBUG_REPORT_LOG(true, ("Connection success\n"));
}
void SessionApiClient::OnConnectionClosed(const char * address, unsigned port)
{
	UNREF(address);
	UNREF(port);
//	DEBUG_REPORT_LOG(true, ("Connection closed.\n"));
}
void SessionApiClient::OnConnectionFailed(const char * address, unsigned port)
{
	UNREF(address);
	UNREF(port);
//	DEBUG_FATAL(true, ("Connection failed"));
}
void SessionApiClient::OnException()
{
//	DEBUG_REPORT_LOG(true, ("Connection exception.\n"));
}


//------------------------------------------------------------------------------------------

void SessionApiClient::update()
{
	Process();
	if (m_sessionTimer.updateZero(Clock::frameTime()))
	{
		std::map<std::string, ClientConnection *>::iterator j = ms_sessionIdMap.begin();
		std::vector<const char*> sessionList;
		sessionList.reserve(ms_sessionIdMap.size());
		for (; j != ms_sessionIdMap.end(); ++j)
		{
			sessionList.push_back(j->first.c_str());
		}
		
		//SessionTouch all clients.
		SessionTouch(&sessionList[0], sessionList.size());
	}
}

//------------------------------------------------------------------------------------------

void SessionApiClient::NotifySessionKickRequest(const apiAccount & account, 
									  const apiSession & session,
									  const apiKickReason reason)
{
	UNREF(reason);
	UNREF(account);
	std::map<std::string, ClientConnection *>::iterator j = ms_sessionIdMap.find(session.GetId());
	if (j != ms_sessionIdMap.end())
	{
		SessionKickReply(session.GetId(), KICK_REPLY_ALLOW);
		LOG("ClientDisconnect", ("Client %s Disconnected by session kick request", j->second->getSessionId().c_str()));
		j->second->disconnect();
	}
	else
	{
		SessionKickReply(session.GetId(), KICK_REPLY_UNKNOWN_SESSION);
	}
}

//------------------------------------------------------------------------------------------

void SessionApiClient::NotifySessionKick(const char ** sessionList,
							   const unsigned sessionCount)
{
	for (unsigned i = 0; i < sessionCount; ++i)
	{
		std::map<std::string, ClientConnection *>::iterator j = ms_sessionIdMap.find(sessionList[i]);
		if (j != ms_sessionIdMap.end())
		{
			LOG("ClientDisconnect", ("Client %s Disconnected by notify session kick", j->second->getSessionId().c_str()));
			j->second->disconnect();
		}
	}
}

//------------------------------------------------------------------------------------------



