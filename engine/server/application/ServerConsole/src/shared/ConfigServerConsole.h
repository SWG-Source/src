// ConfigServerConsole.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ConfigServerConsole_H
#define	_INCLUDED_ConfigServerConsole_H

//-----------------------------------------------------------------------

class ConfigServerConsole
{
public:
	ConfigServerConsole();
	~ConfigServerConsole();

	static const char * const  getServerAddress  ();
	static unsigned short      getServerPort     ();

	static void install();
	static void remove();
	
private:
	ConfigServerConsole & operator = (const ConfigServerConsole & rhs);
	ConfigServerConsole(const ConfigServerConsole & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConfigServerConsole_H
