// ======================================================================
//
// ConfigServerDatabase.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConfigServerDatabase_H
#define INCLUDED_ConfigServerDatabase_H

// ======================================================================

class NetworkId;

// ======================================================================

class ConfigServerDatabase
{
public:

	struct Data
	{
		const char *    centralServerAddress;
		int             centralServerPort;
		const char *    commoditiesServerAddress;
		int             commoditiesServerPort;

		// database configuration:
		const char *    DSN;
		const char *    alternateDSN;
		const char *    databaseUID;
		const char *    databasePWD;
		const char *    databaseProtocol;
		const char *    schemaOwner;
		const char *    goldSchemaOwner;


		int		objvarNameCleanupTime;
		int		orphanedObjectCleanupTime;
		int		marketAttributesCleanupTime;
		int		messagesCleanupTime;
		int		brokenObjectCleanupTime;
		int		vendorObjectCleanupTime;
		const char *    customSQLFilename;
		bool            enableFixBadCells;
		const char *    objectTemplateListUpdateFilename;

		int             taskManagerPort;
		int             expectedDBVersion;
		bool            correctDBVersionRequired;
		int             saveFrequencyLimit;
		float           uniqueMessageCacheTimeSec;
		int             loaderThreads;
		int             persisterThreads;
		int             newCharacterThreads;
		int             characterImmediateDeleteMinutes;
		bool            logObjectLoading;
		bool            enableQueryProfile;
		bool            verboseQueryMode;
		bool            logWorkerThreads;
		const char *    gameServiceBindInterface;
		bool            reportSaveTimes;
		bool            shouldSleep;
		bool            enableLoadLocks;
		int             databaseReconnectTime;
		bool            logChunkLoading;
		bool            useMemoryManagerForOCI;
		int             maxCharactersPerLoadRequest;
		int             maxChunksPerLoadRequest;
		float           maxLoadStartDelay;
		int             maxErrorCountBeforeDisconnect;
		int             maxErrorCountBeforeBailout;
		int             errorSleepTime;
		int             disconnectSleepTime;
		int             saveAtModulus;
		int             saveAtDivisor;
		int             backloggedQueueSize;
		int             backloggedRecoveryQueueSize;
		int             maxTimewarp;
		bool            enableObjvarPacking;
		int             prefetchNumRows;
		int             prefetchMemory;
		int             defaultFetchBatchSize;
		int             queryReportingRate;
		int             enableDatabaseErrorLogging;
		int             defaultMessageBulkBindSize;
		bool            enableGoldDatabase;
		const char*     maxGoldNetworkId;
		float           defaultQueueUpdateTimeLimit;
		bool            enableDataCleanup;
		int             defaultLazyDeleteBulkBindSize;
		int             defaultLazyDeleteSleepTime;
		int             writeDelay;
		bool            delayUnloadIfObjectStillHasData;
		float           experienceConsolidationTime;
		int             maxLoaderFinishedTasks;
		float           reportLongFrameTime;
		bool            enableVerboseMessageLogging;
		bool            profilerExpandAll;
		int             profilerDisplayPercentageMinimum;
		bool            fatalOnDataError;
		int             maxUnackedLoadCount;
		int             maxUnackedLoadCountPerServer;
		int             auctionLocationLoadBatchSize;
		int             auctionLoadBatchSize;
		int             auctionAttributeLoadBatchSize;
		int             auctionBidLoadBatchSize;
		int             oldestUnackedLoadAlertThresholdSeconds;
	};

private:

	static Data *data;

public:

	static void          install                     (void);
	static void          remove                      (void);

	static const int     getObjvarNameCleanupTime    (void);
	static const int     getOrphanedObjectCleanupTime(void);
	static const int     getMarketAttributesCleanupTime(void);
	static const int     getMessagesCleanupTime      (void);
	static const int     getBrokenObjectCleanupTime  (void);
	static const int     getVendorObjectCleanupTime(void);
	static const char *  getCustomSQLFilename        (void);
	static bool          getEnableFixBadCells        (void);
	static const char *  getObjectTemplateListUpdateFilename (void);

	static const char *  getCentralServerAddress     (void);
	static const uint16  getCentralServerPort        (void);
	static const char *  getCommoditiesServerAddress (void);
	static const uint16  getCommoditiesServerPort    (void);
	static const char *  getDSN                      (void);
	static const char *  getAlternateDSN             (void);
	static const char *  getDatabaseUID              (void);
	static const char *  getDatabasePWD              (void);
	static const char *  getDatabaseProtocol         (void);
	static const uint16  getTaskManagerPort          (void);
	static const int     getExpectedDBVersion        (void);
	static const bool    isCorrectDBVersionRequired  (void);
	static const int     getSaveFrequencyLimit       (void);
	static const char *  getSchemaOwner              (void);
	static const char *  getGoldSchemaOwner          (void);
	static const float   getUniqueMessageCacheTimeSec(void);
	static const int     getLoaderThreads            (void);
	static const int     getPersisterThreads         (void);
	static const int     getNewCharacterThreads      (void);
	static const int     getCharacterImmediateDeleteMinutes(void);
	static const bool    getLogObjectLoading         (void);
	static const bool    getEnableQueryProfile       (void);
	static const bool    getVerboseQueryMode         (void);
	static const bool    getLogWorkerThreads         (void);
	static const char *  getGameServiceBindInterface (void);
	static const bool    getReportSaveTimes          (void);
	static const bool    getShouldSleep              (void);
	static const bool    getEnableLoadLocks          (void);
	static const int     getDatabaseReconnectTime    (void);
	static const bool    getLogChunkLoading          (void);
	static const bool    getUseMemoryManagerForOCI   (void);
	static const int     getMaxCharactersPerLoadRequest(void);
	static const int     getMaxChunksPerLoadRequest  (void);
	static const float   getMaxLoadStartDelay        (void);
	static const int     getMaxErrorCountBeforeDisconnect (void);
	static const int     getMaxErrorCountBeforeBailout (void);
	static const int     getErrorSleepTime           (void);
	static const int     getDisconnectSleepTime      (void);
	static const int     getSaveAtModulus            (void);
	static const int     getSaveAtDivisor            (void);
	static const int     getBackloggedQueueSize      (void);
	static const int     getBackloggedRecoveryQueueSize(void);
	static const int     getMaxTimewarp              (void);
	static const bool    getEnableObjvarPacking      (void);
	static const int     getPrefetchNumRows          (void);
	static const int     getPrefetchMemory           (void);
	static const int     getDefaultFetchBatchSize    (void);
	static const int     getQueryReportingRate       (void);
	static const int     getEnableDatabaseErrorLogging (void);
	static const int     getDefaultMessageBulkBindSize (void);
	static const bool    getEnableGoldDatabase       (void);
	static const NetworkId &getMaxGoldNetworkId      (void);
	static const float   getDefaultQueueUpdateTimeLimit(void);
	static const bool    getEnableDataCleanup(void);
	static const int     getDefaultLazyDeleteBulkBindSize (void);
	static const int     getDefaultLazyDeleteSleepTime (void);
	static const int     getWriteDelay               (void);
	static const bool    getDelayUnloadIfObjectStillHasData(void);
	static const float   getExperienceConsolidationTime (void);
	static const int     getMaxLoaderFinishedTasks   (void);
	static const float   getReportLongFrameTime      (void);
	static const bool    getEnableVerboseMessageLogging(void);
	static const bool    getProfilerExpandAll        (void);
	static const int     getProfilerDisplayPercentageMinimum(void);
	static const bool    getFatalOnDataError         (void);
	static const int     getMaxUnackedLoadCount      (void);
	static const int     getMaxUnackedLoadCountPerServer(void);
	static const int     getAuctionLocationLoadBatchSize(void);
	static const int     getAuctionLoadBatchSize(void);
	static const int     getAuctionAttributeLoadBatchSize(void);
	static const int     getAuctionBidLoadBatchSize(void);
	static const int     getOldestUnackedLoadAlertThresholdSeconds(void);
};

//-----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getShouldSleep(void)
{
	return data->shouldSleep;
}



//-----------------------------------------------------------------------

inline const int ConfigServerDatabase::getObjvarNameCleanupTime(void)
{
	return data->objvarNameCleanupTime;
}

//-----------------------------------------------------------------------

inline const int ConfigServerDatabase::getOrphanedObjectCleanupTime(void)
{
	return data->orphanedObjectCleanupTime;
}

//-----------------------------------------------------------------------

inline const int ConfigServerDatabase::getMarketAttributesCleanupTime(void)
{
	return data->marketAttributesCleanupTime;
}

//-----------------------------------------------------------------------

inline const int ConfigServerDatabase::getMessagesCleanupTime(void)
{
	return data->messagesCleanupTime;
}

//-----------------------------------------------------------------------

inline const int ConfigServerDatabase::getBrokenObjectCleanupTime(void)
{
	return data->brokenObjectCleanupTime;
}

//-----------------------------------------------------------------------

inline const int ConfigServerDatabase::getVendorObjectCleanupTime(void)
{
	return data->vendorObjectCleanupTime;
}


//-----------------------------------------------------------------------

inline const char * ConfigServerDatabase::getCustomSQLFilename(void)
{
	return data->customSQLFilename;
}


//-----------------------------------------------------------------------

inline bool ConfigServerDatabase::getEnableFixBadCells(void)
{
	return data->enableFixBadCells;
}


//-----------------------------------------------------------------------

inline const char * ConfigServerDatabase::getObjectTemplateListUpdateFilename(void)
{
	return data->objectTemplateListUpdateFilename;
}


//-----------------------------------------------------------------------

inline const char * ConfigServerDatabase::getCentralServerAddress(void)
{
	return data->centralServerAddress;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigServerDatabase::getCentralServerPort(void)
{
	return static_cast<const uint16>(data->centralServerPort);
}

//-----------------------------------------------------------------------
inline const char * ConfigServerDatabase::getCommoditiesServerAddress(void)
{
	return data->commoditiesServerAddress;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigServerDatabase::getCommoditiesServerPort(void)
{
	return static_cast<const uint16>(data->commoditiesServerPort);
}

//-----------------------------------------------------------------------

inline const char * ConfigServerDatabase::getDSN(void)
{
	return data->DSN;
}

//-----------------------------------------------------------------------

inline const char * ConfigServerDatabase::getAlternateDSN(void)
{
	return data->alternateDSN;
}

//-----------------------------------------------------------------------

inline const char * ConfigServerDatabase::getDatabaseUID(void)
{
	return data->databaseUID;
}

//-----------------------------------------------------------------------

inline const char * ConfigServerDatabase::getDatabasePWD(void)
{
	return data->databasePWD;
}

//-----------------------------------------------------------------------

inline const char * ConfigServerDatabase::getDatabaseProtocol(void)
{
	return data->databaseProtocol;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigServerDatabase::getTaskManagerPort(void)
{
    return static_cast<const uint16>(data->taskManagerPort);
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getExpectedDBVersion(void)
{
	return data->expectedDBVersion;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::isCorrectDBVersionRequired(void)
{
	return data->correctDBVersionRequired;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getSaveFrequencyLimit(void)
{
	return data->saveFrequencyLimit;
}

// ----------------------------------------------------------------------

inline const char *ConfigServerDatabase::getSchemaOwner(void)
{
	return data->schemaOwner;
}

// ----------------------------------------------------------------------

inline const char *ConfigServerDatabase::getGoldSchemaOwner(void)
{
	return data->goldSchemaOwner;
}

// ----------------------------------------------------------------------

inline const float ConfigServerDatabase::getUniqueMessageCacheTimeSec(void)
{
	return data->uniqueMessageCacheTimeSec;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getLoaderThreads(void)
{
	return data->loaderThreads;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getPersisterThreads(void)
{
	return data->persisterThreads;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getCharacterImmediateDeleteMinutes(void)
{
	return data->characterImmediateDeleteMinutes;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getLogObjectLoading(void)
{
	return data->logObjectLoading;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getEnableQueryProfile(void)
{
	return data->enableQueryProfile;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getVerboseQueryMode(void)
{
	return data->verboseQueryMode;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getLogWorkerThreads(void)
{
	return data->logWorkerThreads;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerDatabase::getGameServiceBindInterface(void)
{
	return data->gameServiceBindInterface;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getReportSaveTimes(void)
{
	return data->reportSaveTimes;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getEnableLoadLocks(void)
{
	return data->enableLoadLocks;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getNewCharacterThreads(void)
{
	return data->newCharacterThreads;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getDatabaseReconnectTime(void)
{
	return data->databaseReconnectTime;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getLogChunkLoading(void)
{
	return data->logChunkLoading;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getUseMemoryManagerForOCI(void)
{
	return data->useMemoryManagerForOCI;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getMaxCharactersPerLoadRequest(void)
{
	return data->maxCharactersPerLoadRequest;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getMaxChunksPerLoadRequest(void)
{
	return data->maxChunksPerLoadRequest;
}

// ----------------------------------------------------------------------

inline const float ConfigServerDatabase::getMaxLoadStartDelay(void)
{
	return data->maxLoadStartDelay;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getMaxErrorCountBeforeDisconnect(void)
{
	return data->maxErrorCountBeforeDisconnect;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getMaxErrorCountBeforeBailout(void)
{
	return data->maxErrorCountBeforeBailout;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getErrorSleepTime(void)
{
	return data->errorSleepTime;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getDisconnectSleepTime(void)
{
	return data->disconnectSleepTime;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getSaveAtModulus(void)
{
	return data->saveAtModulus;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getSaveAtDivisor(void)
{
	return data->saveAtDivisor;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getBackloggedQueueSize(void)
{
	return data->backloggedQueueSize;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getBackloggedRecoveryQueueSize(void)
{
	return data->backloggedRecoveryQueueSize;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getMaxTimewarp(void)
{
	return data->maxTimewarp;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getEnableObjvarPacking(void)
{
	return data->enableObjvarPacking;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getPrefetchNumRows(void)
{
	return data->prefetchNumRows;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getPrefetchMemory(void)
{
	return data->prefetchMemory;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getDefaultFetchBatchSize(void)
{
	return data->defaultFetchBatchSize;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getQueryReportingRate(void)
{
	return data->queryReportingRate;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getEnableDatabaseErrorLogging(void)
{
	return data->enableDatabaseErrorLogging;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getDefaultMessageBulkBindSize(void)
{
	return data->defaultMessageBulkBindSize;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getEnableGoldDatabase(void)
{
	return data->enableGoldDatabase;
}

// ----------------------------------------------------------------------

inline const float ConfigServerDatabase::getDefaultQueueUpdateTimeLimit(void)
{
	return data->defaultQueueUpdateTimeLimit;
}

// ----------------------------------------------------------------------


inline const bool ConfigServerDatabase::getEnableDataCleanup(void)
{
	return data->enableDataCleanup;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getDefaultLazyDeleteBulkBindSize(void)
{
	return data->defaultLazyDeleteBulkBindSize;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getDefaultLazyDeleteSleepTime(void)
{
	return data->defaultLazyDeleteSleepTime;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getWriteDelay(void)
{
	return data->writeDelay;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getDelayUnloadIfObjectStillHasData(void)
{
	return data->delayUnloadIfObjectStillHasData;
}

// ----------------------------------------------------------------------

inline const float ConfigServerDatabase::getExperienceConsolidationTime(void)
{
	return data->experienceConsolidationTime;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getMaxLoaderFinishedTasks(void)
{
	return data->maxLoaderFinishedTasks;
}

// ----------------------------------------------------------------------

inline const float ConfigServerDatabase::getReportLongFrameTime(void)
{
	return data->reportLongFrameTime;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getEnableVerboseMessageLogging(void)
{
	return data->enableVerboseMessageLogging;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getProfilerExpandAll(void)
{
	return data->profilerExpandAll;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getProfilerDisplayPercentageMinimum(void)
{
	return data->profilerDisplayPercentageMinimum;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerDatabase::getFatalOnDataError(void)
{
	return data->fatalOnDataError;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getMaxUnackedLoadCount(void)
{
	return data->maxUnackedLoadCount;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getMaxUnackedLoadCountPerServer(void)
{
	return data->maxUnackedLoadCountPerServer;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getAuctionLocationLoadBatchSize(void)
{
	return data->auctionLocationLoadBatchSize;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getAuctionLoadBatchSize(void)
{
	return data->auctionLoadBatchSize;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getAuctionAttributeLoadBatchSize(void)
{
	return data->auctionAttributeLoadBatchSize;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getAuctionBidLoadBatchSize(void)
{
	return data->auctionBidLoadBatchSize;
}

// ----------------------------------------------------------------------

inline const int ConfigServerDatabase::getOldestUnackedLoadAlertThresholdSeconds(void)
{
	return data->oldestUnackedLoadAlertThresholdSeconds;
}

// ======================================================================

#endif
