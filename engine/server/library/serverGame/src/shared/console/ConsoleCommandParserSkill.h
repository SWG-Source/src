// ======================================================================
//
// ConsoleCommandParserSkill.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserSkill_H
#define INCLUDED_ConsoleCommandParserSkill_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to skills
*/

class ConsoleCommandParserSkill : 
public CommandParser
{
public:
                                 ConsoleCommandParserSkill ();
    virtual bool                 performParsing (const NetworkId & userId, const StringVector_t & argv,const String_t & originalCommand,String_t & result,const CommandParser * node);
	
private:
                                 ConsoleCommandParserSkill (const ConsoleCommandParserSkill & rhs);
    ConsoleCommandParserSkill &  operator= (const ConsoleCommandParserSkill & rhs);    
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserSkill_H












