// TaskEnumCluster.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "TaskEnumCluster.h"

//-----------------------------------------------------------------------

TaskEnumCluster::TaskEnumCluster(const std::vector<std::string> & n) :
GameNetworkMessage("TaskEnumCluster"),
clusterNames()
{
	clusterNames.set(n);
	addVariable(clusterNames);
}

//-----------------------------------------------------------------------

TaskEnumCluster::TaskEnumCluster(Archive::ReadIterator & source) :
GameNetworkMessage("TaskEnumCluster"),
clusterNames()
{
	addVariable(clusterNames);
	unpack(source);
}

//-----------------------------------------------------------------------

TaskEnumCluster::~TaskEnumCluster()
{
}

//-----------------------------------------------------------------------

