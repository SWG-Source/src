// ConsoleCommandParserGame.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ConsoleCommandParserDatabase_H
#define	_INCLUDED_ConsoleCommandParserDatabase_H

//-----------------------------------------------------------------------

#include "sharedCommandParser/CommandParser.h"

//-----------------------------------------------------------------------

class ConsoleCommandParserDatabase : public CommandParser
{
public:
	ConsoleCommandParserDatabase();
	~ConsoleCommandParserDatabase();
	virtual bool performParsing(const NetworkId & userId, const StringVector_t & argv,const String_t & originalCommand,String_t & result,const CommandParser * node);

private:
	ConsoleCommandParserDatabase & operator = (const ConsoleCommandParserDatabase & rhs);
	ConsoleCommandParserDatabase(const ConsoleCommandParserDatabase & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConsoleCommandParserDatabase_H
