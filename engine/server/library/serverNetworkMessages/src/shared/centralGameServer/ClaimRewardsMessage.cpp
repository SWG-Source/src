// ======================================================================
//
// ClaimRewardsMessage.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ClaimRewardsMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"

// ======================================================================

ClaimRewardsMessage::ClaimRewardsMessage  (uint32 gameServer, StationId stationId, NetworkId const & player, std::string const & rewardEvent, bool consumeEvent, std::string const & rewardItem, bool consumeItem, uint32 accountFeatureId, bool consumeAccountFeatureId) :
		GameNetworkMessage("ClaimRewardsMessage"),
		m_gameServer(gameServer),
		m_stationId(stationId),
		m_player(player),
		m_rewardEvent(rewardEvent),
		m_consumeEvent(consumeEvent),
		m_accountUniqueItems(),
		m_additionalItems(),
		m_accountFeatureId(accountFeatureId),
		m_consumeAccountFeatureId((accountFeatureId > 0) ? consumeAccountFeatureId : false), // m_consumeAccountFeatureId is only meaningful if m_accountFeatureId > 0
		m_accountFeatureIdOldValue(),
		m_accountFeatureIdNewValue()
{
	if (consumeItem)
		m_accountUniqueItems.get().push_back(rewardItem);
	else
		m_additionalItems.get().push_back(rewardItem);
	
	addVariable(m_gameServer);
	addVariable(m_stationId);
	addVariable(m_player);
	addVariable(m_rewardEvent);
	addVariable(m_consumeEvent);
	addVariable(m_accountUniqueItems);
	addVariable(m_additionalItems);
	addVariable(m_accountFeatureId);
	addVariable(m_consumeAccountFeatureId);
	addVariable(m_accountFeatureIdOldValue);
	addVariable(m_accountFeatureIdNewValue);
}

//-----------------------------------------------------------------------

ClaimRewardsMessage::ClaimRewardsMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("ClaimRewardsMessage"),
		m_gameServer(),
		m_stationId(),
		m_player(),
		m_rewardEvent(),
		m_consumeEvent(),
		m_accountUniqueItems(),
		m_additionalItems(),
		m_accountFeatureId(),
		m_consumeAccountFeatureId(),
		m_accountFeatureIdOldValue(),
		m_accountFeatureIdNewValue()
{
	addVariable(m_gameServer);
	addVariable(m_stationId);
	addVariable(m_player);
	addVariable(m_rewardEvent);
	addVariable(m_consumeEvent);
	addVariable(m_accountUniqueItems);
	addVariable(m_additionalItems);
	addVariable(m_accountFeatureId);
	addVariable(m_consumeAccountFeatureId);
	addVariable(m_accountFeatureIdOldValue);
	addVariable(m_accountFeatureIdNewValue);

	unpack(source); //lint !e1506 Call to virtual from constructor
}

//-----------------------------------------------------------------------

ClaimRewardsMessage::~ClaimRewardsMessage()
{
}

// ----------------------------------------------------------------------

uint32 ClaimRewardsMessage::getGameServer() const
{
	return m_gameServer.get();
}

// ----------------------------------------------------------------------

StationId ClaimRewardsMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

NetworkId const & ClaimRewardsMessage::getPlayer() const
{
	return m_player.get();
}

// ----------------------------------------------------------------------

std::string const & ClaimRewardsMessage::getRewardEvent() const
{
	return m_rewardEvent.get();
}

// ----------------------------------------------------------------------

bool ClaimRewardsMessage::getConsumeEvent() const
{
	return m_consumeEvent.get();
}

// ----------------------------------------------------------------------

std::string const & ClaimRewardsMessage::getRewardItem() const
{
	static const std::string empty;

	if (!m_accountUniqueItems.get().empty())
		return (m_accountUniqueItems.get())[0];
	else if (!m_additionalItems.get().empty())
		return (m_additionalItems.get())[0];

	return empty;
}

// ----------------------------------------------------------------------

bool ClaimRewardsMessage::getConsumeItem() const
{
	return (!m_accountUniqueItems.get().empty());
}

// ----------------------------------------------------------------------

uint32 ClaimRewardsMessage::getAccountFeatureId() const
{
	return m_accountFeatureId.get();
}

// ----------------------------------------------------------------------

bool ClaimRewardsMessage::getConsumeAccountFeatureId() const
{
	return m_consumeAccountFeatureId.get();
}

// ----------------------------------------------------------------------

std::string const & ClaimRewardsMessage::getAccountFeatureIdOldValue() const
{
	return m_accountFeatureIdOldValue.get();
}

// ----------------------------------------------------------------------

void ClaimRewardsMessage::setAccountFeatureIdOldValue(std::string const & accountFeatureIdOldValue)
{
	m_accountFeatureIdOldValue.set(accountFeatureIdOldValue);
}

// ----------------------------------------------------------------------

std::string const & ClaimRewardsMessage::getAccountFeatureIdNewValue() const
{
	return m_accountFeatureIdNewValue.get();
}

// ----------------------------------------------------------------------

void ClaimRewardsMessage::setAccountFeatureIdNewValue(std::string const & accountFeatureIdNewValue)
{
	m_accountFeatureIdNewValue.set(accountFeatureIdNewValue);
}

// ======================================================================
