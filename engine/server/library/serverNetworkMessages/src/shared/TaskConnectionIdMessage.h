// TaskConnectionIdMessage.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TaskConnectionIdMessage_H
#define	_INCLUDED_TaskConnectionIdMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class TaskConnectionIdMessage : public GameNetworkMessage
{
public:
	enum Id
	{
		Login,
		Central,
		Connection,
		Database,
		Metrics,
		Planet,
		Game,
		ServerManager,
		TaskManager
	};
public:
	TaskConnectionIdMessage(enum Id id, const std::string & pCommandLine, const std::string & pClusterName);
	TaskConnectionIdMessage(Archive::ReadIterator & source);
	~TaskConnectionIdMessage();

	const unsigned char  getServerType   () const;
	const std::string &  getCommandLine  () const;
	const std::string &  getClusterName  () const;
	const long           getCurrentEpochTime() const;
private:
	TaskConnectionIdMessage & operator = (const TaskConnectionIdMessage & rhs);
	TaskConnectionIdMessage(const TaskConnectionIdMessage & source);

	Archive::AutoVariable<unsigned char> serverType;
	Archive::AutoVariable<std::string>   commandLine;
	Archive::AutoVariable<std::string>   clusterName;
	Archive::AutoVariable<long>          currentEpochTime;
};

//-----------------------------------------------------------------------

inline const std::string & TaskConnectionIdMessage::getCommandLine() const
{
	return commandLine.get();
}

//-----------------------------------------------------------------------

inline const std::string & TaskConnectionIdMessage::getClusterName() const
{
	return clusterName.get();
}

//-----------------------------------------------------------------------

inline const unsigned char TaskConnectionIdMessage::getServerType() const
{
	return serverType.get();
}

//-----------------------------------------------------------------------

inline const long TaskConnectionIdMessage::getCurrentEpochTime() const
{
	return currentEpochTime.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TaskConnectionIdMessage_H
