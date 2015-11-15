// ======================================================================
//
// ConsoleCommandParserMoney.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserMoney_H
#define INCLUDED_ConsoleCommandParserMoney_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to the resource system.
*/

class ConsoleCommandParserMoney : public CommandParser
{
public:
	                          ConsoleCommandParserMoney ();
	virtual bool              performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	                          ConsoleCommandParserMoney (const ConsoleCommandParserMoney & rhs);
	ConsoleCommandParserMoney &  operator= (const ConsoleCommandParserMoney & rhs);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserMoney_H
