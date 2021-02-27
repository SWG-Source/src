// ======================================================================
//
// ConsoleCommandParserVoice.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "ConsoleCommandParserVoice.h"
#include "UnicodeUtils.h"
#include "serverGame/Chat.h"
#include "sharedLog/Log.h"
//#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedNetworkMessages/VoiceChatMiscMessages.h"


// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"channelannounce", 3, "<oid> <channelName> <channelURI>", "Tell a client about a channel without checking if it exitsts."},
	{"channeljoin", 4, "<oid> <channelName> <channelURI> <channelPassword>", "Force a client to join a channel"},
	{"channelleave", 2, "<oid> <channelName>", "Tell a client to forget about and leave a channel"},
	{"channelcreate", 1, "<channelName>", "Create a voice chat room. Give full path ie. SWG.cluster.chanName"},
	{"channeldestroy", 1, "<channelName>", "Destroy a voice chat room (name must be fully qualified ie SWG.clustername.channelname)"},
	{"addtochannel", 2, "<oid> <channelName>", "Give a client access to a channel. Channel must exist."},
	{"removefromchannel", 2, "<oid> <channelName>", "Remove a client's access to a channel"},
	{"channelcommand", 3, "<destUser> <channelName> <command> <banTimeout>", "Execute a moderator command in the given channel (fully qualify channel name). Commands are [mute unmute kick hangup invite ban unban chan_lock chan_unlock mute_all unmute_all add_moderator delete_moderator add_acl delete_acl]"},
	{"globalchannelmessage", 2, "<channelName> <message> [remove]", "Broadcast a global channel message to everyone on this cluster. Removes if remove != 0."},
	{"allclusterglobalchannelmessage", 2, "<channelName> <message> [remove]", "Broadcast a global channel message to everyone on ALL CLUSTERS. Removes if remove != 0."},
	{"", 0, "", ""} // this must be last
};


//-----------------------------------------------------------------

ConsoleCommandParserVoice::ConsoleCommandParserVoice (void) :
CommandParser ("voice", 0, "...", "Voice chat related commands.", 0)
{
    createDelegateCommands (cmds);
}

//-----------------------------------------------------------------


bool ConsoleCommandParserVoice::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
    NOT_NULL (node);
    UNREF(originalCommand);
	UNREF(userId);

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

	//CreatureObject * creatureObject = safe_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
	//PlayerObject * playerObject = const_cast<PlayerObject *>(PlayerCreatureController::getPlayerObject(creatureObject));

	//-----------------------------------------------------------------

	static Unicode::String requestSubmittedMessage = Unicode::narrowToWide("request successfully submitted");

	if (isAbbrev(argv[0], "channelannounce"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		std::string name = Unicode::wideToNarrow(argv[2]);
		std::string uri = Unicode::wideToNarrow(argv[3]);
		std::string password = "";

		if(Chat::debugTellClientAboutRoom(oid, name, uri, password, false, false))
		{
			result += requestSubmittedMessage;
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
		return true;
	}

	if (isAbbrev(argv[0], "channeljoin"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		std::string name = Unicode::wideToNarrow(argv[2]);
		std::string uri = Unicode::wideToNarrow(argv[3]);
		std::string password = Unicode::wideToNarrow(argv[4]);
		if(Chat::debugTellClientAboutRoom(oid, name, uri, password, true, false))
		{
			result += requestSubmittedMessage;
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
		return true;
	}

	if (isAbbrev(argv[0], "channelleave"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		std::string name = Unicode::wideToNarrow(argv[2]);
		std::string uri = "";
		std::string password = "";
		if(Chat::debugTellClientAboutRoom(oid, name, uri, password, false, true))
		{
			result += requestSubmittedMessage;
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
		return true;
	}

	if (isAbbrev(argv[0], "addtochannel"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		std::string channelName = Unicode::wideToNarrow(argv[2]);
		Chat::requestAddClientToChannel(oid, "", channelName, false);
		result += requestSubmittedMessage;

		return true;
	}

	if (isAbbrev(argv[0], "removefromchannel"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		std::string channelName = Unicode::wideToNarrow(argv[2]);
		Chat::requestRemoveClientFromChannel(oid, "", channelName);
		result += requestSubmittedMessage;

		return true;
	}

	if (isAbbrev(argv[0], "channelcreate"))
	{
		std::string name = Unicode::wideToNarrow(argv[1]);

		Chat::requestGetChannel(name, name,"",false);
		result += requestSubmittedMessage;

		return true;
	}

	if (isAbbrev(argv[0], "channeldestroy"))
	{
		std::string name = Unicode::wideToNarrow(argv[1]);

		Chat::requestDestroyChannel(name);
		result += requestSubmittedMessage;

		return true;
	}

	if(isAbbrev(argv[0], "channelcommand"))
	{
		static bool mapInited = false;
		static std::map<std::string, uint32> commandTypeLookupMap;
		if(!mapInited)
		{
			commandTypeLookupMap["mute"] = COMMAND_MUTE;
			commandTypeLookupMap["unmute"] = COMMAND_UNMUTE;
			commandTypeLookupMap["kick"] = COMMAND_KICK;
			commandTypeLookupMap["hangup"] = COMMAND_HANGUP;
			commandTypeLookupMap["invite"] = COMMAND_INVITE;
			commandTypeLookupMap["ban"] = COMMAND_BAN;
			commandTypeLookupMap["unban"] = COMMAND_UNBAN;
			commandTypeLookupMap["chan_lock"] = COMMAND_CHAN_LOCK;
			commandTypeLookupMap["chan_unlock"] = COMMAND_CHAN_UNLOCK;
			commandTypeLookupMap["mute_all"] = COMMAND_MUTE_ALL;
			commandTypeLookupMap["unmute_all"] = COMMAND_UNMUTE_ALL;
			commandTypeLookupMap["add_moderator"] = COMMAND_ADD_MODERATOR;
			commandTypeLookupMap["delete_moderaror"] = COMMAND_DELETE_MODERATOR;
			commandTypeLookupMap["add_acl"] = COMMAND_ADD_ACL;
			commandTypeLookupMap["delete_acl"] = COMMAND_DELETE_ACL;
			mapInited = true;
		}

		std::string destUser = Unicode::wideToNarrow(argv[1]);
		std::string destChan = Unicode::wideToNarrow(argv[2]);
		std::string strCommandType = Unicode::wideToNarrow(argv[3]);
		uint32 banTimeout = argv.size() > 4 ? static_cast<uint32>(atoi(Unicode::wideToNarrow(argv[4]).c_str())) : 0;

		std::map<std::string,uint32>::const_iterator i = commandTypeLookupMap.find(strCommandType);
		if(i != commandTypeLookupMap.end())
		{
			//@TODO: send a valid source user that will always have moderator powers
			uint32 commandType = i->second;
			Chat::requestChannelCommand("",destUser,destChan,commandType,banTimeout);
			result += requestSubmittedMessage;
		}
		else
		{
			result += Unicode::narrowToWide("Unknown channel command type");
		}
		return true;
	}

	if (isAbbrev(argv[0], "globalchannelmessage"))
	{
		std::string name = Unicode::getTrim(Unicode::wideToNarrow(argv[1]));
		std::string message = Unicode::wideToNarrow(argv[2]);
		bool isRemove = argv.size() > 3 ? (atoi(Unicode::wideToNarrow(argv[3]).c_str()) != 0) : 0;

		LOG("CustomerService", ("BroadcastVoiceChannel: GameServer::globalchannelmessage issued by id(%s) chan(%s) text(%s) remove(%d)", 
			userId.getValueString().c_str(), name.c_str(), message.c_str(), (isRemove ? 1 : 0)));

		Chat::requestBroadcastGlobalChannelMessage(name, message, isRemove, false);
		result += requestSubmittedMessage;

		return true;
	}

	if(isAbbrev(argv[0], "allclusterglobalchannelmessage"))
	{
		std::string name = Unicode::getTrim(Unicode::wideToNarrow(argv[1]));
		std::string message = Unicode::wideToNarrow(argv[2]);
		bool isRemove = argv.size() > 3 ? (atoi(Unicode::wideToNarrow(argv[3]).c_str()) != 0) : 0;

		LOG("CustomerService", ("BroadcastVoiceChannel: GameServer::allclusterglobalchannelmessage issued by id(%s) chan(%s) text(%s) remove(%d)", 
			userId.getValueString().c_str(), name.c_str(), message.c_str(), (isRemove ? 1 : 0)));

		Chat::requestBroadcastGlobalChannelMessage(name, message, isRemove, true);
		result += requestSubmittedMessage;

		return true;
	}

	result += getErrorMessage(argv[0], ERR_NO_HANDLER);

    return true;
}


// ======================================================================





