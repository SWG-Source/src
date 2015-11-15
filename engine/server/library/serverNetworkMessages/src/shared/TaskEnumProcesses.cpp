// TaskEnumProcesses.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "TaskEnumProcesses.h"

//-----------------------------------------------------------------------

TaskEnumProcesses::TaskEnumProcesses(const std::string & h, const std::vector<std::string> & c, const std::vector<uint32> & p, const std::vector<bool> & l) :
GameNetworkMessage("TaskEnumProcesses"),
commandLines(),
hostAddress(h),
loadedOnStartup(),
pids()
{
	commandLines.set(c);
	loadedOnStartup.set(l);
	pids.set(p);
	addVariable(hostAddress);
	addVariable(commandLines);
	addVariable(loadedOnStartup);
	addVariable(pids);
}

//-----------------------------------------------------------------------

TaskEnumProcesses::TaskEnumProcesses(Archive::ReadIterator & source) :
GameNetworkMessage("TaskEnumProcesses"),
commandLines(),
hostAddress(),
pids()
{
	addVariable(hostAddress);
	addVariable(commandLines);
	addVariable(loadedOnStartup);
	addVariable(pids);
	unpack(source);
}

//-----------------------------------------------------------------------

TaskEnumProcesses::~TaskEnumProcesses()
{
}

//-----------------------------------------------------------------------

