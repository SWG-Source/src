// ======================================================================
//
// PvpRuleSetBase.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PvpRuleSetBase.h"

#include "serverGame/PvpFactions.h"
#include "serverGame/PvpInternal.h"
#include "serverGame/PvpUpdateObserver.h"
#include "serverGame/TangibleObject.h"
#include "serverNetworkMessages/MessageQueuePvpCommand.h"
#include "serverUtility/PvpEnemy.h"

// ======================================================================

namespace PvpRuleSetBaseNamespace
{
	void applyRepercussions(TangibleObject &actor, std::vector<PvpEnemy> const &repercussions)
	{
		if (repercussions.empty())
			return;

		// if object is authoritative, then apply repercussions immediately;
		// otherwise, forward repercussions over to the authoritative server
		PvpUpdateObserver * o = nullptr;
		MessageQueuePvpCommand * messageQueuePvpCommand = nullptr;

		if (actor.isAuthoritative())
			o = new PvpUpdateObserver(&actor, Archive::ADOO_generic);
		else
			// this will be owned and deallocated by PvpInternal::forwardPvpMessage()
			messageQueuePvpCommand = new MessageQueuePvpCommand();

		for (std::vector<PvpEnemy>::const_iterator i = repercussions.begin(); i != repercussions.end(); ++i)
		{
			if ((*i).enemyId == NetworkId::cms_invalid)
			{
				if (actor.isAuthoritative())
					PvpInternal::setFactionEnemyFlag(actor, (*i).enemyFaction);
				else
					PvpInternal::buildPvpMessageSetFactionEnemyFlag(*messageQueuePvpCommand, (*i).enemyFaction);
			}
			else
			{
				if (actor.isAuthoritative())
					PvpInternal::setPersonalEnemyFlag(actor, (*i).enemyId, (*i).enemyFaction);
				else
					PvpInternal::buildPvpMessageSetPersonalEnemyFlag(*messageQueuePvpCommand, (*i).enemyId, (*i).enemyFaction);
			}
		}

		if (actor.isAuthoritative())
			delete o;
		else
			// forwardPvpMessage() will own messageQueuePvpCommand
			// and will be responsible for deallocation
			PvpInternal::forwardPvpMessage(actor, messageQueuePvpCommand);
	}
}

// ======================================================================

bool PvpRuleSetBase::canAttack(TangibleObject const &, TangibleObject const &) const
{
	return false;
}

// ----------------------------------------------------------------------

bool PvpRuleSetBase::canHelp(TangibleObject const &, TangibleObject const &) const
{
	return false;
}

// ----------------------------------------------------------------------

bool PvpRuleSetBase::isEnemy(TangibleObject const &, TangibleObject const &) const
{
	return false;
}

// ----------------------------------------------------------------------

bool PvpRuleSetBase::isDuelingAllowed(TangibleObject const &, TangibleObject const &) const
{
	return false;
}

// ----------------------------------------------------------------------

void PvpRuleSetBase::getAttackRepercussions(TangibleObject const &, TangibleObject const &, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const
{
	actorRepercussions.clear();
	targetRepercussions.clear();
}

// ----------------------------------------------------------------------

void PvpRuleSetBase::getHelpRepercussions(TangibleObject const &, TangibleObject const &, std::vector<PvpEnemy> &actorRepercussions, std::vector<PvpEnemy> &targetRepercussions) const
{
	actorRepercussions.clear();
	targetRepercussions.clear();
}

// ----------------------------------------------------------------------

void PvpRuleSetBase::applyRepercussions(TangibleObject &actor, TangibleObject &target, std::vector<PvpEnemy> const &actorRepercussions, std::vector<PvpEnemy> const &targetRepercussions)
{
	PvpRuleSetBaseNamespace::applyRepercussions(actor, actorRepercussions);
	PvpRuleSetBaseNamespace::applyRepercussions(target, targetRepercussions);
}

// ======================================================================
