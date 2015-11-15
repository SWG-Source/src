//===================================================================
//
// ConfigSharedCollision.h
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_ConfigSharedCollision_H
#define INCLUDED_ConfigSharedCollision_H

class CrcLowerString;
class Object;

typedef bool (*PlayEffectHook)(CrcLowerString const & effectName, Object * object, CrcLowerString const & hardpoint );
typedef bool (*IsPlayerHouseHook)(Object const * object);

//===================================================================

class ConfigSharedCollision
{
public:

	static void install ();

	static bool getDrawFloors               ( void );
	static bool getDrawExtents              ( void );
	static bool getDrawCollisionNormals     ( void );
	static bool getDrawSphereTrees          ( void );
	static bool getDrawPathNodes            ( void );
	static bool getDrawBoxTrees             ( void );
	static bool getDrawCMeshes              ( void );
	static bool getDrawFootprints           ( void );
	static bool getDrawCreatureLengths      ( void );

	static void setDrawFloors               ( bool draw );
	static void setDrawExtents              ( bool draw );
	static void setDrawCollisionNormals     ( bool draw );
	static void setDrawSphereTrees          ( bool draw );
	static void setDrawPathNodes            ( bool draw );
	static void setDrawBoxTrees             ( bool draw );
	static void setDrawCMeshes              ( bool draw );
	static void setDrawFootprints           ( bool draw );
	static void setDrawCreatureLengths      ( bool draw );

	static bool getEnableStaticCollision    ( void );
	static bool getEnableFloraCollision     ( void );
	static bool getEnableFloorCollision     ( void );
	static bool getEnableCollisionResolve   ( void );
	static bool getEnableCollisionFlags     ( void );
	static bool getEnableCollisionValidate  ( void );
	static bool getEnableRemoteCollision    ( void );
	static bool getEnableTestCode           ( void );
	static bool getEnableCreatureCollision  ( void ); // If enabled, the player will collide with mobs
	static bool getEnableIntraFrameTests    ( void );
	static bool getEnableMountRadii         ( void );
	static bool getBuildBoxTrees            ( void );
	static bool getBuildDebugData           ( void );
	static void setBuildDebugData           ( bool val );

	static void setEnableCreatureCollision  ( bool enable );

	static bool getReportStatus             ( void ); // Spam that goes to the debug window - # of objects in collision world, profiling
	static bool getReportChanges            ( void ); // Objects entering/leaving the collision world
	static bool getReportMessages           ( void ); // Debugging message spam
	static bool getReportWarnings           ( void ); // Possible errors in the collision system
	static bool getReportEvents             ( void ); // Game-relevant events - objects colliding
	static bool getReportFloating           ( void ); // whether objects are floating, when they start/stop floating
	static bool getLogLongFrames            ( void );
	static bool getIgnoreTerrainLos         ( void );
	static bool getGenerateTerrainLos       ( void );
	static bool getShoveEnabled             ( void );

	static float getWallEpsilon             ( void );
	static float getAreaEpsilon             ( void );
	static float getHopHeight               ( void );
	static float getTerrainLOSMinDistance   ( void );
	static float getTerrainLOSMaxDistance   ( void );
	static float getLosUprightScale         ( void );
	static float getLosProneScale           ( void );

	// ----------

	static void              setPlayEffectHook    ( PlayEffectHook hook );
	static PlayEffectHook    getPlayEffectHook    ( void );

	static void              setIsPlayerHouseHook ( IsPlayerHouseHook hook );
	static IsPlayerHouseHook getIsPlayerHouseHook ( void );

	// SpatialSweepAndResolveDefaultMask deals with SpatialDatabase::Query masks
	// the default is SpatialDatabase::Q_Static
	static int getSpatialSweepAndResolveDefaultMask();
	static void setSpatialSweepAndResolveDefaultMask(int queryMask);

	static bool isSpaceAiLoggingEnabled();
	static bool useOriginalAvoidanceAlgorithm();

private:

	ConfigSharedCollision ();
	ConfigSharedCollision (const ConfigSharedCollision&);
	ConfigSharedCollision& operator= (const ConfigSharedCollision&);
};

//===================================================================

#endif


