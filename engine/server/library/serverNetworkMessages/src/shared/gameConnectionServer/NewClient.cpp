// ======================================================================
//
// NewClient.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/NewClient.h"

// ======================================================================

NewClient::NewClient(NetworkId const & oid, const std::string & account, const std::string & ip, bool isSecure, bool isSkipLoadScreen, unsigned int stationId, const std::vector <NetworkId> *observedObjects, uint32 gameFeatures, uint32 subscriptionFeatures, unsigned int entitlementTotalTime, unsigned int entitlementEntitledTime, unsigned int entitlementTotalTimeSinceLastLogin, unsigned int entitlementEntitledTimeSinceLastLogin, int buddyPoints, std::vector<std::pair<NetworkId, std::string> > const & consumedRewardEvents, std::vector<std::pair<NetworkId, std::string> > const & claimedRewardItems, bool usingAdminLogin, bool canSkipTutorial, bool sendToStarport ) :
	GameNetworkMessage("NewClient"),
	m_account(account),
	m_ipAddr(ip),
	m_oid(oid),
	m_secure(isSecure),
	m_skipLoadScreen(isSkipLoadScreen),
	m_stationId(stationId),
	m_observedObjects(),
	m_gameFeatures(gameFeatures),
	m_subscriptionFeatures(subscriptionFeatures),
	m_entitlementTotalTime(entitlementTotalTime),
	m_entitlementEntitledTime(entitlementEntitledTime),
	m_entitlementTotalTimeSinceLastLogin(entitlementTotalTimeSinceLastLogin),
	m_entitlementEntitledTimeSinceLastLogin(entitlementEntitledTimeSinceLastLogin),
	m_buddyPoints(buddyPoints),
	m_consumedRewardEvents(),
	m_claimedRewardItems(),
	m_usingAdminLogin(usingAdminLogin),
	m_canSkipTutorial(canSkipTutorial),
	m_sendToStarport(sendToStarport)
{
	if (observedObjects)
		m_observedObjects.set(*observedObjects);

	m_consumedRewardEvents.set(consumedRewardEvents);
	m_claimedRewardItems.set(claimedRewardItems);
			
	addVariable(m_account);
	addVariable(m_ipAddr);
	addVariable(m_oid);
	addVariable(m_secure);
	addVariable(m_skipLoadScreen);
	addVariable(m_stationId);
	addVariable(m_observedObjects);
	addVariable(m_gameFeatures);
	addVariable(m_subscriptionFeatures);
	addVariable(m_entitlementTotalTime);
	addVariable(m_entitlementEntitledTime);
	addVariable(m_entitlementTotalTimeSinceLastLogin);
	addVariable(m_entitlementEntitledTimeSinceLastLogin);
	addVariable(m_buddyPoints);
	addVariable(m_consumedRewardEvents);
	addVariable(m_claimedRewardItems);
	addVariable(m_usingAdminLogin);	
	addVariable(m_canSkipTutorial);	
	addVariable(m_sendToStarport);
}

//-----------------------------------------------------------------------

NewClient::NewClient (Archive::ReadIterator & source) : 
	GameNetworkMessage("NewClient"),
	m_account(),
	m_ipAddr(),
	m_oid(NetworkId::cms_invalid),
	m_secure(false),
	m_skipLoadScreen(false),
	m_stationId(0),
	m_observedObjects(),
	m_gameFeatures(0),
	m_subscriptionFeatures(0),
	m_entitlementTotalTime(0),
	m_entitlementEntitledTime(0),
	m_entitlementTotalTimeSinceLastLogin(0),
	m_entitlementEntitledTimeSinceLastLogin(0),
	m_buddyPoints(0),
	m_consumedRewardEvents(),
	m_claimedRewardItems(),
	m_usingAdminLogin(false),
	m_canSkipTutorial(false),
	m_sendToStarport(false)
{
	addVariable(m_account);
	addVariable(m_ipAddr);
	addVariable(m_oid);
	addVariable(m_secure);
	addVariable(m_skipLoadScreen);
	addVariable(m_stationId);
	addVariable(m_observedObjects);
	addVariable(m_gameFeatures);
	addVariable(m_subscriptionFeatures);
	addVariable(m_entitlementTotalTime);
	addVariable(m_entitlementEntitledTime);
	addVariable(m_entitlementTotalTimeSinceLastLogin);
	addVariable(m_entitlementEntitledTimeSinceLastLogin);
	addVariable(m_buddyPoints);
	addVariable(m_consumedRewardEvents);
	addVariable(m_claimedRewardItems);
	addVariable(m_usingAdminLogin);	
	addVariable(m_canSkipTutorial);	
	addVariable(m_sendToStarport);	

	unpack(source);  //lint !e1506 Call to virtual from constructor
}

//-----------------------------------------------------------------------

NewClient::~NewClient()
{
}

//-----------------------------------------------------------------------

const std::string & NewClient::getAccountName() const
{
	return m_account.get();
}

//-----------------------------------------------------------------------

const std::string & NewClient::getIpAddress() const
{
	return m_ipAddr.get();
}

//-----------------------------------------------------------------------

const NetworkId &NewClient::getNetworkId() const
{
	return m_oid.get();
}
//-----------------------------------------------------------------------

bool NewClient::getIsSecure() const
{
	return m_secure.get();
}

//-----------------------------------------------------------------------

const unsigned int NewClient::getStationId() const
{
	return m_stationId.get();
}

//-----------------------------------------------------------------------

bool NewClient::getIsSkipLoadScreen() const
{
	return m_skipLoadScreen.get();
}

//-----------------------------------------------------------------------

const std::vector<NetworkId> &NewClient::getObservedObjects() const
{
	return m_observedObjects.get();
}

//-----------------------------------------------------------------------

unsigned int NewClient::getGameFeatures() const
{
	return m_gameFeatures.get();
}

//-----------------------------------------------------------------------

unsigned int NewClient::getSubscriptionFeatures() const
{
	return m_subscriptionFeatures.get();
}

// ----------------------------------------------------------------------

unsigned int NewClient::getEntitlementTotalTime() const
{
	return m_entitlementTotalTime.get();
}

// ----------------------------------------------------------------------

unsigned int NewClient::getEntitlementEntitledTime() const
{
	return m_entitlementEntitledTime.get();
}

// ----------------------------------------------------------------------

unsigned int NewClient::getEntitlementTotalTimeSinceLastLogin() const
{
	return m_entitlementTotalTimeSinceLastLogin.get();
}

// ----------------------------------------------------------------------

unsigned int NewClient::getEntitlementEntitledTimeSinceLastLogin() const
{
	return m_entitlementEntitledTimeSinceLastLogin.get();
}

// ----------------------------------------------------------------------

std::vector<std::pair<NetworkId, std::string> > const & NewClient::getConsumedRewardEvents() const
{
	return m_consumedRewardEvents.get();
}

// ----------------------------------------------------------------------

std::vector<std::pair<NetworkId, std::string> > const & NewClient::getClaimedRewardItems() const
{
	return m_claimedRewardItems.get();
}

// ----------------------------------------------------------------------

bool NewClient::getUsingAdminLogin() const
{
	return m_usingAdminLogin.get();
}

// ----------------------------------------------------------------------

int NewClient::getBuddyPoints() const
{
	return m_buddyPoints.get();
}

// ----------------------------------------------------------------------

bool NewClient::getCanSkipTutorial() const
{
        return m_canSkipTutorial.get();
}

bool NewClient::getSendToStarport() const
{
        return m_sendToStarport.get();
}


// ======================================================================
