// ======================================================================
//
// GroupIdObserver.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/GroupIdObserver.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/GroupObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"

// ======================================================================

GroupIdObserver::GroupIdObserver(CreatureObject *who, Archive::AutoDeltaObserverOp operation) :
	m_profilerBlock("GroupIdObserver"),
	m_pvpObserver(who, operation),
	m_creature(who),
	m_group(who->getGroup())
{
}

// ----------------------------------------------------------------------

GroupIdObserver::~GroupIdObserver()
{
	if (m_group && m_creature->getGroup() != m_group)
		m_group->removeGroupMember(m_creature->getNetworkId());

	if (m_creature->getGroup() != nullptr)
	{
		PlayerObject *playerObject = PlayerCreatureController::getPlayerObject(m_creature);

		if ((playerObject != nullptr) &&
		    playerObject->isLookingForGroup())
		{
			playerObject->setLookingForGroup(false);
		}
	}
}

// ======================================================================

