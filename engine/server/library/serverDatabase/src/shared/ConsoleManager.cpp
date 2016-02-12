// ConsoleManager.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverDatabase/FirstServerDatabase.h"
#include "sharedFoundation/NetworkId.h"
#include "ConsoleManager.h"
#include "ConsoleCommandParser.h"
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
}

//-----------------------------------------------------------------------

void ConsoleManager::remove()
{
	delete s_consoleCommandParserRoot;
}

//-----------------------------------------------------------------------

void ConsoleManager::processString(const std::string & msg, const int track, std::string & result)
{
	Unicode::String wideResult;
	IGNORE_RETURN(s_consoleCommandParserRoot->parse(NetworkId(static_cast<NetworkId::NetworkIdType>(track)), Unicode::narrowToWide(msg), wideResult));
	result = Unicode::wideToNarrow(wideResult);
}

//-----------------------------------------------------------------------

