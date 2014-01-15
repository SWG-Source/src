// ======================================================================
//
// ConsoleCommandParserAi.h
// Copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserAi_H
#define INCLUDED_ConsoleCommandParserAi_H

#include "sharedCommandParser/CommandParser.h"
#include "sharedFoundation/DynamicVariableList.h"

// ======================================================================

/**
* Commands that are related to objvars
*/

class ConsoleCommandParserAi : public CommandParser
{
public:

	ConsoleCommandParserAi();
	virtual bool performParsing(const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:

	ConsoleCommandParserAi(const ConsoleCommandParserAi & rhs);
	ConsoleCommandParserAi & operator =(const ConsoleCommandParserAi & rhs);
	
	int getArgumentType(const Unicode::NarrowString &arg) const;
	void listObjvars(const DynamicVariableList::NestedList &objvarList, String_t &result, int tabCount, bool withPackingData);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserAi_H
