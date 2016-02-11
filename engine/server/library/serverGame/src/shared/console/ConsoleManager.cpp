// ConsoleMgr.cpp
// copyright 2000 Verant Interactive
// Author: Chris Mayer

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleManager.h"

#include <cstdio>
#include <cstdlib>

#include "SwgGameServer/ConsoleCommandParserCombatEngine.h"
#include "SwgGameServer/ConsoleCommandParserCombatEngine.h"
#include "serverGame/Client.h"
#include "serverGame/ConsoleCommandParserAi.h"
#include "serverGame/ConsoleCommandParserCity.h"
#include "serverGame/ConsoleCommandParserCollection.h"
#include "serverGame/ConsoleCommandParserCraft.h"
#include "serverGame/ConsoleCommandParserDefault.h"
#include "serverGame/ConsoleCommandParserGuild.h"
#include "serverGame/ConsoleCommandParserManufacture.h"
#include "serverGame/ConsoleCommandParserMessageTo.h"
#include "serverGame/ConsoleCommandParserMoney.h"
#include "serverGame/ConsoleCommandParserNpc.h"
#include "serverGame/ConsoleCommandParserObject.h"
#include "serverGame/ConsoleCommandParserObjvar.h"
#include "serverGame/ConsoleCommandParserPvp.h"
#include "serverGame/ConsoleCommandParserResource.h"
#include "serverGame/ConsoleCommandParserScript.h"
#include "serverGame/ConsoleCommandParserServer.h"
#include "serverGame/ConsoleCommandParserShip.h"
#include "serverGame/ConsoleCommandParserSkill.h"
#include "serverGame/ConsoleCommandParserSpaceAi.h"
#include "serverGame/ConsoleCommandParserSpawner.h"
#include "serverGame/ConsoleCommandParserVeteran.h"
#include "serverGame/ConsoleCommandParserVoice.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedCommandParser/CommandParser.h"
#include "sharedNetworkMessages/ConsoleChannelMessages.h"

CommandParser * ConsoleMgr::ms_parser = nullptr;


//-----------------------------------------------------------------------

void ConsoleMgr::install()
{
	if (ms_parser == nullptr)
	{
		ms_parser = new ConsoleCommandParserDefault();
		ms_parser->addSubCommand(new ConsoleCommandParserCombatEngine ());
		ms_parser->addSubCommand(new ConsoleCommandParserCraft        ());
		ms_parser->addSubCommand(new ConsoleCommandParserManufacture  ());
		ms_parser->addSubCommand(new ConsoleCommandParserMoney        ());
		ms_parser->addSubCommand(new ConsoleCommandParserNpc          ());
		ms_parser->addSubCommand(new ConsoleCommandParserObject       ());
		ms_parser->addSubCommand(new ConsoleCommandParserObjvar       ());
		ms_parser->addSubCommand(new ConsoleCommandParserResource     ());
		ms_parser->addSubCommand(new ConsoleCommandParserScript       ());
		ms_parser->addSubCommand(new ConsoleCommandParserServer       ());
		ms_parser->addSubCommand(new ConsoleCommandParserSkill        ());
		ms_parser->addSubCommand(new ConsoleCommandParserSpawner      ());
		ms_parser->addSubCommand(new ConsoleCommandParserShip         ());
		ms_parser->addSubCommand(new ConsoleCommandParserSpaceAi      ());
		ms_parser->addSubCommand(new ConsoleCommandParserAi           ());
		ms_parser->addSubCommand(new ConsoleCommandParserVeteran      ());
		ms_parser->addSubCommand(new ConsoleCommandParserMessageTo    ());
		ms_parser->addSubCommand(new ConsoleCommandParserPvp          ());
		ms_parser->addSubCommand(new ConsoleCommandParserGuild        ());
		ms_parser->addSubCommand(new ConsoleCommandParserCollection   ());
		ms_parser->addSubCommand(new ConsoleCommandParserVoice        ());
		ms_parser->addSubCommand(new ConsoleCommandParserCity         ());
	}
}	// ConsoleMgr::install

//-----------------------------------------------------------------------

void ConsoleMgr::remove()
{
	if (ms_parser != nullptr)
	{
		delete ms_parser;
		ms_parser = nullptr;
	}
}	// ConsoleMgr::remove

//-----------------------------------------------------------------------

void ConsoleMgr::processString(const std::string & msg, Client *from, uint32 msgId)
{
    DEBUG_REPORT_LOG_PRINT(true, ("Console Message Received: %s\n",(msg.c_str())));

	if (ms_parser == nullptr)
	{
		DEBUG_WARNING(true, ("Console command parser has not been created!"));
		return;
	}

	CommandParser::String_t wideMsg = Unicode::narrowToWide(msg);
	CommandParser::String_t result;

	const CommandParser::ErrorType retval = ms_parser->parse(from->getCharacterObjectId (), wideMsg, result);
	if (retval == CommandParser::ERR_NO_HANDLER)
	{
		// @todo: make this message lookup from string table
		result += Unicode::narrowToWide("Console Parser Failed!");
	}
	else if (result.empty())
		result += Unicode::narrowToWide("Command [%s%s] succeeded.") + wideMsg.substr(0, 8) + ((wideMsg.size() > 8) ? Unicode::narrowToWide("...") : Unicode::emptyString);

	broadcastString(result, from, msgId);
}

//-----------------------------------------------------------------------

void ConsoleMgr::broadcastString(const std::string & msg, Client *to, uint32 msgId)
{
    ConGenericMessage m(msg, msgId);

	DEBUG_WARNING(! to, ("Do not BROADCAST messages anymore! specify which client to send to or don't send the message\n"));
	if (to)
	{
		to->send(m, true);
	}
}

//-----------------------------------------------------------------------

void ConsoleMgr::broadcastString(const std::string & msg, NetworkId to, uint32 msgId)
{
	ServerObject * object = ServerWorld::findObjectByNetworkId(to, true);
	if (object)
	{
		if (object->getClient())
			broadcastString(msg, object->getClient(), msgId);
	}
}

//-----------------------------------------------------------------------
