//===================================================================
//
// ProceduralTerrainAppearance.cpp
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"

#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/ExtentList.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#ifndef WIN32
#include "sharedLog/Log.h" // for LOG()
#endif
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/World.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedTerrain/CoordinateHash.h"
#include "sharedUtility/BakedTerrain.h"
#include "sharedUtility/FileName.h"
#include "sharedRandom/RandomGenerator.h"
#include "sharedRandom/FastRandomGenerator.h"

#include <algorithm>
#include <map>
#include <vector>

#include <assert.h>

#ifndef WIN32
#include <string> // for LOG()
#endif

#define FLORA_ALLOWED_DISTANCE (8.0f)

namespace
{
	bool ms_logFloraCreation;
	bool ms_logGetHeightFailures;
	int  ms_maximumNumberOfChunksAllowed;
}

//===================================================================
//
// ProceduralTerrainAppearance::CreateChunkData
//

ProceduralTerrainAppearance::CreateChunkData::CreateChunkData (TerrainGenerator::CreateChunkBuffer* newCreateChunkBuffer) :
	createChunkBuffer (newCreateChunkBuffer),
	chunkX (0),
	chunkZ (0),
	numberOfTilesPerChunk (0),
	chunkWidthInMeters (0),
	tileWidthInMeters (0),
	start (),
	originOffset(0),
	numberOfPoles(0),
	shaderGroup (0),
	floraGroup (0),
	radialGroup (0),
	environmentGroup (0),
	fractalGroup (0),
	bitmapGroup (0)
{
	hasLargerNeighborFlags=0;
}

//-------------------------------------------------------------------

ProceduralTerrainAppearance::CreateChunkData::~CreateChunkData ()
{
	createChunkBuffer = 0;

	floraGroup  = 0;
	shaderGroup = 0;
	radialGroup = 0;
	environmentGroup = 0;
	fractalGroup = 0;
	bitmapGroup = 0;
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::CreateChunkData::validate () const
{
	NOT_NULL (createChunkBuffer);
	createChunkBuffer->validate ();

	DEBUG_FATAL (numberOfTilesPerChunk == 0, ("ProceduralTerrainAppearance::CreateChunkData::validate - numberOfTilesPerChunk == 0"));
	DEBUG_FATAL (chunkWidthInMeters == 0, ("ProceduralTerrainAppearance::CreateChunkData::validate - chunkWidthInMeters == 0"));
//	DEBUG_FATAL (tileWidthInMeters == 0, ("ProceduralTerrainAppearance::CreateChunkData::validate - tileWidthInMeters == 0"));
	DEBUG_FATAL (numberOfPoles == 0, ("ProceduralTerrainAppearance::CreateChunkData::validate - numberOfPoles == 0"));
	NOT_NULL (shaderGroup);
	NOT_NULL (floraGroup);
	NOT_NULL (radialGroup);
	NOT_NULL (environmentGroup);
	NOT_NULL (fractalGroup);
	NOT_NULL (bitmapGroup);
}

//===================================================================
//
// ProceduralTerrainAppearance::SurfaceData
//
ProceduralTerrainAppearance::SurfaceData::SurfaceData () :
	objectTemplate (0)
{
}

//===================================================================
//
// ProceduralTerrainAppearance::StaticFloraData
//
ProceduralTerrainAppearance::StaticFloraData::StaticFloraData () :
	floats (false),
	childChoice (0.f),
	familyChildData ()
{
}

//===================================================================
//
// ProceduralTerrainAppearance::Chunk
//
bool ProceduralTerrainAppearance::Chunk::ms_drawVertexNormals;
bool ProceduralTerrainAppearance::Chunk::ms_drawExtent;

//-------------------------------------------------------------------

ProceduralTerrainAppearance::Chunk::Chunk (ProceduralTerrainAppearance& proceduralTerrainAppearance) :
	Appearance (0),
	m_proceduralTerrainAppearance(proceduralTerrainAppearance),
	chunkX (0),
	chunkZ (0),
	chunkWidthInMeters (0),
	originOffset(0),
	numberOfPoles(0),
	m_boxExtent(),
	shaderMap (0),
	m_floraStaticCollidableMap (0),
	m_excluded(0),
	m_passable(unsigned(-1)),
	m_spatialSubdivisionHandle (0)
{
	hasLargerNeighborFlags=0;

	m_extent=&m_boxExtent;
}

//-------------------------------------------------------------------

ProceduralTerrainAppearance::Chunk::~Chunk ()
{
	_prepareForDelete ();

	DEBUG_FATAL(m_extent!=&m_boxExtent, ("extent illegally set on ProceduralTerrainAppearance::Chunk"));
	m_extent    =0;
	shaderMap   = 0;
	m_floraStaticCollidableMap = 0;
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::Chunk::_prepareForDelete ()
{
	m_proceduralTerrainAppearance.prepareForDelete(this);
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::Chunk::_setExcluded(int const tileX, int const tileZ)
{
	int const tileIndex = tileZ * m_proceduralTerrainAppearance.getNumberOfTilesPerChunk() + tileX;
	m_excluded |= (1 << tileIndex);
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::Chunk::isExcluded(const Vector& position) const
{
	int tileX, tileZ;
	_findTileXz(position, tileX, tileZ);
	int const tileIndex = tileZ * m_proceduralTerrainAppearance.getNumberOfTilesPerChunk() + tileX;
	return (m_excluded & (1 << tileIndex)) != 0;
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::Chunk::isExcluded(int const tileX, int const tileZ) const
{
	int const tileIndex = tileZ * m_proceduralTerrainAppearance.getNumberOfTilesPerChunk() + tileX;
	return (m_excluded & (1 << tileIndex)) != 0;
}

//-------------------------------------------------------------------

int ProceduralTerrainAppearance::Chunk::getChunkMemorySize () const
{
	NOT_NULL (shaderMap);
	const int shaderMapSize = isizeof (ShaderGroup::Info) * shaderMap->getWidth () * shaderMap->getHeight ();

	int floraMapSize=0;
	if (m_floraStaticCollidableMap)
	{
		floraMapSize = isizeof (FloraGroup::Info) * m_floraStaticCollidableMap->getWidth () * m_floraStaticCollidableMap->getHeight ();
	}

#ifdef _DEBUG
	DEBUG_REPORT_PRINT (true, (" shaderMapSize = %i\n", shaderMapSize));
	DEBUG_REPORT_PRINT (true, ("  floraMapSize = %i\n", floraMapSize));
#endif

	return isizeof (*this) + shaderMapSize + floraMapSize;
}

//-------------------------------------------------------------------

#ifdef _DEBUG

bool ProceduralTerrainAppearance::Chunk::debugRenderingEnabled () const
{
	return false;
}

#endif

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::Chunk::setDrawVertexNormals (bool drawVertexNormals)
{
	ms_drawVertexNormals = drawVertexNormals;
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::Chunk::getDrawVertexNormals ()
{
	return ms_drawVertexNormals;
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::Chunk::setDrawExtent (bool drawExtent)
{
	ms_drawExtent = drawExtent;
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::Chunk::getDrawExtent ()
{
	return ms_drawExtent;
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::Chunk::_findMapXz(const Vector& position, int &mapX, int &mapZ) const
{
	const float chunk2Meters          = m_proceduralTerrainAppearance.getChunkWidthInMeters();
	const float minX                  = float(chunkX) * chunk2Meters;
	const float minZ                  = float(chunkZ) * chunk2Meters;
	const float ratioX                = clamp (0.f, (position.x - minX) / chunkWidthInMeters, 1.f);
	const float ratioZ                = clamp (0.f, (position.z - minZ) / chunkWidthInMeters, 1.f);
	const int   numberOfTilesPerChunk = m_proceduralTerrainAppearance.getNumberOfTilesPerChunk();
	mapX = originOffset + static_cast<int>(ratioX * float(numberOfTilesPerChunk * 2));  //lint !e790  //-- integral to float
	mapZ = originOffset + static_cast<int>(ratioZ * float(numberOfTilesPerChunk * 2));  //lint !e790  //-- integral to float
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::Chunk::setPassable(int const tileX, int const tileZ, bool isPassable)
{
	int const tileIndex = tileZ * m_proceduralTerrainAppearance.getNumberOfTilesPerChunk() + tileX;
	if (isPassable)
	{
		m_passable |= (1 << tileIndex);
	}
	else
	{
		m_passable &= ~(1 << tileIndex);
	}
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::Chunk::isPassable(const Vector& position) const
{
	int tileX, tileZ;
	_findTileXz(position, tileX, tileZ);
	int const tileIndex = tileZ * m_proceduralTerrainAppearance.getNumberOfTilesPerChunk() + tileX;
	return (m_passable & (1 << tileIndex)) != 0;
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::Chunk::_findTileXz(Vector const & position, int & tileX, int & tileZ) const
{
	const float  chunk2Meters          = m_proceduralTerrainAppearance.getChunkWidthInMeters();
	const float  minX                  = float(chunkX) * chunk2Meters;
	const float  minZ                  = float(chunkZ) * chunk2Meters;

	const int    numberOfTilesPerChunk = m_proceduralTerrainAppearance.getNumberOfTilesPerChunk();
	const float  tileWidthInMeters     = chunkWidthInMeters / numberOfTilesPerChunk;

	tileX = clamp(0, static_cast<int>((position.x - minX) / tileWidthInMeters), numberOfTilesPerChunk - 1);
	tileZ = clamp(0, static_cast<int>((position.z - minZ) / tileWidthInMeters), numberOfTilesPerChunk - 1);
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::Chunk::findSurface (const Vector& position, SurfaceData& data) const
{
	int tileX;
	int tileZ;
	_findTileXz(position, tileX, tileZ);
	if (isExcluded(tileX, tileZ))
		return false;

	int x;
	int z;
	_findMapXz (position, x, z);

	NOT_NULL (shaderMap);
	const ShaderGroup::Info sgi = shaderMap->getData (x, z);

	if (sgi.getFamilyId ())
	{
		ShaderGroup const & shaderGroup = m_proceduralTerrainAppearance.getShaderGroup();
		data.objectTemplate = shaderGroup.getFamilySurfaceProperties(sgi.getFamilyId());

		return true;
	}

	return false;
}

//-----------------------------------------------------------------

void ProceduralTerrainAppearance::Chunk::_makeStaticFloraData(StaticFloraData& o_data, const FloraGroup::Info &i_groupInfo) const
{
	FloraGroup const & floraGroup = m_proceduralTerrainAppearance.getFloraGroup();
	//-- family data
	o_data.floats       = floraGroup.getFamilyFloats(i_groupInfo.getFamilyId());
	o_data.childChoice  = i_groupInfo.getChildChoice ();

	//-- family child data
	o_data.familyChildData = &floraGroup.createFlora(i_groupInfo);
}

//-----------------------------------------------------------------

bool ProceduralTerrainAppearance::Chunk::_floraAllowed(float positionX, float positionZ) const
{
	if (chunkWidthInMeters <= FLORA_ALLOWED_DISTANCE)
	{
		const Vector pos3(positionX, 0, positionZ);

		int tileX, tileZ;
		_findTileXz(pos3, tileX, tileZ);
		if (isExcluded(tileX, tileZ))
		{
			return false;
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------

bool ProceduralTerrainAppearance::Chunk::_findStaticFlora(const Array2d<FloraGroup::Info>& floraMap, const float positionX, const float positionZ, StaticFloraData& data, bool& floraAllowed) const
{
	FloraGroup::Info groupInfo;
	if (_findStaticFlora(floraMap, positionX, positionZ, groupInfo, floraAllowed))
	{
		_makeStaticFloraData(data, groupInfo);
		return true;
	}
	return false;
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::Chunk::_findStaticFlora(const Array2d<FloraGroup::Info>& floraMap, float positionX, float positionZ, FloraGroup::Info &data, bool& floraAllowed) const
{
	if (chunkWidthInMeters <= FLORA_ALLOWED_DISTANCE)
	{
		const Vector pos3(positionX, 0, positionZ);

		int tileX, tileZ;
		_findTileXz(pos3, tileX, tileZ);
		if (isExcluded(tileX, tileZ))
		{
			return false;
		}

		floraAllowed = true;

		int x, z;
		_findMapXz (pos3, x, z);

		data = floraMap.getData (x, z);
		return (data.getFamilyId()) ? true : false;
	}

	return false;
}

//===================================================================
// STATIC PUBLIC ProceduralTerrainAppearance
//===================================================================

ProceduralTerrainAppearance::CreateFloraHookFunction ProceduralTerrainAppearance::ms_createFloraHookFunction = 0;

//-----------------------------------------------------------------

void ProceduralTerrainAppearance::install ()
{
	DebugFlags::registerFlag (ms_logFloraCreation, "SharedTerrain", "logFloraCreation");
	DebugFlags::registerFlag (ms_logGetHeightFailures, "SharedTerrain", "logGetHeightFailures");

	ms_maximumNumberOfChunksAllowed = ConfigSharedTerrain::getMaximumNumberOfChunksAllowed ();

	ExitChain::add (remove, "ProceduralTerrainAppearance::remove");
}

//-----------------------------------------------------------------

void ProceduralTerrainAppearance::remove ()
{
	DebugFlags::unregisterFlag (ms_logFloraCreation);
	DebugFlags::unregisterFlag (ms_logGetHeightFailures);
}

//-----------------------------------------------------------------

void ProceduralTerrainAppearance::setCreateFloraHookFunction (ProceduralTerrainAppearance::CreateFloraHookFunction createFloraHookFunction)
{
	ms_createFloraHookFunction = createFloraHookFunction;
}

//-----------------------------------------------------------------

void ProceduralTerrainAppearance::setMaximumNumberOfChunksAllowed (int newMaximumNumberOfChunksAllowed)
{
	ms_maximumNumberOfChunksAllowed = newMaximumNumberOfChunksAllowed;
}

//===================================================================
// PUBLIC ProceduralTerrainAppearance
//===================================================================

ProceduralTerrainAppearance::ProceduralTerrainAppearance (const ProceduralTerrainAppearanceTemplate* appearanceTemplate) :
	TerrainAppearance (NON_NULL (appearanceTemplate)),
	originOffset(appearanceTemplate->getChunkOriginOffset()),
	upperPad(appearanceTemplate->getChunkUpperPad()),
	numberOfPoles(2*appearanceTemplate->getNumberOfTilesPerChunk() + originOffset + upperPad),
	sphere (),
	createChunkBuffer (),
	generateHeightChunkBuffer (),
	maximumNumberOfChunksAllowed (0),
	maximumNumberOfChunksAlongSide (0),
	proceduralTerrainAppearanceTemplate (appearanceTemplate),
	m_runTimeRuleList (new RunTimeRuleList),
	m_chunksGeneratedThisFrame (0),
	m_server (false),
	m_floraMap (new FloraMap),
	m_cachedFloraMap (new FloraMap),
	m_clearCollidableFloraMap (new ClearCollidableFloraMap),
	m_hasPassableAffectors(false)
{
	const TerrainGenerator* terrainGenerator      = proceduralTerrainAppearanceTemplate->getTerrainGenerator ();
	const float             mapWidthInMeters      = proceduralTerrainAppearanceTemplate->getMapWidthInMeters ();
	const float             chunkWidthInMeters    = proceduralTerrainAppearanceTemplate->getChunkWidthInMeters ();

	sphere.setRadius (2048.f);

	//-- make sure our generator was loaded
	NOT_NULL (terrainGenerator);

	//-- setup scratch buffer
	createChunkBuffer.allocate (numberOfPoles);

	const int generateHeightPoles = 2*1 + originOffset + upperPad;

	generateHeightChunkBuffer.allocate(generateHeightPoles);

	//-- allocate caches
	const_cast<TerrainGenerator*> (terrainGenerator)->getFractalGroup ().prepare (numberOfPoles, numberOfPoles);

	//
	//-- calculate max chunks
	//
	
	//-- 
	maximumNumberOfChunksAllowed = ms_maximumNumberOfChunksAllowed;
	maximumNumberOfChunksAlongSide = static_cast<int> (mapWidthInMeters / ( 2 * chunkWidthInMeters));

	m_hasPassableAffectors = terrainGenerator->hasPassableAffectors();
}

//-------------------------------------------------------------------

ProceduralTerrainAppearance::~ProceduralTerrainAppearance ()
{
	//-- delete clear flora map
	DEBUG_WARNING (!m_clearCollidableFloraMap->empty (), ("ProceduralTerrainAppearance: %u clear flora objects leaked\n", m_clearCollidableFloraMap->size ()));
	delete m_clearCollidableFloraMap;

	//-- delete flora
	DEBUG_WARNING (!m_floraMap->empty (), ("ProceduralTerrainAppearance: %u flora objects leaked\n", m_floraMap->size ()));

	while (!m_floraMap->empty ())
	{
		Object* const object = NON_NULL (m_floraMap->begin ()->second);
		DEBUG_WARNING (object->getAppearance () && object->getAppearance ()->getAppearanceTemplate () && object->getAppearance ()->getAppearanceTemplate ()->getName (), ("-- [%s]\n", object->getAppearance ()->getAppearanceTemplate ()->getName ()));
		object->removeFromWorld ();
		delete object;

		m_floraMap->erase (m_floraMap->begin ());
	}

	delete m_floraMap;

	//-- delete cached flora map
	while (!m_cachedFloraMap->empty ())
	{
		Object* const object = NON_NULL (m_cachedFloraMap->begin ()->second);
		delete object;

		m_cachedFloraMap->erase (m_cachedFloraMap->begin ());
	}

	delete m_cachedFloraMap;

	//--
	proceduralTerrainAppearanceTemplate = 0;

	delete m_runTimeRuleList;
	m_runTimeRuleList = 0;
}

//-------------------------------------------------------------------
/**
* Tests to see if the chunk indices fall within the map, and if the chunk creation
* clip is enabled, the indices are tested against the specified clip (initially 0,0x0,0)
*
* @see #getChunkCreationClipEnabled ()
* @see #setChunkCreationClipEnabled (bool)
* @see $setChunkCreationClip (const Vector , const Vector &);
*
* @param x the x coordinate in chunk-space
* @param z the z coordinate in chunk-space
*
*/

bool ProceduralTerrainAppearance::areValidChunkIndices (const int x, const int z) const
{
	return (x >= -maximumNumberOfChunksAlongSide && 
			z >= -maximumNumberOfChunksAlongSide && 
			x < maximumNumberOfChunksAlongSide && 
			z < maximumNumberOfChunksAlongSide);
}

//-------------------------------------------------------------------

int ProceduralTerrainAppearance::calculateChunkX (float positionX) const
{
	const float chunkWidthInMeters = proceduralTerrainAppearanceTemplate->getChunkWidthInMeters ();

	const int chunkX = static_cast<int> ((positionX >= 0.f) ? floorf (positionX / chunkWidthInMeters) : ceilf (positionX / chunkWidthInMeters));

	//-- zero is not a valid chunk index
	return (positionX < 0.f) ? chunkX - 1 : chunkX;
}

//-------------------------------------------------------------------

int ProceduralTerrainAppearance::calculateChunkZ (float positionZ) const
{
	const float chunkWidthInMeters = proceduralTerrainAppearanceTemplate->getChunkWidthInMeters ();

	const int chunkZ = static_cast<int> ((positionZ >= 0.f) ? floorf (positionZ / chunkWidthInMeters) : ceilf (positionZ / chunkWidthInMeters));

	//-- zero is not a valid chunk index
	return (positionZ < 0.f) ? chunkZ - 1 : chunkZ;
}
   
//-------------------------------------------------------------------

float ProceduralTerrainAppearance::getChunkHeight (const int chunkX, const int chunkZ) const
{
	const Chunk* chunk = findChunk (chunkX, chunkZ, 1);

	return chunk ? chunk->getBoxExtent ().getHeight () : 0.f;
}

//-------------------------------------------------------------------

const BoxExtent* ProceduralTerrainAppearance::getChunkExtent (const Vector& position_o) const
{
	const Chunk* const chunk = findFirstRenderableChunk (position_o);

	return chunk ? &chunk->getBoxExtent () : 0;
}

//-------------------------------------------------------------------

const BoxExtent* ProceduralTerrainAppearance::getChunkExtentForceChunkCreation (const Vector& position_o) const
{
	const Chunk* chunk = findChunk (position_o, 1);

	if (!chunk)
	{
		DEBUG_REPORT_LOG (ms_logGetHeightFailures, ("getChunkExtentForceChunkCreation: chunk for <%1.2f, %1.2f> does not exist, creating\n", position_o.x, position_o.z));
		const_cast<ProceduralTerrainAppearance*> (this)->createChunk (position_o, 1);

		chunk = findChunk (position_o, 1);
		DEBUG_REPORT_LOG (ms_logGetHeightFailures && !chunk, ("getChunkExtentForceChunkCreation: chunk for <%1.2f, %1.2f> STILL does not exist\n", position_o.x, position_o.z));
	}

	return chunk ? &chunk->getBoxExtent () : 0;
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::createChunk (const Vector& position, const int chunkSize) 
{
	//-- are we over our limit?
	const int maximumNumberOfChunks = std::max (maximumNumberOfChunksAllowed, getNumberOfReferenceObjects () + 10);

	while (getNumberOfChunks () > maximumNumberOfChunks)
		removeUnnecessaryChunk ();

	//-- where is this position in the world in reference to the terrain?
	const int chunkX = calculateChunkX(position.x);
	const int chunkZ = calculateChunkZ(position.z);

	if (m_server)
	{
		for (int j = chunkZ - 1; j <= chunkZ + 1; ++j)
			for (int i = chunkX - 1; i <= chunkX + 1; ++i)
				createChunk (i, j, chunkSize, 0);
	}
	else
		createChunk (chunkX, chunkZ, chunkSize, 0);
}

//-------------------------------------------------------------------
/**
* Obtain the first renderable chunk which contains the specified position
*/
const ProceduralTerrainAppearance::Chunk* ProceduralTerrainAppearance::findFirstRenderableChunk (const Vector& position) const
{
	const int chunkX = calculateChunkX (position.x);
	const int chunkZ = calculateChunkZ (position.z);

	return findFirstRenderableChunk (chunkX, chunkZ);
} 

//-------------------------------------------------------------------
/**
* Obtain the first renderable chunk which contains the specified X,Z coordinates
*/
const ProceduralTerrainAppearance::Chunk* ProceduralTerrainAppearance::findFirstRenderableChunk2D(float positionX, float positionZ) const
{
	const int chunkX = calculateChunkX(positionX);
	const int chunkZ = calculateChunkZ(positionZ);

	return findFirstRenderableChunk (chunkX, chunkZ);
}

//-------------------------------------------------------------------

const ProceduralTerrainAppearance::Chunk* ProceduralTerrainAppearance::findChunk (const Vector& position, const int chunkSize) const
{
	//-- where is this position in the world in reference to the terrain?
	const int chunkX = calculateChunkX (position.x);
	const int chunkZ = calculateChunkZ (position.z);

	return findChunk (chunkX, chunkZ, chunkSize);
}

//-------------------------------------------------------------------

const ProceduralTerrainAppearance::Chunk* ProceduralTerrainAppearance::findAnyChunk () const
{
	return 0;
}

//-------------------------------------------------------------------

const Sphere& ProceduralTerrainAppearance::getSphere () const
{
	return sphere;
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::getHeight (const Vector& position_o, float& height) const
{
	if (ConfigSharedTerrain::getDisableGetHeight ())
	{
		height = 0.f;
		return true;
	}

	const Chunk* chunk = findFirstRenderableChunk (position_o);

	if (chunk && chunk->getHeightAt (position_o, &height))
		return true;
	
	// DEBUG_WARNING (true, ("ProceduralTerrainAppearance::getHeight - could not find height for position <%1.2f, %1.2f>", position_o.x, position_o.z));
	return false;
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::getHeight (const Vector& position_o, float& height, Vector& normal) const
{
	if (ConfigSharedTerrain::getDisableGetHeight ())
	{
		height = 0.f;
		normal = Vector::unitY;

		return true;
	}

	const Chunk* chunk = findFirstRenderableChunk (position_o);

	if (chunk && chunk->getHeightAt (position_o, &height, &normal))
		return true;
	
	DEBUG_REPORT_LOG (ms_logGetHeightFailures, ("getHeight: could not find height for position <%1.2f, %1.2f>\n", position_o.x, position_o.z));
	return false;
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::getHeightForceChunkCreation (const Vector& position_o, float& height) const
{
	const Chunk* chunk = findChunk (position_o, 1);

	if (!chunk)
	{
		DEBUG_REPORT_LOG (ms_logGetHeightFailures, ("getHeightForceChunkCreation: chunk for <%1.2f, %1.2f> does not exist, creating\n", position_o.x, position_o.z));
		const_cast<ProceduralTerrainAppearance*> (this)->createChunk (position_o, 1);

		chunk = findChunk (position_o, 1);
		DEBUG_REPORT_LOG (ms_logGetHeightFailures && !chunk, ("getHeightForceChunkCreation: chunk for <%1.2f, %1.2f> STILL does not exist\n", position_o.x, position_o.z));
	}

	return chunk && chunk->getHeightAt (position_o, &height);
}

//-------------------------------------------------------------------

const ObjectTemplate* ProceduralTerrainAppearance::getSurfaceProperties (const Vector& position_o) const
{
	SurfaceData sd;
	if (findSurface (position_o, sd))
		return sd.objectTemplate;

	return 0;
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::getWaterHeight (const Vector& position_o, float& height) const
{
	return proceduralTerrainAppearanceTemplate->getWaterHeight (position_o, height);
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::getWaterHeight (const Vector& position_o, float& height, TerrainGeneratorWaterType& waterType, bool ignoreNonTransparentWater) const
{
	return proceduralTerrainAppearanceTemplate->getWaterHeight (position_o, height, waterType, ignoreNonTransparentWater);
}

//-------------------------------------------------------------------

TerrainGeneratorWaterType ProceduralTerrainAppearance::getWaterType (const Vector& position_o) const
{
	return proceduralTerrainAppearanceTemplate->getWaterType (position_o);
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::getWater (const int chunkX, const int chunkZ) const
{
	if (proceduralTerrainAppearanceTemplate->getBakedTerrain () != nullptr)
		return proceduralTerrainAppearanceTemplate->getBakedTerrain ()->getWater (chunkX, chunkZ);

	return false;
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::getSlope (const int chunkX, const int chunkZ) const
{
	if (proceduralTerrainAppearanceTemplate->getBakedTerrain () != nullptr)
		return proceduralTerrainAppearanceTemplate->getBakedTerrain ()->getSlope (chunkX, chunkZ);

	return false;
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::getWater (const Rectangle2d& rectangle) const
{
	if (proceduralTerrainAppearanceTemplate->getBakedTerrain () != nullptr)
		return proceduralTerrainAppearanceTemplate->getBakedTerrain ()->getWater (rectangle);

	return false;
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::getSlope (const Rectangle2d& rectangle) const
{
	if (proceduralTerrainAppearanceTemplate->getBakedTerrain () != nullptr)
		return proceduralTerrainAppearanceTemplate->getBakedTerrain ()->getSlope (rectangle);

	return false;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearance::getChunkWidthInMeters () const
{
	return proceduralTerrainAppearanceTemplate->getChunkWidthInMeters ();
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearance::getMapWidthInMeters () const
{
	return proceduralTerrainAppearanceTemplate->getMapWidthInMeters ();
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearance::getEnvironmentCycleTime () const
{
	return proceduralTerrainAppearanceTemplate->getEnvironmentCycleTime ();
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearance::alter (float elapsedTime)
{
	PROFILER_AUTO_BLOCK_DEFINE("ProceduralTerrainAppearance::alter");
	IGNORE_RETURN (TerrainAppearance::alter (elapsedTime));

	//-- clear cached flora list if over 256 items
	int numberOfFloraToDelete = ConfigSharedTerrain::getDisableFloraCaching () ? static_cast<int> (m_cachedFloraMap->size ()) : static_cast<int> (m_cachedFloraMap->size ()) - 256;
	while (numberOfFloraToDelete-- > 0)
	{
		const Object* const object = m_cachedFloraMap->begin ()->second;
		if (object)
		{
#ifdef _DEBUG
			const char* const appearanceTemplateName = object->getAppearance () && object->getAppearance ()->getAppearanceTemplate () ? object->getAppearance ()->getAppearanceTemplate ()->getCrcName ().getString () : 0;
			const Vector position = object->getPosition_w ();
			DEBUG_REPORT_LOG (ms_logFloraCreation, ("flora delete: <%1.2f, %1.2f> [%s]\n", position.x, position.z, appearanceTemplateName ? appearanceTemplateName : "nullptr"));
#endif
			delete object;
		}


		m_cachedFloraMap->erase (m_cachedFloraMap->begin ());
	}

	// @todo figure out what this should return.
	return AlterResult::cms_alterNextFrame;
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::verifyChunk (const Chunk* chunk) const
{
	NOT_NULL (chunk);

#ifdef _DEBUG
	//-- make sure chunk extents are correct
	const BoxExtent& boxExtent = chunk->getBoxExtent ();
	DEBUG_WARNING (!WithinEpsilonInclusive (boxExtent.getWidth (), getChunkWidthInMeters (), 0.001f), ("extent/chunk <%i, %i> width mismatch [%1.2f/%1.2f]", chunk->getChunkX (), chunk->getChunkZ (), boxExtent.getWidth (), getChunkWidthInMeters ()));
	DEBUG_WARNING (!WithinEpsilonInclusive (boxExtent.getLength (), getChunkWidthInMeters (), 0.001f), ("extent/chunk <%i, %i> length mismatch [%1.2f/%1.2f]", chunk->getChunkX (), chunk->getChunkZ (), boxExtent.getLength (), getChunkWidthInMeters ()));
#endif
}

//-------------------------------------------------------------------

int ProceduralTerrainAppearance::getNumberOfTilesPerChunk () const
{
	return proceduralTerrainAppearanceTemplate->getNumberOfTilesPerChunk ();
}

//-----------------------------------------------------------------

void ProceduralTerrainAppearance::setClient ()
{
	m_server = false;
}

//-----------------------------------------------------------------

const ProceduralTerrainAppearance::ClearCollidableFloraMap* ProceduralTerrainAppearance::getClearCollidableFloraMap () const
{
	return m_clearCollidableFloraMap;
}

//-----------------------------------------------------------------

void ProceduralTerrainAppearance::addLayer (const TerrainGenerator::Layer* layer)
{
	TerrainGenerator* const generator = const_cast<TerrainGenerator*> (proceduralTerrainAppearanceTemplate->getTerrainGenerator ());

	generator->addLayer (const_cast<TerrainGenerator::Layer*> (layer));

#ifdef _DEBUG
	RunTimeRuleList::const_iterator i = std::find (m_runTimeRuleList->begin (), m_runTimeRuleList->end (), layer);
	DEBUG_WARNING (i != m_runTimeRuleList->end (), ("adding same run-time rule %s twice", layer->getName ()));
#endif

	m_runTimeRuleList->push_back (const_cast<TerrainGenerator::Layer*> (layer));
}  //lint !e1762  // function could be made const

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::removeLayer (const TerrainGenerator::Layer* layer)
{
	TerrainGenerator* const generator = const_cast<TerrainGenerator*> (proceduralTerrainAppearanceTemplate->getTerrainGenerator ());

	generator->removeLayer (const_cast<TerrainGenerator::Layer*> (layer), false);

	RunTimeRuleList::iterator i = std::find (m_runTimeRuleList->begin (), m_runTimeRuleList->end (), layer);
	if (i != m_runTimeRuleList->end ())
		IGNORE_RETURN (m_runTimeRuleList->erase (i));
	else
		DEBUG_WARNING (true, ("run-time rule %s not found", layer->getName ()));
}  //lint !e1762  // function could be made const

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::prepareGenerator ()
{
	TerrainGenerator* const generator = const_cast<TerrainGenerator*> (proceduralTerrainAppearanceTemplate->getTerrainGenerator ());

	generator->prepare ();
}  //lint !e1762  // function could be made const

//-------------------------------------------------------------------

float ProceduralTerrainAppearance::generateHeight_expensive (const Vector2d& position_w)
{
	const TerrainGenerator* terrainGenerator      = proceduralTerrainAppearanceTemplate->getTerrainGenerator ();
	const int               numberOfTilesPerChunk = 1;
	const float             tileWidthInMeters     = proceduralTerrainAppearanceTemplate->getTileWidthInMeters ();

	//-- chunk does not exist -- it needs to be created. find out what map data i'll need to ask the generator for

	const int   generateHeightPoles = 2*numberOfTilesPerChunk + originOffset + upperPad;
	const float distanceBetweenPoles =  tileWidthInMeters * 0.5f;

	const Vector start (
		position_w.x - static_cast<float>(originOffset) * distanceBetweenPoles, 
		0.0f,
		position_w.y - static_cast<float>(originOffset) * distanceBetweenPoles)
	;

	//-- ask the generator to fill out this area
	TerrainGenerator::GeneratorChunkData generatorChunkData(proceduralTerrainAppearanceTemplate->getLegacyMode());

	generatorChunkData.heightMap            = &generateHeightChunkBuffer.heightMap;
	generatorChunkData.colorMap             = &generateHeightChunkBuffer.colorMap;
	generatorChunkData.shaderMap            = &generateHeightChunkBuffer.shaderMap;
	generatorChunkData.floraStaticCollidableMap    = &generateHeightChunkBuffer.floraStaticCollidableMap;
	generatorChunkData.floraStaticNonCollidableMap = &generateHeightChunkBuffer.floraStaticNonCollidableMap;
	generatorChunkData.floraDynamicNearMap  = &generateHeightChunkBuffer.floraDynamicNearMap;
	generatorChunkData.floraDynamicFarMap   = &generateHeightChunkBuffer.floraDynamicFarMap;
	generatorChunkData.environmentMap       = &generateHeightChunkBuffer.environmentMap;
	generatorChunkData.vertexPositionMap    = &generateHeightChunkBuffer.vertexPositionMap;
	generatorChunkData.vertexNormalMap      = &generateHeightChunkBuffer.vertexNormalMap;
	generatorChunkData.excludeMap           = &generateHeightChunkBuffer.excludeMap;
	generatorChunkData.passableMap          = &generateHeightChunkBuffer.passableMap;
	generatorChunkData.start                = start;
	generatorChunkData.numberOfPoles        = generateHeightPoles;
	generatorChunkData.originOffset         = originOffset;
	generatorChunkData.upperPad             = upperPad;
	generatorChunkData.distanceBetweenPoles = distanceBetweenPoles;
	generatorChunkData.shaderGroup          = &terrainGenerator->getShaderGroup ();
	generatorChunkData.floraGroup           = &terrainGenerator->getFloraGroup ();
	generatorChunkData.radialGroup          = &terrainGenerator->getRadialGroup ();
	generatorChunkData.environmentGroup     = &terrainGenerator->getEnvironmentGroup ();
	generatorChunkData.fractalGroup         = &terrainGenerator->getFractalGroup ();
	generatorChunkData.bitmapGroup          = &terrainGenerator->getBitmapGroup ();

	terrainGenerator->generateChunk (generatorChunkData);

	return generatorChunkData.heightMap->getData(originOffset, originOffset);
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::findSurface (const Vector& position, ProceduralTerrainAppearance::SurfaceData& data) const
{
	const Chunk* chunk = findFirstRenderableChunk (position);

	return chunk && chunk->findSurface (position, data);
}

//----------------------------------------------------------------------

bool ProceduralTerrainAppearance::isPassable(const Vector& position) const
{
	if (!m_hasPassableAffectors)
		return true;

	Chunk const * chunk = findFirstRenderableChunk(position);
	return chunk && chunk->isPassable(position);
}

//----------------------------------------------------------------------

bool ProceduralTerrainAppearance::isPassableForceChunkCreation(const Vector& position) const
{
	if (!m_hasPassableAffectors)
		return true;

	Chunk const * chunk = findChunk(position, 1);
	if (!chunk)
	{
		const_cast<ProceduralTerrainAppearance*> (this)->createChunk (position, 1);
		chunk = findChunk (position, 1);
	}

	bool const isPassable = (chunk && chunk->isPassable(position));

#ifndef WIN32
	if (!chunk)
		LOG("PTA::isPFCC", ("(%g,%g,%g) chunk is nullptr, %d/%d/%d/%d", position.x, position.y, position.z, getNumberOfChunks(), maximumNumberOfChunksAllowed, getNumberOfReferenceObjects(), maximumNumberOfChunksAlongSide));
#endif

	return isPassable;
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::findStaticCollidableFlora (const Vector& position, ProceduralTerrainAppearance::StaticFloraData& data, bool& floraAllowed) const
{
	const Chunk* chunk = findFirstRenderableChunk (position);

	return chunk && chunk->findStaticCollidableFlora (position, data, floraAllowed);
}  //lint !e1763  // function marked as const modifies class

//===================================================================

static int calculateFloraTileX (const Vector& position, const float tileWidthInMeters) 
{
	const int x = static_cast<int> ((position.x >= 0.f) ? floorf (position.x / tileWidthInMeters) : ceilf (position.x / tileWidthInMeters));

	//-- zero is not a valid chunk index
	return (position.x < 0.f) ? x - 1 : x;
}

//-------------------------------------------------------------------

static int calculateFloraTileZ (const Vector& position, const float tileWidthInMeters) 
{
	const int z = static_cast<int> ((position.z >= 0.f) ? floorf (position.z / tileWidthInMeters) : ceilf (position.z / tileWidthInMeters));

	//-- zero is not a valid chunk index
	return (position.z < 0.f) ? z - 1 : z;
}
   
//-------------------------------------------------------------------

static uint32 calculateFloraMapKey (const int x, const int z, const int mapWidthInTiles)
{
	DEBUG_FATAL (x < 0, (""));
	DEBUG_FATAL (z < 0, (""));
	return static_cast<uint32> (z * mapWidthInTiles + x);
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearance::_legacyGetStaticCollidableFloraData(StaticFloraData &o_data, const Chunk* const chunk, const Vector &floraPosition)
{
	bool chunkFloraAllowed=false;
	bool chunkHasFlora=false;

	if (chunk->findStaticCollidableFlora (floraPosition, o_data, chunkFloraAllowed))
	{
		if (o_data.familyChildData->familyId)
		{
			chunkHasFlora = true;
		}
		else
		{
			chunkHasFlora = false;
		}
	}
	return chunkHasFlora;
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::_legacyCreateFlora(const Chunk* const chunk)
{
	NOT_NULL (chunk);

	const float floraTileWidthInMeters  = 16.f;
	const float floraTileBorderInMeters = proceduralTerrainAppearanceTemplate->getCollidableTileBorder ();
	const int   floraMapWidthInTiles    = static_cast<int> (getMapWidthInMeters () / floraTileWidthInMeters);
	const int   floraTileCenterOffset   = floraMapWidthInTiles / 2;

	const BoxExtent& extent     = chunk->getBoxExtent ();
	const Vector     start      = Vector::linearInterpolate (extent.getMin (), extent.getMax (), 0.5f);
	const int        floraTileX = calculateFloraTileX (start, floraTileWidthInMeters);
	const int        floraTileZ = calculateFloraTileZ (start, floraTileWidthInMeters);
	const int        keyX       = floraTileX + floraTileCenterOffset;
	const int        keyZ       = floraTileZ + floraTileCenterOffset;
	const uint32     key        = calculateFloraMapKey (keyX, keyZ, floraMapWidthInTiles);

	FloraMap::iterator iter = m_floraMap->find (key);
	if (iter == m_floraMap->end ())
	{
		RandomGenerator random (key);

		const float xOffset = random.randomReal (0.f, 1.f);
		const float zOffset = random.randomReal (0.f, 1.f);

		const Vector floraTileStart (static_cast<float> (floraTileX) * floraTileWidthInMeters, 0.f, static_cast<float> (floraTileZ) * floraTileWidthInMeters);
		const Vector floraPositionWithinTile (xOffset * (floraTileWidthInMeters - (2.f * floraTileBorderInMeters)) + floraTileBorderInMeters, 0.f, zOffset * (floraTileWidthInMeters - (2.f * floraTileBorderInMeters)) + floraTileBorderInMeters);

		const Rectangle2d extent2d (extent.getLeft (), extent.getBack (), extent.getRight (), extent.getFront ());

		Vector floraPosition = floraTileStart + floraPositionWithinTile;
		if (  !shouldClearCollidableFlora (floraPosition) 
			&& extent2d.isWithin(floraPosition.x, floraPosition.z) 
			&& chunk->getHeightAt(floraPosition, &floraPosition.y)
			)
		{
			StaticFloraData data;
			const bool chunkHasFlora = _legacyGetStaticCollidableFloraData(data, chunk, floraPosition);

			///////////////////////////////////////////////////////////////////////////////

			if (chunkHasFlora)
			{
				DEBUG_REPORT_LOG (ms_logFloraCreation, ("flora create: <%1.2f, %1.2f> %i %1.2f [%s]\n", floraPosition.x, floraPosition.z, data.familyChildData->familyId, data.childChoice, data.familyChildData->appearanceTemplateName));

				iter = m_cachedFloraMap->find (key);
				if (iter != m_cachedFloraMap->end ())
				{
					iter->second->addToWorld ();
					IGNORE_RETURN (m_floraMap->insert (std::make_pair (key, iter->second)));

					m_cachedFloraMap->erase (iter);
				}
				else
				{
					const float yaw = random.randomReal (0.f, PI_TIMES_2);
					const float scale = data.familyChildData->shouldScale ? random.randomReal (data.familyChildData->minimumScale, data.familyChildData->maximumScale) : 1.f;

 					Object* const object = new Object ();
					object->yaw_o (yaw);
					Appearance * const appearance = AppearanceTemplateList::createAppearance (FileName (FileName::P_appearance, data.familyChildData->appearanceTemplateName));

					if (appearance != nullptr) {
						appearance->setKeepAlive (true);
						object->setAppearance (appearance);
						object->setScale (Vector::xyz111 * scale);						
	
						if (ms_createFloraHookFunction)
						ms_createFloraHookFunction (*object);
	
						object->setPosition_p (floraPosition);

						CollisionProperty* const collisionProperty = new CollisionProperty (*object);
						collisionProperty->setFlora (true);
						object->addProperty (*collisionProperty);

						object->addToWorld ();

						IGNORE_RETURN (m_floraMap->insert (std::make_pair (key, object)));
					} else {
						DEBUG_WARNING(true, ("FIX ME: Appearance template in ProceduralTerrainAppearance::_legacyCreateFlora is not found"));
					}
				}  //lint !e429  //-- collisionProperty has not been freed or returned
			}
		}
	}
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::createFlora (const Chunk* const chunk)
{
	if (proceduralTerrainAppearanceTemplate->getLegacyMode())
	{
		_legacyCreateFlora(chunk);
		return;
	}

	NOT_NULL (chunk);

	const float floraTileWidthInMeters  = 16.f;
	const float floraTileBorderInMeters = proceduralTerrainAppearanceTemplate->getCollidableTileBorder ();
	const int   floraMapWidthInTiles    = static_cast<int> (getMapWidthInMeters () / floraTileWidthInMeters);
	const int   floraTileCenterOffset   = floraMapWidthInTiles / 2;

	const BoxExtent& extent     = chunk->getBoxExtent ();
	const Vector     start      = Vector::linearInterpolate (extent.getMin (), extent.getMax (), 0.5f);
	const int        floraTileX = calculateFloraTileX (start, floraTileWidthInMeters);
	const int        floraTileZ = calculateFloraTileZ (start, floraTileWidthInMeters);
	const int        keyX       = floraTileX + floraTileCenterOffset;
	const int        keyZ       = floraTileZ + floraTileCenterOffset;
	const uint32     key        = calculateFloraMapKey(keyX, keyZ, floraMapWidthInTiles);

	FloraMap::iterator iter = m_floraMap->find (key);
	if (iter != m_floraMap->end ())
	{
		return;
	}

	// -----------------------------------------------------
	// flora position
	/*
	const int modKeyX = keyX&7;
	const int modKeyZ = keyZ&7;
	const uint32 seed = calculateFloraMapKey(modKeyX, modKeyZ, floraMapWidthInTiles);
	RandomGenerator random (seed);
	*/
	RandomGenerator random (key);

	const float xOffset = random.randomReal(0.f, 1.f);
	const float zOffset = random.randomReal(0.f, 1.f);

	const Vector floraTileStart (static_cast<float> (floraTileX) * floraTileWidthInMeters, 0.f, static_cast<float> (floraTileZ) * floraTileWidthInMeters);
	const Vector floraPositionWithinTile (xOffset * (floraTileWidthInMeters - (2.f * floraTileBorderInMeters)) + floraTileBorderInMeters, 0.f, zOffset * (floraTileWidthInMeters - (2.f * floraTileBorderInMeters)) + floraTileBorderInMeters);

	const Rectangle2d extent2d (extent.getLeft (), extent.getBack (), extent.getRight (), extent.getFront ());

	Vector floraPosition = floraTileStart + floraPositionWithinTile;
	// -----------------------------------------------------

	// -----------------------------------------------------
	if (!extent2d.isWithin(floraPosition.x, floraPosition.z))
	{
		return;
	}

	if (!chunk->floraAllowed(floraPosition))
	{
		return;
	}

	if (shouldClearCollidableFlora(floraPosition))
	{
		return;
	}
	// -----------------------------------------------------


	// -----------------------------------------------------
	//mapFgi = scmap[modKeyZ*floraMapWidthInTiles + modKeyX];
	int mapFamily = proceduralTerrainAppearanceTemplate->getStaticCollidableFloraFamily(keyX, keyZ);
	if (mapFamily==0)
	{
		return;
	}

	FloraGroup::Info mapFgi;
	StaticFloraData data;

	mapFgi.setFamilyId(mapFamily);

	float seedX = floorf(floraPosition.x);
	float seedZ = floorf(floraPosition.z);
	uint32 hash = CoordinateHash::hashTuple(seedX, seedZ);
	float childChoice = CoordinateHash::makeFloat(hash);
	mapFgi.setChildChoice(childChoice);

	FloraGroup const & floraGroup = getFloraGroup();

	//-- family data
	data.floats       = floraGroup.getFamilyFloats(mapFgi.getFamilyId());
	data.childChoice  = mapFgi.getChildChoice ();

	//-- family child data
	data.familyChildData = &floraGroup.createFlora(mapFgi);

	// -----------------------------------------------------

	DEBUG_REPORT_LOG (ms_logFloraCreation, ("flora create: <%1.2f, %1.2f> %i %1.2f [%s]\n", floraPosition.x, floraPosition.z, data.familyChildData->familyId, data.childChoice, data.familyChildData->appearanceTemplateName));

	iter = m_cachedFloraMap->find (key);
	if (iter != m_cachedFloraMap->end ())
	{
		iter->second->addToWorld ();
		IGNORE_RETURN (m_floraMap->insert (std::make_pair (key, iter->second)));

		m_cachedFloraMap->erase (iter);
	}
	else
	{

		// -----------------------------------------------------
		// we're going to need the height now.
		floraPosition.y=proceduralTerrainAppearanceTemplate->getStaticCollidableFloraHeight(keyX, keyZ);
		/*
		if (!chunk->getHeightAt(floraPosition, &floraPosition.y))
		{
			return;
		}
		*/
		// -----------------------------------------------------

		const float yaw = random.randomReal(0.f, PI_TIMES_2);
		const float scale = data.familyChildData->shouldScale ? random.randomReal(data.familyChildData->minimumScale, data.familyChildData->maximumScale) : 1.f;

		Object* const object = new Object ();
		object->yaw_o (yaw);
		Appearance * const appearance = AppearanceTemplateList::createAppearance (FileName (FileName::P_appearance, data.familyChildData->appearanceTemplateName));

		if (appearance != nullptr) {
			appearance->setKeepAlive (true);
			object->setAppearance (appearance);
			object->setScale (Vector::xyz111 * scale);						

			if (ms_createFloraHookFunction)
			{
				ms_createFloraHookFunction(*object);
			}

			object->setPosition_p (floraPosition);

			CollisionProperty* const collisionProperty = new CollisionProperty (*object);
			collisionProperty->setFlora (true);
			object->addProperty (*collisionProperty);

			object->addToWorld ();

			IGNORE_RETURN (m_floraMap->insert (std::make_pair (key, object)));
	    	} else {
	        	DEBUG_WARNING(true, ("FIX ME: Appearance template in ProceduralTerrainAppearance::createFlora is not found"));
                }

	}  //lint !e429  //-- collisionProperty has not been freed or returned
}

//-------------------------------------------------------------------

template<class T>
inline bool WithinRangeInclusiveExclusive (const T &rangeMin, const T &value, const T &rangeMax)
{
	DEBUG_FATAL (rangeMax < rangeMin, ("range error: max value is less than min value"));
	return (value >= rangeMin) && (value < rangeMax);
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::destroyFlora (const Chunk* const chunk)
{
	NOT_NULL (chunk);

	const float floraTileWidthInMeters  = 16.f;
	const int   floraMapWidthInTiles    = static_cast<int> (getMapWidthInMeters () / floraTileWidthInMeters);
	const int   floraTileCenterOffset   = floraMapWidthInTiles / 2;

	const BoxExtent& extent     = chunk->getBoxExtent ();
	const Vector     start      = Vector::linearInterpolate (extent.getMin (), extent.getMax (), 0.5f);
	const int        floraTileX = calculateFloraTileX (start, floraTileWidthInMeters);
	const int        floraTileZ = calculateFloraTileZ (start, floraTileWidthInMeters);
	const int        keyX       = floraTileX + floraTileCenterOffset;
	const int        keyZ       = floraTileZ + floraTileCenterOffset;
	const uint32     key        = calculateFloraMapKey (keyX, keyZ, floraMapWidthInTiles);

	FloraMap::iterator iter = m_floraMap->find (key);
	if (iter != m_floraMap->end ())
	{
		Object* const object = iter->second;
		const Vector position = object->getPosition_w ();
		if (WithinRangeInclusiveExclusive (extent.getLeft (), position.x, extent.getRight ()) &&
			WithinRangeInclusiveExclusive (extent.getBack (), position.z, extent.getFront ()))
		{
#ifdef _DEBUG
			const char* const appearanceTemplateName = object->getAppearance () && object->getAppearance ()->getAppearanceTemplate () ? object->getAppearance ()->getAppearanceTemplate ()->getCrcName ().getString () : 0;
			DEBUG_REPORT_LOG (ms_logFloraCreation, ("flora cache: <%1.2f, %1.2f> [%s]\n", position.x, position.z, appearanceTemplateName ? appearanceTemplateName : "nullptr"));
#endif
			iter->second->removeFromWorld ();
			IGNORE_RETURN (m_cachedFloraMap->insert (std::make_pair (key, iter->second)));

			m_floraMap->erase (iter);
		}
	}
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::prepareForDelete (const Chunk* const chunk)
{
	destroyFlora (chunk);
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::addClearCollidableFloraObject (const Object* const object, const Vector& position, const float radius)
{
	ClearCollidableFloraEntry entry;
	entry.position.set (position.x, 0.f, position.z);
	entry.radius   = radius;
	IGNORE_RETURN (m_clearCollidableFloraMap->insert (std::make_pair (object, entry)));

	//-- clear the flora objects that have been created
	const int x0 = calculateChunkX (position.x - radius);
	const int x1 = calculateChunkX (position.x + radius);
	const int z0 = calculateChunkZ (position.z - radius);
	const int z1 = calculateChunkZ (position.z + radius);

	int x;
	int z;
	for (z = z0; z <= z1; ++z)
		for (x = x0; x <= x1; ++x)
		{
			const Chunk* const chunk = findChunk (x, z, 1);
			if (chunk)
			{
				destroyFlora (chunk);
				createFlora (chunk);
			}
		}
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::removeClearCollidableFloraObject (const Object* const object)
{
	ClearCollidableFloraMap::iterator iter = m_clearCollidableFloraMap->find (object);
	if (iter != m_clearCollidableFloraMap->end ())
	{
		//-- go through the existing chunks for the area and mark them as visible again
		const ClearCollidableFloraEntry entry = iter->second;

		m_clearCollidableFloraMap->erase (iter);

		//-- clear the flora objects that have been created
		const int x0 = calculateChunkX (entry.position.x - entry.radius);
		const int x1 = calculateChunkX (entry.position.x + entry.radius);
		const int z0 = calculateChunkZ (entry.position.z - entry.radius);
		const int z1 = calculateChunkZ (entry.position.z + entry.radius);

		if (World::isValid ())
		{
			for (int z = z0; z <= z1; ++z)
				for (int x = x0; x <= x1; ++x)
				{
					const Chunk* const chunk = findChunk (x, z, 1);
					if (chunk)
						createFlora (chunk);
				}
		}
	}
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearance::debugDump () const
{
#ifdef _DEBUG
	TerrainAppearance::debugDump ();

	DEBUG_REPORT_PRINT (true, ("-- ProceduralTerrainAppearance\n"));
	DEBUG_REPORT_PRINT (true, ("   maximumNumberOfChunks = %i\n", maximumNumberOfChunksAllowed));
	DEBUG_REPORT_PRINT (true, ("          numberOfChunks = %i\n", getNumberOfChunks ()));
	DEBUG_REPORT_PRINT (true, ("          flora map size = %i\n", m_floraMap->size ()));
	DEBUG_REPORT_PRINT (true, ("   cached flora map size = %i\n", m_cachedFloraMap->size ()));
	DEBUG_REPORT_PRINT (true, ("    clear flora map size = %i\n", m_clearCollidableFloraMap->size ()));
	DEBUG_REPORT_PRINT (true, ("chunksGeneratedThisFrame = %i\n", m_chunksGeneratedThisFrame));

	const Chunk* const chunk = findAnyChunk ();
	if (chunk)
	{
		const int chunkMemorySize = chunk->getChunkMemorySize ();
		DEBUG_REPORT_PRINT (true, ("         chunkMemorySize = %i\n", chunkMemorySize));
		DEBUG_REPORT_PRINT (true, ("       terrainMemorySize = ~%i\n", chunkMemorySize * getNumberOfChunks ()));
	}
#endif
}

// ----------------------------------------------------------------------

ShaderGroup const & ProceduralTerrainAppearance::getShaderGroup() const
{
	return proceduralTerrainAppearanceTemplate->getTerrainGenerator()->getShaderGroup();
}

// ----------------------------------------------------------------------

FloraGroup const & ProceduralTerrainAppearance::getFloraGroup() const
{
	return proceduralTerrainAppearanceTemplate->getTerrainGenerator()->getFloraGroup();
}

// ----------------------------------------------------------------------

RadialGroup const & ProceduralTerrainAppearance::getRadialGroup() const
{
	return proceduralTerrainAppearanceTemplate->getTerrainGenerator()->getRadialGroup();
}

// ----------------------------------------------------------------------

EnvironmentGroup const & ProceduralTerrainAppearance::getEnvironmentGroup() const
{
	return proceduralTerrainAppearanceTemplate->getTerrainGenerator()->getEnvironmentGroup();
}

//----------------------------------------------------------------------

bool ProceduralTerrainAppearance::hasPassableAffectors() const
{
	return m_hasPassableAffectors;
}

//===================================================================
// PRIVATE ProceduralTerrainAppearance
//===================================================================

bool ProceduralTerrainAppearance::shouldClearCollidableFlora (const Vector& position) const
{
	const Vector position2d (position.x, 0.f, position.z);

	ClearCollidableFloraMap::iterator iter = m_clearCollidableFloraMap->begin ();
	for (; iter != m_clearCollidableFloraMap->end (); ++iter)
		if (iter->second.position.magnitudeBetweenSquared (position2d) < sqr (iter->second.radius))
			return true;

	return false;
}

//===================================================================
