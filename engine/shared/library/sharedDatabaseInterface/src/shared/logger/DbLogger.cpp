#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbLogger.h"

#include "sharedDatabaseInterface/DbServer.h"
#include "sharedDatabaseInterface/DbTaskQueue.h"
#include "sharedDatabaseInterface/DbTaskSaveLog.h"

//-----------------------------------------------------------------------------

using namespace DB;

//-----------------------------------------------------------------------------

Logger::EventsType Logger::events;
int Logger::maxQueuedEvents;
real Logger::maxTimeBeforeSave;
real Logger::timeSinceLastSave=0;
TaskQueue *Logger::taskQueue=0;
bool Logger::installed=false;

//-----------------------------------------------------------------------------
/**
 * Set up the logger.
 *
 *
 * @param server The database server to send events to.
 * @param _maxQueuedEvents The maximum number of events that can be queued
 * before a save is forced.
 * @param _maxTimeBeforeSave The maximum amount of time that can pass before a
 * save is forced.
 */

void Logger::install(Server *server, int _maxQueuedEvents, real _maxTimeBeforeSave)
{
	maxQueuedEvents=_maxQueuedEvents;
	maxTimeBeforeSave=_maxTimeBeforeSave;
	//TODO:  establish database connection
	taskQueue=new TaskQueue(1,server,0);

	installed=true;
}

//-----------------------------------------------------------------------------

void Logger::remove()
{
	//TODO:  wait for task queue to complete - or will destructor handle it?
	delete taskQueue;
}

//-----------------------------------------------------------------------------
/**
 * Add an event to the queue to be logged.
 *
 * @param event The event to be logged.  Will be deleted when the logger is done
 * with it.
 */

void Logger::log(LogEvent *event)
{
	events.push_back(event);
}

//-----------------------------------------------------------------------------
/**
 * Evaluates whether it is time to save the accumulated log events.
 *
 * This should be called periodically, i.e. once per game loop.
 *
 * @param time The time in seconds since this function was last invoked.
 */

void Logger::update(real time)
{
	timeSinceLastSave+=time;
	if ((timeSinceLastSave > 10) || (events.size() > 10))
		beginSave();
}

//-----------------------------------------------------------------------------
/**
 * Start an asynchronous task to save the current events.
 *
 * We don't care if another save is already in progress, because save tasks
 * should not conflict with eachother.
 */

void Logger::beginSave()
{
	if (events.size() == 0)
		return;

	TaskSaveLog *tsl=new TaskSaveLog;

	// Copy events to the new task
	for (EventsType::iterator i=events.begin(); i!=events.end(); ++i)
	{
		tsl->addEvent(*i);
	}

	events.clear();

	taskQueue->asyncRequest(tsl);
} //lint !e429 // didn't free tsl

//-----------------------------------------------------------------------------

