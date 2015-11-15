// TaskKillProcess.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TaskKillProcess_H
#define	_INCLUDED_TaskKillProcess_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/NetworkMessageFactory.h"

//-----------------------------------------------------------------------

class TaskKillProcess : public GameNetworkMessage
{
public:
	TaskKillProcess(const std::string & hostName, const unsigned int pid, bool forceCore);
	TaskKillProcess(Archive::ReadIterator & source);
	~TaskKillProcess();

	const std::string &  getHostName     () const;
	const unsigned int   getPid          () const;
	const bool           getForceCore    () const;

private:
	TaskKillProcess & operator = (const TaskKillProcess & rhs);
	TaskKillProcess(const TaskKillProcess & source);

	Archive::AutoVariable<std::string>   m_hostName;
	Archive::AutoVariable<unsigned int>  m_pid;
	Archive::AutoVariable<bool>          m_forceCore;
};


//-----------------------------------------------------------------------

inline const std::string & TaskKillProcess::getHostName() const
{
	return m_hostName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int TaskKillProcess::getPid() const
{
	return m_pid.get();
}

//-----------------------------------------------------------------------

inline const bool TaskKillProcess::getForceCore() const
{
	return m_forceCore.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TaskKillProcess_H
