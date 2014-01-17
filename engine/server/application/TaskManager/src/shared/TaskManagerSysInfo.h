// TaskManagerSysInfo.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TaskManagerSysInfo_H
#define	_INCLUDED_TaskManagerSysInfo_H

#include <list>
//-----------------------------------------------------------------------

class TaskManagerSysInfo
{
public:
	TaskManagerSysInfo();
	~TaskManagerSysInfo();

	void update();
	const float  getScore  () const;

private:
	TaskManagerSysInfo & operator = (const TaskManagerSysInfo & rhs);
	TaskManagerSysInfo(const TaskManagerSysInfo & source);
	std::list<float> averageScore;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TaskManagerSysInfo_H
