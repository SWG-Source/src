// ======================================================================
//
// ConsoleCommandParserGuild.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserGuild_H
#define INCLUDED_ConsoleCommandParserGuild_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to the guild system.
*/

class ConsoleCommandParserGuild : public CommandParser
{
public:
	                          ConsoleCommandParserGuild ();
	virtual bool              performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	                          ConsoleCommandParserGuild (const ConsoleCommandParserGuild & rhs);
	ConsoleCommandParserGuild & operator= (const ConsoleCommandParserGuild & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserGuild_H
