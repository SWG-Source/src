// ======================================================================
//
// ConsoleCommandParserCraft.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserCraft_H
#define INCLUDED_ConsoleCommandParserCraft_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to scripts
*/

class ConsoleCommandParserCraft : public CommandParser
{
public:
    ConsoleCommandParserCraft ();
    virtual bool performParsing (const NetworkId & userId, 
								 const StringVector_t & argv,
                                 const String_t & originalCommand,
                                 String_t & result,
                                 const CommandParser * node);
	
private:
    ConsoleCommandParserCraft (const ConsoleCommandParserCraft & rhs);
    ConsoleCommandParserCraft &  operator= (const ConsoleCommandParserCraft & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserCraft_H












