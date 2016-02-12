// ======================================================================
//
// TangibleConditionObserver.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/TangibleConditionObserver.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/ServerTangibleObjectTemplate.h"


// ======================================================================

TangibleConditionObserver::TangibleConditionObserver(TangibleObject const *who, Archive::AutoDeltaObserverOp operation):
	m_tangibleObject(who),
	m_oldCondition(who ? who->getCondition() : 0),
	m_pvpUpdateObserver(who, operation)
{
}

// ----------------------------------------------------------------------

TangibleConditionObserver::~TangibleConditionObserver()
{
	if (m_tangibleObject != nullptr)
	{
		const CreatureObject * creature = m_tangibleObject->asCreatureObject();
		if (creature != nullptr)
		{
			int currentCondition = creature->getCondition();
			if ((m_oldCondition & TangibleObject::C_hibernating) != 0 && (currentCondition & TangibleObject::C_hibernating) == 0)
			{
				// we've come out of hibernation
				ObjectTracker::removeHibernatingAI();
			}
			else if ((m_oldCondition & TangibleObject::C_hibernating) == 0 && (currentCondition & TangibleObject::C_hibernating) != 0)
			{
				// we're going into hibernation
				ObjectTracker::addHibernatingAI();
			}
		}

		bool const wasInvulnerable = ((m_oldCondition & ServerTangibleObjectTemplate::C_invulnerable) != 0);
		bool const isInvulnerable = m_tangibleObject->hasCondition(ServerTangibleObjectTemplate::C_invulnerable);
		if ((wasInvulnerable != isInvulnerable) && !m_tangibleObject->getObservers().empty())
		{
			// did the object's "pvp sync" status change because of the invulnerability change?
			bool const wasPvpSync = PvpUpdateObserver::satisfyPvpSyncCondition(m_tangibleObject->isNonPvpObject(), wasInvulnerable, (m_tangibleObject->asCreatureObject() != nullptr), m_tangibleObject->getPvpFaction());
			bool const isPvpSync = PvpUpdateObserver::satisfyPvpSyncCondition(m_tangibleObject->isNonPvpObject(), isInvulnerable, (m_tangibleObject->asCreatureObject() != nullptr), m_tangibleObject->getPvpFaction());

			if (wasPvpSync != isPvpSync)
			{
				// force pvp status update
				Pvp::forceStatusUpdate(*(const_cast<TangibleObject *>(m_tangibleObject)));

				const std::set<Client *> &observers = m_tangibleObject->getObservers();
				for (std::set<Client *>::const_iterator i = observers.begin(); i != observers.end(); ++i)
				{
					if (isPvpSync)
						(*i)->addObservingPvpSync(const_cast<TangibleObject *>(m_tangibleObject));
					else
						(*i)->removeObservingPvpSync(const_cast<TangibleObject *>(m_tangibleObject));
				}
			}
		}
	}
}

// ======================================================================
