// ======================================================================
//
// ClaimRewardsReplyMessage.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ClaimRewardsReplyMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"

// ======================================================================

ClaimRewardsReplyMessage::ClaimRewardsReplyMessage(uint32 gameServer, StationId stationId, NetworkId const & player, std::string const & rewardEvent, std::string const & rewardItem, uint32 accountFeatureId, bool consumeAccountFeatureId, int previousAccountFeatureIdCount, int currentAccountFeatureIdCount, bool result) :
		GameNetworkMessage("ClaimRewardsReplyMessage"),
		m_gameServer(gameServer),
		m_stationId(stationId),
		m_player(player),
		m_rewardEvent(rewardEvent),
		m_accountUniqueItems(),
		m_additionalItems(),
		m_accountFeatureId(accountFeatureId),
		m_consumeAccountFeatureId((accountFeatureId > 0) ? consumeAccountFeatureId : false), // m_consumeAccountFeatureId is only meaningful if m_accountFeatureId > 0
		m_previousAccountFeatureIdCount(previousAccountFeatureIdCount),
		m_currentAccountFeatureIdCount(currentAccountFeatureIdCount),
		m_result(result)
{
	m_accountUniqueItems.get().push_back(rewardItem);
	
	addVariable(m_gameServer);
	addVariable(m_stationId);
	addVariable(m_player);
	addVariable(m_rewardEvent);
	addVariable(m_accountUniqueItems);
	addVariable(m_additionalItems);
	addVariable(m_accountFeatureId);
	addVariable(m_consumeAccountFeatureId);
	addVariable(m_previousAccountFeatureIdCount);
	addVariable(m_currentAccountFeatureIdCount);
	addVariable(m_result);
}

//-----------------------------------------------------------------------

ClaimRewardsReplyMessage::ClaimRewardsReplyMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("ClaimRewardsReplyMessage"),
		m_gameServer(),
		m_stationId(),
		m_player(),
		m_rewardEvent(),
		m_accountUniqueItems(),
		m_additionalItems(),
		m_accountFeatureId(),
		m_consumeAccountFeatureId(),
		m_previousAccountFeatureIdCount(),
		m_currentAccountFeatureIdCount(),
		m_result()
{
	addVariable(m_gameServer);
	addVariable(m_stationId);
	addVariable(m_player);
	addVariable(m_rewardEvent);
	addVariable(m_accountUniqueItems);
	addVariable(m_additionalItems);
	addVariable(m_accountFeatureId);
	addVariable(m_consumeAccountFeatureId);
	addVariable(m_previousAccountFeatureIdCount);
	addVariable(m_currentAccountFeatureIdCount);
	addVariable(m_result);
	
	unpack(source); //lint !e1506 Call to virtual from constructor
}

//-----------------------------------------------------------------------

ClaimRewardsReplyMessage::~ClaimRewardsReplyMessage()
{
}

// ----------------------------------------------------------------------

uint32 ClaimRewardsReplyMessage::getGameServer() const
{
	return m_gameServer.get();
}

// ----------------------------------------------------------------------

StationId ClaimRewardsReplyMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

NetworkId const & ClaimRewardsReplyMessage::getPlayer() const
{
	return m_player.get();
}

// ----------------------------------------------------------------------

std::string const & ClaimRewardsReplyMessage::getRewardEvent() const
{
	return m_rewardEvent.get();
}

// ----------------------------------------------------------------------

std::string const & ClaimRewardsReplyMessage::getRewardItem() const
{
	static const std::string empty;

	if (!m_accountUniqueItems.get().empty())
		return (m_accountUniqueItems.get())[0];
	else if (!m_additionalItems.get().empty())
		return (m_additionalItems.get())[0];

	return empty;
}

// ----------------------------------------------------------------------

uint32 ClaimRewardsReplyMessage::getAccountFeatureId() const
{
	return m_accountFeatureId.get();
}

// ----------------------------------------------------------------------

bool ClaimRewardsReplyMessage::getConsumeAccountFeatureId() const
{
	return m_consumeAccountFeatureId.get();
}

// ----------------------------------------------------------------------

int ClaimRewardsReplyMessage::getPreviousAccountFeatureIdCount() const
{
	return m_previousAccountFeatureIdCount.get();
}

// ----------------------------------------------------------------------

int ClaimRewardsReplyMessage::getCurrentAccountFeatureIdCount() const
{
	return m_currentAccountFeatureIdCount.get();
}

// ----------------------------------------------------------------------

bool ClaimRewardsReplyMessage::getResult() const
{
	return m_result.get();
}

// ======================================================================
