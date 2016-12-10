// ======================================================================
//
// Scheduler.h
// Copyright 2000 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	_INCLUDED_Scheduler_H
#define	_INCLUDED_Scheduler_H

// ======================================================================

#include "sharedFoundation/StaticCallbackEntry.h"

//-----------------------------------------------------------------------
/**	@brief Engine scheduler 
	A scheduler is a priority queue containing callbacks, which in turn
	contain pointers to member functions which are executed when the 
	callback is ready for processing. 

	The scheduler is updated with an arbitrary, unsigned long value. This
	value is used to examine a priority queue of callback entries. If
	the top element in the priority queue has an expireCount member that
	is less than the current count (passed to the scheduler update()),
	it is popped. If the object it refers to (or static callback function)
	is valid, it will invoke the requested callback.

	This process is repeated until the top element in the priority queue
	has an expireCount greater than the current count.

	To avoid infinite loops, a callback is never re-inserted in the
	queue during the queue update. If the Scheduler instance is updating,
	re-insertions are deferred, and the next earliest opportunity a 
	callback could occur is during the NEXT update to the scheduler.

	\code
	class MyObject : public Object
	{
	public:
		MyObject::MyObject() :
		{
			World::getObjectScheduler().setCallback(this, &MyObject::myExpireFunction, 1000);
		}

		MyObject::~MyObject()
		{
		}

	private:
		void          myExpireFunction()
		{
			World::killObject(this); // destroy this object now!
		}
	};

	void World::update()
	{
		// each MyObject will put itself on the world kill list
		// 1000 milliseconds after creation. They will be placed on
		// the list during the scheduler update when MyObject::myExpireFunction
		// is invoked.
		World::objectScheduler.update(Clock::timeMs()); 
	}
	\endcode

	@author Justin Randall
*/
class Scheduler
{
public:

	typedef void (*Callback)(const void*);

public:

	Scheduler();
	~Scheduler  ();

	void                 setCallback     (Callback cb, const void *context, unsigned long delay);
	const unsigned long  getCurrentCount () const;
	void                 update          (const unsigned long currentCount);

private:

	void addCallback(StaticCallbackEntry * callback);

	// Disabled.
	Scheduler & operator = (const Scheduler & rhs);
	Scheduler(const Scheduler & source);

private:

	std::vector<StaticCallbackEntry *> * deferredCallbackEntryAdditions;
	std::priority_queue<StaticCallbackEntry *, std::vector<StaticCallbackEntry *, std::allocator<StaticCallbackEntry *> >, StaticCallbackEntry::Compare> * callbackQueue;

private:

	unsigned long  currentCount;
	bool           updating;

};

//-----------------------------------------------------------------------
/** @brief Return the current expire counter for the current/most recent
	update frame.

	currentCount is updated when Scheduler::update() is invoked. It is set
	to the value of the update parameter. This value is used to determine
	which items in the priority queue should be executed next.

	@author Justin Randall
*/
inline const unsigned long Scheduler::getCurrentCount() const
{
	return currentCount;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_Scheduler_H
