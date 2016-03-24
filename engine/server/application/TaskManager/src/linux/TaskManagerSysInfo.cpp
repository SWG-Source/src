// TaskManagerSysInfo.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "TaskManagerSysInfo.h"
#include <sys/sysinfo.h>

#include "TaskManager.h"

//-----------------------------------------------------------------------

TaskManagerSysInfo::TaskManagerSysInfo() :
averageScore()
{
	update();
}

//-----------------------------------------------------------------------

TaskManagerSysInfo::TaskManagerSysInfo(const TaskManagerSysInfo &)
{

}

//-----------------------------------------------------------------------

TaskManagerSysInfo::~TaskManagerSysInfo()
{
}

//-----------------------------------------------------------------------

TaskManagerSysInfo & TaskManagerSysInfo::operator = (const TaskManagerSysInfo & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------

const float TaskManagerSysInfo::getScore() const
{
	float ret = static_cast<float>(TaskManager::getNumGameConnections());
	return ret;
}

//-----------------------------------------------------------------------

void TaskManagerSysInfo::update()
{

}

//-----------------------------------------------------------------------

