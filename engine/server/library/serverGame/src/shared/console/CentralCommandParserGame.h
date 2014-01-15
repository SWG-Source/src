// CentralCommandParserGame.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_CentralCommandParserGame_H
#define	_INCLUDED_CentralCommandParserGame_H

//-----------------------------------------------------------------------

#include "sharedCommandParser/CommandParser.h"

//-----------------------------------------------------------------------

class CentralCommandParserGame : public CommandParser
{
public:
	CentralCommandParserGame();
	~CentralCommandParserGame();

    virtual bool performParsing (const NetworkId & userId, const StringVector_t & argv,const String_t & originalCommand,String_t & result,const CommandParser * node);

private:
	CentralCommandParserGame & operator = (const CentralCommandParserGame & rhs);
	CentralCommandParserGame(const CentralCommandParserGame & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_CentralCommandParserGame_H
