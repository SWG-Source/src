// ======================================================================
//
// ConsoleCommandParserCity.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserCity_H
#define INCLUDED_ConsoleCommandParserCity_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to the city system.
*/

class ConsoleCommandParserCity : public CommandParser
{
public:
	                          ConsoleCommandParserCity ();
	virtual bool              performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	                          ConsoleCommandParserCity (const ConsoleCommandParserCity & rhs);
	ConsoleCommandParserCity & operator= (const ConsoleCommandParserCity & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserCity_H
