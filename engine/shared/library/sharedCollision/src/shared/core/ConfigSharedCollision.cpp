//===================================================================
//
// ConfigSharedCollision.cpp
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/ConfigSharedCollision.h"

#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/SpatialDatabase.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Production.h"
#include "sharedDebug/DebugFlags.h"

//===================================================================

namespace ConfigSharedCollisionNamespace
{

bool    ms_drawFloors               = false;
bool    ms_drawExtents              = false;
bool    ms_drawCollisionNormals     = false;
bool    ms_drawSphereTrees          = false;
bool    ms_drawPathNodes            = false;
bool    ms_drawBoxTrees             = false;
bool    ms_drawCMeshes              = false;
bool    ms_drawFootprints           = false;
bool    ms_drawCreatureLengths      = false;

bool    ms_enableStaticCollision    = true;
bool    ms_enableFloraCollision     = true;
bool    ms_enableFloorCollision     = true;
bool    ms_enableCollisionResolve   = true;
bool    ms_enableCollisionFlags     = false;
bool    ms_enableCollisionValidate  = false;
bool    ms_enableRemoteCollision    = false;
bool    ms_enableTestCode           = false;
bool    ms_enableCreatureCollision  = true;
bool    ms_enableIntraFrameTests    = true;
bool    ms_enableMountRadii         = false;
bool    ms_buildBoxTrees            = false;
bool    ms_buildDebugData           = false;

bool    ms_updateStatus             = true;
bool    ms_reportStatus             = false;
bool    ms_reportChanges            = false;
bool    ms_reportMessages           = false;
#if PRODUCTION == 0
bool    ms_reportWarnings           = true;
#else
bool    ms_reportWarnings           = false;
#endif
bool    ms_reportEvents             = false;
bool    ms_reportFloating           = false;
bool    ms_logLongFrames            = false;
bool    ms_ignoreTerrainLos         = true;
bool    ms_generateTerrainLos       = false;
bool    ms_shoveEnabled             = true;

float    ms_wallEpsilon              = 0.01f;        // the distance that we keep the footprints away from the walls
float    ms_areaEpsilon              = 0.000001f;    // One square millimeter
float    ms_hopHeight                = 0.30f;        // how high a floor can be off the terrain and still allow us to hop up onto it
float    ms_terrainLOSMinDistance    = 4.0f;
float    ms_terrainLOSMaxDistance    = 64.0f;
float    ms_losUprightScale          = 1.5f;
float    ms_losProneScale            = 1.0f;

PlayEffectHook    ms_playEffectHook    = nullptr;
IsPlayerHouseHook ms_isPlayerHouseHook = nullptr;

	int ms_spatialSweepAndResolveDefaultMask = static_cast<int>(SpatialDatabase::Q_Static);

	bool ms_spaceAiLoggingEnabled = false;
	bool ms_useOriginalAvoidanceAlgorithm = false;
}

using namespace ConfigSharedCollisionNamespace;

// ----------------------------------------------------------------------

void ConfigSharedCollision::install ( void )
{
#if PRODUCTION == 0

	DebugFlags::registerFlag( ms_drawFloors,					"SharedCollision",    "drawFloors");
	DebugFlags::registerFlag( ms_drawExtents,					"SharedCollision",    "drawExtents");
	DebugFlags::registerFlag( ms_drawCollisionNormals,          "SharedCollision",    "drawCollisionNormals");
	DebugFlags::registerFlag( ms_drawSphereTrees,				"SharedCollision",    "drawSphereTrees");
	DebugFlags::registerFlag( ms_drawPathNodes,					"SharedCollision",    "drawPathNodes");
	DebugFlags::registerFlag( ms_drawBoxTrees,					"SharedCollision",    "drawBoxTrees");
	DebugFlags::registerFlag( ms_drawCMeshes,					"SharedCollision",    "drawCMeshes");
	DebugFlags::registerFlag( ms_drawFootprints,				"SharedCollision",    "drawFootprints");
	DebugFlags::registerFlag( ms_drawCreatureLengths,			"SharedCollision",    "drawCreatureLengths");
																
																
	DebugFlags::registerFlag( ms_enableStaticCollision,			"SharedCollision",    "enableStaticCollision");
	DebugFlags::registerFlag( ms_enableFloraCollision,			"SharedCollision",    "enableFloraCollision");
	DebugFlags::registerFlag( ms_enableFloorCollision,			"SharedCollision",    "enableFloorCollision");
	DebugFlags::registerFlag( ms_enableCollisionResolve,		"SharedCollision",    "enableCollisionResolve");
	DebugFlags::registerFlag( ms_enableCollisionFlags,			"SharedCollision",    "enableCollisionFlags");
	DebugFlags::registerFlag( ms_enableCollisionValidate,		"SharedCollision",    "enableCollisionValidate");
	DebugFlags::registerFlag( ms_enableTestCode,				"SharedCollision",    "enableTestCode");
	DebugFlags::registerFlag( ms_enableCreatureCollision,		"SharedCollision",    "enableCreatureCollision");
	DebugFlags::registerFlag( ms_enableIntraFrameTests,			"SharedCollision",    "enableIntraFrameTests");
	DebugFlags::registerFlag( ms_enableMountRadii,				"SharedCollision",    "enableMountRadii");
	DebugFlags::registerFlag( ms_buildBoxTrees,					"SharedCollision",    "buildBoxTrees");
	DebugFlags::registerFlag( ms_buildDebugData,				"SharedCollision",    "buildDebugData");
																
	DebugFlags::registerFlag( ms_updateStatus,					"SharedCollision",    "updateStatus", CollisionWorld::reportCallback);
	DebugFlags::registerFlag( ms_reportStatus,					"SharedCollision",    "reportStatus");
	DebugFlags::registerFlag( ms_reportChanges,					"SharedCollision",    "reportChanges");
	DebugFlags::registerFlag( ms_reportMessages,				"SharedCollision",    "reportMessages");
	DebugFlags::registerFlag( ms_reportWarnings,				"SharedCollision",    "reportWarnings");
	DebugFlags::registerFlag( ms_reportEvents,					"SharedCollision",    "reportEvents");
	DebugFlags::registerFlag( ms_reportFloating,				"SharedCollision",    "reportFloating");
	DebugFlags::registerFlag( ms_logLongFrames,					"SharedCollision",    "logLongFrames");
	DebugFlags::registerFlag( ms_shoveEnabled,					"SharedCollision",    "0x4F474F67");  // hash of shoveEnabled
	DebugFlags::registerFlag( ms_spaceAiLoggingEnabled,			"SharedCollision",    "spaceAiLoggingEnabled");
	DebugFlags::registerFlag( ms_useOriginalAvoidanceAlgorithm,	"SharedCollision",    "useOriginalAvoidanceAlgorithm");
	
	// ----------

#endif

	// the following options should be read regardless of production mode
	DebugFlags::registerFlag( ms_ignoreTerrainLos,          "SharedCollision",    "ignoreTerrainLos");
	DebugFlags::registerFlag( ms_generateTerrainLos,        "SharedCollision",    "generateTerrainLos");

	ms_wallEpsilon           = ConfigFile::getKeyFloat("SharedCollision", "wallEpsilon",           ms_wallEpsilon);
	ms_areaEpsilon           = ConfigFile::getKeyFloat("SharedCollision", "areaEpsilon",           ms_areaEpsilon);
	ms_hopHeight             = ConfigFile::getKeyFloat("SharedCollision", "hopHeight",             ms_hopHeight);
	ms_terrainLOSMinDistance = ConfigFile::getKeyFloat("SharedCollision", "terrainLOSMinDistance", ms_terrainLOSMinDistance);
	ms_terrainLOSMaxDistance = ConfigFile::getKeyFloat("SharedCollision", "terrainLOSMaxDistance", ms_terrainLOSMaxDistance);
}

// ----------------------------------------------------------------------

bool ConfigSharedCollision::getDrawFloors               ( void ) { return ms_drawFloors; }
bool ConfigSharedCollision::getDrawExtents              ( void ) { return ms_drawExtents; }
bool ConfigSharedCollision::getDrawCollisionNormals     ( void ) { return ms_drawCollisionNormals; }
bool ConfigSharedCollision::getDrawSphereTrees          ( void ) { return ms_drawSphereTrees; }
bool ConfigSharedCollision::getDrawPathNodes            ( void ) { return ms_drawPathNodes; }
bool ConfigSharedCollision::getDrawBoxTrees             ( void ) { return ms_drawBoxTrees; }
bool ConfigSharedCollision::getDrawCMeshes              ( void ) { return ms_drawCMeshes; }
bool ConfigSharedCollision::getDrawFootprints           ( void ) { return ms_drawFootprints; }
bool ConfigSharedCollision::getDrawCreatureLengths      ( void ) { return ms_drawCreatureLengths; }


void ConfigSharedCollision::setDrawFloors               ( bool draw ) { ms_drawFloors = draw; }
void ConfigSharedCollision::setDrawExtents              ( bool draw ) { ms_drawExtents = draw; }
void ConfigSharedCollision::setDrawCollisionNormals     ( bool draw ) { ms_drawCollisionNormals = draw; }
void ConfigSharedCollision::setDrawSphereTrees          ( bool draw ) { ms_drawSphereTrees = draw; }
void ConfigSharedCollision::setDrawPathNodes            ( bool draw ) { ms_drawPathNodes = draw; }
void ConfigSharedCollision::setDrawBoxTrees             ( bool draw ) { ms_drawBoxTrees = draw; }
void ConfigSharedCollision::setDrawCMeshes              ( bool draw ) { ms_drawCMeshes = draw; }
void ConfigSharedCollision::setDrawFootprints           ( bool draw ) { ms_drawFootprints = draw; }
void ConfigSharedCollision::setDrawCreatureLengths      ( bool draw ) { ms_drawCreatureLengths = draw; }


bool ConfigSharedCollision::getEnableStaticCollision    ( void ) { return ms_enableStaticCollision; }
bool ConfigSharedCollision::getEnableFloraCollision     ( void ) { return ms_enableFloraCollision; }
bool ConfigSharedCollision::getEnableFloorCollision     ( void ) { return ms_enableFloorCollision; }
bool ConfigSharedCollision::getEnableCollisionResolve   ( void ) { return ms_enableCollisionResolve; }
bool ConfigSharedCollision::getEnableCollisionFlags     ( void ) { return ms_enableCollisionFlags; }
bool ConfigSharedCollision::getEnableCollisionValidate  ( void ) { return ms_enableCollisionValidate; }
bool ConfigSharedCollision::getEnableRemoteCollision    ( void ) { return ms_enableRemoteCollision; }
bool ConfigSharedCollision::getEnableTestCode           ( void ) { return ms_enableTestCode; }
bool ConfigSharedCollision::getEnableCreatureCollision  ( void ) { return ms_enableCreatureCollision; }
bool ConfigSharedCollision::getEnableIntraFrameTests    ( void ) { return ms_enableIntraFrameTests; }
bool ConfigSharedCollision::getEnableMountRadii         ( void ) { return ms_enableMountRadii; }
bool ConfigSharedCollision::getBuildBoxTrees            ( void ) { return ms_buildBoxTrees; }
bool ConfigSharedCollision::getBuildDebugData           ( void ) { return ms_buildDebugData; }
void ConfigSharedCollision::setBuildDebugData           (bool val)
{
	ms_buildDebugData = val;
}


bool ConfigSharedCollision::getReportStatus             ( void ) { return ms_reportStatus; }
bool ConfigSharedCollision::getReportChanges            ( void ) { return ms_reportChanges; }
bool ConfigSharedCollision::getReportMessages           ( void ) { return ms_reportMessages; }
bool ConfigSharedCollision::getReportWarnings           ( void ) { return ms_reportWarnings; }
bool ConfigSharedCollision::getReportEvents             ( void ) { return ms_reportEvents; }
bool ConfigSharedCollision::getReportFloating           ( void ) { return ms_reportFloating; }
bool ConfigSharedCollision::getLogLongFrames            ( void ) { return ms_logLongFrames; }
bool ConfigSharedCollision::getIgnoreTerrainLos         ( void ) { return ms_ignoreTerrainLos; }
bool ConfigSharedCollision::getGenerateTerrainLos       ( void ) { return ms_generateTerrainLos; }
bool ConfigSharedCollision::getShoveEnabled             ( void ) { return ms_shoveEnabled; }


float ConfigSharedCollision::getWallEpsilon             ( void ) { return ms_wallEpsilon; }
float ConfigSharedCollision::getAreaEpsilon             ( void ) { return ms_areaEpsilon; }
float ConfigSharedCollision::getHopHeight               ( void ) { return ms_hopHeight; }
float ConfigSharedCollision::getTerrainLOSMinDistance   ( void ) { return ms_terrainLOSMinDistance; }
float ConfigSharedCollision::getTerrainLOSMaxDistance   ( void ) { return ms_terrainLOSMaxDistance; }
float ConfigSharedCollision::getLosUprightScale         ( void ) { return ms_losUprightScale; }
float ConfigSharedCollision::getLosProneScale           ( void ) { return ms_losProneScale; }

bool ConfigSharedCollision::isSpaceAiLoggingEnabled() { return ms_spaceAiLoggingEnabled; }
bool ConfigSharedCollision::useOriginalAvoidanceAlgorithm() { return ms_useOriginalAvoidanceAlgorithm; }

// ----------

void ConfigSharedCollision::setPlayEffectHook ( PlayEffectHook hook ) { ms_playEffectHook = hook; }
PlayEffectHook ConfigSharedCollision::getPlayEffectHook ( void ) { return ms_playEffectHook; }

void ConfigSharedCollision::setIsPlayerHouseHook ( IsPlayerHouseHook hook ) { ms_isPlayerHouseHook = hook; }
IsPlayerHouseHook ConfigSharedCollision::getIsPlayerHouseHook ( void) { return ms_isPlayerHouseHook; }

int ConfigSharedCollision::getSpatialSweepAndResolveDefaultMask()
{
	return 	ms_spatialSweepAndResolveDefaultMask;
}

void ConfigSharedCollision::setSpatialSweepAndResolveDefaultMask(int queryMask)
{
	ms_spatialSweepAndResolveDefaultMask = queryMask;
}

