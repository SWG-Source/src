// ConsoleImplementation.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ConsoleImplementation_H
#define	_INCLUDED_ConsoleImplementation_H

//-----------------------------------------------------------------------

#include <list>
#include <string>

//-----------------------------------------------------------------------

class ConsoleImplementation
{
public:
	ConsoleImplementation();
	~ConsoleImplementation();

	const bool         hasPendingCommand  () const;
	const std::string  popNextCommand     ();
	void               pushCommand        (const std::string & newCommand);
private:
	ConsoleImplementation & operator = (const ConsoleImplementation & rhs);
	ConsoleImplementation(const ConsoleImplementation & source);

private:
	std::list<std::string>  m_pendingCommands;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ConsoleImplementation_H
