// ======================================================================
//
// ConsoleCommandParserNpc.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserNpc_H
#define INCLUDED_ConsoleCommandParserNpc_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to scripts
*/

class ConsoleCommandParserNpc : public CommandParser
{
public:
    ConsoleCommandParserNpc ();
    virtual bool performParsing (const NetworkId & userId, 
								 const StringVector_t & argv,
                                 const String_t & originalCommand,
                                 String_t & result,
                                 const CommandParser * node);
	
private:
    ConsoleCommandParserNpc (const ConsoleCommandParserNpc & rhs);
    ConsoleCommandParserNpc &  operator= (const ConsoleCommandParserNpc & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserNpc_H












