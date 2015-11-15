// ======================================================================
//
// NewClient.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NewClient_H
#define INCLUDED_NewClient_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * The New Client message is sent from Connection Server to the Game Server
 * when that connection server tells the game server that an object is being remotely controlled
 * by one of its clients.
 */
class NewClient : public GameNetworkMessage
{
public:
	NewClient (NetworkId const & oid, const std::string & account, const std::string & ip, bool isSecure, bool isSkipLoadScreen, unsigned int stationId, const std::vector <NetworkId> *observedObjects, uint32 gameFeatures, uint32 subscriptionFeatures, unsigned int entitlementTotalTime, unsigned int entitlementEntitledTime, unsigned int entitlementTotalTimeSinceLastLogin, unsigned int entitlementEntitledTimeSinceLastLogin, int buddyPoints, std::vector<std::pair<NetworkId, std::string> > const & consumedRewardEvents, std::vector<std::pair<NetworkId, std::string> > const & claimedRewardItems, bool usingAdminLogin, bool canSkipTutorial, bool sendToStarport=false );
	NewClient (Archive::ReadIterator & source);
	virtual ~NewClient ();

	const std::string &  getAccountName() const;
	const std::string &  getIpAddress() const;
	const NetworkId &    getNetworkId() const;
	bool                 getIsSecure() const;
	bool                 getIsSkipLoadScreen() const;
	const unsigned int   getStationId() const;
	const std::vector<NetworkId> & getObservedObjects() const;
	unsigned int         getGameFeatures() const;
	unsigned int         getSubscriptionFeatures() const;
	unsigned int         getEntitlementTotalTime  () const;
	unsigned int         getEntitlementEntitledTime () const;
	unsigned int         getEntitlementTotalTimeSinceLastLogin () const;
	unsigned int         getEntitlementEntitledTimeSinceLastLogin () const;
	int                  getBuddyPoints () const;
	std::vector<std::pair<NetworkId, std::string> > const & getConsumedRewardEvents() const;
	std::vector<std::pair<NetworkId, std::string> > const & getClaimedRewardItems() const;
	bool                 getUsingAdminLogin() const;
	bool                 getCanSkipTutorial() const;
	bool	 	     getSendToStarport() const;

private:
	Archive::AutoVariable<std::string>     m_account;
	Archive::AutoVariable<std::string>     m_ipAddr;
	Archive::AutoVariable<NetworkId>       m_oid;
	Archive::AutoVariable<bool>            m_secure;
	Archive::AutoVariable<bool>            m_skipLoadScreen;
	Archive::AutoVariable<unsigned int>    m_stationId;
	Archive::AutoArray<NetworkId>          m_observedObjects;
	Archive::AutoVariable<unsigned int>    m_gameFeatures;
	Archive::AutoVariable<unsigned int>    m_subscriptionFeatures;
	Archive::AutoVariable<unsigned int>    m_entitlementTotalTime;
	Archive::AutoVariable<unsigned int>    m_entitlementEntitledTime;
	Archive::AutoVariable<unsigned int>    m_entitlementTotalTimeSinceLastLogin;
	Archive::AutoVariable<unsigned int>    m_entitlementEntitledTimeSinceLastLogin;
	Archive::AutoVariable<int>             m_buddyPoints;
	Archive::AutoArray<std::pair<NetworkId, std::string> >  m_consumedRewardEvents;
	Archive::AutoArray<std::pair<NetworkId, std::string> >  m_claimedRewardItems;
	Archive::AutoVariable<bool>            m_usingAdminLogin;
	Archive::AutoVariable<bool>            m_canSkipTutorial;
	Archive::AutoVariable<bool>            m_sendToStarport;
	
	NewClient();
	NewClient(const NewClient &);
	NewClient& operator = (const NewClient &);
};

// ======================================================================

#endif
