// ======================================================================
//
// ConsoleCommandParserCombatEngineQueue.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserCombatEngineQueue_H
#define INCLUDED_ConsoleCommandParserCombatEngineQueue_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to the combat engine
*/

class ConsoleCommandParserCombatEngineQueue : public CommandParser
{
public:
    ConsoleCommandParserCombatEngineQueue ();
    virtual bool performParsing (const NetworkId & userId, 
								 const StringVector_t & argv,
                                 const String_t & originalCommand,
                                 String_t & result,
                                 const CommandParser * node);
	
private:
    ConsoleCommandParserCombatEngineQueue (const ConsoleCommandParserCombatEngineQueue & rhs);
    ConsoleCommandParserCombatEngineQueue &  operator= (const ConsoleCommandParserCombatEngineQueue & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserCombatEngineQueue_H












