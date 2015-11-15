// ======================================================================
//
// ConsoleCommandParserServer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserServer_H
#define INCLUDED_ConsoleCommandParserServer_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to scripts
*/

class ConsoleCommandParserServer : public CommandParser
{
public:
    ConsoleCommandParserServer ();
    virtual bool performParsing (const NetworkId & userId, 
								 const StringVector_t & argv,
                                 const String_t & originalCommand,
                                 String_t & result,
                                 const CommandParser * node);
	
private:
    ConsoleCommandParserServer (const ConsoleCommandParserServer & rhs);
    ConsoleCommandParserServer &  operator= (const ConsoleCommandParserServer & rhs);

	bool performParsing2 (const NetworkId & userId, 
		const StringVector_t & argv,
		const String_t & originalCommand,
		String_t & result,
		const CommandParser * node);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserServer_H
