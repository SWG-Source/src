//===================================================================
//
// TerrainAppearance.h
// asommers 7-6-99
//
// copyright 1999, bootprint entertainment
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_TerrainAppearance_H
#define INCLUDED_TerrainAppearance_H

//===================================================================

#include "sharedMath/PackedRgb.h"
#include "sharedObject/Appearance.h"
#include "sharedTerrain/TerrainGeneratorType.h"

class Camera;
class ObjectList;
class ObjectTemplate;
class Rectangle2d;

//===================================================================

class TerrainAppearance : public Appearance
{
public:

	explicit TerrainAppearance (const AppearanceTemplate* newAppearanceTemplate);
	virtual ~TerrainAppearance ();

	//-- Appearance
	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;

	//-- TerrainAppearance
	virtual bool                  getHeight (const Vector& position_o, float& height) const;
	virtual bool                  getHeight (const Vector& position_o, float& height, Vector& normal) const;
	virtual bool                  getHeightForceChunkCreation (const Vector& position_o, float& height) const;
	virtual const ObjectTemplate* getSurfaceProperties (const Vector& position_o) const;
	virtual int                   getTerrainType (const Vector& position_o) const;
	virtual bool                  getWaterHeight (const Vector& position_o, float& height) const;
	virtual bool				  getWaterHeight (const Vector& position_o, float& height, TerrainGeneratorWaterType& waterType, bool ignoreNonTransparentWater=false) const;
	virtual TerrainGeneratorWaterType getWaterType (const Vector& position_w) const;
	virtual bool                  getWater (const Rectangle2d& rectangle) const;
	virtual bool                  getSlope (const Rectangle2d& rectangle) const;
	virtual bool                  getWater (int chunkX, int chunkZ) const;
	virtual bool                  getSlope (int chunkX, int chunkZ) const;
	virtual bool                  isPassable(Vector const & position_w) const;
	virtual bool                  isPassableForceChunkCreation(Vector const & position_w) const;
	virtual bool                  hasPassableAffectors() const;

	virtual float                 getMapWidthInMeters () const;
	virtual float                 getChunkWidthInMeters () const;
	virtual int                   getNumberOfChunks () const;
	virtual bool                  hasHighLevelOfDetailTerrain (const Vector& position_o) const;
	virtual int                   calculateChunkX (float positionX_o) const;
	virtual int                   calculateChunkZ (float positionZ_o) const;
	virtual float                 getChunkHeight (int chunkX, int chunkZ) const;
	virtual const BoxExtent*      getChunkExtent (const Vector& position_o) const;
	virtual const BoxExtent*      getChunkExtentForceChunkCreation (const Vector& position_o) const;
	virtual bool                  collideForceChunkCreation(Vector const & start_o, Vector const & end_o, CollisionInfo & result);

	virtual void                  preRender (const Camera* camera) const;
	virtual void                  postRender () const;

	virtual bool                  collideObjects (const Vector& start_o, const Vector& end_o, CollisionInfo& result) const;
	virtual bool                  approximateCollideObjects (const Vector& start_o, const Vector& end_o, CollisionInfo& result) const;

	virtual bool                  getPauseEnvironment () const;
	virtual void                  setPauseEnvironment (bool pauseEnvironment);
	virtual float                 getEnvironmentCycleTime () const;
	virtual const PackedRgb       getClearColor () const;
	virtual const PackedRgb       getFogColor () const;
	virtual float                 getFogDensity () const;
	virtual void                  getTime (int& hour, int& minute) const;
	virtual float                 getTime () const;
	virtual void                  setTime (float time, bool force);
	virtual bool                  isDay () const;
	virtual bool                  isTimeLocked() const;

	typedef std::vector<std::pair<Vector, float> > ClearFloraEntryList;
	virtual void                  addClearCollidableFloraObject (const Object* object, const Vector& position_o, float radius);
	virtual void                  removeClearCollidableFloraObject (const Object* object);
	virtual void                  addClearNonCollidableFloraObject (const Object* object, const ClearFloraEntryList& clearFloraEntryList);
	virtual void                  removeClearNonCollidableFloraObject (const Object* object);

	virtual float                 getHighLevelOfDetailThreshold () const;
	virtual void                  setHighLevelOfDetailThreshold (float highLevelOfDetailThreshold);
	virtual float                 getLevelOfDetailThreshold () const;
	virtual void                  setLevelOfDetailThreshold (float levelOfDetailThreshold);

	virtual void                  getPolygonSoup (const Rectangle2d& extent2d_o, IndexedTriangleList& indexedTriangleList) const;

	virtual void                  invalidateRegion (const Rectangle2d& extent2d);

	void                          addReferenceObject (const Object* object);
	int                           getNumberOfReferenceObjects () const;
	void                          removeReferenceObject (const Object* object);
	void                          removeAllReferenceObjects ();
	bool                          isReferenceObject (const Object* object) const;

	//-- debugging
	virtual void                  drawExtents (const Vector& position_o) const;
	virtual void                  debugDump () const;
	virtual void                  purgeChunks();

protected:

	const Object* getReferenceObject (int index) const;

private:

	TerrainAppearance ();
	TerrainAppearance (const TerrainAppearance&);
	TerrainAppearance& operator= (const TerrainAppearance&);

private:

	ObjectList* const m_referenceObjectList;
};

//===================================================================

#endif
