// ======================================================================
//
// NonCriticalTaskQueue.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NonCriticalTaskQueue_H
#define INCLUDED_NonCriticalTaskQueue_H

// ======================================================================

#include "Singleton/Singleton2.h"

// ======================================================================

/** 
 * A queue of tasks that should not be allowed to bog down the server.
 * Tasks can be added to this queue as they come in.  As many as possible
 * will be done each frame.  If too many tasks come in the same frame,
 * they will be spread out over multiple frames.
 *
 * A long task can do a little processing and return itself to the queue.
 * If there is not much else in the queue, it may get called again in the
 * same frame.
 *
 * This queue is not proxied, so tasks will be lost if the server crashes.
 * Furthermore, the queue is not aware of what objects tasks require, so
 * the tasks themselves need to check that the objects are still present.
 */
class NonCriticalTaskQueue : public Singleton2<NonCriticalTaskQueue>
{
  public:
	class TaskRequest
	{
	  public:
		/**
		 * Do the processing for the task, or if it is a long task, do
		 * a piece of it.
		 * @return true if it is finished, false it if should
		 * be returned to the queue.
		 */
		virtual bool run() = 0;

	  public:
		virtual ~TaskRequest();
	};
	
  public:
	void addTask           (TaskRequest *newTask);
	void update            ();

  public:
	NonCriticalTaskQueue   ();
	~NonCriticalTaskQueue  ();

  private:
	typedef std::list<TaskRequest*> TaskListType;
	
	TaskListType *m_tasks;
};


// ======================================================================

#endif
