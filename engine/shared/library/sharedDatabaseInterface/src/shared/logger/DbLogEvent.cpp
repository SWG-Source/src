#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbLogEvent.h"

#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedFoundation/Os.h"

//#include "LogQueries.h"
//#include "Schema.h"

//-----------------------------------------------------------------------------

using namespace DB;

//-----------------------------------------------------------------------------

LogEvent::LogEvent() :
		eventTime(Os::getRealSystemTime())
{
}

//-----------------------------------------------------------------------------

//bool TestEvent::save(DB::Session *session)
//{
//  DBQuery::SaveTestEvent qry;
//  	DBSchema::ServerLogRow data;

//  	qry.setData(&data);
//  	qry.insertMode();
	
//  	data.event_time=eventTime;
//  	data.server=3434;
//  	data.event_id=1;

//  	session->exec(&qry);

//  	qry.done();
	
//	return true;
//}

//-----------------------------------------------------------------------------
