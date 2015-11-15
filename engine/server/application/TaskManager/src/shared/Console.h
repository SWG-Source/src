// Console.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_Console_H
#define	_INCLUDED_Console_H

//-----------------------------------------------------------------------

#include <string>

//-----------------------------------------------------------------------

class ConsoleImplementation;

//-----------------------------------------------------------------------

class Console
{
public:
	~Console();

	static const bool         hasPendingCommand  ();
	static const std::string  getNextCommand     ();
	static void               update             ();

private:
	Console();
	Console & operator = (const Console & rhs);
	Console(const Console & source);

	static Console & instance();
	const char  getNextChar();
private:
	ConsoleImplementation *  consoleImplementation;
	std::string              currentCommand;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_Console_H
