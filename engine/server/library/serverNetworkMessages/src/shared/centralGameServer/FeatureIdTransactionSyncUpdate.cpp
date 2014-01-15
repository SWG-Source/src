// ======================================================================
//
// FeatureIdTransactionSyncUpdate.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/FeatureIdTransactionSyncUpdate.h"

// ======================================================================

FeatureIdTransactionSyncUpdate::FeatureIdTransactionSyncUpdate(StationId stationId, NetworkId const & player, std::string const & itemId, int adjustment) :
	GameNetworkMessage("FeatureIdTransactionSyncUpdate"),
	m_stationId(stationId),
	m_player(player),
	m_itemId(itemId),
	m_adjustment(adjustment)
{
	addVariable(m_stationId);
	addVariable(m_player);
	addVariable(m_itemId);
	addVariable(m_adjustment);
}

//-----------------------------------------------------------------------

FeatureIdTransactionSyncUpdate::FeatureIdTransactionSyncUpdate(Archive::ReadIterator & source) :
	GameNetworkMessage("FeatureIdTransactionSyncUpdate"),
	m_stationId(),
	m_player(),
	m_itemId(),
	m_adjustment()
{
	addVariable(m_stationId);
	addVariable(m_player);
	addVariable(m_itemId);
	addVariable(m_adjustment);

	unpack(source); //lint !e1506 Call to virtual from constructor
}

//-----------------------------------------------------------------------

FeatureIdTransactionSyncUpdate::~FeatureIdTransactionSyncUpdate()
{
}

// ======================================================================
