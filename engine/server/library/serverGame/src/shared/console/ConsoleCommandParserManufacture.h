// ======================================================================
//
// ConsoleCommandParserManufacture.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserManufacture_H
#define INCLUDED_ConsoleCommandParserManufacture_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to scripts
*/

class ConsoleCommandParserManufacture : public CommandParser
{
public:
    ConsoleCommandParserManufacture ();
    virtual bool performParsing (const NetworkId & userId, 
								 const StringVector_t & argv,
                                 const String_t & originalCommand,
                                 String_t & result,
                                 const CommandParser * node);
	
private:
    ConsoleCommandParserManufacture (const ConsoleCommandParserManufacture & rhs);
    ConsoleCommandParserManufacture &  operator= (const ConsoleCommandParserManufacture & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserManufacture_H












