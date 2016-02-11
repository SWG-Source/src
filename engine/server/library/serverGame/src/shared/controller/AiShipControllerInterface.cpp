// ======================================================================
//
// AiShipControllerInterface.cpp
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipControllerInterface.h"

#include "serverGame/AiShipController.h"
#include "serverGame/ConfigServerGame.h"
#include "sharedLog/Log.h"

// ======================================================================
//
// AiShipControllerInterface
//
// ======================================================================

// ----------------------------------------------------------------------
bool AiShipControllerInterface::addDamageTaken(NetworkId const & unit, NetworkId const & attackingUnit, float const damage, bool const notifySquad, bool const checkPlayerAttacker)
{
	bool result = false;
	AiShipController * const aiShipController = AiShipController::getAiShipController(unit);

	if (aiShipController != nullptr)
	{
		bool const verifyAttacker = false;

		result = aiShipController->addDamageTaken(attackingUnit, damage, verifyAttacker, notifySquad, checkPlayerAttacker);
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipControllerInterface::addDamageTaken() ERROR: Unable to resolve networkId(%s) to AiShipController.", unit.getValueString().c_str()));
	}

	return result;
}

// ----------------------------------------------------------------------
bool AiShipControllerInterface::setAttackOrders(NetworkId const & unit, AiShipController::AttackOrders const attackOrders)
{
	bool result = false;
	AiShipController * const aiShipController = AiShipController::getAiShipController(unit);

	if (aiShipController != nullptr)
	{
		result = true;
		aiShipController->setAttackOrders(attackOrders);
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipControllerInterface::setAttackOrders() ERROR: Unable to resolve networkId(%s) to AiShipController.", unit.getValueString().c_str()));
	}

	return result;
}

// ----------------------------------------------------------------------
bool AiShipControllerInterface::idle(NetworkId const & unit)
{
	bool result = false;
	AiShipController * const aiShipController = AiShipController::getAiShipController(unit);

	if (aiShipController != nullptr)
	{
		result = true;
		aiShipController->idle();
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipControllerInterface::idle() ERROR: Unable to resolve networkId(%s) to AiShipController.", unit.getValueString().c_str()));
	}

	return result;
}

// ----------------------------------------------------------------------
bool AiShipControllerInterface::track(NetworkId const & unit, Object const & target)
{
	bool result = false;
	AiShipController * const aiShipController = AiShipController::getAiShipController(unit);

	if (aiShipController != nullptr)
	{
		result = true;
		aiShipController->track(target);
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipControllerInterface::idle() ERROR: Unable to resolve networkId(%s) to AiShipController.", unit.getValueString().c_str()));
	}

	return result;
}

// ----------------------------------------------------------------------
bool AiShipControllerInterface::setLeashRadius(NetworkId const & unit, float const radius)
{
	bool result = false;
	AiShipController * const aiShipController = AiShipController::getAiShipController(unit);

	if (aiShipController != nullptr)
	{
		result = true;
		aiShipController->setLeashRadius(radius);
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipControllerInterface::setLeashRadius() ERROR: Unable to resolve networkId(%s) to AiShipController.", unit.getValueString().c_str()));
	}

	return result;
}

// ----------------------------------------------------------------------
bool AiShipControllerInterface::follow(NetworkId const & unit, NetworkId const & followedUnit, Vector const & direction_o, float const direction)
{
	bool result = false;
	AiShipController * const aiShipController = AiShipController::getAiShipController(unit);

	if (aiShipController != nullptr)
	{
		result = true;
		aiShipController->follow(followedUnit, direction_o, direction);
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipControllerInterface::follow() ERROR: Unable to resolve networkId(%s) to AiShipController.", unit.getValueString().c_str()));
	}

	return result;
}

// ----------------------------------------------------------------------
bool AiShipControllerInterface::addPatrolPath(NetworkId const & unit, SpacePath * const path)
{
	bool result = false;
	AiShipController * const aiShipController = AiShipController::getAiShipController(unit);

	if (aiShipController != nullptr)
	{
		result = true;
		aiShipController->addPatrolPath(path);
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipControllerInterface::addPatrolPath() ERROR: Unable to resolve networkId(%s) to AiShipController.", unit.getValueString().c_str()));
	}

	return result;
}

// ----------------------------------------------------------------------
bool AiShipControllerInterface::clearPatrolPath(NetworkId const & unit)
{
	bool result = false;
	AiShipController * const aiShipController = AiShipController::getAiShipController(unit);

	if (aiShipController != nullptr)
	{
		result = true;
		aiShipController->clearPatrolPath();
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipControllerInterface::clearPatrolPath() ERROR: Unable to resolve networkId(%s) to AiShipController.", unit.getValueString().c_str()));
	}

	return result;
}

// ----------------------------------------------------------------------
bool AiShipControllerInterface::moveTo(NetworkId const & unit, SpacePath * const path)
{
	bool result = false;
	AiShipController * const aiShipController = AiShipController::getAiShipController(unit);

	if (aiShipController != nullptr)
	{
		result = true;
		aiShipController->moveTo(path);
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("AiShipControllerInterface::moveTo() ERROR: Unable to resolve networkId(%s) to AiShipController.", unit.getValueString().c_str()));
	}

	return result;
}

// ======================================================================
