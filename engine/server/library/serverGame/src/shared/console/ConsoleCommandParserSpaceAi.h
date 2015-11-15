// ======================================================================
//
// ConsoleCommandParserSpaceAi.h
// Copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserSpaceAi_H
#define INCLUDED_ConsoleCommandParserSpaceAi_H

#include "sharedCommandParser/CommandParser.h"
#include "sharedFoundation/DynamicVariableList.h"

// ======================================================================

/**
* Commands that are related to objvars
*/

class ConsoleCommandParserSpaceAi : public CommandParser
{
public:

	ConsoleCommandParserSpaceAi();
	virtual bool performParsing(const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:

	ConsoleCommandParserSpaceAi(const ConsoleCommandParserSpaceAi & rhs);
	ConsoleCommandParserSpaceAi & operator =(const ConsoleCommandParserSpaceAi & rhs);
	
	int getArgumentType(const Unicode::NarrowString &arg) const;
	void listObjvars(const DynamicVariableList::NestedList &objvarList, String_t &result, int tabCount, bool withPackingData);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserSpaceAi_H
