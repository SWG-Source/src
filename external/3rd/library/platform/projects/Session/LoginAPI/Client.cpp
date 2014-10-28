#pragma warning (disable : 4786 )


#include "Client.h"
#include "ClientCore.h"


namespace LoginAPI
{

	Entitlement::Entitlement() :
        mTotalTime(0),
        mEntitledTime(0),
        mTotalTimeSinceLastLogin(0),
        mEntitledTimeSinceLastLogin(0),
        mReasonUnentitled(0)
	{
	}

	Feature::Feature() :
		mID(0),
		mData()
	{
	}

	bool Feature::SetParameter(const std::string & key, int value)
	{
		if (key.size() > 32)
			return false;

		std::string keyToken = key + "[";
		unsigned offset = mData.find(keyToken);
		if (offset == std::string::npos)
		{
			char token[256];
			sprintf(token, "%s[%d]", key.c_str(), value);
			if (!mData.empty())
				mData += ", ";
			mData += token;
		}
		else
		{
			char token[256];
			unsigned delimiter = mData.find("]", offset);
			sprintf(token, "%d", value);
			std::string newData = mData.substr(0,offset+keyToken.size()) + token + mData.substr(delimiter);
			mData = newData;
		}
		return true;
	}

	bool Feature::SetParameter(const std::string & key, const std::string & value)
	{
		if (key.size() > 32 || value.size() > 32)
			return false;

		std::string keyToken = key + "[";
		unsigned offset = mData.find(keyToken);
		if (offset == std::string::npos)
		{
			char token[256];
			sprintf(token, "%s[%s]", key.c_str(), value.c_str());
			if (!mData.empty())
				mData += ", ";
			mData += token;
		}
		else
		{
			unsigned delimiter = mData.find("]", offset);
			offset += keyToken.size();
			std::string newData = mData.substr(0,offset) + value + mData.substr(delimiter);
			mData = newData;
		}
		return true;
	}

	int Feature::GetParameter(const std::string & key) const
	{
		std::string keyToken = key + "[";
		unsigned offset = mData.find(keyToken);
		if (offset != std::string::npos)
		{
			offset += keyToken.size();
			return atoi(mData.c_str() + offset);
		}
		else
		{
			return 0;
		}
	}

	std::string & Feature::GetParameter(const std::string & key, std::string & value) const
	{
		std::string keyToken = key + "[";
		unsigned offset = mData.find(keyToken);
		if (offset != std::string::npos)
		{
			unsigned delimiter = mData.find("]", offset);
			offset += keyToken.size();
			value = mData.substr(offset,delimiter-offset);
		}
		else
		{
			value.clear();
		}
		return value;
	}

	//	For consumable promotions
	bool Feature::Consume()
	{
		int consumeCount = GetParameter("count");
		if (consumeCount == 0)
			return false;
		SetParameter("count", consumeCount-1);
		return true;
	}

	int Feature::GetConsumeCount() const
	{
		return GetParameter("count");
	}

	bool Feature::IsActive() const
	{
		return GetParameter("active") ? true : false;
	}

	void Feature::SetActive(bool isActive)
	{
		SetParameter("active", (int)isActive);
	}


	FeatureDescription::FeatureDescription() :
		mID(0),
		mDescription(),
		mDefaultData()
	{
	}
	
	UsageLimit::UsageLimit() :
		mType(0),
		mAllowance(0),
		mNextAllowance(0)
	{
	}


    const char * apiVersion      = "2004.07.20";

    Client::Client(const char * serverList, const char * description, unsigned maxConnections)
        : apiClient()
    {
	    mClientCore = new ClientCore(this, apiVersion, serverList, description, maxConnections);
    }

    Client::Client(const char ** serverList, unsigned serverCount, const char * description, unsigned maxConnections)
        : apiClient()
    {
	    mClientCore = new ClientCore(this, apiVersion, serverList, serverCount, description, maxConnections);
    }

    Client::~Client() 
    {
	    delete mClientCore;
    }

    void Client::Process()
    {
        this->apiClient::Process();
    }

    void Client::OnConnectionOpened(const char * address, unsigned connectionCount)
    {
    }

    void Client::OnConnectionClosed(const char * address, unsigned connectionCount)
    {
    }

    void Client::OnConnectionFailed(const char * address, unsigned connectionCount)
    {
    }

    apiTrackingNumber Client::SessionLogin(const char * name, const char * password, const apiSessionType sessionType, const apiIP clientIP, unsigned flags, const char * sessionNamespace, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionLogin(name, password, sessionType, clientIP, flags, sessionNamespace, userData) : 0;
    }

    apiTrackingNumber Client::SessionLogin(const char * parentSession, const apiSessionType sessionType, const apiIP clientIP, unsigned flags, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionLogin(parentSession, sessionType, clientIP, flags, userData) : 0;
    }

	apiTrackingNumber Client::SessionLoginInternal(const apiAccountId accountId, const apiSessionType sessionType, const apiIP clientIP, unsigned flags, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionLoginInternal(accountId, sessionType, clientIP, flags, userData) : 0;
    }
	
    apiTrackingNumber Client::SessionConsume(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionConsume(sessionID, sessionType, userData) : 0;
    }

    apiTrackingNumber Client::SessionValidate(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionValidate(sessionID, sessionType, userData) : 0;
    }

    void Client::SessionLogout(const char * sessionID)
    {
        if (mClientCore)
            static_cast<ClientCore *>(mClientCore)->SessionLogout(sessionID);
    }

    void Client::SessionTouch(const char * sessionID)
    {
        if (mClientCore)
            static_cast<ClientCore *>(mClientCore)->SessionTouch(sessionID);
    }

    apiTrackingNumber Client::GetSessions(const apiAccountId accountId, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->GetSessions(accountId, userData) : 0;
    }

    apiTrackingNumber Client::GetFeatures(const char * sessionID, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->GetFeatures(sessionID, userData) : 0;
    }

	apiTrackingNumber Client::GetFeatures(const apiAccountId accountId, const apiGamecode gamecode, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->GetFeatures(accountId, gamecode, userData) : 0;
    }

    apiTrackingNumber Client::GrantFeature(const char * sessionID, unsigned featureType, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->GrantFeature(sessionID, featureType, userData) : 0;
    }

    apiTrackingNumber Client::ModifyFeature(const char * sessionID, const Feature & feature, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->ModifyFeature(sessionID, feature, userData) : 0;
    }
    apiTrackingNumber Client::RevokeFeature(const char * sessionID, unsigned featureType, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->RevokeFeature(sessionID, featureType, userData) : 0;
    }

    apiTrackingNumber Client::EnumerateFeatures(apiGamecode gamecode, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->EnumerateFeatures(gamecode, userData) : 0;
    }

	apiTrackingNumber Client::SessionStartPlay(const char * sessionID, const char * serverName, const char * characterName, const char * gameData, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionStartPlay(sessionID, serverName, characterName, gameData, userData) : 0;
    }

	apiTrackingNumber Client::SessionStopPlay(const char * sessionID, const char * serverName, const char * characterName, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionStopPlay(sessionID, serverName, characterName, userData) : 0;
    }

	////////////////////////////////////////////////
	//	OLD CALLS
    apiTrackingNumber Client::GetAccountStatus(const char * name, const char * password, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->GetAccountStatus(name, password, userData) : 0;
    }

    apiTrackingNumber Client::GetAccountStatus(const apiAccountId accountId, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->GetAccountStatus(accountId, userData) : 0;
    }

    apiTrackingNumber Client::GetAccountSubscriptions(const char * name, const char * password, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->GetAccountSubscriptions(name, password, userData) : 0;
    }

    apiTrackingNumber Client::GetAccountSubscriptions(const char * sessionID, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->GetAccountSubscriptions(sessionID, userData) : 0;
    }

    apiTrackingNumber Client::GetAccountSubscriptions(const apiAccountId accountId, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->GetAccountSubscriptions(accountId, userData) : 0;
    }

    apiTrackingNumber Client::GetAccountSubscription(const char * name, const char * password, const apiGamecode gamecode, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->GetAccountSubscription(name, password, gamecode, userData) : 0;
    }

    apiTrackingNumber Client::GetAccountSubscription(const char * sessionID, const apiGamecode gamecode, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->GetAccountSubscription(sessionID, gamecode, userData) : 0;
    }
    
    apiTrackingNumber Client::GetAccountSubscription(const apiAccountId accountId, const apiGamecode gamecode, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->GetAccountSubscription(accountId, gamecode, userData) : 0;
    }
    
    apiTrackingNumber Client::SessionLogin_v3(const char * name, const char * password, const apiSessionType sessionType, const apiIP clientIP, const bool forceLogin, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionLogin_v3(name, password, sessionType, clientIP, forceLogin, userData) : 0;
    }

    apiTrackingNumber Client::SessionLogin_v3(const char * parentSession, const apiSessionType sessionType, const apiIP clientIP, const bool forceLogin, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionLogin_v3(parentSession, sessionType, clientIP, forceLogin, userData) : 0;
    }

    apiTrackingNumber Client::SessionLogin_v4(const char * name, const char * password, const apiSessionType sessionType, const apiIP clientIP, const bool forceLogin, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionLogin_v4(name, password, sessionType, clientIP, forceLogin, userData) : 0;
    }

    apiTrackingNumber Client::SessionLogin_v4(const char * parentSession, const apiSessionType sessionType, const apiIP clientIP, const bool forceLogin, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionLogin_v4(parentSession, sessionType, clientIP, forceLogin, userData) : 0;
    }

	apiTrackingNumber Client::SessionLogin_v5(const char * name, const char * password, const apiSessionType sessionType, const apiIP clientIP, unsigned flags, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionLogin_v5(name, password, sessionType, clientIP, flags, userData) : 0;
    }

    apiTrackingNumber Client::SessionLogin_v5(const char * parentSession, const apiSessionType sessionType, const apiIP clientIP, unsigned flags, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionLogin_v5(parentSession, sessionType, clientIP, flags, userData) : 0;
    }

    apiTrackingNumber Client::SessionConsume_v2(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionConsume_v2(sessionID, sessionType, userData) : 0;
    }

    apiTrackingNumber Client::SessionConsume_v3(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionConsume_v3(sessionID, sessionType, userData) : 0;
    }

    apiTrackingNumber Client::SessionValidate_v2(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionValidate_v2(sessionID, sessionType, userData) : 0;
    }

    apiTrackingNumber Client::SessionValidate_v3(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionValidate_v3(sessionID, sessionType, userData) : 0;
    }

    apiTrackingNumber Client::SessionValidateEx(const char * sessionID, const apiSessionType sessionType, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionValidateEx(sessionID, sessionType, userData) : 0;
    }

    apiTrackingNumber Client::SubscriptionValidate(const char * sessionID, const apiGamecode gamecode, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SubscriptionValidate(sessionID, gamecode, userData) : 0;
    }

    void Client::SessionLogout(const char ** sessionList, const unsigned sessionCount)
    {
        if (mClientCore)
            static_cast<ClientCore *>(mClientCore)->SessionLogout(sessionList, sessionCount);
    }

    void Client::SessionTouch(const char ** sessionList, const unsigned sessionCount)
    {
        if (mClientCore)
            static_cast<ClientCore *>(mClientCore)->SessionTouch(sessionList, sessionCount);
    }

    apiTrackingNumber Client::SessionKick(const char * sessionID, apiKickReason reason, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->SessionKick(sessionID, reason, userData) : 0;
    }

    apiTrackingNumber Client::GetMemberInformation(const apiAccountId accountId, const void * userData)
    {
        return mClientCore ? static_cast<ClientCore *>(mClientCore)->GetMemberInformation(accountId, userData) : 0;
	}

	apiTrackingNumber Client::GetMemberInformation_v2(const apiAccountId accountId, const void * userData)
	{
		return mClientCore ? static_cast<ClientCore *>(mClientCore)->GetMemberInformation_v2(accountId, userData) : 0;
	}

    void Client::SessionKickReply(const char * sessionID, apiKickReply reply)
    {
        if (mClientCore)
            static_cast<ClientCore *>(mClientCore)->SessionKickReply(sessionID, reply);
    }

	apiTrackingNumber Client::GrantFeature_v2(const char * sessionID, unsigned featureType, const char * gameCode, unsigned providerID, unsigned promoID, const void * userData)
	{
		return mClientCore ? static_cast<ClientCore *>(mClientCore)->GrantFeature_v2(sessionID, featureType, gameCode, providerID, promoID, userData) : 0;
	}

	apiTrackingNumber Client::ModifyFeature_v2(unsigned accountID, const char * gameCode, const Feature & origFeature, const Feature & newFeature, const void * userData)
	{
		return mClientCore ? static_cast<ClientCore *>(mClientCore)->ModifyFeature_v2(accountID, gameCode, origFeature, newFeature, userData) : 0;
	}


	apiTrackingNumber Client::GrantFeatureByStationID(unsigned accountID, unsigned featureType,	const char * gameCode, unsigned providerID, unsigned promoID, const void * userData)
	{
		return mClientCore ? static_cast<ClientCore *>(mClientCore)->GrantFeatureByStationID(accountID, featureType, gameCode, providerID, promoID, userData) : 0;
	}


}
