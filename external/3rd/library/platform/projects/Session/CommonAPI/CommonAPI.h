#ifndef COMMON_API_H
#define COMMON_API_H


class apiCore;
class apiClient
{
    public:
        apiClient();
        virtual ~apiClient();

        void Connect(const char * address);   
        virtual void Process();   

        ////////////////////////////////////////
        //  State Callbacks
        virtual void OnConnectionOpened(const char * address, unsigned connectionCount);
        virtual void OnConnectionClosed(const char * address, unsigned connectionCount);
        virtual void OnConnectionFailed(const char * address, unsigned connectionCount);

    protected:
        apiCore * mClientCore;
};

#define apiSessionType	unsigned
#define apiGamecode		unsigned

typedef unsigned        apiAccountId;
typedef unsigned        apiTrackingNumber;
typedef unsigned        apiProductFeatures;
typedef unsigned        apiIP;
typedef unsigned        apiResult;
typedef unsigned        apiAccountStatus;
typedef unsigned        apiSubscriptionStatus;
typedef unsigned        apiProviderId;
typedef unsigned        apiKickReason;
typedef unsigned        apiKickReply;

#ifdef API_NAMESPACE
namespace API_NAMESPACE
{
#endif


enum
{
    RESULT_SUCCESS                  = 0,            //  operation successful
    RESULT_CANCELLED,                               //  operation cancelled by user
    RESULT_TIMEOUT,                                 //  operation could not be serviced
    RESULT_FUNCTION_DEPRICATED,                     //  operation is not supported anymore
    RESULT_FUNCTION_NOT_SUPPORTED,                  //  operation is not supported yet
    
    //  Login related result codes
    RESULT_INVALID_NAME_OR_PASSWORD = 1000,
    RESULT_INVALID_ACCOUNT_ID,
    RESULT_INVALID_SESSION,
    RESULT_INVALID_PARENT_SESSION,
    RESULT_INVALID_SESSION_TYPE,
    RESULT_INVALID_GAMECODE,
    RESULT_REQUIRES_VALID_SUBSCRIPTION,
    RESULT_REQUIRES_ACTIVE_ACCOUNT,
    RESULT_REQUIRES_CLIENT_LOGOUT,
    RESULT_SESSION_ALREADY_CONSUMED,
    RESULT_REQUIRES_SECURE_ID_NEXT_CODE,
    RESULT_REQUIRES_SECURE_ID_NEW_PIN,
    RESULT_SECURE_ID_PIN_ACCEPTED,
    RESULT_SECURE_ID_PIN_REJECTED,
    RESULT_USAGE_LIMIT_DENIED_LOGIN,
    RESULT_INVALID_FEATURE,
	RESULT_INVALID_SERVER_NAME,
	RESULT_INVALID_CHARACTER_NAME,
	RESULT_INVALID_NAMESPACE,
    RESULT_INVALID_CLIENT_IP,
    RESULT_USER_LOCKOUT,
    RESULT_INVALID_KICK_REASON,

    //  Chat related result codes
    RESULT_INVALID_AVATAR           = 1032,
    RESULT_INVALID_CHATROOM,
    RESULT_INVALID_MESSAGE,
    RESULT_INVALID_FANCLUB,
    RESULT_INVALID_ROOM_BANNER,
    RESULT_REQUIRES_FANCLUB_MEMBERSHIP,
    RESULT_REQUIRES_ONLINE_AVATAR,
    RESULT_REQUIRES_MODERATOR_PRIVILEGES,
    RESULT_REQUIRES_VALID_MODERATION_STATE,
    RESULT_BANNED_FROM_ROOM,
    RESULT_FULL_ROOM,
    RESULT_INVALID_ROOM_PATH,
    RESULT_INVALID_ROOM_ATTRIBUTES,
	RESULT_INVALID_MAX_ROOM_SIZE,
	RESULT_INVALID_SUBJECT_SIZE,

	RESULT_INVALID_PLAN_DURATION_SIZE = 2000,
	RESULT_INVALID_GAME_CODE_SIZE,
	RESULT_INVALID_PLAN_ID,
	RESULT_INVALID_FEATURE_MATCH,
	RESULT_INVALID_SUBSCRIPTION
};

enum 
{
    ACCOUNT_STATUS_NULL,                            //  Invalid account status
    ACCOUNT_STATUS_ACTIVE,
    ACCOUNT_STATUS_CLOSED,
    //  Add new account status codes here
    ACCOUNT_STATUS_END
};

enum 
{
    SESSION_TYPE_NULL,                              //  Invalid session type
    SESSION_TYPE_LAUNCH_PAD,
    SESSION_TYPE_STATION_UNSECURE,
    SESSION_TYPE_STATION_SECURE,
    SESSION_TYPE_TANARUS,
    SESSION_TYPE_INFANTRY,
    SESSION_TYPE_COSMIC_RIFT,
    SESSION_TYPE_EVERQUEST,
    SESSION_TYPE_EVERQUEST_2,
    SESSION_TYPE_STARWARS,
    SESSION_TYPE_PLANETSIDE,
    SESSION_TYPE_EVERQUEST_ONLINE_ADVENTURES_BETA,
    SESSION_TYPE_EVERQUEST_ONLINE_ADVENTURES,
    SESSION_TYPE_EVERQUEST_INSTANT_MESSENGER,
    SESSION_TYPE_REALM_SERVER,
    SESSION_TYPE_EVERQUEST_MACINTOSH,
	SESSION_TYPE_MATRIX_ONLINE,
	SESSION_TYPE_HARRY_POTTER,
	SESSION_TYPE_NEO_PETS,
	SESSION_TYPE_GOODLIFE,
	SESSION_TYPE_EQ2_GUILDS,
	SESSION_TYPE_SWG_JP_BETA,
	SESSION_TYPE_MARVEL,
	SESSION_TYPE_EQ2_JAPAN,
	SESSION_TYPE_EQ2_TAIWAN,
	SESSION_TYPE_EQ2_CHINA,
	SESSION_TYPE_EQ2_KOREA,
	SESSION_TYPE_VGD,
	SESSION_TYPE_PIRATE,
	SESSION_TYPE_STAR_CHAMBER,
	SESSION_TYPE_STARGATE,
	SESSION_TYPE_DCU_ONLINE,
	SESSION_TYPE_NORRATH_CSG,
    //  Add new session type codes here
    SESSION_TYPE_END
};

enum 
{
    GAMECODE_NULL,                                 //  Invalid gamecode
    GAMECODE_STATION_PASS,
    GAMECODE_EVERQUEST,
    GAMECODE_EVERQUEST_2,
    GAMECODE_STARWARS,
    GAMECODE_PLANETSIDE,
    GAMECODE_EVERQUEST_ONLINE_ADVENTURES_BETA,
    GAMECODE_EVERQUEST_ONLINE_ADVENTURES,
    GAMECODE_EVERQUEST_INSTANT_MESSENGER,
    GAMECODE_EVERQUEST_MACINTOSH,
    GAMECODE_MATRIX_ONLINE,
	GAMECODE_HARRY_POTTER,
	GAMECODE_NEO_PETS,
	GAMECODE_GOODLIFE,
	GAMECODE_SWG_JP_BETA,
	GAMECODE_MARVEL,
	GAMECODE_EQ2_JAPAN,
	GAMECODE_EQ2_TAIWAN,
	GAMECODE_EQ2_CHINA,
	GAMECODE_EQ2_KOREA,
	GAMECODE_VGD,
	GAMECODE_PIRATE,
	GAMECODE_STAR_CHAMBER,
	GAMECODE_STARGATE,
	GAMECODE_DCU_ONLINE,
	GAMECODE_NORRATH_CSG,
   //  Add new gamecodes here
    GAMECODE_END
};

enum 
{
    SUBSCRIPTION_STATUS_NULL,                       //  Invalid subscription status
    SUBSCRIPTION_STATUS_NO_ACCOUNT,
    SUBSCRIPTION_STATUS_ACTIVE,
    SUBSCRIPTION_STATUS_PENDING,
    SUBSCRIPTION_STATUS_CLOSED,
    SUBSCRIPTION_STATUS_TRIAL_ACTIVE,
    SUBSCRIPTION_STATUS_TRIAL_EXPIRED,
    SUBSCRIPTION_STATUS_SUSPENDED,
    SUBSCRIPTION_STATUS_BANNED,
    SUBSCRIPTION_STATUS_RENTAL_ACTIVE,
    SUBSCRIPTION_STATUS_RENTAL_CLOSED,
	SUBSCRIPTION_STATUS_ACTIVE_COMBO,
	SUBSCRIPTION_STATUS_BANNED_CHARGEBACK,
    //  Add new subscription status codes here
    SUBSCRIPTION_STATUS_END
};

enum 
{
    KICK_REASON_NULL,                               //  Invalid kick reason
    KICK_REASON_USER_REQUEST,
    KICK_REASON_ADMIN_REQUEST,
    //  Add new kick reason codes here
    KICK_REASON_END
};

enum 
{
    KICK_REPLY_NULL,                         
    KICK_REPLY_ALLOW,                         
    KICK_REPLY_DENY,
    KICK_REPLY_UNKNOWN_SESSION,
    //  Add new kick reply codes here
    KICK_REPLY_END                         
};

enum 
{
    PROVIDER_NULL,                         
    PROVIDER_SOE,
    PROVIDER_UBISOFT,
    //  Add new provider id codes here
    PROVIDER_END
};


#ifdef API_NAMESPACE
}
#endif


extern const unsigned MAX_ACCOUNT_NAME_LENGTH;  //  15: ASCII characters (alpha-numeric only)
extern const unsigned MAX_PASSWORD_LENGTH;      //  15: ASCII characters (alpha-numeric only)
extern const unsigned MAX_NAMESPACE_LENGTH;		//  64: ASCII characters (alpha-numeric only)
extern const unsigned SESSION_TICKET_LENGTH;    //  16: ASCII characters (alpha numeric + '.' + '?')
extern const unsigned SESSION_SERVER_NAME_LENGTH;
extern const unsigned SESSION_CHARACTER_NAME_LENGTH;
extern const unsigned MAX_CLIENT_IP_LENGTH;
extern const unsigned MAX_PLAN_DURATION_LENGTH;
extern const unsigned MAX_GAME_CODE_LENGTH;

extern const char * _defaultNamespace;

static const int apiAccountNameWidth = 16;
struct apiAccount_v1;
class apiAccount
{
    public:
        apiAccount();
        apiAccount(const apiAccount & copy);
        apiAccount(const apiAccount_v1 & copy);
        ~apiAccount();

        apiAccount & operator=(const apiAccount & rhs);

        const char *          GetName() const			{ return mName; }
        apiAccountId         GetId() const					{ return mId; }
        apiAccountStatus	GetStatus() const			{ return mStatus; }

        void                    SetName(const char * name);
        void                    SetId(apiAccountId id)											{ mId = id; }
        void                    SetStatus(apiAccountStatus status)                      { mStatus = status; }

    private:
        char							mName[apiAccountNameWidth];
        apiAccountId				mId;
        apiAccountStatus      mStatus;
};

struct apiSubscription_v1;
class apiSubscription
{
    public:
        apiSubscription();
        apiSubscription(const apiSubscription & copy);
        apiSubscription(const apiSubscription_v1 & copy);
        ~apiSubscription();

        apiSubscription & operator=(const apiSubscription & rhs);

        apiGamecode             GetGamecode() const                                     { return mGamecode; }
        apiProviderId           GetProviderId() const                                   { return mProviderId; }
        apiSubscriptionStatus   GetStatus() const                                       { return mStatus; }
        apiProductFeatures      GetSubscriptionFeatures() const                         { return mSubscriptionFeatures; }
        apiProductFeatures      GetGameFeatures() const                                 { return mGameFeatures; }
        unsigned                GetDurationSeconds() const                              { return mDurationSeconds; }
        unsigned                GetParentalLimitSeconds() const                         { return mParentalLimitSeconds; }

        void                    SetGamecode(apiGamecode gamecode)                       { mGamecode = gamecode; }
        void                    SetProviderId(apiProviderId provider)                   { mProviderId = provider; }
        void                    SetStatus(apiSubscriptionStatus status)                 { mStatus = status; }
        void                    SetSubscriptionFeatures(apiProductFeatures features)    { mSubscriptionFeatures = features; }
        void                    SetGameFeatures(apiProductFeatures features)            { mGameFeatures = features; }
        void                    SetDurationSeconds(unsigned duration)                   { mDurationSeconds = duration; }
        void                    SetParentalLimitSeconds(unsigned parentalLimit)         { mParentalLimitSeconds = parentalLimit; }

    private:
        apiGamecode             mGamecode;
        apiProviderId           mProviderId;
        apiSubscriptionStatus   mStatus;
        apiProductFeatures      mSubscriptionFeatures;
        apiProductFeatures      mGameFeatures;
        unsigned                mDurationSeconds;
        unsigned                mParentalLimitSeconds;
};

static const int apiSessionIdWidth = 17;
struct apiSession_v1;
class apiSession
{
    public:
        apiSession();
        apiSession(const apiSession & copy);
        apiSession(const apiSession_v1 & copy);
        ~apiSession();

        apiSession & operator=(const apiSession & rhs);

        apiSessionType          GetType() const                                         { return mType; }
        apiProviderId           GetProviderId() const                                   { return mProviderId; }
        const char *            GetId() const                                           { return mId; }
        apiIP                   GetClientIP() const                                     { return mClientIP; }
        unsigned                GetDurationSeconds() const                              { return mDurationSeconds; }
        unsigned                GetParentalLimitSeconds() const                         { return mParentalLimitSeconds; }
        unsigned char           GetTimeoutMinutes() const                               { return mTimeoutMinutes; }
        bool                    GetIsSecure() const                                     { return mIsSecure; }

        void                    SetType(apiSessionType type)                            { mType = type; }
        void                    SetProviderId(apiProviderId provider)                   { mProviderId = provider; }
        void                    SetId(const char * id);
        void                    SetClientIP(apiIP clientIP)                             { mClientIP = clientIP; }
        void                    SetDurationSeconds(unsigned duration)                   { mDurationSeconds = duration; }
        void                    SetParentalLimitSeconds(unsigned parentalLimit)         { mParentalLimitSeconds = parentalLimit; }
        void                    SetTimeoutMinutes(unsigned char timeout)                { mTimeoutMinutes = timeout; }
        void                    SetIsSecure(bool isSecure)                              { mIsSecure = isSecure; }

    private:
        apiSessionType          mType;
        apiProviderId           mProviderId;
        char                    mId[apiSessionIdWidth];
        apiIP                   mClientIP;
        unsigned                mDurationSeconds;
        unsigned                mParentalLimitSeconds;
        unsigned char           mTimeoutMinutes;
        bool                    mIsSecure;
};


static const int apiAccountNameWidth_v1 = 32;
struct apiAccount_v1
{
    apiAccount_v1();
    apiAccount_v1(const apiAccount_v1 & copy);
    apiAccount_v1(const apiAccount & copy);

    char                    name[apiAccountNameWidth_v1];
    apiAccountId            id;
    apiAccountStatus        status;
};

struct apiSubscription_v1
{
    apiSubscription_v1();
    apiSubscription_v1(const apiSubscription_v1 & copy);
    apiSubscription_v1(const apiSubscription & copy);

    apiGamecode             gamecode;
    apiSubscriptionStatus   status;
    apiProductFeatures      subscriptionFeatures;
    apiProductFeatures      gameFeatures;
    unsigned                durationSeconds;
    unsigned                parentalLimitSeconds;
};

static const int apiSessionIdWidth_v1 = 32;
struct apiSession_v1
{
    apiSession_v1();
    apiSession_v1(const apiSession_v1 & copy);
    apiSession_v1(const apiSession & copy);

    apiSessionType          type;
    char                    id[apiSessionIdWidth_v1];
    apiIP                   clientIP;
    unsigned                durationSeconds;
    unsigned                parentalLimitSeconds;
    unsigned char           timeoutMinutes;
    bool                    isSecure;
};

static const int gameCodeWidth = 26;
static const int sessionTypeDescriptionWidth = 257;
struct SessionTypeDesc
{
	SessionTypeDesc() : type(0), gameCodeID(0), independent(false), exclusive(false) {gameCode[0]= 0; description[0] = 0;}
	SessionTypeDesc(const SessionTypeDesc & copy);
	apiSessionType			type;
	char					gameCode[gameCodeWidth];
	apiGamecode				gameCodeID;
	char					description[sessionTypeDescriptionWidth];
	bool					independent;
	bool					exclusive;
};

static const int gameCodeDescriptionWidth = 256;
struct GameCodeDescription
{
	GameCodeDescription() : gameID(0), createDynamic(false) { code[0] = 0; description[0] = 0;}

	apiGamecode				gameID;
	char					code[gameCodeWidth];
	char					description[gameCodeDescriptionWidth];
	bool					createDynamic;
};

/*************** Plan **************************************/
struct apiPlan_v1;
static const int planDescWidth = 256;
static const int planSkuWidth = 256;
static const int planDurationWidth = 11;
static const int billFeatureSkuExtWidth = 101;

class apiPlan
{
public:
	apiPlan();
	apiPlan(const apiPlan & copy);
	apiPlan(const apiPlan_v1 & copy);
	~apiPlan();

	apiPlan & operator=(const apiPlan & rhs);

	unsigned		GetPlanID() const { return m_planID; }
	const char * 	GetDescription() const { return m_description; }
	const char *	GetSku() const { return m_sku; }
	unsigned		GetPaymentType() const { return m_paymentType; }
	const char * 	GetDuration() const { return m_duration; }
	unsigned		GetAttributes() const { return m_attributes; }
	unsigned		GetDefaultStatus() const { return m_defaultStatus; }
	const char * 	GetGameCode() const { return m_gameCode; }
	unsigned		GetReqGameFeatures() const { return m_reqGameFeatures; }
	unsigned		GetGrantedGameFeatures() const { return m_grantedGameFeatures; }
	unsigned		GetReqSubFeatures() const { return m_reqSubFeatures; }
	unsigned		GetGrantedSubFeatures() const { return m_grantedSubFeatures; }
	const char *	GetBillFeatureSkuExt() const { return m_billFeatureSkuExt; }

	void		SetPlanID(unsigned planID) { m_planID = planID; }
	void		SetDescription(const char * description);
	void		SetSku(const char * sku);
	void		SetPaymentType(unsigned	paymentType) { m_paymentType = paymentType; }
	void		SetDuration(const char * duration);
	void		SetAttributes(unsigned	attributes) { m_attributes = attributes; }
	void		SetDefaultStatus(unsigned defaultStatus) { m_defaultStatus = defaultStatus; }
	void		SetGameCode(const char * gameCode);
	void		SetReqGameFeatures(unsigned	reqGameFeatures) { m_reqGameFeatures = reqGameFeatures; }
	void		SetGrantedGameFeatures(unsigned	grantedGameFeatures) { m_grantedGameFeatures = grantedGameFeatures; }
	void		SetReqSubFeatures(unsigned	reqSubFeatures) { m_reqSubFeatures = reqSubFeatures; }
	void		SetGrantedSubFeatures(unsigned grantedSubFeatures) { m_grantedSubFeatures = grantedSubFeatures; }
	void		SetBillFeatureSkuExt(const char * billFeatureSkuExt);

private:

	unsigned	m_planID;
	char		m_description[planDescWidth];
	char		m_sku[planSkuWidth];
	unsigned	m_paymentType;
	char		m_duration[planDurationWidth];
	unsigned	m_attributes;
	unsigned	m_defaultStatus;
	char		m_gameCode[gameCodeWidth];
	unsigned	m_reqGameFeatures;
	unsigned	m_grantedGameFeatures;
	unsigned	m_reqSubFeatures;
	unsigned	m_grantedSubFeatures;
	char		m_billFeatureSkuExt[billFeatureSkuExtWidth];
};

struct apiPlan_v1
{
	apiPlan_v1();
	apiPlan_v1(const apiPlan_v1 & copy);
	apiPlan_v1(const apiPlan & copy);

	unsigned	planID;
	char		description[planDescWidth];
	char		sku[planSkuWidth];
	unsigned	paymentType;
	char		duration[planDurationWidth];
	unsigned	attributes;
	unsigned	defaultStatus;
	char		gameCode[gameCodeWidth];
	unsigned	reqGameFeatures;
	unsigned	grantedGameFeatures;
	unsigned	reqSubFeatures;
	unsigned	grantedSubFeatures;
	char		billFeatureSkuExt[billFeatureSkuExtWidth];
};

#endif

