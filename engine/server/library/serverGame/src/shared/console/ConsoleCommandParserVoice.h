// ======================================================================
//
// ConsoleCommandParserVoice.h
// copyright (c) 2008 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserVoice_H
#define INCLUDED_ConsoleCommandParserVoice_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to voice chat
*/

class ConsoleCommandParserVoice : public CommandParser
{
public:
    ConsoleCommandParserVoice ();
    virtual bool performParsing (const NetworkId & userId, 
								 const StringVector_t & argv,
                                 const String_t & originalCommand,
                                 String_t & result,
                                 const CommandParser * node);
	
private:
    ConsoleCommandParserVoice (const ConsoleCommandParserVoice & rhs);
    ConsoleCommandParserVoice &  operator= (const ConsoleCommandParserVoice & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserVoice_H












