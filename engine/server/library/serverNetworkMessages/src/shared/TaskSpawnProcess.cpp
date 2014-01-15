// TaskSpawnProcess.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "TaskSpawnProcess.h"

//-----------------------------------------------------------------------

namespace TaskSpawnProcessNamespace
{
	int  s_transactionId = 0;
}
using namespace TaskSpawnProcessNamespace;

//-----------------------------------------------------------------------

TaskSpawnProcess::TaskSpawnProcess(const std::string & h, const std::string & p, const std::string & o, const unsigned int d/* = 0*/) :
GameNetworkMessage("TaskSpawnProcess"),
options(o),
processName(p),
targetHostAddress(h),
transactionId(++s_transactionId),
spawnDelay(d)
{
	addVariable(options);
	addVariable(processName);
	addVariable(targetHostAddress);
	addVariable(transactionId);
	addVariable(spawnDelay);
}

//-----------------------------------------------------------------------

TaskSpawnProcess::TaskSpawnProcess(Archive::ReadIterator & source) :
GameNetworkMessage("TaskSpawnProcess"),
options(),
processName(),
targetHostAddress(),
transactionId(),
spawnDelay(0)
{
	addVariable(options);
	addVariable(processName);
	addVariable(targetHostAddress);
	addVariable(transactionId);
	addVariable(spawnDelay);
	unpack(source);
}

//-----------------------------------------------------------------------

TaskSpawnProcess::~TaskSpawnProcess()
{
}

//-----------------------------------------------------------------------

int TaskSpawnProcess::getTransactionId() const
{
	return transactionId.get();
}

//-----------------------------------------------------------------------

unsigned int TaskSpawnProcess::getSpawnDelay() const
{
	return spawnDelay.get();
}

//-----------------------------------------------------------------------
