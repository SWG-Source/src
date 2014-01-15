// ======================================================================
//
// PvpEnemiesObserver.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PvpEnemiesObserver.h"

#include "serverGame/BuildingObject.h"
#include "serverGame/CellObject.h"
#include "serverGame/Client.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/Pvp.h"
#include "sharedNetworkMessages/UpdateCellPermissionMessage.h"
#include "sharedObject/Container.h"

// ======================================================================

static std::set<CreatureObject *> s_activeUpdaters;

// ======================================================================

PvpEnemiesObserver::PvpEnemiesObserver(TangibleObject *who, Archive::AutoDeltaObserverOp operation) :
	m_profilerBlock("PvpEnemiesObserver"),
	m_obj(0),
	m_pvpObserver(who, operation),
	m_hadAnyAlignedTimedEnemyFlag(false),
	m_hadAnyBountyDuelEnemyFlag(false)
{
	CreatureObject * const co = who ? who->asCreatureObject() : 0;
	if (!co || !co->getClient() || s_activeUpdaters.find(co) != s_activeUpdaters.end())
		return;

	m_obj = co;
	s_activeUpdaters.insert(co);

	m_hadAnyAlignedTimedEnemyFlag = Pvp::hasAnyAlignedTimedEnemyFlag(*co);
	m_hadAnyBountyDuelEnemyFlag = Pvp::hasAnyBountyDuelEnemyFlag(*co);
}

// ----------------------------------------------------------------------

PvpEnemiesObserver::~PvpEnemiesObserver()
{
	if (!m_obj)
		return;

	s_activeUpdaters.erase(m_obj);

	Client * const client = m_obj->getClient();

	if (client)
	{
		bool const alignedTimedEnemyFlagChanged = (m_hadAnyAlignedTimedEnemyFlag != Pvp::hasAnyAlignedTimedEnemyFlag(*m_obj));
		bool const bountyDuelEnemyFlagChanged = (m_hadAnyBountyDuelEnemyFlag != Pvp::hasAnyBountyDuelEnemyFlag(*m_obj));

		if (!alignedTimedEnemyFlagChanged && !bountyDuelEnemyFlagChanged)
			return;

		// run through all buildings we are observing that care about this and send updates
		Client::ObservingList const &observing = client->getObserving();
		for (Client::ObservingList::const_iterator i = observing.begin(); i != observing.end(); ++i)
		{
			BuildingObject * const building = (*i)->asBuildingObject();
			if (building && !building->playersWithAlignedEnemyFlagsAllowed())
			{
				// This building cares about enemy flags, so send cell permission updates
				Container * const container = ContainerInterface::getContainer(*building);
				for (ContainerIterator j = container->begin(); j != container->end(); ++j)
				{
					Object const * const o = (*j).getObject();
					if (o)
					{
						ServerObject const * const so = o->asServerObject();
						if (so)
						{
							CellObject const * const co = so->asCellObject();
							if (co)
							{
								// We cannot possibly be allowed unless we just lost our last aligned temp enemy flag,
								// so only query our status if that is the case.  Likewise, we don't need to send a
								// status update if we are still not allowed after losing our last aligned temp
								// enemy flag.  We will still generate unnecesary messages when we were previously
								// not allowed, and are gaining our first aligned temp enemy flag, but not needing
								// to know the old state allows us to avoid a lot of work.
								bool allowed = false;
								if ((alignedTimedEnemyFlagChanged && m_hadAnyAlignedTimedEnemyFlag) || (bountyDuelEnemyFlagChanged && m_hadAnyBountyDuelEnemyFlag))
									allowed = co->isAllowed(*m_obj);
								if (allowed || (alignedTimedEnemyFlagChanged && !m_hadAnyAlignedTimedEnemyFlag) || (bountyDuelEnemyFlagChanged && !m_hadAnyBountyDuelEnemyFlag))
								{
									UpdateCellPermissionMessage const permissionsMessage(co->getNetworkId(), allowed);
									client->send(permissionsMessage, true);
								}
							}
						}
					}
				}
			}
		}

		// expel from the building we are in if no longer allowed
		if ((alignedTimedEnemyFlagChanged && !m_hadAnyAlignedTimedEnemyFlag) || (bountyDuelEnemyFlagChanged && !m_hadAnyBountyDuelEnemyFlag)) // if we didn't have aligned enemy flags but we do now
		{
			Object * const o = m_obj->getAttachedTo();
			if (o)
			{
				ServerObject * const so = o->asServerObject();
				if (so)
				{
					CellObject * const co = so->asCellObject();
					if (co)
					{
						BuildingObject * const bo = co->getOwnerBuilding();
						if (bo && !bo->playersWithAlignedEnemyFlagsAllowed())
							bo->expelObject(*m_obj);
					}
				}
			}
		}
	}
}

// ======================================================================

