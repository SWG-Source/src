#ifndef	_DB_TASK_QUEUE_H
#define	_DB_TASK_QUEUE_H

#include <list>
#include <vector>

#include "sharedSynchronization/Mutex.h"
#include "sharedSynchronization/ConditionVariable.h"
#include "sharedThread/ThreadHandle.h"

namespace DB
{
	
class Server;
class Session;

class TaskRequest;
class TaskWorkerThread;

/** A class organizing a queue of requests to the database.  Worker threads will
* perform the requests on a FIFO basis.
*/
class TaskQueue
{
	unsigned int numThreads;
	DB::Server *server;
	
	std::list<TaskRequest*> requestQ;
	Mutex requestQLock;
	ConditionVariable requestQAdded;
	std::list<TaskRequest*> resultQ;
	Mutex resultQLock;
	Mutex pauseLock;
	ConditionVariable pauseSignal;
	
	std::vector<ThreadHandle> workers;

	/** Set to true when we are trying to delete the queue.  Worker
	 * threads test this to see whether to quit.
	 */
	bool shutdown;
	bool paused;

	Mutex updateCountLock;
	int activeCount;
	int idleCount;

	int threadGroupId;

	int numRequests;
	int numResults;

	Mutex threadIdLock;
	int nextThreadId;

	static bool ms_workerThreadsLoggingEnabled;
	
public:
	TaskQueue(unsigned int _numThreads, Server *_server, int _threadGroupId);
	~TaskQueue();

	/** Give a request to the task queue.  It will hand the request to a
	 *  worker thread to be processed.
	 */
	void asyncRequest(TaskRequest *req);

	/** Called by the worker thread(s).  Pops a request off the queue and
	 * calls Process() on the request.  Sleeps until a request is added if
	 * there are none available.
	 *
	 * Always returns true.  TODO:  Add support for aborting the worker
	 * threads, in which case this will return false when the thread should
	 * stop itself.
	 *
	 * @param session  A database session owned by the worker thread.   It
	 * will be used for running any queries.
	 */
	void workerThreadLoop();
	
	/**
	 * The user of the queue should call this periodically.  It pops all
	 * completed tasks off of the queue and invokes their OnComplete()
	 * functions.
	 *
	 * @param maxTime The maximum time to spend processing, or 0 for unlimited.
	 * If the time limit is reached, tasks will be left on the queue for a
	 * later call to update() .
	 */
	void update(float maxTime);

	/**
	 * Remove all pending tasks that have not been started yet.
	 * If a task is currently in progress, it is not affected.
	 */
	void cancel();

	/**
	 * Pause the queue.  No new tasks will be processed until the queue is unpaused.
	 */
	void pause();

	/**
	 * Unpause the queue.
	 */
	void unpause();
	
	/**
	 * Count the number of tasks that have not yet been assigned to a
	 * worker thread.
	 */
	int getNumPendingTasks();

	/**
	 * Count the number of tasks that have been finished but not yet
	 * processed by the main thred.
	 */
	int getNumFinishedTasks();

	/**
	 * Count the number of threads that are actively working on a task
	 */
	int getNumActiveTasks();

	/**
	 * Return true if the task queue is not doing anything. (No active
	 * threads and no pending tasks.
	 */
	bool isIdle();
	
	void report();

	static void enableWorkerThreadsLogging(bool enabled);
	

	static int getTotalNumRequests() { return (sm_iTotalNumRequests < 0 ? 0 : sm_iTotalNumRequests); }
	static int getTotalNumResults() { return (sm_iTotalNumResults < 0 ? 0 : sm_iTotalNumResults); }

  private:
	TaskQueue(); //disable
	TaskQueue(const TaskQueue &); //disable
	TaskQueue& operator=(const TaskQueue&); //disable


	static volatile int sm_iTotalNumRequests;
	static volatile int sm_iTotalNumResults;
};

} 
#endif
