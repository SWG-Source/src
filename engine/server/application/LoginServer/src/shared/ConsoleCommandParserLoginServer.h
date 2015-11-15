// ConsoleCommandParserGame.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ConsoleCommandParserLoginServer_H
#define	_INCLUDED_ConsoleCommandParserLoginServer_H

//-----------------------------------------------------------------------

#include "sharedCommandParser/CommandParser.h"

//-----------------------------------------------------------------------

class ConsoleCommandParserLoginServer : public CommandParser
{
public:
	ConsoleCommandParserLoginServer();
	~ConsoleCommandParserLoginServer();
	virtual bool performParsing(const NetworkId & userId, const StringVector_t & argv,const String_t & originalCommand,String_t & result,const CommandParser * node);

private:
	ConsoleCommandParserLoginServer & operator = (const ConsoleCommandParserLoginServer & rhs);
	ConsoleCommandParserLoginServer(const ConsoleCommandParserLoginServer & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConsoleCommandParserLoginServer_H
