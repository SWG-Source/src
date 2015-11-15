// ======================================================================
//
// ConsoleCommandParserVeteran.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserVeteran_H
#define INCLUDED_ConsoleCommandParserVeteran_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to the veteran rewards system.
*/

class ConsoleCommandParserVeteran : public CommandParser
{
public:
	                          ConsoleCommandParserVeteran ();
	virtual bool              performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	                          ConsoleCommandParserVeteran (const ConsoleCommandParserVeteran & rhs);
	ConsoleCommandParserVeteran &  operator= (const ConsoleCommandParserVeteran & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserVeteran_H
