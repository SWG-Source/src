// ======================================================================
//
// ConsoleCommandParserObjvar.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserObjvar_H
#define INCLUDED_ConsoleCommandParserObjvar_H

#include "sharedCommandParser/CommandParser.h"
#include "sharedFoundation/DynamicVariableList.h"

// ======================================================================

/**
* Commands that are related to objvars
*/

class ConsoleCommandParserObjvar : public CommandParser
{
public:
	                          ConsoleCommandParserObjvar ();
	virtual bool              performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	                          ConsoleCommandParserObjvar (const ConsoleCommandParserObjvar & rhs);
	ConsoleCommandParserObjvar &  operator= (const ConsoleCommandParserObjvar & rhs);
	
	int                       getArgumentType(const Unicode::NarrowString &arg) const;
	void                      listObjvars(const DynamicVariableList::NestedList &objvarList, String_t &result, int tabCount, bool withPackingData);
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserObjvar_H
