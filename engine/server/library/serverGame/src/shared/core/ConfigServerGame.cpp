//
// ConfigServerGame.cpp
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
#include "serverGame/FirstServerGame.h"
#include "serverGame/ConfigServerGame.h"
#include "serverUtility/ConfigServerUtility.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"

#include <algorithm>

//-------------------------------------------------------------------

ConfigServerGame::Data* ConfigServerGame::data;
Vector ConfigServerGame::startingPosition;
std::map<std::string, std::string> ConfigServerGame::m_components;

//-------------------------------------------------------------------

static ConfigServerGame::Data staticData;

//-------------------------------------------------------------------

#define KEY_INT(a,b)    (data->a = ConfigFile::getKeyInt("GameServer", #a, b))
#define KEY_BOOL(a,b)   (data->a = ConfigFile::getKeyBool("GameServer", #a, b))
#define KEY_FLOAT(a,b)  (data->a = ConfigFile::getKeyFloat("GameServer", #a, b))
#define KEY_STRING(a,b) (data->a = ConfigFile::getKeyString("GameServer", #a, b))
#define KEY_NETWORKID(a,b) (data->a = NetworkId(std::string(ConfigFile::getKeyString("GameServer", #a, b))))

//-------------------------------------------------------------------

void ConfigServerGame::install(void)
{
	ConfigServerUtility::install();
	data = &staticData;

	KEY_BOOL    (allowDefaultTemplateParams, true);
	KEY_BOOL    (skipTutorial, false);
	KEY_STRING	(groundScene, 0);
	KEY_STRING	(spaceScene, 0);
	KEY_INT     (port, 0);
	KEY_STRING  (sceneID, "default");
	KEY_INT     (preloadNumber, 0);
	KEY_FLOAT   (startX, 0.0f);
	KEY_FLOAT   (startY, 0.0f);
	KEY_FLOAT   (startZ, 0.0f);
	startingPosition = Vector(data->startX,data->startY,data->startZ);
	KEY_INT		(taskManagerPort, 60001);	
	KEY_STRING	(centralServerAddress, "localhost");
	KEY_INT		(centralServerPort, 44451);
	KEY_STRING  (scriptPath, "../../data/sku.0/sys.server/compiled/game");
#if defined(WIN32)
	KEY_STRING  (javaLibPath, "jvm.dll");
#elif defined(linux)
	// either the simple library .so name or the full path to the jvm library
	KEY_STRING  (javaLibPath, "libjvm.so");
#else
#error unsupported platform
#endif
	KEY_STRING  (javaDebugPort, "8000");
	KEY_BOOL    (useRemoteDebugJava, false);
	KEY_BOOL    (profileScripts, false);
	KEY_BOOL    (crashOnScriptError, false);
	KEY_BOOL    (compileScripts, false);
	KEY_BOOL    (trapScriptCrashes, false); //this seems to horrifyingly crash java 7/8 sometimes if not always
	KEY_INT     (scriptWatcherWarnTime, 5000);
	KEY_INT     (scriptWatcherInterruptTime, 5000);
	KEY_INT     (scriptStackErrorLimit, 35);
	KEY_INT     (scriptStackErrorLevel, 0);
	KEY_BOOL    (disableObjvarNullCheck, false);
	KEY_INT     (maxGameServerToSendUniverseData, 1000000);
	KEY_INT     (timeoutToAckUniverseDataReceived, 0);
	KEY_BOOL    (disableCombat, false);
	KEY_STRING  (combatConfigFile, "combat.cfg");
	KEY_FLOAT   (combatDamageDelaySeconds,  0.0);
	KEY_INT     (triggerVolumeSystem, 0);
	KEY_INT     (defaultAutoExpireTargetDuration, 6);
	KEY_FLOAT   (interiorTargetDurationFactor, 1.0);
	KEY_INT     (minPoolValue,  -100);
	KEY_INT     (minFaucetValue, 1);
	KEY_INT     (minDrainValue,  1);
	KEY_INT     (minEffectiveFaucetValue, 150);
	KEY_FLOAT   (defaultHealthRegen, 40.0f);
	KEY_FLOAT   (defaultActionRegen, 20.0f);
	KEY_FLOAT   (defaultMindRegen, 10.0f);
	KEY_FLOAT   (regenThreshold, 1.0f);
	KEY_FLOAT   (minForcePowerRegenRate, 1.0f);
	KEY_BOOL    (enableAttribModWarnings, true);
	KEY_BOOL    (checkAttribLimits, false);
	KEY_INT     (maxItemAttribBonus, 50);
	KEY_INT     (maxTotalAttribBonus, 500);
	KEY_BOOL    (enableNewJedi, false);
	KEY_STRING  (jediManagerTemplate, "object/jedi_manager/jedi_manager.iff");
	KEY_INT     (maxJediBounties, 5);
	KEY_INT     (startForcePower, 10);
	KEY_BOOL    (logJediConcludes, false);
	KEY_BOOL    (forceJediConcludes, false);
	KEY_INT     (jediUpdateLocationTimeSeconds, 60 * 5);
	KEY_STRING  (planetObjectTemplate, "object/planet/planet.iff");
	KEY_BOOL    (javaConsoleDebugMessages, false);
	KEY_BOOL    (javaUseXcheck, false);	 
	KEY_BOOL    (useVerboseJava, false);
	KEY_BOOL    (logJavaGc, false);
	KEY_INT     (javaLocalRefLimit, 16);
	KEY_STRING  (reservedNameFile, "datatables/name/reserved.iff");
	KEY_INT     (reservedObjectIds, 10000);
	KEY_INT     (minObjectIds, data->reservedObjectIds/2); // default is 1/2 of whatever is specifed for reservedObjectIds, so that the user can specify reservedObjectIds but doesn't have to specify minObjectIds
	KEY_STRING  (playerObjectTemplate, "object/player/player.iff");
	KEY_INT     (clientOutOfRangeObjectCacheTimeMs, 20000);
	KEY_INT     (playerInterestRange, 300);
	KEY_INT     (pvpUpdateTimeMs, 10000);
	KEY_INT     (pvpTempFlagExpireTimeMs, 5*60*1000/2); // 2.5 minutes
	KEY_INT     (pvpGuildWarCoolDownPeriodTimeMs, 1*60*1000); // 1 minute
	KEY_INT     (pvpGuildWarExemptionExclusiveDelaySeconds, 5*60); // 5 minutes
	KEY_INT     (pvpBattlefieldEnemyFlagExpireTimeMs, 5*60*1000/2); // 2.5 minutes
	KEY_STRING  (pvpFactionOpponentsFilename, "datatables/pvp/faction_opponents.iff");
	KEY_STRING  (pvpNonaggressiveFactionsFilename, "datatables/pvp/faction_nonaggressive.iff");
	KEY_STRING  (pvpUnattackableFactionsFilename, "datatables/pvp/faction_unattackable.iff");
	KEY_STRING  (pvpBountyTargetFactionsFilename, "datatables/pvp/faction_bountytarget.iff");
	KEY_BOOL    (createAppearances,true);
	KEY_STRING  (adminAccountDataTable, "datatables/admin/us_admin.iff");
	KEY_BOOL    (adminGodToAll, false);
	KEY_INT     (adminGodToAllGodLevel, 50);
	KEY_BOOL    (useSecureLoginForGodAccess, false);
	KEY_BOOL    (useIPForGodAccess, false);
	KEY_BOOL    (adminPersistAllCreates, false);
	KEY_BOOL    (buildCluster, false);
	KEY_INT     (requestSceneWarpTimeoutSeconds, 60);
	KEY_BOOL    (moveValidationEnabled, false);
	KEY_FLOAT   (moveSpeedTolerance, 1.03f);
	KEY_INT     (moveSpeedCheckFrequencyMs, 10000);
	KEY_INT     (moveSpeedCheckFailureLeewaySeconds, 5);
	KEY_INT     (moveSpeedCheckFailureLeewayCount, 5);
	KEY_INT     (moveSlowdownFudgeTimeMs, 5000);
	KEY_BOOL    (moveCheckDestinationCollision, true);
	KEY_FLOAT   (moveMaxDistance, 50.0f);

	// should be kept in sync with ConfigClientGame's disableMovementInventoryOverload
	KEY_INT     (moveValidationMaxInventoryOverload, 121);

	KEY_BOOL    (moveValidationCheckForPastTimestamp, true);
	KEY_BOOL    (moveValidationCheckForFutureTimestamp, true);
	KEY_BOOL    (moveValidationCheckForTimestampDrift, true);
	KEY_INT     (moveValidationPastTimestampThresholdMs, 10000);
	KEY_INT     (moveValidationFutureTimestampThresholdMs, 10000);
	KEY_INT     (moveValidationTimestampDriftThresholdMs, 10000);
	KEY_FLOAT   (moveMaxDistanceLeewayPerUpdate, 2.0f);
	KEY_BOOL    (nameValidationAcceptAll, false);
	KEY_FLOAT   (tangiblePvpRegionCheckTime, 20.0f);
	KEY_FLOAT   (creaturePvpRegionCheckTime, 5.0f);
	KEY_STRING  (defaultLightArmorTemplate, "abstract/armor_statistics/armor/crafted/crafted_light_armor.iff");
	KEY_STRING  (defaultMediumArmorTemplate, "abstract/armor_statistics/armor/crafted/crafted_medium_armor.iff");
	KEY_STRING  (defaultHeavyArmorTemplate, "abstract/armor_statistics/armor/crafted/crafted_heavy_armor.iff");
	KEY_FLOAT   (armorDamageReduction, 80.0f);
	KEY_BOOL    (stationPlayersEnabled, false);
	KEY_BOOL    (stationPlayersShowAllDebugInfo, false);
	KEY_BOOL    (commoditiesMarketEnabled, true);
	KEY_INT     (commoditiesServerServiceBindPort, 4069);
	KEY_STRING  (commoditiesServerServiceBindInterface, "localhost");
	KEY_BOOL    (commoditiesShowAllDebugInfo, false);
	KEY_INT     (commoditiesQueryTimeoutSec, 60);
	KEY_INT     (commoditiesServerReconnectIntervalSec, 30);
	KEY_INT     (unclaimedAuctionItemDestroyTimeSec, 30*24*60*60);
	KEY_INT     (maxReimburseAmount, 0);
	KEY_BOOL    (logBaselines, false);
	KEY_STRING  (defaultVendorItemRestrictionFile, "datatables/commodity/vendor_item_restriction_default.iff");
	KEY_STRING  (defaultVendorItemRestrictionRejectionMessage, "@ui_auc:reject_restricted_item");
	KEY_INT     (weatherUpdateSeconds, 15*60);
	KEY_STRING  (instrumentDataTableFilename, "datatables/tangible/instrument_datatable.iff");
	KEY_STRING  (performanceDataTableFilename, "datatables/performance/performance.iff");
	KEY_INT     (createQueueScheduleTime, 75);
	KEY_BOOL    (disableResources, false);
	KEY_INT     (xpMultiplier, 1);
	KEY_STRING  (xpManagerTemplate, "object/xp_manager/xp_manager.iff");
	KEY_FLOAT   (gcwXpBonus, 15.0f);
	KEY_BOOL    (reportAiStateChanges, false);
	KEY_BOOL    (reportAiWarnings, false);
	KEY_BOOL    (sendBreadcrumbs, false);
	KEY_BOOL    (movementWhileRetreatingThrowsException, false);
	KEY_BOOL    (hibernateEnabled, true);
	KEY_BOOL    (hibernateProxies, true);
	KEY_FLOAT   (hibernateDistance, 128.0f);
	KEY_FLOAT   (minHibernateAlter, 4.0f);
	KEY_FLOAT   (maxHibernateAlter, 6.0f);
	KEY_INT     (aiPulseQueuePerFrame, 100);
	KEY_INT     (aiPulseQueueMaxWaitTimeMs, 1000);
	KEY_FLOAT   (aiBaseAggroRadius, 24.0f);
	KEY_FLOAT   (aiMaxAggroRadius, 96.0f);
	KEY_FLOAT   (aiLeashRadius, 256.0f);
	KEY_FLOAT   (aiAssistRadius, 12.0f);
	KEY_INT     (minimumResourcePoolDrainRate, 0);
	KEY_INT     (secondsPerResourceTick, 60);
	KEY_INT     (resourceTimeScale, 60 * 60 * 24);
	KEY_BOOL    (disableCreateQueue, false);
	KEY_STRING  (craftingComponentTableFile0, "datatables/crafting/component.iff");
	KEY_STRING  (craftingComponentTableFile1, "datatables/crafting/muzzle.iff");
	KEY_STRING  (craftingComponentTableFile2, "datatables/crafting/scope.iff");
	KEY_STRING  (craftingComponentTableFile3, "datatables/crafting/stock.iff");
	m_components[std::string("component")] = data->craftingComponentTableFile0;
	m_components[std::string("muzzle")] = data->craftingComponentTableFile1;
	m_components[std::string("scope")] = data->craftingComponentTableFile2;
	m_components[std::string("stock")] = data->craftingComponentTableFile3;
	KEY_BOOL    (craftingComponentStrict, true);
	KEY_INT     (craftingXpChance, 50);
	KEY_STRING  (craftingXpPercentTable, "datatables/crafting/partial_xp_percent.iff");
	KEY_INT     (maxObjectSkillModBonus, 25);
	KEY_INT     (maxSocketSkillModBonus, 25);
	KEY_INT     (maxSkillModSockets, 1);
	KEY_FLOAT   (baseDecayRate, 3600.0f);
	KEY_FLOAT   (weaponDecayThreshold, 25.0f);
	KEY_STRING  (regionFilesName, "datatables/region/planets.iff");
	KEY_INT     (regionTreeDepth, 2);
	KEY_STRING  (battlefieldMarkerName, "datatables/battlefield/marker.iff");
	KEY_FLOAT   (maxTeleportDistanceWithoutSceneWarp, 200.0f);
	KEY_FLOAT   (maxInstrumentPlayDistance, 3.0f);
	KEY_STRING  (characterNameGeneratorDirectory,"name");
	//KEY_FLOAT   (statMigrationTimer, 60.0f * 10.0f); //value in seconds (currently defaults to 10 minutes)
	KEY_INT     (maxLotsPerAccount, 165);
	KEY_INT     (unsafeLogoutTimeMs, 3*60*1000);
	KEY_INT     (idleLogoutTimeSec, 30*60);
	KEY_INT     (idleLogoutTimeAfterCharacterTransferSec, 1*60);
	KEY_STRING  (fallbackDefaultWeapon, "object/weapon/melee/unarmed/unarmed_default.iff");
	KEY_BOOL    (logObservers, false);
	KEY_INT     (lastKnownLocationsResetTimeSec, 60 * 60); // 1 hour
	KEY_INT     (maxMessageTosPerObjectPerFrame, 0); // unlimited
	KEY_INT     (maxMessageToTimePerObjectPerFrame, 0); // unlimited
	KEY_INT     (maxMessageToBounces, 100); // should be large enough to cover the worse case legitimate scenario of having to bounce through every game server
	KEY_INT     (missionRequestsPerFrame, 2);
	KEY_BOOL    (disableMissions, false);
	KEY_INT     (loopProfileLogThresholdMs, 3000);
	KEY_INT     (groundLoopProfileLogThresholdMs, 3000);
	KEY_INT     (spaceLoopProfileLogThresholdMs, 3000);
	KEY_BOOL    (pvpDisableCombat, false);
	KEY_BOOL    (logObjectLoading, false);
	KEY_BOOL    (enforcePlayerTriggerVolumesOnly, true);
	KEY_FLOAT   (locationTargetCheckIntervalSec, 5.0f);
	KEY_STRING  (gameServiceBindInterface, "");
	KEY_BOOL    (logPvpUpdates, false);
	KEY_FLOAT   (npcConversationDistance, 6.0f);
	KEY_INT     (serverSpawnLimit, 1200);
	KEY_INT     (spawnQueueSize, 1);
	KEY_INT     (framesPerSpawn, 0);
	KEY_BOOL    (enableSpawningNearLoadBeacons, true);
	KEY_BOOL    (disableTravel, false);
	KEY_BOOL    (sendPlayerTransform, false);
	KEY_FLOAT   (watchedHarvesterAlterTime, 5.0f);
    KEY_FLOAT   (unwatchedHarvesterAlterTime, 900.0f);
	KEY_FLOAT	(harvesterExtractionRateMultiplier, 1.0f);
	KEY_INT     (nonCriticalTasksPerFrame, 5);
	KEY_INT     (maxMoney,         1000000000); // 1 billion (must be small enough to keep money within an int)
	KEY_INT     (maxGalacticReserveDepositBillion, 3); // 3 billion
	KEY_INT     (maxMoneyTransfer,  100000000); // 100 million
	KEY_INT     (maxFreeTrialMoney, 50000); //50k credits limited to demo customers
	KEY_BOOL    (enablePreload, false);
	KEY_BOOL    (buildPreloadLists, false);
	KEY_BOOL    (logAuthTransfer, false);

	KEY_INT     (overrideUpdateRadius, 0);
	KEY_FLOAT   (buildingUpdateRadiusMultiplier, 1.0f);
	KEY_INT     (maxPopulationForNewbieTravel, 90);
	KEY_BOOL    (debugAllAreasOverpopulated, false);
	KEY_INT     (minNewbieTravelLocations, 5);
	KEY_INT     (numberOfMoveObjectLists, 0);
	KEY_INT     (sitOnObjectReportThreshold, 1000);
	KEY_BOOL    (fatalOnSitThreshold, false);
	KEY_INT     (databasePositionUpdateLongDelayIntervalMs, 3*60*1000);
	KEY_INT     (databasePositionUpdateShortDelayIntervalMs, 1*1000);
	KEY_BOOL    (logPositionUpdates, false);

	KEY_BOOL    (checkOriginCreates, true);
	KEY_BOOL    (enableWho, false);
	KEY_BOOL    (debugGroupChat, false);

	KEY_INT     (numberOfMissionsWantedInMissionBag, 10);	// MUST be 10 (at the moment)
	KEY_INT     (missionCreationQueueSize, 4);
	KEY_INT     (connectToAllGameServersTimeout, 120);
	KEY_INT     (houseItemLimitMultiplier, 100);
	KEY_INT     (maxHouseItemLimit, 500);
	KEY_BOOL    (demandLoadHouseContents, true);
	KEY_INT     (unloadHouseContentsTime, 10 * 60);
	KEY_INT     (loadHouseContentsDelaySeconds, 0);
	KEY_BOOL    (boundarySpawningAllowed, true);
	KEY_BOOL    (debugMovement, false);
	KEY_BOOL    (spawnAllResources, true);
	KEY_BOOL    (skipUnreliableTransformsForOtherCells, false);
	KEY_INT     (maxPlayerSanityCheckFailures, 3);
	KEY_BOOL    (disablePlayerSanityChecker, false);
	KEY_BOOL    (deactivateHarvesterIfDamaged, true);
	KEY_FLOAT   (updateRange7Packets,  40.0f);
	KEY_FLOAT   (updateRange6Packets,  80.0f);
	KEY_FLOAT   (updateRange5Packets,  90.0f);
	KEY_FLOAT   (updateRange4Packets, 100.0f);
	KEY_FLOAT   (updateRange3Packets, 110.0f);
	KEY_FLOAT   (updateRange2Packets, 120.0f);
	KEY_FLOAT   (updateRange1Packets, 130.0f);
	KEY_STRING  (instantDeleteListDataTableName, "datatables/database/instant_delete_list.iff");
	KEY_BOOL    (sendFrameProfilerInfo, false);
	KEY_BOOL    (logAllCommands, true);
	KEY_BOOL    (enableDebugControllerMessageSpam ,false);
	KEY_BOOL    (debugPvp, false);
	KEY_BOOL    (allowMasterObjectCreation, true);
	KEY_BOOL    (mountsEnabled, true);
	KEY_BOOL    (mountsSanityCheckerEnabled, false);
	KEY_BOOL    (mountsSanityCheckerBroadcastEnabled, false);
	KEY_INT     (mountsSanityCheckerBroadcastLimit, 5);
	KEY_INT     (universeCheckFrequencySeconds, 60 * 60);
	KEY_BOOL    (moveSimEnabled, false);
	KEY_BOOL    (exceptionOnObjVarZeroLengthArray, false);
	KEY_BOOL    (trackNonAuthoritativeObjvarSets, false);
	KEY_FLOAT   (triggerVolumeWarpDistance, 128.0f);
	KEY_NETWORKID(maxGoldNetworkId, "0");
	KEY_BOOL    (fatalOnMovingGoldObject, false);
	KEY_INT     (spatialChatLogMinutes, 5);
	KEY_FLOAT   (behaviorMaxAvoidanceStuckTime, 10.0f);
	KEY_INT     (behaviorMaxAvoidancePersistenceFrameCount, 4);
	KEY_BOOL    (shipsEnabled, true);
	KEY_BOOL    (logoutTriggerEnabled, true);
	KEY_BOOL    (loginAsBountyHunter, false);
	KEY_INT     (veteranRewardTradeInWaitPeriodSeconds, 30 * 24 * 60 * 60); // 30 days
	KEY_BOOL    (enableVeteranRewards, true);
	KEY_BOOL    (enableOneYearAnniversary, true);
	KEY_INT     (minEntitledTime, -1);
	KEY_BOOL    (veteranDebugTriggerAll, false);
	KEY_BOOL    (veteranDebugEnableOverrideAccountAge, false);
	KEY_BOOL    (enableNewVeteranRewards, true);
	KEY_FLOAT   (buddyPointTimeBonus, 30.0f/4.0f); // 4 buddy points = 30 days

#ifdef _DEBUG
	KEY_FLOAT   (manufactureTimeOverride, 0.0f);
#endif

	KEY_INT     (theaterCreationLimitMilliseconds, 25);
	KEY_BOOL    (fatalOnGoldPobChange, false);
	KEY_INT     (lineOfSightCacheDurationMs, 1500);
	KEY_FLOAT   (lineOfSightCacheMinHeight, 0.8f);
	KEY_FLOAT   (lineOfSightLocationRoundValue, 5.0f);

#ifdef _DEBUG
	// in debug mode, we default this to true so that unless
	// a controller message is explicitly marked as being allowed
	// from the client, it will be dropped when the game server
	// receives it; this will have the effect of forcing the
	// programmer to make a conscious decision to mark the
	// controller message as allowable from the client; because
	// controller message coming from the client is not secured,
	// the mechanism to prevent a hacked client from sending the
	// game server any controller message is to explicity "mark"
	// those controller messages that are allowed from the client
	KEY_BOOL    (enableClientControllerMessageCheck, true);
#else
	// for live, the this option will be set to true in the config file
	KEY_BOOL    (enableClientControllerMessageCheck, false);
#endif

	KEY_INT     (maxWaypointsPerCharacter,100); // Keep this in sync with the maxWaypoints value in ConfigClientGame.cpp
	KEY_FLOAT   (maxSmallCreatureHeight, 0.7f);
	KEY_FLOAT   (smallCreatureUpdateRadius, 64.0f);
	KEY_FLOAT   (maxMediumCreatureHeight, 3.0f);
	KEY_FLOAT   (mediumCreatureUpdateRadius, 128.0f);
	KEY_FLOAT   (largeCreatureUpdateRadius, 256.0f);
	KEY_INT     (coolDownPersistThresholdSeconds, 1 * 60);
	KEY_BOOL    (checkNotifyRegions, true);
	KEY_BOOL    (allowRegionTriggerOverride, true);
	KEY_INT     (spacePlanetServerUpdatesPerFrame, 0);
	KEY_FLOAT   (hyperspaceRandomOffsetRange, 200);
	
	if (data->theaterCreationLimitMilliseconds <= 0)
	{
		data->theaterCreationLimitMilliseconds = 25;
		WARNING(true, ("Config value [GameServer] theaterCreationLimitMilliseconds "
			"out of range, setting to default value"));
	}

	KEY_BOOL    (sendAsteroidExtents, false);
	KEY_BOOL    (createZoneObjects, true);
	KEY_INT     (shipUpdatesPerSecond, 32);
	KEY_BOOL    (debugSpaceVisibilityManager, false);
	KEY_INT     (groundFrameRateLimit, 5);
	KEY_INT     (spaceFrameRateLimit, 12);
	KEY_BOOL    (aiLoggingEnabled, false);
	KEY_BOOL    (shipShotValidationEnabled, false);
	KEY_INT     (spaceAiEnemySearchesPerFrame, 15);
	KEY_BOOL    (groundShouldSleep, true);
	KEY_BOOL    (spaceShouldSleep, true);
	KEY_BOOL    (shipMoveValidationEnabled, false);
	KEY_INT     (shipMoveCheckIntervalMs, 1000);
	KEY_INT     (shipMoveCheckFudgeTimeMs, 5000);
	KEY_FLOAT   (shipMoveCheckTolerance, 1.03f);
	KEY_FLOAT   (shipMoveCheckTimeToleranceSeconds, 0.1f);
	KEY_INT     (spaceAiAvoidanceChecksPerFrame, 64);
	KEY_BOOL    (spaceAiUsePathRefinement, true);
	KEY_BOOL    (aiClientDebugEnabled, false);
	KEY_BOOL    (shipClientUpdateDebugEnabled, false);
	KEY_FLOAT   (shipLoadInvulnerableTimeWithoutClient, 120.f);
	KEY_FLOAT   (shipLoadInvulnerableTimeWithClient, 10.f);
	KEY_FLOAT   (creatureLoadInvulnerableTimeWithoutClient, 120.f);
	KEY_FLOAT   (creatureLoadInvulnerableTimeWithClient, 1.f);
	KEY_BOOL    (buildoutAreaEditingEnabled, false);
	KEY_BOOL    (debugFloorPathNodeCount, false); // this flag checks cells to make sure the floor part count equals the path graph node count
	KEY_INT     (populationReportTimeSeconds, 600);
	KEY_FLOAT(maxCombatRange, 96.0f);
	KEY_BOOL    (blockBuildRegionPlacement,false);

	KEY_INT     (npeMinInstancePopulation,20);
	KEY_INT     (npeMaxInstancePopulation,50);
	KEY_BOOL    (stripNonFreeAssetsFromPlayersInTutorial, false);

	// weekly time (in GMT) when GCW rating recalculation occurs
	KEY_INT     (gcwRecalcTimeDayOfWeek, 4); // Sunday=0, Monday=1, Tuesday=2, Wednesday=3, Thursday=4, Friday=5, Saturday=6
	KEY_INT     (gcwRecalcTimeHour, 19);     // 0 - 23
	KEY_INT     (gcwRecalcTimeMinute, 0);    // 0 - 59
	KEY_INT     (gcwRecalcTimeSecond, 0);    // 0 - 59

	// weekly time (in GMT) when GCW score decay occurs;
	// additional time(s) can be specified in the config file if
	// you want the GCW score decay to occur more than once a week;
	// the following time is ignored if any time(s) is specified
	// in the config file; if no time(s) is specified in the
	// config file, it will use the following time as the default
	KEY_INT     (gcwScoreDecayTimeDayOfWeek, 1); // Sunday=0, Monday=1, Tuesday=2, Wednesday=3, Thursday=4, Friday=5, Saturday=6
	KEY_INT     (gcwScoreDecayTimeHour, 19);     // 0 - 23
	KEY_INT     (gcwScoreDecayTimeMinute, 0);    // 0 - 59
	KEY_INT     (gcwScoreDecayTimeSecond, 0);    // 0 - 59

	KEY_INT     (chatStatisticsReportIntervalSeconds, 60);
	KEY_INT     (chatSpamLimiterNumCharacters, 400);
	KEY_INT     (chatSpamLimiterIntervalMinutes, 15);
	KEY_BOOL    (chatSpamLimiterEnabledForFreeTrial, true);
	KEY_INT     (chatSpamNotifyPlayerWhenLimitedIntervalSeconds, 30); // <= 0 to disable

	KEY_INT     (pendingLoadRequestLimit, 1000000000); // unlimited

	KEY_BOOL    (enableCityCitizenshipFixup, true);

	KEY_INT     (cityCitizenshipInactivePackupStartTimeEpoch, 1243839660); // June 1st 2009 12:01AM PDT
	KEY_INT     (cityCitizenshipInactivePackupInactiveTimeSeconds, 90 * 24 * 60 * 60); // 90 days

	KEY_STRING  (retroactivePlayerCityCreationTimeFilename, "");

	KEY_INT     (wardenSquelchDurationSeconds, 6 * 60 * 60); // 6 hours
	KEY_BOOL    (enableWarden, true);
	KEY_BOOL    (enableWardenCanSquelchSecuredLogin, false);
	KEY_BOOL    (enableWardenToDeputizeWarden, true);

	KEY_BOOL    (disableCollectionServerFirstGrantForSecuredLogin, false);

	KEY_INT     (locateStructureCommandIntervalSeconds, 24 * 60 * 60); // 24 hours

	KEY_INT     (locateVendorCommandIntervalSeconds, 15 * 60); // 15 minutes

	KEY_INT     (ctsDefaultScriptDictionarySizeBytes, 10 * 1024 * 1024); // 10M
	KEY_STRING  (retroactiveCtsHistoryFilename, "");
	KEY_BOOL    (allowIgnoreFreeCtsTimeRestriction, false);

	KEY_INT     (groupPickupPointApprovalRangeSquared, 20 * 20);
	KEY_INT     (groupPickupPointTimeLimitSeconds, 2 * 60);
	KEY_INT     (groupPickupTravelPlayerCityPercent, 10);

	KEY_BOOL    (groupInviteInviterCombatRestriction, false);
	KEY_BOOL    (groupInviteInviteeCombatRestriction, false);
	KEY_BOOL    (groupJoinInviterCombatRestriction, false);
	KEY_BOOL    (groupJoinInviteeCombatRestriction, false);

	KEY_INT     (characterMatchMaxMatchCount, 32);

	KEY_BOOL    (enableOccupyUnlockedSlotCommand, true);
	KEY_BOOL    (enableVacateUnlockedSlotCommand, true);
	KEY_BOOL    (enableSwapUnlockedSlotCommand, true);
	KEY_INT     (occupyVacateUnlockedSlotCommandCooldownSeconds, 24 * 60 * 60); // 24 hours

	KEY_INT     (accountFeatureIdForMonthlyBoosterPack, 5032);
	KEY_INT     (accountFeatureIdForTcgBetaAnnouncement, 5552);

	KEY_BOOL    (logEventObjectCreation, false);
	KEY_BOOL    (logEventObjectDestruction, false);
	KEY_STRING  (housePackupAccountListDataTableName, "datatables/house_packup/house_packup_accounts.iff");

	KEY_INT     (pickDropAllRoomItemsNumberOfItemsPerRound, 5);

	KEY_BOOL    (enableCovertImperialMercenary, false);
	KEY_BOOL    (enableOvertImperialMercenary, false);
	KEY_BOOL    (enableCovertRebelMercenary, false);
	KEY_BOOL    (enableOvertRebelMercenary, false);
	KEY_INT     (broadcastGcwScoreToOtherGalaxiesIntervalSeconds, 15 * 60); // 15 minutes
	KEY_BOOL    (broadcastGcwScoreToOtherGalaxies, false);
	KEY_BOOL    (receiveGcwScoreFromOtherGalaxies, false);
	KEY_INT     (trackGcwPercentileHistoryCount, 10);
	KEY_BOOL    (gcwFactionalPresenceDisableForSecuredLogin, false);

	KEY_INT     (gcwFactionalPresenceMountedPct, 20); // 0 to disable FP while on a vehicle/mount, 100 for no FP penalty while on a vehicle/mount
	// clamp to 0 - 100
	data->gcwFactionalPresenceMountedPct = std::min(std::max(data->gcwFactionalPresenceMountedPct, 0), 100);

	KEY_INT     (gcwFactionalPresenceLevelPct, 10);
	KEY_INT     (gcwFactionalPresenceGcwRankBonusPct, 10);
	KEY_INT     (gcwFactionalPresenceAlignedCityBonusPct, 100);
	KEY_INT     (gcwFactionalPresenceAlignedCityRankBonusPct, 10);
	KEY_INT     (gcwFactionalPresenceAlignedCityAgeBonusPct, 5);
	KEY_INT     (gcwGuildMinMembersForGcwRegionDefender, 10);
	KEY_INT     (gcwRegionDefenderTotalBonusPct, 20);
	KEY_INT     (gcwDaysRequiredForGcwRegionDefenderBonus, 3);

	KEY_FLOAT   (regionFlagUpdateTime, 0.5f);

	if (data->baseDecayRate <= 1.0f)
		data->baseDecayRate = 1.0f;

	if (data->weaponDecayThreshold < 0.0f)
		data->weaponDecayThreshold = 0.0f;
	else if (data->weaponDecayThreshold > 100.0f)
		data->weaponDecayThreshold = 100.0f;
	data->weaponDecayThreshold /= 100.0f;
	
	if (data->armorDamageReduction < 0.0f)
		data->armorDamageReduction = 0.0f;
	else if (data->armorDamageReduction > 100.0f)
		data->armorDamageReduction = 100.0f;
	data->armorDamageReduction = 1.0f - data->armorDamageReduction / 100.0f;

	if (data->gcwXpBonus < 0.0f)
		data->gcwXpBonus = 0.0f;
	data->gcwXpBonus /= 100.0f;

	// GCW score decay time(s)
	data->gcwScoreDecayTime.clear();
	int index = 0;
	char const * dayOfWeek = nullptr;
	char const * hour = nullptr;
	char const * minute = nullptr;
	char const * second = nullptr;
	do
	{
		dayOfWeek = ConfigFile::getKeyString("GameServer", "gcwScoreDecayTimeDayOfWeek", index, 0);
		hour = ConfigFile::getKeyString("GameServer", "gcwScoreDecayTimeHour", index, 0);
		minute = ConfigFile::getKeyString("GameServer", "gcwScoreDecayTimeMinute", index, 0);
		second = ConfigFile::getKeyString("GameServer", "gcwScoreDecayTimeSecond", index, 0);

		if (dayOfWeek && hour && minute && second)
		{
			data->gcwScoreDecayTime.push_back(std::make_pair(std::make_pair(ConfigFile::getKeyInt("GameServer", "gcwScoreDecayTimeDayOfWeek", index, 0), ConfigFile::getKeyInt("GameServer", "gcwScoreDecayTimeHour", index, 0)), std::make_pair(ConfigFile::getKeyInt("GameServer", "gcwScoreDecayTimeMinute", index, 0), ConfigFile::getKeyInt("GameServer", "gcwScoreDecayTimeSecond", index, 0))));
		}

		++index;
	}
	while (dayOfWeek && hour && minute && second);

	if (data->gcwScoreDecayTime.empty())
	{
		data->gcwScoreDecayTime.push_back(std::make_pair(std::make_pair(data->gcwScoreDecayTimeDayOfWeek, data->gcwScoreDecayTimeHour), std::make_pair(data->gcwScoreDecayTimeMinute, data->gcwScoreDecayTimeSecond)));
	}
}

//-------------------------------------------------------------------

void ConfigServerGame::remove(void)
{
	ConfigServerUtility::remove();
}

//-------------------------------------------------------------------

const std::string & ConfigServerGame::getCraftingComponentTableFile(
	const std::string & name)
{
static const std::string NO_DATA;

	std::map<std::string, std::string>::const_iterator iter;
	iter = m_components.find(name);
	if (iter != m_components.end())
		return (*iter).second;
	if (!m_components.empty())
		return m_components["component"];
	return NO_DATA;
}

// ======================================================================
