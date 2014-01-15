// ======================================================================
//
// ConsoleCommandParserSpawner.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserSpawner_H
#define INCLUDED_ConsoleCommandParserSpawner_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to Spawners
*/

class ConsoleCommandParserSpawner : 
public CommandParser
{
public:
                                 ConsoleCommandParserSpawner ();
    virtual bool                 performParsing (const NetworkId & userId, const StringVector_t & argv,const String_t & originalCommand,String_t & result,const CommandParser * node);
	
private:
                                 ConsoleCommandParserSpawner (const ConsoleCommandParserSpawner & rhs);
    ConsoleCommandParserSpawner &  operator= (const ConsoleCommandParserSpawner & rhs);    
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserSpawner_H












