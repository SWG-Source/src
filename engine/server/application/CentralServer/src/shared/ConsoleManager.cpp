// ConsoleManager.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstCentralServer.h"
#include "ConsoleCommandParser.h"
#include "ConsoleCommandParserGame.h"
#include "ConsoleCommandParserLoginServer.h"
#include "ConsoleCommandParserDatabase.h"
#include "ConsoleManager.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

namespace ConsoleManagerNamespace
{
	ConsoleCommandParser * s_consoleCommandParserRoot = nullptr;
}

using namespace ConsoleManagerNamespace;

//-----------------------------------------------------------------------

ConsoleManager::ConsoleManager()
{
}

//-----------------------------------------------------------------------

ConsoleManager::~ConsoleManager()
{
}

//-----------------------------------------------------------------------

void ConsoleManager::install()
{
	s_consoleCommandParserRoot = new ConsoleCommandParser;
	s_consoleCommandParserRoot->addSubCommand(new ConsoleCommandParserLoginServer);
	s_consoleCommandParserRoot->addSubCommand(new ConsoleCommandParserDatabase);
	IGNORE_RETURN(s_consoleCommandParserRoot->addSubCommand(new ConsoleCommandParserGame));
}

//-----------------------------------------------------------------------

void ConsoleManager::remove()
{
	delete s_consoleCommandParserRoot;
}

//-----------------------------------------------------------------------

CommandParser::ErrorType ConsoleManager::processString(const std::string & msg, const int track, std::string & result)
{
	CommandParser::ErrorType parseResult;
	Unicode::String wideResult;
	parseResult = s_consoleCommandParserRoot->parse(NetworkId(static_cast<NetworkId::NetworkIdType>(track)), Unicode::narrowToWide(msg), wideResult);
	result = Unicode::wideToNarrow(wideResult);
	return parseResult;
}

//-----------------------------------------------------------------------

