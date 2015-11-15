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
#if 0
	//Temporariy remove this since it's not giving us good results
	FILE * avg = popen("uptime", "r");
	float a = 0.0f;
	if(avg)
	{
		std::string output;
		while(!feof(avg))
		{
			char buf[1024] = {"\0"};

			fread(buf, sizeof(buf), 1, avg);
			output += buf;
		}
		char formatted[1024] = {"\0"};
		std::string load = output.substr(output.find("load average:"));
		sscanf(load.c_str(), "load average: %f", &a);
		pclose(avg);
	}
	return a;
#else

	float ret = static_cast<float>(TaskManager::getNumGameConnections());
	return ret;
	
#endif
}

//-----------------------------------------------------------------------

void TaskManagerSysInfo::update()
{

}

//-----------------------------------------------------------------------

