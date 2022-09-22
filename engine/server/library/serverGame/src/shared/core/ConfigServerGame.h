//
// ConfigServerGame.h
//
// copyright 2000, verant interactive
//

#ifndef ConfigServerGame_H
#define ConfigServerGame_H

#include "sharedFoundation/NetworkId.h"
#include <vector>

class Vector;


//-------------------------------------------------------------------

class ConfigServerGame
{
  public:

	struct Data
	{
		bool            allowDefaultTemplateParams;

		const char *    groundScene;
		const char *    spaceScene;
		
		int             port;
		const char *    sceneID;
		int				preloadNumber;

		int             taskManagerPort;

		float           startX;
		float           startY;
		float           startZ;
		bool            skipTutorial;

		const char *    centralServerAddress;
		int             centralServerPort;

		bool            disableCombat;
		const char *    combatConfigFile;

		float		combatDamageDelaySeconds;
		int		triggerVolumeSystem;	            // 0 = SphereTree, 1 = test sphere tree vs. grid, 2 = sphere grid

		int		defaultAutoExpireTargetDuration;    // Seconds until creatures stop chasing targets
		float		interiorTargetDurationFactor;       // Scalar conversion to get the interior factor (0.5 = half of world val )

		// min values attributes are allowed to have
		int             minPoolValue;
		int             minFaucetValue;
		int             minDrainValue;
		int             minEffectiveFaucetValue;
		float           defaultHealthRegen;      // default health regeneration rate (pts/sec)
		float           defaultActionRegen;      // default action regeneration rate (pts/sec)
		float           defaultMindRegen;        // default mind regeneration rate (pts/sec)
		float           regenThreshold;          // time for regeneration to occur before we update the attribute
		float           minForcePowerRegenRate;  // the lowest regeneration rate for force power
		bool            enableAttribModWarnings; // flag to turn on warning spam if an attrib mod tries to permanently change a non-pool attribute
		bool            checkAttribLimits;       // flag to enable checking if attribs or wounds are bad values
		int             maxItemAttribBonus;      // max attrib bonus an item may have
		int             maxTotalAttribBonus;     // max attrib bonus a player may have

		// jedi data
		bool            enableNewJedi;           // flag to start tracking if players can become Jedi
		const char *    jediManagerTemplate;     // template used by the jedi manager
		int             maxJediBounties;         // max bouties a Jedi may have on him
		int             startForcePower;         // initial force power of a new Jedi
		bool            logJediConcludes;        // flag to log the Jedi manager concludes
		bool            forceJediConcludes;      // flag to force the Jedi manager to add itself to the conclude list
		int             jediUpdateLocationTimeSeconds; // how long we wait before updating a Jedi's location in the Jedi manager

		// script data
		const char *    scriptPath;                 // location of scripts
		const char *    javaLibPath;                // location of jvm files
		const char *    javaDebugPort;              // ip port we connect to for remote debugging
		bool            useRemoteDebugJava;         // flag to use the jvm for remote debugging
		bool            profileScripts;             // flag to enable script profiling
		bool            crashOnScriptError;         // flag to crash the gameserver on a script error
		bool            compileScripts;             // flag to compile the script .class files as they are being run
		bool            trapScriptCrashes;          // flag to use our fatal handler to trap script segfaults
		int             scriptWatcherWarnTime;      // time in ms to warn about a script running long
		int             scriptWatcherInterruptTime; // time in ms (after scriptWatcherWarnTime) before we abort a script
		int             scriptStackErrorLimit;      // depth of stack error we assume to be a legit error
		int             scriptStackErrorLevel;      // how we handle a stack error: 0=recover, 1=javacore, 2=fatal
		bool            disableObjvarNullCheck;     // flag to disable the check for a nullptr object when get/setting objvars

		// throttle to limit how universe data is sent from
		// the universe game server to the other game servers;
		// CentralServer server is the bottleneck and if universe
		// data is sent to too many game servers at the same
		// time, CentralServer must do massive amount of buffering
		// which can cause out-of-memory crashes in CentralServer
		int             maxGameServerToSendUniverseData;
		int             timeoutToAckUniverseDataReceived;

		// resource system
		const char *    planetObjectTemplate; // template to use to make PlanetObjects
		int             minimumResourcePoolDrainRate;
		int             secondsPerResourceTick;    // how long does it take to collect your effeciency's worth of resource units
		int             resourceTimeScale;         // for depletion times, how long (in seconds) 1 unit is
		float           watchedHarvesterAlterTime; // how often to alter a harvester if it has a shared interface
        float           unwatchedHarvesterAlterTime; //how often to alter a harvester when its not being observed(interface)
		bool			deactivateHarvesterIfDamaged; // Deactivate a harvester if damaged, instead of just scaling the amount harvested
	    float			harvesterExtractionRateMultiplier; //amount to multiply to adjust extraction rate, usefull for adding bonuses		
		int             universeCheckFrequencySeconds; // how often to scan the resource tree for things that need to be spawned

		bool            javaConsoleDebugMessages;
		bool		javaUseXcheck;
		bool            useVerboseJava;
		bool            logJavaGc;
		int             javaLocalRefLimit;

		// name generation
		const char *    reservedNameFile;
		const char *    characterNameGeneratorDirectory;

		// crafting data
		const char *    craftingComponentTableFile0;
		const char *    craftingComponentTableFile1;
		const char *    craftingComponentTableFile2;
		const char *    craftingComponentTableFile3;
		bool            craftingComponentStrict;
		int             craftingXpChance;
		const char *    craftingXpPercentTable;

		// skill mod data
		int             maxObjectSkillModBonus;     // max skill mod bonus an object can grant for a skill mod
		int             maxSocketSkillModBonus;     // max skill mod bonus that can be added to an object's skill mod socket
		int             maxSkillModSockets;         // max number of skill mod sockets an object can have
		
		// decay data
		float baseDecayRate;		// value in secs
		float weaponDecayThreshold; // % hp a weapon can be reduced to before it affects the weapon's min/max damage

		// region system
		const char * regionFilesName;   // data table that contains data tables that have static region info
		int          regionTreeDepth;   // max depth of region quad tree
		const char * battlefieldMarkerName; // data table that contains the templates used for battlefield markers

		// Object Id Manager -- see ObjectIdManager.h for explanation of these parameters
		int             reservedObjectIds;
		int             minObjectIds;

		// Player and Account system
		const char *    playerObjectTemplate; // template to use to create new PlayerObjects

		// time in which objects out of a client's update range continue to be cached for the client, in ms
		int             clientOutOfRangeObjectCacheTimeMs;
		// time between pvp enemy flag update cycles
		int             pvpUpdateTimeMs;
		// time before a temp flag expires for pvp
		int             pvpTempFlagExpireTimeMs;
		// time after cloning before someone can participate in guild war pvp again
		int             pvpGuildWarCoolDownPeriodTimeMs;
		// time to wait before actually becoming guild war pvp enabled/disabled though using the guild war exemption/exclusive list
		int             pvpGuildWarExemptionExclusiveDelaySeconds;
		// time after an action until someone may leave a battlefield
		int             pvpBattlefieldEnemyFlagExpireTimeMs;
		// name of pvp faction opponent data table file
		const char *    pvpFactionOpponentsFilename;
		// name of pvp nonaggressive factions data table file
		const char *    pvpNonaggressiveFactionsFilename;
		// name of pvp unattackable factions data table file
		const char *    pvpUnattackableFactionsFilename;
		// name of pvp bounty target factions data table file
		const char *    pvpBountyTargetFactionsFilename;
		// debugging and temporary settings
		int             playerInterestRange;
	
		// server-side appearance creation toggle
		bool            createAppearances;

		const char *    adminAccountDataTable;
		bool            adminGodToAll;
		int             adminGodToAllGodLevel;
		bool            useSecureLoginForGodAccess;
		bool            useIPForGodAccess;
		bool            adminPersistAllCreates;
		bool            buildCluster;                  // flag that this is the buildcluster

		int             requestSceneWarpTimeoutSeconds;

		bool            moveValidationEnabled;
		float           moveSpeedTolerance;            // tolerance when validating movement speed.  1.05 is 5% over normal max, etc.
		int             moveSpeedCheckFrequencyMs;     // how often to validate speed
		int             moveSpeedCheckFailureLeewaySeconds; // to lessen the impact of false positives in the speed check, only treat consecutive failure within this time period as true failure
		int             moveSpeedCheckFailureLeewayCount; // to lessen the impact of false positives in the speed check, only treat this many consecutive failure as true failure
		int             moveSlowdownFudgeTimeMs;       // how long to validate at the higher speed when slowing down
		bool            moveCheckDestinationCollision; // whether to check for bad destinations
		float           moveMaxDistance;               // maximum distance allowed to move for a single move packet
		int             moveValidationMaxInventoryOverload; // move validation fails if inventory is overloaded by this amount
		bool            moveValidationCheckForPastTimestamp;
		bool            moveValidationCheckForFutureTimestamp;
		bool            moveValidationCheckForTimestampDrift;
		int             moveValidationPastTimestampThresholdMs;
		int             moveValidationFutureTimestampThresholdMs;
		int             moveValidationTimestampDriftThresholdMs;
		float           moveMaxDistanceLeewayPerUpdate; // The absolute MAX movement between updates before we just warp you back instantly - No consecutive counts.

		// Override the name validation checking
		bool            nameValidationAcceptAll;

		float           tangiblePvpRegionCheckTime;
		float           creaturePvpRegionCheckTime;

		// paths for crafting armor
		const char *    defaultLightArmorTemplate;
		const char *    defaultMediumArmorTemplate;
		const char *    defaultHeavyArmorTemplate;
		float           armorDamageReduction;
		
		bool            stationPlayersEnabled;
		bool            stationPlayersShowAllDebugInfo;
		bool            commoditiesMarketEnabled;
		int             commoditiesServerServiceBindPort;
		const char *    commoditiesServerServiceBindInterface;
		bool            commoditiesShowAllDebugInfo;
		time_t          commoditiesQueryTimeoutSec;
		time_t          commoditiesServerReconnectIntervalSec;   //number of seconds to wait before trying to reconnect to the Commodities Server
		time_t          unclaimedAuctionItemDestroyTimeSec;
		int             maxReimburseAmount;
		bool            logBaselines;

		const char *    defaultVendorItemRestrictionFile;
		const char *    defaultVendorItemRestrictionRejectionMessage;

		int             weatherUpdateSeconds;

		const char *    instrumentDataTableFilename;
		const char *    performanceDataTableFilename;
		int             createQueueScheduleTime;

		bool            disableResources;
		
		// xp data
		int             xpMultiplier;
		const char *    xpManagerTemplate;
		float           gcwXpBonus;
		
		// AI options

		bool            reportAiStateChanges;
		bool            reportAiWarnings;
		bool            sendBreadcrumbs;
		bool            movementWhileRetreatingThrowsException;
		bool            hibernateEnabled;
		bool            hibernateProxies;     // if true, proxied creatures are allowed to hibernate on their authoritative server
		float           hibernateDistance;
		float           minHibernateAlter;
		float           maxHibernateAlter;
		int             aiPulseQueuePerFrame;
		int             aiPulseQueueMaxWaitTimeMs;
		float           aiBaseAggroRadius;
		float           aiMaxAggroRadius;
		float           aiLeashRadius;
		float           aiAssistRadius;

		bool            disableCreateQueue;
		float           maxTeleportDistanceWithoutSceneWarp;
		float           maxInstrumentPlayDistance;

		int             maxLotsPerAccount;
		int             unsafeLogoutTimeMs;
		int             idleLogoutTimeSec;
		int             idleLogoutTimeAfterCharacterTransferSec;

		const char *    fallbackDefaultWeapon;
		bool            logObservers;
		
		int             lastKnownLocationsResetTimeSec;
		int             maxMessageTosPerObjectPerFrame;
		int             maxMessageToTimePerObjectPerFrame;
		int             maxMessageToBounces;				

		int             missionRequestsPerFrame;
		bool            disableMissions;
		int             loopProfileLogThresholdMs;
		int             groundLoopProfileLogThresholdMs;
		int             spaceLoopProfileLogThresholdMs;
		bool            pvpDisableCombat;
		bool            logObjectLoading;

		bool            enforcePlayerTriggerVolumesOnly; 
		float           locationTargetCheckIntervalSec;
		const char *    gameServiceBindInterface;
		bool            logPvpUpdates;

		float           npcConversationDistance;    // max distance we can converse with an npc
		int             serverSpawnLimit;
		int             spawnQueueSize;
		int             framesPerSpawn;
		bool            enableSpawningNearLoadBeacons;
		bool            disableTravel;

		int             nonCriticalTasksPerFrame;

		bool            sendPlayerTransform;
		int             maxMoney;
		int             maxGalacticReserveDepositBillion;
		int             maxMoneyTransfer;
		int             maxFreeTrialMoney;

		bool            buildPreloadLists;
		bool            enablePreload;
		bool            logAuthTransfer;

		int             overrideUpdateRadius;
		float           buildingUpdateRadiusMultiplier;
		int             maxPopulationForNewbieTravel;
		int             debugAllAreasOverpopulated;
		int             minNewbieTravelLocations;
		int             numberOfMoveObjectLists;
		int             sitOnObjectReportThreshold;
		bool            fatalOnSitThreshold;
		int             databasePositionUpdateLongDelayIntervalMs;
		int             databasePositionUpdateShortDelayIntervalMs;
		bool            logPositionUpdates;
		bool            checkOriginCreates;
		bool            enableWho;
		bool            debugGroupChat;
		int             numberOfMissionsWantedInMissionBag;
		int             missionCreationQueueSize;
		int             connectToAllGameServersTimeout;
		int             houseItemLimitMultiplier;
		int             maxHouseItemLimit;
		bool            demandLoadHouseContents;
		int             unloadHouseContentsTime;
		int             loadHouseContentsDelaySeconds;
		bool            boundarySpawningAllowed;
		bool            debugMovement;
		bool            spawnAllResources;
		bool            skipUnreliableTransformsForOtherCells;
		int             maxPlayerSanityCheckFailures;
		bool            disablePlayerSanityChecker;

		// Update ranges for unreliable packets per 2 seconds

		float           updateRange7Packets;
		float           updateRange6Packets;
		float           updateRange5Packets;
		float           updateRange4Packets;
		float           updateRange3Packets;
		float           updateRange2Packets;
		float           updateRange1Packets;

		const char *    instantDeleteListDataTableName;
		bool            sendFrameProfilerInfo;
		bool            logAllCommands;
		bool            enableDebugControllerMessageSpam;
		bool            debugPvp;
		bool            allowMasterObjectCreation;

		bool            mountsEnabled;
		bool            mountsSanityCheckerEnabled;
		bool            mountsSanityCheckerBroadcastEnabled;
		int             mountsSanityCheckerBroadcastLimit;
		
		bool            moveSimEnabled;

		bool            exceptionOnObjVarZeroLengthArray;
		bool            trackNonAuthoritativeObjvarSets;

		float           triggerVolumeWarpDistance;
		NetworkId       maxGoldNetworkId;
		bool            fatalOnMovingGoldObject;
		int             spatialChatLogMinutes;

		float           behaviorMaxAvoidanceStuckTime;
		int             behaviorMaxAvoidancePersistenceFrameCount;
		bool            shipsEnabled;
		bool            logoutTriggerEnabled;

		int             veteranRewardTradeInWaitPeriodSeconds;
		bool            enableVeteranRewards;
		bool            enableOneYearAnniversary;
		int             minEntitledTime; // if the entitled time is less than this value, set it to this value
		bool            veteranDebugTriggerAll;
		bool            veteranDebugEnableOverrideAccountAge;
		bool            enableNewVeteranRewards;
		float           buddyPointTimeBonus; // buddy points can add to the player's entitlement time for veteran rewards
		bool		enableSceneGlobalData;

		float           manufactureTimeOverride;

		// max time we allow for creating theater objects in a frame
		int             theaterCreationLimitMilliseconds;

		bool            fatalOnGoldPobChange;

		int             maxWaypointsPerCharacter;

		float           maxSmallCreatureHeight;
		float           smallCreatureUpdateRadius;
		float           maxMediumCreatureHeight;
		float           mediumCreatureUpdateRadius;
		float           largeCreatureUpdateRadius;

		// long cooldowns need to be persisted to prevent exploit
		int             coolDownPersistThresholdSeconds;

		// TESTCENTER ONLY
		bool            loginAsBountyHunter;
		
		bool            checkNotifyRegions;
		bool            allowRegionTriggerOverride;

		bool            sendAsteroidExtents;
		bool            createZoneObjects;
		int             shipUpdatesPerSecond;
		bool            debugSpaceVisibilityManager;
		int             groundFrameRateLimit;
		int             spaceFrameRateLimit;
		bool            aiLoggingEnabled;
		bool            shipShotValidationEnabled;
		int             spaceAiEnemySearchesPerFrame;
		bool            groundShouldSleep;
		bool            spaceShouldSleep;
		bool            shipMoveValidationEnabled;
		int             shipMoveCheckIntervalMs;
		int             shipMoveCheckFudgeTimeMs; // how long validation uses higher values when they are being reduced
		float           shipMoveCheckTolerance;
		float           shipMoveCheckTimeToleranceSeconds;
		int             spaceAiAvoidanceChecksPerFrame;
		bool            spaceAiUsePathRefinement;
		bool            aiClientDebugEnabled;
		bool            shipClientUpdateDebugEnabled;
		int             spacePlanetServerUpdatesPerFrame;
		float           hyperspaceRandomOffsetRange;
		float           shipLoadInvulnerableTimeWithoutClient;
		float           shipLoadInvulnerableTimeWithClient;
		float           creatureLoadInvulnerableTimeWithoutClient;
		float           creatureLoadInvulnerableTimeWithClient;
		int             lineOfSightCacheDurationMs;
		float           lineOfSightCacheMinHeight;
		float           lineOfSightLocationRoundValue;
		bool            buildoutAreaEditingEnabled;

		bool            debugFloorPathNodeCount;
		int             populationReportTimeSeconds;
		float           maxCombatRange;
		bool            blockBuildRegionPlacement;

		int	            npeMinInstancePopulation;
		int	            npeMaxInstancePopulation;
		bool            stripNonFreeAssetsFromPlayersInTutorial;

		// weekly time (in GMT) when GCW rating recalculation occurs
		int             gcwRecalcTimeDayOfWeek; // Sunday=0, Monday=1, Tuesday=2, Wednesday=3, Thursday=4, Friday=5, Saturday=6
		int             gcwRecalcTimeHour;      // 0 - 23
		int             gcwRecalcTimeMinute;    // 0 - 59
		int             gcwRecalcTimeSecond;    // 0 - 59

		// weekly time (in GMT) when GCW score decay occurs;
		// additional time(s) can be specified in the config file if
		// you want the GCW score decay to occur more than once a week;
		// the following time is ignored if any time(s) is specified
		// in the config file; if no time(s) is specified in the
		// config file, it will use the following time as the default
		int             gcwScoreDecayTimeDayOfWeek; // Sunday=0, Monday=1, Tuesday=2, Wednesday=3, Thursday=4, Friday=5, Saturday=6
		int             gcwScoreDecayTimeHour;      // 0 - 23
		int             gcwScoreDecayTimeMinute;    // 0 - 59
		int             gcwScoreDecayTimeSecond;    // 0 - 59
		std::vector<std::pair<std::pair<int, int>, std::pair<int, int> > > gcwScoreDecayTime;

		// chat spam limiter
		int             chatStatisticsReportIntervalSeconds;
		int             chatSpamLimiterNumCharacters;
		int             chatSpamLimiterIntervalMinutes;
		bool            chatSpamLimiterEnabledForFreeTrial;
		int             chatSpamNotifyPlayerWhenLimitedIntervalSeconds;

		int             pendingLoadRequestLimit;

		// city citizenship fixup at cluster startup
		bool            enableCityCitizenshipFixup;

		// city citizenship inactive packup
		int             cityCitizenshipInactivePackupStartTimeEpoch;
		int             cityCitizenshipInactivePackupInactiveTimeSeconds;

		// city retro player city creation time
		const char *    retroactivePlayerCityCreationTimeFilename;

		// warden
		int             wardenSquelchDurationSeconds;
		bool            enableWarden;
		bool            enableWardenCanSquelchSecuredLogin;
		bool            enableWardenToDeputizeWarden;

		// collection
		bool            disableCollectionServerFirstGrantForSecuredLogin;

		// locateStructure command spam limitter
		int             locateStructureCommandIntervalSeconds;

		// locateVendor command spam limitter
		int             locateVendorCommandIntervalSeconds;

		// CTS
		int             ctsDefaultScriptDictionarySizeBytes;
		const char *    retroactiveCtsHistoryFilename;
		bool            allowIgnoreFreeCtsTimeRestriction;

		// group pickup point
		int             groupPickupPointApprovalRangeSquared;
		int             groupPickupPointTimeLimitSeconds;
		int             groupPickupTravelPlayerCityPercent; // if the destination is a player city's shuttleport, the city gets so many % of the travel cost

		// combat restriction for group invite/join
		bool            groupInviteInviterCombatRestriction;
		bool            groupInviteInviteeCombatRestriction;
		bool            groupJoinInviterCombatRestriction;
		bool            groupJoinInviteeCombatRestriction;

		// lfg
		int             characterMatchMaxMatchCount;

		// unlocked slot management commands
		bool            enableOccupyUnlockedSlotCommand;
		bool            enableVacateUnlockedSlotCommand;
		bool            enableSwapUnlockedSlotCommand;
		int             occupyVacateUnlockedSlotCommandCooldownSeconds;

		// TCG
		// the SWG account feature id that is incremented each time the account receives its
		// free monthly booster pack on the account anniversary date; this will be set by
		// some other product; we'll check it and display an announcement message and then
		// clear it, so that it can be set again next month so we can display the announcement
		// message next month
		int             accountFeatureIdForMonthlyBoosterPack;

		// TCG
		// account feature id to indicate account needs to be notified of qualification for the TCG Beta
		int             accountFeatureIdForTcgBetaAnnouncement;

		// Event Logging
		bool            logEventObjectCreation;
		bool            logEventObjectDestruction;

		float           regionFlagUpdateTime;

		// abandoned house pack up
		const char *    housePackupAccountListDataTableName;

		// /pickupAllRoomItemsIntoInventory and /dropAllInventoryItemsIntoRoom
		// commands how many items to do each pass, to spread out the load
		int             pickDropAllRoomItemsNumberOfItemsPerRound;

		// Neutral mercenary status
		bool            enableCovertImperialMercenary;
		bool            enableOvertImperialMercenary;
		bool            enableCovertRebelMercenary;
		bool            enableOvertRebelMercenary;

		// GCW
		int             broadcastGcwScoreToOtherGalaxiesIntervalSeconds;
		bool            broadcastGcwScoreToOtherGalaxies;
		bool            receiveGcwScoreFromOtherGalaxies;
		int             trackGcwPercentileHistoryCount; // keep the previous nnn GCW percentile score, for historical tracking/analysis
		bool            gcwFactionalPresenceDisableForSecuredLogin;
		int             gcwFactionalPresenceMountedPct;
		int             gcwFactionalPresenceLevelPct;
		int             gcwFactionalPresenceGcwRankBonusPct;
		int             gcwFactionalPresenceAlignedCityBonusPct;
		int             gcwFactionalPresenceAlignedCityRankBonusPct;
		int             gcwFactionalPresenceAlignedCityAgeBonusPct;
		int             gcwGuildMinMembersForGcwRegionDefender;
		int             gcwRegionDefenderTotalBonusPct;
		int             gcwDaysRequiredForGcwRegionDefenderBonus;

		bool			useOldSuidGenerator;

		const char *    serverLoadLevel;

		int             maxHousingLots;
	};

  private:

	static Data *data;
	static Vector startingPosition;
	static std::map<std::string, std::string> m_components;
	

  public:

	static void install    (void);
	static void remove     (void);

	static bool             getAllowDefaultTemplateParams(void);

	static bool             getSkipTutorial             (void);
	static const char *     getGroundScene              (void);
	static bool             getDisableCreateQueue       (void);
	static const uint16     getPort                     (void);
	static const char *     getSpaceScene               (void);
	static const uint16     getTaskManagerPort          (void);

	static const char *     getCentralServerAddress     (void);
	static const uint16     getCentralServerPort        (void);
	static const char *     getSceneID                  (void);
	static int              getPreloadNumber            (void);
	static const Vector&    getStartingPosition         (void);

	static const char *     getDSN                      (void);
	static const char *     getAlternateDSN             (void);
	static const char *     getDatabaseUID              (void);
	static const char *     getDatabasePWD              (void);
	static const char *     getDatabaseProtocol         (void);
	static int              getDatabaseWorkerThreads    (void);

	static bool             getDisableCombat            (void);
	static const char *     getCombatConfigFile         (void);

	static float		getCombatDamageDelaySeconds (void);
	static int              getTriggerVolumeSystem      (void);

	static int		getDefaultAutoExpireTargetDuration(void);
	static float		getInteriorTargetDurationFactor(void);

	static const char *     getJavaVMName               (void);
	static const char *     getScriptPath               (void);
	static const char *     getJavaLibPath              (void);
	static const char *     getJavaDebugPort            (void);
	static bool             getUseRemoteDebugJava       (void);
	static bool             getProfileScripts           (void);
	static bool             getCrashOnScriptError       (void);
	static bool             getCompileScripts           (void);
	static bool             getTrapScriptCrashes        (void);
	static int              getScriptWatcherWarnTime    (void);
	static int              getScriptWatcherInterruptTime(void);
	static int              getScriptStackErrorLimit    (void);
	static int              getScriptStackErrorLevel    (void);
	static int              getJavaLocalRefLimit        (void);
	static bool             getDisableObjvarNullCheck   (void);

	static int              getMaxGameServerToSendUniverseData(void);
	static int              getTimeoutToAckUniverseDataReceived(void);

	static int              getMinPoolValue             (void);
	static int              getMinFaucetValue           (void);
	static int              getMinDrainValue            (void);
	static int              getMinEffectiveFaucetValue  (void);
	static float            getDefaultHealthRegen       (void);
	static float            getDefaultActionRegen       (void);
	static float            getDefaultMindRegen         (void);
	static float            getRegenThreshold           (void);
	static float            getMinForcePowerRegenRate   (void);
	static bool             getEnableAttribModWarnings  (void);
	static bool             getCheckAttribLimits        (void);
	static int              getMaxItemAttribBonus       (void);
	static int              getMaxTotalAttribBonus      (void);

	static bool             getEnableNewJedi            (void);
	static const char *     getJediManagerTemplate      (void);
	static int              getMaxJediBounties          (void);
	static int              getStartForcePower          (void);
	static bool             getLogJediConcludes         (void);
	static bool             getForceJediConcludes       (void);
	static int              getJediUpdateLocationTimeSeconds(void);

	static const char *     getPlanetObjectTemplate     (void);
	static const int        getSecondsPerResourceTick   (void);
	static const uint32     getResourceTimeScale        (void);
	static const float      getWatchedHarvesterAlterTime(void);
    static const float      getUnWatchedHarvesterAlterTime(void);
	static const bool       getDeactivateHarvesterIfDamaged(void);
	static const float		getHarvesterExtractionRateMultiplier(void);
	static const int        getUniverseCheckFrequencySeconds(void);

	static const bool       getJavaConsoleDebugMessages (void);
	static const bool	getUseJavaXcheck	    (void);
	static const bool       getUseVerboseJava           (void);
	static const bool       getLogJavaGc                (void);

	static const char *        getReservedNameFile          (void);
	static const char *        getCharacterNameGeneratorDirectory(void);
	static const std::string & getCraftingComponentTableFile(const std::string & name);
	static bool                getCraftingComponentStrict   (void);
	static int                 getCraftingXpChance          (void);
	static const char *        getCraftingXpPercentTable    (void);

	static int              getMaxObjectSkillModBonus(void);
	static int              getMaxSocketSkillModBonus(void);
	static int              getMaxSkillModSockets(void);

	static float            getBaseDecayRate(void);
	static float            getWeaponDecayThreshold(void);

	static const char *     getRegionFilesName(void);
	static int              getRegionTreeDepth(void);
	static const char *     getBattlefieldMarkerName(void);

	static int              getReservedObjectIds        (void);
	static int              getMinObjectIds             (void);

	static const char *     getPlayerObjectTemplate     (void);

	static int              getClientOutOfRangeObjectCacheTimeMs(void);
	static int              getPlayerInterestRange      (void);
	static int              getPvpUpdateTimeMs          (void);
	static int              getPvpTempFlagExpireTimeMs  (void);
	static int              getPvpGuildWarCoolDownPeriodTimeMs(void);
	static int              getPvpGuildWarExemptionExclusiveDelaySeconds(void);
	static int              getPvpBattlefieldEnemyFlagExpireTimeMs(void);
	static const char *     getPvpFactionOpponentsFilename(void);
	static const char *     getPvpNonaggressiveFactionsFilename(void);
	static const char *     getPvpUnattackableFactionsFilename(void);
	static const char *     getPvpBountyTargetFactionsFilename(void);

	static bool             getCreateAppearances        (void);

	static const char *     getAdminAccountDataTable    (void);
	static bool             getAdminGodToAll            (void);
	static int              getAdminGodToAllGodLevel    (void);
	static bool             getUseSecureLoginForGodAccess();
	static bool             getUseIPForGodAccess(); 
	static bool             getAdminPersistAllCreates   (void);
	static bool             getBuildCluster             (void);

	static int              getRequestSceneWarpTimeoutSeconds(void);

	static bool             getMoveValidationEnabled    (void);
	static float            getMoveSpeedTolerance       (void);
	static int              getMoveSpeedCheckFrequencyMs(void);
	static int              getMoveSpeedCheckFailureLeewaySeconds(void);
	static int              getMoveSpeedCheckFailureLeewayCount(void);
	static int              getMoveSlowdownFudgeTimeMs  (void);
	static bool             getMoveCheckDestinationCollision(void);
	static float            getMoveMaxDistance          (void);
	static int              getMoveValidationMaxInventoryOverload(void);
	static bool             getMoveValidationCheckForPastTimestamp(void);
	static bool             getMoveValidationCheckForFutureTimestamp(void);
	static bool             getMoveValidationCheckForTimestampDrift(void);
	static int              getMoveValidationPastTimestampThresholdMs(void);
	static int              getMoveValidationFutureTimestampThresholdMs(void);
	static int              getMoveValidationTimestampDriftThresholdMs(void);
	static float            getMoveMaxDistanceLeewayPerUpdate(void);

	static bool             getNameValidationAcceptAll();

	static float            getTangiblePvpRegionCheckTime(void);
	static float            getCreaturePvpRegionCheckTime(void);

	static const char *     getDefaultLightArmorTemplate(void);
	static const char *     getDefaultMediumArmorTemplate(void);
	static const char *     getDefaultHeavyArmorTemplate(void);
	static float            getArmorDamageReduction(void);

	static bool             getStationPlayersEnabled(void);
	static bool             getStationPlayersShowAllDebugInfo();
	static bool             getCommoditiesMarketEnabled(void);
	static int              getCommoditiesServerServiceBindPort();
	static const char *     getCommoditiesServerServiceBindInterface();
	static bool             getCommoditiesShowAllDebugInfo();
	static time_t           getCommoditiesQueryTimeoutSec(void);
	static time_t           getCommoditiesServerReconnectIntervalSec(void);
	static time_t           getUnclaimedAuctionItemDestroyTimeSec(void);
	static int              getMaxReimburseAmount();

	static bool             getLogBaselines(void);

	static const char *     getDefaultVendorItemRestrictionFile();
	static const char *     getDefaultVendorItemRestrictionRejectionMessage();

	static int              getWeatherUpdateSeconds();
	static const char *     getInstrumentDataTableFilename(void);
	static const char *     getPerformanceDataTableFilename(void);
	static const int        getCreateQueueScheduleTime(void);
	static const bool       getDisableResources();
	
	static int              getXpMultiplier();
	static const char *     getXpManagerTemplate();
	static float            getGcwXpBonus();

	static const bool       getReportAiStateChanges(void);
	static const bool       getReportAiWarnings(void);
	static const bool       getSendBreadcrumbs(void);
	static const bool       getMovementWhileRetreatingThrowsException(void);
	static const bool       getHibernateEnabled(void);
	static const bool       getHibernateProxies(void);
	static const float      getHibernateDistance(void);
	static const float      getMinHibernateAlter(void);
	static const float      getMaxHibernateAlter(void);
	static const int        getAiPulseQueuePerFrame();
	static const unsigned int getAiPulseQueueMaxWaitTimeMs();

	static const float      getMaxTeleportDistanceWithoutSceneWarp(void);
	static const float      getMaxInstrumentPlayDistance(void);

	static const int        getMaxLotsPerAccount(void);
	static const int        getUnsafeLogoutTimeMs(void);
	static const unsigned long getIdleLogoutTimeSec(void);
	static const unsigned long getIdleLogoutTimeAfterCharacterTransferSec(void);

	static const char *     getFallbackDefaultWeapon(void);

	static bool             getLogObservers();
	static int              getLastKnownLocationsResetTimeSec();
	static int              getMaxMessageTosPerObjectPerFrame();
	static int              getMaxMessageToTimePerObjectPerFrame();
	static int              getMaxMessageToBounces();

	static int              getMissionRequestsPerFrame();
	static bool             getDisableMissions();
	
	static unsigned long    getLoopProfileLogThresholdMs();
	static unsigned long    getGroundLoopProfileLogThresholdMs();
	static unsigned long    getSpaceLoopProfileLogThresholdMs();
	static bool             getPvpDisableCombat();
	static bool             getLogObjectLoading();

	static bool             getEnforcePlayerTriggerVolumesOnly();
	static float            getLocationTargetCheckIntervalSec();
	static const char *     getGameServiceBindInterface();
	static bool             getLogPvpUpdates();

	static float            getNpcConversationDistance();
	static int              getServerSpawnLimit();
	static int              getSpawnQueueSize();
	static int              getFramesPerSpawn();
	static bool             getEnableSpawningNearLoadBeacons();
	static bool             getDisableTravel();

	static int              getNonCriticalTasksPerFrame();

	static bool             getSendPlayerTransform();
	static int              getMaxMoney();
	static int              getMaxGalacticReserveDepositBillion();
	static int              getMaxMoneyTransfer();
	static int              getMaxFreeTrialMoney();

	static bool             getBuildPreloadLists();
	static bool             getEnablePreload();
	static bool             getLogAuthTransfer();

	static int              getOverrideUpdateRadius();
	static float            getBuildingUpdateRadiusMultiplier();
	static int              getMaxPopulationForNewbieTravel();
	static int              getDebugAllAreasOverpopulated();
	static int              getMinNewbieTravelLocations();
	static int              getNumberOfMoveObjectLists();
	static int              getSitOnObjectReportThreshold();
	static bool             getFatalOnSitThreshold();
	static int              getDatabasePositionUpdateLongDelayIntervalMs();
	static int              getDatabasePositionUpdateShortDelayIntervalMs();
	static bool             getLogPositionUpdates();

	static bool             getCheckOriginCreates();
	static bool             getEnableWho();
	static bool             getDebugGroupChat();
	static int              getNumberOfMissionsWantedInMissionBag();
	static int              getMissionCreationQueueSize();
	static int              getConnectToAllGameServersTimeout();
	static int              getHouseItemLimitMultiplier();
	static int              getMaxHouseItemLimit();
	static bool             getDemandLoadHouseContents();
	static int              getUnloadHouseContentsTime();
	static int              getLoadHouseContentsDelaySeconds();
	static bool             getBoundarySpawningAllowed();
	static bool             getDebugMovement();
	static bool             getSpawnAllResources();
	static bool             getSkipUnreliableTransformsForOtherCells();
	static int              getMaxPlayerSanityCheckFailures();
	static bool             getDisablePlayerSanityChecker();
	static float            getUpdateRange7PacketsSquared();
	static float            getUpdateRange6PacketsSquared();
	static float            getUpdateRange5PacketsSquared();
	static float            getUpdateRange4PacketsSquared();
	static float            getUpdateRange3PacketsSquared();
	static float            getUpdateRange2PacketsSquared();
	static float            getUpdateRange1PacketsSquared();
	static const char *     getInstantDeleteListDataTableName();
	static bool             getSendFrameProfilerInfo();
	static bool             getLogAllCommands();
	static bool             getEnableDebugControllerMessageSpam();
	static bool             getDebugPvp();
	static bool             getAllowMasterObjectCreation();

	static bool             getMountsEnabled();
	static bool             getMountsSanityCheckerEnabled();
	static bool             getMountsSanityCheckerBroadcastEnabled();
	static int              getMountsSanityCheckerBroadcastLimit();

	static bool             getMoveSimEnabled ();

	static bool             getExceptionOnObjVarZeroLengthArray();
	static bool             getTrackNonAuthoritativeObjvarSets();

	static float            getTriggerVolumeWarpDistance();
	static const NetworkId &getMaxGoldNetworkId();
	static bool             getFatalOnMovingGoldObject();
	static int              getSpatialChatLogMinutes();

	static float            getBehaviorMaxAvoidanceStuckTime();
	static int              getBehaviorMaxAvoidancePersistenceFrameCount();
	static bool             getShipsEnabled();
	static bool             getLogoutTriggerEnabled();

	static int              getVeteranRewardTradeInWaitPeriodSeconds();
	static bool             getEnableVeteranRewards();
	static bool             getEnableOneYearAnniversary();
	static int              getMinEntitledTime();
	static bool             getVeteranDebugTriggerAll();
	static bool             getVeteranDebugEnableOverrideAccountAge();
	static bool             getEnableNewVeteranRewards();
	static float            getBuddyPointTimeBonus();
	static bool             getEnableSceneGlobalData();

	static float            getManufactureTimeOverride();

	static int              getTheaterCreationLimitMilliseconds();

	static bool             getFatalOnGoldPobChange();

	static int              getMaxWaypointsPerCharacter();

	static float            getMaxSmallCreatureHeight();
	static float            getSmallCreatureUpdateRadius();
	static float            getMaxMediumCreatureHeight();
	static float            getMediumCreatureUpdateRadius();
	static float            getLargeCreatureUpdateRadius();

	static int              getCoolDownPersistThresholdSeconds();

	static bool             getLoginAsBountyHunter();
	
	static bool             getCheckNotifyRegions();
	static bool             getAllowRegionTriggerOverride();

	static bool             getSendAsteroidExtents();
	static bool             getCreateZoneObjects();
	static int              getShipUpdatesPerSecond();
	static bool             getDebugSpaceVisibilityManager();
	static int              getGroundFrameRateLimit();
	static int              getSpaceFrameRateLimit();
	static bool             isSpaceAiLoggingEnabled();
	static bool             isAiLoggingEnabled();
	static bool             getShipShotValidationEnabled();
	static int              getSpaceAiEnemySearchesPerFrame();
	static bool             getGroundShouldSleep();
	static bool             getSpaceShouldSleep();
	static bool             getShipMoveValidationEnabled();
	static int              getShipMoveCheckIntervalMs();
	static int              getShipMoveCheckFudgeTimeMs();
	static float            getShipMoveCheckTolerance();
	static float            getShipMoveCheckTimeToleranceSeconds();
	static int              getSpaceAiAvoidanceChecksPerFrame();
	static bool             getSpaceAiUsePathRefinement();
	static bool             isAiClientDebugEnabled();
	static bool             getShipClientUpdateDebugEnabled();
	static int              getSpacePlanetServerUpdatesPerFrame();
	static float            getHyperspaceRandomOffsetRange();
	static float            getShipLoadInvulnerableTimeWithoutClient();
	static float            getShipLoadInvulnerableTimeWithClient();
	static float            getCreatureLoadInvulnerableTimeWithoutClient();
	static float            getCreatureLoadInvulnerableTimeWithClient();
	static unsigned long    getLineOfSightCacheDurationMs();
	static float            getLineOfSightCacheMinHeight();
	static float            getLineOfSightLocationRoundValue();
	static bool             getBuildoutAreaEditingEnabled();

	static bool             getDebugFloorPathNodeCount();
	static float            getAiBaseAggroRadius();
	static float            getAiMaxAggroRadius();
	static float            getAiLeashRadius();
	static float            getAiAssistRadius();
	static int 		        getPopulationReportTimeSeconds();
	static float            getMaxCombatRange();
	static bool             getBlockBuildRegionPlacement();

	static int              getNpeMinInstancePopulation();
	static int              getNpeMaxInstancePopulation();
	static bool             getStripNonFreeAssetsFromPlayersInTutorial();

	static int              getGcwRecalcTimeDayOfWeek();
	static int              getGcwRecalcTimeHour();
	static int              getGcwRecalcTimeMinute();
	static int              getGcwRecalcTimeSecond();

	static std::vector<std::pair<std::pair<int, int>, std::pair<int, int> > > const & getGcwScoreDecayTime();

	static int              getChatStatisticsReportIntervalSeconds();
	static int              getChatSpamLimiterNumCharacters();
	static int              getChatSpamLimiterIntervalMinutes();
	static bool             getChatSpamLimiterEnabledForFreeTrial();
	static int              getChatSpamNotifyPlayerWhenLimitedIntervalSeconds();

	static int              getPendingLoadRequestLimit();

	static bool             getEnableCityCitizenshipFixup();

	static int              getCityCitizenshipInactivePackupStartTimeEpoch();
	static int              getCityCitizenshipInactivePackupInactiveTimeSeconds();

	static const char *     getRetroactivePlayerCityCreationTimeFilename();

	static int              getWardenSquelchDurationSeconds();
	static bool             getEnableWarden();
	static bool             getEnableWardenCanSquelchSecuredLogin();
	static bool             getEnableWardenToDeputizeWarden();

	static bool             getDisableCollectionServerFirstGrantForSecuredLogin();

	static int              getLocateStructureCommandIntervalSeconds();

	static int              getLocateVendorCommandIntervalSeconds();

	static size_t           getCtsDefaultScriptDictionarySizeBytes();
	static const char *     getRetroactiveCtsHistoryFilename();
	static bool             getAllowIgnoreFreeCtsTimeRestriction();

	static int              getGroupPickupPointApprovalRangeSquared();
	static int              getGroupPickupPointTimeLimitSeconds();
	static int              getGroupPickupTravelPlayerCityPercent();

	static bool             getGroupInviteInviterCombatRestriction();
	static bool             getGroupInviteInviteeCombatRestriction();
	static bool             getGroupJoinInviterCombatRestriction();
	static bool             getGroupJoinInviteeCombatRestriction();

	static size_t           getCharacterMatchMaxMatchCount();

	static bool             getEnableOccupyUnlockedSlotCommand();
	static bool             getEnableVacateUnlockedSlotCommand();
	static bool             getEnableSwapUnlockedSlotCommand();
	static int              getOccupyVacateUnlockedSlotCommandCooldownSeconds();

	static int              getAccountFeatureIdForMonthlyBoosterPack();
	static int              getAccountFeatureIdForTcgBetaAnnouncement();

	static bool             getLogEventObjectCreation();
	static bool             getLogEventObjectDestruction();

	static float            getRegionFlagUpdateTimeSecs();
	static const char *     getHousePackupAccountListDataTableName();

	static int              getPickDropAllRoomItemsNumberOfItemsPerRound();

	static bool             getEnableCovertImperialMercenary();
	static bool             getEnableOvertImperialMercenary();
	static bool             getEnableCovertRebelMercenary();
	static bool             getEnableOvertRebelMercenary();
	static int              getBroadcastGcwScoreToOtherGalaxiesIntervalSeconds();
	static bool             getBroadcastGcwScoreToOtherGalaxies();
	static bool             getReceiveGcwScoreFromOtherGalaxies();
	static int              getTrackGcwPercentileHistoryCount();
	static bool             getGcwFactionalPresenceDisableForSecuredLogin();
	static int              getGcwFactionalPresenceMountedPct();
	static int              getGcwFactionalPresenceLevelPct();
	static int              getGcwFactionalPresenceGcwRankBonusPct();
	static int              getGcwFactionalPresenceAlignedCityBonusPct();
	static int              getGcwFactionalPresenceAlignedCityRankBonusPct();
	static int              getGcwFactionalPresenceAlignedCityAgeBonusPct();
	static int              getGcwGuildMinMembersForGcwRegionDefender();
	static int              getGcwRegionDefenderTotalBonusPct();
	static int              getGcwDaysRequiredForGcwRegionDefenderBonus();

	static bool				getUseOldSuidGenerator();

	static const char *     getServerLoadLevel();

	static int              getMaxHousingLots();
};

//-----------------------------------------------------------------------

inline int ConfigServerGame::getSitOnObjectReportThreshold()
{
	return data->sitOnObjectReportThreshold;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getFatalOnSitThreshold()
{
	return data->fatalOnSitThreshold;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getSendPlayerTransform(void)
{

	return data->sendPlayerTransform;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getDisableTravel()
{
	return data->disableTravel;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getAllowDefaultTemplateParams(void)
{
	return data->allowDefaultTemplateParams;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getSkipTutorial(void)
{
	return data->skipTutorial;
}

inline const char * ConfigServerGame::getGroundScene(void)
{
	return data->groundScene;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigServerGame::getTaskManagerPort(void)
{
	return static_cast<const uint16>(data->taskManagerPort);
}

//-----------------------------------------------------------------------

inline const char * ConfigServerGame::getSpaceScene(void)
{
	return data->spaceScene;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigServerGame::getPort(void)
{
	return static_cast<const uint16>(data->port);
}

//-----------------------------------------------------------------------
inline const char * ConfigServerGame::getSceneID(void)
{
	return data->sceneID;
}

//-----------------------------------------------------------------------
inline int ConfigServerGame::getPreloadNumber(void)
{
	return data->preloadNumber;
}

//-----------------------------------------------------------------------

inline const char * ConfigServerGame::getCentralServerAddress(void)
{
	return data->centralServerAddress;
}

//-----------------------------------------------------------------------

inline const uint16 ConfigServerGame::getCentralServerPort(void)
{
	return static_cast<const uint16>(data->centralServerPort);
}

//-----------------------------------------------------------------------


inline const Vector& ConfigServerGame::getStartingPosition(void)
{
	return startingPosition;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getDisableCombat(void)
{
	return data->disableCombat;
}

//-----------------------------------------------------------------------

inline const char *ConfigServerGame::getCombatConfigFile(void)
{
	return data->combatConfigFile;
}

//-----------------------------------------------------------------------

inline float ConfigServerGame::getCombatDamageDelaySeconds(void)
{
	return data->combatDamageDelaySeconds;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getTriggerVolumeSystem(void)
{
	return data->triggerVolumeSystem;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getDefaultAutoExpireTargetDuration(void)
{
	return data->defaultAutoExpireTargetDuration;
}


//-----------------------------------------------------------------------

inline float ConfigServerGame::getInteriorTargetDurationFactor(void)
{
	return data->interiorTargetDurationFactor;
}


//-----------------------------------------------------------------------

inline const char * ConfigServerGame::getScriptPath(void)
{
	return data->scriptPath;
}

//-----------------------------------------------------------------------

inline const char *ConfigServerGame::getJavaLibPath(void)
{
	return data->javaLibPath;
}

//-----------------------------------------------------------------------

inline const char *ConfigServerGame::getJavaDebugPort(void)
{
	return data->javaDebugPort;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getUseRemoteDebugJava(void)
{
	return data->useRemoteDebugJava;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getProfileScripts(void)
{
	return data->profileScripts;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getCrashOnScriptError(void)
{
	return data->crashOnScriptError;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getCompileScripts(void)
{
	return data->compileScripts;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getTrapScriptCrashes(void)
{
	return data->trapScriptCrashes;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getScriptWatcherWarnTime(void)
{
	return data->scriptWatcherWarnTime;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getScriptWatcherInterruptTime(void)
{
	return data->scriptWatcherInterruptTime;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getScriptStackErrorLimit(void)
{
	return data->scriptStackErrorLimit;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getScriptStackErrorLevel(void)
{
	return data->scriptStackErrorLevel;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getJavaLocalRefLimit(void)
{
	return data->javaLocalRefLimit;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getDisableObjvarNullCheck(void)
{
	return data->disableObjvarNullCheck;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getMaxGameServerToSendUniverseData(void)
{
	return data->maxGameServerToSendUniverseData;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getTimeoutToAckUniverseDataReceived(void)
{
	return data->timeoutToAckUniverseDataReceived;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getMinPoolValue(void)
{
	return data->minPoolValue;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getMinFaucetValue(void)
{
	return data->minFaucetValue;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getMinDrainValue(void)
{
	return data->minDrainValue;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getMinEffectiveFaucetValue(void)
{
	return data->minEffectiveFaucetValue;
}

//-----------------------------------------------------------------------

inline float ConfigServerGame::getDefaultHealthRegen(void)
{
	return data->defaultHealthRegen;
}

//-----------------------------------------------------------------------

inline float ConfigServerGame::getDefaultActionRegen(void)
{
	return data->defaultActionRegen;
}

//-----------------------------------------------------------------------

inline float ConfigServerGame::getDefaultMindRegen(void)
{
	return data->defaultMindRegen;
}

//-----------------------------------------------------------------------

inline float ConfigServerGame::getRegenThreshold(void)
{
	return data->regenThreshold;
}

//-----------------------------------------------------------------------

inline float ConfigServerGame::getMinForcePowerRegenRate(void)
{
	return data->minForcePowerRegenRate;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableAttribModWarnings(void)
{
	return data->enableAttribModWarnings;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getCheckAttribLimits(void)
{
	return data->checkAttribLimits;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getMaxItemAttribBonus(void)
{
	return data->maxItemAttribBonus;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getMaxTotalAttribBonus(void)
{
	return data->maxTotalAttribBonus;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableNewJedi(void)
{
	return data->enableNewJedi;
}

//-----------------------------------------------------------------------

inline const char * ConfigServerGame::getJediManagerTemplate(void)
{
	return data->jediManagerTemplate;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getMaxJediBounties(void)
{
	return data->maxJediBounties;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getStartForcePower(void)
{
	return data->startForcePower;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getLogJediConcludes(void)
{
	return data->logJediConcludes;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getForceJediConcludes(void)
{
	return data->forceJediConcludes;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getJediUpdateLocationTimeSeconds(void)
{
	return data->jediUpdateLocationTimeSeconds;
}

//-----------------------------------------------------------------------

inline const bool ConfigServerGame::getJavaConsoleDebugMessages()
{
	return data->javaConsoleDebugMessages;
}

//-----------------------------------------------------------------------

inline const bool ConfigServerGame::getUseJavaXcheck()
{
        return data->javaUseXcheck;
}

//-----------------------------------------------------------------------

inline const bool ConfigServerGame::getUseVerboseJava(void)
{
	return data->useVerboseJava;
}

//-----------------------------------------------------------------------

inline const bool ConfigServerGame::getLogJavaGc(void)
{
	return data->logJavaGc;
}

//-----------------------------------------------------------------------

inline const char *ConfigServerGame::getPlanetObjectTemplate()
{
	return data->planetObjectTemplate;
}

//-----------------------------------------------------------------------

inline const char *ConfigServerGame::getReservedNameFile()
{
	return data->reservedNameFile;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getCraftingComponentStrict(void)
{
	return data->craftingComponentStrict;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getCraftingXpChance(void)
{
	return data->craftingXpChance;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getCraftingXpPercentTable(void)
{
	return data->craftingXpPercentTable;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMaxObjectSkillModBonus(void)
{
	return data->maxObjectSkillModBonus;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMaxSocketSkillModBonus(void)
{
	return data->maxSocketSkillModBonus;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMaxSkillModSockets(void)
{
	return data->maxSkillModSockets;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getBaseDecayRate(void)
{
	return data->baseDecayRate;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getWeaponDecayThreshold(void)
{
	return data->weaponDecayThreshold;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getRegionFilesName(void)
{
	return data->regionFilesName;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getRegionTreeDepth(void)
{
	return data->regionTreeDepth;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getBattlefieldMarkerName(void)
{
	return data->battlefieldMarkerName;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getReservedObjectIds()
{
	return data->reservedObjectIds;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMinObjectIds()
{
	return data->minObjectIds;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getPlayerObjectTemplate()
{
	return data->playerObjectTemplate;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getClientOutOfRangeObjectCacheTimeMs()
{
	return data->clientOutOfRangeObjectCacheTimeMs;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getPlayerInterestRange()
{
	return data->playerInterestRange;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getPvpUpdateTimeMs()
{
	return data->pvpUpdateTimeMs;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getPvpTempFlagExpireTimeMs()
{
	return data->pvpTempFlagExpireTimeMs;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getPvpGuildWarCoolDownPeriodTimeMs()
{
	return data->pvpGuildWarCoolDownPeriodTimeMs;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getPvpGuildWarExemptionExclusiveDelaySeconds()
{
	return data->pvpGuildWarExemptionExclusiveDelaySeconds;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getPvpBattlefieldEnemyFlagExpireTimeMs()
{
	return data->pvpBattlefieldEnemyFlagExpireTimeMs;
}

// ----------------------------------------------------------------------

inline const char *ConfigServerGame::getPvpFactionOpponentsFilename()
{
	return data->pvpFactionOpponentsFilename;
}

// ----------------------------------------------------------------------

inline const char *ConfigServerGame::getPvpNonaggressiveFactionsFilename()
{
	return data->pvpNonaggressiveFactionsFilename;
}

// ----------------------------------------------------------------------

inline const char *ConfigServerGame::getPvpUnattackableFactionsFilename()
{
	return data->pvpUnattackableFactionsFilename;
}

// ----------------------------------------------------------------------

inline const char *ConfigServerGame::getPvpBountyTargetFactionsFilename()
{
	return data->pvpBountyTargetFactionsFilename;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getCreateAppearances()
{
	return data->createAppearances;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getAdminAccountDataTable(void)
{
	return data->adminAccountDataTable;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getAdminGodToAll()
{
	return data->adminGodToAll;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getAdminGodToAllGodLevel()
{
	return data->adminGodToAllGodLevel;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getUseSecureLoginForGodAccess()
{
	return data->useSecureLoginForGodAccess;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getUseIPForGodAccess()
{
	return data->useIPForGodAccess;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getAdminPersistAllCreates()
{
	return data->adminPersistAllCreates;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getBuildCluster(void)
{
	return data->buildCluster;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getRequestSceneWarpTimeoutSeconds(void)
{
	return data->requestSceneWarpTimeoutSeconds;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getMoveValidationEnabled()
{
	return data->moveValidationEnabled;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getMoveSpeedTolerance()
{
	return data->moveSpeedTolerance;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMoveSpeedCheckFrequencyMs()
{
	return data->moveSpeedCheckFrequencyMs;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMoveSpeedCheckFailureLeewaySeconds()
{
	return data->moveSpeedCheckFailureLeewaySeconds;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMoveSpeedCheckFailureLeewayCount()
{
	return data->moveSpeedCheckFailureLeewayCount;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMoveSlowdownFudgeTimeMs()
{
	return data->moveSlowdownFudgeTimeMs;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getMoveCheckDestinationCollision()
{
	return data->moveCheckDestinationCollision;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getMoveMaxDistance()
{
	return data->moveMaxDistance;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMoveValidationMaxInventoryOverload()
{
	return data->moveValidationMaxInventoryOverload;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getMoveValidationCheckForPastTimestamp(void)
{
	return data->moveValidationCheckForPastTimestamp;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getMoveValidationCheckForFutureTimestamp(void)
{
	return data->moveValidationCheckForFutureTimestamp;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getMoveValidationCheckForTimestampDrift(void)
{
	return data->moveValidationCheckForTimestampDrift;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMoveValidationPastTimestampThresholdMs(void)
{
	return data->moveValidationPastTimestampThresholdMs;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMoveValidationFutureTimestampThresholdMs(void)
{
	return data->moveValidationFutureTimestampThresholdMs;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMoveValidationTimestampDriftThresholdMs(void)
{
	return data->moveValidationTimestampDriftThresholdMs;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getMoveMaxDistanceLeewayPerUpdate(void)
{
	return data->moveMaxDistanceLeewayPerUpdate;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getNameValidationAcceptAll()
{
	return data->nameValidationAcceptAll;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getTangiblePvpRegionCheckTime(void)
{
	return data->tangiblePvpRegionCheckTime;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getCreaturePvpRegionCheckTime(void)
{
	return data->creaturePvpRegionCheckTime;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getDefaultLightArmorTemplate(void)
{
	return data->defaultLightArmorTemplate;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getDefaultMediumArmorTemplate(void)
{
	return data->defaultMediumArmorTemplate;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getDefaultHeavyArmorTemplate(void)
{
	return data->defaultHeavyArmorTemplate;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getArmorDamageReduction(void)
{
	return data->armorDamageReduction;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getStationPlayersEnabled(void)
{
	return data->stationPlayersEnabled;
}

// ----------------------------------------------------------------------


inline bool ConfigServerGame::getStationPlayersShowAllDebugInfo(void)
{
	return data->stationPlayersShowAllDebugInfo;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getCommoditiesMarketEnabled(void)
{
	return data->commoditiesMarketEnabled;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getCommoditiesServerServiceBindPort()
{
	return data->commoditiesServerServiceBindPort;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getCommoditiesServerServiceBindInterface()
{
	return data->commoditiesServerServiceBindInterface;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getCommoditiesShowAllDebugInfo(void)
{
	return data->commoditiesShowAllDebugInfo;
}

// ----------------------------------------------------------------------

inline time_t ConfigServerGame::getCommoditiesQueryTimeoutSec(void)
{
	return data->commoditiesQueryTimeoutSec;
}

//-----------------------------------------------------------------------

inline time_t ConfigServerGame::getCommoditiesServerReconnectIntervalSec(void)
{
	return data->commoditiesServerReconnectIntervalSec;
}

// ----------------------------------------------------------------------

inline time_t ConfigServerGame::getUnclaimedAuctionItemDestroyTimeSec(void)
{
	return data->unclaimedAuctionItemDestroyTimeSec;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMaxReimburseAmount()
{
	return data->maxReimburseAmount;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getLogBaselines(void)
{
	return data->logBaselines;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getDefaultVendorItemRestrictionFile()
{
	return data->defaultVendorItemRestrictionFile;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getDefaultVendorItemRestrictionRejectionMessage()
{
	return data->defaultVendorItemRestrictionRejectionMessage;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getWeatherUpdateSeconds(void)
{
	return data->weatherUpdateSeconds;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getInstrumentDataTableFilename(void)
{
	return data->instrumentDataTableFilename;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getPerformanceDataTableFilename(void)
{
	return data->performanceDataTableFilename;
}

//-----------------------------------------------------------------------

inline const int ConfigServerGame::getCreateQueueScheduleTime(void)
{
	return data->createQueueScheduleTime;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerGame::getDisableResources()
{
	return data->disableResources;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getXpMultiplier()
{
	return data->xpMultiplier;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getXpManagerTemplate()
{
	return data->xpManagerTemplate;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getGcwXpBonus()
{
	return data->gcwXpBonus;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerGame::getReportAiStateChanges(void)
{
	return data->reportAiStateChanges;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerGame::getReportAiWarnings(void)
{
	return data->reportAiWarnings;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerGame::getSendBreadcrumbs(void)
{
	return data->sendBreadcrumbs;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerGame::getMovementWhileRetreatingThrowsException(void)
{
	return data->movementWhileRetreatingThrowsException;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerGame::getHibernateEnabled(void)
{
	return data->hibernateEnabled;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerGame::getHibernateProxies(void)
{
	return data->hibernateProxies;
}

// ----------------------------------------------------------------------

inline const float ConfigServerGame::getHibernateDistance(void)
{
	return data->hibernateDistance;
}

// ----------------------------------------------------------------------

inline const float ConfigServerGame::getMinHibernateAlter(void)
{
	return data->minHibernateAlter;
}

// ----------------------------------------------------------------------

inline const float ConfigServerGame::getMaxHibernateAlter(void)
{
	return data->maxHibernateAlter;
}

// ----------------------------------------------------------------------

inline const int ConfigServerGame::getAiPulseQueuePerFrame()
{
	return data->aiPulseQueuePerFrame;
}

// ----------------------------------------------------------------------

inline const unsigned int ConfigServerGame::getAiPulseQueueMaxWaitTimeMs()
{
	return static_cast<unsigned int>(data->aiPulseQueueMaxWaitTimeMs);
}

// ----------------------------------------------------------------------

inline const int ConfigServerGame::getSecondsPerResourceTick()
{
	return data->secondsPerResourceTick;
}

// ----------------------------------------------------------------------

inline const uint32 ConfigServerGame::getResourceTimeScale()
{
	return static_cast<uint32>(data->resourceTimeScale);
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getDisableCreateQueue()
{
	return data->disableCreateQueue;
}

//-----------------------------------------------------------------------

inline const float ConfigServerGame::getMaxTeleportDistanceWithoutSceneWarp()
{
	return data->maxTeleportDistanceWithoutSceneWarp;
}

//-----------------------------------------------------------------------

inline const float ConfigServerGame::getMaxInstrumentPlayDistance()
{
	return data->maxInstrumentPlayDistance;
}

//-----------------------------------------------------------------------

inline const char * ConfigServerGame::getCharacterNameGeneratorDirectory(void)
{
	return data->characterNameGeneratorDirectory;
}

//-----------------------------------------------------------------------

inline const int ConfigServerGame::getMaxLotsPerAccount()
{
	return data->maxLotsPerAccount;
}

// ----------------------------------------------------------------------

inline const int ConfigServerGame::getUnsafeLogoutTimeMs()
{
	return data->unsafeLogoutTimeMs;
}

// ----------------------------------------------------------------------

inline const unsigned long ConfigServerGame::getIdleLogoutTimeSec()
{
	return static_cast<unsigned long>(data->idleLogoutTimeSec);
}

// ----------------------------------------------------------------------

inline const unsigned long ConfigServerGame::getIdleLogoutTimeAfterCharacterTransferSec()
{
	return static_cast<unsigned long>(data->idleLogoutTimeAfterCharacterTransferSec);
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getFallbackDefaultWeapon(void)
{
	return data->fallbackDefaultWeapon;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getLogObservers()
{
	return data->logObservers;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMissionRequestsPerFrame()
{
	return data->missionRequestsPerFrame;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getDisableMissions()
{
	return data->disableMissions;
}

// ----------------------------------------------------------------------

inline unsigned long ConfigServerGame::getLoopProfileLogThresholdMs()
{
	return static_cast<unsigned long>(data->loopProfileLogThresholdMs);
}

// ----------------------------------------------------------------------

inline unsigned long ConfigServerGame::getGroundLoopProfileLogThresholdMs()
{
	return static_cast<unsigned long>(data->groundLoopProfileLogThresholdMs);
}

// ----------------------------------------------------------------------

inline unsigned long ConfigServerGame::getSpaceLoopProfileLogThresholdMs()
{
	return static_cast<unsigned long>(data->spaceLoopProfileLogThresholdMs);
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getPvpDisableCombat()
{
	return data->pvpDisableCombat;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getLogObjectLoading()
{
	return data->logObjectLoading;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getEnforcePlayerTriggerVolumesOnly()
{
	return data->enforcePlayerTriggerVolumesOnly;	
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getLocationTargetCheckIntervalSec()
{
	return data->locationTargetCheckIntervalSec;
}

// ----------------------------------------------------------------------

inline const char * ConfigServerGame::getGameServiceBindInterface()
{
	return data->gameServiceBindInterface;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getLogPvpUpdates()
{
	return data->logPvpUpdates;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getServerSpawnLimit()
{
	return data->serverSpawnLimit;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getSpawnQueueSize()
{
	return data->spawnQueueSize;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getFramesPerSpawn()
{
	return data->framesPerSpawn;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableSpawningNearLoadBeacons()
{
	return data->enableSpawningNearLoadBeacons;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getNpcConversationDistance()
{
	//@todo: this value should actually come from the radial_menu datatable, 
	// CONVERSE_START entry
	return data->npcConversationDistance;
}

// ----------------------------------------------------------------------

inline const float ConfigServerGame::getWatchedHarvesterAlterTime()
{
	return data->watchedHarvesterAlterTime;
}

// ----------------------------------------------------------------------

inline const float ConfigServerGame::getUnWatchedHarvesterAlterTime()
{
	return data->unwatchedHarvesterAlterTime;
}

// ----------------------------------------------------------------------

inline const float ConfigServerGame::getHarvesterExtractionRateMultiplier()
{
	return data->harvesterExtractionRateMultiplier;
}

// ----------------------------------------------------------------------

inline const bool ConfigServerGame::getDeactivateHarvesterIfDamaged()
{
	return data->deactivateHarvesterIfDamaged;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getNonCriticalTasksPerFrame()
{
	return data->nonCriticalTasksPerFrame;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMaxMoney()
{
	return data->maxMoney;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMaxGalacticReserveDepositBillion()
{
	return data->maxGalacticReserveDepositBillion;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMaxMoneyTransfer()
{
	return data->maxMoneyTransfer;
}

//------------------------------------------------------------------------------------------

inline int ConfigServerGame::getMaxFreeTrialMoney()
{
	return data->maxFreeTrialMoney;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getEnablePreload()
{
	return data->enablePreload;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getBuildPreloadLists()
{
	return data->buildPreloadLists;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getLogAuthTransfer()
{
	return data->logAuthTransfer;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getOverrideUpdateRadius()
{
	return data->overrideUpdateRadius;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getBuildingUpdateRadiusMultiplier()
{
	return data->buildingUpdateRadiusMultiplier;
}

//------------------------------------------------------------------------------------------

inline int ConfigServerGame::getMaxPopulationForNewbieTravel()
{
	return data->maxPopulationForNewbieTravel;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getNumberOfMoveObjectLists()
{
	return data->numberOfMoveObjectLists;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMinNewbieTravelLocations()
{
	return data->minNewbieTravelLocations;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getDatabasePositionUpdateLongDelayIntervalMs()
{
	return data->databasePositionUpdateLongDelayIntervalMs;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getDatabasePositionUpdateShortDelayIntervalMs()
{
	return data->databasePositionUpdateShortDelayIntervalMs;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getLogPositionUpdates()
{
	return data->logPositionUpdates;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getCheckOriginCreates()
{
	return data->checkOriginCreates;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableWho()
{
	return data->enableWho;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getDebugGroupChat()
{
	return data->debugGroupChat;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getNumberOfMissionsWantedInMissionBag()
{
	return data->numberOfMissionsWantedInMissionBag;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMissionCreationQueueSize()
{
	return data->missionCreationQueueSize;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getConnectToAllGameServersTimeout()
{
	return data->connectToAllGameServersTimeout;
}

//------------------------------------------------------------------------------------------

inline int ConfigServerGame::getHouseItemLimitMultiplier()
{
	return data->houseItemLimitMultiplier;
}


inline int ConfigServerGame::getMaxHouseItemLimit()
{
	return data->maxHouseItemLimit;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getDemandLoadHouseContents()
{
	return data->demandLoadHouseContents;
}

//------------------------------------------------------------------------------------------

inline int ConfigServerGame::getUnloadHouseContentsTime()
{
	return data->unloadHouseContentsTime;
}

//------------------------------------------------------------------------------------------

inline int ConfigServerGame::getLoadHouseContentsDelaySeconds()
{
	return data->loadHouseContentsDelaySeconds;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getBoundarySpawningAllowed()
{
	return data->boundarySpawningAllowed;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getDebugMovement()
{
	return data->debugMovement;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getSkipUnreliableTransformsForOtherCells()
{
	return data->skipUnreliableTransformsForOtherCells;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMaxPlayerSanityCheckFailures()
{
	return data->maxPlayerSanityCheckFailures;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getDisablePlayerSanityChecker()
{
	return data->disablePlayerSanityChecker;
}

//-------------------------------------------------------------------

inline float ConfigServerGame::getUpdateRange7PacketsSquared()
{
	return data->updateRange7Packets * data->updateRange7Packets;
}

//-------------------------------------------------------------------

inline float ConfigServerGame::getUpdateRange6PacketsSquared()
{
	return data->updateRange6Packets * data->updateRange6Packets;
}

//-------------------------------------------------------------------

inline float ConfigServerGame::getUpdateRange5PacketsSquared()
{
	return data->updateRange5Packets * data->updateRange5Packets;
}

//-------------------------------------------------------------------

inline float ConfigServerGame::getUpdateRange4PacketsSquared()
{
	return data->updateRange4Packets * data->updateRange4Packets;
}

//-------------------------------------------------------------------

inline float ConfigServerGame::getUpdateRange3PacketsSquared()
{
	return data->updateRange3Packets * data->updateRange3Packets;
}

//-------------------------------------------------------------------

inline float ConfigServerGame::getUpdateRange2PacketsSquared()
{
	return data->updateRange2Packets * data->updateRange2Packets;
}

//-------------------------------------------------------------------

inline float ConfigServerGame::getUpdateRange1PacketsSquared()
{
	return data->updateRange1Packets * data->updateRange1Packets;
}

// ----------------------------------------------------------------------

inline const char *ConfigServerGame::getInstantDeleteListDataTableName()
{
	return data->instantDeleteListDataTableName;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getSendFrameProfilerInfo()
{
	return data->sendFrameProfilerInfo;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getLogAllCommands()
{
	return data->logAllCommands;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableDebugControllerMessageSpam()
{
	return data->enableDebugControllerMessageSpam;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getDebugPvp()
{
	return data->debugPvp;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getAllowMasterObjectCreation()
{
	return data->allowMasterObjectCreation;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getMountsEnabled()
{
	return data->mountsEnabled;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getMountsSanityCheckerEnabled()
{
	return data->mountsSanityCheckerEnabled;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getMountsSanityCheckerBroadcastEnabled()
{
	return data->mountsSanityCheckerBroadcastEnabled;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMountsSanityCheckerBroadcastLimit()
{
	return data->mountsSanityCheckerBroadcastLimit;
}

// ----------------------------------------------------------------------

inline const int ConfigServerGame::getUniverseCheckFrequencySeconds()
{
	return data->universeCheckFrequencySeconds;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getMoveSimEnabled ()
{
	return data->moveSimEnabled;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getExceptionOnObjVarZeroLengthArray()
{
	return data->exceptionOnObjVarZeroLengthArray;
}

inline bool ConfigServerGame::getTrackNonAuthoritativeObjvarSets()
{
	return data->trackNonAuthoritativeObjvarSets;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getTriggerVolumeWarpDistance()
{
	return data->triggerVolumeWarpDistance;
}

//-------------------------------------------------------------------

inline const NetworkId & ConfigServerGame::getMaxGoldNetworkId(void)
{
	return data->maxGoldNetworkId;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getFatalOnMovingGoldObject()
{
	return data->fatalOnMovingGoldObject;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getShipsEnabled()
{
	return data->shipsEnabled;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getLogoutTriggerEnabled()
{
	return data->logoutTriggerEnabled;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getBehaviorMaxAvoidanceStuckTime()
{
	return data->behaviorMaxAvoidanceStuckTime;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getBehaviorMaxAvoidancePersistenceFrameCount()
{
	return data->behaviorMaxAvoidancePersistenceFrameCount;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getVeteranRewardTradeInWaitPeriodSeconds()
{
	return data->veteranRewardTradeInWaitPeriodSeconds;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableVeteranRewards()
{
	return data->enableVeteranRewards;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableSceneGlobalData()
{
	        return data->enableSceneGlobalData;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableOneYearAnniversary()
{
	return data->enableOneYearAnniversary;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMinEntitledTime()
{
	return data->minEntitledTime;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getManufactureTimeOverride()
{
	return data->manufactureTimeOverride;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getTheaterCreationLimitMilliseconds()
{
	return data->theaterCreationLimitMilliseconds;
}

//------------------------------------------------------------

inline bool ConfigServerGame::getFatalOnGoldPobChange()
{
	return data->fatalOnGoldPobChange;
}

//------------------------------------------------------------

inline float ConfigServerGame::getMaxSmallCreatureHeight()
{
	return data->maxSmallCreatureHeight;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getSmallCreatureUpdateRadius()
{
	return data->smallCreatureUpdateRadius;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getMaxMediumCreatureHeight()
{
	return data->maxMediumCreatureHeight;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getMediumCreatureUpdateRadius()
{
	return data->mediumCreatureUpdateRadius;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getLargeCreatureUpdateRadius()
{
	return data->largeCreatureUpdateRadius;
}

inline int ConfigServerGame::getCoolDownPersistThresholdSeconds()
{
	return data->coolDownPersistThresholdSeconds;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getLoginAsBountyHunter()
{
	return data->loginAsBountyHunter;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getSendAsteroidExtents()
{
	return data->sendAsteroidExtents;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getCreateZoneObjects()
{
	return data->createZoneObjects;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getShipUpdatesPerSecond()
{
	return data->shipUpdatesPerSecond;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getDebugSpaceVisibilityManager()
{
	return data->debugSpaceVisibilityManager;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGroundFrameRateLimit()
{
	return data->groundFrameRateLimit;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getSpaceFrameRateLimit()
{
	return data->spaceFrameRateLimit;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::isSpaceAiLoggingEnabled()
{
	return data->aiLoggingEnabled;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::isAiLoggingEnabled()
{
	return data->aiLoggingEnabled;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getShipShotValidationEnabled()
{
	return data->shipShotValidationEnabled;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getSpaceAiEnemySearchesPerFrame()
{
	return data->spaceAiEnemySearchesPerFrame;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getGroundShouldSleep()
{
	return data->groundShouldSleep;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getSpaceShouldSleep()
{
	return data->spaceShouldSleep;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getShipMoveValidationEnabled()
{
	return data->shipMoveValidationEnabled;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getShipMoveCheckIntervalMs()
{
	return data->shipMoveCheckIntervalMs;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getShipMoveCheckFudgeTimeMs()
{
	return data->shipMoveCheckFudgeTimeMs;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getShipMoveCheckTolerance()
{
	return data->shipMoveCheckTolerance;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getShipMoveCheckTimeToleranceSeconds()
{
	return data->shipMoveCheckTimeToleranceSeconds;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getSpaceAiAvoidanceChecksPerFrame()
{
	return data->spaceAiAvoidanceChecksPerFrame;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getSpaceAiUsePathRefinement()
{
	return data->spaceAiUsePathRefinement;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::isAiClientDebugEnabled()
{
	return data->aiClientDebugEnabled;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getShipClientUpdateDebugEnabled()
{
	return data->shipClientUpdateDebugEnabled;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getSpacePlanetServerUpdatesPerFrame()
{
	return data->spacePlanetServerUpdatesPerFrame;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getShipLoadInvulnerableTimeWithoutClient()
{
	return data->shipLoadInvulnerableTimeWithoutClient;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getShipLoadInvulnerableTimeWithClient()
{
	return data->shipLoadInvulnerableTimeWithClient;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getCreatureLoadInvulnerableTimeWithoutClient()
{
	return data->creatureLoadInvulnerableTimeWithoutClient;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getCreatureLoadInvulnerableTimeWithClient()
{
	return data->creatureLoadInvulnerableTimeWithClient;
}

// ----------------------------------------------------------------------

inline unsigned long ConfigServerGame::getLineOfSightCacheDurationMs()
{
	return static_cast<unsigned long>(data->lineOfSightCacheDurationMs);
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getLineOfSightCacheMinHeight()
{
	return data->lineOfSightCacheMinHeight;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getLineOfSightLocationRoundValue()
{
	return data->lineOfSightLocationRoundValue;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getBuildoutAreaEditingEnabled()
{
	return data->buildoutAreaEditingEnabled;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getDebugFloorPathNodeCount()
{
	return data->debugFloorPathNodeCount;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGcwRecalcTimeDayOfWeek()
{
	return data->gcwRecalcTimeDayOfWeek;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGcwRecalcTimeHour()
{
	return data->gcwRecalcTimeHour;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGcwRecalcTimeMinute()
{
	return data->gcwRecalcTimeMinute;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGcwRecalcTimeSecond()
{
	return data->gcwRecalcTimeSecond;
}

// ----------------------------------------------------------------------

inline std::vector<std::pair<std::pair<int, int>, std::pair<int, int> > > const & ConfigServerGame::getGcwScoreDecayTime()
{
	return data->gcwScoreDecayTime;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getChatStatisticsReportIntervalSeconds()
{
	return data->chatStatisticsReportIntervalSeconds;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getChatSpamLimiterNumCharacters()
{
	return data->chatSpamLimiterNumCharacters;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getChatSpamLimiterIntervalMinutes()
{
	return data->chatSpamLimiterIntervalMinutes;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getChatSpamLimiterEnabledForFreeTrial()
{
	return data->chatSpamLimiterEnabledForFreeTrial;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getChatSpamNotifyPlayerWhenLimitedIntervalSeconds()
{
	return data->chatSpamNotifyPlayerWhenLimitedIntervalSeconds;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getPendingLoadRequestLimit()
{
	return data->pendingLoadRequestLimit;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableCityCitizenshipFixup()
{
	return data->enableCityCitizenshipFixup;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getCityCitizenshipInactivePackupStartTimeEpoch()
{
	return data->cityCitizenshipInactivePackupStartTimeEpoch;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getCityCitizenshipInactivePackupInactiveTimeSeconds()
{
	return data->cityCitizenshipInactivePackupInactiveTimeSeconds;
}

//-----------------------------------------------------------------------

inline const char *ConfigServerGame::getRetroactivePlayerCityCreationTimeFilename()
{
	return data->retroactivePlayerCityCreationTimeFilename;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getWardenSquelchDurationSeconds()
{
	return data->wardenSquelchDurationSeconds;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableWarden()
{
	return data->enableWarden;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableWardenCanSquelchSecuredLogin()
{
	return data->enableWardenCanSquelchSecuredLogin;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableWardenToDeputizeWarden()
{
	return data->enableWardenToDeputizeWarden;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getDisableCollectionServerFirstGrantForSecuredLogin()
{
	return data->disableCollectionServerFirstGrantForSecuredLogin;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getLocateStructureCommandIntervalSeconds()
{
	return data->locateStructureCommandIntervalSeconds;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getLocateVendorCommandIntervalSeconds()
{
	return data->locateVendorCommandIntervalSeconds;
}

//-----------------------------------------------------------------------

inline size_t ConfigServerGame::getCtsDefaultScriptDictionarySizeBytes()
{
	return static_cast<size_t>(data->ctsDefaultScriptDictionarySizeBytes);
}

//-----------------------------------------------------------------------

inline const char *ConfigServerGame::getRetroactiveCtsHistoryFilename()
{
	return data->retroactiveCtsHistoryFilename;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getAllowIgnoreFreeCtsTimeRestriction()
{
	return data->allowIgnoreFreeCtsTimeRestriction;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getGroupPickupPointApprovalRangeSquared()
{
	return data->groupPickupPointApprovalRangeSquared;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getGroupPickupPointTimeLimitSeconds()
{
	return data->groupPickupPointTimeLimitSeconds;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getGroupPickupTravelPlayerCityPercent()
{
	return data->groupPickupTravelPlayerCityPercent;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getGroupInviteInviterCombatRestriction()
{
	return data->groupInviteInviterCombatRestriction;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getGroupInviteInviteeCombatRestriction()
{
	return data->groupInviteInviteeCombatRestriction;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getGroupJoinInviterCombatRestriction()
{
	return data->groupJoinInviterCombatRestriction;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getGroupJoinInviteeCombatRestriction()
{
	return data->groupJoinInviteeCombatRestriction;
}

//-----------------------------------------------------------------------

inline size_t ConfigServerGame::getCharacterMatchMaxMatchCount()
{
	return static_cast<size_t>(data->characterMatchMaxMatchCount);
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableOccupyUnlockedSlotCommand()
{
	return data->enableOccupyUnlockedSlotCommand;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableVacateUnlockedSlotCommand()
{
	return data->enableVacateUnlockedSlotCommand;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableSwapUnlockedSlotCommand()
{
	return data->enableSwapUnlockedSlotCommand;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getOccupyVacateUnlockedSlotCommandCooldownSeconds()
{
	return data->occupyVacateUnlockedSlotCommandCooldownSeconds;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getAccountFeatureIdForMonthlyBoosterPack()
{
	return data->accountFeatureIdForMonthlyBoosterPack;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getAccountFeatureIdForTcgBetaAnnouncement()
{
	return data->accountFeatureIdForTcgBetaAnnouncement;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getLogEventObjectCreation()
{
	return data->logEventObjectCreation;
}

//-----------------------------------------------------------------------

inline bool ConfigServerGame::getLogEventObjectDestruction()
{
	return data->logEventObjectDestruction;
}

//-----------------------------------------------------------------------

inline float ConfigServerGame::getRegionFlagUpdateTimeSecs()
{
	return data->regionFlagUpdateTime;
}

// ----------------------------------------------------------------------

inline const char *ConfigServerGame::getHousePackupAccountListDataTableName()
{
	return data->housePackupAccountListDataTableName;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getPickDropAllRoomItemsNumberOfItemsPerRound()
{
	return data->pickDropAllRoomItemsNumberOfItemsPerRound;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableCovertImperialMercenary()
{
	return data->enableCovertImperialMercenary;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableOvertImperialMercenary()
{
	return data->enableOvertImperialMercenary;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableCovertRebelMercenary()
{
	return data->enableCovertRebelMercenary;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableOvertRebelMercenary()
{
	return data->enableOvertRebelMercenary;
}

inline int ConfigServerGame::getBroadcastGcwScoreToOtherGalaxiesIntervalSeconds()
{
	return data->broadcastGcwScoreToOtherGalaxiesIntervalSeconds;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getBroadcastGcwScoreToOtherGalaxies()
{
	return data->broadcastGcwScoreToOtherGalaxies;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getReceiveGcwScoreFromOtherGalaxies()
{
	return data->receiveGcwScoreFromOtherGalaxies;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getTrackGcwPercentileHistoryCount()
{
	return data->trackGcwPercentileHistoryCount;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getGcwFactionalPresenceDisableForSecuredLogin()
{
	return data->gcwFactionalPresenceDisableForSecuredLogin;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGcwFactionalPresenceMountedPct()
{
	return data->gcwFactionalPresenceMountedPct;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGcwFactionalPresenceLevelPct()
{
	return data->gcwFactionalPresenceLevelPct;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGcwFactionalPresenceGcwRankBonusPct()
{
	return data->gcwFactionalPresenceGcwRankBonusPct;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGcwFactionalPresenceAlignedCityBonusPct()
{
	return data->gcwFactionalPresenceAlignedCityBonusPct;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGcwFactionalPresenceAlignedCityRankBonusPct()
{
	return data->gcwFactionalPresenceAlignedCityRankBonusPct;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGcwFactionalPresenceAlignedCityAgeBonusPct()
{
	return data->gcwFactionalPresenceAlignedCityAgeBonusPct;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGcwGuildMinMembersForGcwRegionDefender()
{
	return data->gcwGuildMinMembersForGcwRegionDefender;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGcwRegionDefenderTotalBonusPct()
{
	return data->gcwRegionDefenderTotalBonusPct;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getGcwDaysRequiredForGcwRegionDefenderBonus()
{
	return data->gcwDaysRequiredForGcwRegionDefenderBonus;
}

//-------------------------------------------------------------------

inline bool ConfigServerGame::getCheckNotifyRegions()
{
	return data->checkNotifyRegions;
}

//-------------------------------------------------------------------

inline bool ConfigServerGame::getAllowRegionTriggerOverride()
{
	return data->allowRegionTriggerOverride;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getHyperspaceRandomOffsetRange()
{
	return data->hyperspaceRandomOffsetRange;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getSpawnAllResources()
{
	return data->spawnAllResources;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getDebugAllAreasOverpopulated()
{
	return data->debugAllAreasOverpopulated;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMaxWaypointsPerCharacter()
{
	return data->maxWaypointsPerCharacter;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getVeteranDebugTriggerAll()
{
	return data->veteranDebugTriggerAll;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getVeteranDebugEnableOverrideAccountAge()
{
	return data->veteranDebugEnableOverrideAccountAge;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getEnableNewVeteranRewards()
{
	return data->enableNewVeteranRewards;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getBuddyPointTimeBonus()
{
	return data->buddyPointTimeBonus;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getLastKnownLocationsResetTimeSec()
{
	return data->lastKnownLocationsResetTimeSec;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMaxMessageTosPerObjectPerFrame()
{
	return data->maxMessageTosPerObjectPerFrame;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMaxMessageToTimePerObjectPerFrame()
{
	return data->maxMessageToTimePerObjectPerFrame;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getMaxMessageToBounces()
{
	return data->maxMessageToBounces;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getAiBaseAggroRadius()
{
	return data->aiBaseAggroRadius;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getAiMaxAggroRadius()
{
	return data->aiMaxAggroRadius;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getAiLeashRadius()
{
	return data->aiLeashRadius;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getAiAssistRadius()
{
	return data->aiAssistRadius;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getPopulationReportTimeSeconds()
{
	return data->populationReportTimeSeconds;
}

// ----------------------------------------------------------------------

inline float ConfigServerGame::getMaxCombatRange()
{
	return data->maxCombatRange;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getBlockBuildRegionPlacement()
{
	return data->blockBuildRegionPlacement;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getNpeMinInstancePopulation()
{
	return data->npeMinInstancePopulation;
}

// ----------------------------------------------------------------------

inline int ConfigServerGame::getNpeMaxInstancePopulation()
{
	return data->npeMaxInstancePopulation;
}

// ----------------------------------------------------------------------

inline bool ConfigServerGame::getStripNonFreeAssetsFromPlayersInTutorial()
{
	return data->stripNonFreeAssetsFromPlayersInTutorial;
}

inline bool ConfigServerGame::getUseOldSuidGenerator() {
	return data->useOldSuidGenerator;
}

// ----------------------------------------------------------------------

inline const char *ConfigServerGame::getServerLoadLevel()
{
	return data->serverLoadLevel;
}

//-----------------------------------------------------------------------

inline int ConfigServerGame::getMaxHousingLots(void)
{
	return data->maxHousingLots;
}

#endif
