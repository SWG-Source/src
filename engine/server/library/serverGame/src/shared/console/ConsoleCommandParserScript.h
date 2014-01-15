// ======================================================================
//
// ConsoleCommandParserScript.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserScript_H
#define INCLUDED_ConsoleCommandParserScript_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to scripts
*/

class ConsoleCommandParserScript : public CommandParser
{
public:
	                          ConsoleCommandParserScript ();
	virtual bool              performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	                          ConsoleCommandParserScript (const ConsoleCommandParserScript & rhs);
	ConsoleCommandParserScript &  operator= (const ConsoleCommandParserScript & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserScript_H
