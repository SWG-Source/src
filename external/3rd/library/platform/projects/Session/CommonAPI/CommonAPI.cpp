#include <memory.h>
#include <string.h>
#include "CommonAPI.h"


const unsigned MAX_ACCOUNT_NAME_LENGTH = 15;
const unsigned MAX_PASSWORD_LENGTH = 15;
const unsigned MAX_NAMESPACE_LENGTH = 64;
const unsigned SESSION_TICKET_LENGTH = 16;
const unsigned SESSION_SERVER_NAME_LENGTH = 40;
const unsigned SESSION_CHARACTER_NAME_LENGTH = 128;
const unsigned MAX_CLIENT_IP_LENGTH = 16;
const unsigned MAX_PLAN_DURATION_LENGTH = 64;
const unsigned MAX_GAME_CODE_LENGTH = 64;

const char * _defaultNamespace = "STATION";

////////////////////////////////////////////////////////////////////////////////

apiAccount::apiAccount() :
    mName(),
    mId(0),
    mStatus(ACCOUNT_STATUS_NULL)
{
    mName[0] = 0;
}

apiAccount::apiAccount(const apiAccount & copy) :
    mName(),
    mId(copy.mId),
    mStatus(copy.mStatus)
{
    memcpy(mName, copy.mName, apiAccountNameWidth);
    mName[apiAccountNameWidth-1] = 0;
}

apiAccount::apiAccount(const apiAccount_v1 & copy) :
    mName(),
    mId(copy.id),
    mStatus(copy.status)
{
    strncpy(mName, copy.name, apiAccountNameWidth);
    mName[apiAccountNameWidth-1] = 0;
}

apiAccount::~apiAccount()
{
}

apiAccount & apiAccount::operator=(const apiAccount & rhs)
{
    if (this != &rhs)
    {
        memcpy(mName, rhs.mName, apiAccountNameWidth);
        mName[apiAccountNameWidth-1] = 0;
        mId = rhs.mId;
        mStatus = rhs.mStatus;
    }

    return *this;
}

void apiAccount::SetName(const char * name)
{
    strncpy(mName, name, apiAccountNameWidth-1);
    mName[apiAccountNameWidth-1] = 0;
}


////////////////////////////////////////////////////////////////////////////////


apiSubscription::apiSubscription() :
    mGamecode(GAMECODE_NULL),
    mProviderId(PROVIDER_NULL),
    mStatus(SUBSCRIPTION_STATUS_NULL),
    mSubscriptionFeatures(0),
    mGameFeatures(0),
    mDurationSeconds(0),
    mParentalLimitSeconds(0)
{
}

apiSubscription::apiSubscription(const apiSubscription & copy) :
    mGamecode(copy.mGamecode),
    mProviderId(copy.mProviderId),
    mStatus(copy.mStatus),
    mSubscriptionFeatures(copy.mSubscriptionFeatures),
    mGameFeatures(copy.mGameFeatures),
    mDurationSeconds(copy.mDurationSeconds),
    mParentalLimitSeconds(copy.mParentalLimitSeconds)
{
}

apiSubscription::apiSubscription(const apiSubscription_v1 & copy) :
    mGamecode(copy.gamecode),
    mProviderId(PROVIDER_SOE),
    mStatus(copy.status),
    mSubscriptionFeatures(copy.subscriptionFeatures),
    mGameFeatures(copy.gameFeatures),
    mDurationSeconds(copy.durationSeconds),
    mParentalLimitSeconds(copy.parentalLimitSeconds)
{
}

apiSubscription::~apiSubscription()
{
}

apiSubscription & apiSubscription::operator=(const apiSubscription & rhs)
{
    if (this != &rhs)
    {
        mGamecode = rhs.mGamecode;
        mProviderId = rhs.mProviderId;
        mStatus = rhs.mStatus;
        mSubscriptionFeatures = rhs.mSubscriptionFeatures;
        mGameFeatures = rhs.mGameFeatures;
        mDurationSeconds = rhs.mDurationSeconds;
        mParentalLimitSeconds = rhs.mParentalLimitSeconds;
    }

    return *this;
}


////////////////////////////////////////////////////////////////////////////////


apiSession::apiSession() :
    mType(SESSION_TYPE_NULL),
    mProviderId(PROVIDER_NULL),
    mId(),
    mClientIP(0),
    mDurationSeconds(0),
    mParentalLimitSeconds(0),
    mTimeoutMinutes(0),
    mIsSecure(false)
{
    mId[0] = 0;
}

apiSession::apiSession(const apiSession & copy) :
    mType(copy.mType),
    mProviderId(copy.mProviderId),
    mId(),
    mClientIP(copy.mClientIP),
    mDurationSeconds(copy.mDurationSeconds),
    mParentalLimitSeconds(copy.mParentalLimitSeconds),
    mTimeoutMinutes(copy.mTimeoutMinutes),
    mIsSecure(copy.mIsSecure)
{
    memcpy(mId, copy.mId, apiSessionIdWidth);
    mId[apiSessionIdWidth-1] = 0;
}

apiSession::apiSession(const apiSession_v1 & copy) :
    mType(copy.type),
    mProviderId(PROVIDER_SOE),
    mId(),
    mClientIP(copy.clientIP),
    mDurationSeconds(copy.durationSeconds),
    mParentalLimitSeconds(copy.parentalLimitSeconds),
    mTimeoutMinutes(copy.timeoutMinutes),
    mIsSecure(copy.isSecure)
{
    strncpy(mId, copy.id, apiSessionIdWidth);
    mId[apiSessionIdWidth-1] = 0;
}

apiSession::~apiSession()
{
}

apiSession & apiSession::operator=(const apiSession & rhs)
{
    if (this != &rhs)
    {
        mType = rhs.mType;
        mProviderId = rhs.mProviderId;
        mClientIP = rhs.mClientIP;
        mDurationSeconds = rhs.mDurationSeconds;
        mParentalLimitSeconds = rhs.mParentalLimitSeconds;
        mTimeoutMinutes = rhs.mTimeoutMinutes;
        mIsSecure = rhs.mIsSecure;

        memcpy(mId, rhs.mId, apiSessionIdWidth);
        mId[apiSessionIdWidth-1] = 0;
    }

    return *this;
}

void apiSession::SetId(const char * id)
{
    strncpy(mId, id, apiSessionIdWidth-1);
    mId[apiSessionIdWidth-1] = 0;
}

////////////////////////////////////////////////////////////////////////////////


apiAccount_v1::apiAccount_v1()
{ 
    memset(this,0,sizeof(apiAccount_v1)); 
}

apiAccount_v1::apiAccount_v1(const apiAccount_v1 & copy)
{ 
    memcpy(this,&copy,sizeof(apiAccount_v1)); 
}

apiAccount_v1::apiAccount_v1(const apiAccount & copy) :
    name(),
    id(copy.GetId()),
    status(copy.GetStatus())
{ 
    strncpy(name, copy.GetName(), apiAccountNameWidth_v1-1);
    name[apiAccountNameWidth_v1-1] = 0;
}


////////////////////////////////////////////////////////////////////////////////


apiSubscription_v1::apiSubscription_v1() 
{ 
    memset(this,0,sizeof(apiSubscription_v1)); 
}

apiSubscription_v1::apiSubscription_v1(const apiSubscription_v1 & copy) 
{ 
    memcpy(this,&copy,sizeof(apiSubscription_v1)); 
}

apiSubscription_v1::apiSubscription_v1(const apiSubscription & copy) :
    gamecode(copy.GetGamecode()),
    status(copy.GetStatus()),
    subscriptionFeatures(copy.GetSubscriptionFeatures()),
    gameFeatures(copy.GetGameFeatures()),
    durationSeconds(copy.GetDurationSeconds()),
    parentalLimitSeconds(copy.GetParentalLimitSeconds())
{ 
}

/********************* Plan *********************************/
apiPlan::apiPlan():
	m_planID(0),
	m_paymentType(0),
	m_attributes(0),
	m_defaultStatus(0),
	m_reqGameFeatures(0),
	m_grantedGameFeatures(0),
	m_reqSubFeatures(0),
	m_grantedSubFeatures(0)
{

	m_description[0] = 0;
	m_sku[0] = 0;
	m_duration[0] = 0;
	m_gameCode[0] = 0;
	m_billFeatureSkuExt[0] = 0;
}

apiPlan::apiPlan(const apiPlan & copy): 
	m_planID(copy.m_planID),
	m_paymentType(copy.m_paymentType),
	m_attributes(copy.m_attributes),
	m_defaultStatus(copy.m_defaultStatus),
	m_reqGameFeatures(copy.m_reqGameFeatures),
	m_grantedGameFeatures(copy.m_grantedGameFeatures),
	m_reqSubFeatures(copy.m_reqSubFeatures),
	m_grantedSubFeatures(copy.m_grantedSubFeatures)
{
	memcpy(m_description, copy.m_description, planDescWidth);
	m_description[planDescWidth-1] = 0;

	memcpy(m_sku, copy.m_sku, planSkuWidth);
	m_sku[planSkuWidth-1] = 0;

	memcpy(m_duration, copy.m_duration, planDurationWidth);
	m_duration[planDurationWidth-1] = 0;

	memcpy(m_gameCode, copy.m_gameCode, gameCodeWidth);
	m_gameCode[gameCodeWidth-1] = 0;

	memcpy(m_billFeatureSkuExt, copy.m_billFeatureSkuExt, billFeatureSkuExtWidth);
	m_billFeatureSkuExt[billFeatureSkuExtWidth-1] = 0;
}

apiPlan::apiPlan(const apiPlan_v1 & copy):
	m_planID(copy.planID),
	m_paymentType(copy.paymentType),
	m_attributes(copy.attributes),
	m_defaultStatus(copy.defaultStatus),
	m_reqGameFeatures(copy.reqGameFeatures),
	m_grantedGameFeatures(copy.grantedGameFeatures),
	m_reqSubFeatures(copy.reqSubFeatures),
	m_grantedSubFeatures(copy.grantedSubFeatures)
{
	strncpy(m_description, copy.description, planDescWidth);
	m_description[planDescWidth-1] = 0;
	
	strncpy(m_sku, copy.sku, planSkuWidth);
	m_sku[planSkuWidth-1] = 0;

	strncpy(m_duration, copy.duration, planDurationWidth);
	m_duration[planDurationWidth-1] = 0;

	strncpy(m_gameCode, copy.gameCode, gameCodeWidth);
	m_gameCode[gameCodeWidth-1] = 0;
	
	strncpy(m_billFeatureSkuExt, copy.billFeatureSkuExt, billFeatureSkuExtWidth);
	m_billFeatureSkuExt[billFeatureSkuExtWidth-1] = 0;
}

apiPlan::~apiPlan()
{

}

apiPlan & apiPlan::operator=(const apiPlan & rhs)
{
	if (this != &rhs)
	{
		m_planID = rhs.m_planID;
		m_paymentType = rhs.m_paymentType;
		m_attributes = rhs.m_attributes;
		m_defaultStatus = rhs.m_defaultStatus;
		m_reqGameFeatures = rhs.m_reqGameFeatures;
		m_grantedGameFeatures = rhs.m_grantedGameFeatures;
		m_reqSubFeatures = rhs.m_reqSubFeatures;
		m_grantedSubFeatures = rhs.m_grantedSubFeatures;

		memcpy(m_description, rhs.m_description, planDescWidth);
		m_description[planDescWidth-1] = 0;

		memcpy(m_sku, rhs.m_sku, planSkuWidth);
		m_sku[planSkuWidth-1] = 0;

		memcpy(m_duration, rhs.m_duration, planDurationWidth);
		m_duration[planDurationWidth-1] = 0;

		memcpy(m_gameCode, rhs.m_gameCode, gameCodeWidth);
		m_gameCode[gameCodeWidth-1] = 0;

		memcpy(m_billFeatureSkuExt, rhs.m_billFeatureSkuExt, billFeatureSkuExtWidth);
		m_billFeatureSkuExt[billFeatureSkuExtWidth-1] = 0;
	}

	return *this;
}

void apiPlan::SetDescription(const char * description) 
{ 
	memcpy(m_description, description, planDescWidth); 
	m_description[planDescWidth-1] = 0;
}

void apiPlan::SetSku(const char * sku) 
{ 
	memcpy(m_sku, sku, planSkuWidth); 
	m_sku[planSkuWidth-1] = 0; 
}

void apiPlan::SetDuration(const char * duration) 
{ 
	memcpy(m_duration, duration, planDurationWidth); 
	m_duration[planDurationWidth-1] = 0; 
}

void apiPlan::SetGameCode(const char * gameCode)
{ 
	memcpy(m_gameCode, gameCode, gameCodeWidth); 
	m_gameCode[gameCodeWidth-1] = 0; 
}

void apiPlan::SetBillFeatureSkuExt(const char * billFeatureSkuExt) 
{ 
	memcpy(m_billFeatureSkuExt, billFeatureSkuExt, billFeatureSkuExtWidth); 
	m_billFeatureSkuExt[billFeatureSkuExtWidth-1] = 0; 
}


///////////////// V1 ////////////////

apiPlan_v1::apiPlan_v1()
{
	memset(this,0,sizeof(apiPlan_v1)); 
}

apiPlan_v1::apiPlan_v1(const apiPlan_v1 & copy)
{
	memcpy(this,&copy,sizeof(apiPlan_v1)); 
}

apiPlan_v1::apiPlan_v1(const apiPlan & copy): 
	planID(copy.GetPlanID()),
	paymentType(copy.GetPaymentType()),
	attributes(copy.GetAttributes()),
	defaultStatus(copy.GetDefaultStatus()),
	reqGameFeatures(copy.GetReqGameFeatures()),
	grantedGameFeatures(copy.GetGrantedGameFeatures()),
	reqSubFeatures(copy.GetReqSubFeatures()),
	grantedSubFeatures(copy.GetGrantedSubFeatures())
{
	strncpy(description, copy.GetDescription(), planDescWidth);
	description[planDescWidth-1] = 0;

	strncpy(sku, copy.GetSku(), planSkuWidth);
	sku[planSkuWidth-1] = 0;

	strncpy(duration, copy.GetDuration(), planDurationWidth);
	duration[planDurationWidth-1] = 0;

	strncpy(gameCode, copy.GetGameCode(), gameCodeWidth);
	gameCode[gameCodeWidth-1] = 0;

	strncpy(billFeatureSkuExt, copy.GetBillFeatureSkuExt(), billFeatureSkuExtWidth);
	billFeatureSkuExt[billFeatureSkuExtWidth-1] = 0;
}

////////////////////////////////////////////////////////////////////////////////


apiSession_v1::apiSession_v1() 
{ 
    memset(this,0,sizeof(apiSession_v1)); 
}

apiSession_v1::apiSession_v1(const apiSession_v1 & copy) 
{ 
    memcpy(this,&copy,sizeof(apiSession_v1)); 
}

apiSession_v1::apiSession_v1(const apiSession & copy) :
    type(copy.GetType()),
    id(),
    clientIP(copy.GetClientIP()),
    durationSeconds(copy.GetDurationSeconds()),
    parentalLimitSeconds(copy.GetParentalLimitSeconds()),
    timeoutMinutes(copy.GetTimeoutMinutes()),
    isSecure(copy.GetIsSecure())
{ 
    strncpy(id, copy.GetId(), apiSessionIdWidth_v1-1);
    id[apiSessionIdWidth_v1-1] = 0;
}
SessionTypeDesc::SessionTypeDesc(const SessionTypeDesc & copy)
{
	memcpy( this, &copy, sizeof(SessionTypeDesc));
}


////////////////////////////////////////////////////////////////////////////////

