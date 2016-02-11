// SessionApiClient.cpp
// copyright 2002 Sony Online Entertainment


#include "FirstLoginServer.h"
#include "SessionApiClient.h"

#include "ClientConnection.h"
#include "ConfigLoginServer.h"
#include "CSToolConnection.h"
#include "DatabaseConnection.h"
#include "LoginServer.h"
#include "PurgeManager.h"
#include "serverNetworkMessages/ClaimRewardsMessage.h"
#include "serverNetworkMessages/ClaimRewardsReplyMessage.h"
#include "Session/CommonAPI/CommonAPIStrings.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ErrorMessage.h"
#include "sharedSynchronization/Mutex.h"

#include <vector>

//------------------------------------------------------------------------------------------

std::map<apiTrackingNumber, ClientConnection *> SessionApiClient::m_validationMap;

//------------------------------------------------------------------------------------------

namespace SessionApiClientNamespace
{
	std::map<apiTrackingNumber, std::pair<uint32, ClaimRewardsMessage const *> > ms_modifyFeatureTrackingNumberMap;
};

using namespace SessionApiClientNamespace;

//------------------------------------------------------------

SessionApiClient::SessionApiClient(const char ** serverList, int serverCount) :
		Client(serverList, static_cast<unsigned int>(serverCount), "Starwars Login Server")
{
}

//------------------------------------------------------------

SessionApiClient::~SessionApiClient()
{
}

//------------------------------------------------------------

void SessionApiClient::OnSessionLogin(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const LoginAPI::UsageLimit & usageLimit,
                                            const LoginAPI::Entitlement & entitlement,
                                            void * userData)
{
	OnSessionValidate(trackingNumber, result, account, subscription, session, usageLimit, entitlement, userData);

	
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
	UNREF(usageLimit);
	UNREF(userData);

	std::map<apiTrackingNumber, ClientConnection*>::iterator i = m_validationMap.find(trackingNumber);
	DEBUG_REPORT_LOG(true, ("OnSessionValidate result: %d for suid: %d (entitlement total: %u/%u, since last login: %u/%u, reason unentitled: %d)\n", result, account.GetId(), entitlement.GetEntitledTime(), entitlement.GetTotalTime(), entitlement.GetEntitledTimeSinceLastLogin(), entitlement.GetTotalTimeSinceLastLogin(), static_cast<int>(entitlement.GetReasonUnentitled())));

	if (i != m_validationMap.end())
	{
		if (result == RESULT_SUCCESS)
		{
			LOG("LoginClientConnection", ("OnSessionValidate() for stationId (%u) at IP (%s), id (%s), apiTrackingNumber (%u), SUCCESS", account.GetId(), i->second->getRemoteAddress().c_str(), account.GetName(), trackingNumber));
			LoginServer::getInstance().onValidateClient(account.GetId(), account.GetName(), i->second, session.GetIsSecure(), session.GetId(), subscription.GetGameFeatures(), subscription.GetSubscriptionFeatures());
			m_validationMap.erase(i);
		}
		else
		{
			LOG("LoginClientConnection", ("OnSessionValidate() for stationId (%u) at IP (%s), id (%s), apiTrackingNumber (%u), VALIDATION FAILED (%u)", account.GetId(), i->second->getRemoteAddress().c_str(), account.GetName(), trackingNumber, result));
			ErrorMessage err("VALIDATION FAILED", "Your station Id was not valid. Wrong password? Account closed?");
			i->second->send(err, true);
			i->second->disconnect();
		}
	}
	else
	{
		// attempt to validate a CS Tool associated with this request.
		CSToolConnection::validateCSTool( ( uint32 )userData, result, session );
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
	UNREF(trackingNumber);
	UNREF(result);
	UNREF(featureCount);
	UNREF(featureArray);
	UNREF(userData);
//	DEBUG_REPORT_LOG(true, ("Get features.\n"));
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

	std::map<apiTrackingNumber, std::pair<uint32, ClaimRewardsMessage const *> >::iterator i = ms_modifyFeatureTrackingNumberMap.find(trackingNumber);
	if (i != ms_modifyFeatureTrackingNumberMap.end())
	{
		if (result != RESULT_SUCCESS)
		{
			// if failed to update feature id, send failure message
			LOG("CustomerService",("VeteranRewards: received non-success result code (%u) from session to adjust feature id %lu (%s -> %s) for account (%lu), character (%s), reward event (%s), reward item (%s), session tracking number (%u)", result, i->second.second->getAccountFeatureId(), i->second.second->getAccountFeatureIdOldValue().c_str(), i->second.second->getAccountFeatureIdNewValue().c_str(), i->second.second->getStationId(), i->second.second->getPlayer().getValueString().c_str(), i->second.second->getRewardEvent().c_str(), i->second.second->getRewardItem().c_str(), trackingNumber));

			ClaimRewardsReplyMessage const rsp(i->second.second->getGameServer(), i->second.second->getStationId(), i->second.second->getPlayer(), i->second.second->getRewardEvent(), i->second.second->getRewardItem(), i->second.second->getAccountFeatureId(), true, 0, 0, false);
			LoginServer::getInstance().sendToCluster(i->second.first, rsp);
		}
		else
		{
			// if feature id updated successfully, record transaction
			LoginAPI::Feature oldFeature;
			oldFeature.SetID(i->second.second->getAccountFeatureId());
			oldFeature.SetData(i->second.second->getAccountFeatureIdOldValue());

			LoginAPI::Feature newFeature;
			newFeature.SetID(i->second.second->getAccountFeatureId());
			newFeature.SetData(i->second.second->getAccountFeatureIdNewValue());

			DatabaseConnection::getInstance().claimRewards(i->second.first, i->second.second->getGameServer(), i->second.second->getStationId(), i->second.second->getPlayer(), i->second.second->getRewardEvent(), i->second.second->getConsumeEvent(), i->second.second->getRewardItem(), i->second.second->getConsumeItem(), i->second.second->getAccountFeatureId(), true, oldFeature.GetConsumeCount(), newFeature.GetConsumeCount());
		}

		delete i->second.second;
		ms_modifyFeatureTrackingNumberMap.erase(i);
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

void SessionApiClient::handleClaimRewardsMessage(uint32 clusterId, ClaimRewardsMessage const * msg)
{
	UNREF(AccountStatusString);
	UNREF(SessionTypeString);
	UNREF(GamecodeString);
	UNREF(SubscriptionStatusString);

	if (!msg)
		return;

	LoginAPI::Feature oldFeature;
	oldFeature.SetID(msg->getAccountFeatureId());
	oldFeature.SetData(msg->getAccountFeatureIdOldValue());

	LoginAPI::Feature newFeature;
	newFeature.SetID(msg->getAccountFeatureId());
	newFeature.SetData(msg->getAccountFeatureIdNewValue());

	apiTrackingNumber const tn = ModifyFeature_v2(msg->getStationId(), PlatformGameCode::getGamecodeName(PlatformGameCode::SWG).c_str(), oldFeature, newFeature);
	LOG("CustomerService",("VeteranRewards: requesting session to adjust feature id %lu (%s -> %s) for account (%lu), character (%s), reward event (%s), reward item (%s), session tracking number (%u)", msg->getAccountFeatureId(), msg->getAccountFeatureIdOldValue().c_str(), msg->getAccountFeatureIdNewValue().c_str(), msg->getStationId(), msg->getPlayer().getValueString().c_str(), msg->getRewardEvent().c_str(), msg->getRewardItem().c_str(), tn));
	ms_modifyFeatureTrackingNumberMap[tn] = std::make_pair(clusterId, msg);
}

//------------------------------------------------------------

void SessionApiClient::dropClient(const ClientConnection* client) const
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

}

//------------------------------------------------------------

void SessionApiClient::validateClient (ClientConnection* client, const std::string & key)
{

		//Key will be the real session key
		//We will only use id if we aren't validating.
		//The key will provide a username and an id from the station.

		//We call SessionValidate(key, type)  on the Session client
		//It will callback with OnSessionValidate(trackingNumber, result, account, subscription, userdata);

		//type will be ConfigLoginServer::getSessionType()
		//result is hopefully RESULT_SUCCESS
		//acount is struct (name, id, status) where status is ACCOUNT_STATUS_ACTIVE and id is unsigned
		//subscription is hopefully SUBSCRIPTION_STATUS_ACTIVE could be trial
		//user data is a void* and I think un-used.

		//Then store the client in a map based on the key.

	apiTrackingNumber track = SessionValidate(key.c_str(), static_cast<apiSessionType>(ConfigLoginServer::getSessionType()));
	//apiTrackingNumber track = SessionConsume(key.c_str(), static_cast<apiSessionType>(ConfigLoginServer::getSessionType()));

	LOG("LoginClientConnection", ("validateClient() for stationId (%lu) at IP (%s), key (%s), apiTrackingNumber (%u), validating session", client->getStationId(), client->getRemoteAddress().c_str(), key.c_str(), track));

	//Ok to overwrite old or add new here.
	m_validationMap[track] = client;
}

//------------------------------------------------------------

void SessionApiClient::loginClient (ClientConnection* client, const std::string& username, const std::string & password)
{
	apiTrackingNumber track = SessionLogin(username.c_str(), password.c_str(), static_cast<apiSessionType>(ConfigLoginServer::getSessionType()), 0, 0);

	LOG("LoginClientConnection", ("loginClient() for stationId (%lu) at IP (%s), key (%s), apiTrackingNumber (%u), validating session", client->getStationId(), client->getRemoteAddress().c_str(), password.c_str(), track));

	//Ok to overwrite old or add new here.
	m_validationMap[track] = client;
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

void SessionApiClient::NotifySessionKick(const char ** sessionList,
							   const unsigned sessionCount)
{
	UNREF(sessionList);
	UNREF(sessionCount);
//	DEBUG_REPORT_LOG(true, ("Session kick.\n"));
}

// ----------------------------------------------------------------------

void SessionApiClient::checkStatusForPurge(StationId account)
{
	GetAccountSubscription(account, PlatformGameCode::SWG, nullptr);
}

//------------------------------------------------------------------------------------------

void SessionApiClient::OnGetAccountSubscription(const apiTrackingNumber trackingNumber, 
												const apiResult result, 
												const apiAccount & account, 
												const apiSubscription & subscription,
												void * userData)
{
	// This assumes that the only reason this would be called would be to check the status
	// for the purge process.  If something else ever uses getAccountSubscription, this will
	// need to be changed.
	if (result==RESULT_SUCCESS)
		PurgeManager::onCheckStatusForPurge(account.GetId(), (account.GetStatus()==ACCOUNT_STATUS_ACTIVE));
}

// ======================================================================
