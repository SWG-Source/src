#include <stdio.h>
#include "Messages.h"


using namespace std;


namespace Base
{
    void  get(ByteStream::ReadIterator & source, LoginAPI::UsageLimit & target)
    {
		LoginAPI::UsageLimit::TypeCode type;
		unsigned allowance;
		unsigned nextAllowance;

        get(source, type);
        get(source, allowance);
        get(source, nextAllowance);

        target.SetType(type);
        target.SetAllowance(allowance);
        target.SetNextAllowance(nextAllowance);
    }

    void  put(ByteStream & target, const LoginAPI::UsageLimit & source)
    {
	unsigned next = static_cast<unsigned>(source.GetNextAllowance());
        put(target, source.GetType());
        put(target, source.GetAllowance());
        put(target, next);
    }

    void  get(ByteStream::ReadIterator & source, LoginAPI::Entitlement & target)
    {
        unsigned        totalTime;
        unsigned        entitledTime;
        unsigned        totalTimeSinceLastLogin;
        unsigned        entitledTimeSinceLastLogin;
		LoginAPI::Entitlement::UnentitledCode  reasonUnentitled;

        get(source, totalTime);
        get(source, entitledTime);
        get(source, totalTimeSinceLastLogin);
        get(source, entitledTimeSinceLastLogin);
        get(source, reasonUnentitled);

        target.SetTotalTime(totalTime);
        target.SetEntitledTime(entitledTime);
        target.SetTotalTimeSinceLastLogin(totalTimeSinceLastLogin);
        target.SetEntitledTimeSinceLastLogin(entitledTimeSinceLastLogin);
        target.SetReasonUnentitled(reasonUnentitled);
    }

    void  put(ByteStream & target, const LoginAPI::Entitlement & source)
    {
        put(target, source.GetTotalTime());
        put(target, source.GetEntitledTime());
        put(target, source.GetTotalTimeSinceLastLogin());
        put(target, source.GetEntitledTimeSinceLastLogin());
        put(target, source.GetReasonUnentitled());
    }

    void  get(ByteStream::ReadIterator & source, LoginAPI::Feature & target)
    {
        unsigned id;
		std::string data;

        get(source, id);
        get(source, data);

        target.SetID(id);
        target.SetData(data);
    }

    void  put(ByteStream & target, const LoginAPI::Feature & source)
    {
        put(target, source.GetID());
        put(target, source.GetData());
    }

    void  get(ByteStream::ReadIterator & source, LoginAPI::FeatureDescription & target)
    {
        unsigned id;
		std::string description;
		std::string defaultData;

        get(source, id);
        get(source, description);
        get(source, defaultData);

        target.SetID(id);
        target.SetDescription(description);
        target.SetDefaultData(defaultData);
    }

    void  put(ByteStream & target, const LoginAPI::FeatureDescription & source)
    {
        put(target, source.GetID());
        put(target, source.GetDescription());
        put(target, source.GetDefaultData());
    }

}

namespace Message
{


////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    //  GetClientConnections
    BeginImplementMessage(GetClientConnections, Tracked)
    EndImplementMessage


    ////////////////////////////////////////
    //  GetClientConnections
    BeginImplementMessage(GetClientConnectionsReply, TrackedReply)
		ImplementMessageMember(Output, std::string())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  AccountGetStatus_v1
    BeginImplementMessage(AccountGetStatus_v1, Tracked)
        ImplementMessageMember(UserName, "")
        ImplementMessageMember(UserPassword, "")
        ImplementMessageMember(SessionID, "")
    EndImplementMessage


    ////////////////////////////////////////
    //  AccountGetStatusReply_v1
    BeginImplementMessage(AccountGetStatusReply_v1, TrackedReply)
        ImplementMessageMember(Account, apiAccount_v1())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  AccountGetSubscriptions_v1
    BeginImplementMessage(AccountGetSubscriptions_v1, Tracked)
        ImplementMessageMember(UserName, "")
        ImplementMessageMember(UserPassword, "")
        ImplementMessageMember(SessionID, "")
    EndImplementMessage


    ////////////////////////////////////////
    //  AccountGetSubscriptionsReply_v1
    BeginImplementMessage(AccountGetSubscriptionsReply_v1, TrackedReply)
        ImplementMessageMember(Account, apiAccount_v1())
        ImplementMessageMember(Subscriptions, std::vector<apiSubscription_v1>())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  AccountGetSubscription_v1
    BeginImplementMessage(AccountGetSubscription_v1, Tracked)
        ImplementMessageMember(UserName, "")
        ImplementMessageMember(UserPassword, "")
        ImplementMessageMember(SessionID, "")
        ImplementMessageMember(Gamecode, GAMECODE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  AccountGetSubscriptionReply_v1
    BeginImplementMessage(AccountGetSubscriptionReply_v1, TrackedReply)
        ImplementMessageMember(Account, apiAccount_v1())
        ImplementMessageMember(Subscription, apiSubscription_v1())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  AccountGetStatus_v2
    BeginImplementMessage(AccountGetStatus_v2, Tracked)
        ImplementMessageMember(UserName, "")
        ImplementMessageMember(UserPassword, "")
        ImplementMessageMember(SessionID, "")
    EndImplementMessage


    ////////////////////////////////////////
    //  AccountGetStatusReply_v2
    BeginImplementMessage(AccountGetStatusReply_v2, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(SecureIdPrompt, std::string())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  AccountGetSubscriptions_v2
    BeginImplementMessage(AccountGetSubscriptions_v2, Tracked)
        ImplementMessageMember(UserName, "")
        ImplementMessageMember(UserPassword, "")
        ImplementMessageMember(SessionID, "")
    EndImplementMessage


    ////////////////////////////////////////
    //  AccountGetSubscriptionsReply_v2
    BeginImplementMessage(AccountGetSubscriptionsReply_v2, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscriptions, std::vector<apiSubscription>())
        ImplementMessageMember(SecureIdPrompt, std::string())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  AccountGetSubscription_v2
    BeginImplementMessage(AccountGetSubscription_v2, Tracked)
        ImplementMessageMember(UserName, "")
        ImplementMessageMember(UserPassword, "")
        ImplementMessageMember(SessionID, "")
        ImplementMessageMember(Gamecode, GAMECODE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  AccountGetSubscriptionReply_v2
    BeginImplementMessage(AccountGetSubscriptionReply_v2, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(SecureIdPrompt, std::string())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    //  AccountGetStatus
    BeginImplementMessage(AccountGetStatus, Tracked)
        ImplementMessageMember(UserName, std::string())
        ImplementMessageMember(UserPassword, std::string())
        ImplementMessageMember(SessionID, std::string())
        ImplementMessageMember(AccountId, 0)
    EndImplementMessage


    ////////////////////////////////////////
    //  AccountGetStatusReply
    BeginImplementMessage(AccountGetStatusReply, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  AccountGetSubscriptions
    BeginImplementMessage(AccountGetSubscriptions, Tracked)
        ImplementMessageMember(UserName, std::string())
        ImplementMessageMember(UserPassword, std::string())
        ImplementMessageMember(SessionID, std::string())
        ImplementMessageMember(AccountId, 0)
    EndImplementMessage


    ////////////////////////////////////////
    //  AccountGetSubscriptionsReply
    BeginImplementMessage(AccountGetSubscriptionsReply, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscriptions, std::vector<apiSubscription>())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  AccountGetSubscription
    BeginImplementMessage(AccountGetSubscription, Tracked)
        ImplementMessageMember(UserName, std::string())
        ImplementMessageMember(UserPassword, std::string())
        ImplementMessageMember(SessionID, std::string())
        ImplementMessageMember(AccountId, 0)
        ImplementMessageMember(Gamecode, GAMECODE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  AccountGetSubscriptionReply
    BeginImplementMessage(AccountGetSubscriptionReply, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogin_v1
    BeginImplementMessage(SessionLogin_v1, Tracked)
        ImplementMessageMember(UserName, "")
        ImplementMessageMember(UserPassword, "")
        ImplementMessageMember(ParentSessionID, "")
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
        ImplementMessageMember(ClientIP, 0)
    EndImplementMessage

    
    ////////////////////////////////////////
    //  SessionLoginReply_v1
    BeginImplementMessage(SessionLoginReply_v1, TrackedReply)
        ImplementMessageMember(Account, apiAccount_v1())
        ImplementMessageMember(Subscription, apiSubscription_v1())
        ImplementMessageMember(Session, apiSession_v1())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  SessionConsume_v1
    BeginImplementMessage(SessionConsume_v1, Tracked)
        ImplementMessageMember(SessionID, "")
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  SessionConsumeReply_v1
    BeginImplementMessage(SessionConsumeReply_v1, TrackedReply)
        ImplementMessageMember(Account, apiAccount_v1())
        ImplementMessageMember(Subscription, apiSubscription_v1())
        ImplementMessageMember(Session, apiSession_v1())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  SessionValidate_v1
    BeginImplementMessage(SessionValidate_v1, Tracked)
        ImplementMessageMember(SessionID, "")
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  SessionValidateReply_v1
    BeginImplementMessage(SessionValidateReply_v1, TrackedReply)
        ImplementMessageMember(Account, apiAccount_v1())
        ImplementMessageMember(Subscription, apiSubscription_v1())
        ImplementMessageMember(Session, apiSession_v1())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  SessionTouch
    BeginImplementMessage(SessionTouch, Basic)
        ImplementMessageMember(SessionArray, vector<string>())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  SessionLogout
    BeginImplementMessage(SessionLogout, Basic)
        ImplementMessageMember(SessionArray, vector<string>())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionTouchEx
    BeginImplementMessage(SessionTouchEx, Basic)
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
        ImplementMessageMember(AccountArray, std::vector<apiAccountId>())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogoutEx
    BeginImplementMessage(SessionLogoutEx, Basic)
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
        ImplementMessageMember(AccountArray, std::vector<apiAccountId>())
    EndImplementMessage

    
////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogin_v2
    BeginImplementMessage(SessionLogin_v2, Tracked)
        ImplementMessageMember(UserName, "")
        ImplementMessageMember(UserPassword, "")
        ImplementMessageMember(ParentSessionID, "")
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
        ImplementMessageMember(ClientIP, 0)
        ImplementMessageMember(Forced, false)
    EndImplementMessage

    
    ////////////////////////////////////////
    //  SessionLoginReply_v2
    BeginImplementMessage(SessionLoginReply_v2, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(Session, apiSession())
        ImplementMessageMember(SecureIdPrompt, std::string())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  SessionConsume_v2
    BeginImplementMessage(SessionConsume_v2, Tracked)
        ImplementMessageMember(SessionID, "")
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  SessionConsumeReply_v2
    BeginImplementMessage(SessionConsumeReply_v2, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(Session, apiSession())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  SessionConsume_v3
    BeginImplementMessage(SessionConsume_v3, Tracked)
        ImplementMessageMember(SessionID, "")
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  SessionConsumeReply_v3
    BeginImplementMessage(SessionConsumeReply_v3, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(Session, apiSession())
        ImplementMessageMember(UsageLimit, LoginAPI::UsageLimit())
        ImplementMessageMember(Padding, 0)
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  SessionConsume
    BeginImplementMessage(SessionConsume, Tracked)
        ImplementMessageMember(SessionID, "")
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  SessionConsumeReply
    BeginImplementMessage(SessionConsumeReply, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(Session, apiSession())
        ImplementMessageMember(UsageLimit, LoginAPI::UsageLimit())
        ImplementMessageMember(Entitlement, LoginAPI::Entitlement())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  SessionValidate_v2
    BeginImplementMessage(SessionValidate_v2, Tracked)
        ImplementMessageMember(SessionID, "")
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  SessionValidateReply_v2
    BeginImplementMessage(SessionValidateReply_v2, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(Session, apiSession())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  SessionValidate_v3
    BeginImplementMessage(SessionValidate_v3, Tracked)
        ImplementMessageMember(SessionID, "")
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  SessionValidateReply_v3
    BeginImplementMessage(SessionValidateReply_v3, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(Session, apiSession())
        ImplementMessageMember(UsageLimit, LoginAPI::UsageLimit())
        ImplementMessageMember(Padding, 0)
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  SessionValidate
    BeginImplementMessage(SessionValidate, Tracked)
        ImplementMessageMember(SessionID, "")
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  SessionValidateReply
    BeginImplementMessage(SessionValidateReply, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(Session, apiSession())
        ImplementMessageMember(UsageLimit, LoginAPI::UsageLimit())
        ImplementMessageMember(Entitlement, LoginAPI::Entitlement())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionValidateEx
    BeginImplementMessage(SessionValidateEx, Tracked)
        ImplementMessageMember(SessionID, std::string())
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  SessionValidateExReply
    BeginImplementMessage(SessionValidateExReply, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(Session, apiSession())
        ImplementMessageMember(Password, std::string())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogin_v3
    BeginImplementMessage(SessionLogin_v3, Tracked)
        ImplementMessageMember(UserName, std::string())
        ImplementMessageMember(UserPassword, std::string())
        ImplementMessageMember(ParentSessionID, std::string())
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
        ImplementMessageMember(ClientIP, 0)
        ImplementMessageMember(Forced, false)
    EndImplementMessage

    
    ////////////////////////////////////////
    //  SessionLoginReply_v3
    BeginImplementMessage(SessionLoginReply_v3, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(Session, apiSession())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogin_v4
    BeginImplementMessage(SessionLogin_v4, Tracked)
        ImplementMessageMember(UserName, std::string())
        ImplementMessageMember(UserPassword, std::string())
        ImplementMessageMember(ParentSessionID, std::string())
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
        ImplementMessageMember(ClientIP, 0)
        ImplementMessageMember(Forced, false)
    EndImplementMessage

    
    ////////////////////////////////////////
    //  SessionLoginReply_v4
    BeginImplementMessage(SessionLoginReply_v4, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(Session, apiSession())
        ImplementMessageMember(UsageLimit, LoginAPI::UsageLimit())
        ImplementMessageMember(Padding, 0)
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogin_v5
    BeginImplementMessage(SessionLogin_v5, Tracked)
        ImplementMessageMember(UserName, std::string())
        ImplementMessageMember(UserPassword, std::string())
        ImplementMessageMember(ParentSessionID, std::string())
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
        ImplementMessageMember(ClientIP, 0)
        ImplementMessageMember(Flags, 0)
    EndImplementMessage

    
    ////////////////////////////////////////
    //  SessionLoginReply_v5
    BeginImplementMessage(SessionLoginReply_v5, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(Session, apiSession())
        ImplementMessageMember(UsageLimit, LoginAPI::UsageLimit())
        ImplementMessageMember(Entitlement, LoginAPI::Entitlement())
    EndImplementMessage

////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogin
    BeginImplementMessage(SessionLogin, Tracked)
        ImplementMessageMember(UserName, std::string())
        ImplementMessageMember(UserPassword, std::string())
        ImplementMessageMember(ParentSessionID, std::string())
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
        ImplementMessageMember(ClientIP, 0)
        ImplementMessageMember(Flags, 0)
		ImplementMessageMember(Namespace, _defaultNamespace)
    EndImplementMessage

    
    ////////////////////////////////////////
    //  SessionLoginReply
    BeginImplementMessage(SessionLoginReply, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(Session, apiSession())
        ImplementMessageMember(UsageLimit, LoginAPI::UsageLimit())
        ImplementMessageMember(Entitlement, LoginAPI::Entitlement())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLoginInternal
    BeginImplementMessage(SessionLoginInternal, Tracked)
        ImplementMessageMember(AccountId, 0)
        ImplementMessageMember(SessionType, SESSION_TYPE_NULL)
        ImplementMessageMember(ClientIP, 0)
        ImplementMessageMember(Flags, 0)
    EndImplementMessage

    
    ////////////////////////////////////////
    //  SessionLoginInternalReply
    BeginImplementMessage(SessionLoginInternalReply, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Subscription, apiSubscription())
        ImplementMessageMember(Session, apiSession())
        ImplementMessageMember(UsageLimit, LoginAPI::UsageLimit())
        ImplementMessageMember(Entitlement, LoginAPI::Entitlement())
    EndImplementMessage



////////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////
    //  NotifySessionKick
    BeginImplementMessage(NotifySessionKick, Basic)
        ImplementMessageMember(SessionList, vector<string>())
    EndImplementMessage


    ////////////////////////////////////////
    //  NotifySessionKickRequest
    BeginImplementMessage(NotifySessionKickRequest, Basic)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(Session, apiSession())
        ImplementMessageMember(Reason, KICK_REASON_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  SessionKickReply
    BeginImplementMessage(SessionKickReply, Basic)
        ImplementMessageMember(SessionId, std::string())
        ImplementMessageMember(Reply, KICK_REPLY_DENY)
    EndImplementMessage

    
////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SubscriptionValidate_v1
    BeginImplementMessage(SubscriptionValidate_v1, Tracked)
        ImplementMessageMember(SessionID, "")
        ImplementMessageMember(Gamecode, GAMECODE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  SubscriptionValidateReply_v1
    BeginImplementMessage(SubscriptionValidateReply_v1, TrackedReply)
        ImplementMessageMember(Subscription, apiSubscription_v1())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SubscriptionValidate
    BeginImplementMessage(SubscriptionValidate, Tracked)
        ImplementMessageMember(SessionID, "")
        ImplementMessageMember(Gamecode, GAMECODE_NULL)
    EndImplementMessage


    ////////////////////////////////////////
    //  SubscriptionValidateReply
    BeginImplementMessage(SubscriptionValidateReply, TrackedReply)
        ImplementMessageMember(Subscription, apiSubscription())
    EndImplementMessage


///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  MemberGetInformation
    BeginImplementMessage(MemberGetInformation, Tracked)
        ImplementMessageMember(AccountId, 0)
    EndImplementMessage


    ////////////////////////////////////////
    //  MemberGetInformationReply
    BeginImplementMessage(MemberGetInformationReply, TrackedReply)
        ImplementMessageMember(Account, apiAccount())
        ImplementMessageMember(FirstName, std::string())
        ImplementMessageMember(LastName, std::string())
        ImplementMessageMember(Gender, std::string())
        ImplementMessageMember(Email, std::string())
		EndImplementMessage


		///////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////
	//  MemberGetInformation_v2
	BeginImplementMessage(MemberGetInformation_v2, Tracked)
		ImplementMessageMember(AccountId, 0)
		EndImplementMessage


	////////////////////////////////////////
	//  MemberGetInformationReply_v2
	BeginImplementMessage(MemberGetInformationReply_v2, TrackedReply)
		ImplementMessageMember(Account, apiAccount())
		ImplementMessageMember(FirstName, std::string())
		ImplementMessageMember(LastName, std::string())
		ImplementMessageMember(Gender, std::string())
		ImplementMessageMember(Email, std::string())
		ImplementMessageMember(DefaultCurrency, std::string())
		ImplementMessageMember(DefaultCountry, std::string())
		ImplementMessageMember(StationHandle, std::string())
		EndImplementMessage


///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  Kick
    BeginImplementMessage(Kick, Tracked)
        ImplementMessageMember(SessionID, std::string())
        ImplementMessageMember(Reason, 0)
    EndImplementMessage


    ////////////////////////////////////////
    //  KickReply
    BeginImplementMessage(KickReply, TrackedReply)
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    //  ReqGetSessions
    BeginImplementMessage(ReqGetSessions, Tracked)
        ImplementMessageMember(AccountId, 0)
    EndImplementMessage


    ////////////////////////////////////////
    //  ReqGetSessionsReply
    BeginImplementMessage(ReqGetSessionsReply, TrackedReply)
		ImplementMessageMember(SessionArray, std::vector<apiSession>())
        ImplementMessageMember(SubscriptionArray, std::vector<apiSubscription>())
        ImplementMessageMember(UsageLimitArray, std::vector<LoginAPI::UsageLimit>())
        ImplementMessageMember(TimeCreated, std::vector<unsigned>())
        ImplementMessageMember(TimeTouched, std::vector<unsigned>())
    EndImplementMessage

////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  GetFeatures_v1
    BeginImplementMessage(GetFeatures_v1, Tracked)
        ImplementMessageMember(SessionID, std::string())
    EndImplementMessage


    ////////////////////////////////////////
    //  GetFeaturesReply_v1
    BeginImplementMessage(GetFeaturesReply_v1, TrackedReply)
        ImplementMessageMember(FeatureArray, std::vector<LoginAPI::Feature>())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  GetFeatures
    BeginImplementMessage(GetFeatures, Tracked)
        ImplementMessageMember(SessionID, std::string())
		ImplementMessageMember(AccountId, 0)
		ImplementMessageMember(Gamecode, 0)
    EndImplementMessage


    ////////////////////////////////////////
    //  GetFeaturesReply
    BeginImplementMessage(GetFeaturesReply, TrackedReply)
        ImplementMessageMember(FeatureArray, std::vector<LoginAPI::Feature>())
    EndImplementMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  GrantFeature
    BeginImplementMessage(GrantFeature, Tracked)
        ImplementMessageMember(SessionID, std::string())
        ImplementMessageMember(FeatureID, 0)
    EndImplementMessage


    ////////////////////////////////////////
    //  GrantFeatureReply
    BeginImplementMessage(GrantFeatureReply, TrackedReply)
    EndImplementMessage


///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  ModifyFeature
    BeginImplementMessage(ModifyFeature, Tracked)
        ImplementMessageMember(SessionID, std::string())
        ImplementMessageMember(Feature, LoginAPI::Feature())
    EndImplementMessage


    ////////////////////////////////////////
    //  ModifyFeatureReply
    BeginImplementMessage(ModifyFeatureReply, TrackedReply)
    EndImplementMessage


///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  RevokeFeature
    BeginImplementMessage(RevokeFeature, Tracked)
        ImplementMessageMember(SessionID, std::string())
        ImplementMessageMember(FeatureID, 0)
    EndImplementMessage


    ////////////////////////////////////////
    //  RevokeFeatureReply
    BeginImplementMessage(RevokeFeatureReply, TrackedReply)
    EndImplementMessage


///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  EnumerateFeatures
    BeginImplementMessage(EnumerateFeatures, Tracked)
        ImplementMessageMember(Gamecode, 0)
    EndImplementMessage


    ////////////////////////////////////////
    //  EnumerateFeaturesReply
    BeginImplementMessage(EnumerateFeaturesReply, TrackedReply)
	ImplementMessageMember(FeatureArray, std::vector<LoginAPI::FeatureDescription>())
    EndImplementMessage


///////////////////////////////////////////////////////////////////////////////
    
	////////////////////////////////////////
    //  SessionStartPlay
    BeginImplementMessage(SessionStartPlay, Tracked)
        ImplementMessageMember(SessionID, std::string())
		ImplementMessageMember(ServerName, std::string())
		ImplementMessageMember(CharacterName, std::string())
		ImplementMessageMember(GameData, std::string())
    EndImplementMessage


    ////////////////////////////////////////
    //  SessionStartPlayReply
    BeginImplementMessage(SessionStartPlayReply, TrackedReply)
    EndImplementMessage


	///////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////
	//  SessionStopPlay
	BeginImplementMessage(SessionStopPlay, Tracked)
	ImplementMessageMember(SessionID, std::string())
	ImplementMessageMember(ServerName, std::string())
	ImplementMessageMember(CharacterName, std::string())
	EndImplementMessage


	////////////////////////////////////////
	//  SessionStopPlayReply
	BeginImplementMessage(SessionStopPlayReply, TrackedReply)
	EndImplementMessage

///////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////
	//  GrantFeatureV2
	BeginImplementMessage(GrantFeatureV2, Tracked)
	ImplementMessageMember(SessionID, std::string())
	ImplementMessageMember(FeatureID, 0)
	ImplementMessageMember(GameCode, std::string())
	ImplementMessageMember(ProviderID, 0)
	ImplementMessageMember(PromoID, 0)
	EndImplementMessage


	////////////////////////////////////////
	//  GrantFeatureReplyV2
	BeginImplementMessage(GrantFeatureReplyV2, TrackedReply)
	EndImplementMessage

///////////////////////////////////////////////////////////////////////////////
	BeginImplementMessage(ModifyFeature_v2, Tracked)
	ImplementMessageMember(AccountID, 0)
	ImplementMessageMember(GameCode, std::string())
	ImplementMessageMember(OrigFeature, LoginAPI::Feature())
	ImplementMessageMember(NewFeature, LoginAPI::Feature())
	EndImplementMessage


////////////////////////////////////////
//  ModifyFeatureReply_v2
	BeginImplementMessage(ModifyFeatureReply_v2, TrackedReply)
	ImplementMessageMember(CurrentFeature, LoginAPI::Feature())
	EndImplementMessage
	
///////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////
	//  GrantFeatureByStationID
	BeginImplementMessage(GrantFeatureByStationID, Tracked)
	ImplementMessageMember(AccountID, 0)
	ImplementMessageMember(FeatureID, 0)
	ImplementMessageMember(GameCode, std::string())
	ImplementMessageMember(ProviderID, 0)
	ImplementMessageMember(PromoID, 0)
	EndImplementMessage


	////////////////////////////////////////
	//  GrantFeatureReplyV2
	BeginImplementMessage(GrantFeatureByStationIDReply, TrackedReply)
	EndImplementMessage

///////////////////////////////////////////////////////////////////////////////////////
}

