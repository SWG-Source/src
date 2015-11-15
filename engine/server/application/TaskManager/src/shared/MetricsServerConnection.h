// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_MetricsServerConnection_H
#define	_INCLUDED_MetricsServerConnection_H

//-----------------------------------------------------------------------

#include "sharedMessageDispatch/Transceiver.h"
#include "TaskHandler.h"

class TaskConnection;
struct Closed;

//-----------------------------------------------------------------------

class MetricsServerConnection : public TaskHandler
{
public:
	MetricsServerConnection(TaskConnection * newConnection, const std::string & commandLine);
	~MetricsServerConnection();

	void closed(const Closed &);
	void receive(const Archive::ByteStream & message);

private:
	MetricsServerConnection();
	MetricsServerConnection & operator = (const MetricsServerConnection & rhs);
	MetricsServerConnection(const MetricsServerConnection & source);
	void onProcessKilled(const struct ProcessKilled &);

private:
	MessageDispatch::Callback   callback;
	std::string                 commandLine;
	bool                        connected;
	TaskConnection *            connection;
	unsigned long lastSpawnTime;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MetricsServerConnection_H
