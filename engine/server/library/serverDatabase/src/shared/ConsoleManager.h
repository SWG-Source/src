// ConsoleManager.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ConsoleManager_H
#define	_INCLUDED_ConsoleManager_H

#include <string>

//-----------------------------------------------------------------------

class ConsoleManager
{
public:
	static void install();
	static void remove();
	static void processString(const std::string & msg, const int track, std::string & result);
private:
	ConsoleManager & operator = (const ConsoleManager & rhs);
	ConsoleManager(const ConsoleManager & source);
	ConsoleManager();
	~ConsoleManager();
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConsoleManager_H
