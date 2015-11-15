// ConsoleCommandParserGame.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ConsoleCommandParserGame_H
#define	_INCLUDED_ConsoleCommandParserGame_H

//-----------------------------------------------------------------------

#include "sharedCommandParser/CommandParser.h"

//-----------------------------------------------------------------------

class ConsoleCommandParserGame : public CommandParser
{
public:
	ConsoleCommandParserGame();
	~ConsoleCommandParserGame();
	virtual bool performParsing(const NetworkId & userId, const StringVector_t & argv,const String_t & originalCommand,String_t & result,const CommandParser * node);

private:
	ConsoleCommandParserGame & operator = (const ConsoleCommandParserGame & rhs);
	ConsoleCommandParserGame(const ConsoleCommandParserGame & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConsoleCommandParserGame_H
