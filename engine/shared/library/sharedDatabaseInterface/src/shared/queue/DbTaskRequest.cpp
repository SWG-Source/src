#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

#include "sharedDatabaseInterface/DbException.h"
#include "sharedDatabaseInterface/DbServer.h"
#include "sharedDatabaseInterface/DbTestQuery.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"

DB::TaskRequest::TaskRequest() :
		needsProcessing(true),
		errorCount(0),
		reconnectErrorCount(0)
{
}

DB::TaskRequest::~TaskRequest()
{
}

void DB::TaskRequest::workerThreadLoop(DB::Session *ses)
{
	int i = 0;

	while (needsProcessing)
	{
		i++;
		if (process(ses))
		{
			needsProcessing=false;
			errorCount=0;
			if (i > 1)
			{
				LOG("DatabaseError", ("Task succeeded after %d tries", i));
				ses->setFatalOnError(false);
			}
		}
		else
		{
			// save off the failed query statement from the process() that failed
			// because it will be overwritten by the test query statement

			ses->rollbackTransaction();
			Os::sleep(DB::Server::getErrorSleepTime());
			DBTestQuery qry;
			if (ses->exec(&qry))
			{
				if (qry.fetch() > 0)
				{
					int result;
					qry.value.getValue(result);
					if (result == 1)
					{
						// the connection is good, but the query in process() failed
						errorCount++;
						LOG("DatabaseError", ("  -- DBTestQuery succeeded, errorCount = %d", errorCount));
						if (errorCount==2)
							ses->setFatalOnError(true);
						continue;
					}
				}
			}
			else
				LOG("DatabaseError", ("  -- DBTestQuery failed, will disconnect/reconnect"));
			ses->disconnect();
			while (! (ses->connect()))
			{
				ses->disconnect();
				Os::sleep(DB::Server::getDisconnectSleepTime());
			};
			LOG("DatabaseError", ("Database connection reestablished"));
		}
	}
	ses->updateServerQueryCount();
	ses->reset();
}

bool DB::TaskRequest::mainThreadFinish()
{
	bool allDone=true;

	if (!isFinished())
	{
		allDone=false;
		needsProcessing=true;
	}
	return allDone;
}

void DB::TaskRequest::mainThreadOnComplete()
{
	onComplete();
}

bool DB::TaskRequest::isFinished()
{
	return true;
}

