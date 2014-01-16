#ifndef SESSION_LOGIN_API__CLIENT_H
#define SESSION_LOGIN_API__CLIENT_H


#include <string>
#include "Session/CommonAPI/CommonAPI.h"


namespace LoginAPI
{


    class UsageLimit;
    class Feature;
	class FeatureDescription;
    class Entitlement;
    class Client : public apiClient
    {
        public:
            // server list is space delimited
            Client(const char * serverList, const char * description, unsigned maxConnections = 2);   
            // server list is c-style string array
            Client(const char ** serverList, unsigned serverCount, const char * description, unsigned maxConnections = 2);  
            virtual ~Client();

            virtual void Process();   

            ////////////////////////////////////////
            //  State Callbacks
            virtual void OnConnectionOpened(const char * address, unsigned connectionCount);
            virtual void OnConnectionClosed(const char * address, unsigned connectionCount);
            virtual void OnConnectionFailed(const char * address, unsigned connectionCount);

            
            ////////////////////////////////////////
            //  Requests
            apiTrackingNumber SessionLogin(const char * name, 
                                            const char * password, 
                                            const apiSessionType sessionType, 
                                            const apiIP clientIP,
                                            const unsigned flags,
											const char * sessionNamespace = _defaultNamespace,
                                            const void * userData = 0);
            apiTrackingNumber SessionLogin(const char * parentSessionID, 
                                            const apiSessionType sessionType, 
                                            const apiIP clientIP,
                                            const unsigned flags,
                                            const void * userData = 0);
			apiTrackingNumber SessionLoginInternal(const apiAccountId accountId, 
                                            const apiSessionType sessionType, 
                                            const apiIP clientIP,
                                            const unsigned flags,
                                            const void * userData = 0);
            apiTrackingNumber SessionConsume(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            apiTrackingNumber SessionValidate(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            void						  SessionLogout(const char * sessionID);
            void						  SessionTouch(const char * sessionID);

			apiTrackingNumber SessionStartPlay(const char * sessionID, 
											const char * serverName, 
											const char * characterName,
											const char * gameData,
											const void * userData = 0);
			apiTrackingNumber SessionStopPlay(const char * sessionID, 
											const char * serverName, 
											const char * characterName,
											const void * userData = 0);
            apiTrackingNumber SessionKick(const char * sessionID, 
                                            apiKickReason reason,
                                            const void * userData = 0);

            apiTrackingNumber GetSessions(const apiAccountId accountId,
                                            const void * userData = 0);
            apiTrackingNumber GetFeatures(const char * sessionID,
                                            const void * userData = 0);
			apiTrackingNumber GetFeatures(const apiAccountId accountId,
											const apiGamecode gamecode,
                                            const void * userData = 0);
            apiTrackingNumber GrantFeature(const char * sessionID,
                                            unsigned featureType,
											const void * userData = 0);

            apiTrackingNumber ModifyFeature(const char * sessionID,
                                            const Feature & feature,
                                            const void * userData = 0);



            apiTrackingNumber RevokeFeature(const char * sessionID,
                                            unsigned featureType,
                                            const void * userData = 0);
            apiTrackingNumber EnumerateFeatures(apiGamecode, 
											const void * userData = 0);
			
			apiTrackingNumber GrantFeature_v2(const char * sessionID,
											  unsigned featureType,
											  const char * gameCode,
											  unsigned providerID = 0,
											  unsigned promoID = 0,
											  const void * userData = 0);

			apiTrackingNumber ModifyFeature_v2( unsigned accountID, 
												const char * gameCode,
												const Feature & origFeature,
												const Feature & newFeature,
												const void * userData = 0);

			apiTrackingNumber GrantFeatureByStationID(unsigned accountID,
													  unsigned featureType,
													  const char * gameCode,
													  unsigned providerID = 0,
													  unsigned promoID = 0,
													  const void * userData = 0);


            ////////////////////////////////////////
            //  Result Callbacks
            virtual void OnSessionLogin(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const UsageLimit & usageLimit,
                                            const Entitlement & entitlement,
                                            void * userData) = 0;
			virtual void OnSessionLoginInternal(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const UsageLimit & usageLimit,
                                            const Entitlement & entitlement,
                                            void * userData) = 0;
            virtual void OnSessionValidate(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const UsageLimit & usageLimit,
                                            const Entitlement & entitlement,
                                            void * userData) = 0;
            virtual void OnSessionConsume(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const UsageLimit & usageLimit,
                                            const Entitlement & entitlement,
                                            void * userData) = 0;
			virtual void OnSessionStartPlay(const apiTrackingNumber trackingNumber, 
											const apiResult result, 
											void * userData) = 0;
			virtual void OnSessionStopPlay(const apiTrackingNumber trackingNumber, 
											const apiResult result, 
											void * userData) = 0;
            virtual void OnSessionKick(const unsigned trackingNumber, 
                                            const apiResult result,
                                            void * userData) = 0;
            virtual void OnGetSessions(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const unsigned count, 
                                            const apiSession session[], 
                                            const apiSubscription subscription[], 
                                            const UsageLimit usageLimit[],
                                            const unsigned timeCreated[],
                                            const unsigned timeTouched[],
                                            void * userData) = 0;
            virtual void OnGetFeatures(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
											const unsigned featureCount,
											const Feature featureArray[],
                                            const void * userData = 0) = 0;
            virtual void OnGrantFeature(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData) = 0;
            virtual void OnModifyFeature(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData) = 0;
            virtual void OnRevokeFeature(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            void * userData) = 0;
            virtual void OnEnumerateFeatures(const apiTrackingNumber trackingNumber, 
                                            const apiResult result,
											const unsigned featureCount,
											const FeatureDescription featureArray[],
											const void * userData = 0) = 0;
            virtual void NotifySessionKick(const char ** sessionList,
                                            const unsigned sessionCount) = 0;
    
            ////////////////////////////////////////
            //  Call support for backwards compatability
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
            apiTrackingNumber   SessionConsume_v2(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            apiTrackingNumber   SessionConsume_v3(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            apiTrackingNumber   SessionValidate_v2(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            apiTrackingNumber   SessionValidate_v3(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            apiTrackingNumber   SessionValidateEx(const char * sessionID,
                                            const apiSessionType sessionType, 
                                            const void * userData = 0);
            apiTrackingNumber   SubscriptionValidate(const char * sessionID,
                                            const apiGamecode gamecode, 
                                            const void * userData = 0);
            void				SessionLogout(const char ** sessionList,
                                            const unsigned sessionCount);
            void				SessionTouch(const char ** sessionList,
                                            const unsigned sessionCount);
            apiTrackingNumber   GetMemberInformation(const apiAccountId accountId,
											const void * userData = 0);
			apiTrackingNumber   GetMemberInformation_v2(const apiAccountId accountId,
											const void * userData = 0);

            void                SessionKickReply(const char * sessionID, 
                                            apiKickReply reply);

            virtual void NotifySessionKickRequest(const apiAccount & account, 
                                            const apiSession & session,
                                            const apiKickReason reason) {}
            virtual void OnGetAccountStatus(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account,
                                            void * userData) {}
            virtual void OnGetAccountSubscriptions(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription * subscriptionArray,
                                            unsigned subscriptionCount,
                                            void * userData) {}
            virtual void OnGetAccountSubscription(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            void * userData) {}
            virtual void OnSessionLogin_v3(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            void * userData) {}
            virtual void OnSessionLogin_v4(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const UsageLimit & usageLimit,
                                            void * userData) {}
			virtual void OnSessionLogin_v5(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const UsageLimit & usageLimit,
                                            const Entitlement & entitlement,
                                            void * userData) {};
            virtual void OnSessionValidate_v2(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            void * userData) {}
            virtual void OnSessionValidate_v3(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const UsageLimit & usageLimit,
                                            void * userData) {}
            virtual void OnSessionValidateEx(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const char * password,
                                            void * userData) {}
            virtual void OnSessionConsume_v2(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            void * userData) {}
            virtual void OnSessionConsume_v3(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account, 
                                            const apiSubscription & subscription,
                                            const apiSession & session,
                                            const UsageLimit & usageLimit,
                                            void * userData) {}
            virtual void OnSubscriptionValidate(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiSubscription & subscription,
                                            void * userData) {}
            virtual void OnGetMemberInformation(const apiTrackingNumber trackingNumber, 
                                            const apiResult result, 
                                            const apiAccount & account,
                                            const char * firstName,
                                            const char * lastName,
                                            const char * gender,
                                            const char * email,
											void * userData) {}
			virtual void OnGetMemberInformation_v2(const apiTrackingNumber trackingNumber, 
											const apiResult result, 
											const apiAccount & account,
											const char * firstName,
											const char * lastName,
											const char * gender,
											const char * email,
											const char * defaultCurrency,
											const char * defaultCountry,
											const char * stationHandle,
											void * userData) {}

			virtual void OnGrantFeatureV2(const apiTrackingNumber trackingNumber, 
										  const apiResult result, 
										  void * userData) {}

			virtual void OnModifyFeature_v2(const apiTrackingNumber trackingNumber, 
											const apiResult result, 
											const Feature & currentFeature,
											void * userData) {}

			virtual void OnGrantFeatureByStationID(const apiTrackingNumber trackingNumber, 
												   const apiResult result, 
										           void * userData) {}


    };

    class Entitlement
    {
        public:
            typedef unsigned char UnentitledCode;
            enum
            {
                UNENTITLED_REASON_NULL,
                UNENTITLED_REASON_USER_CLOSED,
				UNENTITLED_REASON_ADMIN_CLOSED,
				UNENTITLED_REASON_BILL_DECLINED,
                UNENTITLED_REASON_END
            };

        public:
            Entitlement();
        
            unsigned        GetTotalTime() const                    { return mTotalTime; }
            unsigned        GetEntitledTime() const                 { return mEntitledTime; }
            unsigned        GetTotalTimeSinceLastLogin() const      { return mTotalTimeSinceLastLogin; }
            unsigned        GetEntitledTimeSinceLastLogin() const   { return mEntitledTimeSinceLastLogin; }
            UnentitledCode  GetReasonUnentitled() const             { return mReasonUnentitled; }

            void            SetTotalTime(unsigned value)                { mTotalTime = value; }
            void            SetEntitledTime(unsigned value)             { mEntitledTime = value; }
            void            SetTotalTimeSinceLastLogin(unsigned value)  { mTotalTimeSinceLastLogin = value; }
            void            SetEntitledTimeSinceLastLogin(unsigned value) { mEntitledTimeSinceLastLogin = value; }
            void            SetReasonUnentitled(UnentitledCode value)   { mReasonUnentitled = value; }

        private:
            unsigned        mTotalTime;
            unsigned        mEntitledTime;
            unsigned        mTotalTimeSinceLastLogin;
            unsigned        mEntitledTimeSinceLastLogin;
            UnentitledCode  mReasonUnentitled;
    };

    class Feature
    {
        public:
            Feature();
        
            unsigned			GetID() const                               { return mID; }
            const std::string & GetData() const								{ return mData; }

            void				SetID(unsigned value)						{ mID = value; }
            void				SetData(const std::string & value)			{ mData = value; }

			bool				SetParameter(const std::string & key, int value);
			bool				SetParameter(const std::string & key, const std::string & value);
			int					GetParameter(const std::string & key) const;
			std::string &	    GetParameter(const std::string & key, std::string & value) const;

			//	For consumable promotions
			bool				Consume();
			int					GetConsumeCount() const;
			bool				IsActive() const;
			void				SetActive(bool isActive);

        private:
            unsigned			mID;
            std::string			mData;
    };

	class FeatureDescription
	{
        public:
            FeatureDescription();
            unsigned			GetID() const                               { return mID; }
            const std::string & GetDescription() const						{ return mDescription; }
            const std::string & GetDefaultData() const						{ return mDefaultData; }
            void				SetID(unsigned value)						{ mID = value; }
            void				SetDescription(const std::string & value)	{ mDescription = value; }
            void				SetDefaultData(const std::string & value)	{ mDefaultData = value; }
		private:
			unsigned			mID;
			std::string			mDescription;
			std::string			mDefaultData;
	};

    class UsageLimit
    {
        public:
            typedef unsigned char TypeCode;
            enum
            {
                TYPE_NONE,
                TYPE_COMPULSORY,
                TYPE_ADVISORY
            };

        public:
            UsageLimit();
        
            TypeCode        GetType() const                 { return mType; }
            unsigned        GetAllowance() const            { return mAllowance; }
            time_t          GetNextAllowance() const        { return mNextAllowance; }

            void            SetType(unsigned char value)    { mType = value; }
            void            SetAllowance(unsigned value)    { mAllowance = value; }
            void            SetNextAllowance(time_t value)  { mNextAllowance = value; }

        private:
            TypeCode        mType;
            unsigned        mAllowance;
            time_t          mNextAllowance;
    };


}

#endif

