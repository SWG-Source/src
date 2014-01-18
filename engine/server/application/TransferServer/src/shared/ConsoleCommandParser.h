// ConsoleCommandParser.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ConsoleCommandParser_H
#define	_INCLUDED_ConsoleCommandParser_H

//-----------------------------------------------------------------------

#include "sharedCommandParser/CommandParser.h"

//-----------------------------------------------------------------------

class ConsoleCommandParser : public CommandParser
{
public:
	ConsoleCommandParser();
	~ConsoleCommandParser();
	virtual bool performParsing(const NetworkId & userId, const StringVector_t & argv,const String_t & originalCommand,String_t & result,const CommandParser * node);

private:
	ConsoleCommandParser & operator = (const ConsoleCommandParser & rhs);
	ConsoleCommandParser(const ConsoleCommandParser & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConsoleCommandParser_H

