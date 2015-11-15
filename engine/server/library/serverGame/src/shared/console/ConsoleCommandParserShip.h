//======================================================================
//
// ConsoleCommandParserShip.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ConsoleCommandParserShip_H
#define INCLUDED_ConsoleCommandParserShip_H

//======================================================================

#include "sharedCommandParser/CommandParser.h"

//----------------------------------------------------------------------

class ConsoleCommandParserShip : 
public CommandParser
{
public:
                                 ConsoleCommandParserShip ();
    virtual bool                 performParsing (const NetworkId & userId, const StringVector_t & argv,const String_t & originalCommand,String_t & result,const CommandParser * node);
	
private:
                                 ConsoleCommandParserShip (const ConsoleCommandParserShip & rhs);
    ConsoleCommandParserShip &  operator= (const ConsoleCommandParserShip & rhs);    
};

//======================================================================

#endif
