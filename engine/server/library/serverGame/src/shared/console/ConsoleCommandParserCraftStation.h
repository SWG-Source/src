// ======================================================================
//
// ConsoleCommandParserCraftStation.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserCraftStation_H
#define INCLUDED_ConsoleCommandParserCraftStation_H

#include "sharedCommandParser/CommandParser.h"

class TangibleObject;

// ======================================================================

/**
* Commands that are related to scripts
*/

class ConsoleCommandParserCraftStation : public CommandParser
{
public:
    ConsoleCommandParserCraftStation ();
    virtual bool performParsing (const NetworkId & userId, 
								 const StringVector_t & argv,
                                 const String_t & originalCommand,
                                 String_t & result,
                                 const CommandParser * node);
	
private:
    ConsoleCommandParserCraftStation (const ConsoleCommandParserCraftStation & rhs);
    ConsoleCommandParserCraftStation &  operator= (const ConsoleCommandParserCraftStation & rhs);

	TangibleObject * getStation(const StringVector_t & argv, String_t & result);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserCraftStation_H












