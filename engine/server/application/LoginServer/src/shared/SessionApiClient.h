// SessionApiClient.h
// copyright 2002 Sony Online Entertainment

#ifndef	_SessionApiClient_H
#define	_SessionApiClient_H

#include <map>
#include "Session/LoginAPI/Client.h"

class ClaimRewardsMessage;
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

	virtual void OnGetAccountSubscription(const apiTrackingNumber trackingNumber, 
										  const apiResult result, 
										  const apiAccount & account, 
										  const apiSubscription & subscription,
										  void * userData);
      
	virtual void OnConnectionOpened(const char * address, unsigned port);
	virtual void OnConnectionClosed(const char * address, unsigned port);
	virtual void OnConnectionFailed(const char * address, unsigned port);
	virtual void OnException();

	virtual void NotifySessionKick(const char ** sessionList,
								   const unsigned sessionCount);

	void         handleClaimRewardsMessage(uint32 clusterId, ClaimRewardsMessage const * msg);
	void         dropClient(const ClientConnection* client) const;
	void         validateClient (ClientConnection* client, const std::string & key);
	void         loginClient(ClientConnection* client, const std::string & username, const std::string & password);
	void         checkStatusForPurge (StationId account);

	static void FlushSessionQueue();
	
private:

	static std::map<apiTrackingNumber, ClientConnection *> m_validationMap;
	SessionApiClient();
};


#endif
