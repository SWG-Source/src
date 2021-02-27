// ======================================================================
//
// ConsoleCommandParserVeteran.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserVeteran.h"

#include "UnicodeUtils.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/VeteranRewardManager.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/NetworkIdManager.h"

//-----------------------------------------------------------------------

static const CommandParser::CmdInfo cmds[] =
{
	{"getRewardInfo",       1,"<player oid>","Get the reward status of a player."},
	{"claimReward",         3,"<player oid> <event tag> <reward item tag>","For testing, claim a reward using the specified event."},
	{"launchBrowser",       1,"<url>","Test launching a web browser."},
	{"setOverrideAccountAge",1,"<age>","For testing, set all accounts as being the speficied age (in days)"},
	{"",                    0, "", ""} // this must be last
};

ConsoleCommandParserVeteran::ConsoleCommandParserVeteran() :
		CommandParser ("veteran", 0, "...", "commands for the veteran reward system", 0)
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------------

bool ConsoleCommandParserVeteran::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & /* originalMessage */, String_t & result, const CommandParser *)
{

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

	if (isCommand( argv [0], "getRewardInfo"))
	{
		NetworkId targetPlayer(Unicode::wideToNarrow (argv[1]));
		ServerObject const * const so = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(targetPlayer));
		Client const * const client = so ? so->getClient() : nullptr;
		if (client)
		{
			std::string output;

			// Claimed Rewards:
			std::vector<std::pair<NetworkId, std::string> > const & accountConsumedRewardEvents = client->getConsumedRewardEvents();
			std::vector<std::pair<NetworkId, std::string> > const & accountClaimedRewardItems = client->getClaimedRewardItems();
			std::vector<std::string> localConsumedRewardEvents;
			VeteranRewardManager::getConsumedEvents(*so, localConsumedRewardEvents);
			std::vector<std::string> localClaimedRewardItems;
			VeteranRewardManager::getClaimedItems(*so, localClaimedRewardItems);

			std::vector<std::pair<NetworkId, std::string> >::const_iterator i;
			output += "Consumed reward events (login server db):  ";
			for (i = accountConsumedRewardEvents.begin(); i != accountConsumedRewardEvents.end(); ++i)
			{
				if (i != accountConsumedRewardEvents.begin())
					output += ", ";
				output += '(' + i->first.getValueString() + ") " + i->second;
			}
			output += "\nClaimed reward items (login server db):  ";
			for (i = accountClaimedRewardItems.begin(); i != accountClaimedRewardItems.end(); ++i)
			{
				if (i != accountClaimedRewardItems.begin())
					output += ", ";
				output += '(' + i->first.getValueString() + ") " + i->second;
			}

			std::vector<std::string>::const_iterator j;
			output += "\nConsumed reward events (cluster db):  ";
			for (j = localConsumedRewardEvents.begin(); j != localConsumedRewardEvents.end(); ++j)
			{
				if (j != localConsumedRewardEvents.begin())
					output += ", ";
				output += *j;
			}
			output += "\nClaimed reward items (cluster db):  ";
			for (j = localClaimedRewardItems.begin(); j != localClaimedRewardItems.end(); ++j)
			{
				if (j != localClaimedRewardItems.begin())
					output += ", ";
				output += *j;
			}
			output += "\n";

			// Criteria used for granting rewards:
			unsigned int const entitledTime = client->getEntitlementEntitledTime();
			uint32 const gameFeatures = client->getGameFeatures();
			bool const adminLogin = client->isUsingAdminLogin();
			char buffer[256];
			snprintf(buffer,sizeof(buffer),"Entitled time:  %u\nGame features:  %lu\nUsing admin login:  %s\n",
					 entitledTime, gameFeatures, adminLogin ? "true" : "false");
			buffer[sizeof(buffer)-1]='\0';
			output += buffer;

			result += Unicode::narrowToWide(output);
		}
		else
			result += getErrorMessage(argv[0],ERR_INVALID_OBJECT);
	}
	if (isCommand( argv [0], "claimReward"))
	{
		NetworkId targetPlayer(Unicode::wideToNarrow(argv[1]));
		ServerObject const * const so = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(targetPlayer));
		CreatureObject const * const playerCreature = so ? so->asCreatureObject() : nullptr;
		if (!playerCreature)
			result += getErrorMessage(argv[0],ERR_INVALID_OBJECT);
		else
		{
			std::string event(Unicode::wideToNarrow(argv[2]));
			std::string item(Unicode::wideToNarrow(argv[3]));

			std::string resultMessage;
			if (VeteranRewardManager::claimRewards(*playerCreature, event, item, &resultMessage))
				result += Unicode::narrowToWide("returned true");
			else
				result += Unicode::narrowToWide("returned false, message:  " + resultMessage);
		}
	}
	if (isCommand( argv [0], "launchBrowser"))
	{
		std::string url(Unicode::wideToNarrow(argv[1]));
		ServerObject const * const so = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(userId));
		Client const * const client = so ? so->getClient() : nullptr;
		if (client)
		{
			client->launchWebBrowser(url);
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
			result+=Unicode::narrowToWide("Could not get Client object");		
	}
	if (isCommand( argv [0], "setOverrideAccountAge"))
	{
		if (ConfigServerGame::getVeteranDebugEnableOverrideAccountAge())
		{
			int age(atoi(Unicode::wideToNarrow(argv[1]).c_str()));
			VeteranRewardManager::setOverrideAccountAge(age);
			ServerMessageForwarding::beginBroadcast();
			ServerMessageForwarding::send(GenericValueTypeMessage<int>("SetOverrideAccountAgeMessage",age));
			ServerMessageForwarding::end();
			
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
			result += Unicode::narrowToWide("\"veteranDebugEnableOverrideAccountAge\" must be set to \"true\" in the server config file to use this feature.");
	}
	return true;
}

//-----------------------------------------------------------------------

