//===================================================================
//
// TerrainObject.h
// asommers 3-1-99
//
// copyright 1999, bootprint entertainment
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_TerrainObject_H
#define INCLUDED_TerrainObject_H

//===================================================================

#include "sharedMath/PackedRgb.h"
#include "sharedMath/Vector2d.h"
#include "sharedObject/Object.h"
#include "sharedTerrain/TerrainGeneratorType.h"

class BoxExtent;
class CollisionInfo;
class IndexedTriangleList;
class Rectangle2d;
class TerrainAppearance;

//===================================================================

class TerrainObject : public Object
{
public:

	static void                 install ();

	static TerrainObject*       getInstance ();
	static const TerrainObject* getConstInstance ();

	static void                 setUseCache (bool useCache);

	typedef void (*TerrainChangedFunction) (Rectangle2d const & extent2d);
	static void                 addTerrainChangedFunction (TerrainChangedFunction terrainChangedFunction);
	static void                 terrainChanged (Rectangle2d const & extent2d);

public:

	TerrainObject ();
	TerrainObject (const ObjectNotification &notification);
	virtual ~TerrainObject ();

	bool                  placeObject (Object& object, bool alignToTerrain=false, bool forceChunkCreation=false) const;

	bool                  getLogicalHeight (const Vector& position_w, float waterHeight, float& logicalHeight, float& realHeight) const;
	bool                  getLogicalHeight (const Vector& position_w, float waterHeight, float& logicalHeight, float& realHeight, Vector& normal) const;
	bool                  getHeight (const Vector& position_w, float& height) const;
	bool                  getHeight (const Vector& position_w, float& height, Vector& normal) const;
	bool                  getHeightForceChunkCreation (const Vector& position_w, float& height) const;
	const ObjectTemplate* getSurfaceProperties (const Vector& position_w) const;
	int                   getTerrainType (const Vector& position_w) const;
	bool                  getWaterHeight (const Vector& position_w, float& height) const;
	TerrainGeneratorWaterType getWaterType (const Vector& position_w) const;
	bool                  isBelowWater (const Vector& position_w) const;
	bool				  isBelowWater (const Vector& position_w, TerrainGeneratorWaterType& waterType, bool ignoreNonTransparentWater=false) const;
	bool                  getWaterHeight (const Vector& position_w, float& height, TerrainGeneratorWaterType& waterType, bool ignoreNonTransparenetWater=false) const;
	bool                  getWater (const Rectangle2d& rectangle) const;
	bool                  getSlope (const Rectangle2d& rectangle) const;
	bool                  getWater (int chunkX, int chunkZ) const;
	bool                  getSlope (int chunkX, int chunkZ) const;

	float                 getMapWidthInMeters () const;
	float                 getMaximumValidHeightInMeters () const;
	float                 getChunkWidthInMeters () const;
	int                   getNumberOfChunks () const;
	bool                  hasHighLevelOfDetailTerrain (const Vector& position_w) const;
	int                   calculateChunkX (const Vector& position_w) const;
	int                   calculateChunkZ (const Vector& position_w) const;
	float                 getChunkHeight (int chunkX, int chunkZ) const;
	const BoxExtent*      getChunkExtent (const Vector& position_w) const;
	const BoxExtent*      getChunkExtentForceChunkCreation (const Vector& position_w) const;

	bool                  collide (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const;
	bool                  collideForceChunkCreation (const Vector& start_w, const Vector& end_w, CollisionInfo& result);
	bool                  collideObjects (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const;
	bool                  approximateCollideObjects (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const;

	void                  preRender (const Camera* camera) const;
	void                  postRender () const;

	void                  addReferenceObject (const Object* object);
	int                   getNumberOfReferenceObjects() const;
	void                  removeReferenceObject (const Object* object);
	void                  removeAllReferenceObjects ();
	bool                  isReferenceObject (const Object* object) const;

	bool                  getPauseEnvironment () const;
	void                  setPauseEnvironment (bool pauseEnvironment);
	float                 getEnvironmentCycleTime () const;
	const PackedRgb       getClearColor () const;
	const PackedRgb       getFogColor () const;
	float                 getFogDensity () const;
	void                  getTime (int& hour, int& minute) const;
	float                 getTime () const;
	void                  setTime (float time, bool force=false);
	bool                  isDay () const;
	bool                  isTimeLocked() const;

	typedef std::vector<std::pair<Vector, float> > ClearFloraEntryList;
	void                  addClearCollidableFloraObject (const Object* object, const Vector& position_w, float radius);
	void                  removeClearCollidableFloraObject (const Object* object);
	void                  addClearNonCollidableFloraObject (const Object* object, const ClearFloraEntryList& clearFloraEntryList);
	void                  removeClearNonCollidableFloraObject (const Object* object);

	static float          getHighLevelOfDetailThreshold ();
	static void           setHighLevelOfDetailThreshold (float highLevelOfDetailThreshold);
	static float          getLevelOfDetailThreshold ();
	static void           setLevelOfDetailThreshold (float levelOfDetailThreshold);

	void                  getPolygonSoup (const Rectangle2d& extent2d_w, IndexedTriangleList& indexedTriangleList) const;

	void                  invalidateRegion (const Rectangle2d& extent2d);

	//-- debugging
	void                  drawExtents (const Vector& position_w) const;
	void                  purgeChunks();

//	void  writeChunkData (const Vector& position_w) const;
//	bool  placeObject (Object& object, const Vector& position_w, bool alignToTerrain=false) const;
//	bool  placeObject (Object& object, const Vector& position_w, const Vector& up_w) const;

	bool                  isWithinTerrainBoundaries (Vector const &position_w) const;
	bool                  isPassable(Vector const & position_w) const;
	bool                  isPassableForceChunkCreation(Vector const & position_w) const;

	bool                  hasPassableAffectors() const;

private:

	static void remove ();

	static void debugDump ();

private:

	TerrainObject (const TerrainObject&);
	TerrainObject& operator= (const TerrainObject&);

private:

	static TerrainObject* ms_instance;
	static bool           ms_useCache;

private:

	mutable Vector2d m_cachedPosition_w;
	mutable float    m_cachedPositionHeight;
	mutable Vector2d m_cachedPositionNormal_w;
	mutable float    m_cachedPositionNormalHeight;
	mutable Vector   m_cachedPositionNormalNormal_w;
	mutable Vector2d m_cachedPositionWater_w;
	mutable float    m_cachedPositionWaterHeight;
	mutable TerrainGeneratorWaterType m_cachedPositionWaterType;
};

//===================================================================

#endif
