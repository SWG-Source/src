// ======================================================================
//
// ConsoleCommandParserCombatEngine.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserCombatEngine_H
#define INCLUDED_ConsoleCommandParserCombatEngine_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to the combat engine
*/

class ConsoleCommandParserCombatEngine : public CommandParser
{
public:
    ConsoleCommandParserCombatEngine ();
    virtual bool performParsing (const NetworkId & userId, 
								 const StringVector_t & argv,
                                 const String_t & originalCommand,
                                 String_t & result,
                                 const CommandParser * node);
	
private:
    ConsoleCommandParserCombatEngine (const ConsoleCommandParserCombatEngine & rhs);
    ConsoleCommandParserCombatEngine &  operator= (const ConsoleCommandParserCombatEngine & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserCombatEngine_H












