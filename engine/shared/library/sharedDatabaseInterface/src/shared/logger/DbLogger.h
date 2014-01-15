#ifndef _LOGGER_H
#define _LOGGER_H

//-----------------------------------------------------------------------------

#include <vector>

//-----------------------------------------------------------------------------

namespace DB
{
	class LogEvent;
	class TaskQueue;
	class Server;
}

//-----------------------------------------------------------------------------

namespace DB
{

	/**
	 * A class that organizes sending LogEvents to the database.
	 *
	 * This class will collect LogEvents and periodically send them to
	 * the database to be recorded.
	 */
	class Logger
	{
	  public:
		static void log(LogEvent *event);
		// Maybe, in derived class:
		//   logPlayerEvent(int player, ...);
		//   etc.

		static void update(real time);

	  public:
		//TODO: come up with reasonable defaults for these parameters:
		static void install(Server *server, int _maxQueuedEvents=10, real _maxTimeBeforeSave=10.0);
		static void remove(void);

	  private:
		static void beginSave(void);
	
	  private:
		static bool installed;

		static int maxQueuedEvents;
		static real maxTimeBeforeSave;
		
		typedef std::vector<LogEvent*> EventsType;
		static EventsType events;

		static real timeSinceLastSave;

		static TaskQueue *taskQueue;
	};

} 

//-----------------------------------------------------------------------------

#endif
