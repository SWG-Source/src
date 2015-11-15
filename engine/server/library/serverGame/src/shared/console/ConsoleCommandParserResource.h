// ======================================================================
//
// ConsoleCommandParserResource.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserResource_H
#define INCLUDED_ConsoleCommandParserResource_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to the resource system.
*/

class ConsoleCommandParserResource : public CommandParser
{
public:
	                          ConsoleCommandParserResource ();
	virtual bool              performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	                          ConsoleCommandParserResource (const ConsoleCommandParserResource & rhs);
	ConsoleCommandParserResource &  operator= (const ConsoleCommandParserResource & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserResource_H
