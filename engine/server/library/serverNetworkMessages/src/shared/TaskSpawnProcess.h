// TaskSpawnProcess.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TaskSpawnProcess_H
#define	_INCLUDED_TaskSpawnProcess_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class TaskSpawnProcess : public GameNetworkMessage
{
public:
	TaskSpawnProcess(const std::string & targetHostAddress, const std::string & processName, const std::string & options, unsigned int spawnDelay = 0);
	TaskSpawnProcess(Archive::ReadIterator & r);
	~TaskSpawnProcess();

	const std::string &              getOptions            () const;
	const std::string &              getProcessName        () const;
	const std::string &              getTargetHostAddress  () const;
	int                              getTransactionId      () const;
	unsigned int                     getSpawnDelay         () const;

private:
	TaskSpawnProcess & operator = (const TaskSpawnProcess & rhs);
	TaskSpawnProcess(const TaskSpawnProcess & source);

	Archive::AutoVariable<std::string>    options;
	Archive::AutoVariable<std::string>    processName;
	Archive::AutoVariable<std::string>    targetHostAddress;
	Archive::AutoVariable<int>            transactionId;
	Archive::AutoVariable<unsigned int>   spawnDelay;	// seconds
};

//-----------------------------------------------------------------------

inline const std::string & TaskSpawnProcess::getOptions() const
{
	return options.get();
}

//-----------------------------------------------------------------------

inline const std::string & TaskSpawnProcess::getProcessName() const
{
	return processName.get();
}

//-----------------------------------------------------------------------

inline const std::string & TaskSpawnProcess::getTargetHostAddress() const
{
	return targetHostAddress.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TaskSpawnProcess_H
