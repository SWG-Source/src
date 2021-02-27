// ======================================================================
//
// ConsoleCommandParserMessageTo.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserMessageTo.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "UnicodeUtils.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/ServerObject.h"
#include "serverUtility/ServerClock.h"
#include "sharedObject/NetworkIdManager.h"

//-----------------------------------------------------------------------

static const CommandParser::CmdInfo cmds[] =
{
	{"listMessages",  1,"<object id>","Print a list of all the messages queued for the specified object."},
	{"cancelMessage",  2,"<object id> <message name>","Cancel all instances of a specific message pending for a specific object.  USE WITH CAUTION."},
	{"cancelMessageById", 2,"<object id> <message id>","Cancel a specific message pending for a specific object.  USE WITH CAUTION."},
	{"viewLastKnownLocations",  0,"","Print a list of the last known locations of objects."},
	{"viewScheduler",  0,"","Print the state of the object scheduler in MessageToQueue."},
	{"",                    0, "", ""} // this must be last
};

ConsoleCommandParserMessageTo::ConsoleCommandParserMessageTo() :
		CommandParser ("messageto", 0, "...", "commands for the MessageTo system", 0)
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------------

bool ConsoleCommandParserMessageTo::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & /* originalMessage */, String_t & result, const CommandParser *)
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


	if (isCommand( argv [0], "viewLastKnownLocations"))
	{
		result += Unicode::narrowToWide(MessageToQueue::getInstance().debugGetLastKnownLocations());
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	if (isCommand( argv [0], "viewScheduler"))
	{
		char temp[256];
		snprintf(temp,sizeof(temp),"Current time is %li\n",ServerClock::getInstance().getGameTimeSeconds());
		result += Unicode::narrowToWide(temp);
		result += Unicode::narrowToWide(MessageToQueue::getInstance().debugGetSchedulerData());
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	if (isCommand( argv [0], "listMessages"))
	{
		NetworkId networkId(Unicode::wideToNarrow (argv[1]));
		ServerObject const * const object = safe_cast<ServerObject const *>(NetworkIdManager::getObjectById(networkId));
		if (object)
		{
			char temp[256];
			snprintf(temp,sizeof(temp),"Current time is %li\n",ServerClock::getInstance().getGameTimeSeconds());
			result += Unicode::narrowToWide(temp);
			result += Unicode::narrowToWide(object->debugGetMessageToList());
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
			result += getErrorMessage(argv[0],ERR_INVALID_OBJECT);
	}
	if (isCommand( argv [0], "cancelMessage"))
	{
		NetworkId networkId(Unicode::wideToNarrow (argv[1]));
		std::string message(Unicode::wideToNarrow (argv[2]));
		ServerObject * const object = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(networkId));
		if (object)
		{
			char buffer[255];
			snprintf(buffer,sizeof(buffer)-1,"%i messages cancelled\n", object->cancelMessageTo(message));
			buffer[sizeof(buffer)-1]='\0';
			result += Unicode::narrowToWide(buffer);
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
			result += getErrorMessage(argv[0],ERR_INVALID_OBJECT);
	}
	if (isCommand( argv [0], "cancelMessageById"))
	{
		NetworkId networkId(Unicode::wideToNarrow (argv[1]));
		NetworkId messageId(Unicode::wideToNarrow (argv[2]));
		ServerObject * const object = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(networkId));
		if (object)
		{
			char buffer[255];
			snprintf(buffer,sizeof(buffer)-1,"%i messages cancelled\n", object->cancelMessageToByMessageId(messageId));
			buffer[sizeof(buffer)-1]='\0';
			result += Unicode::narrowToWide(buffer);
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
			result += getErrorMessage(argv[0],ERR_INVALID_OBJECT);
	}

	return true;
}

//-----------------------------------------------------------------------

