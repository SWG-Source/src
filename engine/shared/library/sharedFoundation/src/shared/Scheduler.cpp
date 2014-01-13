// ======================================================================
//
// Scheduler.cpp
// Copyright 2000 - 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/Scheduler.h"
#include "sharedFoundation/StaticCallbackEntry.h"

#include <queue>
#include <set>
#include <vector>

//-----------------------------------------------------------------------
/** @brief construct a new Scheduler object

	A system may have any number of schedulers running. Periodically
	invoke update(const unsigned long) on the Scheduler instance to
	trigger callback events.

	\code
	class MyWorld
	{
	public:
		static void update()
		{
			instance().worldScheduler.update(Clock::timeMs());
		}
	private:
		static MyWorld & instance()
		{
			static MyWorld world;
			return world;
		}
		Scheduler worldScheduler;
	};

	inst main(int, char **)
	{
		while(true)
		{
			MyWorld::update();
		}
		return 0;
	}
	\endcode

	@author Justin Randall
*/
Scheduler::Scheduler() :
deferredCallbackEntryAdditions(new std::vector<StaticCallbackEntry *>),
callbackQueue(new std::priority_queue<StaticCallbackEntry *, std::vector<StaticCallbackEntry *, std::allocator<StaticCallbackEntry *> >, StaticCallbackEntry::Compare>),
currentCount(0),
updating(false)
{
}

//-----------------------------------------------------------------------

Scheduler::~Scheduler()
{
	StaticCallbackEntry * c;
	while(!callbackQueue->empty())
	{
		c = callbackQueue->top();
		delete c;
		callbackQueue->pop();
	}
	delete callbackQueue;

	std::vector<StaticCallbackEntry *>::iterator i = deferredCallbackEntryAdditions->begin();
	for (; i != deferredCallbackEntryAdditions->end(); ++i)
	{
		delete *i;
	}
	deferredCallbackEntryAdditions->clear();
	
	delete deferredCallbackEntryAdditions;
}

//-----------------------------------------------------------------------
/**
	@brief Register a callback entry with the priority queue

	Used internally in a few places to register StaticCallbackEntry object
	pointers with the CallbackEntrySet priority queue.

	@param callback   A StaticCallbackEntry object that contains callback
	                  information such as a callback function pointer and
	                  expire count.

	@author Justin Randall
*/
void Scheduler::addCallback(StaticCallbackEntry * callback)
{
	// if the scheduler is in the middle of an update, ensure that
	// the callback is not added THIS frame (prevent infiniet callback loops
	// through long loop times or zero callback times)
	if(updating)
		deferredCallbackEntryAdditions->push_back(callback);
	else
		callbackQueue->push(callback);
}

//-----------------------------------------------------------------------
/**
	@brief The guts of the Scheduler. Request the next callback request
	for a static function.

	The Scheduler may make callbacks on object member function pointers 
	as well as static function pointers. When the Scheduler is updated
	with a value greater than (when + Scheduler::currentCount) it will
	invoke the supplied function.

	@param cb           a static function pointer
	@param context      the context for the callback function.
	@param delay        Used to calculate the expire count/time for the
	                    callback. The current count of the scheduler is
	                    added to 'delay', then the callback is inserted
	                    into the scheduler's priority queue, sorted by
	                    the calculated absolute time.

	\code
	Scheduler globalScheduler;

	void foo()
	{
		globalScheduler.setCallback(bar, 1000);
	}

	void bar()
	{
		globalScheduler.setCallback(foo, 500);
	}
	\encode

	@author Justin Randall
		
*/
void Scheduler::setCallback(Callback cb, const void *context, unsigned long delay)
{
	addCallback(new StaticCallbackEntry(getCurrentCount() + delay, cb, context));
}

//-----------------------------------------------------------------------
/** @brief Trigger pending callbacks

	A scheduler must be periodically updated to trigger callback events
	that are enqueued. A single unsigned long parameter is passed to the 
	Scheduler indicating the current "time" or "frame" or some other 
	value that ensures all callbacks with an expireCount value less than
	t will be triggered.

	@param t   The "time" or "frame" that triggers callbacks with expireCounts
	           less than t.

	\code
	int main(int, char **)
	{
		Scheduler s;
		while(1)
		{
			s.update(Clock::timeMs());
		}
		return 0;
	}
	\endcode

	@author Justin Randall
*/
void Scheduler::update(const unsigned long t)
{
	// prevent recursive updates on a scheduler
	if(!updating)
	{
		updating = true;
		currentCount = t;
		StaticCallbackEntry * c;
		while( (!callbackQueue->empty()) && (c = callbackQueue->top())->getExpireCount() <= currentCount)
		{
			c->expired();
			delete c;
			callbackQueue->pop();
		}

		updating = false;

		if(!deferredCallbackEntryAdditions->empty())
		{
			std::vector<StaticCallbackEntry *>::iterator i;
			for(i = deferredCallbackEntryAdditions->begin(); i != deferredCallbackEntryAdditions->end(); ++i)
			{
				addCallback((*i));
			}
			deferredCallbackEntryAdditions->clear();
		}
	}
}

//-----------------------------------------------------------------------
