// ======================================================================
//
// ConsoleCommandParserServer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserServer.h"

#include "serverGame/AiCombatPulseQueue.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/Chat.h"
#include "serverGame/Client.h"
#include "serverGame/CommoditiesMarket.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/FormManagerServer.h"
#include "serverGame/GameServer.h"
#include "serverGame/GroupObject.h"
#include "serverGame/GuildObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NameManager.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/PlayerSanityChecker.h"
#include "serverGame/Region.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ServerBuildoutManager.h"
#include "serverGame/ServerCommandTable.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/TriggerVolume.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "serverNetworkMessages/ChangeUniverseProcessMessage.h"
#include "serverNetworkMessages/EnableNewJediTrackingMessage.h"
#include "serverNetworkMessages/ExcommunicateGameServerMessage.h"
#include "serverNetworkMessages/ProfilerOperationMessage.h"
#include "serverNetworkMessages/ReleaseAuthoritativeMessage.h"
#include "serverNetworkMessages/ReloadAdminTableMessage.h"
#include "serverNetworkMessages/ReloadCommandTableMessage.h"
#include "serverNetworkMessages/ReloadDatatableMessage.h"
#include "serverNetworkMessages/RestartServerMessage.h"
#include "serverNetworkMessages/SetConnectionServerPublic.h"
#include "serverNetworkMessages/TransferRequestMoveValidation.h"
#include "serverScript/GameScriptObject.h"
#include "serverUtility/AdminAccountManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/QuestManager.h"
#include "sharedMath/Sphere.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"
#include "sharedNetworkMessages/EnterStructurePlacementModeMessage.h"
#include "sharedNetworkMessages/ErrorMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/MessageRegionListCircleResponse.h"
#include "sharedNetworkMessages/MessageRegionListRectResponse.h"
#include "sharedNetworkMessages/WhoListMessage.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "serverUtility/FreeCtsDataTable.h"

#include "sharedFoundation/CrcConstexpr.hpp"

#include <stdio.h>

// ======================================================================

namespace ConsoleCommandParserServerNamespace
{
	const char* const ms_testStructurePlacement = "testStructurePlacement";
}

using namespace ConsoleCommandParserServerNamespace;

// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"kill",                 1,  "<1: local, 2: planet, 3: cluster",  "Shutdown the server, planet, or cluster immediately."},
	{"shutdown",                 3,  "<secs to wait> <secs max> <system message to broadcast>",  "Shutdown the cluster gracefully.  System message will be broadcast to players each minute until <secs to wait> has passed then all players will be disconnected and the database will perform a final save and shutdown.  If <secs max> time passes, cluster will shutdown without waiting any more."},
	{"abortShutdown",            0,  "",  "abortShutdown"},
	{"error",                    1,  "<error message>",                   "Display an error message."},
	{"getPlanetId",              0,  "[planet]",                          "Get the current obj_id of the planet for this server (or for the specified planet)."},
	{"memUsage",                 0,  "",                                  "Query memory usage statistics."},
	{"dumpMemToFile",            2,  "<fileName> <leaksOnly>",            "Dump memory report to file on server, can be called to dump all allocations or only known leaks."},
	{"clock",                    0,  "",                                  "Query clock statistics."},
	{"releaseAuth",              1,  "<oid>",                             "Release Authority for object with oid."},
	{"showObjectSpheres",        0,  "",                                  "dump the server object sphere tree"},
	{"showTriggerSpheres",       0,  "",                                  "dump the server trigger sphere tree"},
	{"snapAllObjectsToTerrain",  0,  "",                                  "Snap all objects to the terrain"},
	{"getSceneId",               0,  "",                                  "Get the name of the scene we are running in"},
	{"setGodMode",               1,  "<1 or 0>",                          "Set the client as god or not."},
	{"reloadCommandTable",       0,  "",                                  "Reload the command table."},
	{"reloadAdminTable",         0,  "",                                  "Reload the admin permissions table."},
	{"reloadTable",              1,  "<tableName>",                       "Reload a given datatable."},
	{"dataTableGetString",       3,  "<table> <col> <row>",               "GetString from table."},
	{"reloadTerrain",            0,  "",                                  "Reload the terrain"},
	{"listRegions",              0,  "[planet]",                          "Lists the regions for a planet"},
	{"messageCount",             0,  "",                                  "Enumerate messages by name and count"},
	{"setPublic",                1, "0 | 1", "Set the cluster public or non-public. The cluster is closed (private) if the first parameter is 0"},
	{"profiler",                 1, "<up|down|toggle|expand|collapse|enable|disable|enableOutput|disableOutput|showAll|showNormal|displayMinimum <percent>> <processId or 0 for all>", "modify profiler state"},
	{"setFrameRateLimit",        1, "<float>",                            "Set the maximum framerate the server will run at"},
	{ms_testStructurePlacement,  1,  "<template name>",                   "Put the client into structure placement mode"},
	{"setDebugFlag",             3, "<section> <name> <0 | 1>",           "Toggle a debugFlag on the server"},
	{"validateWorld",            0, "",                                   "Run a sanity check on the world"},
	{"setUniverseProcess",       1, "<int>",                              "Change universe authority to that process id"},
	{"getRegionsAt",             2, "<x> <z>",                            "List the regions at the given point"},
	{"enableNewJedi",            1, "<true | false>",                     "Enable/disable tracking of players to see if they can become Jedi"},
	{"setMessageToTimeLimit",    1, "<time limit in ms>",                 "Sets the time limit we will process messageTos in a frame. <= 0 means process all messages"},
	{"setMessageToBacklogThreshold", 1, "<time limit in ms>",             "Sets the time limit to print backlogged messages <= 0 means never print messages"},
	{"startSave",                0, "",                                   "Begin saving data to the database, if a save is not already in progress"},
	{"enablePlayerSanityChecker",1, "<true | false>",                     "Enable/disable checking players for authority problems (cluster-wide)"},
	{"getNameList",              0, "",                                   "Print the list of all player names on this cluster."},
	{"getCharacterInfo",         1, "<oid | name>",                       "Get info for the specified character, even if the character is not currently online (can specify either the character's oid or name)"},
	{"reloadForms",              0, "",                                   "Reload the server's GodClient Form data"},
	{"setAiCombatPulseQueueNumber", 1,"<number>",                         "Set the number of AI pulses per frame"},
	{"setAiCombatPulseQueueMaxWaitTimeMs", 1,"<number>",                  "Set the max time that an AI has to wait for an AI pulse"},
	{"reloadQuests",             0, "",                                   "Reload SERVER quest data"},
	{"saveBuildoutArea",         6, "<server_table> <client_table> <x1> <z1> <x2> <z2>", "Save out an area for buildout datatables"},
	{"clientSaveBuildoutArea",   6, "<scene> <area_name> <x1> <z1> <x2> <z2>", "Create area buildout datatables and send to requesting client (for use by god client)"},
	{"destroyPersistedBuildoutAreaDuplicates", 0, "",                     "Destroy persisted objects which are duplicates of loaded buildout area objects"},
	{"editBuildoutArea",         1, "<area_name>",                        "Make all objects for a buildout area not be client cached (for use by god client)"},
	{"setQuestDebugging",        1, "<1 or 0>",                           "Set the quest system to output debug info or not"},
	{"restartServer",            1, "<server process id> | <planet> <server number> | <planet> <x> <z>", "Restart a game server, specified by server id, planet and number, or by geographic location."},
	{"restartPlanet",            1, "<planet>",                           "Restart the planet server and all game servers for the specified planet."},
	{"dumpCreatures",            1, "<filename>",                         "Get a list of all creatures on the current game server (with tab delimited info about each creature)."},
	{"getNumberOfMoveObjectLists", 0, "",                                 "Get the number of move object lists"},
	{"setNumberOfMoveObjectLists", 1, "<number>",                         "Set the number of move object lists"},
	{"getSystemClockTimeFromAllGameServers", 0, "",                       "Ask every game server to report the system clock time of the box it is running on, which is useful to detect out of sync system clock"},
	{"getPlanetaryTimeFromAllGameServers", 0, "",                         "Ask every game server to report its planetary time"},
	{"requestSameServer",        2, "<oid1> <oid2>",                      "Send a request to the planet server to ask for either oid1 or oid2 to be moved, if possible, to the same game server. Either or neither may be moved."},
	{"displayTerrain",           1, "<range in meters>",                  "Display the generated/ungenerated status of terrain chunks around the current location"},
	{"generateTerrain",          1, "<range in meters>",                  "Generate terrain chunks (if the terrain chunks are not currently generated) around the current location"},
	{"purgeTerrain",             0, "",                                   "Purge all generated terrain chunks (***FOR DEBUGGING ONLY***)"},
	{"listConnectedCharacters",  0, "",                                   "List all characters currently connected to the cluster."},
	{"listConnectedCharacterData", 1, "<oid>",                            "List the character data for a connected character."},
	{"listTravelPoints",         0, "[<planet name> <x> <y> <z>]",        "List all the travel points on the current or specified planet sorted by the distance from the current or specified location"},
	{"listCharacterLastLoginTimeBrief", 1, "<within the past nnn number of days>", "List the count of all characters on this cluster who last logged in within the specified number of days"},
	{"listCharacterLastLoginTimeAfterBrief", 6, "<yyyy> <mm> <dd> <h> <m> <s>", "List the count of all characters on this cluster who last logged in after the specified date/time"},
	{"listCharacterLastLoginTimeBeforeBrief", 6, "<yyyy> <mm> <dd> <h> <m> <s>", "List the count of all characters on this cluster who last logged in before the specified date/time"},
	{"listCharacterLastLoginTimeBetweenBrief", 12, "<yyyy> <mm> <dd> <h> <m> <s> <yyyy> <mm> <dd> <h> <m> <s>", "List the count of all characters on this cluster who last logged in between the specified date/time"},
	{"listCharacterLastLoginTimeDetailed", 1, "<within the past nnn number of days>", "List the characters on this cluster who last logged in within the specified number of days (this could be a long list and cause you to get disconnected because too much information is sent to your client)"},
	{"listCharacterLastLoginTimeAfterDetailed", 6, "<yyyy> <mm> <dd> <h> <m> <s>", "List the characters on this cluster who last logged in after the specified date/time (this could be a long list and cause you to get disconnected because too much information is sent to your client)"},
	{"listCharacterLastLoginTimeBeforeDetailed", 6, "<yyyy> <mm> <dd> <h> <m> <s>", "List the characters on this cluster who last logged in before the specified date/time (this could be a long list and cause you to get disconnected because too much information is sent to your client)"},
	{"listCharacterLastLoginTimeBetweenDetailed", 12, "<yyyy> <mm> <dd> <h> <m> <s> <yyyy> <mm> <dd> <h> <m> <s>", "List the characters on this cluster who last logged in between the specified date/time (this could be a long list and cause you to get disconnected because too much information is sent to your client)"},
	{"sendMailToCharacterLastLoginTime", 4, "<within the past nnn number of days> <from name> <mail subject> <mail body paragraph> ... <mail body paragraph>", "Sends mail to characters on this cluster who last logged in within the specified number of days"},
	{"sendMailToCharacterLastLoginTimeAfter", 9, "<yyyy> <mm> <dd> <h> <m> <s> <from name> <mail subject> <mail body paragraph> ... <mail body paragraph>", "Sends mail to characters on this cluster who last logged in after the specified date/time"},
	{"sendMailToCharacterLastLoginTimeBefore", 9, "<yyyy> <mm> <dd> <h> <m> <s> <from name> <mail subject> <mail body paragraph> ... <mail body paragraph>", "Sends mail to characters on this cluster who last logged in before the specified date/time"},
	{"sendMailToCharacterLastLoginTimeBetween", 15, "<yyyy> <mm> <dd> <h> <m> <s> <yyyy> <mm> <dd> <h> <m> <s> <from name> <mail subject> <mail body paragraph> ... <mail body paragraph>", "Sends mail to characters on this cluster who last logged in between the specified date/time"},
	{"listCharacterCreateTimeBrief", 1, "<within the past nnn number of days>", "List the count of all characters on this cluster created within the specified number of days"},
	{"listCharacterCreateTimeAfterBrief", 6, "<yyyy> <mm> <dd> <h> <m> <s>", "List the count of all characters on this cluster created after the specified date/time"},
	{"listCharacterCreateTimeBeforeBrief", 6, "<yyyy> <mm> <dd> <h> <m> <s>", "List the count of all characters on this cluster created before the specified date/time"},
	{"listCharacterCreateTimeBetweenBrief", 12, "<yyyy> <mm> <dd> <h> <m> <s> <yyyy> <mm> <dd> <h> <m> <s>", "List the count of all characters on this cluster created between the specified date/time"},
	{"listCharacterCreateTimeDetailed", 1, "<within the past nnn number of days>", "List the characters on this cluster created within the specified number of days (this could be a long list and cause you to get disconnected because too much information is sent to your client)"},
	{"listCharacterCreateTimeAfterDetailed", 6, "<yyyy> <mm> <dd> <h> <m> <s>", "List the characters on this cluster created after the specified date/time (this could be a long list and cause you to get disconnected because too much information is sent to your client)"},
	{"listCharacterCreateTimeBeforeDetailed", 6, "<yyyy> <mm> <dd> <h> <m> <s>", "List the characters on this cluster created before the specified date/time (this could be a long list and cause you to get disconnected because too much information is sent to your client)"},
	{"listCharacterCreateTimeBetweenDetailed", 12, "<yyyy> <mm> <dd> <h> <m> <s> <yyyy> <mm> <dd> <h> <m> <s>", "List the characters on this cluster created between the specified date/time (this could be a long list and cause you to get disconnected because too much information is sent to your client)"},
	{"sendMailToCharacterCreateTime", 4, "<within the past nnn number of days> <from name> <mail subject> <mail body paragraph> ... <mail body paragraph>", "Sends mail to characters on this cluster created within the specified number of days"},
	{"sendMailToCharacterCreateTimeAfter", 9, "<yyyy> <mm> <dd> <h> <m> <s> <from name> <mail subject> <mail body paragraph> ... <mail body paragraph>", "Sends mail to characters on this cluster created after the specified date/time"},
	{"sendMailToCharacterCreateTimeBefore", 9, "<yyyy> <mm> <dd> <h> <m> <s> <from name> <mail subject> <mail body paragraph> ... <mail body paragraph>", "Sends mail to characters on this cluster created before the specified date/time"},
	{"sendMailToCharacterCreateTimeBetween", 15, "<yyyy> <mm> <dd> <h> <m> <s> <yyyy> <mm> <dd> <h> <m> <s> <from name> <mail subject> <mail body paragraph> ... <mail body paragraph>", "Sends mail to characters on this cluster created between the specified date/time"},
	{"freeCtsTest", 5, "<character create time> <source station id> <source cluster> <target station id> <target cluster>", "Tests to see if a CTS transaction would be free"},
	{"isFreeCtsSourceCluster", 1, "<source cluster>",                     "Checks to see if the specified cluster is a free CTS source cluster"},
	{"freeCtsInfo", 0, "",                                                "Display information about what CTS transactions are free"},
#ifdef _DEBUG
	{"listRetroactiveCtsHistory", 2, "<oid> <cluster>",                   "List the retroactive CTS history for the specified character on the specified cluster (the character does not have to be logged in or nearby)."},
#else
	{"listRetroactiveCtsHistory", 1, "<oid>",                             "List the retroactive CTS history for the specified character (the character does not have to be logged in or nearby)."},
#endif
	{"transferCharacter", 1, "<destination galaxy> [<destination character name>] [<destination station id>]", "Transfers this character to the specified destination galaxy (which can be the same as the current galaxy).  Optionally specify the destination character name (surrounded in quotes if there is embedded space in the character's name).  Optionally specify the destination station id if you want to transfer the character to a different station id."},
	{"showGcwScore",             0, "",                                   "Display the GCW score for this cluster"},
	{"adjustGcwImperialScore",   2, "<category> <adjustment>",            "Adjust the GCW Imperial score for this cluster"},
	{"adjustGcwRebelScore",      2, "<category> <adjustment>",            "Adjust the GCW Rebel score for this cluster"},
	{"decayGcwScore",            1, "<category>",                         "Force an immediate decay of the GCW score"},
	{"showGcwFactionalPresence", 0, "",                                   "Display the GCW contested regions that have factional presence"},
	{"hasGcwFactionalPresence",  1, "<oid>",                              "Checks to see if the specified player character would have factional presence in its current state"},
	{"hasGcwRegionDefenderBonus", 1, "<oid>",                             "Checks to see if the specified player character would have GCW Region Defender bonus in its current state"},
	{"getCommoditiesItemAttributeData", 1, "<output filename> [<got=xxx>] [<got_exact=xxx>] [<ignore_searchable_attribute>] [<throttle count>]", "Writes a summary of the item attribute name of all items on the commodities server to the specified filename on the client.  Specify got=xxx or got_exact=xxx (like got=weapon_ranged_pistol) to only include items of the specified GOT type.  got_exact= will only include items whose GOT type exactly matches the specified GOT type.  got= will also use the item's masked GOT type to check for a GOT match.  Specify ignore_searchable_attribute to ignore attributes that are already flagged as searchable.  Throttle count (default 100) specifies how many item to process per frame, to spread out the load over multiple frames."},
	{"getCommoditiesItemAttribute", 1, "<oid>",                           "Gets the attribute data for the specified commodities item"},
	{"getCommoditiesItemAttributeDataValues", 2, "<game object type> <attribute name> [<got_exact>] [<throttle count>]", "For all items in the commodities system with the given game object type, get the attribute value for the given attribute name.  Specify got_exact to only include items whose GOT type exactly matches the specified GOT type (GOT types derived from the specified GOT type will be excluded).  Throttle count (default 100) specifies how many item to process per frame, to spread out the load over multiple frames."},
	{"getCommoditiesAuctionLocationPriorityQueue", 1, "<count>",          "Gets the top <count> items in the auction location priority queue"},
	{"getCommoditiesItemTypeMap", 1, "<game object type>",                "For the specified game object type, gets the list of item type"},
	{"setCompletedTutorial",      1, "<oid>",                             "Mark the account belonging to the specified character (character does not have to be logged in) as having completed the tutorial, so that the skip tutorial option will be available during character creation for that account"},
	{"getUniverseProcess", 0, "", "Returns the Game Server Universe Process ID"},
#ifdef _DEBUG
	{"setExtraDelayPerFrameMs",   1, "<ms>",                              "Do an intentional sleep each frame, to emulate long loop time"},
	{"serverinfo",   0, "",                              "Serverinformation"},
#endif
	{"", 0, "", ""} // this must be last
};

//-----------------------------------------------------------------

ConsoleCommandParserServer::ConsoleCommandParserServer(void) :
	CommandParser("server", 0, "...", "Server related commands.", 0)
{
	createDelegateCommands(cmds);
}

//-----------------------------------------------------------------

bool ConsoleCommandParserServer::performParsing(const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	NOT_NULL(node);

	UNREF(originalCommand);

	ServerObject * user = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(userId));

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

	if (isAbbrev(argv[0], ms_testStructurePlacement))
	{
		//-- make sure the user is valid
		if (!user)
		{
			result += Unicode::narrowToWide("invalid user");
			return true;
		}

		if (!user->getClient())
		{
			result += Unicode::narrowToWide("invalid client");
			return true;
		}

		if (!user->getClient()->isGod())
		{
			result += Unicode::narrowToWide("user does not have admin privileges");
			return true;
		}

		const std::string serverObjectTemplateName = Unicode::wideToNarrow(argv[1]);

		//-- make sure the name is not empty
		if (serverObjectTemplateName.empty())
		{
			result += Unicode::narrowToWide("serverObjectTemplateName is empty");
			return true;
		}

		//-- fetch the object template
		const ObjectTemplate* const objectTemplate = ObjectTemplateList::fetch(serverObjectTemplateName);
		if (!objectTemplate)
		{
			result += Unicode::narrowToWide("serverObjectTemplate does not exist");
			return true;
		}

		//-- fetch the server object template
		const ServerObjectTemplate* const serverObjectTemplate = dynamic_cast<const ServerObjectTemplate*> (objectTemplate);
		if (!serverObjectTemplate)
		{
			objectTemplate->releaseReference();

			result += Unicode::narrowToWide("serverObjectTemplate is not a server object template");
			return true;
		}

		//-- fetch the shared object template
		const std::string sharedObjectTemplateName = serverObjectTemplate->getSharedTemplate();
		serverObjectTemplate->releaseReference();
		if (sharedObjectTemplateName.empty())
		{
			result += Unicode::narrowToWide("serverObjectTemplate specifies no sharedObjectTemplateName");
			return true;
		}

		//-- send the message to the client
		EnterStructurePlacementModeMessage const msg(NetworkId::cms_invalid, sharedObjectTemplateName);
		user->getClient()->send(msg, true);
	}
	else if (isAbbrev(argv[0], "setPublic"))
	{
		//-- make sure the user is valid
		if (!user)
		{
			result += Unicode::narrowToWide("invalid user");
			return true;
		}

		if (!user->getClient())
		{
			result += Unicode::narrowToWide("invalid client");
			return true;
		}

		if (!user->getClient()->isGod())
		{
			result += Unicode::narrowToWide("user does not have admin privileges");
			return true;
		}

		unsigned long val = strtoul(Unicode::wideToNarrow(argv[1]).c_str(), nullptr, 10);
		const bool isPublic = (val != 0);
		SetConnectionServerPublic const p(isPublic);

		GameServer::getInstance().sendToCentralServer(p);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "abortShutdown"))
	{
		if (!user)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_USER);
			return true;
		}
		if (!user->getClient())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		if (!user->getClient()->isGod())
		{
			result += Unicode::narrowToWide("User is not a god");
			return true;
		}
		AbortShutdown const msg;
		GameServer::getInstance().sendToCentralServer(msg);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "shutdown"))
	{
		if (!user)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_USER);
			return true;
		}
		if (!user->getClient())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		if (!user->getClient()->isGod())
		{
			result += Unicode::narrowToWide("User is not a god");
			return true;
		}
		if (argv.size() < 4)
		{
			result += Unicode::narrowToWide("Not enough parameters.  usage is \"shutdown [secsToWait] [maxSecs] [systemMessage]\"");
			return true;
		}

		unsigned long secsToWait = strtoul(Unicode::wideToNarrow(argv[1]).c_str(), nullptr, 10);
		unsigned long maxSecs = strtoul(Unicode::wideToNarrow(argv[2]).c_str(), nullptr, 10);
		Unicode::String systemMessage;
		for (size_t i = 3; i < argv.size(); ++i)
		{
			systemMessage += argv[i] + Unicode::narrowToWide(" ");
		}

		ShutdownCluster const msg(secsToWait, maxSecs, systemMessage);
		GameServer::getInstance().sendToCentralServer(msg);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	//-----------------------------------------------------------------
	else if (isAbbrev(argv[0], "kill"))
	{
		if (!user)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_USER);
			return true;
		}
		if (!user->getClient())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		if (!user->getClient()->isGod())
		{
			result += Unicode::narrowToWide("User is not a god");
			return true;
		}

		unsigned long val = strtoul(Unicode::wideToNarrow(argv[1]).c_str(), nullptr, 10);
		switch (val)
		{
		case 1:
		{
			ExcommunicateGameServerMessage exmsg(GameServer::getInstance().getProcessId(), 0, "");
			GameServer::getInstance().sendToCentralServer(exmsg);
			result += getErrorMessage(argv[0], ERR_SUCCESS);
			break;
		}
		case 2:
		{
			GenericValueTypeMessage<int> const msg("ShutdownMessage", 0);
			GameServer::getInstance().sendToPlanetServer(msg);
			result += getErrorMessage(argv[0], ERR_SUCCESS);
			break;
		}
		case 3:
		{
			GenericValueTypeMessage<int> const msg("RequestClusterShutdown", 0);
			GameServer::getInstance().sendToCentralServer(msg);
			result += getErrorMessage(argv[0], ERR_SUCCESS);
			break;
		}
		default:
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
	}
	//-----------------------------------------------------------------
	else if (isAbbrev(argv[0], "error"))
	{
		if (user && user->getClient())
		{
			ErrorMessage const em("GameServer Console", Unicode::wideToNarrow(argv[1]).c_str());
			user->getClient()->send(em, true);
		}
	}

	//-----------------------------------------------------------------------

	else if (isAbbrev(argv[0], "getPlanetId"))
	{
		PlanetObject * p = ServerUniverse::getInstance().getCurrentPlanet();
		if (argv.size() > 1)
			p = ServerUniverse::getInstance().getPlanetByName(Unicode::wideToNarrow(argv[1]));

		NetworkId pid;
		if (p)
		{
			pid = p->getNetworkId();
		}
		result += Unicode::narrowToWide(pid.getValueString());
	}

	//-----------------------------------------------------------------
	else if (isAbbrev(argv[0], "clock"))
	{
		char text[128];
		sprintf(text, "FPS: %f FrameTime: %f\n", Clock::framesPerSecond(), Clock::frameTime());
		result += Unicode::narrowToWide(text);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	//-----------------------------------------------------------------
	else if (isAbbrev(argv[0], "releaseAuth"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject* object = ServerWorld::findObjectByNetworkId(oid);
		if (object == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		if (!object->isAuthoritative() || object->getProxyCount() < 1)
		{
			result += getErrorMessage(argv[0], ERR_CANNOT_XFER_AUTH);
			return true;
		}
		ReleaseAuthoritativeMessage const msg(object->getNetworkId());
		GameServer::getInstance().sendToCentralServer(msg);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "showObjectSpheres"))
	{
		std::vector<std::pair<ServerObject *, Sphere> > results;
		ServerWorld::dumpObjectSphereTree(results);
		result = Unicode::narrowToWide("Sending ");
		char count[1024];
		sprintf(count, "%i", results.size());
		result += Unicode::narrowToWide(count);
		result += Unicode::narrowToWide(" object sphere tree nodes to client\n");

		std::vector<std::pair<ServerObject *, Sphere> >::iterator i;
		for (i = results.begin(); i != results.end(); ++i)
		{
			std::string name = "SPHERE NODE";
			NetworkId id(NetworkId::cms_invalid);

			if ((*i).first)
			{
				name = Unicode::wideToNarrow((*i).first->getObjectName());
				id = (*i).first->getNetworkId();
			}
			sprintf(
				count,
				"Sphere id=[%lu] origin=[%f, %f, %f] radius=[%f] OID=[%s] %s\n",
				std::distance(results.begin(), i),//reinterpret_cast<unsigned long>(i),
				(*i).second.getCenter().x,
				(*i).second.getCenter().y,
				(*i).second.getCenter().z,
				(*i).second.getRadius(),
				id.getValueString().c_str(),
				name.c_str()
			);
			result += Unicode::narrowToWide(count);
		}
	}
	else if (isAbbrev(argv[0], "showTriggerSpheres"))
	{
		std::vector<std::pair<TriggerVolume *, Sphere> > results;
		ServerWorld::dumpTriggerSphereTree(results);
		result = Unicode::narrowToWide("Sending ");
		char count[128];
		sprintf(count, "%i", results.size());
		result += Unicode::narrowToWide(count);
		result += Unicode::narrowToWide(" trigger sphere tree nodes to client\n");

		std::vector<std::pair<TriggerVolume *, Sphere> >::iterator i;
		for (i = results.begin(); i != results.end(); ++i)
		{
			std::string name = "SPHERE NODE";
			NetworkId id(NetworkId::cms_invalid);
			if ((*i).first)
			{
				name = Unicode::wideToNarrow((*i).first->getOwner().getObjectName());
				id = (*i).first->getOwner().getNetworkId();
			}

			sprintf(
				count,
				"Sphere id=[%lu] origin=[%f, %f, %f] radius=[%f] OID=[%s] %s\n",
				std::distance(results.begin(), i),
				(*i).second.getCenter().x,
				(*i).second.getCenter().y,
				(*i).second.getCenter().z,
				(*i).second.getRadius(),
				id.getValueString().c_str(),
				name.c_str()
			);
			result += Unicode::narrowToWide(count);
		}
	}
	else if (isAbbrev(argv[0], "snapAllObjectsToTerrain"))
	{
		ServerWorld::snapAllObjectsToTerrain();
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "getSceneId"))
	{
		if (user == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		result += Unicode::narrowToWide(user->getSceneId());
		result += Unicode::narrowToWide("\n");
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "setGodMode"))
	{
		if (!user)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_USER);
			return true;
		}
		if (!user->getClient())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		unsigned long val = strtoul(Unicode::wideToNarrow(argv[1]).c_str(), nullptr, 10);
		if (user->getClient()->setGodMode(val != 0))
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += Unicode::narrowToWide("Cannot set god mode on unvalidated client\n");
	}
	else if (isAbbrev(argv[0], "reloadCommandTable"))
	{
		result += getErrorMessage(argv[0], ERR_SUCCESS);

		ServerCommandTable::load();

		// send a message to other servers to reload the table

		ServerMessageForwarding::beginBroadcast();

		ReloadCommandTableMessage const reloadCommandTableMessage;
		ServerMessageForwarding::send(reloadCommandTableMessage);

		ServerMessageForwarding::end();
	}
	else if (isAbbrev(argv[0], "reloadAdminTable"))
	{
		AdminAccountManager::reload();

		// send a message to other servers to reload the table

		ServerMessageForwarding::beginBroadcast();

		ReloadAdminTableMessage const reloadAdminTableMessage;
		ServerMessageForwarding::send(reloadAdminTableMessage);

		ServerMessageForwarding::end();

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "reloadTable"))
	{
		std::string tableName;
		tableName = Unicode::wideToNarrow(argv[1]);
		if (DataTableManager::reload(tableName) != nullptr)
		{
			ServerMessageForwarding::beginBroadcast();

			ReloadDatatableMessage const reloadDatatableMessage(tableName);
			ServerMessageForwarding::send(reloadDatatableMessage);

			ServerMessageForwarding::end();

			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
			result += getErrorMessage(argv[0], ERR_FAIL);
	}
	else if (isAbbrev(argv[0], "dataTableGetString"))
	{
		std::string tableName;
		tableName = Unicode::wideToNarrow(argv[1]);
		int col = atoi(Unicode::wideToNarrow(argv[2]).c_str());
		int row = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		DataTable * dt = DataTableManager::getTable(tableName);
		if (dt)
		{
			result += Unicode::narrowToWide(dt->getStringValue(col, row));
		}
		else
			result += Unicode::narrowToWide("No such table");
	}
	else if (isAbbrev(argv[0], "reloadTerrain"))
	{
		GameServer::getInstance().loadTerrain();
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "messageCount"))
	{
		std::vector<std::pair<std::string, int> > messages = GameNetworkMessage::getMessageCount();
		std::vector<std::pair<std::string, int> >::iterator i;
		char buf[128] = { "\0" };
		for (i = messages.begin(); i != messages.end(); ++i)
		{
			IGNORE_RETURN(snprintf(buf, sizeof(buf), "[%9d] - \"%s\"\n", (*i).second, (*i).first.c_str()));
			result += Unicode::narrowToWide(buf);
		}
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "listRegions"))
	{
		Client * client = user->getClient();
		if (client)
		{
			//get all the regions, and build messages, and sender to client (should be priveledged client, i.e. godclient user)
			std::string planetName;
			if (argv.size() > 1)
				planetName = Unicode::wideToNarrow(argv[1]);
			else
			{
				PlanetObject * planet = ServerUniverse::getInstance().getCurrentPlanet();
				if (planet == nullptr)
				{
					result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
					return true;
				}
				planetName = planet->getName();
			}

			std::vector<const Region *> regions;
			RegionMaster::getRegionsForPlanet(planetName, regions);

			for (std::vector<const Region *>::const_iterator i = regions.begin(); i != regions.end(); ++i)
			{
				const RegionRectangle * ro = dynamic_cast<const RegionRectangle*>(*i);
				if (ro != nullptr)
				{
					float minX, minY, maxX, maxY;
					ro->getExtent(minX, minY, maxX, maxY);

					MessageRegionListRectResponse const mrgrr(minX, minY, maxX, maxY, ro->getPvp(), ro->getMunicipal(), ro->getBuildable(), ro->getGeography(), ro->getMinDifficulty(), ro->getMaxDifficulty(), ro->getSpawn(), ro->getMission(), ro->getName(), ro->getPlanet());
					client->send(mrgrr, true);
				}
				const RegionCircle* co = dynamic_cast<const RegionCircle*>(*i);
				if (co != nullptr)
				{
					float centerX, centerY, radius;
					co->getExtent(centerX, centerY, radius);

					MessageRegionListCircleResponse const mrgcr(centerX, centerY, radius, co->getPvp(), co->getMunicipal(), co->getBuildable(), co->getGeography(), co->getMinDifficulty(), co->getMaxDifficulty(), co->getSpawn(), co->getMission(), co->getName(), co->getPlanet());
					client->send(mrgcr, true);
				}
			}
		}
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "profiler"))
	{
		if (argv.size() > 1)
		{
			if (isAbbrev(argv[1], "get"))
			{
				result += Unicode::narrowToWide(PROFILER_GET_LAST_FRAME_DATA());
			}
			else if (isAbbrev(argv[1], "displayMinimum"))
			{
				if (argv.size() > 2)
				{
					uint32 processId = GameServer::getInstance().getProcessId();
					if (argv.size() > 3)
						processId = strtoul(Unicode::wideToNarrow(argv[3]).c_str(), nullptr, 10);

					std::string op = Unicode::wideToNarrow(argv[1]) + " " + Unicode::wideToNarrow(argv[2]);

					if (processId == GameServer::getInstance().getProcessId())
						Profiler::handleOperation(op.c_str());
					else
					{
						ProfilerOperationMessage const msg(processId, op.c_str());
						GameServer::getInstance().sendToCentralServer(msg);
					}
				}
			}
			else
			{
				uint32 processId = GameServer::getInstance().getProcessId();
				if (argv.size() > 2)
					processId = strtoul(Unicode::wideToNarrow(argv[2]).c_str(), nullptr, 10);

				std::string op(Unicode::wideToNarrow(argv[1]));
				if (processId == GameServer::getInstance().getProcessId())
					Profiler::handleOperation(op.c_str());
				else
				{
					ProfilerOperationMessage const msg(processId, op.c_str());
					GameServer::getInstance().sendToCentralServer(msg);
				}
			}
		}
	}
	else if (isAbbrev(argv[0], "setFrameRateLimit"))
	{
		if (argv.size() > 1)
		{
			float newLimit = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str()));

			Clock::setFrameRateLimit(newLimit);

			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_FAIL);
		}
	}
	else if (isAbbrev(argv[0], "setDebugFlag"))
	{
		if (!user->getClient()->isGod())
		{
			result += Unicode::narrowToWide("user does not have admin privileges");
		}
		else if (argv.size() > 3)
		{
			const std::string section = Unicode::wideToNarrow(argv[1]);
			const std::string name = Unicode::wideToNarrow(argv[2]);
			const bool value = argv[3][0] != '0';

			bool * const flag = DebugFlags::findFlag(section.c_str(), name.c_str());
			if (flag)
			{
				*flag = value;
				result += Unicode::narrowToWide("set flag " + section + "/" + name + " to ");
				if (value)
				{
					result += Unicode::narrowToWide("true");
				}
				else
				{
					result += Unicode::narrowToWide("false");
				}
			}
			else
			{
				result += Unicode::narrowToWide("setDebugFlag: could not find flag " + section + "/" + name);
			}
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_FAIL);
		}
	}
	else if (isAbbrev(argv[0], "validateWorld"))
	{
		World::validate();

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "setUniverseProcess"))
	{
		int processId = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		//Send message to central
		ChangeUniverseProcessMessage const m(processId);
		GameServer::getInstance().sendToCentralServer(m);
	}
	else if (isAbbrev(argv[0], "getRegionsAt"))
	{
		float x = static_cast<real>(strtod(Unicode::wideToNarrow(argv[1]).c_str(), nullptr));
		float z = static_cast<real>(strtod(Unicode::wideToNarrow(argv[2]).c_str(), nullptr));

		std::vector<const Region *> results;

		RegionMaster::getRegionsAtPoint(ServerWorld::getSceneId(), x, z, results);

		size_t resultCount = results.size();

		if (resultCount)
		{
			for (size_t i = 0; i < resultCount; i++)
			{
				result += results[i]->getName();
				result += Unicode::narrowToWide("\n");
			}
		}
		else
		{
			result += Unicode::narrowToWide("No regions at the given point\n");
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "enableNewJedi"))
	{
		std::string flag(Unicode::wideToNarrow(argv[1]));
		if (flag == "true" || flag == "false")
		{
			bool enableTracking = (flag == "true");
			GameScriptObject::enableNewJediTracking(enableTracking);

			ServerMessageForwarding::beginBroadcast();

			EnableNewJediTrackingMessage const enableNewJediTrackingMessage(enableTracking);
			ServerMessageForwarding::send(enableNewJediTrackingMessage);

			ServerMessageForwarding::end();

			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
			result += getErrorMessage(argv[0], ERR_INVALID_ARGUMENTS);
	}
	else if (isAbbrev(argv[0], "startSave"))
	{
		GenericValueTypeMessage<NetworkId> const msg("StartSaveMessage", userId);
		GameServer::getInstance().sendToDatabaseServer(msg);

		result += Unicode::narrowToWide("Request sent.  Please wait for reply");
	}
	else if (isAbbrev(argv[0], "getNameList"))
	{
		result += Unicode::narrowToWide(NameManager::getInstance().debugGetNameList());
	}
	else if (isAbbrev(argv[0], "getCharacterInfo"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		if (oid.isValid())
		{
			if (NameManager::getInstance().isPlayer(oid))
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character oid:             %s\n", oid.getValueString().c_str()));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character station id:      %lu\n", NameManager::getInstance().getPlayerStationId(oid)));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character first name:      %s\n", NameManager::getInstance().getPlayerName(oid).c_str()));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character full name:       %s\n", NameManager::getInstance().getPlayerFullName(oid).c_str()));

				time_t const createTime = static_cast<time_t>(NameManager::getInstance().getPlayerCreateTime(oid));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character create time:     %ld, %s, %s\n", createTime, CalendarTime::convertEpochToTimeStringLocal(createTime).c_str(), CalendarTime::convertEpochToTimeStringGMT(createTime).c_str()));

				time_t const lastLoginTime = static_cast<time_t>(NameManager::getInstance().getPlayerLastLoginTime(oid));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character last login time: %ld, %s, %s\n", lastLoginTime, CalendarTime::convertEpochToTimeStringLocal(lastLoginTime).c_str(), CalendarTime::convertEpochToTimeStringGMT(lastLoginTime).c_str()));
			}
			else
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("no player with id (%s)\n", oid.getValueString().c_str()));
			}
		}
		else
		{
			oid = NameManager::getInstance().getPlayerId(Unicode::wideToNarrow(Unicode::toLower(argv[1])));
			if (oid.isValid())
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character oid:             %s\n", oid.getValueString().c_str()));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character station id:      %lu\n", NameManager::getInstance().getPlayerStationId(oid)));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character first name:      %s\n", NameManager::getInstance().getPlayerName(oid).c_str()));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character full name:       %s\n", NameManager::getInstance().getPlayerFullName(oid).c_str()));

				time_t const createTime = static_cast<time_t>(NameManager::getInstance().getPlayerCreateTime(oid));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character create time:     %ld, %s, %s\n", createTime, CalendarTime::convertEpochToTimeStringLocal(createTime).c_str(), CalendarTime::convertEpochToTimeStringGMT(createTime).c_str()));

				time_t const lastLoginTime = static_cast<time_t>(NameManager::getInstance().getPlayerLastLoginTime(oid));
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character last login time: %ld, %s, %s\n", lastLoginTime, CalendarTime::convertEpochToTimeStringLocal(lastLoginTime).c_str(), CalendarTime::convertEpochToTimeStringGMT(lastLoginTime).c_str()));
			}
			else
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("no player with name (%s)\n", Unicode::wideToNarrow(argv[1]).c_str()));
			}
		}
	}
	else if (isAbbrev(argv[0], "enablePlayerSanityChecker"))
	{
		std::string flag(Unicode::wideToNarrow(argv[1]));
		if (flag == "true" || flag == "false")
		{
			bool enableFlag = (flag == "true");
			PlayerSanityChecker::enable(enableFlag);

			ServerMessageForwarding::beginBroadcast();

			GenericValueTypeMessage<bool> const enablePlayerSanityCheckerMessage("EnablePlayerSanityCheckerMessage", enableFlag);
			ServerMessageForwarding::send(enablePlayerSanityCheckerMessage);

			ServerMessageForwarding::end();

			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
			result += getErrorMessage(argv[0], ERR_INVALID_ARGUMENTS);
	}
	else if (isAbbrev(argv[0], "reloadForms"))
	{
		FormManagerServer::reloadData();
	}
	else if (isAbbrev(argv[0], "setAiCombatPulseQueueNumber"))
	{
		int limit = atol(Unicode::wideToNarrow(argv[1]).c_str());
		AiCombatPulseQueue::setAiPerFrame(limit);
	}
	else if (isAbbrev(argv[0], "setAiCombatPulseQueueMaxWaitTimeMs"))
	{
		int limit = atol(Unicode::wideToNarrow(argv[1]).c_str());
		AiCombatPulseQueue::setAiMaxWaitTimeMs(limit);
	}
	else if (isAbbrev(argv[0], "reloadQuests"))
	{
		QuestManager::reloadQuests();
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "setQuestDebugging"))
	{
		bool const debugging = Unicode::wideToNarrow(argv[1]) == "1";
		QuestManager::setDebugging(debugging);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "saveBuildoutArea"))
	{
		//"<server_table> <client_table> <x1> <z1> <x2> <z2>", "Save out an area for buildout datatables"
		std::string const &serverFilename = Unicode::wideToNarrow(argv[1]);
		std::string const &clientFilename = Unicode::wideToNarrow(argv[2]);
		float x1 = static_cast<float>(strtod(Unicode::wideToNarrow(argv[3]).c_str(), nullptr));
		float z1 = static_cast<float>(strtod(Unicode::wideToNarrow(argv[4]).c_str(), nullptr));
		float x2 = static_cast<float>(strtod(Unicode::wideToNarrow(argv[5]).c_str(), nullptr));
		float z2 = static_cast<float>(strtod(Unicode::wideToNarrow(argv[6]).c_str(), nullptr));
		ServerBuildoutManager::saveArea(serverFilename, clientFilename, x1, z1, x2, z2);
	}
	else if (isAbbrev(argv[0], "clientSaveBuildoutArea"))
	{
		//"<scene> <area_name> <x1> <z1> <x2> <z2>"
		std::string const &scene = Unicode::wideToNarrow(argv[1]);
		std::string const &areaName = Unicode::wideToNarrow(argv[2]);
		float x1 = static_cast<float>(strtod(Unicode::wideToNarrow(argv[3]).c_str(), nullptr));
		float z1 = static_cast<float>(strtod(Unicode::wideToNarrow(argv[4]).c_str(), nullptr));
		float x2 = static_cast<float>(strtod(Unicode::wideToNarrow(argv[5]).c_str(), nullptr));
		float z2 = static_cast<float>(strtod(Unicode::wideToNarrow(argv[6]).c_str(), nullptr));
		if (scene == ConfigServerGame::getSceneID() && user->getClient())
			ServerBuildoutManager::clientSaveArea(*user->getClient(), areaName, x1, z1, x2, z2);
	}
	else
	{
		// ConsoleCommandParserServer::performParsing() is getting too big for the compiler to handle
		// so some of the message handling is done in the private helper performParsing2()
		return performParsing2(userId, argv, originalCommand, result, node);
	}

	return true;
}

//-----------------------------------------------------------------
// ConsoleCommandParserServer::performParsing() is getting too big for the compiler to handle
// so some of the message handling is done in this private helper performParsing2()

bool ConsoleCommandParserServer::performParsing2(const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	NOT_NULL(node);

	UNREF(originalCommand);

	ServerObject * user = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(userId));

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

	if (isAbbrev(argv[0], "destroyPersistedBuildoutAreaDuplicates"))
	{
		ServerBuildoutManager::destroyPersistedDuplicates();
	}
	else if (isAbbrev(argv[0], "editBuildoutArea"))
	{
		std::string const &areaName = Unicode::wideToNarrow(argv[1]);
		ServerBuildoutManager::editBuildoutArea(areaName);
	}
	else if (isAbbrev(argv[0], "restartServer"))
	{
		if (argv.size() == 2)
		{
			// specify server by process id
			uint32 serverId = strtoul(Unicode::wideToNarrow(argv[1]).c_str(), nullptr, 10);
			if (serverId != 0)
			{
				ExcommunicateGameServerMessage exmsg(serverId, 0, "");
				GameServer::getInstance().sendToCentralServer(exmsg);
				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
			else
				result += Unicode::narrowToWide("Invalid server number");
		}
		else if (argv.size() == 3)
		{
			// specify server by scene & preload role number
			std::string const &scene = Unicode::wideToNarrow(argv[1]);
			uint32 preloadRole = strtoul(Unicode::wideToNarrow(argv[2]).c_str(), nullptr, 10);

			GenericValueTypeMessage<std::pair<std::string, uint32> > msg("RestartServerByRoleMessage", std::make_pair(scene, preloadRole));
			if (scene == ConfigServerGame::getSceneID())
				GameServer::getInstance().sendToPlanetServer(msg);
			else
				GameServer::getInstance().sendToCentralServer(msg);

			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else if (argv.size() == 4)
		{
			// specify server by geographic location
			std::string const &scene = Unicode::wideToNarrow(argv[1]);
			int x = static_cast<int>(strtod(Unicode::wideToNarrow(argv[2]).c_str(), nullptr));
			int z = static_cast<int>(strtod(Unicode::wideToNarrow(argv[3]).c_str(), nullptr));

			RestartServerMessage msg(scene, x, z);
			if (scene == ConfigServerGame::getSceneID())
				GameServer::getInstance().sendToPlanetServer(msg);
			else
				GameServer::getInstance().sendToCentralServer(msg);

			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
	}
	else if (isAbbrev(argv[0], "restartPlanet"))
	{
		if (argv.size() == 2)
		{
			std::string const &scene = Unicode::wideToNarrow(argv[1]);

			GenericValueTypeMessage<std::string> msg("RestartPlanetMessage", scene);

			if (scene == ConfigServerGame::getSceneID())
			{
				GenericValueTypeMessage<int> const msg("ShutdownMessage", 0);
				GameServer::getInstance().sendToPlanetServer(msg);
			}
			else
				GameServer::getInstance().sendToCentralServer(msg);

			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
			result += Unicode::narrowToWide("Syntax:  remote server restartPlanet <sceneId>");
	}
	else if (isAbbrev(argv[0], "dumpCreatures"))
	{
		// This is potentially an expensive command, so make sure that this can only be done in god mode
		if (!user)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_USER);
			return true;
		}
		if (!user->getClient())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		std::string resultNarrowString;

		const CreatureObject::AllCreaturesSet& creatureList = CreatureObject::getAllCreatures();

		// Compose a "header" so that people can decipher the tab delimited columns
		resultNarrowString += "Object ID\t";
		resultNarrowString += "Authoritative\t";
		resultNarrowString += "Parent ID\t";
		resultNarrowString += "Object Template\t";
		resultNarrowString += "World X\t";
		resultNarrowString += "World Y\t";
		resultNarrowString += "World Z\t";
		resultNarrowString += "Creature Name\t";
		resultNarrowString += "Creature Level\t";
		resultNarrowString += "Invulnerable\t";
		resultNarrowString += "Hibernating\t";
		resultNarrowString += "Movement State\t";
		resultNarrowString += "Observer Count\t";
		resultNarrowString += "Observer List\t";
		resultNarrowString += "\n";

		CreatureObject::AllCreaturesSet::const_iterator i;
		for (i = creatureList.begin(); i != creatureList.end(); ++i)
		{
			const CreatureObject* creatureObj = (*i);

			// Object ID
			resultNarrowString += creatureObj->getNetworkId().getValueString();
			resultNarrowString += "\t";

			// Authoritative
			resultNarrowString += creatureObj->isAuthoritative() ? "yes" : "no";
			resultNarrowString += "\t";

			// Parent ID
			resultNarrowString += ((creatureObj->getRootParent() != creatureObj) ? creatureObj->getRootParent()->getNetworkId().getValueString() : "none");
			resultNarrowString += "\t";

			// Object Template
			resultNarrowString += creatureObj->getObjectTemplateName() ? creatureObj->getObjectTemplateName() : "<NO OBJECT TEMPLATE NAME>";
			resultNarrowString += "\t";

			// World coordinates
			{
				char buffer[32];

				// World X
				sprintf(buffer, "%.1f", creatureObj->getPosition_w().x);
				resultNarrowString += buffer;
				resultNarrowString += "\t";

				// World Y
				sprintf(buffer, "%.1f", creatureObj->getPosition_w().y);
				resultNarrowString += buffer;
				resultNarrowString += "\t";

				// World Z
				sprintf(buffer, "%.1f", creatureObj->getPosition_w().z);
				resultNarrowString += buffer;
				resultNarrowString += "\t";
			}

			// AI Information
			const AICreatureController * controller = dynamic_cast<const AICreatureController*>(creatureObj->getController());
			if (controller)
			{
				char buffer[512];

				// Creature name
				sprintf(buffer, "%s", controller->getCreatureName().getString());
				resultNarrowString += buffer;
				resultNarrowString += "\t";

				// Creature level
				sprintf(buffer, "%d", creatureObj->getLevel());
				resultNarrowString += buffer;
				resultNarrowString += "\t";

				// Invulnerable
				sprintf(buffer, "%s", creatureObj->isInvulnerable() ? "yes" : "no");
				resultNarrowString += buffer;
				resultNarrowString += "\t";

				// Hibernating
				sprintf(buffer, "%s", controller->getHibernate() ? "yes" : "no");
				resultNarrowString += buffer;
				resultNarrowString += "\t";

				// Movement state
				sprintf(buffer, "%s", AiMovementBase::getMovementString(controller->getMovementType()));
				resultNarrowString += buffer;
				resultNarrowString += "\t";
			}
			else
			{
				// Creature name
				resultNarrowString += "\t";

				// Creature level
				resultNarrowString += "\t";

				// Invulnerable
				resultNarrowString += "\t";

				// Hibernating
				resultNarrowString += "\t";

				// Movement state
				resultNarrowString += "\t";
			}

			// Observer count
			{
				char buffer[32];
				sprintf(buffer, "%d", creatureObj->getObserversCount());
				resultNarrowString += buffer;
				resultNarrowString += "\t";
			}

			// Observer list
			const std::set<Client *>& observerList = creatureObj->getObservers();
			std::set<Client *>::const_iterator j;
			for (j = observerList.begin(); j != observerList.end(); ++j)
			{
				const Client* observerClient = (*j);
				if (observerClient)
				{
					resultNarrowString += observerClient->getCharacterObjectId().getValueString();
					resultNarrowString += " ";
				}
			}

			// End the row of creature info
			resultNarrowString += "\n";
		}

		// Parse the parameters to get the filename
		const std::string & filename = Unicode::wideToNarrow(argv[1]);

		// Save the data in a text file on the client
		GenericValueTypeMessage<std::pair<std::string, std::string> > const msg("SaveTextOnClient", std::make_pair(filename, resultNarrowString));
		user->getClient()->send(msg, true);

		result += getErrorMessage(argv[0], ERR_SUCCESS);

		return true;
	}
	else if (isAbbrev(argv[0], "getNumberOfMoveObjectLists"))
	{
		char buffer[128];
		std::vector<int> moveListSize;

		snprintf(buffer, sizeof(buffer) - 1, "%d move object lists\n", ServerWorld::getNumMoveLists(moveListSize));
		buffer[sizeof(buffer) - 1] = '\0';
		result += Unicode::narrowToWide(buffer);

		int moveListIndex = 1;
		for (std::vector<int>::const_iterator i = moveListSize.begin(); i != moveListSize.end(); ++i)
		{
			snprintf(buffer, sizeof(buffer) - 1, "list #%d size: %d\n", moveListIndex++, *i);
			buffer[sizeof(buffer) - 1] = '\0';
			result += Unicode::narrowToWide(buffer);
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "setNumberOfMoveObjectLists"))
	{
		int numMoveLists = atoi(Unicode::wideToNarrow(argv[1]).c_str());

		ServerWorld::setNumMoveLists(numMoveLists);

		char buffer[128];
		snprintf(buffer, sizeof(buffer) - 1, "setting the number of move object lists to %d\n", numMoveLists);
		buffer[sizeof(buffer) - 1] = '\0';

		result += Unicode::narrowToWide(buffer);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "getSystemClockTimeFromAllGameServers"))
	{
		GenericValueTypeMessage<std::pair<uint32, NetworkId> > rsct(
			"ReportSystemClockTime", std::make_pair(GameServer::getInstance().getProcessId(), userId));
		GameServer::getInstance().sendToCentralServer(rsct);

		result += Unicode::narrowToWide("Asking every game server to report the system clock time of the box it is running on\n");
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "getPlanetaryTimeFromAllGameServers"))
	{
		GenericValueTypeMessage<std::pair<uint32, NetworkId> > rpt(
			"ReportPlanetaryTime", std::make_pair(GameServer::getInstance().getProcessId(), userId));
		GameServer::getInstance().sendToCentralServer(rpt);

		result += Unicode::narrowToWide("Asking every game server to report its planetary time\n");
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "requestSameServer"))
	{
		NetworkId oid1(Unicode::wideToNarrow(argv[1]));
		NetworkId oid2(Unicode::wideToNarrow(argv[2]));

		ServerObject const * const object1 = ServerWorld::findObjectByNetworkId(oid1);
		if (object1 == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		ServerObject const * const object2 = ServerWorld::findObjectByNetworkId(oid2);
		if (object2 == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		if (!object1->isAuthoritative() || !object2->isAuthoritative())
		{
			GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const rssMessage(
				"RequestSameServer",
				std::make_pair(oid1, oid2));

			GameServer::getInstance().sendToPlanetServer(rssMessage);
		}

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isAbbrev(argv[0], "displayTerrain"))
	{
		if (!user)
		{
			result += Unicode::narrowToWide("invalid user\n");
			return true;
		}

		int const range = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		if (range < 0)
		{
			result += Unicode::narrowToWide("range must be >= 0\n");
			return true;
		}

		Vector const location = user->findPosition_w();

		TerrainObject const * terrain = TerrainObject::getConstInstance();
		if (!terrain)
		{
			result += Unicode::narrowToWide("terrain object is nullptr\n");
			return true;
		}

		float const chunkWidthInMeters = terrain->getChunkWidthInMeters();
		if (chunkWidthInMeters < 0.1f)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("terrain chunk width %.2fm is too small for this operation\n", chunkWidthInMeters));
			return true;
		}

		int numberOfChunks = static_cast<int>(static_cast<float>(range) / chunkWidthInMeters);
		if (numberOfChunks > 71)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("the specified range (%dm) results in too many chunks (%d, limit is 71) of chunk width (%.2fm)\n", range, numberOfChunks, chunkWidthInMeters));
			return true;
		}

		float currentX = 0.0f;
		float currentZ = location.z + (numberOfChunks * chunkWidthInMeters);
		for (int z = -numberOfChunks; z <= numberOfChunks; ++z)
		{
			currentX = location.x - (numberOfChunks * chunkWidthInMeters);
			for (int x = -numberOfChunks; x <= numberOfChunks; ++x)
			{
				Vector const v(currentX, 0.0f, currentZ);
				float height;
				bool const hasChunk = terrain->getHeight(v, height); // this will fail if the terrain chunk is not yet generated

				if ((x == 0) && (z == 0))
				{
					if (hasChunk)
						result += Unicode::narrowToWide("*");
					else
						result += Unicode::narrowToWide("X");
				}
				else
				{
					if (hasChunk)
						result += Unicode::narrowToWide("+");
					else
						result += Unicode::narrowToWide("-");
				}

				currentX += chunkWidthInMeters;
			}

			currentZ -= chunkWidthInMeters;

			result += Unicode::narrowToWide("\n");
		}

		result += Unicode::narrowToWide("\n");
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("chunk width (%.2fm)\n", chunkWidthInMeters));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("x-range (%.2f, %.2f)\n", (location.x - (numberOfChunks * chunkWidthInMeters)), (currentX - chunkWidthInMeters)));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("z-range (%.2f, %.2f)\n", (location.z + (numberOfChunks * chunkWidthInMeters)), (currentZ + chunkWidthInMeters)));
	}
	else if (isAbbrev(argv[0], "generateTerrain"))
	{
		if (!user)
		{
			result += Unicode::narrowToWide("invalid user\n");
			return true;
		}

		int const range = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		if (range < 0)
		{
			result += Unicode::narrowToWide("range must be >= 0\n");
			return true;
		}

		Vector const location = user->findPosition_w();

		TerrainObject const * terrain = TerrainObject::getConstInstance();
		if (!terrain)
		{
			result += Unicode::narrowToWide("terrain object is nullptr\n");
			return true;
		}

		float const chunkWidthInMeters = terrain->getChunkWidthInMeters();
		if (chunkWidthInMeters < 0.1f)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("terrain chunk width %.2fm is too small for this operation\n", chunkWidthInMeters));
			return true;
		}

		int numberOfChunks = static_cast<int>(static_cast<float>(range) / chunkWidthInMeters);
		if (numberOfChunks > 71)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("the specified range (%dm) results in too many chunks (%d, limit is 71) of chunk width (%.2fm)\n", range, numberOfChunks, chunkWidthInMeters));
			return true;
		}

		float currentX = 0.0f;
		float currentZ = location.z + (numberOfChunks * chunkWidthInMeters);
		for (int z = -numberOfChunks; z <= numberOfChunks; ++z)
		{
			currentX = location.x - (numberOfChunks * chunkWidthInMeters);
			for (int x = -numberOfChunks; x <= numberOfChunks; ++x)
			{
				Vector const v(currentX, 0.0f, currentZ);
				float height;
				if (!terrain->getHeightForceChunkCreation(v, height)) // this will force terrain chunk creation, if necessary
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("getHeightForceChunkCreation() failed at (%.2f, %.2f)\n", currentX, currentZ));
				}

				currentX += chunkWidthInMeters;
			}

			currentZ -= chunkWidthInMeters;
		}

		result += Unicode::narrowToWide("\n");
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("chunk width (%.2fm)\n", chunkWidthInMeters));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("x-range (%.2f, %.2f)\n", (location.x - (numberOfChunks * chunkWidthInMeters)), (currentX - chunkWidthInMeters)));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("z-range (%.2f, %.2f)\n", (location.z + (numberOfChunks * chunkWidthInMeters)), (currentZ + chunkWidthInMeters)));
	}
	else if (isAbbrev(argv[0], "purgeTerrain"))
	{
		TerrainObject * terrain = TerrainObject::getInstance();
		if (!terrain)
		{
			result += Unicode::narrowToWide("terrain object is nullptr\n");
			return true;
		}

		result += Unicode::narrowToWide("purging all generated terrain chunks\n");
		terrain->purgeChunks();
	}
	else if (isAbbrev(argv[0], "listConnectedCharacters"))
	{
		std::multimap<std::pair<Unicode::String, NetworkId>, Unicode::String> orderedList;
		std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();
		for (std::map<NetworkId, LfgCharacterData>::const_iterator iter = connectedCharacterLfgData.begin(); iter != connectedCharacterLfgData.end(); ++iter)
		{
			IGNORE_RETURN(orderedList.insert(std::make_pair(std::make_pair(Unicode::toUpper(iter->second.characterName), iter->first), iter->second.characterName)));
		}

		for (std::multimap<std::pair<Unicode::String, NetworkId>, Unicode::String>::const_iterator iter2 = orderedList.begin(); iter2 != orderedList.end(); ++iter2)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s, %s\n", Unicode::wideToNarrow(iter2->second).c_str(), iter2->first.second.getValueString().c_str()));
		}

		if (!connectedCharacterLfgData.empty())
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("\nPopulation statistics: %d total\n", connectedCharacterLfgData.size()));

			std::map<std::string, int> const & statistics = LfgCharacterData::calculateStatistics(connectedCharacterLfgData);
			std::map<std::string, int>::const_iterator iterStatistics;

			size_t maxStatisticsLength = 0;
			for (iterStatistics = statistics.begin(); iterStatistics != statistics.end(); ++iterStatistics)
			{
				if ((iterStatistics->second >= 0) && (iterStatistics->first.size() > maxStatisticsLength))
					maxStatisticsLength = iterStatistics->first.size();
			}

			for (iterStatistics = statistics.begin(); iterStatistics != statistics.end(); ++iterStatistics)
			{
				if (iterStatistics->second < 0)
				{
					result += Unicode::narrowToWide("\n");
					continue;
				}

				result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s", iterStatistics->first.c_str()));

				std::string padding;
				for (size_t i = iterStatistics->first.size(); i < maxStatisticsLength; ++i)
					padding += ".";

				if (!padding.empty())
					result += Unicode::narrowToWide(padding);

				result += Unicode::narrowToWide(FormattedString<512>().sprintf(": %4d\n", iterStatistics->second));
			}
		}
	}
	else if (isAbbrev(argv[0], "listConnectedCharacterData"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData = ServerUniverse::getConnectedCharacterLfgData();

		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = connectedCharacterLfgData.find(oid);
		if (iterFind != connectedCharacterLfgData.end())
		{
			result += Unicode::narrowToWide(iterFind->second.getDebugString());

			ServerObject const * const soGroupObject = (iterFind->second.groupId.isValid() ? safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(iterFind->second.groupId)) : nullptr);
			GroupObject const * const groupObject = (soGroupObject ? soGroupObject->asGroupObject() : nullptr);
			if (groupObject)
			{
				unsigned int const secondsLeftOnGroupPickup = groupObject->getSecondsLeftOnGroupPickup();
				if (secondsLeftOnGroupPickup)
				{
					std::pair<int32, int32> const & groupPickupTimer = groupObject->getGroupPickupTimer();
					std::pair<std::string, Vector> const & groupPickupLocation = groupObject->getGroupPickupLocation();

					result += Unicode::narrowToWide(FormattedString<512>().sprintf("Group (%s) Pickup Point: %s (%.2f, %.2f, %.2f), (%ld - %ld) (%u seconds left)\n", groupObject->getNetworkId().getValueString().c_str(), groupPickupLocation.first.c_str(), groupPickupLocation.second.x, groupPickupLocation.second.y, groupPickupLocation.second.z, groupPickupTimer.first, groupPickupTimer.second, secondsLeftOnGroupPickup));
				}

				result += Unicode::narrowToWide(FormattedString<512>().sprintf("Group (%s) Members:\n", groupObject->getNetworkId().getValueString().c_str()));

				std::multimap<std::pair<std::string, NetworkId>, std::string> orderedMemberList;
				NetworkId const groupLeaderId = groupObject->getGroupLeaderId();
				GroupObject::GroupMemberVector const & groupMembers = groupObject->getGroupMembers();
				unsigned int groupMemberIndex = 0;
				for (GroupObject::GroupMemberVector::const_iterator iter = groupMembers.begin(); iter != groupMembers.end(); ++iter)
				{
					std::string groupMemberName = iter->second;
					if (iter->first == groupLeaderId)
						groupMemberName += " (Leader)";
					if (!groupObject->isMemberPC(iter->first))
						groupMemberName += " (NPC)";

					groupMemberName += FormattedString<512>().sprintf(" (%s), %d, %s", iter->first.getValueString().c_str(), groupObject->getMemberLevelByMemberIndex(groupMemberIndex), LfgCharacterData::getProfessionDebugString(static_cast<LfgCharacterData::Profession>(groupObject->getMemberProfessionByMemberIndex(groupMemberIndex))).c_str());

					++groupMemberIndex;

					IGNORE_RETURN(orderedMemberList.insert(std::make_pair(std::make_pair(Unicode::toUpper(iter->second), iter->first), groupMemberName)));
				}

				for (std::multimap<std::pair<std::string, NetworkId>, std::string>::const_iterator iter2 = orderedMemberList.begin(); iter2 != orderedMemberList.end(); ++iter2)
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("    %s\n", iter2->second.c_str()));
				}
			}
			else
			{
				if (iterFind->second.groupId.isValid())
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("(invalid group %s)\n", iterFind->second.groupId.getValueString().c_str()));
				else
					result += Unicode::narrowToWide("(ungrouped)\n");
			}

			std::map<NetworkId, Unicode::String> const & connectedCharacterBiographyData = ServerUniverse::getConnectedCharacterBiographyData();
			std::map<NetworkId, Unicode::String>::const_iterator iterFindBiography = connectedCharacterBiographyData.find(oid);
			if (iterFindBiography != connectedCharacterBiographyData.end())
			{
				result += Unicode::narrowToWide("Biography:\n");
				result += iterFindBiography->second;
				result += Unicode::narrowToWide("\n");
			}
			else
			{
				result += Unicode::narrowToWide("(no biography)\n");
			}
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("no connected character with oid %s\n", oid.getValueString().c_str()));
		}
	}
	else if (isAbbrev(argv[0], "listTravelPoints"))
	{
		std::string planetName;
		Vector location;
		if (argv.size() >= 5)
		{
			planetName = Unicode::wideToNarrow(argv[1]);
			location.x = static_cast<real>(atof(Unicode::wideToNarrow(argv[2]).c_str()));
			location.y = static_cast<real>(atof(Unicode::wideToNarrow(argv[3]).c_str()));
			location.z = static_cast<real>(atof(Unicode::wideToNarrow(argv[4]).c_str()));
		}
		else
		{
			planetName = ServerWorld::getSceneId();

			if (user)
				location = user->findPosition_w();
		}

		PlanetObject const * const planetObject = ServerUniverse::getInstance().getPlanetByName(planetName);
		if (planetObject)
		{
			std::multimap<float, TravelPoint const *> sortedTravelPoint;
			int const numberOfTravelPoints = planetObject->getNumberOfTravelPoints();
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d travel points for planet (%s)\n", numberOfTravelPoints, planetName.c_str()));

			for (int i = 0; i < numberOfTravelPoints; ++i)
			{
				TravelPoint const * tp = planetObject->getTravelPoint(i);
				if (tp)
					IGNORE_RETURN(sortedTravelPoint.insert(std::make_pair(location.magnitudeBetween(tp->getPosition_w()), tp)));
			}

			for (std::multimap<float, TravelPoint const *>::const_iterator iter = sortedTravelPoint.begin(); iter != sortedTravelPoint.end(); ++iter)
			{
				std::string tpType;
				if (iter->second->getType() == TravelPoint::TPT_Unknown)
				{
					tpType = "TPT_Unknown";
				}
				else
				{
					if (iter->second->getType() & TravelPoint::TPT_NPC_Starport)
					{
						tpType = "TPT_NPC_Starport";
					}

					if (iter->second->getType() & TravelPoint::TPT_NPC_Shuttleport)
					{
						tpType = "TPT_NPC_Shuttleport";
					}

					if (iter->second->getType() & TravelPoint::TPT_NPC_StaticBaseBeacon)
					{
						tpType = "TPT_NPC_StaticBaseBeacon";
					}

					if (iter->second->getType() & TravelPoint::TPT_PC_Shuttleport)
					{
						tpType = "TPT_PC_Shuttleport";
					}

					if (iter->second->getType() & TravelPoint::TPT_PC_CampShuttleBeacon)
					{
						tpType = "TPT_PC_CampShuttleBeacon";
					}
				}

				result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s (%s) (%s) (%s) (%.2f, %.2f, %.2f) (cost=%d) [%.2fm away from (%.2f, %.2f, %.2f)]\n", iter->second->getName().c_str(), iter->second->getGcwContestedRegion().c_str(), tpType.c_str(), (iter->second->getInterplanetary() ? "interplanetary" : "intraplanetary"), iter->second->getPosition_w().x, iter->second->getPosition_w().y, iter->second->getPosition_w().z, iter->second->getCost(), iter->first, location.x, location.y, location.z));
			}
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("couldn't find PlanetObject for planet (%s)\n", planetName.c_str()));
		}
	}
	else if (isAbbrev(argv[0], "listCharacterLastLoginTimeBrief"))
	{
		int const days = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		time_t const cutoff = std::max(static_cast<time_t>(0), static_cast<time_t>(::time(nullptr) - (60 * 60 * 24 * days)));

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithLastLoginTimeAfter(cutoff, resultList);

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters have logged in since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterLastLoginTimeAfterBrief"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoff = ::mktime(timeinfo);

		if (cutoff <= 0)
		{
			result += Unicode::narrowToWide("specified time is invalid\n");
			return true;
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithLastLoginTimeAfter(cutoff, resultList);

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters have logged in since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterLastLoginTimeBeforeBrief"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoff = ::mktime(timeinfo);

		if (cutoff <= 0)
		{
			result += Unicode::narrowToWide("specified time is invalid\n");
			return true;
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithLastLoginTimeBefore(cutoff, resultList);

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters logged in before %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterLastLoginTimeBetweenBrief"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoffLower = ::mktime(timeinfo);

		if (cutoffLower <= 0)
		{
			result += Unicode::narrowToWide("specified start time is invalid\n");
			return true;
		}

		timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[7]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[8]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[9]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[10]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[11]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[12]).c_str());
		time_t const cutoffUpper = ::mktime(timeinfo);

		if (cutoffUpper <= 0)
		{
			result += Unicode::narrowToWide("specified end time is invalid\n");
			return true;
		}

		if (cutoffLower > cutoffUpper)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("specified start time (%s, %s) is after specified end time (%s, %s)\n", CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));
			return true;
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithLastLoginTimeBetween(cutoffLower, cutoffUpper, resultList);

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters logged in between %s, %s and %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterLastLoginTimeDetailed"))
	{
		int const days = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		time_t const cutoff = std::max(static_cast<time_t>(0), static_cast<time_t>(::time(nullptr) - (60 * 60 * 24 * days)));

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithLastLoginTimeAfter(cutoff, resultList);

		for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("[%s, %s] %s, %lu, %s\n", CalendarTime::convertEpochToTimeStringLocal(iter->first).c_str(), CalendarTime::convertEpochToTimeStringGMT(iter->first).c_str(), iter->second.first.first.getValueString().c_str(), iter->second.first.second, iter->second.second.c_str()));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters have logged in since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterLastLoginTimeAfterDetailed"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoff = ::mktime(timeinfo);

		if (cutoff <= 0)
		{
			result += Unicode::narrowToWide("specified time is invalid\n");
			return true;
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithLastLoginTimeAfter(cutoff, resultList);

		for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("[%s, %s] %s, %lu, %s\n", CalendarTime::convertEpochToTimeStringLocal(iter->first).c_str(), CalendarTime::convertEpochToTimeStringGMT(iter->first).c_str(), iter->second.first.first.getValueString().c_str(), iter->second.first.second, iter->second.second.c_str()));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters have logged in since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterLastLoginTimeBeforeDetailed"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoff = ::mktime(timeinfo);

		if (cutoff <= 0)
		{
			result += Unicode::narrowToWide("specified time is invalid\n");
			return true;
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithLastLoginTimeBefore(cutoff, resultList);

		for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("[%s, %s] %s, %lu, %s\n", CalendarTime::convertEpochToTimeStringLocal(iter->first).c_str(), CalendarTime::convertEpochToTimeStringGMT(iter->first).c_str(), iter->second.first.first.getValueString().c_str(), iter->second.first.second, iter->second.second.c_str()));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters logged in before %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterLastLoginTimeBetweenDetailed"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoffLower = ::mktime(timeinfo);

		if (cutoffLower <= 0)
		{
			result += Unicode::narrowToWide("specified start time is invalid\n");
			return true;
		}

		timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[7]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[8]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[9]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[10]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[11]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[12]).c_str());
		time_t const cutoffUpper = ::mktime(timeinfo);

		if (cutoffUpper <= 0)
		{
			result += Unicode::narrowToWide("specified end time is invalid\n");
			return true;
		}

		if (cutoffLower > cutoffUpper)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("specified start time (%s, %s) is after specified end time (%s, %s)\n", CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));
			return true;
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithLastLoginTimeBetween(cutoffLower, cutoffUpper, resultList);

		for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("[%s, %s] %s, %lu, %s\n", CalendarTime::convertEpochToTimeStringLocal(iter->first).c_str(), CalendarTime::convertEpochToTimeStringGMT(iter->first).c_str(), iter->second.first.first.getValueString().c_str(), iter->second.first.second, iter->second.second.c_str()));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters logged in between %s, %s and %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));
	}
	else if (isAbbrev(argv[0], "sendMailToCharacterLastLoginTime"))
	{
		int const days = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		time_t const cutoff = std::max(static_cast<time_t>(0), static_cast<time_t>(::time(nullptr) - (60 * 60 * 24 * days)));

		int const argc = argv.size();
		Unicode::String mailBody;
		for (int i = 4; i < argc; ++i)
		{
			if (i > 4)
				mailBody += Unicode::narrowToWide("\n");

			mailBody += argv[i];
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithLastLoginTimeAfter(cutoff, resultList);

		// to prevent accidentally sending mass spam to muliple players, command must be entered
		// a second time as confirmation with the exact same parameters within 60 seconds
		static int s_days = -1;
		static std::string s_fromName;
		static Unicode::String s_mailSubject;
		static Unicode::String s_mailBody;
		static time_t s_confirmationTimeout = 0;

		time_t const timeNow = ::time(nullptr);

		if ((s_days == days) && (s_confirmationTimeout > timeNow) && (s_fromName == Unicode::wideToNarrow(argv[2])) && (s_mailSubject == argv[3]) && (s_mailBody == mailBody))
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sending mail to %d (out of %d total) characters who have logged in since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));

			for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			{
				if (!iter->second.second.empty())
					Chat::sendPersistentMessage(s_fromName, iter->second.second, s_mailSubject, s_mailBody, Unicode::String());
			}

			s_days = -1;
			s_fromName.clear();
			s_mailSubject.clear();
			s_mailBody.clear();
			s_confirmationTimeout = 0;
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("The following is the mail that ***WILL BE SENT*** to %d (out of %d total) characters who have logged in since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
			result += Unicode::narrowToWide("\nPlease confirm that you ***REALLY WANT TO SEND*** this mail and run the command again with the same arguments within 60 seconds to send the mail\n\n");

			s_days = days;
			s_fromName = Unicode::wideToNarrow(argv[2]);
			s_mailSubject = argv[3];
			s_mailBody = mailBody;
			s_confirmationTimeout = timeNow + 60;

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sender: %s\n", s_fromName.c_str()));
			result += Unicode::narrowToWide("Subject: ");
			result += s_mailSubject;
			result += Unicode::narrowToWide("\n\n");
			result += s_mailBody;
			result += Unicode::narrowToWide("\n");
		}
	}
	else if (isAbbrev(argv[0], "sendMailToCharacterLastLoginTimeAfter"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoff = ::mktime(timeinfo);

		if (cutoff <= 0)
		{
			result += Unicode::narrowToWide("specified time is invalid\n");
			return true;
		}

		int const argc = argv.size();
		Unicode::String mailBody;
		for (int i = 9; i < argc; ++i)
		{
			if (i > 9)
				mailBody += Unicode::narrowToWide("\n");

			mailBody += argv[i];
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithLastLoginTimeAfter(cutoff, resultList);

		// to prevent accidentally sending mass spam to muliple players, command must be entered
		// a second time as confirmation with the exact same parameters within 60 seconds
		static time_t s_cutoff = -1;
		static std::string s_fromName;
		static Unicode::String s_mailSubject;
		static Unicode::String s_mailBody;
		static time_t s_confirmationTimeout = 0;

		time_t const timeNow = ::time(nullptr);

		if ((s_cutoff == cutoff) && (s_confirmationTimeout > timeNow) && (s_fromName == Unicode::wideToNarrow(argv[7])) && (s_mailSubject == argv[8]) && (s_mailBody == mailBody))
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sending mail to %d (out of %d total) characters who have logged in since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));

			for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			{
				if (!iter->second.second.empty())
					Chat::sendPersistentMessage(s_fromName, iter->second.second, s_mailSubject, s_mailBody, Unicode::String());
			}

			s_cutoff = -1;
			s_fromName.clear();
			s_mailSubject.clear();
			s_mailBody.clear();
			s_confirmationTimeout = 0;
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("The following is the mail that ***WILL BE SENT*** to %d (out of %d total) characters who have logged in since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
			result += Unicode::narrowToWide("\nPlease confirm that you ***REALLY WANT TO SEND*** this mail and run the command again with the same arguments within 60 seconds to send the mail\n\n");

			s_cutoff = cutoff;
			s_fromName = Unicode::wideToNarrow(argv[7]);
			s_mailSubject = argv[8];
			s_mailBody = mailBody;
			s_confirmationTimeout = timeNow + 60;

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sender: %s\n", s_fromName.c_str()));
			result += Unicode::narrowToWide("Subject: ");
			result += s_mailSubject;
			result += Unicode::narrowToWide("\n\n");
			result += s_mailBody;
			result += Unicode::narrowToWide("\n");
		}
	}
	else if (isAbbrev(argv[0], "sendMailToCharacterLastLoginTimeBefore"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoff = ::mktime(timeinfo);

		if (cutoff <= 0)
		{
			result += Unicode::narrowToWide("specified time is invalid\n");
			return true;
		}

		int const argc = argv.size();
		Unicode::String mailBody;
		for (int i = 9; i < argc; ++i)
		{
			if (i > 9)
				mailBody += Unicode::narrowToWide("\n");

			mailBody += argv[i];
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithLastLoginTimeBefore(cutoff, resultList);

		// to prevent accidentally sending mass spam to muliple players, command must be entered
		// a second time as confirmation with the exact same parameters within 60 seconds
		static time_t s_cutoff = -1;
		static std::string s_fromName;
		static Unicode::String s_mailSubject;
		static Unicode::String s_mailBody;
		static time_t s_confirmationTimeout = 0;

		time_t const timeNow = ::time(nullptr);

		if ((s_cutoff == cutoff) && (s_confirmationTimeout > timeNow) && (s_fromName == Unicode::wideToNarrow(argv[7])) && (s_mailSubject == argv[8]) && (s_mailBody == mailBody))
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sending mail to %d (out of %d total) characters who logged in before %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));

			for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			{
				if (!iter->second.second.empty())
					Chat::sendPersistentMessage(s_fromName, iter->second.second, s_mailSubject, s_mailBody, Unicode::String());
			}

			s_cutoff = -1;
			s_fromName.clear();
			s_mailSubject.clear();
			s_mailBody.clear();
			s_confirmationTimeout = 0;
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("The following is the mail that ***WILL BE SENT*** to %d (out of %d total) characters who logged in before %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
			result += Unicode::narrowToWide("\nPlease confirm that you ***REALLY WANT TO SEND*** this mail and run the command again with the same arguments within 60 seconds to send the mail\n\n");

			s_cutoff = cutoff;
			s_fromName = Unicode::wideToNarrow(argv[7]);
			s_mailSubject = argv[8];
			s_mailBody = mailBody;
			s_confirmationTimeout = timeNow + 60;

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sender: %s\n", s_fromName.c_str()));
			result += Unicode::narrowToWide("Subject: ");
			result += s_mailSubject;
			result += Unicode::narrowToWide("\n\n");
			result += s_mailBody;
			result += Unicode::narrowToWide("\n");
		}
	}
	else if (isAbbrev(argv[0], "sendMailToCharacterLastLoginTimeBetween"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoffLower = ::mktime(timeinfo);

		if (cutoffLower <= 0)
		{
			result += Unicode::narrowToWide("specified start time is invalid\n");
			return true;
		}

		timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[7]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[8]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[9]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[10]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[11]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[12]).c_str());
		time_t const cutoffUpper = ::mktime(timeinfo);

		if (cutoffUpper <= 0)
		{
			result += Unicode::narrowToWide("specified end time is invalid\n");
			return true;
		}

		if (cutoffLower > cutoffUpper)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("specified start time (%s, %s) is after specified end time (%s, %s)\n", CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));
			return true;
		}

		int const argc = argv.size();
		Unicode::String mailBody;
		for (int i = 15; i < argc; ++i)
		{
			if (i > 15)
				mailBody += Unicode::narrowToWide("\n");

			mailBody += argv[i];
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithLastLoginTimeBetween(cutoffLower, cutoffUpper, resultList);

		// to prevent accidentally sending mass spam to muliple players, command must be entered
		// a second time as confirmation with the exact same parameters within 60 seconds
		static time_t s_cutoffLower = -1;
		static time_t s_cutoffUpper = -1;
		static std::string s_fromName;
		static Unicode::String s_mailSubject;
		static Unicode::String s_mailBody;
		static time_t s_confirmationTimeout = 0;

		time_t const timeNow = ::time(nullptr);

		if ((s_cutoffLower == cutoffLower) && (s_cutoffUpper == cutoffUpper) && (s_confirmationTimeout > timeNow) && (s_fromName == Unicode::wideToNarrow(argv[13])) && (s_mailSubject == argv[14]) && (s_mailBody == mailBody))
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sending mail to %d (out of %d total) characters who logged in between %s, %s and %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));

			for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			{
				if (!iter->second.second.empty())
					Chat::sendPersistentMessage(s_fromName, iter->second.second, s_mailSubject, s_mailBody, Unicode::String());
			}

			s_cutoffLower = -1;
			s_cutoffUpper = -1;
			s_fromName.clear();
			s_mailSubject.clear();
			s_mailBody.clear();
			s_confirmationTimeout = 0;
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("The following is the mail that ***WILL BE SENT*** to %d (out of %d total) characters who logged in between %s, %s and %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));
			result += Unicode::narrowToWide("\nPlease confirm that you ***REALLY WANT TO SEND*** this mail and run the command again with the same arguments within 60 seconds to send the mail\n\n");

			s_cutoffLower = cutoffLower;
			s_cutoffUpper = cutoffUpper;
			s_fromName = Unicode::wideToNarrow(argv[13]);
			s_mailSubject = argv[14];
			s_mailBody = mailBody;
			s_confirmationTimeout = timeNow + 60;

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sender: %s\n", s_fromName.c_str()));
			result += Unicode::narrowToWide("Subject: ");
			result += s_mailSubject;
			result += Unicode::narrowToWide("\n\n");
			result += s_mailBody;
			result += Unicode::narrowToWide("\n");
		}
	}
	else if (isAbbrev(argv[0], "listCharacterCreateTimeBrief"))
	{
		int const days = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		time_t const cutoff = std::max(static_cast<time_t>(0), static_cast<time_t>(::time(nullptr) - (60 * 60 * 24 * days)));

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithCreateTimeAfter(cutoff, resultList);

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters created since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterCreateTimeAfterBrief"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoff = ::mktime(timeinfo);

		if (cutoff <= 0)
		{
			result += Unicode::narrowToWide("specified time is invalid\n");
			return true;
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithCreateTimeAfter(cutoff, resultList);

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters created since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterCreateTimeBeforeBrief"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoff = ::mktime(timeinfo);

		if (cutoff <= 0)
		{
			result += Unicode::narrowToWide("specified time is invalid\n");
			return true;
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithCreateTimeBefore(cutoff, resultList);

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters created before %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterCreateTimeBetweenBrief"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoffLower = ::mktime(timeinfo);

		if (cutoffLower <= 0)
		{
			result += Unicode::narrowToWide("specified start time is invalid\n");
			return true;
		}

		timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[7]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[8]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[9]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[10]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[11]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[12]).c_str());
		time_t const cutoffUpper = ::mktime(timeinfo);

		if (cutoffUpper <= 0)
		{
			result += Unicode::narrowToWide("specified end time is invalid\n");
			return true;
		}

		if (cutoffLower > cutoffUpper)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("specified start time (%s, %s) is after specified end time (%s, %s)\n", CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));
			return true;
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithCreateTimeBetween(cutoffLower, cutoffUpper, resultList);

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters created between %s, %s and %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterCreateTimeDetailed"))
	{
		int const days = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		time_t const cutoff = std::max(static_cast<time_t>(0), static_cast<time_t>(::time(nullptr) - (60 * 60 * 24 * days)));

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithCreateTimeAfter(cutoff, resultList);

		for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("[%s, %s] %s, %lu, %s\n", CalendarTime::convertEpochToTimeStringLocal(iter->first).c_str(), CalendarTime::convertEpochToTimeStringGMT(iter->first).c_str(), iter->second.first.first.getValueString().c_str(), iter->second.first.second, iter->second.second.c_str()));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters created since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterCreateTimeAfterDetailed"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoff = ::mktime(timeinfo);

		if (cutoff <= 0)
		{
			result += Unicode::narrowToWide("specified time is invalid\n");
			return true;
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithCreateTimeAfter(cutoff, resultList);

		for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("[%s, %s] %s, %lu, %s\n", CalendarTime::convertEpochToTimeStringLocal(iter->first).c_str(), CalendarTime::convertEpochToTimeStringGMT(iter->first).c_str(), iter->second.first.first.getValueString().c_str(), iter->second.first.second, iter->second.second.c_str()));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters created since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterCreateTimeBeforeDetailed"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoff = ::mktime(timeinfo);

		if (cutoff <= 0)
		{
			result += Unicode::narrowToWide("specified time is invalid\n");
			return true;
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithCreateTimeBefore(cutoff, resultList);

		for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("[%s, %s] %s, %lu, %s\n", CalendarTime::convertEpochToTimeStringLocal(iter->first).c_str(), CalendarTime::convertEpochToTimeStringGMT(iter->first).c_str(), iter->second.first.first.getValueString().c_str(), iter->second.first.second, iter->second.second.c_str()));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters created before %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
	}
	else if (isAbbrev(argv[0], "listCharacterCreateTimeBetweenDetailed"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoffLower = ::mktime(timeinfo);

		if (cutoffLower <= 0)
		{
			result += Unicode::narrowToWide("specified start time is invalid\n");
			return true;
		}

		timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[7]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[8]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[9]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[10]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[11]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[12]).c_str());
		time_t const cutoffUpper = ::mktime(timeinfo);

		if (cutoffUpper <= 0)
		{
			result += Unicode::narrowToWide("specified end time is invalid\n");
			return true;
		}

		if (cutoffLower > cutoffUpper)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("specified start time (%s, %s) is after specified end time (%s, %s)\n", CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));
			return true;
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithCreateTimeBetween(cutoffLower, cutoffUpper, resultList);

		for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("[%s, %s] %s, %lu, %s\n", CalendarTime::convertEpochToTimeStringLocal(iter->first).c_str(), CalendarTime::convertEpochToTimeStringGMT(iter->first).c_str(), iter->second.first.first.getValueString().c_str(), iter->second.first.second, iter->second.second.c_str()));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d (out of %d total) characters created between %s, %s and %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));
	}
	else if (isAbbrev(argv[0], "sendMailToCharacterCreateTime"))
	{
		int const days = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		time_t const cutoff = std::max(static_cast<time_t>(0), static_cast<time_t>(::time(nullptr) - (60 * 60 * 24 * days)));

		int const argc = argv.size();
		Unicode::String mailBody;
		for (int i = 4; i < argc; ++i)
		{
			if (i > 4)
				mailBody += Unicode::narrowToWide("\n");

			mailBody += argv[i];
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithCreateTimeAfter(cutoff, resultList);

		// to prevent accidentally sending mass spam to muliple players, command must be entered
		// a second time as confirmation with the exact same parameters within 60 seconds
		static int s_days = -1;
		static std::string s_fromName;
		static Unicode::String s_mailSubject;
		static Unicode::String s_mailBody;
		static time_t s_confirmationTimeout = 0;

		time_t const timeNow = ::time(nullptr);

		if ((s_days == days) && (s_confirmationTimeout > timeNow) && (s_fromName == Unicode::wideToNarrow(argv[2])) && (s_mailSubject == argv[3]) && (s_mailBody == mailBody))
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sending mail to %d (out of %d total) characters created since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));

			for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			{
				if (!iter->second.second.empty())
					Chat::sendPersistentMessage(s_fromName, iter->second.second, s_mailSubject, s_mailBody, Unicode::String());
			}

			s_days = -1;
			s_fromName.clear();
			s_mailSubject.clear();
			s_mailBody.clear();
			s_confirmationTimeout = 0;
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("The following is the mail that ***WILL BE SENT*** to %d (out of %d total) characters created since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
			result += Unicode::narrowToWide("\nPlease confirm that you ***REALLY WANT TO SEND*** this mail and run the command again with the same arguments within 60 seconds to send the mail\n\n");

			s_days = days;
			s_fromName = Unicode::wideToNarrow(argv[2]);
			s_mailSubject = argv[3];
			s_mailBody = mailBody;
			s_confirmationTimeout = timeNow + 60;

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sender: %s\n", s_fromName.c_str()));
			result += Unicode::narrowToWide("Subject: ");
			result += s_mailSubject;
			result += Unicode::narrowToWide("\n\n");
			result += s_mailBody;
			result += Unicode::narrowToWide("\n");
		}
	}
	else if (isAbbrev(argv[0], "sendMailToCharacterCreateTimeAfter"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoff = ::mktime(timeinfo);

		if (cutoff <= 0)
		{
			result += Unicode::narrowToWide("specified time is invalid\n");
			return true;
		}

		int const argc = argv.size();
		Unicode::String mailBody;
		for (int i = 9; i < argc; ++i)
		{
			if (i > 9)
				mailBody += Unicode::narrowToWide("\n");

			mailBody += argv[i];
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithCreateTimeAfter(cutoff, resultList);

		// to prevent accidentally sending mass spam to muliple players, command must be entered
		// a second time as confirmation with the exact same parameters within 60 seconds
		static time_t s_cutoff = -1;
		static std::string s_fromName;
		static Unicode::String s_mailSubject;
		static Unicode::String s_mailBody;
		static time_t s_confirmationTimeout = 0;

		time_t const timeNow = ::time(nullptr);

		if ((s_cutoff == cutoff) && (s_confirmationTimeout > timeNow) && (s_fromName == Unicode::wideToNarrow(argv[7])) && (s_mailSubject == argv[8]) && (s_mailBody == mailBody))
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sending mail to %d (out of %d total) characters created since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));

			for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			{
				if (!iter->second.second.empty())
					Chat::sendPersistentMessage(s_fromName, iter->second.second, s_mailSubject, s_mailBody, Unicode::String());
			}

			s_cutoff = -1;
			s_fromName.clear();
			s_mailSubject.clear();
			s_mailBody.clear();
			s_confirmationTimeout = 0;
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("The following is the mail that ***WILL BE SENT*** to %d (out of %d total) characters created since %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
			result += Unicode::narrowToWide("\nPlease confirm that you ***REALLY WANT TO SEND*** this mail and run the command again with the same arguments within 60 seconds to send the mail\n\n");

			s_cutoff = cutoff;
			s_fromName = Unicode::wideToNarrow(argv[7]);
			s_mailSubject = argv[8];
			s_mailBody = mailBody;
			s_confirmationTimeout = timeNow + 60;

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sender: %s\n", s_fromName.c_str()));
			result += Unicode::narrowToWide("Subject: ");
			result += s_mailSubject;
			result += Unicode::narrowToWide("\n\n");
			result += s_mailBody;
			result += Unicode::narrowToWide("\n");
		}
	}
	else if (isAbbrev(argv[0], "sendMailToCharacterCreateTimeBefore"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoff = ::mktime(timeinfo);

		if (cutoff <= 0)
		{
			result += Unicode::narrowToWide("specified time is invalid\n");
			return true;
		}

		int const argc = argv.size();
		Unicode::String mailBody;
		for (int i = 9; i < argc; ++i)
		{
			if (i > 9)
				mailBody += Unicode::narrowToWide("\n");

			mailBody += argv[i];
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithCreateTimeBefore(cutoff, resultList);

		// to prevent accidentally sending mass spam to muliple players, command must be entered
		// a second time as confirmation with the exact same parameters within 60 seconds
		static time_t s_cutoff = -1;
		static std::string s_fromName;
		static Unicode::String s_mailSubject;
		static Unicode::String s_mailBody;
		static time_t s_confirmationTimeout = 0;

		time_t const timeNow = ::time(nullptr);

		if ((s_cutoff == cutoff) && (s_confirmationTimeout > timeNow) && (s_fromName == Unicode::wideToNarrow(argv[7])) && (s_mailSubject == argv[8]) && (s_mailBody == mailBody))
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sending mail to %d (out of %d total) characters created before %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));

			for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			{
				if (!iter->second.second.empty())
					Chat::sendPersistentMessage(s_fromName, iter->second.second, s_mailSubject, s_mailBody, Unicode::String());
			}

			s_cutoff = -1;
			s_fromName.clear();
			s_mailSubject.clear();
			s_mailBody.clear();
			s_confirmationTimeout = 0;
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("The following is the mail that ***WILL BE SENT*** to %d (out of %d total) characters created before %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoff).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoff).c_str()));
			result += Unicode::narrowToWide("\nPlease confirm that you ***REALLY WANT TO SEND*** this mail and run the command again with the same arguments within 60 seconds to send the mail\n\n");

			s_cutoff = cutoff;
			s_fromName = Unicode::wideToNarrow(argv[7]);
			s_mailSubject = argv[8];
			s_mailBody = mailBody;
			s_confirmationTimeout = timeNow + 60;

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sender: %s\n", s_fromName.c_str()));
			result += Unicode::narrowToWide("Subject: ");
			result += s_mailSubject;
			result += Unicode::narrowToWide("\n\n");
			result += s_mailBody;
			result += Unicode::narrowToWide("\n");
		}
	}
	else if (isAbbrev(argv[0], "sendMailToCharacterCreateTimeBetween"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[1]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[3]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		time_t const cutoffLower = ::mktime(timeinfo);

		if (cutoffLower <= 0)
		{
			result += Unicode::narrowToWide("specified start time is invalid\n");
			return true;
		}

		timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[7]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[8]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[9]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[10]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[11]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[12]).c_str());
		time_t const cutoffUpper = ::mktime(timeinfo);

		if (cutoffUpper <= 0)
		{
			result += Unicode::narrowToWide("specified end time is invalid\n");
			return true;
		}

		if (cutoffLower > cutoffUpper)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("specified start time (%s, %s) is after specified end time (%s, %s)\n", CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));
			return true;
		}

		int const argc = argv.size();
		Unicode::String mailBody;
		for (int i = 15; i < argc; ++i)
		{
			if (i > 15)
				mailBody += Unicode::narrowToWide("\n");

			mailBody += argv[i];
		}

		std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> > resultList;
		NameManager::getInstance().getPlayerWithCreateTimeBetween(cutoffLower, cutoffUpper, resultList);

		// to prevent accidentally sending mass spam to muliple players, command must be entered
		// a second time as confirmation with the exact same parameters within 60 seconds
		static time_t s_cutoffLower = -1;
		static time_t s_cutoffUpper = -1;
		static std::string s_fromName;
		static Unicode::String s_mailSubject;
		static Unicode::String s_mailBody;
		static time_t s_confirmationTimeout = 0;

		time_t const timeNow = ::time(nullptr);

		if ((s_cutoffLower == cutoffLower) && (s_cutoffUpper == cutoffUpper) && (s_confirmationTimeout > timeNow) && (s_fromName == Unicode::wideToNarrow(argv[13])) && (s_mailSubject == argv[14]) && (s_mailBody == mailBody))
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sending mail to %d (out of %d total) characters created between %s, %s and %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));

			for (std::multimap<time_t, std::pair<std::pair<NetworkId, uint32>, std::string> >::const_iterator iter = resultList.begin(); iter != resultList.end(); ++iter)
			{
				if (!iter->second.second.empty())
					Chat::sendPersistentMessage(s_fromName, iter->second.second, s_mailSubject, s_mailBody, Unicode::String());
			}

			s_cutoffLower = -1;
			s_cutoffUpper = -1;
			s_fromName.clear();
			s_mailSubject.clear();
			s_mailBody.clear();
			s_confirmationTimeout = 0;
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("The following is the mail that ***WILL BE SENT*** to %d (out of %d total) characters created between %s, %s and %s, %s\n", resultList.size(), NameManager::getInstance().getTotalPlayerCount(), CalendarTime::convertEpochToTimeStringLocal(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffLower).c_str(), CalendarTime::convertEpochToTimeStringLocal(cutoffUpper).c_str(), CalendarTime::convertEpochToTimeStringGMT(cutoffUpper).c_str()));
			result += Unicode::narrowToWide("\nPlease confirm that you ***REALLY WANT TO SEND*** this mail and run the command again with the same arguments within 60 seconds to send the mail\n\n");

			s_cutoffLower = cutoffLower;
			s_cutoffUpper = cutoffUpper;
			s_fromName = Unicode::wideToNarrow(argv[13]);
			s_mailSubject = argv[14];
			s_mailBody = mailBody;
			s_confirmationTimeout = timeNow + 60;

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Sender: %s\n", s_fromName.c_str()));
			result += Unicode::narrowToWide("Subject: ");
			result += s_mailSubject;
			result += Unicode::narrowToWide("\n\n");
			result += s_mailBody;
			result += Unicode::narrowToWide("\n");
		}
	}
	else if (isAbbrev(argv[0], "freeCtsTest"))
	{
		time_t const sourceCharacterCreateTime = static_cast<time_t>(atoi(Unicode::wideToNarrow(argv[1]).c_str()));
		uint32 const sourceStationId = static_cast<uint32>(atoi(Unicode::wideToNarrow(argv[2]).c_str()));
		std::string const sourceCluster = Unicode::wideToNarrow(argv[3]);
		uint32 const targetStationId = static_cast<uint32>(atoi(Unicode::wideToNarrow(argv[4]).c_str()));
		std::string const targetCluster = Unicode::wideToNarrow(argv[5]);

		time_t const timeNow = ::time(nullptr);
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("              currentTime: %ld, %s\n", timeNow, CalendarTime::convertEpochToTimeStringLocal(timeNow).c_str()));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("\n      characterCreateTime: %ld\n", sourceCharacterCreateTime));
		if (sourceCharacterCreateTime > 0)
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("      characterCreateTime: %s\n", CalendarTime::convertEpochToTimeStringLocal(sourceCharacterCreateTime).c_str()));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("\n          sourceStationId: %lu\n", sourceStationId));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("            sourceCluster: %s\n", sourceCluster.c_str()));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("\n          targetStationId: %lu\n", targetStationId));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("            targetCluster: %s\n", targetCluster.c_str()));

		FreeCtsDataTable::FreeCtsInfo const * freeCtsInfo = FreeCtsDataTable::wouldCharacterTransferBeFree(sourceCharacterCreateTime, sourceStationId, sourceCluster, targetStationId, targetCluster, false);

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("\n                     free: %s\n", (freeCtsInfo ? "true" : "false")));

		if (freeCtsInfo)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("\n       free CTS info file: %s\n", FreeCtsDataTable::getFreeCtsFileName().c_str()));

			std::string targetClusters;
			for (std::map<std::string, std::string>::const_iterator iter = freeCtsInfo->targetCluster.begin(); iter != freeCtsInfo->targetCluster.end(); ++iter)
			{
				if (!targetClusters.empty())
					targetClusters += ", ";

				targetClusters += iter->first;
			}

			std::string startDate("NA");
			std::string endDate("NA");

			if (freeCtsInfo->startTime > 0)
				startDate = CalendarTime::convertEpochToTimeStringLocal(freeCtsInfo->startTime);

			if (freeCtsInfo->endTime > 0)
				endDate = CalendarTime::convertEpochToTimeStringLocal(freeCtsInfo->endTime);

			std::string characterCreateDateLower("NA");
			std::string characterCreateDateUpper("NA");

			if (freeCtsInfo->sourceCharacterCreateTimeLower > 0)
				characterCreateDateLower = CalendarTime::convertEpochToTimeStringLocal(freeCtsInfo->sourceCharacterCreateTimeLower);

			if (freeCtsInfo->sourceCharacterCreateTimeUpper > 0)
				characterCreateDateUpper = CalendarTime::convertEpochToTimeStringLocal(freeCtsInfo->sourceCharacterCreateTimeUpper);

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("\n                     rule: %s, %s\n", freeCtsInfo->ruleName.c_str(), freeCtsInfo->ruleDescription.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("           source cluster: %s\n", sourceCluster.c_str()));
			result += Unicode::narrowToWide(FormattedString<8192>().sprintf("          target clusters: %s\n", targetClusters.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("               start time: %ld, %s\n", freeCtsInfo->startTime, startDate.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("                 end time: %ld, %s\n", freeCtsInfo->endTime, endDate.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("character create time min: %ld, %s\n", freeCtsInfo->sourceCharacterCreateTimeLower, characterCreateDateLower.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("character create time max: %ld, %s\n", freeCtsInfo->sourceCharacterCreateTimeUpper, characterCreateDateUpper.c_str()));
		}
	}
	else if (isAbbrev(argv[0], "isFreeCtsSourceCluster"))
	{
		std::string const sourceCluster = Unicode::wideToNarrow(argv[1]);

		time_t const timeNow = ::time(nullptr);
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("              currentTime: %ld, %s\n", timeNow, CalendarTime::convertEpochToTimeStringLocal(timeNow).c_str()));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("            sourceCluster: %s\n", sourceCluster.c_str()));

		FreeCtsDataTable::FreeCtsInfo const * freeCtsInfo = FreeCtsDataTable::isFreeCtsSourceCluster(sourceCluster);

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("\n  free CTS source cluster: %s\n", (freeCtsInfo ? "true" : "false")));

		if (freeCtsInfo)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("\n       free CTS info file: %s\n", FreeCtsDataTable::getFreeCtsFileName().c_str()));

			std::string targetClusters;
			for (std::map<std::string, std::string>::const_iterator iter = freeCtsInfo->targetCluster.begin(); iter != freeCtsInfo->targetCluster.end(); ++iter)
			{
				if (!targetClusters.empty())
					targetClusters += ", ";

				targetClusters += iter->first;
			}

			std::string startDate("NA");
			std::string endDate("NA");

			if (freeCtsInfo->startTime > 0)
				startDate = CalendarTime::convertEpochToTimeStringLocal(freeCtsInfo->startTime);

			if (freeCtsInfo->endTime > 0)
				endDate = CalendarTime::convertEpochToTimeStringLocal(freeCtsInfo->endTime);

			std::string characterCreateDateLower("NA");
			std::string characterCreateDateUpper("NA");

			if (freeCtsInfo->sourceCharacterCreateTimeLower > 0)
				characterCreateDateLower = CalendarTime::convertEpochToTimeStringLocal(freeCtsInfo->sourceCharacterCreateTimeLower);

			if (freeCtsInfo->sourceCharacterCreateTimeUpper > 0)
				characterCreateDateUpper = CalendarTime::convertEpochToTimeStringLocal(freeCtsInfo->sourceCharacterCreateTimeUpper);

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("\n                     rule: %s, %s\n", freeCtsInfo->ruleName.c_str(), freeCtsInfo->ruleDescription.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("           source cluster: %s\n", sourceCluster.c_str()));
			result += Unicode::narrowToWide(FormattedString<8192>().sprintf("          target clusters: %s\n", targetClusters.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("               start time: %ld, %s\n", freeCtsInfo->startTime, startDate.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("                 end time: %ld, %s\n", freeCtsInfo->endTime, endDate.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("character create time min: %ld, %s\n", freeCtsInfo->sourceCharacterCreateTimeLower, characterCreateDateLower.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("character create time max: %ld, %s\n", freeCtsInfo->sourceCharacterCreateTimeUpper, characterCreateDateUpper.c_str()));
		}
	}
	else if (isAbbrev(argv[0], "freeCtsInfo"))
	{
		std::multimap<std::string, FreeCtsDataTable::FreeCtsInfo> const & freeCtsInfo = FreeCtsDataTable::getFreeCtsInfo();

		if (!freeCtsInfo.empty())
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("       free CTS info file: %s\n", FreeCtsDataTable::getFreeCtsFileName().c_str()));

			time_t const timeNow = ::time(nullptr);
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("              currentTime: %ld, %s\n", timeNow, CalendarTime::convertEpochToTimeStringLocal(timeNow).c_str()));
		}

		for (std::multimap<std::string, FreeCtsDataTable::FreeCtsInfo>::const_iterator iter = freeCtsInfo.begin(); iter != freeCtsInfo.end(); ++iter)
		{
			std::string targetClusters;
			for (std::map<std::string, std::string>::const_iterator iter2 = iter->second.targetCluster.begin(); iter2 != iter->second.targetCluster.end(); ++iter2)
			{
				if (!targetClusters.empty())
					targetClusters += ", ";

				targetClusters += iter2->first;
			}

			std::string startDate("NA");
			std::string endDate("NA");

			if (iter->second.startTime > 0)
				startDate = CalendarTime::convertEpochToTimeStringLocal(iter->second.startTime);

			if (iter->second.endTime > 0)
				endDate = CalendarTime::convertEpochToTimeStringLocal(iter->second.endTime);

			std::string characterCreateDateLower("NA");
			std::string characterCreateDateUpper("NA");

			if (iter->second.sourceCharacterCreateTimeLower > 0)
				characterCreateDateLower = CalendarTime::convertEpochToTimeStringLocal(iter->second.sourceCharacterCreateTimeLower);

			if (iter->second.sourceCharacterCreateTimeUpper > 0)
				characterCreateDateUpper = CalendarTime::convertEpochToTimeStringLocal(iter->second.sourceCharacterCreateTimeUpper);

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("\n                     rule: %s, %s\n", iter->second.ruleName.c_str(), iter->second.ruleDescription.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("           source cluster: %s\n", iter->first.c_str()));
			result += Unicode::narrowToWide(FormattedString<8192>().sprintf("          target clusters: %s\n", targetClusters.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("               start time: %ld, %s\n", iter->second.startTime, startDate.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("                 end time: %ld, %s\n", iter->second.endTime, endDate.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("character create time min: %ld, %s\n", iter->second.sourceCharacterCreateTimeLower, characterCreateDateLower.c_str()));
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("character create time max: %ld, %s\n", iter->second.sourceCharacterCreateTimeUpper, characterCreateDateUpper.c_str()));
		}

		if (freeCtsInfo.empty())
			result += Unicode::narrowToWide("there is no free CTS info\n");
	}
	else if (isAbbrev(argv[0], "listRetroactiveCtsHistory"))
	{
		NetworkId const oid(Unicode::wideToNarrow(argv[1]));
#ifdef _DEBUG
		std::string const clusterName = Unicode::wideToNarrow(argv[2]);
#else
		std::string const clusterName = GameServer::getInstance().getClusterName();
#endif

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("retroactive CTS history for (%s, %s):\n\n", clusterName.c_str(), oid.getValueString().c_str()));

		std::string const ctsHistory = GameServer::getRetroactiveCtsHistory(clusterName, oid);
		if (ctsHistory.empty())
		{
			result += Unicode::narrowToWide("(no retroactive CTS history)\n");
		}
		else
		{
			result += Unicode::narrowToWide(ctsHistory);
			result += Unicode::narrowToWide("\n");
		}
	}
	else if (isAbbrev(argv[0], "transferCharacter"))
	{
		std::string const destGalaxy = Unicode::wideToNarrow(argv[1]);
		std::string destCharacterName;
		unsigned int sourceStationId = 0;
		unsigned int destStationId = 0;
		bool parseSuccess = true;

		if (argv.size() >= 3)
		{
			destCharacterName = Unicode::wideToNarrow(argv[2]);

			if (argv.size() >= 4)
			{
				if (argv.size() == 4)
				{
					destStationId = static_cast<unsigned int>(::atoi(Unicode::wideToNarrow(argv[3]).c_str()));

					if (destStationId == 0)
					{
						parseSuccess = false;
						result += Unicode::narrowToWide(FormattedString<512>().sprintf("invalid destination station id (%s) specified\n", Unicode::wideToNarrow(argv[3]).c_str()));
					}
				}
				else
				{
					parseSuccess = false;
					result += Unicode::narrowToWide("too many parameters specified in command\n");
				}
			}
		}

		if (parseSuccess)
		{
			if (!user)
			{
				parseSuccess = false;
				result += Unicode::narrowToWide("invalid user\n");
			}

			if (parseSuccess)
			{
				if (!user->getClient())
				{
					parseSuccess = false;
					result += Unicode::narrowToWide("invalid client\n");
				}
			}

			if (parseSuccess)
			{
				PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(user->asCreatureObject());
				if (!playerObject)
				{
					parseSuccess = false;
					result += Unicode::narrowToWide("invalid player object\n");
				}
				else
				{
					if (destCharacterName.empty())
					{
						destCharacterName = Unicode::wideToNarrow(user->getAssignedObjectName());
					}

					sourceStationId = playerObject->getStationId();

					if (destStationId == 0)
					{
						destStationId = playerObject->getStationId();
					}
				}
			}

			if (parseSuccess)
			{
				static std::set<std::string> s_tenOriginalGroundPlanets;
				if (s_tenOriginalGroundPlanets.empty())
				{
					IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("tatooine"));
					IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("naboo"));
					IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("corellia"));
					IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("rori"));
					IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("talus"));
					IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("yavin4"));
					IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("endor"));
					IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("lok"));
					IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("dantooine"));
					IGNORE_RETURN(s_tenOriginalGroundPlanets.insert("dathomir"));
				}

				if (s_tenOriginalGroundPlanets.count(ServerWorld::getSceneId()) <= 0)
				{
					parseSuccess = false;
					result += Unicode::narrowToWide("you must be on one of the 10 original ground planets for character transfer\n");
				}
			}

			if (parseSuccess)
			{
				result += Unicode::narrowToWide(FormattedString<1024>().sprintf("requesting transfer of this character [%u, %s, %s (%s)] to [%u, %s, %s]\n", sourceStationId, GameServer::getInstance().getClusterName().c_str(), Unicode::wideToNarrow(user->getAssignedObjectName()).c_str(), user->getNetworkId().getValueString().c_str(), destStationId, destGalaxy.c_str(), destCharacterName.c_str()));
				result += Unicode::narrowToWide("you will be disconnected once the transfer is validated and the transfer process begins.\n");

				TransferRequestMoveValidation const trmv(TransferRequestMoveValidation::TRS_console_god_command, GameServer::getInstance().getProcessId(), sourceStationId, destStationId, GameServer::getInstance().getClusterName(), destGalaxy, Unicode::wideToNarrow(user->getAssignedObjectName()), user->getNetworkId(), user->getTemplateCrc(), destCharacterName, std::string("en"));
				GameServer::getInstance().sendToCentralServer(trmv);
			}
		}
	}
	else if (isAbbrev(argv[0], "showGcwScore"))
	{
		PlanetObject const * const tatooine = ServerUniverse::getInstance().getTatooinePlanet();
		if (tatooine)
		{
			std::map<std::string, std::pair<int64, int64> > const & gcwImperialScore = tatooine->getGcwImperialScore();
			result += Unicode::narrowToWide("Imperial:\n");
			for (std::map<std::string, std::pair<int64, int64> >::const_iterator iterImp = gcwImperialScore.begin(); iterImp != gcwImperialScore.end(); ++iterImp)
				result += Unicode::narrowToWide(FormattedString<1024>().sprintf("%s (cumulative = %s, current = %s)\n", iterImp->first.c_str(), NetworkId(iterImp->second.first).getValueString().c_str(), NetworkId(iterImp->second.second).getValueString().c_str()));

			std::map<std::string, std::pair<int64, int64> > const & gcwRebelScore = tatooine->getGcwRebelScore();
			result += Unicode::narrowToWide("\nRebel:\n");
			for (std::map<std::string, std::pair<int64, int64> >::const_iterator iterReb = gcwRebelScore.begin(); iterReb != gcwRebelScore.end(); ++iterReb)
				result += Unicode::narrowToWide(FormattedString<1024>().sprintf("%s (cumulative = %s, current = %s)\n", iterReb->first.c_str(), NetworkId(iterReb->second.first).getValueString().c_str(), NetworkId(iterReb->second.second).getValueString().c_str()));
		}
		else
		{
			result += Unicode::narrowToWide("couldn't find the tatooine planet object.\n");
		}

		{
			result += Unicode::narrowToWide("\nGuildObject::m_gcwGroupImperialScoreRawThisGalaxy\n");
			std::map<std::string, int> const & gcwGroupImperialScoreRawThisGalaxy = ServerUniverse::getInstance().getMasterGuildObject()->getGcwGroupImperialScoreRaw();
			for (std::map<std::string, int>::const_iterator iter = gcwGroupImperialScoreRawThisGalaxy.begin(); iter != gcwGroupImperialScoreRawThisGalaxy.end(); ++iter)
			{
				result += Unicode::narrowToWide(FormattedString<1024>().sprintf("(%s) (%d)\n", iter->first.c_str(), iter->second));
			}
		}

		{
			result += Unicode::narrowToWide("\nGuildObject::m_gcwGroupCategoryImperialScoreRawThisGalaxy\n");
			std::map<std::pair<std::string, std::string>, int> const & gcwGroupCategoryImperialScoreRawThisGalaxy = ServerUniverse::getInstance().getMasterGuildObject()->getGcwGroupCategoryImperialScoreRaw();
			for (std::map<std::pair<std::string, std::string>, int>::const_iterator iter = gcwGroupCategoryImperialScoreRawThisGalaxy.begin(); iter != gcwGroupCategoryImperialScoreRawThisGalaxy.end(); ++iter)
			{
				result += Unicode::narrowToWide(FormattedString<1024>().sprintf("(%s) (%s) (%d)\n", iter->first.first.c_str(), iter->first.second.c_str(), iter->second));
			}
		}
	}
	else if (isAbbrev(argv[0], "adjustGcwImperialScore"))
	{
		std::string category = Unicode::wideToNarrow(argv[1]);
		int adjustment = ::atoi(Unicode::wideToNarrow(argv[2]).c_str());

		if (!Pvp::getGcwScoreCategory(category))
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("(%s) is not a valid GCW score category.\n", category.c_str()));
			result += Unicode::narrowToWide("\nThe GCW score categories are as follows:\n");

			std::map<std::string, Pvp::GcwScoreCategory const *> const & allGcwScoreCategory = Pvp::getAllGcwScoreCategory();
			for (std::map<std::string, Pvp::GcwScoreCategory const *>::const_iterator iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
			{
				result += Unicode::narrowToWide(iter->first);
				result += Unicode::narrowToWide("\n");
			}
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("Requesting adjustment of imperial score for GCW score category (%s) by (%d).  It can take up to 1 minute for the adjustment request to be completed.\n", category.c_str(), adjustment));
			ServerUniverse::getInstance().adjustGcwImperialScore("remote server adjustGcwImperialScore", nullptr, category, adjustment);
		}
	}
	else if (isAbbrev(argv[0], "adjustGcwRebelScore"))
	{
		std::string category = Unicode::wideToNarrow(argv[1]);
		int adjustment = ::atoi(Unicode::wideToNarrow(argv[2]).c_str());

		if (!Pvp::getGcwScoreCategory(category))
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("(%s) is not a valid GCW score category.\n", category.c_str()));
			result += Unicode::narrowToWide("\nThe GCW score categories are as follows:\n");

			std::map<std::string, Pvp::GcwScoreCategory const *> const & allGcwScoreCategory = Pvp::getAllGcwScoreCategory();
			for (std::map<std::string, Pvp::GcwScoreCategory const *>::const_iterator iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
			{
				result += Unicode::narrowToWide(iter->first);
				result += Unicode::narrowToWide("\n");
			}
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("Requesting adjustment of rebel score for GCW score category (%s) by (%d).  It can take up to 1 minute for the adjustment request to be completed.\n", category.c_str(), adjustment));
			ServerUniverse::getInstance().adjustGcwRebelScore("remote server adjustGcwRebelScore", nullptr, category, adjustment);
		}
	}
	else if (isAbbrev(argv[0], "decayGcwScore"))
	{
		std::string category = Unicode::wideToNarrow(argv[1]);

		if (!Pvp::getGcwScoreCategory(category))
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("(%s) is not a valid GCW score category.\n", category.c_str()));
			result += Unicode::narrowToWide("\nThe GCW score categories are as follows:\n");

			std::map<std::string, Pvp::GcwScoreCategory const *> const & allGcwScoreCategory = Pvp::getAllGcwScoreCategory();
			for (std::map<std::string, Pvp::GcwScoreCategory const *>::const_iterator iter = allGcwScoreCategory.begin(); iter != allGcwScoreCategory.end(); ++iter)
			{
				result += Unicode::narrowToWide(iter->first);
				result += Unicode::narrowToWide("\n");
			}
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("Requesting immediate GCW score decay for category (%s).\n", category.c_str()));
			PlanetObject const * const tatooine = ServerUniverse::getInstance().getTatooinePlanet();
			if (tatooine)
			{
				MessageToQueue::sendMessageToC(tatooine->getNetworkId(), "C++DoGcwDecayImmediate", category, 0, false);
			}
		}
	}
	else if (isAbbrev(argv[0], "showGcwFactionalPresence"))
	{
		PlanetObject const * const tatooine = ServerUniverse::getInstance().getTatooinePlanet();
		if (tatooine)
		{
			std::map<std::string, std::pair<int, int> > const & connectedCharacterLfgDataFactionalPresence = tatooine->getConnectedCharacterLfgDataFactionalPresence();
			if (connectedCharacterLfgDataFactionalPresence.empty())
			{
				result += Unicode::narrowToWide("no GCW contested region has factional presence.\n");
			}
			else
			{
				for (std::map<std::string, std::pair<int, int> >::const_iterator iter = connectedCharacterLfgDataFactionalPresence.begin(); iter != connectedCharacterLfgDataFactionalPresence.end(); ++iter)
					result += Unicode::narrowToWide(FormattedString<1024>().sprintf("(%s) imp=%d, reb=%d\n", iter->first.c_str(), iter->second.first, iter->second.second));
			}
		}
		else
		{
			result += Unicode::narrowToWide("couldn't find the tatooine planet object.\n");
		}
	}
	else if (isAbbrev(argv[0], "hasGcwFactionalPresence"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject const * const o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject const * const c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		if (PlayerCreatureController::getPlayerObject(c) == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		LfgCharacterData lfgCharacterData;
		c->getLfgCharacterData(lfgCharacterData);

		if (lfgCharacterData.locationFactionalPresenceGcwRegion.empty())
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("object %s does not have factional presence in its current state\n", o->getNetworkId().getValueString().c_str()));
		}
		else
		{
			if (::strncmp("space_", lfgCharacterData.locationPlanet.c_str(), 6))
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("object %s has factional presence for GCW contested region %s (%d, %d) [grid (%d, %d) -> (%d, %d)] in its current state\n", o->getNetworkId().getValueString().c_str(), lfgCharacterData.locationFactionalPresenceGcwRegion.c_str(), lfgCharacterData.locationFactionalPresenceGridX, lfgCharacterData.locationFactionalPresenceGridZ, (lfgCharacterData.locationFactionalPresenceGridX - (PvpData::getGcwFactionalPresenceGridSize() / 2)), (lfgCharacterData.locationFactionalPresenceGridZ - (PvpData::getGcwFactionalPresenceGridSize() / 2)), (lfgCharacterData.locationFactionalPresenceGridX + (PvpData::getGcwFactionalPresenceGridSize() / 2)), (lfgCharacterData.locationFactionalPresenceGridZ + (PvpData::getGcwFactionalPresenceGridSize() / 2))));
			else
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("object %s has factional presence for GCW contested region %s in its current state\n", o->getNetworkId().getValueString().c_str(), lfgCharacterData.locationFactionalPresenceGcwRegion.c_str()));
		}
	}
	else if (isAbbrev(argv[0], "hasGcwRegionDefenderBonus"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject const * const o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject const * const c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject const * const p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		float bonus = 0.0f;
		if (Pvp::getGcwDefenderRegionBonus(*c, *p, bonus))
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("object %s has GCW Region Defender bonus of %.10g%% in its current state\n", o->getNetworkId().getValueString().c_str(), bonus));
		else
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("object %s does not have GCW Region Defender bonus in its current state\n", o->getNetworkId().getValueString().c_str()));
	}
	else if (isAbbrev(argv[0], "getCommoditiesItemAttribute"))
	{
		if (!ConfigServerGame::getCommoditiesMarketEnabled() || !CommoditiesMarket::isCommoditiesServerAvailable())
		{
			result += Unicode::narrowToWide("The commodities server is currently unavailable.\n");
		}
		else
		{
			GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const msg("GetItemAttribute", std::make_pair(userId, NetworkId(Unicode::wideToNarrow(argv[1]))));
			CommoditiesMarket::sendToCommoditiesServer(msg);

			result += Unicode::narrowToWide("getCommoditiesItemAttribute request sent to the commodities server.  Please wait for reply.\n");
		}
	}
	else if (isAbbrev(argv[0], "getCommoditiesItemAttributeData"))
	{
		if (!ConfigServerGame::getCommoditiesMarketEnabled() || !CommoditiesMarket::isCommoditiesServerAvailable())
		{
			result += Unicode::narrowToWide("The commodities server is currently unavailable.\n");
		}
		else
		{
			int got = 0;
			bool exactGotMatch = false;
			bool ignoreSearchableAttribute = false;
			int throttle = 100;

			if (argv.size() >= 3)
			{
				std::string argvNarrow;
				for (size_t i = 2, size = argv.size(); i < size; ++i)
				{
					argvNarrow = Unicode::wideToNarrow(argv[i]);

					if (argvNarrow == "ignore_searchable_attribute")
					{
						ignoreSearchableAttribute = true;
						continue;
					}

					std::string::size_type pos = argvNarrow.find("got_exact=");
					if (pos == 0)
					{
						argvNarrow = argvNarrow.substr(10);
						got = GameObjectTypes::getGameObjectType(argvNarrow);
						if (got <= 0)
						{
							result += Unicode::narrowToWide(FormattedString<1024>().sprintf("argument (%s) contains an invalid game object type\n", Unicode::wideToNarrow(argv[i]).c_str()));
							return true;
						}

						exactGotMatch = true;
						continue;
					}

					pos = argvNarrow.find("got=");
					if (pos == 0)
					{
						argvNarrow = argvNarrow.substr(4);
						got = GameObjectTypes::getGameObjectType(argvNarrow);
						if (got <= 0)
						{
							result += Unicode::narrowToWide(FormattedString<1024>().sprintf("argument (%s) contains an invalid game object type\n", Unicode::wideToNarrow(argv[i]).c_str()));
							return true;
						}

						exactGotMatch = false;
						continue;
					}

					throttle = ::atoi(argvNarrow.c_str());
					if (throttle <= 0)
						throttle = 100;
				}
			}

			GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::string>, std::pair<std::pair<int, bool>, std::pair<bool, int> > > > const msg("GetItemAttributeData", std::make_pair(std::make_pair(userId, Unicode::wideToNarrow(argv[1])), std::make_pair(std::make_pair(got, exactGotMatch), std::make_pair(ignoreSearchableAttribute, throttle))));
			CommoditiesMarket::sendToCommoditiesServer(msg);

			result += Unicode::narrowToWide("getCommoditiesItemAttributeData request sent to the commodities server.  Please wait for reply.\n");
		}
	}
	else if (isAbbrev(argv[0], "getCommoditiesItemAttributeDataValues"))
	{
		if (!ConfigServerGame::getCommoditiesMarketEnabled() || !CommoditiesMarket::isCommoditiesServerAvailable())
		{
			result += Unicode::narrowToWide("The commodities server is currently unavailable.\n");
		}
		else
		{
			int const got = GameObjectTypes::getGameObjectType(Unicode::wideToNarrow(argv[1]));
			if (got <= 0)
			{
				result += Unicode::narrowToWide(FormattedString<1024>().sprintf("invalid game object type (%s) specified\n", Unicode::wideToNarrow(argv[1]).c_str()));
			}
			else
			{
				bool exactGotMatch = false;
				int throttle = 100;
				if (argv.size() >= 4)
				{
					std::string argvNarrow;
					for (size_t i = 3, size = argv.size(); i < size; ++i)
					{
						argvNarrow = Unicode::wideToNarrow(argv[i]);

						if (argvNarrow == "got_exact")
						{
							exactGotMatch = true;
							continue;
						}

						throttle = ::atoi(argvNarrow.c_str());
						if (throttle <= 0)
							throttle = 100;
					}
				}

				GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::pair<int, bool> >, std::pair<std::string, int> > > const msg("GetItemAttributeDataValues", std::make_pair(std::make_pair(userId, std::make_pair(got, exactGotMatch)), std::make_pair(Unicode::wideToNarrow(argv[2]), throttle)));
				CommoditiesMarket::sendToCommoditiesServer(msg);

				result += Unicode::narrowToWide("getCommoditiesItemAttributeDataValues request sent to the commodities server.  Please wait for reply.\n");
			}
		}
	}
	else if (isAbbrev(argv[0], "getCommoditiesAuctionLocationPriorityQueue"))
	{
		if (!ConfigServerGame::getCommoditiesMarketEnabled() || !CommoditiesMarket::isCommoditiesServerAvailable())
		{
			result += Unicode::narrowToWide("The commodities server is currently unavailable.\n");
		}
		else
		{
			GenericValueTypeMessage<std::pair<NetworkId, int> > const msg("GetALPQ", std::make_pair(userId, atoi(Unicode::wideToNarrow(argv[1]).c_str())));
			CommoditiesMarket::sendToCommoditiesServer(msg);

			result += Unicode::narrowToWide("getCommoditiesAuctionLocationPriorityQueue request sent to the commodities server.  Please wait for reply.\n");
		}
	}
	else if (isAbbrev(argv[0], "getCommoditiesItemTypeMap"))
	{
		if (!ConfigServerGame::getCommoditiesMarketEnabled() || !CommoditiesMarket::isCommoditiesServerAvailable())
		{
			result += Unicode::narrowToWide("The commodities server is currently unavailable.\n");
		}
		else
		{
			int const got = GameObjectTypes::getGameObjectType(Unicode::wideToNarrow(argv[1]));
			if (got <= 0)
			{
				result += Unicode::narrowToWide(FormattedString<1024>().sprintf("invalid game object type (%s) specified\n", Unicode::wideToNarrow(argv[1]).c_str()));
			}
			else
			{
				result += Unicode::narrowToWide(FormattedString<1024>().sprintf("item types for game object type (%d, %s)\n", got, GameObjectTypes::getCanonicalName(got).c_str()));

				std::map<int, std::map<int, std::pair<int, StringId> > > const & itemTypeMap = CommoditiesMarket::getItemTypeMap();
				std::map<int, std::map<int, std::pair<int, StringId> > >::const_iterator const iterFind = itemTypeMap.find(got);
				if (iterFind != itemTypeMap.end())
				{
					std::set<std::pair<std::string, std::pair<std::pair<int, int>, std::string> > > output;

					for (std::map<int, std::pair<int, StringId> >::const_iterator iter = iterFind->second.begin(); iter != iterFind->second.end(); ++iter)
					{
						std::string const name(Unicode::wideToNarrow(iter->second.second.localize()));
						output.insert(std::make_pair(Unicode::toLower(name), std::make_pair(std::make_pair(iter->first, iter->second.first), name)));
					}

					for (std::set<std::pair<std::string, std::pair<std::pair<int, int>, std::string> > >::const_iterator iterOutput = output.begin(); iterOutput != output.end(); ++iterOutput)
					{
						result += Unicode::narrowToWide(FormattedString<1024>().sprintf("(%d, %s) [%d, %s]\n", iterOutput->second.first.first, iterOutput->second.second.c_str(), iterOutput->second.first.second, GameObjectTypes::getCanonicalName(iterOutput->second.first.second).c_str()));
					}

					result += Unicode::narrowToWide(FormattedString<1024>().sprintf("count=%d\n", output.size()));
				}
				else
				{
					result += Unicode::narrowToWide("count=0\n");
				}
			}
		}
	}
	else if (isAbbrev(argv[0], "setCompletedTutorial"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		if (NameManager::getInstance().isPlayer(oid))
		{
			unsigned int const stationId = static_cast<unsigned int>(NameManager::getInstance().getPlayerStationId(oid));
			if (stationId != 0)
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("Marking station id (%u) for character (%s, %s) as having completed the tutorial\n", stationId, oid.getValueString().c_str(), NameManager::getInstance().getPlayerFullName(oid).c_str()));

				GenericValueTypeMessage< std::pair<unsigned int, bool> > const updateTutorial("LoginToggleCompletedTutorial", std::pair<unsigned int, bool>(stationId, true));
				GameServer::getInstance().sendToCentralServer(updateTutorial);
			}
			else
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("Invalid station id (%u) found for character (%s, %s)\n", stationId, oid.getValueString().c_str(), NameManager::getInstance().getPlayerFullName(oid).c_str()));
			}
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("There is no character on this galaxy with id (%s)\n", oid.getValueString().c_str()));
		}
	}
	else if (isAbbrev(argv[0], "getUniverseProcess"))
    {
	    result += Unicode::narrowToWide(FormattedString<512>().sprintf("Universe Process ID is: %d\n", ServerUniverse::getInstance().getUniverseProcess()));
        result += getErrorMessage(argv[0], ERR_SUCCESS);
    }
#ifdef _DEBUG
	else if (isAbbrev(argv[0], "setExtraDelayPerFrameMs"))
	{
		int const ms = ::atoi(Unicode::wideToNarrow(argv[1]).c_str());
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Setting extra sleep time per frame to %dms.\n", ms));
		GameServer::setExtraDelayPerFrameMs(ms);
	}
	else if (isAbbrev(argv[0], "serverinfo"))
	{
		char numBuf[64] = { "\0" };
		static const Unicode::String unl(Unicode::narrowToWide(std::string("\n")));

		result += Unicode::narrowToWide("PID: ");
		snprintf(numBuf, sizeof(numBuf), "%lu", GameServer::getInstance().getProcessId());
		result += Unicode::narrowToWide(std::string(numBuf)) + unl;

		result += Unicode::narrowToWide("Scene: ") + Unicode::narrowToWide(ConfigServerGame::getSceneID()) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumObjects());
		result += Unicode::narrowToWide(std::string("Number of Objects: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumAI());
		result += Unicode::narrowToWide(std::string("Number of AI: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumBuildings());
		result += Unicode::narrowToWide(std::string("Number of Buildings: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumInstallations());
		result += Unicode::narrowToWide(std::string("Number of Installations: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumCreatures());
		result += Unicode::narrowToWide(std::string("Number of Creatures: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumPlayers());
		result += Unicode::narrowToWide(std::string("Number of Players: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumTangibles());
		result += Unicode::narrowToWide(std::string("Number of Tangibles: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumUniverseObjects());
		result += Unicode::narrowToWide(std::string("Number of UniverseObjects: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumDynamicAI());
		result += Unicode::narrowToWide(std::string("Number of Dynamic AI: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumStaticAI());
		result += Unicode::narrowToWide(std::string("Number of Static AI: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumCombatAI());
		result += Unicode::narrowToWide(std::string("Number of Combat AI: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumHibernatingAI());
		result += Unicode::narrowToWide(std::string("Number of Hibernating AI: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumDelayedHibernatingAI());
		result += Unicode::narrowToWide(std::string("Number of Delayed Hibernating AI: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumIntangibles());
		result += Unicode::narrowToWide(std::string("Number of Intangibles: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumMissionDatas());
		result += Unicode::narrowToWide(std::string("Number of MissionDatas: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumMissionObjects());
		result += Unicode::narrowToWide(std::string("Number of MissionObjects: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumRunTimeRules());
		result += Unicode::narrowToWide(std::string("Number of Runtime Rules: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumGroupObjects());
		result += Unicode::narrowToWide(std::string("Number of GroupObjects: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumMissionListEntries());
		result += Unicode::narrowToWide(std::string("Number of MissionListEntries: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumWaypoints());
		result += Unicode::narrowToWide(std::string("Number of Waypoints: ") + std::string(numBuf)) + unl;

		snprintf(numBuf, sizeof(numBuf), "%d", ObjectTracker::getNumPlayerQuestObjects());
		result += Unicode::narrowToWide(std::string("Number of PlayerQuestObjects: ") + std::string(numBuf)) + unl;
	}
#endif
	else
	{
		result += getErrorMessage(argv[0], ERR_NO_HANDLER);
	}

	return true;
		}

// ======================================================================
