// ======================================================================
//
// ConsoleCommandParserCollection.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserCollection_H
#define INCLUDED_ConsoleCommandParserCollection_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to the Collection system.
*/

class ConsoleCommandParserCollection : public CommandParser
{
public:
	                          ConsoleCommandParserCollection ();
	virtual bool              performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	                          ConsoleCommandParserCollection (const ConsoleCommandParserCollection & rhs);
	ConsoleCommandParserCollection & operator= (const ConsoleCommandParserCollection & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserCollection_H
