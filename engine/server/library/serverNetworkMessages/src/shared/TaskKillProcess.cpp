// TaskKillProcess.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "TaskKillProcess.h"

//-----------------------------------------------------------------------

TaskKillProcess::TaskKillProcess(const std::string & h, const unsigned int p, bool f) :
GameNetworkMessage("TaskKillProcess"),
m_hostName(h),
m_pid(p),
m_forceCore(f)
{
	addVariable(m_hostName);
	addVariable(m_pid);
	addVariable(m_forceCore);
}

//-----------------------------------------------------------------------

TaskKillProcess::TaskKillProcess(Archive::ReadIterator & source) :
GameNetworkMessage("TaskKillProcess"),
m_hostName(),
m_pid(),
m_forceCore()
{
	addVariable(m_hostName);
	addVariable(m_pid);
	addVariable(m_forceCore);
	unpack(source);
}

//-----------------------------------------------------------------------

TaskKillProcess::~TaskKillProcess()
{
}

//-----------------------------------------------------------------------

