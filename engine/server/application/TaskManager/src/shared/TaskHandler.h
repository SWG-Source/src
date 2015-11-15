// TaskHandler.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TaskHandler_H
#define	_INCLUDED_TaskHandler_H

//-----------------------------------------------------------------------

namespace Archive
{
	class ByteStream;
}

//-----------------------------------------------------------------------

class TaskHandler
{
public:
	TaskHandler();
	virtual ~TaskHandler() = 0;

	virtual void receive(const Archive::ByteStream & message) = 0;

private:
	TaskHandler & operator = (const TaskHandler & rhs);
	TaskHandler(const TaskHandler & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TaskHandler_H
