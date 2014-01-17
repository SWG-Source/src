// DatabaseConnection.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_DatabaseConnection_H
#define	_INCLUDED_DatabaseConnection_H

//-----------------------------------------------------------------------

#include "TaskHandler.h"
//-----------------------------------------------------------------------

class DatabaseConnection : public TaskHandler
{
public:
	DatabaseConnection();
	~DatabaseConnection();

	void receive(const Archive::ByteStream & message);

private:
	DatabaseConnection & operator = (const DatabaseConnection & rhs);
	DatabaseConnection(const DatabaseConnection & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_DatabaseConnection_H
