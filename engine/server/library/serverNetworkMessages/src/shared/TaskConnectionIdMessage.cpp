// TaskConnectionIdMessage.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "TaskConnectionIdMessage.h"

//-----------------------------------------------------------------------

TaskConnectionIdMessage::TaskConnectionIdMessage(enum Id id, const std::string & pCommandLine, const std::string & pClusterName) :
GameNetworkMessage("TaskConnectionIdMessage"),
serverType(static_cast<unsigned char>(id)),
commandLine(pCommandLine),
clusterName(pClusterName),
currentEpochTime(static_cast<long>(::time(nullptr)))
{
	addVariable(serverType);
	addVariable(commandLine);
	addVariable(clusterName);
	addVariable(currentEpochTime);
}

//-----------------------------------------------------------------------

TaskConnectionIdMessage::TaskConnectionIdMessage(Archive::ReadIterator & source) :
GameNetworkMessage("TaskConnectionIdMessage"),
serverType(0),
commandLine(),
clusterName(),
currentEpochTime(0)
{
	addVariable(serverType);
	addVariable(commandLine);
	addVariable(clusterName);
	addVariable(currentEpochTime);
	unpack(source);
}

//-----------------------------------------------------------------------

TaskConnectionIdMessage::~TaskConnectionIdMessage()
{
}

//-----------------------------------------------------------------------

TaskConnectionIdMessage & TaskConnectionIdMessage::operator = (const TaskConnectionIdMessage & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------
