#pragma warning (disable : 4786 )

#include <stdio.h>
#include <time.h>
#include "ClientCore.h"
#include "Messages.h"


using namespace std;
using namespace Base;


const int      KEEP_ALIVE_DELAY     = 15;


namespace LoginAPI
{


    ClientCore::ClientCore(apiClient * parent, const char * version, const char * serverList, const char * description, unsigned maxConnections) :
        apiCore(parent,version,serverList,description,maxConnections)
    {
        RegisterTrackedMessage(Message::MESSAGE_ACCOUNT_GET_STATUS_REPLY           );
        RegisterTrackedMessage(Message::MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_REPLY    );
        RegisterTrackedMessage(Message::MESSAGE_ACCOUNT_GET_SUBSCRIPTION_REPLY     );
        
        RegisterTrackedMessage(Message::MESSAGE_SESSION_CONSUME_REPLY_v2           );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_VALIDATE_REPLY_v2          );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_LOGIN_REPLY_v3             );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_VALIDATE_EX_REPLY          );

        RegisterTrackedMessage(Message::MESSAGE_SUBSCRIPTION_VALIDATE_REPLY        );

		RegisterTrackedMessage(Message::MESSAGE_MEMBER_GET_INFORMATION_REPLY       );
		RegisterTrackedMessage(Message::MESSAGE_MEMBER_GET_INFORMATION_REPLY_v2    );

        RegisterTrackedMessage(Message::MESSAGE_SESSION_LOGIN_REPLY_v4             );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_CONSUME_REPLY_v3           );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_VALIDATE_REPLY_v3          );
        RegisterTrackedMessage(Message::MESSAGE_REQ_GET_SESSIONS_REPLY			   );

        RegisterTrackedMessage(Message::MESSAGE_SESSION_LOGIN_REPLY_v5             );
		RegisterTrackedMessage(Message::MESSAGE_SESSION_LOGIN_INTERNAL_REPLY       );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_CONSUME_REPLY              );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_VALIDATE_REPLY             );
        RegisterTrackedMessage(Message::MESSAGE_FEATURE_GRANT_REPLY				   );
        RegisterTrackedMessage(Message::MESSAGE_FEATURE_MODIFY_REPLY               );
        RegisterTrackedMessage(Message::MESSAGE_FEATURE_REVOKE_REPLY               );
        RegisterTrackedMessage(Message::MESSAGE_FEATURE_ENUMERATE_REPLY			   );

		RegisterTrackedMessage(Message::MESSAGE_SESSION_START_PLAY_REPLY		   );
		RegisterTrackedMessage(Message::MESSAGE_SESSION_STOP_PLAY_REPLY			   );
		RegisterTrackedMessage(Message::MESSAGE_FEATURE_GET_REPLY				   );
		RegisterTrackedMessage(Message::MESSAGE_SESSION_LOGIN_REPLY                );
		RegisterTrackedMessage(Message::MESSAGE_KICK_REPLY                         );
		RegisterTrackedMessage(Message::MESSAGE_FEATURE_GRANT_REPLY_v2             );
		RegisterTrackedMessage(Message::MESSAGE_MODIFY_FEATURE_REPLY_v2            );
		RegisterTrackedMessage(Message::MESSAGE_GRANT_FEATURE_BY_STATION_ID_REPLY  );
	}

    ClientCore::ClientCore(apiClient * parent, const char * version, const char ** serverList, unsigned serverCount, const char * description, unsigned maxConnections) :
        apiCore(parent,version,serverList,serverCount,description,maxConnections)
    {
        RegisterTrackedMessage(Message::MESSAGE_ACCOUNT_GET_STATUS_REPLY           );
        RegisterTrackedMessage(Message::MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_REPLY    );
        RegisterTrackedMessage(Message::MESSAGE_ACCOUNT_GET_SUBSCRIPTION_REPLY     );
        
        RegisterTrackedMessage(Message::MESSAGE_SESSION_CONSUME_REPLY_v2           );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_VALIDATE_REPLY_v2          );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_LOGIN_REPLY_v3             );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_VALIDATE_EX_REPLY          );

        RegisterTrackedMessage(Message::MESSAGE_SUBSCRIPTION_VALIDATE_REPLY        );

		RegisterTrackedMessage(Message::MESSAGE_MEMBER_GET_INFORMATION_REPLY       );
		RegisterTrackedMessage(Message::MESSAGE_MEMBER_GET_INFORMATION_REPLY_v2    );

        RegisterTrackedMessage(Message::MESSAGE_SESSION_LOGIN_REPLY_v4             );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_CONSUME_REPLY_v3           );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_VALIDATE_REPLY_v3          );
        RegisterTrackedMessage(Message::MESSAGE_REQ_GET_SESSIONS_REPLY			   );

        RegisterTrackedMessage(Message::MESSAGE_SESSION_LOGIN_REPLY_v5             );
		RegisterTrackedMessage(Message::MESSAGE_SESSION_LOGIN_INTERNAL_REPLY       );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_CONSUME_REPLY              );
        RegisterTrackedMessage(Message::MESSAGE_SESSION_VALIDATE_REPLY             );
        RegisterTrackedMessage(Message::MESSAGE_FEATURE_GRANT_REPLY				   );
        RegisterTrackedMessage(Message::MESSAGE_FEATURE_MODIFY_REPLY               );
        RegisterTrackedMessage(Message::MESSAGE_FEATURE_REVOKE_REPLY               );
        RegisterTrackedMessage(Message::MESSAGE_FEATURE_ENUMERATE_REPLY			   );

		RegisterTrackedMessage(Message::MESSAGE_SESSION_START_PLAY_REPLY		   );
		RegisterTrackedMessage(Message::MESSAGE_SESSION_STOP_PLAY_REPLY			   );
		RegisterTrackedMessage(Message::MESSAGE_FEATURE_GET_REPLY				   );
		RegisterTrackedMessage(Message::MESSAGE_SESSION_LOGIN_REPLY                );
		RegisterTrackedMessage(Message::MESSAGE_KICK_REPLY                         );
		RegisterTrackedMessage(Message::MESSAGE_FEATURE_GRANT_REPLY_v2             );
		RegisterTrackedMessage(Message::MESSAGE_MODIFY_FEATURE_REPLY_v2            );
		RegisterTrackedMessage(Message::MESSAGE_GRANT_FEATURE_BY_STATION_ID_REPLY  );
    }

    ClientCore::~ClientCore()
    {
    }

    int ClientCore::GetKeepAliveDelay() 
    { 
        return KEEP_ALIVE_DELAY; 
    }
    
    apiTrackingNumber ClientCore::GetAccountStatus(const char * name, const char * password, const void * userData)
    {
        Message::AccountGetStatus        input;
        Message::AccountGetStatusReply   output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) user name
        if (!name || !name[0] || strlen(name) > MAX_ACCOUNT_NAME_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserName(name);
        //  (2) password
        if (!password || !password[0] || strlen(password) > MAX_PASSWORD_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserPassword(password);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::GetAccountStatus(const apiAccountId accountId, const void * userData)
    {
        Message::AccountGetStatus        input;
        Message::AccountGetStatusReply   output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) accountId
        if (!accountId)
            output.SetResult(RESULT_INVALID_ACCOUNT_ID);
        else
            input.SetAccountId(accountId);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::GetAccountSubscriptions(const char * name, const char * password, const void * userData)
    {
        Message::AccountGetSubscriptions       input;
        Message::AccountGetSubscriptionsReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) user name
        if (!name || !name[0] || strlen(name) > MAX_ACCOUNT_NAME_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserName(name);
        //  (2) password
        if (!password || !password[0] || strlen(password) > MAX_PASSWORD_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserPassword(password);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::GetAccountSubscriptions(const char * sessionID, const void * userData)
    {
        Message::AccountGetSubscriptions       input;
        Message::AccountGetSubscriptionsReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) sessionID
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::GetAccountSubscriptions(const apiAccountId accountId, const void * userData)
    {
        Message::AccountGetSubscriptions       input;
        Message::AccountGetSubscriptionsReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) accountId
        if (!accountId)
            output.SetResult(RESULT_INVALID_ACCOUNT_ID);
        else
            input.SetAccountId(accountId);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::GetAccountSubscription(const char * name, const char * password, const apiGamecode gamecode, const void * userData)
    {
        Message::AccountGetSubscription       input;
        Message::AccountGetSubscriptionReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) user name
        if (!name || !name[0] || strlen(name) > MAX_ACCOUNT_NAME_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserName(name);
        //  (2) password
        if (!password || !password[0] || strlen(password) > MAX_PASSWORD_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserPassword(password);
        
		input.SetGamecode(gamecode);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::GetAccountSubscription(const char * sessionID, const apiGamecode gamecode, const void * userData)
    {
        Message::AccountGetSubscription       input;
        Message::AccountGetSubscriptionReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) sessionID
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);
        
		input.SetGamecode(gamecode);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::GetAccountSubscription(const apiAccountId accountId, const apiGamecode gamecode, const void * userData)
    {
        Message::AccountGetSubscription       input;
        Message::AccountGetSubscriptionReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) accountId
        if (!accountId)
            output.SetResult(RESULT_INVALID_ACCOUNT_ID);
        else
            input.SetAccountId(accountId);
        
		input.SetGamecode(gamecode);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionLogin_v3(const char * name, const char * password, const apiSessionType sessionType, const apiIP clientIP, const bool forceLogin, const void * userData)
    {
        Message::SessionLogin_v3      input;
        Message::SessionLoginReply_v3 output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) user name
        if (!name || !name[0] || strlen(name) > MAX_ACCOUNT_NAME_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserName(name);
        //  (2) password
        if (!password || !password[0] || strlen(password) > MAX_PASSWORD_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserPassword(password);
        //  (3) session type
        if (sessionType<=SESSION_TYPE_NULL)
            output.SetResult(RESULT_INVALID_SESSION_TYPE);
        else
            input.SetSessionType(sessionType);
        //  (4) client ip address
        input.SetClientIP(clientIP);
        //  (5) force login (attempt will be made to kick current session)
        input.SetForced(forceLogin);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionLogin_v3(const char * parentSession, const apiSessionType sessionType, const apiIP clientIP, const bool forceLogin, const void * userData)
    {
        Message::SessionLogin_v3      input;
        Message::SessionLoginReply_v3 output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) user name
        if (!parentSession || !parentSession[0] || strlen(parentSession) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_PARENT_SESSION);
        else
            input.SetParentSessionID(parentSession);
        //  (2) session type
        if (sessionType<=SESSION_TYPE_NULL)
            output.SetResult(RESULT_INVALID_SESSION_TYPE);
        else
            input.SetSessionType(sessionType);
        //  (3) client ip address
        input.SetClientIP(clientIP);
        //  (4) force login (attempt will be made to kick current session)
        input.SetForced(forceLogin);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionLogin_v4(const char * name, const char * password, const apiSessionType sessionType, const apiIP clientIP, const bool forceLogin, const void * userData)
    {
        Message::SessionLogin_v4      input;
        Message::SessionLoginReply_v4 output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) user name
        if (!name || !name[0] || strlen(name) > MAX_ACCOUNT_NAME_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserName(name);
        //  (2) password
        if (!password || !password[0] || strlen(password) > MAX_PASSWORD_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserPassword(password);
        //  (3) session type
        if (sessionType<=SESSION_TYPE_NULL)
            output.SetResult(RESULT_INVALID_SESSION_TYPE);
        else
            input.SetSessionType(sessionType);
        //  (4) client ip address
        input.SetClientIP(clientIP);
        //  (5) force login (attempt will be made to kick current session)
        input.SetForced(forceLogin);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionLogin_v4(const char * parentSession, const apiSessionType sessionType, const apiIP clientIP, const bool forceLogin, const void * userData)
    {
        Message::SessionLogin_v4      input;
        Message::SessionLoginReply_v4 output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) user name
        if (!parentSession || !parentSession[0] || strlen(parentSession) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_PARENT_SESSION);
        else
            input.SetParentSessionID(parentSession);
        //  (2) session type
        if (sessionType<=SESSION_TYPE_NULL)
            output.SetResult(RESULT_INVALID_SESSION_TYPE);
        else
            input.SetSessionType(sessionType);
        //  (3) client ip address
        input.SetClientIP(clientIP);
        //  (4) force login (attempt will be made to kick current session)
        input.SetForced(forceLogin);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionLogin_v5(const char * name, const char * password, const apiSessionType sessionType, const apiIP clientIP, const unsigned flags, const void * userData)
    {
        Message::SessionLogin_v5      input;
        Message::SessionLoginReply_v5 output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) user name
        if (!name || !name[0] || strlen(name) > MAX_ACCOUNT_NAME_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserName(name);
        //  (2) password
        if (!password || !password[0] || strlen(password) > MAX_PASSWORD_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserPassword(password);
        //  (3) session type
        if (sessionType<=SESSION_TYPE_NULL)
            output.SetResult(RESULT_INVALID_SESSION_TYPE);
        else
            input.SetSessionType(sessionType);
        //  (4) client ip address
        input.SetClientIP(clientIP);
        //  (5) flags
        input.SetFlags(flags);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionLogin_v5(const char * parentSession, const apiSessionType sessionType, const apiIP clientIP, const unsigned flags, const void * userData)
    {
        Message::SessionLogin_v5      input;
        Message::SessionLoginReply_v5 output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) user name
        if (!parentSession || !parentSession[0] || strlen(parentSession) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_PARENT_SESSION);
        else
            input.SetParentSessionID(parentSession);
        //  (2) session type
        if (sessionType<=SESSION_TYPE_NULL)
            output.SetResult(RESULT_INVALID_SESSION_TYPE);
        else
            input.SetSessionType(sessionType);
        //  (3) client ip address
        input.SetClientIP(clientIP);
        //  (4) flags
        input.SetFlags(flags);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

	apiTrackingNumber ClientCore::SessionLogin(const char * name, const char * password, const apiSessionType sessionType, const apiIP clientIP, const unsigned flags, const char * sessionNamespace, const void * userData)
    {
        Message::SessionLogin      input;
        Message::SessionLoginReply output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) user name
        if (!name || !name[0] || strlen(name) > MAX_ACCOUNT_NAME_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserName(name);
        //  (2) password
        if (!password || !password[0] || strlen(password) > MAX_PASSWORD_LENGTH)
            output.SetResult(RESULT_INVALID_NAME_OR_PASSWORD);
        else
            input.SetUserPassword(password);
        //  (3) session type
        if (sessionType<=SESSION_TYPE_NULL)
            output.SetResult(RESULT_INVALID_SESSION_TYPE);
        else
            input.SetSessionType(sessionType);
        //  (4) client ip address
        input.SetClientIP(clientIP);
        //  (5) flags
        input.SetFlags(flags);
		//  (6) namespace
		if (!sessionNamespace || !sessionNamespace[0])
			input.SetNamespace(_defaultNamespace);
		else if (strlen(sessionNamespace) > MAX_NAMESPACE_LENGTH)
            output.SetResult(RESULT_INVALID_NAMESPACE);
        else
            input.SetNamespace(sessionNamespace);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionLogin(const char * parentSession, const apiSessionType sessionType, const apiIP clientIP, const unsigned flags, const void * userData)
    {
        Message::SessionLogin      input;
        Message::SessionLoginReply output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) user name
        if (!parentSession || !parentSession[0] || strlen(parentSession) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_PARENT_SESSION);
        else
            input.SetParentSessionID(parentSession);
        //  (2) session type
        if (sessionType<=SESSION_TYPE_NULL)
            output.SetResult(RESULT_INVALID_SESSION_TYPE);
        else
            input.SetSessionType(sessionType);
        //  (3) client ip address
        input.SetClientIP(clientIP);
        //  (4) flags
        input.SetFlags(flags);
	
        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

	apiTrackingNumber ClientCore::SessionLoginInternal(const apiAccountId accountId, const apiSessionType sessionType, const apiIP clientIP, const unsigned flags, const void * userData)
    {
        Message::SessionLoginInternal      input;
        Message::SessionLoginInternalReply output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) accountId
		if (!accountId)
            output.SetResult(RESULT_INVALID_ACCOUNT_ID);
        else
            input.SetAccountId(accountId);
        //  (2) session type
        if (sessionType<=SESSION_TYPE_NULL)
            output.SetResult(RESULT_INVALID_SESSION_TYPE);
        else
            input.SetSessionType(sessionType);
        //  (3) client ip address
        input.SetClientIP(clientIP);
        //  (4) flags
        input.SetFlags(flags);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionConsume_v2(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        Message::SessionConsume_v2        input;
        Message::SessionConsumeReply_v2   output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) sessionID
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);
        //  (2) session type
        if (sessionType<=SESSION_TYPE_NULL)
            output.SetResult(RESULT_INVALID_SESSION_TYPE);
        else
            input.SetSessionType(sessionType);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionConsume_v3(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        Message::SessionConsume_v3        input;
        Message::SessionConsumeReply_v3   output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) sessionID
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);
        //  (2) session type
        if (sessionType<=SESSION_TYPE_NULL)
            output.SetResult(RESULT_INVALID_SESSION_TYPE);
        else
            input.SetSessionType(sessionType);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionConsume(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        Message::SessionConsume        input;
        Message::SessionConsumeReply   output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) sessionID
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);
        //  (2) session type
        if (sessionType<=SESSION_TYPE_NULL)
            output.SetResult(RESULT_INVALID_SESSION_TYPE);
        else
            input.SetSessionType(sessionType);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionValidate_v2(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        Message::SessionValidate_v2       input;
        Message::SessionValidateReply_v2  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) sessionID
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);

        // don't validate session type, there are clients that pass in nullptr.
        input.SetSessionType(sessionType);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionValidate_v3(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        Message::SessionValidate_v3       input;
        Message::SessionValidateReply_v3  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) sessionID
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);

        // don't validate session type, there are clients that pass in nullptr.
        input.SetSessionType(sessionType);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionValidate(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        Message::SessionValidate       input;
        Message::SessionValidateReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) sessionID
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);

        // don't validate session type, there are clients that pass in nullptr.
        input.SetSessionType(sessionType);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::SessionValidateEx(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        Message::SessionValidateEx       input;
        Message::SessionValidateExReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) sessionID
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);

        // don't validate session type, there are clients that pass in nullptr.
        input.SetSessionType(sessionType);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    void ClientCore::SessionLogout(const char * sessionID)
    {
        Message::SessionLogout        input;

        vector<std::string> sessionList;
        
        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) sessionID
        if (!sessionID || strlen(sessionID) > SESSION_TICKET_LENGTH)
            return;
        else
            sessionList.push_back(sessionID);

        input.SetSessionArray(sessionList);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input);
    }

    void ClientCore::SessionLogout(const char ** sessionList, const unsigned sessionCount)
    {
        Message::SessionLogout input;

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) session array
        vector<std::string> sessionArray;
        for (unsigned i=0; i<sessionCount; i++)
            sessionArray.push_back(std::string(sessionList[i]));
        input.SetSessionArray(sessionArray);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input);
    }

    void ClientCore::SessionTouch(const char * sessionID)
    {
        Message::SessionTouch        input;

        vector<std::string> sessionList;
        
        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) sessionID
        if (!sessionID || strlen(sessionID) > SESSION_TICKET_LENGTH)
            return;
        else
            sessionList.push_back(sessionID);

        input.SetSessionArray(sessionList);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input);
    }

    void ClientCore::SessionTouch(const char ** sessionList, const unsigned sessionCount)
    {
        Message::SessionTouch input;

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) session array
        vector<std::string> sessionArray;
        for (unsigned i=0; i<sessionCount; i++)
            sessionArray.push_back(std::string(sessionList[i]));
        input.SetSessionArray(sessionArray);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input);
    }

    apiTrackingNumber ClientCore::SubscriptionValidate(const char * sessionID, const apiGamecode gamecode, const void * userData)
    {
        Message::SubscriptionValidate       input;
        Message::SubscriptionValidateReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) sessionID
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);
        //  (2) gamecode
        
		input.SetGamecode(gamecode);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::GetMemberInformation(const apiAccountId accountId, const void * userData)
    {
        Message::MemberGetInformation       input;
        Message::MemberGetInformationReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) accountId
        if (!accountId)
            output.SetResult(RESULT_INVALID_ACCOUNT_ID);
        else
            input.SetAccountId(accountId);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
	}

	apiTrackingNumber ClientCore::GetMemberInformation_v2(const apiAccountId accountId, const void * userData)
	{
		Message::MemberGetInformation_v2       input;
		Message::MemberGetInformationReply_v2  output;

		////////////////////////////////////////
		//  Set default result value
		output.SetResult(RESULT_TIMEOUT);

		////////////////////////////////////////
		//  Validate and set input parameters
		//  (1) accountId
		if (!accountId)
			output.SetResult(RESULT_INVALID_ACCOUNT_ID);
		else
			input.SetAccountId(accountId);

		////////////////////////////////////////
		//  Submit the request
		SubmitRequest(input, output, userData);

		return input.GetTrackingNumber();
	}

    apiTrackingNumber ClientCore::GetSessions(const apiAccountId accountId, const void * userData)
    {
        Message::ReqGetSessions input;
        Message::ReqGetSessionsReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) accountId
        if (!accountId)
            output.SetResult(RESULT_INVALID_ACCOUNT_ID);
        else
            input.SetAccountId(accountId);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    void ClientCore::SessionKickReply(const char * sessionID, apiKickReply reply)
    {
        Message::SessionKickReply input;

        ////////////////////////////////////////
        //  Validate and set input parameters
        //  (1) sessionID
        if (!sessionID || strlen(sessionID) > SESSION_TICKET_LENGTH)
            return;
        else
            input.SetSessionId(sessionID);
        //  (2) kick reply
        if (reply<=KICK_REPLY_NULL || reply>=KICK_REPLY_END)
            return;
        else
            input.SetReply(reply);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input);
    }

	apiTrackingNumber ClientCore::GetFeatures(const char * sessionID, const void * userData)
	{
        Message::GetFeatures input;
        Message::GetFeaturesReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
	}

	apiTrackingNumber ClientCore::GetFeatures(const apiAccountId accountId, const apiGamecode gamecode, const void * userData)
	{
        Message::GetFeatures input;
        Message::GetFeaturesReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
		//  (1) accountId
        if (!accountId)
            output.SetResult(RESULT_INVALID_ACCOUNT_ID);
        else
            input.SetAccountId(accountId);
		
		input.SetGamecode(gamecode);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
	}

    apiTrackingNumber ClientCore::GrantFeature(const char * sessionID, unsigned featureType, const void * userData)
    {
        Message::GrantFeature input;
        Message::GrantFeatureReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);
        input.SetFeatureID(featureType);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::ModifyFeature(const char * sessionID, const Feature & feature, const void * userData)
    {
        Message::ModifyFeature input;
        Message::ModifyFeatureReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);
        input.SetFeature(feature);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }
    apiTrackingNumber ClientCore::RevokeFeature(const char * sessionID, unsigned featureType, const void * userData)
    {
        Message::RevokeFeature input;
        Message::RevokeFeatureReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);
        input.SetFeatureID(featureType);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

    apiTrackingNumber ClientCore::EnumerateFeatures(apiGamecode gamecode, const void * userData)
    {
        Message::EnumerateFeatures input;
        Message::EnumerateFeaturesReply  output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        ////////////////////////////////////////
        //  Validate and set input parameters
        
		input.SetGamecode(gamecode);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

	apiTrackingNumber ClientCore::SessionStartPlay(const char * sessionID, 
													const char * serverName, 
													const char * characterName,
													const char * gameData,
													const void * userData)
	{
        Message::SessionStartPlay      input;
        Message::SessionStartPlayReply output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

		////////////////////////////////////////
        //  Validate and set input parameters
        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);

		if (!serverName || !serverName[0] || strlen(serverName) > SESSION_SERVER_NAME_LENGTH)
            output.SetResult(RESULT_INVALID_SERVER_NAME);
        else
            input.SetServerName(serverName);

		if (!characterName || !characterName[0] || strlen(characterName) > SESSION_CHARACTER_NAME_LENGTH)
            output.SetResult(RESULT_INVALID_CHARACTER_NAME);
        else
            input.SetCharacterName(characterName);

		if (gameData && gameData[0])
			input.SetGameData(gameData);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
	}

	apiTrackingNumber ClientCore::SessionStopPlay(const char * sessionID, 
													const char * serverName, 
													const char * characterName,
													const void * userData)
	{
        Message::SessionStopPlay      input;
        Message::SessionStopPlayReply output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);
        
		if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);

		if (!serverName || !serverName[0] || strlen(serverName) > SESSION_SERVER_NAME_LENGTH)
            output.SetResult(RESULT_INVALID_SERVER_NAME);
        else
            input.SetServerName(serverName);

		if (!characterName || !characterName[0] || strlen(characterName) > SESSION_CHARACTER_NAME_LENGTH)
            output.SetResult(RESULT_INVALID_CHARACTER_NAME);
        else
            input.SetCharacterName(characterName);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
	}

    apiTrackingNumber ClientCore::SessionKick(const char * sessionID, apiKickReason reason, const void * userData)
    {
        Message::Kick      input;
        Message::KickReply output;

        ////////////////////////////////////////
        //  Set default result value
        output.SetResult(RESULT_TIMEOUT);

        if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
            output.SetResult(RESULT_INVALID_SESSION);
        else
            input.SetSessionID(sessionID);

        if (reason <= KICK_REASON_NULL || reason >= KICK_REASON_END)
            output.SetResult(RESULT_INVALID_KICK_REASON);
        else
            input.SetReason(reason);

        ////////////////////////////////////////
        //  Submit the request
        SubmitRequest(input, output, userData);

        return input.GetTrackingNumber();
    }

	apiTrackingNumber ClientCore::GrantFeature_v2(const char * sessionID, unsigned featureType,	const char * gameCode, unsigned providerID, unsigned promoID, const void * userData)
	{
		Message::GrantFeatureV2      input;
		Message::GrantFeatureReplyV2 output;

		////////////////////////////////////////
		//  Set default result value
		output.SetResult(RESULT_TIMEOUT);

		if (!sessionID || !sessionID[0] || strlen(sessionID) > SESSION_TICKET_LENGTH)
			output.SetResult(RESULT_INVALID_SESSION);
		else
			input.SetSessionID(sessionID);
		
		input.SetFeatureID(featureType);

		if (!gameCode || !gameCode[0] || strlen(gameCode) > MAX_GAME_CODE_LENGTH)
			output.SetResult(RESULT_INVALID_GAME_CODE_SIZE);
		else
			input.SetGameCode(gameCode);

		input.SetProviderID(providerID);

		input.SetPromoID(promoID);

		////////////////////////////////////////
		//  Submit the request
		SubmitRequest(input, output, userData);

		return input.GetTrackingNumber();
	}


	apiTrackingNumber ClientCore::ModifyFeature_v2( unsigned accountID, 
													const char * gameCode,
													const Feature & origFeature,
													const Feature & newFeature,
													const void * userData)
	{
		Message::ModifyFeature_v2      input;
		Message::ModifyFeatureReply_v2 output;

		////////////////////////////////////////
		//  Set default result value
		output.SetResult(RESULT_TIMEOUT);

		input.SetAccountID(accountID);

		if (!gameCode || !gameCode[0] || strlen(gameCode) > MAX_GAME_CODE_LENGTH)
			output.SetResult(RESULT_INVALID_GAME_CODE_SIZE);
		else
			input.SetGameCode(gameCode);
		input.SetOrigFeature(origFeature);
		input.SetNewFeature(newFeature);

		////////////////////////////////////////
		//  Submit the request
		SubmitRequest(input, output, userData);

		return input.GetTrackingNumber();
	}

	apiTrackingNumber ClientCore::GrantFeatureByStationID(unsigned accountID,
														  unsigned featureType,
														  const char * gameCode,
														  unsigned providerID,
														  unsigned promoID,
														  const void * userData)
	{
		Message::GrantFeatureByStationID input;
		Message::GrantFeatureByStationIDReply output;

		////////////////////////////////////////
		//  Set default result value
		output.SetResult(RESULT_TIMEOUT);

		input.SetAccountID(accountID);

		input.SetFeatureID(featureType);

		if (!gameCode || !gameCode[0] || strlen(gameCode) > MAX_GAME_CODE_LENGTH)
			output.SetResult(RESULT_INVALID_GAME_CODE_SIZE);
		else
			input.SetGameCode(gameCode);

		input.SetProviderID(providerID);

		input.SetPromoID(promoID);

		////////////////////////////////////////
		//  Submit the request
		SubmitRequest(input, output, userData);

		return input.GetTrackingNumber();
	}

    void ClientCore::OnGetAccountStatus(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::AccountGetStatusReply message(iterator);
        
        GetClient()->OnGetAccountStatus(message.GetTrackingNumber(),
                                        message.GetResult(),
                                        message.GetAccount(),
                                        userData);
    }

    void ClientCore::OnGetAccountSubscriptions(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::AccountGetSubscriptionsReply message(iterator);
        
        GetClient()->OnGetAccountSubscriptions(message.GetTrackingNumber(),
                                               message.GetResult(),
                                               message.GetAccount(),
                                               message.GetSubscriptions().size() ? &(message.GetSubscriptions()[0]) : 0,
                                               message.GetSubscriptions().size(),
                                               userData);
    }

    void ClientCore::OnGetAccountSubscription(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::AccountGetSubscriptionReply message(iterator);
        
        GetClient()->OnGetAccountSubscription(message.GetTrackingNumber(),
                                            message.GetResult(),
                                            message.GetAccount(),
                                            message.GetSubscription(),
                                            userData);
    }

    void ClientCore::OnSessionLogin_v3(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionLoginReply_v3 message(iterator);

        GetClient()->OnSessionLogin_v3(message.GetTrackingNumber(),
                                    message.GetResult(),
                                    message.GetAccount(),
                                    message.GetSubscription(),
                                    message.GetSession(),
                                    userData);
    }

    void ClientCore::OnSessionLogin_v4(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionLoginReply_v4 message(iterator);

        GetClient()->OnSessionLogin_v4(message.GetTrackingNumber(),
                                    message.GetResult(),
                                    message.GetAccount(),
                                    message.GetSubscription(),
                                    message.GetSession(),
									message.GetUsageLimit(),
                                    userData);
    }

    void ClientCore::OnSessionLogin_v5(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionLoginReply_v5 message(iterator);

        GetClient()->OnSessionLogin_v5(message.GetTrackingNumber(),
                                    message.GetResult(),
                                    message.GetAccount(),
                                    message.GetSubscription(),
                                    message.GetSession(),
									message.GetUsageLimit(),
                                    message.GetEntitlement(),
                                    userData);
    }

    void ClientCore::OnSessionLogin(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionLoginReply message(iterator);

        GetClient()->OnSessionLogin(message.GetTrackingNumber(),
                                    message.GetResult(),
                                    message.GetAccount(),
                                    message.GetSubscription(),
                                    message.GetSession(),
									message.GetUsageLimit(),
									message.GetEntitlement(),
                                    userData);
    }

	void ClientCore::OnSessionLoginInternal(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionLoginInternalReply message(iterator);

        GetClient()->OnSessionLoginInternal(message.GetTrackingNumber(),
                                    message.GetResult(),
                                    message.GetAccount(),
                                    message.GetSubscription(),
                                    message.GetSession(),
									message.GetUsageLimit(),
									message.GetEntitlement(),
                                    userData);
    }

    void ClientCore::OnSessionValidate_v2(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionValidateReply_v2 message(iterator);
        
        GetClient()->OnSessionValidate_v2(message.GetTrackingNumber(),
                                       message.GetResult(),
                                       message.GetAccount(),
                                       message.GetSubscription(),
                                       message.GetSession(),
									   userData);
    }

    void ClientCore::OnSessionValidate_v3(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionValidateReply_v3 message(iterator);
        
        GetClient()->OnSessionValidate_v3(message.GetTrackingNumber(),
                                       message.GetResult(),
                                       message.GetAccount(),
                                       message.GetSubscription(),
                                       message.GetSession(),
                                       message.GetUsageLimit(),
									   userData);
    }

    void ClientCore::OnSessionValidate(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionValidateReply message(iterator);
        
        GetClient()->OnSessionValidate(message.GetTrackingNumber(),
                                       message.GetResult(),
                                       message.GetAccount(),
                                       message.GetSubscription(),
                                       message.GetSession(),
                                       message.GetUsageLimit(),
									   message.GetEntitlement(),
									   userData);
    }

    void ClientCore::OnSessionValidateEx(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionValidateExReply message(iterator);
        
        GetClient()->OnSessionValidateEx(message.GetTrackingNumber(),
                                       message.GetResult(),
                                       message.GetAccount(),
                                       message.GetSubscription(),
                                       message.GetSession(),
                                       message.GetPassword().c_str(),
                                       userData);
    }

    void ClientCore::OnSessionConsume_v2(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionConsumeReply_v2 message(iterator);
        
        GetClient()->OnSessionConsume_v2(message.GetTrackingNumber(),
                                      message.GetResult(),
                                      message.GetAccount(),
                                      message.GetSubscription(),
                                      message.GetSession(),
                                      userData);
    }

    void ClientCore::OnSessionConsume_v3(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionConsumeReply_v3 message(iterator);
        
        GetClient()->OnSessionConsume_v3(message.GetTrackingNumber(),
                                      message.GetResult(),
                                      message.GetAccount(),
                                      message.GetSubscription(),
                                      message.GetSession(),
                                      message.GetUsageLimit(),
                                      userData);
    }

    void ClientCore::OnSessionConsume(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionConsumeReply message(iterator);
        
        GetClient()->OnSessionConsume(message.GetTrackingNumber(),
                                      message.GetResult(),
                                      message.GetAccount(),
                                      message.GetSubscription(),
                                      message.GetSession(),
                                      message.GetUsageLimit(),
									  message.GetEntitlement(),
                                      userData);
    }

    void ClientCore::OnSubscriptionValidate(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SubscriptionValidateReply message(iterator);
        
        GetClient()->OnSubscriptionValidate(message.GetTrackingNumber(),
                                            message.GetResult(),
                                            message.GetSubscription(),
                                            userData);
    }

    void ClientCore::OnGetMemberInformation(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::MemberGetInformationReply message(iterator);
        
        GetClient()->OnGetMemberInformation(message.GetTrackingNumber(),
                                            message.GetResult(),
                                            message.GetAccount(),
                                            message.GetFirstName().c_str(),
                                            message.GetLastName().c_str(),
                                            message.GetGender().c_str(),
                                            message.GetEmail().c_str(),
                                            userData);
	}

	void ClientCore::OnGetMemberInformation_v2(const Base::ByteStream & stream, void * userData)
	{
		Base::ByteStream::ReadIterator iterator = stream.begin();
		Message::MemberGetInformationReply_v2 message(iterator);

		GetClient()->OnGetMemberInformation_v2(message.GetTrackingNumber(),
			message.GetResult(),
			message.GetAccount(),
			message.GetFirstName().c_str(),
			message.GetLastName().c_str(),
			message.GetGender().c_str(),
			message.GetEmail().c_str(),
			message.GetDefaultCurrency().c_str(),
			message.GetDefaultCountry().c_str(),
			message.GetStationHandle().c_str(),
			userData);
	}

    void ClientCore::OnGetSessions(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::ReqGetSessionsReply message(iterator);
        
		unsigned count = message.GetSessionArray().size();
        GetClient()->OnGetSessions(message.GetTrackingNumber(),
                             message.GetResult(),
							 count,
							 count ? &message.GetSessionArray()[0] : 0,
							 count ? &message.GetSubscriptionArray()[0] : 0,
							 count ? &message.GetUsageLimitArray()[0] : 0,
							 count ? &message.GetTimeCreated()[0] : 0,
							 count ? &message.GetTimeTouched()[0] : 0,
                             userData);
    }

    void ClientCore::NotifySessionKickRequest(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::NotifySessionKickRequest message(iterator);
        
        GetClient()->NotifySessionKickRequest(message.GetAccount(),
                                   message.GetSession(),
                                   message.GetReason());
    }

    void ClientCore::NotifySessionKick(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::NotifySessionKick message(iterator);

        std::vector<const char *> sessionList;
        for (std::vector<std::string>::const_iterator i=message.GetSessionList().begin(); i!=message.GetSessionList().end(); i++)
        {
            sessionList.push_back(i->c_str());
        }

        GetClient()->NotifySessionKick(&sessionList[0],
                                   sessionList.size());
	}

    void ClientCore::OnGetFeatures(const Base::ByteStream & stream, void * userData)
	{
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::GetFeaturesReply message(iterator);
        
        GetClient()->OnGetFeatures(message.GetTrackingNumber(),
                                            message.GetResult(),
											message.GetFeatureArray().size(),
											message.GetFeatureArray().size() ? &message.GetFeatureArray()[0] : 0,
                                            userData);
	}

    void ClientCore::OnGrantFeature(const Base::ByteStream & stream, void * userData)
	{
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::GrantFeatureReply message(iterator);
        
        GetClient()->OnGrantFeature(message.GetTrackingNumber(),
                                            message.GetResult(),
                                            userData);
	}

    void ClientCore::OnModifyFeature(const Base::ByteStream & stream, void * userData)
	{
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::ModifyFeatureReply message(iterator);
        
        GetClient()->OnModifyFeature(message.GetTrackingNumber(),
                                            message.GetResult(),
                                            userData);
	}

    void ClientCore::OnRevokeFeature(const Base::ByteStream & stream, void * userData)
	{
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::RevokeFeatureReply message(iterator);
        
        GetClient()->OnRevokeFeature(message.GetTrackingNumber(),
                                            message.GetResult(),
                                            userData);
	}

    void ClientCore::OnEnumerateFeatures(const Base::ByteStream & stream, void * userData)
	{
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::EnumerateFeaturesReply message(iterator);

        int count = message.GetFeatureArray().size();
        GetClient()->OnEnumerateFeatures(message.GetTrackingNumber(),
                                            message.GetResult(),
											count,
											count ? &message.GetFeatureArray()[0] : 0,
                                            userData);
	}

	void ClientCore::OnSessionStartPlay(const Base::ByteStream & stream, void * userData)
	{
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionStartPlayReply message(iterator);

        GetClient()->OnSessionStartPlay(message.GetTrackingNumber(),
                                    message.GetResult(),
                                    userData);
	}

	void ClientCore::OnSessionStopPlay(const Base::ByteStream & stream, void * userData)
	{
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::SessionStopPlayReply message(iterator);

        GetClient()->OnSessionStopPlay(message.GetTrackingNumber(),
                                    message.GetResult(),
                                    userData);
	}

    void ClientCore::OnSessionKick(const Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::KickReply message(iterator);

        GetClient()->OnSessionKick(message.GetTrackingNumber(),
                                    message.GetResult(),
                                    userData);
    }


	void ClientCore::OnGrantFeatureV2(const Base::ByteStream & stream, void * userData)
	{
		Base::ByteStream::ReadIterator iterator = stream.begin();
		Message::GrantFeatureReplyV2 message(iterator);

		GetClient()->OnGrantFeatureV2(message.GetTrackingNumber(),
			message.GetResult(),
			userData);
	}

	void ClientCore::OnModifyFeature_v2(const Base::ByteStream & stream, void * userData)
	{
		Base::ByteStream::ReadIterator iterator = stream.begin();
		Message::ModifyFeatureReply_v2 message(iterator);

		GetClient()->OnModifyFeature_v2(message.GetTrackingNumber(),
			message.GetResult(),
			message.GetCurrentFeature(),
			userData);
	}

	void ClientCore::OnGrantFeatureByStationID(const Base::ByteStream & stream, void * userData)
	{
		Base::ByteStream::ReadIterator iterator = stream.begin();
		Message::GrantFeatureByStationIDReply message(iterator);

		GetClient()->OnGrantFeatureByStationID(message.GetTrackingNumber(),
			message.GetResult(),
			userData);
	}

    bool ClientCore::Callback(Base::ByteStream & stream, void * userData)
    {
        Base::ByteStream::ReadIterator iterator = stream.begin();
        Message::Basic message(iterator);

        switch (message.GetMessageID())
        {
            case Message::MESSAGE_ACCOUNT_GET_STATUS_REPLY:
                OnGetAccountStatus(stream, userData);
                break;
            case Message::MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_REPLY:
                OnGetAccountSubscriptions(stream, userData);
                break;
            case Message::MESSAGE_ACCOUNT_GET_SUBSCRIPTION_REPLY:
                OnGetAccountSubscription(stream, userData);
                break;
            case Message::MESSAGE_SESSION_LOGIN_REPLY_v3:
                OnSessionLogin_v3(stream, userData);
                break;
            case Message::MESSAGE_SESSION_LOGIN_REPLY_v4:
                OnSessionLogin_v4(stream, userData);
                break;
            case Message::MESSAGE_SESSION_LOGIN_REPLY_v5:
                OnSessionLogin_v5(stream, userData);
                break;
            case Message::MESSAGE_SESSION_LOGIN_REPLY:
                OnSessionLogin(stream, userData);
                break;
			case Message::MESSAGE_SESSION_LOGIN_INTERNAL_REPLY:
                OnSessionLoginInternal(stream, userData);
                break;
            case Message::MESSAGE_SESSION_CONSUME_REPLY_v2:
                OnSessionConsume_v2(stream, userData);
                break;
            case Message::MESSAGE_SESSION_CONSUME_REPLY_v3:
                OnSessionConsume_v3(stream, userData);
                break;
            case Message::MESSAGE_SESSION_CONSUME_REPLY:
                OnSessionConsume(stream, userData);
                break;
            case Message::MESSAGE_SESSION_VALIDATE_REPLY_v2:
                OnSessionValidate_v2(stream, userData);
                break;
            case Message::MESSAGE_SESSION_VALIDATE_REPLY_v3:
                OnSessionValidate_v3(stream, userData);
                break;
            case Message::MESSAGE_SESSION_VALIDATE_REPLY:
                OnSessionValidate(stream, userData);
                break;
            case Message::MESSAGE_SESSION_VALIDATE_EX_REPLY:
                OnSessionValidateEx(stream, userData);
                break;
            case Message::MESSAGE_SUBSCRIPTION_VALIDATE_REPLY:
                OnSubscriptionValidate(stream, userData);
                break;
            case Message::MESSAGE_MEMBER_GET_INFORMATION_REPLY:
                OnGetMemberInformation(stream, userData);
				break;
			case Message::MESSAGE_MEMBER_GET_INFORMATION_REPLY_v2:
				OnGetMemberInformation_v2(stream, userData);
				break;
            case Message::MESSAGE_REQ_GET_SESSIONS_REPLY:
                OnGetSessions(stream, userData);
                break;
            case Message::MESSAGE_FEATURE_GET_REPLY:
                OnGetFeatures(stream, userData);
                break;
            case Message::MESSAGE_FEATURE_GRANT_REPLY:
                OnGrantFeature(stream, userData);
                break;
            case Message::MESSAGE_FEATURE_REVOKE_REPLY:
                OnRevokeFeature(stream, userData);
                break;
            case Message::MESSAGE_FEATURE_MODIFY_REPLY:
                OnModifyFeature(stream, userData);
                break;
            case Message::MESSAGE_FEATURE_ENUMERATE_REPLY:
                OnEnumerateFeatures(stream, userData);
                break;
			case Message::MESSAGE_SESSION_START_PLAY_REPLY:
                OnSessionStartPlay(stream, userData);
                break;
			case Message::MESSAGE_SESSION_STOP_PLAY_REPLY:
                OnSessionStopPlay(stream, userData);
				break;
			case Message::MESSAGE_KICK_REPLY:
				OnSessionKick(stream, userData);
				break;
			case Message::MESSAGE_FEATURE_GRANT_REPLY_v2:
				OnGrantFeatureV2(stream, userData);
				break;
            case Message::MESSAGE_NOTIFY_SESSION_KICK_REQUEST:
                NotifySessionKickRequest(stream, userData);
				break;
			case Message::MESSAGE_NOTIFY_SESSION_KICK:
				NotifySessionKick(stream, userData);
				break;
			case Message::MESSAGE_MODIFY_FEATURE_REPLY_v2:
				OnModifyFeature_v2(stream, userData);
				break;
			case Message::MESSAGE_GRANT_FEATURE_BY_STATION_ID_REPLY:
				OnGrantFeatureByStationID(stream, userData);
				break;
            default:
                break;
        }

        return true;
    }    

    void ClientCore::Timeout(unsigned short messageId, apiTrackingNumber trackingNumber, void * userData)
    {
        switch (messageId)
        {
            case Message::MESSAGE_ACCOUNT_GET_STATUS_REPLY:
                GetClient()->OnGetAccountStatus(trackingNumber,
                                                RESULT_TIMEOUT,
                                                apiAccount(),
                                                userData);
                break;
            case Message::MESSAGE_ACCOUNT_GET_SUBSCRIPTIONS_REPLY:
                GetClient()->OnGetAccountSubscriptions(trackingNumber,
                                                       RESULT_TIMEOUT,
                                                       apiAccount(),
                                                       0,
                                                       0,
                                                       userData);
                break;
            case Message::MESSAGE_ACCOUNT_GET_SUBSCRIPTION_REPLY:
                GetClient()->OnGetAccountSubscription(trackingNumber,
                                                      RESULT_TIMEOUT,
                                                      apiAccount(),
                                                      apiSubscription(),
                                                      userData);
                break;
            case Message::MESSAGE_SESSION_LOGIN_REPLY_v3:
                GetClient()->OnSessionLogin_v3(trackingNumber,
                                            RESULT_TIMEOUT,
                                            apiAccount(),
                                            apiSubscription(),
                                            apiSession(),
                                            userData);
                break;
            case Message::MESSAGE_SESSION_LOGIN_REPLY_v4:
                GetClient()->OnSessionLogin_v4(trackingNumber,
                                            RESULT_TIMEOUT,
                                            apiAccount(),
                                            apiSubscription(),
                                            apiSession(),
                                            UsageLimit(),
                                            userData);
                break;
            case Message::MESSAGE_SESSION_LOGIN_REPLY_v5:
                GetClient()->OnSessionLogin_v5(trackingNumber,
                                            RESULT_TIMEOUT,
                                            apiAccount(),
                                            apiSubscription(),
                                            apiSession(),
                                            UsageLimit(),
											Entitlement(),
                                            userData);
                break;
            case Message::MESSAGE_SESSION_LOGIN_REPLY:
                GetClient()->OnSessionLogin(trackingNumber,
                                            RESULT_TIMEOUT,
                                            apiAccount(),
                                            apiSubscription(),
                                            apiSession(),
											UsageLimit(),
											Entitlement(),
                                            userData);
                break;
			case Message::MESSAGE_SESSION_LOGIN_INTERNAL_REPLY:
                GetClient()->OnSessionLoginInternal(trackingNumber,
                                            RESULT_TIMEOUT,
                                            apiAccount(),
                                            apiSubscription(),
                                            apiSession(),
											UsageLimit(),
											Entitlement(),
                                            userData);
                break;
            case Message::MESSAGE_SESSION_CONSUME_REPLY_v2:
                GetClient()->OnSessionConsume_v2(trackingNumber,
                                              RESULT_TIMEOUT,
                                              apiAccount(),
                                              apiSubscription(),
                                              apiSession(),
                                              userData);
                break;
            case Message::MESSAGE_SESSION_CONSUME_REPLY_v3:
                GetClient()->OnSessionConsume_v3(trackingNumber,
                                              RESULT_TIMEOUT,
                                              apiAccount(),
                                              apiSubscription(),
                                              apiSession(),
  											  UsageLimit(),
                                              userData);
                break;
            case Message::MESSAGE_SESSION_CONSUME_REPLY:
                GetClient()->OnSessionConsume(trackingNumber,
                                              RESULT_TIMEOUT,
                                              apiAccount(),
                                              apiSubscription(),
                                              apiSession(),
											  UsageLimit(),
										  	  Entitlement(),
                                              userData);
                break;
            case Message::MESSAGE_SESSION_VALIDATE_REPLY_v2:
                GetClient()->OnSessionValidate_v2(trackingNumber,
                                               RESULT_TIMEOUT,
                                               apiAccount(),
                                               apiSubscription(),
                                               apiSession(),
                                               userData);
                break;
            case Message::MESSAGE_SESSION_VALIDATE_REPLY_v3:
                GetClient()->OnSessionValidate_v3(trackingNumber,
                                               RESULT_TIMEOUT,
                                               apiAccount(),
                                               apiSubscription(),
                                               apiSession(),
											   UsageLimit(),
                                               userData);
                break;
            case Message::MESSAGE_SESSION_VALIDATE_REPLY:
                GetClient()->OnSessionValidate(trackingNumber,
                                               RESULT_TIMEOUT,
                                               apiAccount(),
                                               apiSubscription(),
                                               apiSession(),
											   UsageLimit(),
											   Entitlement(),
                                               userData);
                break;
            case Message::MESSAGE_SESSION_VALIDATE_EX_REPLY:
                GetClient()->OnSessionValidateEx(trackingNumber,
                                               RESULT_TIMEOUT,
                                               apiAccount(),
                                               apiSubscription(),
                                               apiSession(),
                                               "",
                                               userData);
                break;
            case Message::MESSAGE_SUBSCRIPTION_VALIDATE_REPLY:
                GetClient()->OnSubscriptionValidate(trackingNumber,
                                                    RESULT_TIMEOUT,
                                                    apiSubscription(),
                                                    userData);
                break;
            case Message::MESSAGE_MEMBER_GET_INFORMATION_REPLY:
                GetClient()->OnGetMemberInformation(trackingNumber,
                                                    RESULT_TIMEOUT,
                                                    apiAccount(),
                                                    "",
                                                    "",
                                                    "",
                                                    "",
                                                    userData);
				break;
			case Message::MESSAGE_MEMBER_GET_INFORMATION_REPLY_v2:
				GetClient()->OnGetMemberInformation_v2(trackingNumber,
													RESULT_TIMEOUT,
													apiAccount(),
													"",
													"",
													"",
													"",
													"",
													"",
													"",
													userData);
				break;
            case Message::MESSAGE_REQ_GET_SESSIONS_REPLY:
                GetClient()->OnGetSessions(trackingNumber,
                                                    RESULT_TIMEOUT,
                                                    0,
                                                    0,
                                                    0,
                                                    0,
                                                    0,
													0,
                                                    userData);
                break;
			
            case Message::MESSAGE_FEATURE_GET_REPLY:
                GetClient()->OnGetFeatures(trackingNumber,
                                             RESULT_TIMEOUT,
											 0,
											 0,
											 userData);
                break;
            case Message::MESSAGE_FEATURE_GRANT_REPLY:
                GetClient()->OnGrantFeature(trackingNumber,
                                             RESULT_TIMEOUT,
											 userData);
                break;
            case Message::MESSAGE_FEATURE_REVOKE_REPLY:
                GetClient()->OnRevokeFeature(trackingNumber,
                                             RESULT_TIMEOUT,
											 userData);
                break;
            case Message::MESSAGE_FEATURE_MODIFY_REPLY:
                GetClient()->OnModifyFeature(trackingNumber,
                                             RESULT_TIMEOUT,
											 userData);
                break;
            case Message::MESSAGE_FEATURE_ENUMERATE_REPLY:
                GetClient()->OnEnumerateFeatures(trackingNumber,
                                             RESULT_TIMEOUT,
											 0,0,
											 userData);
                break;
			case Message::MESSAGE_SESSION_START_PLAY_REPLY:
                GetClient()->OnSessionStartPlay(trackingNumber,
                                             RESULT_TIMEOUT,
											 userData);
				break;
			case Message::MESSAGE_SESSION_STOP_PLAY_REPLY:
                GetClient()->OnSessionStopPlay(trackingNumber,
                                             RESULT_TIMEOUT,
											 userData);
                break;
            case Message::MESSAGE_KICK_REPLY:
                GetClient()->OnSessionKick(trackingNumber,
                                            RESULT_TIMEOUT,
                                            userData);
				break;
			case Message::MESSAGE_FEATURE_GRANT_REPLY_v2:
				GetClient()->OnGrantFeatureV2(trackingNumber,
					RESULT_TIMEOUT,
					userData);
				break;
			case Message::MESSAGE_MODIFY_FEATURE_REPLY_v2:
				GetClient()->OnModifyFeature_v2(trackingNumber,
												RESULT_TIMEOUT,
												Feature(),
												userData);
				break;

			case Message::MESSAGE_GRANT_FEATURE_BY_STATION_ID_REPLY:
				GetClient()->OnGrantFeatureByStationID(trackingNumber,
					RESULT_TIMEOUT,
					userData);
				break;
            default:
                break;
        }
    }

}

