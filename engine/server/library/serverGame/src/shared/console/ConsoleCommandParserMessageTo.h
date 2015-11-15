// ======================================================================
//
// ConsoleCommandParserMessageTo.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserMessageTo_H
#define INCLUDED_ConsoleCommandParserMessageTo_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to the MessageTo system.
*/

class ConsoleCommandParserMessageTo : public CommandParser
{
public:
	                          ConsoleCommandParserMessageTo ();
	virtual bool              performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	                          ConsoleCommandParserMessageTo (const ConsoleCommandParserMessageTo & rhs);
	ConsoleCommandParserMessageTo &  operator= (const ConsoleCommandParserMessageTo & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserMessageTo_H
