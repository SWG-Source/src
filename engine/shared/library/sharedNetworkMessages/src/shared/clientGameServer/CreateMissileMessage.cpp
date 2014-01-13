// ======================================================================
//
// CreateMissileMessage.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CreateMissileMessage.h"

// ======================================================================

CreateMissileMessage::CreateMissileMessage(int missileId, const NetworkId &source, const NetworkId &target, const Vector & sourceLocation, const Vector & targetLocation, int impactTime, int missileTypeId, int weaponId, int targetComponent) :
		GameNetworkMessage("CreateMissileMessage"),
		m_missileId(missileId),
		m_source(source),
		m_target(target),
		m_sourceLocation(sourceLocation),
		m_targetLocation(targetLocation),
		m_impactTime(impactTime),
		m_missileTypeId(missileTypeId),
		m_weaponId(weaponId),
		m_targetComponent(targetComponent)
{
	addVariable(m_missileId);
	addVariable(m_source);
	addVariable(m_target);
	addVariable(m_sourceLocation);
	addVariable(m_targetLocation);
	addVariable(m_impactTime);
	addVariable(m_missileTypeId);
	addVariable(m_weaponId);
	addVariable(m_targetComponent);
}

// ----------------------------------------------------------------------

CreateMissileMessage::CreateMissileMessage(Archive::ReadIterator &source) :
		GameNetworkMessage("CreateMissileMessage"),
		m_missileId(),
		m_source(),
		m_target(),
		m_sourceLocation(),
		m_targetLocation(),
		m_impactTime(),
		m_missileTypeId(),
		m_weaponId(),
		m_targetComponent()
{
	addVariable(m_missileId);
	addVariable(m_source);
	addVariable(m_target);
	addVariable(m_sourceLocation);
	addVariable(m_targetLocation);
	addVariable(m_impactTime);
	addVariable(m_missileTypeId);
	addVariable(m_weaponId);
	addVariable(m_targetComponent);
	unpack(source); //lint !e1506 // Call to virtual -- the way our system works requires this, and it seems to work OK
}

// ======================================================================
