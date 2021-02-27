// ======================================================================
//
// ConsoleCommandParserSpaceAi.cpp
// Copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserSpaceAi.h"

#include "serverGame/AiServerShipObjectInterface.h"
#include "serverGame/AiShipBehaviorAttackFighter.h"
#include "serverGame/AiShipBehaviorAttackFighter_Maneuver.h"
#include "serverGame/AiShipBehaviorBase.h"
#include "serverGame/AiShipController.h"
#include "serverGame/AiShipPilotData.h"
#include "serverGame/Chat.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/SpacePath.h"
#include "serverGame/SpaceAttackSquad.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/AiDebugString.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/NetworkIdManager.h"
#include "UnicodeUtils.h"

// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"war", 0, "", "Sets all AI to resume attacking"},
	{"peace", 0, "", "Sets all AI to stop attacking."},
	{"path", 1, "", "Sends the pathing points for the specified AI to the client."},
	{"reloaddata", 0, "", "Reloads pilottype.tab"},
#ifdef _DEBUG
	{"clientDebug", 0, "", "Toggles client AI debugging."},
	{"serverDebug", 0, "", "Toggles the server to send AI debugging."},
	{"clientDebugText", 0, "", "Toggles the client AI debugging text."},
	{"maneuver", 1, "", "Forces the target ship to use evade, leash, or attack maneuvers."},
	{"fastAxis", 0, "", "Ships rotate toward the target along the fast axis of rotation."},
	//{"logall", 0, "", "Logs all AI to the server output log file."},
	//{"log", 0, "", "Logs the specified objectIds to the server output log file."},
#endif // _DEBUG
	{"", 0, "", ""} // this must be last
};

//-----------------------------------------------------------------

ConsoleCommandParserSpaceAi::ConsoleCommandParserSpaceAi()
 : CommandParser("spaceai", 0, "...", "Space Ai related commands.", 0)
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------

bool ConsoleCommandParserSpaceAi::performParsing(const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
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

	if (isAbbrev(argv[0], "war"))
	{
		AiShipController::setAttackingEnabled(true);

		Object * const object = NetworkIdManager::getObjectById(userId);
		ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;
		
		if (serverObject != nullptr)
		{
			Chat::sendSystemMessage(*serverObject, Unicode::narrowToWide("[WAR] AI will now attack."), Unicode::emptyString);
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "peace"))
	{
		AiShipController::setAttackingEnabled(false);

		Object * const object = NetworkIdManager::getObjectById(userId);
		ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;
		
		if (serverObject != nullptr)
		{
			Chat::sendSystemMessage(*serverObject, Unicode::narrowToWide("[PEACE] AI will no longer attack."), Unicode::emptyString);
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "path"))
	{
		Object * const object = NetworkIdManager::getObjectById(userId);
		ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;

		if (   (serverObject != nullptr)
		    && (argv.size () > 1))
		{
			NetworkId networkId(Unicode::wideToNarrow(argv[1]));

			AiShipController * const aiShipController = AiShipController::getAiShipController(networkId);

			if (aiShipController != nullptr)
			{
				SpacePath * const spacePath = aiShipController->getPath();

				if (spacePath != nullptr)
				{
					SpacePath::TransformList const & transformList = spacePath->getTransformList();
					SpacePath::TransformList::const_iterator iterTransformList = transformList.begin();
					FormattedString<512> formattedString;
					int index = 1;
					std::string message;

					message += formattedString.sprintf("Path of size(%u) for unit(%s)\n", transformList.size(), networkId.getValueString().c_str());

					for (; iterTransformList != transformList.end(); ++iterTransformList)
					{
						Vector const & position_w = iterTransformList->getPosition_p();

						message += formattedString.sprintf("[%d] %.2f, %.2f, %.2f%s", (index+1), position_w.x, position_w.y, position_w.z, (index < static_cast<int>(transformList.size())) ? "\n" : "");
						index++;
					}

					Chat::sendSystemMessage(*serverObject, Unicode::narrowToWide(message.c_str()), Unicode::emptyString);
				}
				else
				{
					Chat::sendSystemMessage(*serverObject, Unicode::narrowToWide("nullptr path"), Unicode::emptyString);
				}
			}
			else
			{
				Chat::sendSystemMessage(*serverObject, Unicode::narrowToWide("[ERROR] Unable to resolve the obj_id for a path."), Unicode::emptyString);
			}
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "reloaddata"))
	{
		Object * const object = NetworkIdManager::getObjectById(userId);
		ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;

		if (serverObject != nullptr)
		{
			AiShipPilotData::reload();

			Chat::sendSystemMessage(*serverObject, Unicode::narrowToWide("[RELOAD DATA] pilottype.tab was reloaded"), Unicode::emptyString);
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
#ifdef _DEBUG
	else if (isAbbrev(argv[0], "clientDebug"))
	{
		Object * const object = NetworkIdManager::getObjectById(userId);
		ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;
		int temp;
		
		if (serverObject != nullptr)
		{
		    if (   serverObject->getObjVars().getItem("ai_debug_string", temp)
		        && (temp > 0))
			{
				if (!AiShipController::isClientDebugEnabled())
				{
					AiShipController::setClientDebugEnabled(true);

					Chat::sendSystemMessage(*serverObject, Unicode::narrowToWide("[ENABLED] Your client is requesting ai debugging."), Unicode::emptyString);
				}
				else
				{
					IGNORE_RETURN(serverObject->setObjVarItem("ai_debug_string", 0));

					Chat::sendSystemMessage(*serverObject, Unicode::narrowToWide("[DISABLED] Your client is NO LONGER requesting ai debugging."), Unicode::emptyString);

					// Tell the client to remove all ai debugging

					Client * const client = serverObject->getClient();

					if (client != nullptr)
					{
						AiDebugString aiDebugString;
						aiDebugString.enableClearClientFlag();
						bool const reliable = true;
						GenericValueTypeMessage<std::pair<NetworkId, std::string> > message("AiDebugString", std::make_pair(serverObject->getNetworkId(), aiDebugString.toString()));
					
						client->send(message, reliable);
					}
				}
			}
			else
			{
				IGNORE_RETURN(serverObject->setObjVarItem("ai_debug_string", 1));

				Chat::sendSystemMessage(*serverObject, Unicode::narrowToWide("[ENABLED] Your client is requesting ai debugging."), Unicode::emptyString);

				if (!AiShipController::isClientDebugEnabled())
				{
					AiShipController::setClientDebugEnabled(true);
				}
			}
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "serverDebug"))
	{
		Object * const object = NetworkIdManager::getObjectById(userId);
		ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;

		if (serverObject != nullptr)
		{
			AiShipController::setClientDebugEnabled(!AiShipController::isClientDebugEnabled());
			
			Unicode::String systemMessage;
			systemMessage = Unicode::narrowToWide(AiShipController::isClientDebugEnabled() ? "[ENABLED] Server is sending debug ai to requesting clients." : "[DISABLED] Server is NO LONGER sending debug ai to requesting clients.");
			Chat::sendSystemMessage(*serverObject, systemMessage, Unicode::emptyString);
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "clientDebugText"))
	{
		Object * const object = NetworkIdManager::getObjectById(userId);
		ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;

		if (serverObject != nullptr)
		{
			AiDebugString::setTextEnabled(!AiDebugString::isTextEnabled());
			
			Unicode::String systemMessage;
			systemMessage = Unicode::narrowToWide(AiDebugString::isTextEnabled() ? "[ENABLED] Client-side debug text is enabled." : "[DISABLED] Client-side debug text is disabled.");
			Chat::sendSystemMessage(*serverObject, systemMessage, Unicode::emptyString);
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "maneuver"))
	{
		Object * const object = NetworkIdManager::getObjectById(userId);
		ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;
		CreatureObject const * const creatureObject = serverObject ? serverObject->asCreatureObject() : nullptr;

		if (serverObject)
		{
			if ((creatureObject != nullptr) && (argv.size () > 1))
			{
				AiShipController const * const lookAtAiShipController = AiShipController::getAiShipController(creatureObject->getLookAtTarget());
				
				if (lookAtAiShipController)
				{
					SpaceAttackSquad const & attackSquad = lookAtAiShipController->getAttackSquad();
					AiShipController * const squadLeaderAiShipController = AiShipController::getAiShipController(attackSquad.getLeader());

					bool success = false;
					
					if (isAbbrev(argv[1], "evade"))
					{
						success = squadLeaderAiShipController->debug_forceFighterAttackManeuver(static_cast<int>(AiShipBehaviorAttackFighter::Maneuver::FM_evade));
					}
					else if (isAbbrev(argv[1], "chase"))
					{
						success = squadLeaderAiShipController->debug_forceFighterAttackManeuver(static_cast<int>(AiShipBehaviorAttackFighter::Maneuver::FM_chase));
					}
					else if (isAbbrev(argv[1], "leash"))
					{
						success = squadLeaderAiShipController->debug_forceFighterAttackManeuver(static_cast<int>(AiShipBehaviorAttackFighter::Maneuver::FM_leash));
					}
					else if (isAbbrev(argv[1], "none"))
					{
						success = squadLeaderAiShipController->debug_forceFighterAttackManeuver(static_cast<int>(AiShipBehaviorAttackFighter::Maneuver::FM_none));
					}
					else
					{
						Chat::sendSystemMessage(*serverObject, Unicode::narrowToWide("Valid parameters are: [evade], [attack] or [chase], [leash], and [none]."), Unicode::emptyString);
					}

					if (!success)
					{
						Chat::sendSystemMessage(*serverObject, Unicode::narrowToWide("Failed to set the maneuver on the target. The target must be in the AiShipBehaviorAttackFighter state. This fails on a bomber or capship behavior."), Unicode::emptyString);
					}
				}
				else
				{
					Chat::sendSystemMessage(*serverObject, Unicode::narrowToWide("Incorrect number of arguments."), Unicode::emptyString);
				}
			}
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "fastAxis"))
	{
		Object * const object = NetworkIdManager::getObjectById(userId);
		ServerObject * const serverObject = (object != nullptr) ? object->asServerObject() : nullptr;
		
		if (serverObject != nullptr)
		{
			Unicode::String systemMessage;

			AiServerShipObjectInterface::setFastAxisEnabled(!AiServerShipObjectInterface::isFastAxisEnabled());

			systemMessage += Unicode::narrowToWide(AiServerShipObjectInterface::isFastAxisEnabled() ? "[ENABLED] AI ships now use the fast axis." : "[DISABLED] AI ships are no longer using the fast axis.");
			
			Chat::sendSystemMessage(*serverObject, systemMessage, Unicode::emptyString);
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
#endif //_DEBUG
	else
	{
		result += getErrorMessage(argv[0], ERR_NO_HANDLER);
	}

	return true;
}

// ======================================================================
