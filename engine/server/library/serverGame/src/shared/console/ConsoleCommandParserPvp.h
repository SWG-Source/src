// ======================================================================
//
// ConsoleCommandParserPvp.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserPvp_H
#define INCLUDED_ConsoleCommandParserPvp_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to the Pvp system.
*/

class ConsoleCommandParserPvp : public CommandParser
{
public:
	                          ConsoleCommandParserPvp ();
	virtual bool              performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	                          ConsoleCommandParserPvp (const ConsoleCommandParserPvp & rhs);
	ConsoleCommandParserPvp & operator= (const ConsoleCommandParserPvp & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserPvp_H
