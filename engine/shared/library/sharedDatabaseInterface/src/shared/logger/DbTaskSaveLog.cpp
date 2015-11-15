#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbTaskSaveLog.h"

#include "sharedDatabaseInterface/DbLogEvent.h"

//-----------------------------------------------------------------------------

using namespace DB;

//-----------------------------------------------------------------------------
/**
 * Add a LogEvent to the list to be saved.
 */

void TaskSaveLog::addEvent(LogEvent *event)
{
	events.push_back(event);
}
	
//-----------------------------------------------------------------------------

bool TaskSaveLog::process(DB::Session *session)
{
	for (EventsType::iterator i=events.begin(); i!=events.end(); ++i)
	{
		if(! ((*i)->save(session)))
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------------

void TaskSaveLog::onComplete()
{

}

//-----------------------------------------------------------------------------

TaskSaveLog::~TaskSaveLog()
{
	for (EventsType::iterator i=events.begin(); i!=events.end(); ++i)
		delete (*i);
}

//-----------------------------------------------------------------------------
