// ======================================================================
//
// ConfigServerDatabase.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverUtility/ConfigServerUtility.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkId.h"
#include <string>

//-------------------------------------------------------------------

ConfigServerDatabase::Data* ConfigServerDatabase::data;
static ConfigServerDatabase::Data staticData;

//-------------------------------------------------------------------

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("dbProcess", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("dbProcess", #a, b))
#define KEY_FLOAT(a,b)  (data->a = ConfigFile::getKeyFloat("dbProcess", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("dbProcess", #a, b))

//-------------------------------------------------------------------

void ConfigServerDatabase::install(void)
{
	ExitChain::add(ConfigServerDatabase::remove,"ConfigServerDatabase::remove");

	ConfigServerUtility::install();
	data = &staticData;

	KEY_INT     (objvarNameCleanupTime, 0);
	KEY_INT     (orphanedObjectCleanupTime, 0);
	KEY_INT     (marketAttributesCleanupTime, 0);
	KEY_INT     (messagesCleanupTime, 0);
	KEY_INT     (brokenObjectCleanupTime, 0);
	KEY_INT     (vendorObjectCleanupTime, 0);
	KEY_STRING  (customSQLFilename,"");
	KEY_BOOL    (enableFixBadCells, false);
	KEY_STRING  (objectTemplateListUpdateFilename,"");

	KEY_STRING  (DSN,"gameserver");
	KEY_STRING  (alternateDSN,"");
	KEY_STRING  (databaseUID,"gameserver");
	KEY_STRING  (databasePWD,"gameserver");
	KEY_STRING  (databaseProtocol,"DEFAULT");
	KEY_STRING  (centralServerAddress, "localhost");
	KEY_INT     (centralServerPort, 44451);
	KEY_STRING  (commoditiesServerAddress, "localhost");
	KEY_INT     (commoditiesServerPort, 44457);	//todo: confirm that this is a good port MSH
	KEY_INT     (taskManagerPort, 60001);
	KEY_INT     (expectedDBVersion, 270);
	KEY_BOOL    (correctDBVersionRequired,true);
	KEY_INT     (saveFrequencyLimit,10);
	KEY_STRING  (schemaOwner, "");
	KEY_STRING  (goldSchemaOwner, "");
	KEY_FLOAT   (uniqueMessageCacheTimeSec, 30.0f);
	KEY_INT     (loaderThreads,2);
	KEY_INT     (persisterThreads,1);
	KEY_INT     (newCharacterThreads,1);
	KEY_INT     (characterImmediateDeleteMinutes,120);
	KEY_BOOL    (logObjectLoading, false);
	KEY_BOOL    (enableQueryProfile, false);
	KEY_BOOL    (verboseQueryMode, false);
	KEY_BOOL    (logWorkerThreads, false);
	KEY_STRING  (gameServiceBindInterface, "");
	KEY_BOOL    (reportSaveTimes, false);
	KEY_BOOL    (shouldSleep, true);
	KEY_BOOL    (enableLoadLocks, true);
	KEY_INT     (databaseReconnectTime, 0);
	KEY_BOOL    (logChunkLoading,false);
	KEY_BOOL    (useMemoryManagerForOCI,false);
	KEY_INT     (maxCharactersPerLoadRequest,10);
	KEY_INT     (maxChunksPerLoadRequest,200);
	KEY_FLOAT   (maxLoadStartDelay,300.0f);
	KEY_INT     (maxErrorCountBeforeDisconnect,5);
	KEY_INT     (maxErrorCountBeforeBailout,15);
	KEY_INT     (errorSleepTime,5000);
	KEY_INT     (disconnectSleepTime,30000);
	KEY_INT     (saveAtModulus,-1);
	KEY_INT     (saveAtDivisor,10);
	KEY_INT     (backloggedQueueSize,50);
	KEY_INT     (backloggedRecoveryQueueSize,25);
	KEY_INT     (maxTimewarp,data->saveFrequencyLimit * 2 < 600 ? 600 : data->saveFrequencyLimit * 2);
	KEY_BOOL    (enableObjvarPacking, true);
	KEY_INT     (prefetchNumRows,0);
	KEY_INT     (prefetchMemory,0);
	KEY_INT     (defaultFetchBatchSize,1000);
	KEY_INT     (queryReportingRate,60);
	KEY_INT     (enableDatabaseErrorLogging, 0);
	KEY_INT     (defaultMessageBulkBindSize, 1000);
	KEY_BOOL    (enableGoldDatabase, false);
	KEY_STRING  (maxGoldNetworkId, "10000000");
	KEY_FLOAT   (defaultQueueUpdateTimeLimit, 0.25f);
	KEY_BOOL    (enableDataCleanup, false);
	KEY_INT     (defaultLazyDeleteBulkBindSize, 100);
	KEY_INT     (defaultLazyDeleteSleepTime, 1000);
	KEY_INT     (writeDelay, 0);
	KEY_BOOL    (delayUnloadIfObjectStillHasData, true);
	KEY_FLOAT   (experienceConsolidationTime, 0.0f);
	KEY_INT     (maxLoaderFinishedTasks, 100);
	KEY_FLOAT   (reportLongFrameTime, 1.0f);
	KEY_BOOL    (enableVerboseMessageLogging, false);
	KEY_BOOL    (profilerExpandAll, true);
	KEY_INT     (profilerDisplayPercentageMinimum, 0);
	KEY_BOOL    (fatalOnDataError, false);
	KEY_INT     (maxUnackedLoadCount, 1000000000);  // by default, set to an "unlimited" number, and use maxUnackedLoadCountPerServer as the cap
	KEY_INT     (maxUnackedLoadCountPerServer, 2);
	KEY_INT     (auctionLocationLoadBatchSize, 100);
	KEY_INT     (auctionLoadBatchSize, 100);
	KEY_INT     (auctionAttributeLoadBatchSize, 100);
	KEY_INT     (auctionBidLoadBatchSize, 100);
	KEY_INT     (oldestUnackedLoadAlertThresholdSeconds, 10*60); // seconds
}

//-------------------------------------------------------------------

void ConfigServerDatabase::remove(void)
{
	ConfigServerUtility::remove();
}

// ----------------------------------------------------------------------

const NetworkId & ConfigServerDatabase::getMaxGoldNetworkId(void)
{
	static const NetworkId theValue(std::string(data->maxGoldNetworkId));
	return theValue;
}

// ======================================================================
