//===================================================================
//
// TerrainObject.cpp
// asommers 3-1-99
//
// copyright 1999, bootprint entertainment
// copyright 2001, sony online entertainment
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/TerrainObject.h"

#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/Sphere.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedTerrain/TerrainAppearance.h"

#include <string>
#include <vector>

//===================================================================

namespace
{
	inline const TerrainAppearance* getCastedAppearance (const Object* const object)
	{
		NOT_NULL (object);
		return safe_cast<const TerrainAppearance*> (object->getAppearance ());
	}

	inline TerrainAppearance* getCastedAppearance (Object* const object)
	{
		NOT_NULL (object);
		return safe_cast<TerrainAppearance*> (object->getAppearance ());
	}

	const Vector2d ms_invalidPosition_w (FLT_MAX, FLT_MAX);
	bool           ms_debugReport;

	typedef std::vector<TerrainObject::TerrainChangedFunction> TerrainChangedFunctionList;
	TerrainChangedFunctionList ms_terrainChangedFunctionList;
}

//===================================================================
// STATIC PUBLIC TerrainObject
//===================================================================

TerrainObject* TerrainObject::ms_instance;
bool           TerrainObject::ms_useCache = true;

//===================================================================

void TerrainObject::install ()
{
	DebugFlags::registerFlag (ms_debugReport, "SharedTerrain", "debugReport", debugDump);

	ExitChain::add (remove, "TerrainObject::remove");
}

//-------------------------------------------------------------------

void TerrainObject::remove ()
{
	DebugFlags::unregisterFlag (ms_debugReport);
}

//-------------------------------------------------------------------

TerrainObject* TerrainObject::getInstance ()
{
	return ms_instance;
}

//-------------------------------------------------------------------

const TerrainObject* TerrainObject::getConstInstance ()
{
	return ms_instance;
}

//-------------------------------------------------------------------

void TerrainObject::setUseCache (bool useCache)
{
	ms_useCache = useCache;
}

//-------------------------------------------------------------------

void TerrainObject::addTerrainChangedFunction (TerrainObject::TerrainChangedFunction terrainChangedFunction)
{
	ms_terrainChangedFunctionList.push_back (terrainChangedFunction);
}

//-------------------------------------------------------------------

void TerrainObject::terrainChanged (Rectangle2d const & extent2d)
{
	TerrainChangedFunctionList::iterator end = ms_terrainChangedFunctionList.end ();
	for (TerrainChangedFunctionList::iterator iter = ms_terrainChangedFunctionList.begin (); iter != end; ++iter)
		(*iter) (extent2d);
}

//-------------------------------------------------------------------

void TerrainObject::debugDump ()
{
	if (ms_instance)
		getCastedAppearance (ms_instance)->debugDump ();
}

//===================================================================
// PUBLIC TerrainObject
//===================================================================

TerrainObject::TerrainObject () :
	Object (),
	m_cachedPosition_w (ms_invalidPosition_w),
	m_cachedPositionHeight (0.f),
	m_cachedPositionNormal_w (ms_invalidPosition_w),
	m_cachedPositionNormalHeight (0.f),
	m_cachedPositionNormalNormal_w (Vector::unitY),
	m_cachedPositionWater_w (ms_invalidPosition_w),
	m_cachedPositionWaterHeight (0.f),
	m_cachedPositionWaterType(TGWT_invalid)
{
	DEBUG_FATAL (ms_instance, ("TerrainObject instance already exists"));
	ms_instance = this;
}

//-------------------------------------------------------------------

TerrainObject::TerrainObject (const ObjectNotification &notification) :
	Object (),
	m_cachedPosition_w (ms_invalidPosition_w),
	m_cachedPositionHeight (0.f),
	m_cachedPositionNormal_w (ms_invalidPosition_w),
	m_cachedPositionNormalHeight (0.f),
	m_cachedPositionNormalNormal_w (Vector::unitY),
	m_cachedPositionWater_w (ms_invalidPosition_w),
	m_cachedPositionWaterHeight (0.f),
	m_cachedPositionWaterType(TGWT_invalid)
{
	DEBUG_FATAL (ms_instance, ("TerrainObject instance already exists"));
	ms_instance = this;
	addNotification (notification);
}

//-------------------------------------------------------------------

TerrainObject::~TerrainObject ()
{
		//remove from world
	if (isInWorld ())
		TerrainObject::removeFromWorld ();
	
	DEBUG_FATAL (ms_instance != this, ("TerrainObject instance is not this object"));
	ms_instance = nullptr;
}

//-------------------------------------------------------------------

bool TerrainObject::placeObject (Object& object, bool alignToTerrain, bool forceChunkCreation) const
{
	// @todo ALS handle align to terrain
	UNREF (alignToTerrain);

	//-- make sure the object is both in the world and in the world cell
	if (!object.isInWorld () || !object.isInWorldCell ())
	{
		DEBUG_WARNING (true, ("TerrainObject::placeObject - object is not in world or not in world cell (id=%s, template=%s)", object.getNetworkId ().getValueString ().c_str (), object.getObjectTemplateName ()));
		return false;
	}

	const Vector& position_w = object.getPosition_w ();

	float height;

	if (forceChunkCreation)
	{
		if (getHeightForceChunkCreation (position_w, height))
		{
			object.setPosition_w (Vector (position_w.x, height, position_w.z));
			return true;
		}
	}
	else
	{
		if (getHeight (position_w, height))
		{
			object.setPosition_w (Vector (position_w.x, height, position_w.z));
			return true;
		}
	}

	DEBUG_WARNING (true, ("TerrainObject::placeObject (id=%s, template=%s) failed for %s location <%1.2f, %1.2f>", object.getNetworkId ().getValueString ().c_str (), object.getObjectTemplateName (), forceChunkCreation ? "forced" : "non-forced", position_w.x, position_w.z));
	return false;
}

//-------------------------------------------------------------------

bool TerrainObject::getLogicalHeight (const Vector& position_w, const float swimHeight, float& logicalHeight, float& realHeight) const
{
	if (getHeight (position_w, realHeight))
	{
		float waterHeight;
		if (getWaterHeight (position_w, waterHeight) && (waterHeight - swimHeight > realHeight))
			logicalHeight = waterHeight - swimHeight;
		else
			logicalHeight = realHeight;

		return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool TerrainObject::getLogicalHeight (const Vector& position_w, const float swimHeight, float& logicalHeight, float& realHeight, Vector& normal) const
{
	if (getHeight (position_w, realHeight, normal))
	{
		float waterHeight;
		if (getWaterHeight (position_w, waterHeight) && (waterHeight - swimHeight > realHeight))
			logicalHeight = waterHeight - swimHeight;
		else
			logicalHeight = realHeight;

		return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool TerrainObject::getHeight (const Vector& position_w, float& height) const
{
	const Vector2d position (position_w.x, position_w.z);

	if (ms_useCache && position == m_cachedPosition_w)
	{
		height = m_cachedPositionHeight;

		return true;
	}

	const bool result = getCastedAppearance (this)->getHeight (rotateTranslate_w2o (position_w), height);

	if (result)
	{
		m_cachedPosition_w     = position;
		m_cachedPositionHeight = height;
	}
	else
		m_cachedPosition_w = ms_invalidPosition_w;

	return result;
}

//-------------------------------------------------------------------

bool TerrainObject::getHeight (const Vector& position_w, float& height, Vector& normal) const
{
	const Vector2d position (position_w.x, position_w.z);

	if (ms_useCache && position == m_cachedPositionNormal_w)
	{
		height = m_cachedPositionNormalHeight;
		normal = m_cachedPositionNormalNormal_w;

		return true;
	}

	const bool result = getCastedAppearance (this)->getHeight (rotateTranslate_w2o (position_w), height, normal);

	if (result)
	{
		normal = rotate_o2w (normal);

		m_cachedPositionNormal_w       = position;
		m_cachedPositionNormalHeight   = height;
		m_cachedPositionNormalNormal_w = normal;
	}
	else
		m_cachedPositionNormal_w = ms_invalidPosition_w;

	return result;
}

//-------------------------------------------------------------------

bool TerrainObject::getHeightForceChunkCreation (const Vector& position_w, float& height) const
{
	return getCastedAppearance (this)->getHeightForceChunkCreation (rotateTranslate_w2o (position_w), height);
}

//-------------------------------------------------------------------

const ObjectTemplate* TerrainObject::getSurfaceProperties (const Vector& position_w) const
{
	return getCastedAppearance (this)->getSurfaceProperties (rotateTranslate_w2o (position_w));
}

//-------------------------------------------------------------------

int TerrainObject::getTerrainType (const Vector &position_w) const
{
	return getCastedAppearance (this)->getTerrainType (rotateTranslate_w2o (position_w));
}

//-------------------------------------------------------------------

bool TerrainObject::getWaterHeight (const Vector& position_w, float& height) const
{
	TerrainGeneratorWaterType waterType;
	return getWaterHeight(position_w,height,waterType);
}

//-------------------------------------------------------------------

bool TerrainObject::getWaterHeight (const Vector& position_w, float& height, TerrainGeneratorWaterType& waterType, bool ignoreNonTransparentWater) const
{
	const Vector2d position (position_w.x, position_w.z);

	if (ms_useCache && position == m_cachedPositionWater_w)
	{
		height = m_cachedPositionWaterHeight;
		waterType = m_cachedPositionWaterType;
		return true;
	}

	const bool result = getCastedAppearance (this)->getWaterHeight (rotateTranslate_w2o (position_w), height, waterType, ignoreNonTransparentWater);

	if (result)
	{
		m_cachedPositionWater_w     = position;
		m_cachedPositionWaterHeight = height;
		m_cachedPositionWaterType = waterType;
	}
	else
		m_cachedPositionWater_w = ms_invalidPosition_w;

	return result;
}

//-------------------------------------------------------------------

TerrainGeneratorWaterType TerrainObject::getWaterType (const Vector& position_w) const
{
	const TerrainGeneratorWaterType ret = getCastedAppearance (this)->getWaterType (rotateTranslate_w2o (position_w));
	return ret;
}

//-------------------------------------------------------------------

bool TerrainObject::isBelowWater (const Vector& position_w) const
{
	float waterHeight;
	return getWaterHeight (position_w, waterHeight) && (position_w.y < waterHeight);
}

//-------------------------------------------------------------------

bool TerrainObject::isBelowWater (const Vector& position_w, TerrainGeneratorWaterType& waterType, bool ignoreNonTransparentWater) const
{
	float waterHeight;
	return getWaterHeight (position_w, waterHeight, waterType, ignoreNonTransparentWater) && (position_w.y < waterHeight);
}

//-------------------------------------------------------------------

bool TerrainObject::getWater (const Rectangle2d& rectangle) const
{
	return getCastedAppearance (this)->getWater (rectangle);
}

//-------------------------------------------------------------------

bool TerrainObject::getSlope (const Rectangle2d& rectangle) const
{
	return getCastedAppearance (this)->getSlope (rectangle);
}

//-------------------------------------------------------------------

bool TerrainObject::getWater (const int chunkX, const int chunkZ) const
{
	return getCastedAppearance (this)->getWater (chunkX, chunkZ);
}

//-------------------------------------------------------------------

bool TerrainObject::getSlope (const int chunkX, const int chunkZ) const
{
	return getCastedAppearance (this)->getSlope (chunkX, chunkZ);
}

//-------------------------------------------------------------------

float TerrainObject::getChunkWidthInMeters () const
{
	return getCastedAppearance (this)->getChunkWidthInMeters ();
}

//-------------------------------------------------------------------

float TerrainObject::getMapWidthInMeters () const
{
	return getCastedAppearance (this)->getMapWidthInMeters ();
}

//-------------------------------------------------------------------

float TerrainObject::getMaximumValidHeightInMeters () const
{
	return ConfigSharedTerrain::getMaximumValidHeightInMeters ();
}

//-------------------------------------------------------------------

int TerrainObject::getNumberOfChunks () const
{
	return getCastedAppearance (this)->getNumberOfChunks ();
}

//-------------------------------------------------------------------

bool TerrainObject::hasHighLevelOfDetailTerrain (const Vector& position_w) const
{
	return getCastedAppearance (this)->hasHighLevelOfDetailTerrain (rotateTranslate_w2o (position_w));
}

//-------------------------------------------------------------------

int TerrainObject::calculateChunkX (const Vector& position_w) const
{
	return getCastedAppearance (this)->calculateChunkX (rotateTranslate_w2o (position_w).x);
}

//-------------------------------------------------------------------

int TerrainObject::calculateChunkZ (const Vector& position_w) const
{
	return getCastedAppearance (this)->calculateChunkZ (rotateTranslate_w2o (position_w).z);
}

//-------------------------------------------------------------------

float TerrainObject::getChunkHeight (const int chunkX, const int chunkZ) const
{
	return getCastedAppearance (this)->getChunkHeight (chunkX, chunkZ);
}

//-------------------------------------------------------------------

const BoxExtent* TerrainObject::getChunkExtent (const Vector& position_w) const
{
	return getCastedAppearance (this)->getChunkExtent (rotateTranslate_w2o (position_w));
}

//-------------------------------------------------------------------

const BoxExtent* TerrainObject::getChunkExtentForceChunkCreation (const Vector& position_w) const
{
	return getCastedAppearance (this)->getChunkExtentForceChunkCreation (rotateTranslate_w2o (position_w));
}

//-------------------------------------------------------------------

bool TerrainObject::collide (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const
{
	if (getCastedAppearance(this)->collide(rotateTranslate_w2o(start_w), rotateTranslate_w2o(end_w), CollideParameters::cms_default, result))
	{
		result.setObject (this);
		result.setPoint (rotateTranslate_o2w (result.getPoint ()));
		result.setNormal (rotate_o2w (result.getNormal ()));

		return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool TerrainObject::collideForceChunkCreation (Vector const & start_w, Vector const & end_w, CollisionInfo & result)
{
	return getCastedAppearance(this)->collideForceChunkCreation(rotateTranslate_w2o(start_w), rotateTranslate_w2o(end_w), result);
}

//-------------------------------------------------------------------

bool TerrainObject::collideObjects (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const
{
	if (getCastedAppearance (this)->collideObjects (rotateTranslate_w2o (start_w), rotateTranslate_w2o (end_w), result))
	{
		result.setPoint (rotateTranslate_o2w (result.getPoint ()));
		result.setNormal (rotate_o2w (result.getNormal ()));

		return true;
	}

	return false;
}

//-------------------------------------------------------------------

bool TerrainObject::approximateCollideObjects (const Vector& start_w, const Vector& end_w, CollisionInfo& result) const
{
	if (getCastedAppearance (this)->approximateCollideObjects (rotateTranslate_w2o (start_w), rotateTranslate_w2o (end_w), result))
	{
		result.setPoint (rotateTranslate_o2w (result.getPoint ()));
		result.setNormal (rotate_o2w (result.getNormal ()));

		return true;
	}

	return false;
}

//-------------------------------------------------------------------

void TerrainObject::preRender (const Camera* const camera) const
{
	getCastedAppearance (this)->preRender (camera);
}

//-------------------------------------------------------------------

void TerrainObject::postRender () const
{
	getCastedAppearance (this)->postRender ();
}

//-------------------------------------------------------------------

void TerrainObject::addReferenceObject (const Object* const referenceObject)
{
	getCastedAppearance (this)->addReferenceObject (referenceObject);
}

//-------------------------------------------------------------------

int TerrainObject::getNumberOfReferenceObjects() const
{
	return getCastedAppearance (this)->getNumberOfReferenceObjects();
}

//-------------------------------------------------------------------

void TerrainObject::removeReferenceObject (const Object* const referenceObject)
{
	getCastedAppearance (this)->removeReferenceObject (referenceObject);
}

//-------------------------------------------------------------------

void TerrainObject::removeAllReferenceObjects ()
{
	getCastedAppearance (this)->removeAllReferenceObjects ();
}

//-------------------------------------------------------------------

bool TerrainObject::isReferenceObject (const Object* const referenceObject) const
{
	return getCastedAppearance (this)->isReferenceObject (referenceObject);
}

//-------------------------------------------------------------------

bool TerrainObject::getPauseEnvironment () const
{
	return getCastedAppearance (this)->getPauseEnvironment ();
}

//-------------------------------------------------------------------

void TerrainObject::setPauseEnvironment (bool const pauseEnvironment)
{
	getCastedAppearance (this)->setPauseEnvironment (pauseEnvironment);
}

//-------------------------------------------------------------------

float TerrainObject::getEnvironmentCycleTime () const
{
	return getCastedAppearance (this)->getEnvironmentCycleTime ();
}

//-------------------------------------------------------------------

const PackedRgb TerrainObject::getClearColor () const
{
	return getCastedAppearance (this)->getClearColor ();
}

//-------------------------------------------------------------------

const PackedRgb TerrainObject::getFogColor () const
{
	return getCastedAppearance (this)->getFogColor ();
}

//-------------------------------------------------------------------

float TerrainObject::getFogDensity () const
{
	return getCastedAppearance (this)->getFogDensity ();
}

//-------------------------------------------------------------------

void TerrainObject::getTime (int& hour, int& minute) const
{
	getCastedAppearance (this)->getTime (hour, minute);
}

//-------------------------------------------------------------------

float TerrainObject::getTime () const
{
	return getCastedAppearance (this)->getTime ();
}

//-------------------------------------------------------------------

void TerrainObject::setTime (const float time, const bool force)
{
	getCastedAppearance (this)->setTime (time, force);
}

//-------------------------------------------------------------------

bool TerrainObject::isDay () const
{
	return getCastedAppearance (this)->isDay ();
}

//----------------------------------------------------------------------

bool TerrainObject::isTimeLocked() const
{
	return getCastedAppearance(this)->isTimeLocked();
}

//-------------------------------------------------------------------

void TerrainObject::addClearCollidableFloraObject (const Object* const object, const Vector& position_w, const float radius)
{
	getCastedAppearance (this)->addClearCollidableFloraObject (object, position_w, radius);
}

//-------------------------------------------------------------------

void TerrainObject::removeClearCollidableFloraObject (const Object* const object)
{
	getCastedAppearance (this)->removeClearCollidableFloraObject (object);
}

//-------------------------------------------------------------------

void TerrainObject::addClearNonCollidableFloraObject (const Object* const object, const ClearFloraEntryList& clearFloraEntryList)
{
	getCastedAppearance (this)->addClearNonCollidableFloraObject (object, clearFloraEntryList);
}

//-------------------------------------------------------------------

void TerrainObject::removeClearNonCollidableFloraObject (const Object* const object)
{
	getCastedAppearance (this)->removeClearNonCollidableFloraObject (object);
}

//-------------------------------------------------------------------

float TerrainObject::getHighLevelOfDetailThreshold ()
{
	TerrainObject * const to = getInstance ();
	if (to)
		return getCastedAppearance (to)->getHighLevelOfDetailThreshold ();

	return 0.0f;
}

//-------------------------------------------------------------------

void TerrainObject::setHighLevelOfDetailThreshold (const float highLevelOfDetailThreshold)
{
	TerrainObject * const to = getInstance ();
	if (to)
		getCastedAppearance (to)->setHighLevelOfDetailThreshold (highLevelOfDetailThreshold);
}

//-------------------------------------------------------------------

float TerrainObject::getLevelOfDetailThreshold ()
{
	TerrainObject * const to = getInstance ();
	if (to)
		return getCastedAppearance (to)->getLevelOfDetailThreshold ();

	return 1.0f;
}

//-------------------------------------------------------------------

void TerrainObject::setLevelOfDetailThreshold (const float levelOfDetailThreshold)
{
	TerrainObject * const to = getInstance ();
	if (to)
		getCastedAppearance (to)->setLevelOfDetailThreshold (levelOfDetailThreshold);
}

//-------------------------------------------------------------------

void TerrainObject::drawExtents (const Vector& position_w) const
{
	getCastedAppearance (this)->drawExtents (rotateTranslate_w2o (position_w));
}

// ----------------------------------------------------------------------
/**
 * Test whether a world x,z position falls within the boundaries of 
 * the terrain map.
 *
 * @param position_w  a world-space position to test.
 *
 * @return  true if the given position falls within the boundaries 
 * of the terrain map; false otherwise.
 */

bool TerrainObject::isWithinTerrainBoundaries (Vector const &position_w) const
{
	//-- check x and z
	float const maxCoordinate = getMapWidthInMeters () * 0.5f;
	if ((std::abs (position_w.x) > maxCoordinate) || (std::abs (position_w.z) > maxCoordinate))
		return false;

	//-- check y
	if (std::abs (position_w.y) > getMaximumValidHeightInMeters ())
		return false;

	return true;
}

//----------------------------------------------------------------------

bool TerrainObject::isPassable(Vector const & position_w) const
{
	return getCastedAppearance(this)->isPassable(position_w);
}

//----------------------------------------------------------------------

bool TerrainObject::isPassableForceChunkCreation(Vector const & position_w) const
{
	return getCastedAppearance(this)->isPassableForceChunkCreation(position_w);
}

//-------------------------------------------------------------------

void TerrainObject::getPolygonSoup (const Rectangle2d& extent2d_w, IndexedTriangleList& indexedTriangleList) const
{
	getCastedAppearance (this)->getPolygonSoup (extent2d_w, indexedTriangleList);
}

//-------------------------------------------------------------------

void TerrainObject::invalidateRegion (const Rectangle2d& extent2d)
{
	getCastedAppearance (this)->invalidateRegion (extent2d);
}

//-------------------------------------------------------------------

void TerrainObject::purgeChunks()
{
	getCastedAppearance(this)->purgeChunks();
}

//----------------------------------------------------------------------

bool TerrainObject::hasPassableAffectors() const
{
	return getCastedAppearance(this)->hasPassableAffectors();
}

//===================================================================
