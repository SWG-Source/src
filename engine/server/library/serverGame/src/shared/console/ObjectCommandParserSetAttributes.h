// ======================================================================
//
// ObjectCommandParserSetAttributes.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectCommandParserSetAttributes_H
#define INCLUDED_ObjectCommandParserSetAttributes_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to scripts
*/

class ObjectCommandParserSetAttributes : public CommandParser
{
public:
	                          ObjectCommandParserSetAttributes ();
	virtual bool              performParsing (NetworkId userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	                          ObjectCommandParserSetAttributes (const ObjectCommandParserSetAttributes & rhs);
	ObjectCommandParserSetAttributes &  operator= (const ObjectCommandParserSetAttributes & rhs);
};

// ======================================================================

#endif	// INCLUDED_ObjectCommandParserSetAttributes_H
