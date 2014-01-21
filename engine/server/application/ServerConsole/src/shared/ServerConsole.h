// ServerConsole.h
// Copyright 2000-03, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ServerConsole_H
#define	_INCLUDED_ServerConsole_H

//-----------------------------------------------------------------------

class ServerConsole
{
public:
	ServerConsole();
	~ServerConsole();

	static void run();
	static void done();

private:
	ServerConsole & operator = (const ServerConsole & rhs);
	ServerConsole(const ServerConsole & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ServerConsole_H
