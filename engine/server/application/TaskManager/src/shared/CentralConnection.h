// CentralConnection.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_CentralConnection_H
#define	_INCLUDED_CentralConnection_H

//-----------------------------------------------------------------------

#include "sharedMessageDispatch/Transceiver.h"
#include "TaskHandler.h"

class TaskConnection;
struct Closed;

//-----------------------------------------------------------------------

class CentralConnection : public TaskHandler
{
public:
	CentralConnection(TaskConnection * newConnection, const std::string & commandLine);
	~CentralConnection();

	void closed(const Closed &);
	void receive(const Archive::ByteStream & message);

private:
	CentralConnection & operator = (const CentralConnection & rhs);
	CentralConnection(const CentralConnection & source);
	void onProcessKilled(const struct ProcessKilled &);

private:
	MessageDispatch::Callback   callback;
	std::string                 commandLine;
	bool                        connected;
	TaskConnection *            connection;
	unsigned long lastSpawnTime;
}; //lint !e1712 default constructor not defined for class 'CentralConnection'

//-----------------------------------------------------------------------

#endif	// _INCLUDED_CentralConnection_H
