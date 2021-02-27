// ======================================================================
//
// ConsoleCommandParserMoney.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserMoney.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedObject/NetworkIdManager.h"
#include "UnicodeUtils.h"

#include <stdio.h>

// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"balance",1,"<oid>","Show how much money an object has (cash and bank)."},
	{"bankTransfer",3,"<source oid> <target oid> <amount>","Transfers bank credits from one object to another."},
	{"cashTransfer",3,"<source oid> <target oid> <amount>","Transfers cash from one object to another."},
	{"deposit",2,"<oid> <amount>","Deposit money to the bank."},
	{"namedTransfer",3,"<source oid> <target account> <amount>","Transfers bank credits from an object to a named account."},
	{"withdraw",2,"<oid> <amount>","Withdraw money from the bank."},
	{"setGalacticReserve",2,"<oid> <amount>","Sets the galactic reserve balance (in billion of credits) for the specified player character object."},
	{"", 0, "", ""} // this must be last
};


//-----------------------------------------------------------------

ConsoleCommandParserMoney::ConsoleCommandParserMoney (void) :
CommandParser ("money", 0, "...", "Money related commands.", 0)
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------


bool ConsoleCommandParserMoney::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	NOT_NULL (node);
	UNREF (userId);

	UNREF(originalCommand);

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

	//-----------------------------------------------------------------

	if (isAbbrev( argv [0], "cashTransfer"))
	{
		NetworkId sourceId(Unicode::wideToNarrow (argv[1]));
		NetworkId targetId(Unicode::wideToNarrow (argv[2]));
		int amount(strtoul(Unicode::wideToNarrow (argv[3]).c_str (), nullptr, 10));

		ServerObject* source = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(sourceId));
		if (!source)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}
		if (source->transferCashTo(targetId,amount))
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_INVALID_CONTAINER_TRANSFER);
	}
	else if (isAbbrev( argv [0], "bankTransfer"))
	{
		NetworkId sourceId(Unicode::wideToNarrow (argv[1]));
		NetworkId targetId(Unicode::wideToNarrow (argv[2]));
		int amount(strtoul(Unicode::wideToNarrow (argv[3]).c_str (), nullptr, 10));

		ServerObject* source = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(sourceId));
		if (!source)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}
		if (source->transferBankCreditsTo(targetId,amount))
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_INVALID_CONTAINER_TRANSFER);
	}
	else if (isAbbrev( argv [0], "namedTransfer"))
	{
		NetworkId sourceId(Unicode::wideToNarrow (argv[1]));
		int amount(strtol(Unicode::wideToNarrow (argv[3]).c_str (), nullptr, 10));

		ServerObject* source = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(sourceId));
		if (!source)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}
		if (amount > 0)
		{
			if (source->transferBankCreditsTo(Unicode::wideToNarrow(argv[2]),amount))
				result += getErrorMessage(argv[0], ERR_SUCCESS);
			else
				result += getErrorMessage(argv[0], ERR_INVALID_CONTAINER_TRANSFER);
		}
		else
		{
			if (source->transferBankCreditsFrom(Unicode::wideToNarrow(argv[2]),-amount))
				result += getErrorMessage(argv[0], ERR_SUCCESS);
			else
				result += getErrorMessage(argv[0], ERR_INVALID_CONTAINER_TRANSFER);
		}	
	}
	else if (isAbbrev( argv [0], "withdraw"))
	{
		NetworkId sourceId(Unicode::wideToNarrow (argv[1]));
		int amount(strtoul(Unicode::wideToNarrow (argv[2]).c_str (), nullptr, 10));

		ServerObject* source = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(sourceId));
		if (!source)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}
		if (source->withdrawCashFromBank(amount))
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_INVALID_CONTAINER_TRANSFER);
	}
	else if (isAbbrev( argv [0], "deposit"))
	{
		NetworkId sourceId(Unicode::wideToNarrow (argv[1]));
		int amount(strtoul(Unicode::wideToNarrow (argv[2]).c_str (), nullptr, 10));

		ServerObject* source = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(sourceId));
		if (!source)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}
		if (source->depositCashToBank(amount))
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_INVALID_CONTAINER_TRANSFER);
	}
	else if (isAbbrev( argv [0], "balance"))
	{
		NetworkId objectId(Unicode::wideToNarrow (argv[1]));
		ServerObject const * const object = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(objectId));
		if (!object)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}

		char buffer[30];
		sprintf(buffer,"%i",object->getCashBalance());
		result += Unicode::narrowToWide("Cash ") + Unicode::narrowToWide(buffer) + Unicode::narrowToWide("\n");
		sprintf(buffer,"%i",object->getBankBalance());
		result += Unicode::narrowToWide("Bank ") + Unicode::narrowToWide(buffer) + Unicode::narrowToWide("\n");
		sprintf(buffer,"%i",object->getTotalMoney());
		result += Unicode::narrowToWide("Total ") + Unicode::narrowToWide(buffer) + Unicode::narrowToWide("\n");

		PlayerObject const * const po = PlayerCreatureController::getPlayerObject(object->asCreatureObject());
		if (po)
		{
			sprintf(buffer,"%i",static_cast<int>(po->getGalacticReserveDeposit()));
			result += Unicode::narrowToWide("Galactic Reserve ") + Unicode::narrowToWide(buffer) + Unicode::narrowToWide("\n");
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev( argv [0], "setGalacticReserve"))
	{
		int const newGalacticReserve(strtoul(Unicode::wideToNarrow (argv[2]).c_str (), nullptr, 10));
		if ((newGalacticReserve < 0) || (newGalacticReserve > ConfigServerGame::getMaxGalacticReserveDepositBillion()))
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("specified galactic reserve balance (%d) must be in the (inclusive) range (0, %d)\n", newGalacticReserve, ConfigServerGame::getMaxGalacticReserveDepositBillion()));
			return true;
		}

		NetworkId const id(Unicode::wideToNarrow (argv[1]));
		ServerObject * const so = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(id));
		if (!so)
		{
			result += getErrorMessage (argv [0], ERR_INVALID_OBJECT);
			return true;
		}

		PlayerObject * const po = PlayerCreatureController::getPlayerObject(so->asCreatureObject());
		if (!po)
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("object (%s) is not a player character object\n", so->getNetworkId().getValueString().c_str()));
			return true;
		}

		if (!so->isAuthoritative() || !po->isAuthoritative())
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("object (%s) is not authoritative\n", so->getNetworkId().getValueString().c_str()));
			return true;
		}

		int currentGalacticReserve = 0;
		if (!so->getObjVars().getItem("galactic_reserve", currentGalacticReserve))
		{
			currentGalacticReserve = 0;
		}

		if (currentGalacticReserve != newGalacticReserve)
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("modifying player character object (%s) galactic reserve balance from (%d) to (%d)\n", so->getNetworkId().getValueString().c_str(), currentGalacticReserve, newGalacticReserve));

			if (newGalacticReserve > 0)
				so->setObjVarItem("galactic_reserve", newGalacticReserve);
			else
				so->removeObjVarItem("galactic_reserve");

			po->depersistGalacticReserveDeposit();
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("player character object (%s) already has a galactic reserve balance of (%d)\n", so->getNetworkId().getValueString().c_str(), newGalacticReserve));
		}
	}
	else
	{
		result += getErrorMessage(argv[0], ERR_NO_HANDLER);
	}

	return true;
}	// ConsoleCommandParserMoney::performParsing

// ======================================================================
