// TaskHandler.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "TaskHandler.h"

//-----------------------------------------------------------------------

TaskHandler::TaskHandler()
{
}

//-----------------------------------------------------------------------

TaskHandler::TaskHandler(const TaskHandler &)
{

}

//-----------------------------------------------------------------------

TaskHandler::~TaskHandler()
{
}

//-----------------------------------------------------------------------

TaskHandler & TaskHandler::operator = (const TaskHandler & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------

