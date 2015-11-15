// ======================================================================
//
// ObjectCommandParserGetAttributes.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectCommandParserGetAttributes_H
#define INCLUDED_ObjectCommandParserGetAttributes_H

#include "sharedCommandParser/CommandParser.h"

// ======================================================================

/**
* Commands that are related to scripts
*/

class ObjectCommandParserGetAttributes : public CommandParser
{
public:
	                          ObjectCommandParserGetAttributes ();
	virtual bool              performParsing (NetworkId userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);
	
private:
	                          ObjectCommandParserGetAttributes (const ObjectCommandParserGetAttributes & rhs);
	ObjectCommandParserGetAttributes &  operator= (const ObjectCommandParserGetAttributes & rhs);
};

// ======================================================================

#endif	// INCLUDED_ObjectCommandParserGetAttributes_H
