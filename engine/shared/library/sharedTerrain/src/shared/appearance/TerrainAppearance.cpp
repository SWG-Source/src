//===================================================================
//
// TerrainAppearance.cpp
// asommers 7-6-99
//
// copyright 1999, bootprint entertainment
// copyright 2001, sony online entertainment
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/TerrainAppearance.h"

#include "sharedCollision/CollisionInfo.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectList.h"

//===================================================================
// PUBLIC TerrainAppearance
//===================================================================

TerrainAppearance::TerrainAppearance (const AppearanceTemplate* newAppearanceTemplate) : 
	Appearance (newAppearanceTemplate),
	m_referenceObjectList (NON_NULL (new ObjectList (10)))
{
}

//-------------------------------------------------------------------
	
TerrainAppearance::~TerrainAppearance ()
{
	delete m_referenceObjectList;
}

//-------------------------------------------------------------------

bool TerrainAppearance::collide(Vector const & /*start_o*/, Vector const & /*end_o*/, CollideParameters const & /*collideParameters*/, CollisionInfo & /*result*/) const
{
	return false;
} 

//-------------------------------------------------------------------

bool TerrainAppearance::collideObjects (const Vector& /*start_o*/, const Vector& /*end_o*/, CollisionInfo& /*result*/) const
{
	return false;
} 

//-------------------------------------------------------------------

bool TerrainAppearance::approximateCollideObjects (const Vector& /*start_o*/, const Vector& /*end_o*/, CollisionInfo& /*result*/) const
{
	return false;
} 

//-------------------------------------------------------------------

bool TerrainAppearance::getPauseEnvironment () const
{
	return false;
}

//-------------------------------------------------------------------

void TerrainAppearance::setPauseEnvironment (bool const /*pauseEnvironment*/)
{
}

//-------------------------------------------------------------------

float TerrainAppearance::getEnvironmentCycleTime () const
{
	return 0.f;
}

//-------------------------------------------------------------------

const PackedRgb TerrainAppearance::getClearColor () const
{
	return PackedRgb::solidBlack;
}

//-------------------------------------------------------------------

const PackedRgb TerrainAppearance::getFogColor () const
{
	return PackedRgb::solidBlack;
}

//-------------------------------------------------------------------

float TerrainAppearance::getFogDensity () const
{
	return 0.f;
}

//-------------------------------------------------------------------

void TerrainAppearance::getTime (int& /*hour*/, int& /*minute*/) const
{
}

//-------------------------------------------------------------------

float TerrainAppearance::getTime () const
{
	return 0.f;
}

//-------------------------------------------------------------------

void TerrainAppearance::setTime (float /*time*/, bool /*force*/)
{
}

//-------------------------------------------------------------------

bool TerrainAppearance::isDay () const
{
	return true;
}

//----------------------------------------------------------------------

bool TerrainAppearance::isTimeLocked() const
{
	return false;
}

//-------------------------------------------------------------------

bool TerrainAppearance::getHeight (const Vector& /*position_o*/, float& /*height*/) const
{
	return false;
} 

//-------------------------------------------------------------------

bool TerrainAppearance::getHeight (const Vector& /*position_o*/, float& /*height*/, Vector& /*normal*/) const
{
	return false;
} 

//-------------------------------------------------------------------

bool TerrainAppearance::getHeightForceChunkCreation (const Vector& /*position_o*/, float& /*height*/) const
{
	return false;
}

//-------------------------------------------------------------------

const ObjectTemplate* TerrainAppearance::getSurfaceProperties (const Vector& /*position_o*/) const
{
	return 0;
}

//-------------------------------------------------------------------

int TerrainAppearance::getTerrainType (const Vector& /*position_o*/) const
{
	return 0;
} 

//-----------------------------------------------------------------

bool TerrainAppearance::getWaterHeight (const Vector& /*position_o*/, float& /*height*/) const
{
	return false;
} 

//-----------------------------------------------------------------

bool TerrainAppearance::getWaterHeight (const Vector& /*position_o*/, float& /*height*/, TerrainGeneratorWaterType& /*waterType*/, bool /*ignoreNonTransparentWater*/) const
{
	return false;
} 

//-----------------------------------------------------------------

TerrainGeneratorWaterType TerrainAppearance::getWaterType (const Vector& /*position_w*/) const
{
	return TGWT_invalid;
}

//-----------------------------------------------------------------

bool TerrainAppearance::getWater (const Rectangle2d& /*rectangle*/) const
{
	return false;
}

//-------------------------------------------------------------------

bool TerrainAppearance::getSlope (const Rectangle2d& /*rectangle*/) const
{
	return false;
}

//----------------------------------------------------------------------

bool TerrainAppearance::isPassable(Vector const & /*position_w*/) const
{
	return true;
}

//----------------------------------------------------------------------

bool TerrainAppearance::isPassableForceChunkCreation(Vector const & /*position_w*/) const
{
	return true;
}

//----------------------------------------------------------------------

bool TerrainAppearance::hasPassableAffectors() const
{
	return false;
}

//-------------------------------------------------------------------

bool TerrainAppearance::getWater (const int /*chunkX*/, const int /*chunkZ*/) const
{
	return false;
}

//-------------------------------------------------------------------

bool TerrainAppearance::getSlope (const int /*chunkX*/, const int /*chunkZ*/) const
{
	return false;
}

//-------------------------------------------------------------------

float TerrainAppearance::getMapWidthInMeters () const
{
	return 0.f;
}

//-------------------------------------------------------------------

float TerrainAppearance::getChunkWidthInMeters () const
{
	return 0.f;
}

//-------------------------------------------------------------------

int TerrainAppearance::getNumberOfChunks () const
{
	return 0;
}

//-------------------------------------------------------------------

bool TerrainAppearance::hasHighLevelOfDetailTerrain (const Vector& /*position_o*/) const
{
	return true;
}

//-------------------------------------------------------------------

int TerrainAppearance::calculateChunkX (float) const
{
	return 0;
}

//-------------------------------------------------------------------

int TerrainAppearance::calculateChunkZ (float) const
{
	return 0;
}

//-------------------------------------------------------------------

float TerrainAppearance::getChunkHeight (const int /*chunkX*/, const int /*chunkZ*/) const
{
	return 0.f;
}

//-------------------------------------------------------------------

const BoxExtent* TerrainAppearance::getChunkExtent (const Vector& /*position_o*/) const
{
	return 0;
}

//-------------------------------------------------------------------

const BoxExtent* TerrainAppearance::getChunkExtentForceChunkCreation (const Vector& /*position_o*/) const
{
	return 0;
}

//-------------------------------------------------------------------

bool TerrainAppearance::collideForceChunkCreation(Vector const & /*start_o*/, Vector const & /*end_o*/, CollisionInfo & /*result*/)
{
	return false;
}

//-------------------------------------------------------------------

void TerrainAppearance::preRender (const Camera* /*camera*/) const
{
}

//-----------------------------------------------------------------

void TerrainAppearance::postRender () const
{
}

//-----------------------------------------------------------------

void TerrainAppearance::addClearCollidableFloraObject (const Object* const /*object*/, const Vector& /*position_w*/, const float /*radius*/)
{
}

//-------------------------------------------------------------------

void TerrainAppearance::removeClearCollidableFloraObject (const Object* const /*object*/)
{
}

//-------------------------------------------------------------------

void TerrainAppearance::addClearNonCollidableFloraObject (const Object* const /*object*/, const ClearFloraEntryList& /*clearFloraEntryList*/)
{
}

//-------------------------------------------------------------------

void TerrainAppearance::removeClearNonCollidableFloraObject (const Object* const /*object*/)
{
}

//-------------------------------------------------------------------

float TerrainAppearance::getHighLevelOfDetailThreshold () const
{
	return 1.f;
}

//-------------------------------------------------------------------

void TerrainAppearance::setHighLevelOfDetailThreshold (const float /*highLevelOfDetailThreshold*/)
{
}

//-------------------------------------------------------------------

float TerrainAppearance::getLevelOfDetailThreshold () const
{
	return 1.f;
}

//-------------------------------------------------------------------

void TerrainAppearance::setLevelOfDetailThreshold (const float /*levelOfDetailThreshold*/)
{
}

//-------------------------------------------------------------------

void TerrainAppearance::getPolygonSoup (const Rectangle2d& /*extent2d_o*/, IndexedTriangleList& /*indexedTriangleList*/) const
{
}

//-------------------------------------------------------------------

void TerrainAppearance::invalidateRegion (const Rectangle2d& /*extent2d*/)
{
}

//-------------------------------------------------------------------

void TerrainAppearance::addReferenceObject (const Object* const object)
{
	m_referenceObjectList->addObject (const_cast<Object*> (object));
}

//-------------------------------------------------------------------

void TerrainAppearance::removeReferenceObject (const Object* const object)
{
	m_referenceObjectList->removeObject (const_cast<Object*> (object));
}

//-------------------------------------------------------------------

void TerrainAppearance::removeAllReferenceObjects ()
{
	m_referenceObjectList->removeAll ();
}

//-------------------------------------------------------------------

bool TerrainAppearance::isReferenceObject (const Object* const object) const
{
	return m_referenceObjectList->find (object);
}

//-------------------------------------------------------------------

void TerrainAppearance::drawExtents (const Vector& /*position_o*/) const
{
}

//-------------------------------------------------------------------

void TerrainAppearance::debugDump () const
{
#ifdef _DEBUG
	DEBUG_REPORT_PRINT (true, ("-- TerrainAppearance\n"));

	DEBUG_REPORT_PRINT (true, ("numberOfReferenceObjects = %i\n", getNumberOfReferenceObjects ()));

	if (getNumberOfReferenceObjects ())
	{
		const Object* const object = getReferenceObject (0);
		if (object)
		{
			const Vector position_w = object->getPosition_w ();
			DEBUG_REPORT_PRINT (true, ("referenceObject0Position = <%1.2f, %1.2f, %1.2f>\n", position_w.x, position_w.y, position_w.z));
		}
	}
#endif
}

//-------------------------------------------------------------------

void TerrainAppearance::purgeChunks()
{
}

//===================================================================
// PRIVATE TerrainAppearance
//===================================================================

int TerrainAppearance::getNumberOfReferenceObjects () const
{
	return m_referenceObjectList->getNumberOfObjects ();
}

//-------------------------------------------------------------------

const Object* TerrainAppearance::getReferenceObject (const int index) const
{
	return m_referenceObjectList->getObject (index);
}

//===================================================================
