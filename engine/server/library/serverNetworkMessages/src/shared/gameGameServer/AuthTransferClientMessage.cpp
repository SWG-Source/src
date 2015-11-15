// ======================================================================
//
// AuthTransferClientMessage.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/AuthTransferClientMessage.h"

// ======================================================================

AuthTransferClientMessage::AuthTransferClientMessage(NetworkId const & networkId,
                                                     std::string const & connectionServerIp,
                                                     uint16 const connectionServerPort,
                                                     bool const skipLoadScreen,
                                                     std::string const & account,
                                                     std::string const & ipAddr,
                                                     bool const secure,
                                                     unsigned int const stationId,
                                                     std::vector<NetworkId> const * observedObjects,
                                                     uint32 const gameFeatures,
                                                     uint32 const subscriptionFeatures,
                                                     std::map<uint32, int> const & accountFeatureIds,
                                                     unsigned int entitlementTotalTime,
                                                     unsigned int entitlementEntitledTime,
                                                     unsigned int entitlementTotalTimeSinceLastLogin,
                                                     unsigned int entitlementEntitledTimeSinceLastLogin,
													 int buddyPoints,
                                                     uint32 sourceServerPid,
                                                     std::vector<std::pair<NetworkId, std::string> > const & consumedRewardEvents,
                                                     std::vector<std::pair<NetworkId, std::string> > const & claimedRewardItems,
													 bool usingAdminLogin,
													 int combatSpamFilter,
													 int combatSpamRangeSquaredFilter,
													 int furnitureRotationDegree,
													 bool hasUnoccupiedJediSlot,
													 bool isJediSlotCharacter) :
	GameNetworkMessage("AuthTransferClientMessage"),
	m_networkId(networkId),
	m_connectionServerIp(connectionServerIp),
	m_connectionServerPort(connectionServerPort),
	m_skipLoadScreen(skipLoadScreen),
	m_account(account),
	m_ipAddr(ipAddr),
	m_secure(secure),
	m_stationId(stationId),
	m_observedObjects(),
	m_gameFeatures(gameFeatures),
	m_subscriptionFeatures(subscriptionFeatures),
	m_accountFeatureIds(accountFeatureIds),
	m_entitlementTotalTime(entitlementTotalTime),
	m_entitlementEntitledTime(entitlementEntitledTime),
	m_entitlementTotalTimeSinceLastLogin(entitlementTotalTimeSinceLastLogin),
	m_entitlementEntitledTimeSinceLastLogin(entitlementEntitledTimeSinceLastLogin),
	m_buddyPoints(buddyPoints),
	m_sourceServerPid(sourceServerPid),
	m_consumedRewardEvents(),
	m_claimedRewardItems(),
	m_usingAdminLogin(usingAdminLogin),
	m_combatSpamFilter(combatSpamFilter),
	m_combatSpamRangeSquaredFilter(combatSpamRangeSquaredFilter),
	m_furnitureRotationDegree(furnitureRotationDegree),
	m_hasUnoccupiedJediSlot(hasUnoccupiedJediSlot),
	m_isJediSlotCharacter(isJediSlotCharacter)
{
	if (observedObjects)
		m_observedObjects.set(*observedObjects);
	m_consumedRewardEvents.set(consumedRewardEvents);
	m_claimedRewardItems.set(claimedRewardItems);
	
	addVariable(m_networkId);
	addVariable(m_connectionServerIp);
	addVariable(m_connectionServerPort);
	addVariable(m_skipLoadScreen);
	addVariable(m_account);
	addVariable(m_ipAddr);
	addVariable(m_secure);
	addVariable(m_stationId);
	addVariable(m_observedObjects);
	addVariable(m_gameFeatures);
	addVariable(m_subscriptionFeatures);
	addVariable(m_accountFeatureIds);
	addVariable(m_entitlementTotalTime);
	addVariable(m_entitlementEntitledTime);
	addVariable(m_entitlementTotalTimeSinceLastLogin);
	addVariable(m_entitlementEntitledTimeSinceLastLogin);
	addVariable(m_buddyPoints);
	addVariable(m_sourceServerPid);
	addVariable(m_consumedRewardEvents);
	addVariable(m_claimedRewardItems);
	addVariable(m_usingAdminLogin);
	addVariable(m_combatSpamFilter);
	addVariable(m_combatSpamRangeSquaredFilter);
	addVariable(m_furnitureRotationDegree);
	addVariable(m_hasUnoccupiedJediSlot);
	addVariable(m_isJediSlotCharacter);
}

// ----------------------------------------------------------------------

AuthTransferClientMessage::AuthTransferClientMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("AuthTransferClientMessage"),
	m_networkId(),
	m_connectionServerIp(),
	m_connectionServerPort(),
	m_skipLoadScreen(),
	m_account(),
	m_ipAddr(),
	m_secure(),
	m_stationId(),
	m_observedObjects(),
	m_gameFeatures(),
	m_subscriptionFeatures(),
	m_accountFeatureIds(),
	m_entitlementTotalTime(),
	m_entitlementEntitledTime(),
	m_entitlementTotalTimeSinceLastLogin(),
	m_entitlementEntitledTimeSinceLastLogin(),
	m_buddyPoints(),
	m_sourceServerPid(),
	m_consumedRewardEvents(),
	m_claimedRewardItems(),
	m_usingAdminLogin(),
	m_combatSpamFilter(),
	m_combatSpamRangeSquaredFilter(),
	m_furnitureRotationDegree(),
	m_hasUnoccupiedJediSlot(),
	m_isJediSlotCharacter()
{
	addVariable(m_networkId);
	addVariable(m_connectionServerIp);
	addVariable(m_connectionServerPort);
	addVariable(m_skipLoadScreen);
	addVariable(m_account);
	addVariable(m_ipAddr);
	addVariable(m_secure);
	addVariable(m_stationId);
	addVariable(m_observedObjects);
	addVariable(m_gameFeatures);
	addVariable(m_subscriptionFeatures);
	addVariable(m_accountFeatureIds);
	addVariable(m_entitlementTotalTime);
	addVariable(m_entitlementEntitledTime);
	addVariable(m_entitlementTotalTimeSinceLastLogin);
	addVariable(m_entitlementEntitledTimeSinceLastLogin);
	addVariable(m_buddyPoints);
	addVariable(m_sourceServerPid);
	addVariable(m_consumedRewardEvents);
	addVariable(m_claimedRewardItems);
	addVariable(m_usingAdminLogin);
	addVariable(m_combatSpamFilter);
	addVariable(m_combatSpamRangeSquaredFilter);
	addVariable(m_furnitureRotationDegree);
	addVariable(m_hasUnoccupiedJediSlot);
	addVariable(m_isJediSlotCharacter);

	unpack(source);  //lint !e1506 Call to virtual from constructor
}

// ----------------------------------------------------------------------

AuthTransferClientMessage::~AuthTransferClientMessage()
{
}

// ======================================================================

