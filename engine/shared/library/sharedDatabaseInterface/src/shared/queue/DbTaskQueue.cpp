#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbTaskQueue.h"

#include "sharedDatabaseInterface/DbServer.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedThread/RunThread.h"

#include <string>

using namespace DB;

volatile int TaskQueue::sm_iTotalNumRequests = 0;
volatile int TaskQueue::sm_iTotalNumResults = 0;


bool TaskQueue::ms_workerThreadsLoggingEnabled = false;

TaskQueue::TaskQueue(unsigned int _numThreads, Server *_server, int _threadGroupId) :
		numThreads(_numThreads),
		server(_server),
		requestQ(),
		requestQLock(),
		requestQAdded(requestQLock),
		resultQ(),
		resultQLock(),
		pauseLock(),
		pauseSignal(pauseLock),
		workers(),
		shutdown(false),
		paused(false),
		updateCountLock(),
		activeCount(_numThreads),
		idleCount(0),
		threadGroupId(_threadGroupId),
		numRequests(0),
		numResults(0),
		threadIdLock(),
		nextThreadId(0)
{ //lint !e1926 // using default constructors for several members -- is intentional
	for (unsigned int i=0; i<numThreads; ++i)
	{
		Thread * threadObject = new MemberFunctionThreadZero<TaskQueue>("DBTaskQueue", *this, &TaskQueue::workerThreadLoop);
		ThreadHandle tempThreadHandle(threadObject);
		workers.push_back(tempThreadHandle);
	}
}

TaskQueue::~TaskQueue()
{
	shutdown=true;
	unpause();
	requestQAdded.signal(); // get the attention of waiting threads
	for(unsigned int i=0; i<numThreads; ++i)
	{
		workers[i]->wait();
	}

	while (!resultQ.empty())
	{
		update(0); // empty out the result queue before quitting
	}
} //lint !e1740 // didn't delete server

void TaskQueue::asyncRequest(TaskRequest *req)
{
	requestQLock.enter();
	++numRequests;
	++sm_iTotalNumRequests;
	requestQ.push_front(req);
	requestQAdded.signal();
	requestQLock.leave();
}

void TaskQueue::workerThreadLoop()
{
	threadIdLock.enter();
	int threadId=nextThreadId++;
	threadIdLock.leave();
	Os::OsPID_t processId=Os::getProcessId();
	LOG("WorkerThreads",("Thread %d-%d (%d) starting",threadGroupId,threadId,processId));
	
	Session * session = server->getSession();
	while (!shutdown)
	{
		TaskRequest *req;

		if (ms_workerThreadsLoggingEnabled)
			LOG("WorkerThreads",("Thread %d-%d (%d) about to grab a task",threadGroupId,threadId, processId));
		requestQLock.enter();

		updateCountLock.enter();
		--activeCount;
		++idleCount;
		updateCountLock.leave();

		while (requestQ.empty())
		{
			if (shutdown) // only check for shutdown when there's nothing left to do
			{
				// .leave and wake next thread
				requestQAdded.signal(); // on shutdown, each thread wakes up the next one

				updateCountLock.enter();
				--idleCount;
				updateCountLock.leave();

				requestQLock.leave();
				
				server->releaseSession(session);
				session=0;
				
				return;
			}	

			if (ms_workerThreadsLoggingEnabled)
				LOG("WorkerThreads",("Thread %d-%d (%d) waiting for a task",threadGroupId,threadId,processId));
			requestQAdded.wait();
		}

        // locked
		req=requestQ.back(); // FIFO:  added to front, removed from back
		requestQ.pop_back();
		--numRequests;
		--sm_iTotalNumRequests;
		if (!requestQ.empty())
		{
			requestQAdded.signal();
		}

		updateCountLock.enter();
		--idleCount;
		++activeCount;
		updateCountLock.leave();

        requestQLock.leave();

		// Check for the queue being paused
		pauseLock.enter();
		while (paused)
		{
			updateCountLock.enter();
			++idleCount;
			--activeCount;
			updateCountLock.leave();

  			pauseSignal.wait();

			updateCountLock.enter();
			--idleCount;
			++activeCount;
			updateCountLock.leave();
  		}
		pauseLock.leave();
		
		NOT_NULL(session);
		if (ms_workerThreadsLoggingEnabled)
			LOG("WorkerThreads",("Thread %d-%d (%d) running",threadGroupId,threadId,processId));
		req->workerThreadLoop(session);
		if (ms_workerThreadsLoggingEnabled)
			LOG("WorkerThreads",("Thread %d-%d (%d) finished a loop (about to enter resultQLock)",threadGroupId,threadId,processId));
		resultQLock.enter();
		if (ms_workerThreadsLoggingEnabled)
			LOG("WorkerThreads",("Thread %d-%d (%d) finished a loop (entered resultQLock)",threadGroupId,threadId,processId));
		++numResults;
		++sm_iTotalNumResults;
		if (ms_workerThreadsLoggingEnabled)
			LOG("WorkerThreads",("Thread %d-%d (%d) finished a loop (incremented numResults)",threadGroupId,threadId,processId));
		resultQ.push_front(req);
		if (ms_workerThreadsLoggingEnabled)
			LOG("WorkerThreads",("Thread %d-%d (%d) finished a loop (did resultQ.push_front)",threadGroupId,threadId,processId));
		resultQLock.leave();
		if (ms_workerThreadsLoggingEnabled)
			LOG("WorkerThreads",("Thread %d-%d (%d) finished a loop (left resultQLock)",threadGroupId,threadId,processId));
	}

	server->releaseSession(session);
	session=0;
}

void TaskQueue::update(float maxTime)
{
	PerformanceTimer timer;
	timer.start();
	
	TaskRequest *req;

	resultQLock.enter();

	while (!resultQ.empty() && (maxTime==0 || timer.getSplitTime() < maxTime))
	{
		req=resultQ.back();
		resultQ.pop_back();
		--numResults;
		--sm_iTotalNumResults;
		resultQLock.leave();
		
		if (req->mainThreadFinish())
		{
			req->mainThreadOnComplete();
			delete req;
		}
		else
		{
			// if Finish() returns false, more worker thread processing is needed
			asyncRequest(req);
		}
		resultQLock.enter();
	}
	resultQLock.leave();
}

void TaskQueue::cancel()
{
	requestQLock.enter();
	while(!requestQ.empty())
	{
		TaskRequest *req=requestQ.back();
		delete req;
		requestQ.pop_back();
	}
	requestQLock.leave();	
}

int TaskQueue::getNumPendingTasks()
{
	return numRequests;
}

int TaskQueue::getNumFinishedTasks()
{
	return numResults;
}

int TaskQueue::getNumActiveTasks()
{
	return activeCount;
}

void TaskQueue::report()
{
	DEBUG_REPORT_LOG(true,("Task queue status:  %i threads, %i active, %i idle, %i undelegated tasks\n",numThreads,activeCount,idleCount,getNumPendingTasks()));
}

bool TaskQueue::isIdle()
{
	return (activeCount==0 && getNumPendingTasks()==0 && getNumFinishedTasks()==0 && !paused);
}

void TaskQueue::pause()
{
	pauseLock.enter();
	paused=true;
	pauseLock.leave();
}

void TaskQueue::unpause()
{
	pauseLock.enter();
	if (paused)
	{
		paused=false;
		pauseSignal.broadcast();
	}
	pauseLock.leave();
}

void TaskQueue::enableWorkerThreadsLogging(bool enabled)
{
	ms_workerThreadsLoggingEnabled=enabled;
}
