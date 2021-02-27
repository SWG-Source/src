// ======================================================================
//
// ConsoleCommandParserAi.cpp
// Copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserAi.h"

#include "serverGame/AiCreatureController.h"
#include "serverGame/AiShipController.h"
#include "serverGame/Chat.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/HateList.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/WeaponObject.h"
#include "serverUtility/ServerClock.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/AiDebugString.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "UnicodeUtils.h"

// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"info", 0, "[oid]", "Displays information about the specified AI. Assumes target if no oid specified."},
	{"hatelist", 0, "[oid]", "Displays the specified object's hate list. Assumes target if no oid specified."},
	{"kill", 0, "[oid]", "Kills the specified AI. Assumes target if no oid specified."},
	{"log", 0, "[oid]", "Toggles log file data for the specified AI. Assumes target if no oid specified."},
	{"retreat", 0, "[oid]", "Toggles whether the specified AI can auto retreat due to a player running away without attacking. Assumes target if no oid specified."},
	{"walkspeed", 1, "<speed | oid speed>", "Displays the current walk speed or sets the walk speed for the specified AI. Assumes target if no oid specified."},
	{"runspeed", 1, "<speed | oid speed>", "Displays the current run speed or sets the run speed for the specified AI. Assumes target if no oid specified."},
#ifdef _DEBUG
	{"killall", 0, "", "Kills all the AI on the server."},
	{"debug", 0, "", "Toggles client AI debugging."},
	{"debugText", 0, "", "Toggles the client AI debugging text."},
#endif // _DEBUG
	{"", 0, "", ""} // this must be last
};

// ======================================================================
//
// ConsoleCommandParserAiNamespace
//
// ======================================================================

namespace ConsoleCommandParserAiNamespace
{
	CreatureObject * const getAiCreatureObject(CreatureObject const & user, std::vector<Unicode::String> const & argv);
}

//-----------------------------------------------------------------

CreatureObject * const ConsoleCommandParserAiNamespace::getAiCreatureObject(CreatureObject const & user, std::vector<Unicode::String> const & argv)
{
	NetworkId aiNetworkId;

	if (argv.size() > 1)
	{
		aiNetworkId = NetworkId(Unicode::wideToNarrow(argv[1]));
	}
	else
	{
		aiNetworkId = user.getLookAtTarget();
	}

	CreatureObject * const aiCreatureObject = CreatureObject::getCreatureObject(aiNetworkId);

	if (   (aiCreatureObject == nullptr)
		|| aiCreatureObject->isPlayerControlled())
	{
		Chat::sendSystemMessage(user, Unicode::narrowToWide(FormattedString<1024>().sprintf("[FAILURE] Unable to resolve %s to an AI", aiNetworkId.getValueString().c_str())), Unicode::emptyString);
	}

	return aiCreatureObject;
}

using namespace ConsoleCommandParserAiNamespace;

// ======================================================================
//
// ConsoleCommandParserAi
//
// ======================================================================

//-----------------------------------------------------------------

ConsoleCommandParserAi::ConsoleCommandParserAi()
 : CommandParser("ai", 0, "...", "Ai related commands.", 0)
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------

bool ConsoleCommandParserAi::performParsing(const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
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

	FormattedString<1024> fs;

	if (isAbbrev(argv[0], "log"))
	{
		CreatureObject * const userCreatureObject = CreatureObject::getCreatureObject(userId);

		if (userCreatureObject == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
		{
			CreatureObject * const aiCreatureObject = getAiCreatureObject(*userCreatureObject, argv);

			if (aiCreatureObject == nullptr)
			{
				result += getErrorMessage(argv[0], ERR_FAIL);
			}
			else
			{
				AICreatureController * const aiCreatureController = AICreatureController::asAiCreatureController(aiCreatureObject->getController());

				if (aiCreatureController == nullptr)
				{
					Chat::sendSystemMessage(*userCreatureObject, Unicode::narrowToWide(fs.sprintf("[FAILURE] Unable to resolve %s to an AI", aiCreatureObject->getNetworkId().getValueString().c_str())), Unicode::emptyString);
					result += getErrorMessage(argv[0], ERR_FAIL);
				}
				else
				{
					aiCreatureController->setLogging(!aiCreatureController->isLogging());
					
					Chat::sendSystemMessage(*userCreatureObject, Unicode::narrowToWide(fs.sprintf("[%s] Ai logging %s", (aiCreatureController->isLogging() ? "ENABLED" : "DISABLED"), aiCreatureObject->getNetworkId().getValueString().c_str())), Unicode::emptyString);
					result += getErrorMessage(argv[0], ERR_SUCCESS);
				}
			}
		}
	}
	else if (isAbbrev(argv[0], "info"))
	{
		CreatureObject * const userCreatureObject = CreatureObject::getCreatureObject(userId);

		if (userCreatureObject == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
		{
			NetworkId targetNetworkId;

			if (argv.size () > 1)
			{
				targetNetworkId = NetworkId(Unicode::wideToNarrow(argv[1]));
			}
			else
			{
				targetNetworkId = userCreatureObject->getLookAtTarget();
			}

			AICreatureController * const aiCreatureController = AICreatureController::getAiCreatureController(targetNetworkId);

			if (aiCreatureController == nullptr)
			{
				result = Unicode::narrowToWide(fs.sprintf("[FAILURE] Unable to resolve %s to an AI\n", targetNetworkId.getValueString().c_str()));
				result += getErrorMessage(argv[0], ERR_FAIL);
			}
			else
			{
				CreatureObject const * const creatureOwner = aiCreatureController->getCreature();

				result += Unicode::narrowToWide(fs.sprintf("\nobj_id: %s\n", aiCreatureController->getCreature()->getNetworkId().getValueString().c_str()));
				result += Unicode::narrowToWide(fs.sprintf("creatureName: %s\n", aiCreatureController->getCreatureName().getString()));
				result += Unicode::narrowToWide(fs.sprintf("level: %i\n", aiCreatureController->getCreature()->getLevel()));

				// World position
				{
					Vector const & position_w = creatureOwner->getPosition_w();
					result += Unicode::narrowToWide(fs.sprintf("world position: pos(%.1f, %.1f, %.1f)\n", position_w.x, position_w.y, position_w.z));
				}

				// Cell position
				{
					CellProperty const * const cellProperty = creatureOwner->getParentCell();

					if (cellProperty != nullptr)
					{
						Vector const & position_p = creatureOwner->getPosition_p();

						result += Unicode::narrowToWide(fs.sprintf("cell location:  pos(%.1f, %.1f, %.1f) cell(%s:%s)\n", position_p.x, position_p.y, position_p.z, cellProperty->getOwner().getNetworkId().getValueString().c_str(), cellProperty->getCellName()));
					}
				}

				Location const & location = aiCreatureController->getHomeLocation();
				result += Unicode::narrowToWide(fs.sprintf("homeLocation: pos(%.1f, %.1f, %.1f) cell(%s:%s) scene(%s)\n", location.getCoordinates().x, location.getCoordinates().y, location.getCoordinates().z, location.getCell().getValueString().c_str(), (location.getCell() == NetworkId::cms_invalid) ? ":world" : "", location.getSceneId()));
				result += Unicode::narrowToWide(fs.sprintf("invulnerable: %s\n", aiCreatureController->getCreature()->isInvulnerable() ? "yes" : "no"));
				result += Unicode::narrowToWide(fs.sprintf("hibernating: %s\n", aiCreatureController->getHibernate() ? "yes" : "no"));
				result += Unicode::narrowToWide(fs.sprintf("movement state: %s\n", AiMovementBase::getMovementString(aiCreatureController->getMovementType())));
				result += Unicode::narrowToWide(fs.sprintf("walk speed(%.1f)\n", creatureOwner->getWalkSpeed()));
				result += Unicode::narrowToWide(fs.sprintf("run speed(%.1f) modifier(%.2f)\n", creatureOwner->getRunSpeed(), aiCreatureController->getMovementSpeedPercent()));

				if (aiCreatureController->isAggressive())
				{
					result += Unicode::narrowToWide(fs.sprintf("aggressive: yes @ %.1fm\n", aiCreatureController->getAggroRadius()));
				}
				else
				{
					result += Unicode::narrowToWide("aggressive: no\n");
				}
				if (aiCreatureController->isAssist())
				{
					result += Unicode::narrowToWide(fs.sprintf("assist: yes @ %.1fm\n", aiCreatureController->getAssistRadius()));
				}
				else
				{
					result += Unicode::narrowToWide("assist: no\n");
				}
				result += Unicode::narrowToWide(fs.sprintf("killer: %s\n", aiCreatureController->isKiller() ? "yes" : "no"));

				// Hate List
				{
					HateList::SortedList hateList;
					aiCreatureController->getCreature()->getSortedHateList(hateList);

					if (hateList.empty())
					{
						result += Unicode::narrowToWide(fs.sprintf("* NO HATE TARGETS *\n"));
					}
					else
					{
						result += Unicode::narrowToWide(fs.sprintf("* HATE LIST *\n"));
						
						HateList::SortedList::const_iterator iterHateList = hateList.begin();

						for (; iterHateList != hateList.end(); ++iterHateList)
						{
							CachedNetworkId const & hateTarget = iterHateList->first;
							TangibleObject const * const hateTargetTangibleObject = TangibleObject::asTangibleObject(hateTarget.getObject());
							float const hate = iterHateList->second;
							std::string hateTargetName;
							if (hateTargetTangibleObject != nullptr)
							{
								hateTargetName = Unicode::wideToNarrow(hateTargetTangibleObject->getEncodedObjectName()).c_str();
							}
							else
							{
								hateTargetName = "nullptr";
							}

							result += Unicode::narrowToWide(fs.sprintf("%s:%s(%.1f)\n", hateTargetName.c_str(), hateTarget.getValueString().c_str(), hate));
						}
					}
				}

				// Hate Over Time List
				{
					std::map<NetworkId, std::pair<float, std::pair<unsigned long, unsigned long> > > const & hateOverTime = aiCreatureController->getCreature()->getHateOverTime();
					if (!hateOverTime.empty())
					{
						result += Unicode::narrowToWide(fs.sprintf("* HATE OVER TIME LIST currentGameTime=(%lu)*\n", ServerClock::getInstance().getGameTimeSeconds()));

						for (std::map<NetworkId, std::pair<float, std::pair<unsigned long, unsigned long> > >::const_iterator iter = hateOverTime.begin(); iter != hateOverTime.end(); ++iter)
						{
							CachedNetworkId const hateTarget(iter->first);
							TangibleObject const * const hateTargetTangibleObject = TangibleObject::asTangibleObject(hateTarget.getObject());
							std::string hateTargetName;
							if (hateTargetTangibleObject != nullptr)
							{
								hateTargetName = Unicode::wideToNarrow(hateTargetTangibleObject->getEncodedObjectName()).c_str();
							}
							else
							{
								hateTargetName = "nullptr";
							}

							result += Unicode::narrowToWide(fs.sprintf("%s:%s (%.2f, %lu, %lu)\n", hateTargetName.c_str(), hateTarget.getValueString().c_str(), iter->second.first, iter->second.second.first, iter->second.second.second));
						}
					}
				}

				result += Unicode::narrowToWide(aiCreatureController->getCombatActionsString());

				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
		}
	}
	else if (isAbbrev(argv[0], "hatelist"))
	{
		CreatureObject * const userCreatureObject = CreatureObject::getCreatureObject(userId);

		if (userCreatureObject == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
		{
			NetworkId targetNetworkId;

			if (argv.size () > 1)
			{
				targetNetworkId = NetworkId(Unicode::wideToNarrow(argv[1]));
			}
			else
			{
				targetNetworkId = userCreatureObject->getLookAtTarget();
			}

			TangibleObject const * const to = TangibleObject::asTangibleObject(NetworkIdManager::getObjectById(targetNetworkId));
			if (to == nullptr)
			{
				result += Unicode::narrowToWide(fs.sprintf("Target %s not found or not TangibleObject\n", targetNetworkId.getValueString().c_str()));
			}
			else
			{
				result += Unicode::narrowToWide(fs.sprintf("Displaying hate list for %s\n", targetNetworkId.getValueString().c_str()));

				// Hate List
				{
					HateList::SortedList hateList;
					to->getSortedHateList(hateList);

					if (hateList.empty())
					{
						result += Unicode::narrowToWide(fs.sprintf("* NO HATE TARGETS *\n"));
					}
					else
					{
						result += Unicode::narrowToWide(fs.sprintf("* HATE LIST *\n"));

						HateList::SortedList::const_iterator iterHateList = hateList.begin();

						for (; iterHateList != hateList.end(); ++iterHateList)
						{
							CachedNetworkId const & hateTarget = iterHateList->first;
							TangibleObject const * const hateTargetTangibleObject = TangibleObject::asTangibleObject(hateTarget.getObject());
							float const hate = iterHateList->second;
							std::string hateTargetName;
							if (hateTargetTangibleObject != nullptr)
							{
								hateTargetName = Unicode::wideToNarrow(hateTargetTangibleObject->getEncodedObjectName()).c_str();
							}
							else
							{
								hateTargetName = "nullptr";
							}

							result += Unicode::narrowToWide(fs.sprintf("%s:%s(%.1f)\n", hateTargetName.c_str(), hateTarget.getValueString().c_str(), hate));
						}
					}
				}

				// Hate Over Time List
				{
					std::map<NetworkId, std::pair<float, std::pair<unsigned long, unsigned long> > > const & hateOverTime = to->getHateOverTime();
					if (!hateOverTime.empty())
					{
						result += Unicode::narrowToWide(fs.sprintf("* HATE OVER TIME LIST currentGameTime=(%lu)*\n", ServerClock::getInstance().getGameTimeSeconds()));

						for (std::map<NetworkId, std::pair<float, std::pair<unsigned long, unsigned long> > >::const_iterator iter = hateOverTime.begin(); iter != hateOverTime.end(); ++iter)
						{
							CachedNetworkId const hateTarget(iter->first);
							TangibleObject const * const hateTargetTangibleObject = TangibleObject::asTangibleObject(hateTarget.getObject());
							std::string hateTargetName;
							if (hateTargetTangibleObject != nullptr)
							{
								hateTargetName = Unicode::wideToNarrow(hateTargetTangibleObject->getEncodedObjectName()).c_str();
							}
							else
							{
								hateTargetName = "nullptr";
							}

							result += Unicode::narrowToWide(fs.sprintf("%s:%s (%.2f, %lu, %lu)\n", hateTargetName.c_str(), hateTarget.getValueString().c_str(), iter->second.first, iter->second.second.first, iter->second.second.second));
						}
					}
				}

				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
		}
	}
	else if (isAbbrev(argv[0], "kill"))
	{
		CreatureObject * const userCreatureObject = CreatureObject::getCreatureObject(userId);

		if (userCreatureObject == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
		{
			CreatureObject * const aiCreatureObject = getAiCreatureObject(*userCreatureObject, argv);

			if (aiCreatureObject == nullptr)
			{
				result += getErrorMessage(argv[0], ERR_FAIL);
			}
			else
			{
				aiCreatureObject->setIncapacitated(true, NetworkId::cms_invalid);

				Chat::sendSystemMessage(*userCreatureObject, Unicode::narrowToWide(fs.sprintf("[SUCCESS] AI %s terminated", aiCreatureObject->getNetworkId().getValueString().c_str())), Unicode::emptyString);

				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
		}
	}
	else if (isAbbrev(argv[0], "retreat"))
	{
		CreatureObject * const userCreatureObject = CreatureObject::getCreatureObject(userId);

		if (userCreatureObject == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
		{
			CreatureObject * const aiCreatureObject = getAiCreatureObject(*userCreatureObject, argv);

			if (aiCreatureObject == nullptr)
			{
				result += getErrorMessage(argv[0], ERR_FAIL);
			}
			else
			{
				aiCreatureObject->setHateListAutoExpireTargetEnabled(!aiCreatureObject->isHateListAutoExpireTargetEnabled());

				Chat::sendSystemMessage(*userCreatureObject, Unicode::narrowToWide(fs.sprintf("[SUCCESS] AI %s %s retreat", aiCreatureObject->getNetworkId().getValueString().c_str(), aiCreatureObject->isHateListAutoExpireTargetEnabled() ? "will now" : "will no longer")), Unicode::emptyString);

				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
		}
	}
	else if (isAbbrev(argv[0], "walkspeed"))
	{
		CreatureObject * const userCreatureObject = CreatureObject::getCreatureObject(userId);

		if (userCreatureObject == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
		{
			NetworkId aiNetworkId;
			CreatureObject * aiCreatureObject = nullptr;

			if (argv.size() > 2)
			{
				// Get the network id and speed

				aiNetworkId = NetworkId(Unicode::wideToNarrow(argv[1]));
				aiCreatureObject = CreatureObject::getCreatureObject(aiNetworkId);

				if (   (aiCreatureObject != nullptr)
				    && !aiCreatureObject->isPlayerControlled())
				{
					aiCreatureObject->setBaseWalkSpeed(Unicode::toFloat(argv[2]));
				}
			}
			else if (argv.size() > 1)
			{
				// Get the look at target and speed

				aiNetworkId = userCreatureObject->getLookAtTarget();
				aiCreatureObject = CreatureObject::getCreatureObject(aiNetworkId);

				if (   (aiCreatureObject != nullptr)
				    && !aiCreatureObject->isPlayerControlled())
				{
					aiCreatureObject->setBaseWalkSpeed(Unicode::toFloat(argv[1]));
				}
			}

			if (   (aiCreatureObject == nullptr)
				|| aiCreatureObject->isPlayerControlled())
			{
				Chat::sendSystemMessage(*userCreatureObject, Unicode::narrowToWide(fs.sprintf("[FAILURE] Unable to resolve %s to an AI", aiNetworkId.getValueString().c_str())), Unicode::emptyString);
				result += getErrorMessage(argv[0], ERR_FAIL);
			}
			else
			{
				Chat::sendSystemMessage(*userCreatureObject, Unicode::narrowToWide(fs.sprintf("[SUCCESS] AI %s has a walk speed of %.1fm/s", aiCreatureObject->getNetworkId().getValueString().c_str(), aiCreatureObject->getBaseWalkSpeed())), Unicode::emptyString);
				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
		}
	}
	else if (isAbbrev(argv[0], "runspeed"))
	{
		CreatureObject * const userCreatureObject = CreatureObject::getCreatureObject(userId);

		if (userCreatureObject == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
		{
			NetworkId aiNetworkId;
			CreatureObject * aiCreatureObject = nullptr;

			if (argv.size() > 2)
			{
				// Get the network id and speed

				aiNetworkId = NetworkId(Unicode::wideToNarrow(argv[1]));
				aiCreatureObject = CreatureObject::getCreatureObject(aiNetworkId);

				if (   (aiCreatureObject != nullptr)
				    && !aiCreatureObject->isPlayerControlled())
				{
					aiCreatureObject->setBaseRunSpeed(Unicode::toFloat(argv[2]));
				}
			}
			else if (argv.size() > 1)
			{
				// Get the look at target and speed

				aiNetworkId = userCreatureObject->getLookAtTarget();
				aiCreatureObject = CreatureObject::getCreatureObject(aiNetworkId);

				if (   (aiCreatureObject != nullptr)
				    && !aiCreatureObject->isPlayerControlled())
				{
					aiCreatureObject->setBaseRunSpeed(Unicode::toFloat(argv[1]));
				}
			}

			if (   (aiCreatureObject == nullptr)
				|| aiCreatureObject->isPlayerControlled())
			{
				Chat::sendSystemMessage(*userCreatureObject, Unicode::narrowToWide(fs.sprintf("[FAILURE] Unable to resolve %s to an AI", aiNetworkId.getValueString().c_str())), Unicode::emptyString);
				result += getErrorMessage(argv[0], ERR_FAIL);
			}
			else
			{
				Chat::sendSystemMessage(*userCreatureObject, Unicode::narrowToWide(fs.sprintf("[SUCCESS] AI %s has a run speed of %.1fm/s", aiCreatureObject->getNetworkId().getValueString().c_str(), aiCreatureObject->getBaseRunSpeed())), Unicode::emptyString);
				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
		}
	}
#ifdef _DEBUG
	else if (isAbbrev(argv[0], "killall"))
	{
		CreatureObject * const userCreatureObject = CreatureObject::getCreatureObject(userId);

		if (userCreatureObject != nullptr)
		{
			NetworkId sparedAiNetworkId;

			// See if we need to spare any single AI
			{
				if (argv.size() > 1)
				{
					sparedAiNetworkId = NetworkId(Unicode::wideToNarrow(argv[1]));
				}
				else
				{
					sparedAiNetworkId = userCreatureObject->getLookAtTarget();
				}
			}

			// Run through all the AI on the server and kill them

			int const serverObjectCount = ServerWorld::getNumObjects();
			int killCount = 0;

			for (int i = 0; i < serverObjectCount; ++i)
			{
				ServerObject * const serverObject = ServerWorld::getObject(i);
				CreatureObject * const creatureObject = (serverObject != nullptr) ? serverObject->asCreatureObject() : nullptr;

				if (   (creatureObject != nullptr)
				    && !creatureObject->isDead()
				    && !creatureObject->isPlayerControlled()
				    && (creatureObject->getNetworkId() != sparedAiNetworkId))
				{
					creatureObject->setIncapacitated(true, NetworkId::cms_invalid);
					++killCount;
				}
			}

			Chat::sendSystemMessage(*userCreatureObject, Unicode::narrowToWide(fs.sprintf("[SUCCESS] %d AI terminated.", killCount)), Unicode::emptyString);

			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_FAIL);
		}
	}
	else if (isAbbrev(argv[0], "debug"))
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
	else if (isAbbrev(argv[0], "debugText"))
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
#endif //_DEBUG
	else
	{
		result += getErrorMessage(argv[0], ERR_NO_HANDLER);
	}

	return true;
}

// ======================================================================
