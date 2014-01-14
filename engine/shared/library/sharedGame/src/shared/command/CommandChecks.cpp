// ======================================================================
//
// CommandChecks.cpp
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/CommandChecks.h"

#include "sharedGame/CombatDataTable.h"
#include "sharedGame/Command.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "swgSharedUtility/Locomotions.h"

// ======================================================================

void CommandChecks::getRangeForCommand(Command const * command, float minWeaponRange, float maxWeaponRange, bool weaponCheckLast, float & min, float & max)
{
	// DEFAULT: weapon range
	if (!weaponCheckLast && (minWeaponRange > 0.0f || maxWeaponRange > 0.0f))
	{ 
		min = minWeaponRange;
		max = maxWeaponRange;
	}

	// OVERRIDDEN BY: command_table
	if (command->m_maxRangeToTarget > 0.0f)
	{
		min = -1;
		max = command->m_maxRangeToTarget;
	}

	// OVERRIDDEN BY: combat_data table 
	const float minDist = CombatDataTable::getMinRange(command->m_commandHash);
	const float maxDist = CombatDataTable::getMaxRange(command->m_commandHash);
	if (minDist > 0.0f || maxDist > 0.0f) 
	{
		min = minDist;
		max = maxDist;
	}

	// OVERRIDDEN BY: optionally the weapon check can be last and most authoratative check...
	if (weaponCheckLast && (minWeaponRange > 0.0f || maxWeaponRange > 0.0f))
	{ 
		min = minWeaponRange;
		max = maxWeaponRange;
	}
}

//-----------------------------------------------------------------------

bool CommandChecks::isLocomotionValidForCommand(Command const * command, Locomotions::Enumerator locomotion, float speed)
{
	// if you are not allowed to run, still allow the command if you are running slowly enough
	if(locomotion == Locomotions::Running && !command->m_locomotionPermissions[Locomotions::Running])
	{
		return (speed < ConfigSharedGame::getMovementThresholdForActionValidation());
	}

	return command->m_locomotionPermissions[locomotion];
}

//-----------------------------------------------------------------------

bool CommandChecks::isEnoughActionPointsForCommand(Command const * command, int currentActionPoints)
{
	bool result = false;
	
	float actionCost = CombatDataTable::getActionCost(command->m_commandHash);

	if (actionCost <= currentActionPoints)
		result = true;

	return result;
}

//-----------------------------------------------------------------------

bool CommandChecks::isEnoughMindPointsForCommand(Command const * command, int currentMindPoints)
{
	bool result = false;
	
	float mindCost = CombatDataTable::getMindCost(command->m_commandHash);

	if (mindCost <= currentMindPoints)
		result = true;

	return result;
}

// ======================================================================

bool CommandChecks::isMob(NetworkId const & targetId)
{
	bool isMob = false;

	Object * targetObj = NetworkIdManager::getObjectById(targetId);
	if (targetObj)
	{
		SharedCreatureObjectTemplate const * sharedTemplate = dynamic_cast<SharedCreatureObjectTemplate const *>(targetObj->getObjectTemplate());
		if (sharedTemplate)
		{
			isMob = true;
		}
	}

	return isMob;
}

//-----------------------------------------------------------------------

bool CommandChecks::isVehicle(NetworkId const & targetId)
{
	bool isVehicle = false;

	Object * targetObj = NetworkIdManager::getObjectById(targetId);
	if (targetObj)
	{
		SharedObjectTemplate const * sharedTemplate = dynamic_cast<SharedObjectTemplate const *>(targetObj->getObjectTemplate());
		if (sharedTemplate)
		{
			isVehicle = GameObjectTypes::isTypeOf(sharedTemplate->getGameObjectType(), SharedObjectTemplate::GOT_vehicle);
		}
	}

	return isVehicle;
}

//-----------------------------------------------------------------------

bool CommandChecks::isMonster(NetworkId const & targetId)
{
	bool isMonster = false;

	Object * targetObj = NetworkIdManager::getObjectById(targetId);
	if (targetObj)
	{
		SharedCreatureObjectTemplate const * sharedTemplate = dynamic_cast<SharedCreatureObjectTemplate const *>(targetObj->getObjectTemplate());
		SharedCreatureObjectTemplate::Niche niche = sharedTemplate->getNiche();
		if (niche == SharedCreatureObjectTemplate::NI_monster
			|| niche == SharedCreatureObjectTemplate::NI_herbivore
			|| niche == SharedCreatureObjectTemplate::NI_carnivore
			|| niche == SharedCreatureObjectTemplate::NI_predator)
		{
			isMonster = true;
		}
	}

	return isMonster;
}

//-----------------------------------------------------------------------

bool CommandChecks::isNpc(NetworkId const & targetId)
{
	bool isNPC = false;

	Object * targetObj = NetworkIdManager::getObjectById(targetId);
	if (targetObj)
	{
		SharedCreatureObjectTemplate const * sharedTemplate = dynamic_cast<SharedCreatureObjectTemplate const *>(targetObj->getObjectTemplate());
		SharedCreatureObjectTemplate::Niche niche = sharedTemplate->getNiche();
		if (niche == SharedCreatureObjectTemplate::NI_npc)
		{
			isNPC = true;
		}
	}

	return isNPC;
}

//-----------------------------------------------------------------------

bool CommandChecks::isDroid(NetworkId const & targetId)
{
	bool isDroid = false;

	Object * targetObj = NetworkIdManager::getObjectById(targetId);
	if (targetObj)
	{
		SharedCreatureObjectTemplate const * sharedTemplate = dynamic_cast<SharedCreatureObjectTemplate const *>(targetObj->getObjectTemplate());
		SharedCreatureObjectTemplate::Niche niche = sharedTemplate->getNiche();
		if (niche == SharedCreatureObjectTemplate::NI_droid)
		{
			isDroid = true;
		}
	}

	return isDroid;
}

// ======================================================================

