// ======================================================================
//
// DataCleanupManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "DataCleanupManager.h"

#include "SwgDatabaseServer/TaskConsolidateExperience.h"
#include "SwgDatabaseServer/TaskDailyCleanup.h"
#include "SwgDatabaseServer/TaskAnySQL.h"
#include "SwgDatabaseServer/TaskObjvarNameCleanup.h"
#include "SwgDatabaseServer/TaskOrphanedObjectCleanup.h"
#include "SwgDatabaseServer/TaskMarketAttributesCleanup.h"
#include "SwgDatabaseServer/TaskMessagesCleanup.h"
#include "SwgDatabaseServer/TaskBrokenObjectCleanup.h"
#include "SwgDatabaseServer/TaskVendorObjectCleanup.h"
#include "SwgDatabaseServer/TaskFixBadCells.h"
#include "SwgDatabaseServer/TaskObjectTemplateListUpdater.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedDatabaseInterface/DbTaskQueue.h"
#include "sharedLog/Log.h"
#include "sharedLog/LogManager.h"
#include "sharedFoundation/Os.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverMetrics/MetricsManager.h"
#include "sharedNetwork/NetworkHandler.h"

// ======================================================================

DataCleanupManager::DataCleanupManager() :
		m_taskQueue(new DB::TaskQueue(1, DatabaseProcess::getInstance().getDBServer(), 0))
{
}

// ----------------------------------------------------------------------

DataCleanupManager::~DataCleanupManager()
{
	delete m_taskQueue; // Note:  The destructor of TaskQueue waits for all the tasks to be finished
}

// ----------------------------------------------------------------------

void DataCleanupManager::runDailyCleanup( void )
{
	if ( ConfigServerDatabase::getEnableFixBadCells() )  // do this first...
	{
 		TaskFixBadCells *task = new TaskFixBadCells;
       		m_taskQueue->asyncRequest(task);
	}

	if ( ConfigServerDatabase::getObjectTemplateListUpdateFilename() != 0)
	{
       		TaskObjectTemplateListUpdater *task = new TaskObjectTemplateListUpdater( ConfigServerDatabase::getObjectTemplateListUpdateFilename() );
       		m_taskQueue->asyncRequest(task);
	}

	if ( ConfigServerDatabase::getEnableDataCleanup() )
	{
		if ( ConfigServerDatabase::getExperienceConsolidationTime() != 0)
		{
        		TaskConsolidateExperience *task = new TaskConsolidateExperience( ConfigServerDatabase::getExperienceConsolidationTime() );
        		m_taskQueue->asyncRequest(task);
		}

		if ( ConfigServerDatabase::getObjvarNameCleanupTime() != 0)
		{
        		TaskObjvarNameCleanup *task = new TaskObjvarNameCleanup( ConfigServerDatabase::getObjvarNameCleanupTime() );
        		m_taskQueue->asyncRequest(task);
		}

		if ( ConfigServerDatabase::getOrphanedObjectCleanupTime() != 0)
		{
        		TaskOrphanedObjectCleanup *task = new TaskOrphanedObjectCleanup( ConfigServerDatabase::getOrphanedObjectCleanupTime() );
        		m_taskQueue->asyncRequest(task);
		}

		LOG("Cleanup:attributes:",("%d", ConfigServerDatabase::getMarketAttributesCleanupTime()  ));
		if ( ConfigServerDatabase::getMarketAttributesCleanupTime() != 0)
		{
        		TaskMarketAttributesCleanup *task = new TaskMarketAttributesCleanup( ConfigServerDatabase::getMarketAttributesCleanupTime() );
        		m_taskQueue->asyncRequest(task);
		}

		LOG("Cleanup:messages:",("%d", ConfigServerDatabase::getMarketAttributesCleanupTime()  ));
		if ( ConfigServerDatabase::getMessagesCleanupTime() != 0)
		{
        		TaskMessagesCleanup *task = new TaskMessagesCleanup( ConfigServerDatabase::getMessagesCleanupTime() );
        		m_taskQueue->asyncRequest(task);
		}

		LOG("Cleanup:brokenobj:",("%d", ConfigServerDatabase::getBrokenObjectCleanupTime()  ));
		if ( ConfigServerDatabase::getBrokenObjectCleanupTime() != 0)
		{
        		TaskBrokenObjectCleanup *task = new TaskBrokenObjectCleanup( ConfigServerDatabase::getBrokenObjectCleanupTime() );
        		m_taskQueue->asyncRequest(task);
		}

		if ( ConfigServerDatabase::getVendorObjectCleanupTime() != 0)
		{
        		TaskVendorObjectCleanup *task = new TaskVendorObjectCleanup( ConfigServerDatabase::getVendorObjectCleanupTime() );
        		m_taskQueue->asyncRequest(task);
		}
	}

	
	//
	// Open custom sql file and execute everything in there (in queries folder)
	//
	std::string s_filename( ConfigServerDatabase::getCustomSQLFilename() );
	if ( s_filename.length() )
	{
		std::string s_path( "../../src/game/server/database/queries/" );
		s_path += s_filename;
		FILE* in = fopen(s_path.c_str(), "r");
		if ( in )
		{
			char line[4096];
			std::string s_sql;
			while(  fgets(line, 4096, in ) )
			{
				if ( line[0] != '#' )
					s_sql+=line;
				if ( s_sql.find(';') != std::string::npos )
				{
					TaskAnySQL *task = new TaskAnySQL( s_sql.c_str() );
					m_taskQueue->asyncRequest( task );
					s_sql.clear();
				}
			}
			fclose( in );
		}
		else
		{
			LOG("Cleanup:AnySQL:",("Couldn't open: %s",s_path.c_str()));
		}
	}


	// Sleep until all the cleanup is done....
	while( m_taskQueue->isIdle() == false )
	{
		const int ki_sleeptime = 100;
		Os::sleep(ki_sleeptime); 
   		m_taskQueue->update(float(ki_sleeptime) / float(1000.0));
   		MetricsManager::update(float(ki_sleeptime) / float(1000.0));
		NetworkHandler::update();
		NetworkHandler::dispatch();
		NetworkHandler::clearBytesThisFrame();
	}
}


// ======================================================================
