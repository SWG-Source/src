#ifndef SESSION_API__MESSAGES_H
#define SESSION_API__MESSAGES_H

#pragma warning( disable: 4786 )

#include <string>
#include <vector>
#include "Session/CommonAPI/CommonMessages.h"
#include "Session/CommonAPI/CommonAPI.h"
#include "Session/LoginAPI/Client.h"


namespace Base
{
    void  get(ByteStream::ReadIterator & source, LoginAPI::UsageLimit & target);
    void  put(ByteStream & target, const LoginAPI::UsageLimit & source);

    void  get(ByteStream::ReadIterator & source, LoginAPI::Entitlement & target);
    void  put(ByteStream & target, const LoginAPI::Entitlement & source);

    void  get(ByteStream::ReadIterator & source, LoginAPI::Feature & target);
    void  put(ByteStream & target, const LoginAPI::Feature & source);

    void  get(ByteStream::ReadIterator & source, LoginAPI::FeatureDescription & target);
    void  put(ByteStream & target, const LoginAPI::FeatureDescription & source);
}

namespace Message
{


    enum
    {
        //  Client Messages
        MESSAGE_ACCOUNT_GET_STATUS_v1								= 0x1000,
        MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_v1					= 0x1001,
        MESSAGE_ACCOUNT_GET_SUBSCRIPTION_v1					= 0x1002,
        MESSAGE_ACCOUNT_GET_STATUS_v2								= 0x1003,
        MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_v2					= 0x1004,
        MESSAGE_ACCOUNT_GET_SUBSCRIPTION_v2					= 0x1005,
        MESSAGE_ACCOUNT_GET_STATUS									= 0x1006,
        MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS						= 0x1007,
        MESSAGE_ACCOUNT_GET_SUBSCRIPTION						= 0x1008,
        MESSAGE_SESSION_LOGIN_v1											= 0x1010,
        MESSAGE_SESSION_CONSUME_v1									= 0x1011,
        MESSAGE_SESSION_VALIDATE_v1										= 0x1012,
        MESSAGE_SESSION_TOUCH												= 0x1013,
        MESSAGE_SESSION_LOGOUT											= 0x1014,
        MESSAGE_SESSION_TOUCH_EX										= 0x1015,
        MESSAGE_SESSION_LOGOUT_EX										= 0x1016,
        MESSAGE_SESSION_LOGIN_v2											= 0x1017,
        MESSAGE_SESSION_CONSUME_v2									= 0x1018,
        MESSAGE_SESSION_VALIDATE_v2										= 0x1019,
        MESSAGE_SESSION_LOGIN_v3											= 0x101a,
        MESSAGE_SESSION_VALIDATE_EX									= 0x101b,
        MESSAGE_SESSION_KICK_REPLY										= 0x101f,
        MESSAGE_SUBSCRIPTION_VALIDATE_v1							= 0x1020,
        MESSAGE_SUBSCRIPTION_VALIDATE									= 0x1021,
        MESSAGE_MEMBER_GET_INFORMATION							= 0x1030,
        MESSAGE_SESSION_LOGIN_v4											= 0x1031,
        MESSAGE_SESSION_CONSUME_v3									= 0x1032,
        MESSAGE_SESSION_VALIDATE_v3										= 0x1033,
        MESSAGE_KICK																	= 0x1034,
        MESSAGE_REQ_GET_SESSIONS										= 0x1035,
        MESSAGE_SESSION_LOGIN_v5											= 0x1036,
        MESSAGE_SESSION_CONSUME											= 0x1037,
        MESSAGE_SESSION_VALIDATE											= 0x1038,
        MESSAGE_FEATURE_GET_v1												= 0x1039,
        MESSAGE_FEATURE_GRANT												= 0x1040,
        MESSAGE_FEATURE_MODIFY												= 0x1041,
        MESSAGE_FEATURE_REVOKE											= 0x1042,
        MESSAGE_FEATURE_ENUMERATE										= 0x1043,
        MESSAGE_GET_CLIENT_CONNECTIONS								= 0x1044,
		MESSAGE_SESSION_START_PLAY										= 0x1045,
		MESSAGE_SESSION_STOP_PLAY										= 0x1046,
		MESSAGE_SESSION_LOGIN_INTERNAL									= 0x1047,
		MESSAGE_FEATURE_GET												= 0x1048,
		MESSAGE_SESSION_LOGIN											= 0x1049,
		MESSAGE_MEMBER_GET_INFORMATION_v2								= 0x104a,
		MESSAGE_FEATURE_GRANT_v2										= 0x104b,
		MESSAGE_MODIFY_FEATURE_v2										= 0x104c,
		MESSAGE_GRANT_FEATURE_BY_STATION_ID								= 0x104d,

        //  Server Messages
        MESSAGE_ACCOUNT_GET_STATUS_REPLY_v1					= 0x8000,
        MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_REPLY_v1	= 0x8001,
        MESSAGE_ACCOUNT_GET_SUBSCRIPTION_REPLY_v1		= 0x8002,
        MESSAGE_ACCOUNT_GET_STATUS_REPLY_v2					= 0x8000,
        MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_REPLY_v2	= 0x8001,
        MESSAGE_ACCOUNT_GET_SUBSCRIPTION_REPLY_v2		= 0x8002,
        MESSAGE_ACCOUNT_GET_STATUS_REPLY						= 0x8006,
        MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_REPLY			= 0x8007,
        MESSAGE_ACCOUNT_GET_SUBSCRIPTION_REPLY			= 0x8008,
        MESSAGE_SESSION_LOGIN_REPLY_v1								= 0x8010,
        MESSAGE_SESSION_CONSUME_REPLY_v1						= 0x8011,
        MESSAGE_SESSION_VALIDATE_REPLY_v1						= 0x8012,
        MESSAGE_SESSION_LOGIN_REPLY_v2								= 0x8017,
        MESSAGE_SESSION_CONSUME_REPLY_v2						= 0x8018,
        MESSAGE_SESSION_VALIDATE_REPLY_v2						= 0x8019,
        MESSAGE_SESSION_LOGIN_REPLY_v3								= 0x801a,
        MESSAGE_SESSION_VALIDATE_EX_REPLY						= 0x801b,
        MESSAGE_NOTIFY_SESSION_KICK										= 0x801e,
        MESSAGE_NOTIFY_SESSION_KICK_REQUEST					= 0x801f,
        MESSAGE_SUBSCRIPTION_VALIDATE_REPLY_v1				= 0x8020,
        MESSAGE_SUBSCRIPTION_VALIDATE_REPLY					= 0x8021,
        MESSAGE_MEMBER_GET_INFORMATION_REPLY				= 0x8030,
        MESSAGE_SESSION_LOGIN_REPLY_v4								= 0x8031,
        MESSAGE_SESSION_CONSUME_REPLY_v3						= 0x8032,
        MESSAGE_SESSION_VALIDATE_REPLY_v3						= 0x8033,
        MESSAGE_KICK_REPLY														= 0x8034,
        MESSAGE_REQ_GET_SESSIONS_REPLY							= 0x8035,
        MESSAGE_SESSION_LOGIN_REPLY_v5								= 0x8036,
        MESSAGE_SESSION_CONSUME_REPLY								= 0x8037,
        MESSAGE_SESSION_VALIDATE_REPLY								= 0x8038,
        MESSAGE_FEATURE_GET_REPLY_v1									= 0x8039,
        MESSAGE_FEATURE_GRANT_REPLY									= 0x8040,
        MESSAGE_FEATURE_MODIFY_REPLY								= 0x8041,
        MESSAGE_FEATURE_REVOKE_REPLY								= 0x8042,
        MESSAGE_FEATURE_ENUMERATE_REPLY						= 0x8043,
        MESSAGE_GET_CLIENT_CONNECTIONS_REPLY					= 0x8044,
		MESSAGE_SESSION_START_PLAY_REPLY						= 0x8045,
		MESSAGE_SESSION_STOP_PLAY_REPLY							= 0x8046,
		MESSAGE_SESSION_LOGIN_INTERNAL_REPLY					= 0x8047,
		MESSAGE_FEATURE_GET_REPLY								= 0x8048,
		MESSAGE_SESSION_LOGIN_REPLY								= 0x8049,
		MESSAGE_MEMBER_GET_INFORMATION_REPLY_v2					= 0x804a,
		MESSAGE_FEATURE_GRANT_REPLY_v2							= 0x804b,
		MESSAGE_MODIFY_FEATURE_REPLY_v2							= 0x804c,
		MESSAGE_GRANT_FEATURE_BY_STATION_ID_REPLY				= 0x804d,
   };


////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    //  GetClientConnections
    BeginDefineMessage(GetClientConnections, Tracked, MESSAGE_GET_CLIENT_CONNECTIONS)
    EndDefineMessage


    ////////////////////////////////////////
    //  GetClientConnections
    BeginDefineMessage(GetClientConnectionsReply, TrackedReply, MESSAGE_GET_CLIENT_CONNECTIONS_REPLY)
	DefineMessageMember(Output, std::string)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    //  AccountGetStatus_v1
    BeginDefineMessage(AccountGetStatus_v1, Tracked, MESSAGE_ACCOUNT_GET_STATUS_v1)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(SessionID, std::string)
    EndDefineMessage


    ////////////////////////////////////////
    //  AccountGetStatusReply_v1
    BeginDefineMessage(AccountGetStatusReply_v1, TrackedReply, MESSAGE_ACCOUNT_GET_STATUS_REPLY_v1)
        DefineMessageMember(Account, apiAccount_v1)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  AccountGetSubscriptions_v1
    BeginDefineMessage(AccountGetSubscriptions_v1, Tracked, MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_v1)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(SessionID, std::string)
    EndDefineMessage


    ////////////////////////////////////////
    //  AccountGetSubscriptionsReply_v1
    BeginDefineMessage(AccountGetSubscriptionsReply_v1, TrackedReply, MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_REPLY_v1)
        DefineMessageMember(Account, apiAccount_v1)
        DefineMessageMemberArray(Subscriptions, apiSubscription_v1)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  AccountGetSubscription_v1
    BeginDefineMessage(AccountGetSubscription_v1, Tracked, MESSAGE_ACCOUNT_GET_SUBSCRIPTION_v1)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(Gamecode, apiGamecode)
    EndDefineMessage


    ////////////////////////////////////////
    //  AccountGetSubscriptionReply_v1
    BeginDefineMessage(AccountGetSubscriptionReply_v1, TrackedReply, MESSAGE_ACCOUNT_GET_SUBSCRIPTION_REPLY_v1)
        DefineMessageMember(Account, apiAccount_v1)
        DefineMessageMember(Subscription, apiSubscription_v1)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    //  AccountGetStatus_v2
    BeginDefineMessage(AccountGetStatus_v2, Tracked, MESSAGE_ACCOUNT_GET_STATUS_v2)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(SessionID, std::string)
    EndDefineMessage


    ////////////////////////////////////////
    //  AccountGetStatusReply_v2
    BeginDefineMessage(AccountGetStatusReply_v2, TrackedReply, MESSAGE_ACCOUNT_GET_STATUS_REPLY_v2)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(SecureIdPrompt, std::string)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  AccountGetSubscriptions_v2
    BeginDefineMessage(AccountGetSubscriptions_v2, Tracked, MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_v2)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(SessionID, std::string)
    EndDefineMessage


    ////////////////////////////////////////
    //  AccountGetSubscriptionsReply_v2
    BeginDefineMessage(AccountGetSubscriptionsReply_v2, TrackedReply, MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_REPLY_v2)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMemberArray(Subscriptions, apiSubscription)
        DefineMessageMember(SecureIdPrompt, std::string)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  AccountGetSubscription_v2
    BeginDefineMessage(AccountGetSubscription_v2, Tracked, MESSAGE_ACCOUNT_GET_SUBSCRIPTION_v2)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(Gamecode, apiGamecode)
    EndDefineMessage


    ////////////////////////////////////////
    //  AccountGetSubscriptionReply_v2
    BeginDefineMessage(AccountGetSubscriptionReply_v2, TrackedReply, MESSAGE_ACCOUNT_GET_SUBSCRIPTION_REPLY_v2)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(SecureIdPrompt, std::string)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////
    //  AccountGetStatus
    BeginDefineMessage(AccountGetStatus, Tracked, MESSAGE_ACCOUNT_GET_STATUS)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(AccountId, apiAccountId)
    EndDefineMessage


    ////////////////////////////////////////
    //  AccountGetStatusReply
    BeginDefineMessage(AccountGetStatusReply, TrackedReply, MESSAGE_ACCOUNT_GET_STATUS_REPLY)
        DefineMessageMember(Account, apiAccount)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  AccountGetSubscriptions
    BeginDefineMessage(AccountGetSubscriptions, Tracked, MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(AccountId, apiAccountId)
    EndDefineMessage


    ////////////////////////////////////////
    //  AccountGetSubscriptionsReply
    BeginDefineMessage(AccountGetSubscriptionsReply, TrackedReply, MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_REPLY)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMemberArray(Subscriptions, apiSubscription)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  AccountGetSubscription
    BeginDefineMessage(AccountGetSubscription, Tracked, MESSAGE_ACCOUNT_GET_SUBSCRIPTION)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(AccountId, apiAccountId)
        DefineMessageMember(Gamecode, apiGamecode)
    EndDefineMessage


    ////////////////////////////////////////
    //  AccountGetSubscriptionReply
    BeginDefineMessage(AccountGetSubscriptionReply, TrackedReply, MESSAGE_ACCOUNT_GET_SUBSCRIPTION_REPLY)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogin_v1
    BeginDefineMessage(SessionLogin_v1, Tracked, MESSAGE_SESSION_LOGIN_v1)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(ParentSessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
        DefineMessageMember(ClientIP, apiIP)
    EndDefineMessage

    
    ////////////////////////////////////////
    //  SessionLoginReply_v1
    BeginDefineMessage(SessionLoginReply_v1, TrackedReply, MESSAGE_SESSION_LOGIN_REPLY_v1)
        DefineMessageMember(Account, apiAccount_v1)
        DefineMessageMember(Subscription, apiSubscription_v1)
        DefineMessageMember(Session, apiSession_v1)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionConsume_v1
    BeginDefineMessage(SessionConsume_v1, Tracked, MESSAGE_SESSION_CONSUME_v1)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
    EndDefineMessage


    ////////////////////////////////////////
    //  SessionConsumeReply_v1
    BeginDefineMessage(SessionConsumeReply_v1, TrackedReply, MESSAGE_SESSION_CONSUME_REPLY_v1)
        DefineMessageMember(Account, apiAccount_v1)
        DefineMessageMember(Subscription, apiSubscription_v1)
        DefineMessageMember(Session, apiSession_v1)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionValidate_v1
    BeginDefineMessage(SessionValidate_v1, Tracked, MESSAGE_SESSION_VALIDATE_v1)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
    EndDefineMessage


    ////////////////////////////////////////
    //  SessionValidateReply_v1
    BeginDefineMessage(SessionValidateReply_v1, TrackedReply, MESSAGE_SESSION_VALIDATE_REPLY_v1)
        DefineMessageMember(Account, apiAccount_v1)
        DefineMessageMember(Subscription, apiSubscription_v1)
        DefineMessageMember(Session, apiSession_v1)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionTouch
    BeginDefineMessage(SessionTouch, Basic, MESSAGE_SESSION_TOUCH)
        DefineMessageMemberArray(SessionArray, std::string)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogout
    BeginDefineMessage(SessionLogout, Basic, MESSAGE_SESSION_LOGOUT)
        DefineMessageMemberArray(SessionArray, std::string)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionTouchEx
    BeginDefineMessage(SessionTouchEx, Basic, MESSAGE_SESSION_TOUCH_EX)
        DefineMessageMember(SessionType, apiSessionType)
        DefineMessageMemberArray(AccountArray, apiAccountId)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogoutEx
    BeginDefineMessage(SessionLogoutEx, Basic, MESSAGE_SESSION_LOGOUT_EX)
        DefineMessageMember(SessionType, apiSessionType)
        DefineMessageMemberArray(AccountArray, apiAccountId)
    EndDefineMessage

    
////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogin_v2
    BeginDefineMessage(SessionLogin_v2, Tracked, MESSAGE_SESSION_LOGIN_v2)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(ParentSessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
        DefineMessageMember(ClientIP, apiIP)
        DefineMessageMember(Forced, bool)
    EndDefineMessage

    
    ////////////////////////////////////////
    //  SessionLoginReply_v2
    BeginDefineMessage(SessionLoginReply_v2, TrackedReply, MESSAGE_SESSION_LOGIN_REPLY_v2)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(Session, apiSession)
        DefineMessageMember(SecureIdPrompt, std::string)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionConsume_v2
    BeginDefineMessage(SessionConsume_v2, Tracked, MESSAGE_SESSION_CONSUME_v2)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
    EndDefineMessage


    ////////////////////////////////////////
    //  SessionConsumeReply_v2
    BeginDefineMessage(SessionConsumeReply_v2, TrackedReply, MESSAGE_SESSION_CONSUME_REPLY_v2)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(Session, apiSession)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionConsume_v3
    BeginDefineMessage(SessionConsume_v3, Tracked, MESSAGE_SESSION_CONSUME_v3)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
    EndDefineMessage


    ////////////////////////////////////////
    //  SessionConsumeReply_v3
    BeginDefineMessage(SessionConsumeReply_v3, TrackedReply, MESSAGE_SESSION_CONSUME_REPLY_v3)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(Session, apiSession)
        DefineMessageMember(UsageLimit, LoginAPI::UsageLimit)
        DefineMessageMember(Padding, unsigned)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionConsume
    BeginDefineMessage(SessionConsume, Tracked, MESSAGE_SESSION_CONSUME)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
    EndDefineMessage


    ////////////////////////////////////////
    //  SessionConsumeReply
    BeginDefineMessage(SessionConsumeReply, TrackedReply, MESSAGE_SESSION_CONSUME_REPLY)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(Session, apiSession)
        DefineMessageMember(UsageLimit, LoginAPI::UsageLimit)
        DefineMessageMember(Entitlement, LoginAPI::Entitlement)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionValidate_v2
    BeginDefineMessage(SessionValidate_v2, Tracked, MESSAGE_SESSION_VALIDATE_v2)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
    EndDefineMessage


    ////////////////////////////////////////
    //  SessionValidateReply_v2
    BeginDefineMessage(SessionValidateReply_v2, TrackedReply, MESSAGE_SESSION_VALIDATE_REPLY_v2)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(Session, apiSession)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionValidate_v3
    BeginDefineMessage(SessionValidate_v3, Tracked, MESSAGE_SESSION_VALIDATE_v3)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
    EndDefineMessage


    ////////////////////////////////////////
    //  SessionValidateReply_v3
    BeginDefineMessage(SessionValidateReply_v3, TrackedReply, MESSAGE_SESSION_VALIDATE_REPLY_v3)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(Session, apiSession)
        DefineMessageMember(UsageLimit, LoginAPI::UsageLimit)
        DefineMessageMember(Padding, unsigned)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionValidate
    BeginDefineMessage(SessionValidate, Tracked, MESSAGE_SESSION_VALIDATE)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
    EndDefineMessage


    ////////////////////////////////////////
    //  SessionValidateReply
    BeginDefineMessage(SessionValidateReply, TrackedReply, MESSAGE_SESSION_VALIDATE_REPLY)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(Session, apiSession)
        DefineMessageMember(UsageLimit, LoginAPI::UsageLimit)
        DefineMessageMember(Entitlement, LoginAPI::Entitlement)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionValidateEx
    BeginDefineMessage(SessionValidateEx, Tracked, MESSAGE_SESSION_VALIDATE_EX)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
    EndDefineMessage


    ////////////////////////////////////////
    //  SessionValidateExReply
    BeginDefineMessage(SessionValidateExReply, TrackedReply, MESSAGE_SESSION_VALIDATE_EX_REPLY)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(Session, apiSession)
        DefineMessageMember(Password, std::string)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogin_v3
    BeginDefineMessage(SessionLogin_v3, Tracked, MESSAGE_SESSION_LOGIN_v3)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(ParentSessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
        DefineMessageMember(ClientIP, apiIP)
        DefineMessageMember(Forced, bool)
    EndDefineMessage

    
    ////////////////////////////////////////
    //  SessionLoginReply_v3
    BeginDefineMessage(SessionLoginReply_v3, TrackedReply, MESSAGE_SESSION_LOGIN_REPLY_v3)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(Session, apiSession)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogin_v4
    BeginDefineMessage(SessionLogin_v4, Tracked, MESSAGE_SESSION_LOGIN_v4)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(ParentSessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
        DefineMessageMember(ClientIP, apiIP)
        DefineMessageMember(Forced, bool)
    EndDefineMessage

    
    ////////////////////////////////////////
    //  SessionLoginReply_v4
    BeginDefineMessage(SessionLoginReply_v4, TrackedReply, MESSAGE_SESSION_LOGIN_REPLY_v4)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(Session, apiSession)
        DefineMessageMember(UsageLimit, LoginAPI::UsageLimit)
        DefineMessageMember(Padding, unsigned)
    EndDefineMessage


////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogin_v5
    BeginDefineMessage(SessionLogin_v5, Tracked, MESSAGE_SESSION_LOGIN_v5)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(ParentSessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
        DefineMessageMember(ClientIP, apiIP)
        DefineMessageMember(Flags, unsigned)
    EndDefineMessage

    
    ////////////////////////////////////////
    //  SessionLoginReply_v5
    BeginDefineMessage(SessionLoginReply_v5, TrackedReply, MESSAGE_SESSION_LOGIN_REPLY_v5)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(Session, apiSession)
        DefineMessageMember(UsageLimit, LoginAPI::UsageLimit)
        DefineMessageMember(Entitlement, LoginAPI::Entitlement)
    EndDefineMessage

////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLogin
    BeginDefineMessage(SessionLogin, Tracked, MESSAGE_SESSION_LOGIN)
        DefineMessageMember(UserName, std::string)
        DefineMessageMember(UserPassword, std::string)
        DefineMessageMember(ParentSessionID, std::string)
        DefineMessageMember(SessionType, apiSessionType)
        DefineMessageMember(ClientIP, apiIP)
        DefineMessageMember(Flags, unsigned)
		DefineMessageMember(Namespace, std::string)
    EndDefineMessage

    
    ////////////////////////////////////////
    //  SessionLoginReply
    BeginDefineMessage(SessionLoginReply, TrackedReply, MESSAGE_SESSION_LOGIN_REPLY)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(Session, apiSession)
        DefineMessageMember(UsageLimit, LoginAPI::UsageLimit)
        DefineMessageMember(Entitlement, LoginAPI::Entitlement)
    EndDefineMessage

////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SessionLoginInternal
    BeginDefineMessage(SessionLoginInternal, Tracked, MESSAGE_SESSION_LOGIN_INTERNAL)
        DefineMessageMember(AccountId, apiAccountId)
        DefineMessageMember(SessionType, apiSessionType)
        DefineMessageMember(ClientIP, apiIP)
        DefineMessageMember(Flags, unsigned)
    EndDefineMessage

    
    ////////////////////////////////////////
    //  SessionLoginInternalReply
    BeginDefineMessage(SessionLoginInternalReply, TrackedReply, MESSAGE_SESSION_LOGIN_INTERNAL_REPLY)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Subscription, apiSubscription)
        DefineMessageMember(Session, apiSession)
        DefineMessageMember(UsageLimit, LoginAPI::UsageLimit)
        DefineMessageMember(Entitlement, LoginAPI::Entitlement)
    EndDefineMessage



////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  NotifySessionKick
    BeginDefineMessage(NotifySessionKick, Basic, MESSAGE_NOTIFY_SESSION_KICK)
        DefineMessageMemberArray(SessionList, std::string)
    EndDefineMessage


    ////////////////////////////////////////
    //  NotifySessionKickRequest
    BeginDefineMessage(NotifySessionKickRequest, Basic, MESSAGE_NOTIFY_SESSION_KICK_REQUEST)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(Session, apiSession)
        DefineMessageMember(Reason, apiKickReason)
    EndDefineMessage


    ////////////////////////////////////////
    //  SessionKickReply
    BeginDefineMessage(SessionKickReply, Basic, MESSAGE_SESSION_KICK_REPLY)
        DefineMessageMember(SessionId, std::string)
        DefineMessageMember(Reply, apiKickReply)
    EndDefineMessage

    
////////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SubscriptionValidate_v1
    BeginDefineMessage(SubscriptionValidate_v1, Tracked, MESSAGE_SUBSCRIPTION_VALIDATE_v1)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(Gamecode, apiGamecode)
    EndDefineMessage


    ////////////////////////////////////////
    //  SubscriptionValidateReply_v1
    BeginDefineMessage(SubscriptionValidateReply_v1, TrackedReply, MESSAGE_SUBSCRIPTION_VALIDATE_REPLY_v1)
        DefineMessageMember(Subscription, apiSubscription_v1)
    EndDefineMessage


///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  SubscriptionValidate
    BeginDefineMessage(SubscriptionValidate, Tracked, MESSAGE_SUBSCRIPTION_VALIDATE)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(Gamecode, apiGamecode)
    EndDefineMessage


    ////////////////////////////////////////
    //  SubscriptionValidateReply
    BeginDefineMessage(SubscriptionValidateReply, TrackedReply, MESSAGE_SUBSCRIPTION_VALIDATE_REPLY)
        DefineMessageMember(Subscription, apiSubscription)
    EndDefineMessage


///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  MemberGetInformation
    BeginDefineMessage(MemberGetInformation, Tracked, MESSAGE_MEMBER_GET_INFORMATION)
        DefineMessageMember(AccountId, apiAccountId)
    EndDefineMessage


    ////////////////////////////////////////
    //  MemberGetInformationReply
    BeginDefineMessage(MemberGetInformationReply, TrackedReply, MESSAGE_MEMBER_GET_INFORMATION_REPLY)
        DefineMessageMember(Account, apiAccount)
        DefineMessageMember(FirstName, std::string)
        DefineMessageMember(LastName, std::string)
        DefineMessageMember(Gender, std::string)
        DefineMessageMember(Email, std::string)
		EndDefineMessage


		///////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////
	//  MemberGetInformation_v2
	BeginDefineMessage(MemberGetInformation_v2, Tracked, MESSAGE_MEMBER_GET_INFORMATION_v2)
		DefineMessageMember(AccountId, apiAccountId)
		EndDefineMessage


	////////////////////////////////////////
	//  MemberGetInformationReply_v2
	BeginDefineMessage(MemberGetInformationReply_v2, TrackedReply, MESSAGE_MEMBER_GET_INFORMATION_REPLY_v2)
		DefineMessageMember(Account, apiAccount)
		DefineMessageMember(FirstName, std::string)
		DefineMessageMember(LastName, std::string)
		DefineMessageMember(Gender, std::string)
		DefineMessageMember(Email, std::string)
		DefineMessageMember(DefaultCurrency, std::string)
		DefineMessageMember(DefaultCountry, std::string)
		DefineMessageMember(StationHandle, std::string)
		EndDefineMessage


///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  Kick
    BeginDefineMessage(Kick, Tracked, MESSAGE_KICK)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(Reason, unsigned)
    EndDefineMessage


    ////////////////////////////////////////
    //  KickReply
    BeginDefineMessage(KickReply, TrackedReply, MESSAGE_KICK_REPLY)
    EndDefineMessage


///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  ReqGetSessions
    BeginDefineMessage(ReqGetSessions, Tracked, MESSAGE_REQ_GET_SESSIONS)
        DefineMessageMember(AccountId, unsigned)
    EndDefineMessage


    ////////////////////////////////////////
    //  ReqGetSessionsReply
    BeginDefineMessage(ReqGetSessionsReply, TrackedReply, MESSAGE_REQ_GET_SESSIONS_REPLY)
		DefineMessageMemberArray(SessionArray, apiSession)
        DefineMessageMemberArray(SubscriptionArray, apiSubscription)
        DefineMessageMemberArray(UsageLimitArray, LoginAPI::UsageLimit)
        DefineMessageMemberArray(TimeCreated, unsigned)
        DefineMessageMemberArray(TimeTouched, unsigned)
    EndDefineMessage

///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  GetFeatures_v1
    BeginDefineMessage(GetFeatures_v1, Tracked, MESSAGE_FEATURE_GET_v1)
        DefineMessageMember(SessionID, std::string)
    EndDefineMessage


    ////////////////////////////////////////
    //  GetFeaturesReply_v1
    BeginDefineMessage(GetFeaturesReply_v1, TrackedReply, MESSAGE_FEATURE_GET_REPLY_v1)
        DefineMessageMemberArray(FeatureArray, LoginAPI::Feature)
    EndDefineMessage

///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  GetFeatures
    BeginDefineMessage(GetFeatures, Tracked, MESSAGE_FEATURE_GET)
        DefineMessageMember(SessionID, std::string)
		DefineMessageMember(AccountId, unsigned)
		DefineMessageMember(Gamecode, apiGamecode)
    EndDefineMessage


    ////////////////////////////////////////
    //  GetFeaturesReply
    BeginDefineMessage(GetFeaturesReply, TrackedReply, MESSAGE_FEATURE_GET_REPLY)
        DefineMessageMemberArray(FeatureArray, LoginAPI::Feature)
    EndDefineMessage

	///////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////
	//  GrantFeature
	BeginDefineMessage(GrantFeature, Tracked, MESSAGE_FEATURE_GRANT)
	DefineMessageMember(SessionID, std::string)
	DefineMessageMember(FeatureID, unsigned)
	EndDefineMessage


	////////////////////////////////////////
	//  GrantFeatureReply
	BeginDefineMessage(GrantFeatureReply, TrackedReply, MESSAGE_FEATURE_GRANT_REPLY)
	EndDefineMessage

	///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  ModifyFeature
    BeginDefineMessage(ModifyFeature, Tracked, MESSAGE_FEATURE_MODIFY)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(Feature, LoginAPI::Feature)
    EndDefineMessage


    ////////////////////////////////////////
    //  ModifyFeatureReply
    BeginDefineMessage(ModifyFeatureReply, TrackedReply, MESSAGE_FEATURE_MODIFY_REPLY)
    EndDefineMessage


///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  RevokeFeature
    BeginDefineMessage(RevokeFeature, Tracked, MESSAGE_FEATURE_REVOKE)
        DefineMessageMember(SessionID, std::string)
        DefineMessageMember(FeatureID, unsigned)
    EndDefineMessage


    ////////////////////////////////////////
    //  RevokeFeatureReply
    BeginDefineMessage(RevokeFeatureReply, TrackedReply, MESSAGE_FEATURE_REVOKE_REPLY)
    EndDefineMessage


///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  EnumerateFeatures
    BeginDefineMessage(EnumerateFeatures, Tracked, MESSAGE_FEATURE_ENUMERATE)
        DefineMessageMember(Gamecode, apiGamecode)
    EndDefineMessage


    ////////////////////////////////////////
    //  EnumerateFeaturesReply
    BeginDefineMessage(EnumerateFeaturesReply, TrackedReply, MESSAGE_FEATURE_ENUMERATE_REPLY)
        DefineMessageMemberArray(FeatureArray, LoginAPI::FeatureDescription)
    EndDefineMessage


///////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////
    //  SessionStartPlay
    BeginDefineMessage(SessionStartPlay, Tracked, MESSAGE_SESSION_START_PLAY)
        DefineMessageMember(SessionID, std::string)
		DefineMessageMember(ServerName, std::string)
		DefineMessageMember(CharacterName, std::string)
		DefineMessageMember(GameData, std::string)
    EndDefineMessage


    ////////////////////////////////////////
    //  SessionStartPlayReply
    BeginDefineMessage(SessionStartPlayReply, TrackedReply, MESSAGE_SESSION_START_PLAY_REPLY)
    EndDefineMessage

///////////////////////////////////////////////////////////////////////////////

    
    ////////////////////////////////////////
    //  StopPlay
    BeginDefineMessage(SessionStopPlay, Tracked, MESSAGE_SESSION_STOP_PLAY)
        DefineMessageMember(SessionID, std::string)
		DefineMessageMember(ServerName, std::string)
		DefineMessageMember(CharacterName, std::string)
    EndDefineMessage


    ////////////////////////////////////////
    //  StopPlayReply
    BeginDefineMessage(SessionStopPlayReply, TrackedReply, MESSAGE_SESSION_STOP_PLAY_REPLY)
    EndDefineMessage

/////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////
	//  GrantFeatureV2
	BeginDefineMessage(GrantFeatureV2, Tracked, MESSAGE_FEATURE_GRANT_v2)
	DefineMessageMember(SessionID, std::string)
	DefineMessageMember(FeatureID, unsigned)
	DefineMessageMember(GameCode, std::string)
	DefineMessageMember(ProviderID, unsigned)
	DefineMessageMember(PromoID, unsigned)
	EndDefineMessage


	////////////////////////////////////////
	//  GrantFeatureReplyV2
	BeginDefineMessage(GrantFeatureReplyV2, TrackedReply, MESSAGE_FEATURE_GRANT_REPLY_v2)
	EndDefineMessage

/////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////
	//  ModifyFeature_v2
	BeginDefineMessage(ModifyFeature_v2, Tracked, MESSAGE_MODIFY_FEATURE_v2)
	DefineMessageMember(AccountID, unsigned)
	DefineMessageMember(GameCode, std::string)
	DefineMessageMember(OrigFeature, LoginAPI::Feature)
	DefineMessageMember(NewFeature, LoginAPI::Feature)
	EndDefineMessage


	////////////////////////////////////////
	//  ModifyFeatureReply_v2
	BeginDefineMessage(ModifyFeatureReply_v2, TrackedReply, MESSAGE_MODIFY_FEATURE_REPLY_v2)
	DefineMessageMember(CurrentFeature, LoginAPI::Feature)
	EndDefineMessage

/////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////
	//  GrantFeatureByStationID
	BeginDefineMessage(GrantFeatureByStationID, Tracked, MESSAGE_GRANT_FEATURE_BY_STATION_ID)
	DefineMessageMember(AccountID, unsigned)
	DefineMessageMember(FeatureID, unsigned)
	DefineMessageMember(GameCode, std::string)
	DefineMessageMember(ProviderID, unsigned)
	DefineMessageMember(PromoID, unsigned)
	EndDefineMessage


	////////////////////////////////////////
	//  GrantFeatureReplyV2
	BeginDefineMessage(GrantFeatureByStationIDReply, TrackedReply, MESSAGE_GRANT_FEATURE_BY_STATION_ID_REPLY)
	EndDefineMessage

///////////////////////////////////////////////////////////////////////////////

}

#endif // MESSAGES_H
