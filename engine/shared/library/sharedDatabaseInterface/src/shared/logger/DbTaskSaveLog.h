#ifndef _TASK_SAVE_LOG_H
#define _TASK_SAVE_LOG_H

//-----------------------------------------------------------------------------

#include <vector>

#include "sharedDatabaseInterface/DbTaskRequest.h"

//-----------------------------------------------------------------------------

namespace DB
{
	class LogEvent;
}

//-----------------------------------------------------------------------------

namespace DB
{

	/**
	 * An asynchronous task to save a set of LogEvents.
	 */
	class TaskSaveLog:public DB::TaskRequest
	{
	  public:
		void addEvent(LogEvent *event);
		
	  public:
		bool process(DB::Session *session);
		void onComplete();

	  public:
		virtual ~TaskSaveLog();
	
	  private:
		typedef std::vector<LogEvent*> EventsType;
		EventsType events;
	};

} 
//-----------------------------------------------------------------------------

#endif
