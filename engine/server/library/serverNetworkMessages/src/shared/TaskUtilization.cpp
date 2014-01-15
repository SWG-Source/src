// TaskUtilization.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "TaskUtilization.h"

//-----------------------------------------------------------------------

TaskUtilization::TaskUtilization(const unsigned char t, const float a) :
GameNetworkMessage("TaskUtilization"),
utilType(t),
utilAmount(a)
{
	addVariable(utilType);
	addVariable(utilAmount);
}

//-----------------------------------------------------------------------

TaskUtilization::TaskUtilization(Archive::ReadIterator & source) :
GameNetworkMessage("TaskUtilization"),
utilType(0),
utilAmount(0)
{
	addVariable(utilType);
	addVariable(utilAmount);
	unpack(source);
}

//-----------------------------------------------------------------------

TaskUtilization::~TaskUtilization()
{
}

//-----------------------------------------------------------------------

TaskUtilization & TaskUtilization::operator = (const TaskUtilization & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------

