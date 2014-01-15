// ======================================================================
//
// AuthTransferClientMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_AuthTransferClientMessage_H
#define	_INCLUDED_AuthTransferClientMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include <map>

// ======================================================================

class AuthTransferClientMessage: public GameNetworkMessage
{
public:
	AuthTransferClientMessage(NetworkId const & networkId,
	                          std::string const & connectionServerIp,
	                          uint16 connectionServerPort,
	                          bool skipLoadScreen,
	                          std::string const & account,
	                          std::string const & ipAddr,
	                          bool secure,
	                          unsigned int stationId,
	                          std::vector<NetworkId> const * observedObjects,
	                          uint32 gameFeatures,
	                          uint32 subscriptionFeatures,
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
							  bool isJediSlotCharacter);

	AuthTransferClientMessage(Archive::ReadIterator &source);
	~AuthTransferClientMessage();

public:
	NetworkId const &                      getNetworkId() const;
	std::string const &                    getConnectionServerIp() const;
	uint16                                 getConnectionServerPort() const;
	bool                                   getSkipLoadScreen() const;
	std::string const &                    getAccount() const;
	std::string const &                    getIpAddress() const;
	bool                                   getSecure() const;
	unsigned int                           getStationId() const;
	std::vector<NetworkId> const &         getObservedObjects() const;
	uint32                                 getGameFeatures() const;
	uint32                                 getSubscriptionFeatures() const;
	std::map<uint32, int> const &          getAccountFeatureIds() const;
	unsigned int                           getEntitlementTotalTime  () const;
	unsigned int                           getEntitlementEntitledTime () const;
	unsigned int                           getEntitlementTotalTimeSinceLastLogin () const;
	unsigned int                           getEntitlementEntitledTimeSinceLastLogin () const;
	int                                    getBuddyPoints() const;
	uint32                                 getSourceServerPid() const;
	std::vector<std::pair<NetworkId, std::string> > const & getConsumedRewardEvents() const;
	std::vector<std::pair<NetworkId, std::string> > const & getClaimedRewardItems() const;
	bool                                   getUsingAdminLogin() const;
	int                                    getCombatSpamFilter() const;
	int                                    getCombatSpamRangeSquaredFilter() const;
	int                                    getFurnitureRotationDegree() const;
	bool                                   getHasUnoccupiedJediSlot() const;
	bool                                   getIsJediSlotCharacter() const;

private:
	Archive::AutoVariable<NetworkId>       m_networkId;
	Archive::AutoVariable<std::string>     m_connectionServerIp;
	Archive::AutoVariable<uint16>          m_connectionServerPort;
	Archive::AutoVariable<bool>            m_skipLoadScreen;
	Archive::AutoVariable<std::string>     m_account;
	Archive::AutoVariable<std::string>     m_ipAddr;
	Archive::AutoVariable<bool>            m_secure;
	Archive::AutoVariable<unsigned int>    m_stationId;
	Archive::AutoArray<NetworkId>          m_observedObjects;
	Archive::AutoVariable<uint32>          m_gameFeatures;
	Archive::AutoVariable<uint32>          m_subscriptionFeatures;
	Archive::AutoVariable<std::map<uint32, int> > m_accountFeatureIds;
	Archive::AutoVariable<unsigned int>    m_entitlementTotalTime;
	Archive::AutoVariable<unsigned int>    m_entitlementEntitledTime;
	Archive::AutoVariable<unsigned int>    m_entitlementTotalTimeSinceLastLogin;
	Archive::AutoVariable<unsigned int>    m_entitlementEntitledTimeSinceLastLogin;
	Archive::AutoVariable<int>             m_buddyPoints;
	Archive::AutoVariable<uint32>          m_sourceServerPid;
	Archive::AutoArray<std::pair<NetworkId, std::string> > m_consumedRewardEvents;
	Archive::AutoArray<std::pair<NetworkId, std::string> > m_claimedRewardItems;
	Archive::AutoVariable<bool>            m_usingAdminLogin;
	Archive::AutoVariable<int>             m_combatSpamFilter;
	Archive::AutoVariable<int>             m_combatSpamRangeSquaredFilter;
	Archive::AutoVariable<int>             m_furnitureRotationDegree;
	Archive::AutoVariable<bool>            m_hasUnoccupiedJediSlot;
	Archive::AutoVariable<bool>            m_isJediSlotCharacter;

private:
	AuthTransferClientMessage(AuthTransferClientMessage const &);
	AuthTransferClientMessage &operator=(AuthTransferClientMessage const &);
};

// ======================================================================

inline NetworkId const & AuthTransferClientMessage::getNetworkId() const
{
	return m_networkId.get();
}

//-----------------------------------------------------------------------

inline std::string const & AuthTransferClientMessage::getConnectionServerIp() const
{
	return m_connectionServerIp.get();
}

//-----------------------------------------------------------------------

inline uint16 AuthTransferClientMessage::getConnectionServerPort() const
{
	return m_connectionServerPort.get();
}

//-----------------------------------------------------------------------

inline bool AuthTransferClientMessage::getSkipLoadScreen() const
{
	return m_skipLoadScreen.get();
}

//-----------------------------------------------------------------------

inline std::string const & AuthTransferClientMessage::getAccount() const
{
	return m_account.get();
}

//-----------------------------------------------------------------------

inline std::string const & AuthTransferClientMessage::getIpAddress() const
{
	return m_ipAddr.get();
}

//-----------------------------------------------------------------------

inline bool AuthTransferClientMessage::getSecure() const
{
	return m_secure.get();
}

//-----------------------------------------------------------------------

inline unsigned int AuthTransferClientMessage::getStationId() const
{
	return m_stationId.get();
}

//-----------------------------------------------------------------------

inline std::vector<NetworkId> const & AuthTransferClientMessage::getObservedObjects() const
{
	return m_observedObjects.get();
}

//-----------------------------------------------------------------------

inline uint32 AuthTransferClientMessage::getGameFeatures() const
{
	return m_gameFeatures.get();
}

//-----------------------------------------------------------------------

inline uint32 AuthTransferClientMessage::getSubscriptionFeatures() const
{
	return m_subscriptionFeatures.get();
}

//-----------------------------------------------------------------------

inline std::map<uint32, int> const & AuthTransferClientMessage::getAccountFeatureIds() const
{
	return m_accountFeatureIds.get();
}

// ----------------------------------------------------------------------

inline unsigned int AuthTransferClientMessage::getEntitlementTotalTime() const
{
	return m_entitlementTotalTime.get();
}

// ----------------------------------------------------------------------

inline unsigned int AuthTransferClientMessage::getEntitlementEntitledTime() const
{
	return m_entitlementEntitledTime.get();
}

// ----------------------------------------------------------------------

inline unsigned int AuthTransferClientMessage::getEntitlementTotalTimeSinceLastLogin() const
{
	return m_entitlementTotalTimeSinceLastLogin.get();
}

// ----------------------------------------------------------------------

inline unsigned int AuthTransferClientMessage::getEntitlementEntitledTimeSinceLastLogin() const
{
	return m_entitlementEntitledTimeSinceLastLogin.get();
}

// ----------------------------------------------------------------------

inline int AuthTransferClientMessage::getBuddyPoints() const
{
	return m_buddyPoints.get();
}

// ----------------------------------------------------------------------

inline uint32 AuthTransferClientMessage::getSourceServerPid() const
{
	return m_sourceServerPid.get();
}

// ----------------------------------------------------------------------

inline std::vector<std::pair<NetworkId, std::string> > const & AuthTransferClientMessage::getConsumedRewardEvents() const
{
	return m_consumedRewardEvents.get();
}

// ----------------------------------------------------------------------

inline std::vector<std::pair<NetworkId, std::string> > const & AuthTransferClientMessage::getClaimedRewardItems() const
{
	return m_claimedRewardItems.get();
}

// ----------------------------------------------------------------------

inline bool AuthTransferClientMessage::getUsingAdminLogin() const
{
	return m_usingAdminLogin.get();
}

// ----------------------------------------------------------------------

inline int AuthTransferClientMessage::getCombatSpamFilter() const
{
	return m_combatSpamFilter.get();
}

// ----------------------------------------------------------------------

inline int AuthTransferClientMessage::getCombatSpamRangeSquaredFilter() const
{
	return m_combatSpamRangeSquaredFilter.get();
}

// ----------------------------------------------------------------------

inline int AuthTransferClientMessage::getFurnitureRotationDegree() const
{
	return m_furnitureRotationDegree.get();
}

//-----------------------------------------------------------------------

inline bool AuthTransferClientMessage::getHasUnoccupiedJediSlot() const
{
	return m_hasUnoccupiedJediSlot.get();
}

//-----------------------------------------------------------------------

inline bool AuthTransferClientMessage::getIsJediSlotCharacter() const
{
	return m_isJediSlotCharacter.get();
}

// ======================================================================

#endif	// _INCLUDED_AuthTransferClientMessage_H
