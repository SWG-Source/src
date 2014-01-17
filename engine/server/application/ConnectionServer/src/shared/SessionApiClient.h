// SessionApiClient.h
// copyright 2002 Sony Online Entertainment

#ifndef	_SessionApiClient_H
#define	_SessionApiClient_H

#include <map>
#pragma warning(push)
#pragma warning(disable: 4100) // Client.h has inlined functions with unreferenced formal parameters
#include "Session/LoginAPI/Client.h"
#pragma warning(pop)

#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Timer.h"


class ClientConnection;

class SessionApiClient : public LoginAPI::Client
{
public:

	SessionApiClient(const char ** serverList, int serverCount);
	~SessionApiClient();

	virtual void OnSessionLogin(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const LoginAPI::UsageLimit & usageLimit,
                                            const LoginAPI::Entitlement & entitlement,
                                            void * userData);

	virtual void OnSessionLoginInternal(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const LoginAPI::UsageLimit & usageLimit,
                                            const LoginAPI::Entitlement & entitlement,
                                            void * userData);

	virtual void OnSessionValidate(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const LoginAPI::UsageLimit & usageLimit,
                                            const LoginAPI::Entitlement & entitlement,
                                            void * userData);

	virtual void OnSessionConsume(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const LoginAPI::UsageLimit & usageLimit,
                                            const LoginAPI::Entitlement & entitlement,
                                            void * userData);

	virtual void OnSessionStartPlay(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData);

	virtual void OnSessionStopPlay(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData);

	virtual void OnSessionKick(const unsigned trackingNumber, 
                                            const apiResult result,
                                            void * userData);

	virtual void OnGetSessions(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const unsigned count, 
                                            const apiSession session[], 
                                            const apiSubscription subscription[], 
                                            const LoginAPI::UsageLimit usageLimit[],
                                            const unsigned timeCreated[],
                                            const unsigned timeTouched[],
                                            void * userData);

	virtual void OnGetFeatures(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const unsigned featureCount,
                                            const LoginAPI::Feature featureArray[],
                                            const void * userData = 0);

	virtual void OnGrantFeature(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData);

	virtual void OnGrantFeatureByStationID(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData);

	virtual void OnModifyFeature(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData);

	virtual void OnModifyFeature_v2(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
											const LoginAPI::Feature & currentFeature,
                                            void * userData);

	virtual void OnRevokeFeature(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData);

	virtual void OnEnumerateFeatures(const apiTrackingNumber trackingNumber, 
                                            const apiResult result,
                                            const unsigned featureCount,
                                            const LoginAPI::FeatureDescription featureArray[],
                                            const void * userData = 0);
	
	virtual void OnConnectionOpened(const char * address, unsigned port);
	virtual void OnConnectionClosed(const char * address, unsigned port);
	virtual void OnConnectionFailed(const char * address, unsigned port);
	virtual void OnException();

	virtual void NotifySessionKickRequest(const apiAccount & account, 
										  const apiSession & session,
										  const apiKickReason reason);
	
	virtual void NotifySessionKick(const char ** sessionList,
								   const unsigned sessionCount);
	
	void         getFeatures(apiAccountId accountId, apiGamecode gameCode, GameNetworkMessage* gnm);
	void         dropClient(const ClientConnection* client, bool forceSessionLogout);
	void         validateClient(ClientConnection* client, const std::string & key);
	void         startPlay(const ClientConnection& client);
	void         stopPlay(const ClientConnection& client);
	void         update();
	
	static void FlushSessionQueue();
	
private:

	static std::map<apiTrackingNumber, ClientConnection *> m_validationMap;
	SessionApiClient();
	Timer  m_sessionTimer;
};


#endif
