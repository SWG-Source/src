// ======================================================================
//
// UpdateMissileMessage.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/UpdateMissileMessage.h"

// ======================================================================

UpdateMissileMessage::UpdateMissileMessage(int missileId, NetworkId const & shipId, int countermeasureType, UpdateType updateType) :
		GameNetworkMessage("UpdateMissileMessage"),
		m_missileId(missileId),
		m_shipId(shipId),
		m_countermeasureType(countermeasureType),
		m_updateType(static_cast<int>(updateType))
{
	addVariable(m_missileId);
	addVariable(m_shipId);
	addVariable(m_countermeasureType);
	addVariable(m_updateType);
}

// ----------------------------------------------------------------------

UpdateMissileMessage::UpdateMissileMessage(Archive::ReadIterator &source) :
		GameNetworkMessage("UpdateMissileMessage"),
		m_missileId(),
		m_shipId(),
		m_countermeasureType(),
		m_updateType()
{
	addVariable(m_missileId);
	addVariable(m_shipId);
	addVariable(m_countermeasureType);
	addVariable(m_updateType);
	unpack(source); //lint !e1506 // Call to virtual -- the way our system works requires this, and it seems to work OK
}

// ======================================================================

