#ifndef SESSION_LOGIN_API__CLIENT_CORE_H
#define SESSION_LOGIN_API__CLIENT_CORE_H


#include "Session/CommonAPI/CommonClient.h"
#include "Client.h"


namespace LoginAPI
{


    class ClientCore : public apiCore
    {
        public:
            ClientCore(apiClient * parent, const char * version, const char * serverList, const char * description, unsigned maxConnections);
            ClientCore(apiClient * parent, const char * version, const char ** serverList, unsigned serverCount, const char * description, unsigned maxConnections);
            virtual ~ClientCore();  

            Client *            GetClient() { return static_cast<Client *>(GetParent()); }

            virtual int         GetKeepAliveDelay();

        ////////////////////////////////////////
        //  Requests
        public:
            apiTrackingNumber   GetAccountStatus(const char * name, 
                                            const char * password,
                                            const void * userData = 0);
            apiTrackingNumber   GetAccountStatus(const apiAccountId accountId,
                                            const void * userData = 0);
            apiTrackingNumber   GetAccountSubscriptions(const char * name, 
                                            const char * password,
                                            const void * userData = 0);
            apiTrackingNumber   GetAccountSubscriptions(const char * sessionID,
                                            const void * userData = 0);
            apiTrackingNumber   GetAccountSubscriptions(const apiAccountId accountId,
                                            const void * userData = 0);
            apiTrackingNumber   GetAccountSubscription(const char * name, 
                                            const char * password,
                                            const apiGamecode gamecode,
                                            const void * userData = 0);
            apiTrackingNumber   GetAccountSubscription(const char * sessionID,
                                            const apiGamecode gamecode,
                                            const void * userData = 0);
            apiTrackingNumber   GetAccountSubscription(const apiAccountId accountId,
                                            const apiGamecode gamecode,
                                            const void * userData = 0);
            apiTrackingNumber   SessionLogin_v3(const char * name, 
                                            const char * password, 
                                            const apiSessionType sessionType, 
                                            const apiIP clientIP,
                                            const bool forceLogin,
                                            const void * userData = 0);
            apiTrackingNumber   SessionLogin_v3(const char * parentSessionID, 
                                            const apiSessionType sessionType, 
                                            const apiIP clientIP,
                                            const bool forceLogin,
                                            const void * userData = 0);
            apiTrackingNumber   SessionLogin_v4(const char * name, 
                                            const char * password, 
                                            const apiSessionType sessionType, 
                                            const apiIP clientIP,
                                            const bool forceLogin,
                                            const void * userData = 0);
            apiTrackingNumber   SessionLogin_v4(const char * parentSessionID, 
                                            const apiSessionType sessionType, 
                                            const apiIP clientIP,
                                            const bool forceLogin,
                                            const void * userData = 0);
            apiTrackingNumber	SessionLogin_v5(const char * name, 
                                            const char * password, 
                                            const apiSessionType sessionType, 
                                            const apiIP clientIP,
                                            const unsigned flags,
                                            const void * userData = 0);
            apiTrackingNumber	SessionLogin_v5(const char * parentSessionID, 
                                            const apiSessionType sessionType, 
                                            const apiIP clientIP,
                                            const unsigned flags,
                                            const void * userData = 0);
			apiTrackingNumber	SessionLogin(const char * name, 
                                            const char * password, 
                                            const apiSessionType sessionType, 
                                            const apiIP clientIP,
                                            const unsigned flags,
											const char * sessionNamespace = _defaultNamespace,
                                            const void * userData = 0);
            apiTrackingNumber	SessionLogin(const char * parentSessionID, 
                                            const apiSessionType sessionType, 
                                            const apiIP clientIP,
                                            const unsigned flags,
                                            const void * userData = 0);
			apiTrackingNumber	SessionLoginInternal(const apiAccountId accountId, 
                                            const apiSessionType sessionType, 
                                            const apiIP clientIP,
                                            const unsigned flags,
                                            const void * userData = 0);
            apiTrackingNumber   SessionConsume_v2(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            apiTrackingNumber   SessionConsume_v3(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            apiTrackingNumber   SessionConsume(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            apiTrackingNumber   SessionValidate_v2(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            apiTrackingNumber   SessionValidate_v3(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            apiTrackingNumber   SessionValidate(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            apiTrackingNumber   SessionValidateEx(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            apiTrackingNumber   SubscriptionValidate(const char * sessionID,
                                            const apiGamecode gamecode, 
                                            const void * userData = 0);

            void                SessionLogout(const char * sessionID);
            void                SessionLogout(const char ** sessionList,
                                            const unsigned sessionCount);
            void                SessionTouch(const char * sessionID);
            void                SessionTouch(const char ** sessionList,
                                            const unsigned sessionCount);

            apiTrackingNumber   GetMemberInformation(const apiAccountId accountId,
													const void * userData = 0);

			apiTrackingNumber   GetMemberInformation_v2(const apiAccountId accountId,
													const void * userData = 0);

            void                SessionKickReply(const char * sessionID, 
                                            apiKickReply reply);

            apiTrackingNumber   GetSessions(const apiAccountId accountId,
                                            const void * userData = 0);
            
			apiTrackingNumber	GetFeatures(const char * sessionID,
                                            const void * userData = 0);
			apiTrackingNumber	GetFeatures(const apiAccountId accountId,
											const apiGamecode gamecode,
                                            const void * userData = 0);
			apiTrackingNumber	GrantFeature(const char * sessionID,
											unsigned featureType,
                                            const void * userData = 0);
            apiTrackingNumber	ModifyFeature(const char * sessionID,
											const Feature & feature,
                                            const void * userData = 0);
            apiTrackingNumber	RevokeFeature(const char * sessionID,
											unsigned featureType,
                                            const void * userData = 0);
            apiTrackingNumber   EnumerateFeatures(apiGamecode, 
											const void * userData = 0);
			apiTrackingNumber	SessionStartPlay(const char * sessionId,
											const char * serverName, 
											const char * characterName, 
											const char * gameData,
											const void * userData = 0);
			apiTrackingNumber	SessionStopPlay(const char * sessionId, 
											const char * serverName, 
											const char * characterName,
											const void * userData = 0);
            apiTrackingNumber   SessionKick(const char * sessionID, 
                                            apiKickReason reason,
                                            const void * userData = 0);

			apiTrackingNumber GrantFeature_v2(const char * sessionID,
											  unsigned featureType,
											  const char * gameCode,
											  unsigned providerID, 
											  unsigned promoID, 
											  const void * userData = 0);

			apiTrackingNumber ModifyFeature_v2( unsigned accountID, 
												const char * gameCode,
												const Feature & origFeature,
												const Feature & newFeature,
												const void * userData = 0);

			apiTrackingNumber GrantFeatureByStationID(unsigned accountID,
													  unsigned featureType,
													  const char * gameCode,
													  unsigned providerID,
													  unsigned promoID,
													  const void * userData = 0);


        ////////////////////////////////////////
        //  Result Callbacks
        public:
            void                OnGetAccountStatus(const Base::ByteStream & stream, void * userData);
            void                OnGetAccountSubscriptions(const Base::ByteStream & stream, void * userData);
            void                OnGetAccountSubscription(const Base::ByteStream & stream, void * userData);
            void                OnSessionLogin_v3(const Base::ByteStream & stream, void * userData);
            void                OnSessionLogin_v4(const Base::ByteStream & stream, void * userData);
            void                OnSessionLogin_v5(const Base::ByteStream & stream, void * userData);
            void                OnSessionLogin(const Base::ByteStream & stream, void * userData);
			void                OnSessionLoginInternal(const Base::ByteStream & stream, void * userData);
            void                OnSessionValidate_v2(const Base::ByteStream & stream, void * userData);
            void                OnSessionValidate_v3(const Base::ByteStream & stream, void * userData);
            void                OnSessionValidate(const Base::ByteStream & stream, void * userData);
            void                OnSessionValidateEx(const Base::ByteStream & stream, void * userData);
            void                OnSessionConsume_v2(const Base::ByteStream & stream, void * userData);
            void                OnSessionConsume_v3(const Base::ByteStream & stream, void * userData);
            void                OnSessionConsume(const Base::ByteStream & stream, void * userData);
            void                OnSessionKick(const Base::ByteStream & stream);
            void                OnSubscriptionValidate(const Base::ByteStream & stream, void * userData);
			void                OnGetMemberInformation(const Base::ByteStream & stream, void * userData);
			void                OnGetMemberInformation_v2(const Base::ByteStream & stream, void * userData);
            void                OnGetSessions(const Base::ByteStream & stream, void * userData);
            void                OnGetFeatures(const Base::ByteStream & stream, void * userData);
            void                OnGrantFeature(const Base::ByteStream & stream, void * userData);
            void                OnModifyFeature(const Base::ByteStream & stream, void * userData);
            void                OnRevokeFeature(const Base::ByteStream & stream, void * userData);
            void				OnEnumerateFeatures(const Base::ByteStream & stream, void * userData);
			void				OnSessionStartPlay(const Base::ByteStream & stream, void * userData);
			void				OnSessionStopPlay(const Base::ByteStream & stream, void * userData);
			void				OnSessionKick(const Base::ByteStream & stream, void * userData);
			void                OnGrantFeatureV2(const Base::ByteStream & stream, void * userData);
            void                NotifySessionKickRequest(const Base::ByteStream & stream, void * userData);
			void                NotifySessionKick(const Base::ByteStream & stream, void * userData);
			void				OnModifyFeature_v2(const Base::ByteStream & stream, void * userData);
			void				OnGrantFeatureByStationID(const Base::ByteStream & stream, void * userData);


            virtual bool        Callback(Base::ByteStream & stream, void * userData);
            virtual void        Timeout(unsigned short messageId, apiTrackingNumber trackingNumber, void * userData);
    };


}


#endif

