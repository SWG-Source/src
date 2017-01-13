// ======================================================================
//
// AggroListProperty.cpp
// Copyright 2005 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AggroListProperty.h"

#include "serverGame/AiLogManager.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/TangibleObject.h"
#include "serverGame/Pvp.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedLog/Log.h"

#include <set>

// ======================================================================
//
// AggroListPropertyNamespace
//
// ======================================================================

// ----------------------------------------------------------------------
namespace AggroListPropertyNamespace
{
	bool canAttackTarget(TangibleObject const & attacker, TangibleObject const & target);

	bool isIncapacitated(TangibleObject const & target);
	bool isDead(TangibleObject const & target);
	bool isInvulnerable(TangibleObject const & target);
	bool isGm(TangibleObject const & target);
	bool isFeigningDeath(TangibleObject const & target);
	bool isInPlayerBuilding(TangibleObject const & target);
	bool isAggroImmune(TangibleObject const & target);
}

using namespace AggroListPropertyNamespace;

// ----------------------------------------------------------------------
bool AggroListPropertyNamespace::canAttackTarget(TangibleObject const & attacker, TangibleObject const & target)
{
	bool result = true;

	if (isIncapacitated(target))
	{
		//LOGC(AiLogManager::isLogging(attacker), "debug_ai", ("AggroListPropertyNamespace::canAttackTarget() attacker(%s) target(%s) Target is incapped", attacker.getDebugInformation().c_str(), target.getDebugInformation().c_str()));
		result = false;
	}
	else if (isDead(target))
	{
		//LOGC(AiLogManager::isLogging(attacker), "debug_ai", ("AggroListPropertyNamespace::canAttackTarget() attacker(%s) target(%s) Target is dead", attacker.getDebugInformation().c_str(), target.getDebugInformation().c_str()));
		result = false;
	}
	else if (isInvulnerable(target))
	{
		//LOGC(AiLogManager::isLogging(attacker), "debug_ai", ("AggroListPropertyNamespace::canAttackTarget() attacker(%s) target(%s) Target is in combat", attacker.getDebugInformation().c_str(), target.getDebugInformation().c_str()));
		result = false;
	}
	else if (!Pvp::canAttack(attacker, target))
	{
		//LOGC(AiLogManager::isLogging(attacker), "debug_ai", ("AggroListPropertyNamespace::canAttackTarget() attacker(%s) target(%s) Target is an invalid pvp target", attacker.getDebugInformation().c_str(), target.getDebugInformation().c_str()));
		result = false;
	}
	else if (isGm(target))
	{
		//LOGC(AiLogManager::isLogging(attacker), "debug_ai", ("AggroListPropertyNamespace::canAttackTarget() attacker(%s) target(%s) Target is a GM", attacker.getDebugInformation().c_str(), target.getDebugInformation().c_str()));
		result = false;
	}
	else if (isFeigningDeath(target))
	{
		//LOGC(AiLogManager::isLogging(attacker), "debug_ai", ("AggroListPropertyNamespace::canAttackTarget() attacker(%s) target(%s) Target is feigning death", attacker.getDebugInformation().c_str(), target.getDebugInformation().c_str()));
		result = false;

	}
	else if (isInPlayerBuilding(target))
	{
		//LOGC(AiLogManager::isLogging(attacker), "debug_ai", ("AggroListPropertyNamespace::canAttackTarget() attacker(%s) target(%s) Target is in a player building", attacker.getDebugInformation().c_str(), target.getDebugInformation().c_str()));
		result = false;
	}
	else if (isAggroImmune(target))
	{
		//LOGC(AiLogManager::isLogging(attacker), "debug_ai", ("AggroListPropertyNamespace::canAttackTarget() attacker(%s) target(%s) Target is aggro immune", attacker.getDebugInformation().c_str(), target.getDebugInformation().c_str()));
		result = false;
	}
    else if (!attacker.checkLOSTo(target))
	{
		//LOGC(AiLogManager::isLogging(attacker), "debug_ai", ("AggroListPropertyNamespace::canAttackTarget() attacker(%s) target(%s) No LOS to target", attacker.getDebugInformation().c_str(), target.getDebugInformation().c_str()));
		result = false;
	}

	// We can attack!

	return result;
}

// ----------------------------------------------------------------------
bool AggroListPropertyNamespace::isIncapacitated(TangibleObject const & target)
{
	CreatureObject const * const targetCreatureObject = target.asCreatureObject();

	return (targetCreatureObject != nullptr) ? targetCreatureObject->isIncapacitated() : false;
}

// ----------------------------------------------------------------------
bool AggroListPropertyNamespace::isDead(TangibleObject const & target)
{
	CreatureObject const * const targetCreatureObject = target.asCreatureObject();

	return (targetCreatureObject != nullptr) ? targetCreatureObject->isDead() : false;
}

// ----------------------------------------------------------------------
bool AggroListPropertyNamespace::isInvulnerable(TangibleObject const & target)
{
	CreatureObject const * const targetCreatureObject = target.asCreatureObject();

	return (targetCreatureObject != nullptr) ? targetCreatureObject->isInvulnerable() : false;
}

// ----------------------------------------------------------------------
bool AggroListPropertyNamespace::isGm(TangibleObject const & target)
{
	int temp = 0;

	return target.getObjVars().getItem("gm", temp) && (temp > 0);
}

// ----------------------------------------------------------------------
bool AggroListPropertyNamespace::isFeigningDeath(TangibleObject const & target)
{
	CreatureObject const * const targetCreatureObject = target.asCreatureObject();

	return (targetCreatureObject != nullptr) ? targetCreatureObject->getState(States::FeignDeath) : false;
}

// ----------------------------------------------------------------------
bool AggroListPropertyNamespace::isInPlayerBuilding(TangibleObject const & target)
{
	ServerObject const * const topMostServerObject = ServerObject::asServerObject(ContainerInterface::getTopmostContainer(target));

	return (topMostServerObject != nullptr) ? (topMostServerObject->getGameObjectType() == SharedObjectTemplate::GOT_building_player) : false;
}

// ----------------------------------------------------------------------
bool AggroListPropertyNamespace::isAggroImmune(TangibleObject const & target)
{
	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(target.asCreatureObject());

	return (playerObject != nullptr) ? playerObject->isAggroImmune() : false;
}

// ======================================================================
//
// AggroListProperty
//
// ======================================================================

// ----------------------------------------------------------------------
AggroListProperty::AggroListProperty(TangibleObject & owner)
 : Property(getClassPropertyId(), owner)
 , m_targetList()
 , m_aggroDistance(0.0f)
{
	setAggroDistance(ConfigServerGame::getAiBaseAggroRadius() + owner.getCollisionSphereExtent_o().getRadius());
}

// ----------------------------------------------------------------------
AggroListProperty::~AggroListProperty()
{
	clear();
}

// ----------------------------------------------------------------------
PropertyId AggroListProperty::getClassPropertyId()
{
	return PROPERTY_HASH(AggroListProperty, 1403544761);
}

// ----------------------------------------------------------------------
void AggroListProperty::addServerNpAutoDeltaVariables(Archive::AutoDeltaByteStream & stream)
{
	stream.addVariable(m_targetList);
}

// ----------------------------------------------------------------------
void AggroListProperty::alter()
{
	typedef std::vector<TargetList::const_iterator> PurgeList;
	static PurgeList purgeList;

	TangibleObject & tangibleOwner = getTangibleOwner();

	// Verify each entry in the list and see if we need to escalate it to the hate list
	// Once verified for the hate list, we need to give a warning to the target that the
	// aggro is about to occur

	TargetList::const_iterator iterTargetList = m_targetList.begin();

	for (; iterTargetList != m_targetList.end(); ++iterTargetList)
	{
		CachedNetworkId const & target = *iterTargetList;
		TangibleObject * const targetTangibleObject = TangibleObject::asTangibleObject(target.getObject());

		// First, list things that invalidate this target in the aggro list
		// Second, list the things that promote the target to the hate list
		 
		if (target.getObject() == nullptr)
		{
			purgeList.push_back(iterTargetList);
		}
		else if (tangibleOwner.getDistanceBetweenCollisionSpheres_w(*target.getObject()) > getAggroDistance())
		{
			purgeList.push_back(iterTargetList);
		}
		else if (canAttackTarget(tangibleOwner, *targetTangibleObject))
		{
			if (!tangibleOwner.addHate(target, 0.0f))
			{
				WARNING(true, ("AggroListProperty::alter() owner(%s) Trying to add a target to the hate list, but the hate list is rejecting it.", getOwner().getDebugInformation().c_str()));
			}

			purgeList.push_back(iterTargetList);
		}
	}

	// Purge the old items
	{
		PurgeList::const_iterator iterPurgeList(purgeList.begin());
		
		for (; iterPurgeList != purgeList.end(); ++iterPurgeList)
		{
			TargetList::const_iterator iter = *iterPurgeList;
			m_targetList.erase(iter);
		}

		purgeList.clear();
	}


}

// ----------------------------------------------------------------------
void AggroListProperty::addTarget(NetworkId const & target)
{
	if (target == getOwner().getNetworkId())
	{
		WARNING(true, ("AggroListProperty::addTarget() owner(%s) Owner trying to add hate to itself.", getOwner().getDebugInformation().c_str()));
	}
	else
	{
		m_targetList.insert(CachedNetworkId(target));
	}
}

// ----------------------------------------------------------------------
void AggroListProperty::removeTarget(NetworkId const & target)
{
	TargetList::const_iterator iterAggroListProperty = m_targetList.find(CachedNetworkId(target));

	if (iterAggroListProperty != m_targetList.end())
	{
		m_targetList.erase(iterAggroListProperty);
	}
	else
	{
		DEBUG_WARNING(true, ("AggroListProperty::removeTarget() owner(%s auth=%s) Unable to find the target(%s) in the hate list.", getOwner().getDebugInformation().c_str(), (getOwner().isAuthoritative() ? "yes" : "no"), target.getValueString().c_str()));
	}
}

// ----------------------------------------------------------------------
void AggroListProperty::clear()
{
	m_targetList.clear();
}

// ----------------------------------------------------------------------
bool AggroListProperty::isEmpty() const
{
	return getTargetList().empty();
}

// ----------------------------------------------------------------------
AggroListProperty::TargetList const & AggroListProperty::getTargetList() const
{
	return m_targetList.get();
}

// ----------------------------------------------------------------------
void AggroListProperty::setAggroDistance(float aggroDistance)
{
	m_aggroDistance = clamp(0.0f, aggroDistance, ConfigServerGame::getAiMaxAggroRadius());
}

// ----------------------------------------------------------------------
float AggroListProperty::getAggroDistance() const
{
	return m_aggroDistance.get();
}

// ----------------------------------------------------------------------
TangibleObject & AggroListProperty::getTangibleOwner()
{
	return *TangibleObject::asTangibleObject(&getOwner());
}

// ----------------------------------------------------------------------
AggroListProperty * AggroListProperty::getAggroListProperty(Object & object)
{
	Property * const property = object.getProperty(getClassPropertyId());
	AggroListProperty * const aggroProperty = (property != nullptr) ? static_cast<AggroListProperty *>(property) : nullptr;

	return aggroProperty;
}

// ======================================================================
