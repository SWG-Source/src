//===================================================================
//
// ProceduralTerrainAppearanceTemplate.cpp
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"

#include "sharedFile/TreeFile.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedMath/Line2d.h"
#include "sharedMath/Plane.h"
#include "sharedTerrain/AffectorRibbon.h"
#include "sharedTerrain/Boundary.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedTerrain/WaterTypeManager.h"
#include "sharedUtility/PackedIntegerMap.h"
#include "sharedUtility/PackedFixedPointMap.h"
#include "sharedUtility/BakedTerrain.h"

#include <algorithm>
#include <string>
#include <vector>

//===================================================================

namespace
{
	struct StaticFloraSampleFileHeader
	{
		char  terrainName[64];
		float floraTileWidthInMeters;
		int   numberOfFloraSampled;
	};
}

//===================================================================

ProceduralTerrainAppearanceTemplate::ProceduralTerrainAppearanceTemplate(
	const char* filename, 
	Iff* iff, 
	bool legacyMode, 
	int  chunkOriginOffset, // size of sample pad on lower side of chunk.
	int  chunkUpperPad,     // size of sample pad on upper side of chunk.
	bool samplingMode
) :
	AppearanceTemplate (filename),
	m_name (0),
	m_mapWidthInMeters (0),
	m_chunkWidthInMeters (0),
	m_numberOfTilesPerChunk (0),
	m_useGlobalWaterTable (false),
	m_globalWaterTableHeight (0),
	m_globalWaterTableShaderTemplateName (0),
	m_globalWaterTableShaderSize (2.f),
	m_environmentCycleTime (1800.f),
	m_collidableMinimumDistance (0.f),
	m_collidableMaximumDistance (0.f),
	m_collidableTileSize (0.f),
	m_collidableTileBorder (0.f),
	m_collidableSeed (0),
	m_nonCollidableMinimumDistance (0.f),
	m_nonCollidableMaximumDistance (0.f),
	m_nonCollidableTileSize (0.f),
	m_nonCollidableTileBorder (0.f),
	m_nonCollidableSeed (0),
	m_radialMinimumDistance (0.f),
	m_radialMaximumDistance (0.f),
	m_radialTileSize (0.f),
	m_radialTileBorder (0.f),
	m_radialSeed (0),
	m_farRadialMinimumDistance (0.f),
	m_farRadialMaximumDistance (0.f),
	m_farRadialTileSize (0.f),
	m_farRadialTileBorder (0.f),
	m_farRadialSeed (0),
	m_legacyMap(false),
	m_legacyMode(legacyMode),
	m_samplingMode(samplingMode),
	m_tileWidthInMeters (0),
	m_chunkOriginOffset(legacyMode ? 3 : chunkOriginOffset),
	m_chunkUpperPad(legacyMode ? 3 : chunkUpperPad),
	m_terrainGenerator (0),
	m_bakedTerrain (0),
	m_waterTableList (new WaterTableList),
	m_ribbonQuadList (new RibbonQuadList),
	m_ribbonEndCapList (new RibbonEndCapList),
	m_floraTileWidthInMeters(16.0f),
	m_mapWidthInFlora(0),
	m_staticCollidableFloraMap(0),
	m_staticCollidableFloraHeightMap(0)
{

	NOT_NULL (iff);
	load (*iff);

	//-- 

	createWaterTableAndRibbonQuadLists ();
}

//-------------------------------------------------------------------

ProceduralTerrainAppearanceTemplate::~ProceduralTerrainAppearanceTemplate ()
{
	//-- delete name
	delete [] m_name;
	m_name = 0;

	delete [] m_globalWaterTableShaderTemplateName;
	m_globalWaterTableShaderTemplateName = 0;

	delete m_terrainGenerator;
	m_terrainGenerator = 0;

	delete m_bakedTerrain;
	m_bakedTerrain = 0;

	delete m_waterTableList;
	m_waterTableList = 0;

	delete m_ribbonQuadList;
	m_ribbonQuadList = 0;

	delete m_ribbonEndCapList;
	m_ribbonEndCapList = 0;

	_destroyStaticCollidableFloraMaps();
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearanceTemplate::_setSamplingParameters(bool legacyMode, int chunkOriginOffset, int chunkUpperPad)
{
	m_legacyMode = legacyMode;
	m_chunkOriginOffset = legacyMode ? 3 : chunkOriginOffset;
	m_chunkUpperPad     = legacyMode ? 3 : chunkUpperPad;
}

//-------------------------------------------------------------------

const char* ProceduralTerrainAppearanceTemplate::getName () const
{
	return m_name;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getMapWidthInMeters () const
{
	return m_mapWidthInMeters;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getChunkWidthInMeters () const
{
	return m_chunkWidthInMeters;
}

//-------------------------------------------------------------------

int ProceduralTerrainAppearanceTemplate::getNumberOfTilesPerChunk () const
{
	return m_numberOfTilesPerChunk;
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearanceTemplate::getUseGlobalWaterTable () const
{
	return m_useGlobalWaterTable;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getGlobalWaterTableHeight () const
{
	return m_globalWaterTableHeight;
}

//-------------------------------------------------------------------

const char* ProceduralTerrainAppearanceTemplate::getGlobalWaterTableShaderTemplateName () const
{
	return m_globalWaterTableShaderTemplateName;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getGlobalWaterTableShaderSize () const
{
	return m_globalWaterTableShaderSize;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getCollidableMinimumDistance () const
{
	return m_collidableMinimumDistance;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getCollidableMaximumDistance () const
{
	return m_collidableMaximumDistance;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getCollidableTileSize () const
{
	return m_collidableTileSize;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getCollidableTileBorder () const
{
	return m_collidableTileBorder;
}

//-------------------------------------------------------------------

uint32 ProceduralTerrainAppearanceTemplate::getCollidableSeed () const
{
	return m_collidableSeed;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getNonCollidableMinimumDistance () const
{
	return m_nonCollidableMinimumDistance;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getNonCollidableMaximumDistance () const
{
	return m_nonCollidableMaximumDistance;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getNonCollidableTileSize () const
{
	return m_nonCollidableTileSize;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getNonCollidableTileBorder () const
{
	return m_nonCollidableTileBorder;
}

//-------------------------------------------------------------------

uint32 ProceduralTerrainAppearanceTemplate::getNonCollidableSeed () const
{
	return m_nonCollidableSeed;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getRadialMinimumDistance () const
{
	return m_radialMinimumDistance;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getRadialMaximumDistance () const
{
	return m_radialMaximumDistance;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getRadialTileSize () const
{
	return m_radialTileSize;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getRadialTileBorder () const
{
	return m_radialTileBorder;
}

//-------------------------------------------------------------------

uint32 ProceduralTerrainAppearanceTemplate::getRadialSeed () const
{
	return m_radialSeed;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getFarRadialMinimumDistance () const
{
	return m_farRadialMinimumDistance;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getFarRadialMaximumDistance () const
{
	return m_farRadialMaximumDistance;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getFarRadialTileSize () const
{
	return m_farRadialTileSize;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getFarRadialTileBorder () const
{
	return m_farRadialTileBorder;
}

//-------------------------------------------------------------------

uint32 ProceduralTerrainAppearanceTemplate::getFarRadialSeed () const
{
	return m_farRadialSeed;
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getTileWidthInMeters () const
{
	return m_tileWidthInMeters;
}

//-----------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getEnvironmentCycleTime () const
{
	return m_environmentCycleTime;
}

//-----------------------------------------------------------------

const TerrainGenerator* ProceduralTerrainAppearanceTemplate::getTerrainGenerator () const
{
	return m_terrainGenerator;
}

//-----------------------------------------------------------------

const BakedTerrain* ProceduralTerrainAppearanceTemplate::getBakedTerrain () const
{
	return m_bakedTerrain;
}

//-----------------------------------------------------------------

void ProceduralTerrainAppearanceTemplate::createWaterTableAndRibbonQuadLists (const TerrainGenerator::Layer* const layer)
{
	{
		//-- check boundaries
		int i;
		for (i = 0; i < layer->getNumberOfBoundaries (); i++)
		{
			if (layer->getBoundary (i)->isActive () && layer->getBoundary (i)->getType () == TGBT_polygon)
			{
				const BoundaryPolygon* const boundaryPolygon = safe_cast<const BoundaryPolygon*> (layer->getBoundary (i));

				if (boundaryPolygon->isLocalWaterTable ())
				{
					WaterTable waterTable;
					waterTable.waterType = boundaryPolygon->getWaterType();
					waterTable.boundary = boundaryPolygon;
					waterTable.height   = boundaryPolygon->getLocalWaterTableHeight ();
					m_waterTableList->push_back (waterTable);
				}
			}

			if (layer->getBoundary (i)->isActive () && layer->getBoundary (i)->getType () == TGBT_rectangle)
			{
				const BoundaryRectangle* const boundaryRectangle = safe_cast<const BoundaryRectangle*> (layer->getBoundary (i));

				if (boundaryRectangle->isLocalWaterTable ())
				{
					WaterTable waterTable;
					waterTable.waterType = boundaryRectangle->getWaterType();
					waterTable.boundary = boundaryRectangle;
					waterTable.height   = boundaryRectangle->getLocalWaterTableHeight ();
					m_waterTableList->push_back (waterTable);
				}
			}
		}

		//-- check for ribbon affectors
		for (i = 0; i < layer->getNumberOfAffectors (); i++)
		{
			if (layer->getAffector (i)->isActive () && layer->getAffector (i)->getType () == TGAT_ribbon)
			{
				const AffectorRibbon* const affectorRibbon = safe_cast<const AffectorRibbon*> (layer->getAffector (i));
				// build the individual quads for the ribbon
				
				ArrayList<AffectorRibbon::Quad> ribbonQuadList;
				affectorRibbon->createQuadList (ribbonQuadList);
				if(ribbonQuadList.getNumberOfElements () != 0)
				{
					int j;
					for(j = 0; j < ribbonQuadList.getNumberOfElements (); ++j)
					{
						RibbonQuad ribbonQuad;
						ribbonQuad.waterType = affectorRibbon->getWaterType();
						ribbonQuad.points[0] = ribbonQuadList[j].points[0];
						ribbonQuad.points[1] = ribbonQuadList[j].points[1];
						ribbonQuad.points[2] = ribbonQuadList[j].points[2];
						ribbonQuad.points[3] = ribbonQuadList[j].points[3];
						m_ribbonQuadList->push_back(ribbonQuad);
					}
				}

				// endcap
				const ArrayList<Vector2d>& endCapPointList = affectorRibbon->getEndCapPointList();
				if(endCapPointList.getNumberOfElements() != 0)
				{
					DEBUG_FATAL((endCapPointList.getNumberOfElements() != 8),("ProceduralTerrainAppearanceTemplate::createWaterTableAndRibbonQuadLists - endcap has %d points, needs 8",endCapPointList.getNumberOfElements()));
					
					RibbonEndCap endCap;
					endCap.waterType = affectorRibbon->getWaterType();
					endCap.height = affectorRibbon->getHeightList()[0];
					endCap.extent = affectorRibbon->getEndCapExtent();
					int j;
					for(j = 0; j < endCapPointList.getNumberOfElements(); ++j)
					{
						endCap.points[j].x = endCapPointList[j].x;
						endCap.points[j].y = endCapPointList[j].y;
					}

					m_ribbonEndCapList->push_back(endCap);
				}
			}
		}
	}

	{
		//-- check sublayers
		int i;
		for (i = 0; i < layer->getNumberOfLayers (); i++)
		{
			if (layer->getLayer (i)->isActive ())
			{
				createWaterTableAndRibbonQuadLists (layer->getLayer (i));
			}
		}
	}
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearanceTemplate::createWaterTableAndRibbonQuadLists ()
{
	NOT_NULL (m_terrainGenerator);
	NOT_NULL (m_waterTableList);
	NOT_NULL (m_ribbonQuadList);
	NOT_NULL (m_ribbonEndCapList);

	m_waterTableList->clear ();
	m_ribbonQuadList->clear();
	m_ribbonEndCapList->clear();

	int i;
	for (i = 0; i < m_terrainGenerator->getNumberOfLayers (); i++)
	{
		if (m_terrainGenerator->getLayer (i)->isActive ())
		{
			createWaterTableAndRibbonQuadLists (m_terrainGenerator->getLayer (i));
		}
	}
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearanceTemplate::getWaterHeight (const Vector& position, float& height) const
{
	TerrainGeneratorWaterType waterType;
	return getWaterHeight(position,height,waterType);
}

//-------------------------------------------------------------------

bool ProceduralTerrainAppearanceTemplate::getWaterHeight (const Vector& position, float& height, TerrainGeneratorWaterType& waterType, bool ignoreNonTransparentWater) const
{
	bool valid = false;
	float result = -FLT_MAX;
	waterType = TGWT_invalid;

	if (m_useGlobalWaterTable)
	{
		result = m_globalWaterTableHeight;
		valid = true;	
	}
		
	uint i;
	for (i = 0; i < m_waterTableList->size (); ++i)
	{
		const WaterTable& waterTable = (*m_waterTableList) [i];

		// check if only interested in transparent water
		if(ignoreNonTransparentWater && !WaterTypeManager::getTransparent(waterTable.waterType))
			continue;

		if (waterTable.boundary->isWithin (position.x, position.z))
		{
			if(waterTable.height > result)
			{
				result = waterTable.height;
				waterType = waterTable.waterType;
			}
			valid = true;
		}
	}

	Vector pos = position;
	pos.y = 0.0f;
	
	for (i = 0; i < m_ribbonQuadList->size (); ++i)
	{
		const RibbonQuad& ribbonQuad = (*m_ribbonQuadList) [i];

		// check if only interested in transparent water
		if(ignoreNonTransparentWater && !WaterTypeManager::getTransparent(ribbonQuad.waterType))
			continue;

		const float highHeight = std::max(ribbonQuad.points[0].y,ribbonQuad.points[2].y);
		if(highHeight < result)
			continue;

		float x0 = std::min(std::min(std::min(ribbonQuad.points[0].x,ribbonQuad.points[1].x),ribbonQuad.points[2].x),ribbonQuad.points[3].x);
		float z0 = std::min(std::min(std::min(ribbonQuad.points[0].z,ribbonQuad.points[1].z),ribbonQuad.points[2].z),ribbonQuad.points[3].z);
		float x1 = std::max(std::max(std::max(ribbonQuad.points[0].x,ribbonQuad.points[1].x),ribbonQuad.points[2].x),ribbonQuad.points[3].x);
		float z1 = std::max(std::max(std::max(ribbonQuad.points[0].z,ribbonQuad.points[1].z),ribbonQuad.points[2].z),ribbonQuad.points[3].z);
		
		if(pos.x < x0 || pos.x > x1 || pos.z < z0 || pos.z > z1)
			continue;

		// check the 2 triangles on the quad
		const Vector p0 = Vector(ribbonQuad.points[0].x,0.0f,ribbonQuad.points[0].z);
		const Vector p1 = Vector(ribbonQuad.points[1].x,0.0f,ribbonQuad.points[1].z);
		const Vector p2 = Vector(ribbonQuad.points[2].x,0.0f,ribbonQuad.points[2].z);
		const Vector p3 = Vector(ribbonQuad.points[3].x,0.0f,ribbonQuad.points[3].z);
		
		const bool inPoly1 = pos.inPolygon(p0,p1,p2);
		const bool inPoly2 = !inPoly1 && pos.inPolygon(p0,p2,p3);

		if(inPoly1 || inPoly2)
		{
			const Vector& v0  = ribbonQuad.points[0];
			Vector v1; 
			const Vector& v2 = ribbonQuad.points[2];	
			
			if(inPoly1)
			{
				v1 = ribbonQuad.points[1];
			}
			else
			{
				v1 = ribbonQuad.points[3];
			}

			const Plane plane(v0,v1,v2);
			const float lowHeight = std::min(ribbonQuad.points[0].y,ribbonQuad.points[2].y);
			const Vector start = Vector(pos.x,highHeight + 1.0f,pos.z);
			const Vector end = Vector(pos.x,lowHeight - 1.0f,pos.z);
			
			Vector intersection;
			if(plane.findIntersection(start,end,intersection))
			{
				if(intersection.y > result)
				{
					result = intersection.y;
					waterType = ribbonQuad.waterType;
				}
				valid = true;
			}
		}
	}

	for(i = 0; i < m_ribbonEndCapList->size (); ++i)
	{
		const RibbonEndCap& ribbonEndCap = (*m_ribbonEndCapList)[i];

		// check if only interested in transparent water
		if(ignoreNonTransparentWater && !WaterTypeManager::getTransparent(ribbonEndCap.waterType))
			continue;

		if(ribbonEndCap.extent.isWithin(pos.x,pos.z))
		{
			const Vector p0 = Vector(ribbonEndCap.points[0].x,0.0f,ribbonEndCap.points[0].y);
			const Vector p1 = Vector(ribbonEndCap.points[1].x,0.0f,ribbonEndCap.points[1].y);
			const Vector p2 = Vector(ribbonEndCap.points[2].x,0.0f,ribbonEndCap.points[2].y);
			const Vector p3 = Vector(ribbonEndCap.points[3].x,0.0f,ribbonEndCap.points[3].y);
			const Vector p4 = Vector(ribbonEndCap.points[4].x,0.0f,ribbonEndCap.points[4].y);
			const Vector p5 = Vector(ribbonEndCap.points[5].x,0.0f,ribbonEndCap.points[5].y);
			const Vector p6 = Vector(ribbonEndCap.points[6].x,0.0f,ribbonEndCap.points[6].y);
			const Vector p7 = Vector(ribbonEndCap.points[7].x,0.0f,ribbonEndCap.points[7].y);
   
			// check the triangles in 2 fans
			if(
				pos.inPolygon(p0,p1,p2)
				|| pos.inPolygon(p0,p2,p3)
				|| pos.inPolygon(p0,p3,p4)
				|| pos.inPolygon(p4,p5,p6)
				|| pos.inPolygon(p4,p6,p7)
				|| pos.inPolygon(p4,p7,p0)
			)
			{
				if(ribbonEndCap.height > result)
				{
					result = ribbonEndCap.height;
					waterType = ribbonEndCap.waterType;
				}
				valid = true;
			}
		}
	}

	if (valid)
		height = result;

	return valid;
}

//-------------------------------------------------------------------

TerrainGeneratorWaterType ProceduralTerrainAppearanceTemplate::getWaterType (const Vector& position) const
{
	TerrainGeneratorWaterType waterType = TGWT_invalid;
	float highestTableFound = -FLT_MAX;

	
	// for the tables, it will return find the type of highest table
	uint i;
	for (i = 0; i < m_waterTableList->size (); ++i)
	{
		const WaterTable& waterTable = (*m_waterTableList) [i];

		if (waterTable.boundary->isWithin (position.x, position.z) && waterTable.height > highestTableFound)
		{
			waterType = waterTable.waterType;
			highestTableFound = waterTable.height;
		}
	}

	Vector pos = position;
	pos.y = 0.0f;
	
	// for the ribbons it will return the the first ribbon type found - they shouldn't overlap and should always be above water tables
	for (i = 0; i < m_ribbonQuadList->size (); ++i)
	{
		const RibbonQuad& ribbonQuad = (*m_ribbonQuadList) [i];
		// check the 2 triangles on the quad
		const Vector p0 = Vector(ribbonQuad.points[0].x,0.0f,ribbonQuad.points[0].z);
		const Vector p1 = Vector(ribbonQuad.points[1].x,0.0f,ribbonQuad.points[1].z);
		const Vector p2 = Vector(ribbonQuad.points[2].x,0.0f,ribbonQuad.points[2].z);
		const Vector p3 = Vector(ribbonQuad.points[3].x,0.0f,ribbonQuad.points[3].z);

		if(pos.inPolygon(p0,p1,p2) || pos.inPolygon(p0,p2,p3)
		)
		{
			waterType = ribbonQuad.waterType;	
			break;
		}
	}

	for(i = 0; i < m_ribbonEndCapList->size (); ++i)
	{
		const RibbonEndCap& ribbonEndCap = (*m_ribbonEndCapList)[i];
		if(ribbonEndCap.extent.isWithin(pos.x,pos.z))
		{
			const Vector p0 = Vector(ribbonEndCap.points[0].x,0.0f,ribbonEndCap.points[0].y);
			const Vector p1 = Vector(ribbonEndCap.points[1].x,0.0f,ribbonEndCap.points[1].y);
			const Vector p2 = Vector(ribbonEndCap.points[2].x,0.0f,ribbonEndCap.points[2].y);
			const Vector p3 = Vector(ribbonEndCap.points[3].x,0.0f,ribbonEndCap.points[3].y);
			const Vector p4 = Vector(ribbonEndCap.points[4].x,0.0f,ribbonEndCap.points[4].y);
			const Vector p5 = Vector(ribbonEndCap.points[5].x,0.0f,ribbonEndCap.points[5].y);
			const Vector p6 = Vector(ribbonEndCap.points[6].x,0.0f,ribbonEndCap.points[6].y);
			const Vector p7 = Vector(ribbonEndCap.points[7].x,0.0f,ribbonEndCap.points[7].y);
			
			// check the triangles in 2 fans
			if(
				pos.inPolygon(p0,p1,p2)
				|| pos.inPolygon(p0,p2,p3)
				|| pos.inPolygon(p0,p3,p4)
				|| pos.inPolygon(p4,p5,p6)
				|| pos.inPolygon(p4,p6,p7)
				|| pos.inPolygon(p4,p7,p0)
			)
			{
				waterType = ribbonEndCap.waterType;	
				break;
			}
		}
	}

	return waterType;
}

//-------------------------------------------------------------------


int ProceduralTerrainAppearanceTemplate::getStaticCollidableFloraFamily(int floraTileX, int floraTileY) const
{
	return (m_staticCollidableFloraMap) ? m_staticCollidableFloraMap->getValue(floraTileX, floraTileY) : int(0);
}

//-------------------------------------------------------------------

float ProceduralTerrainAppearanceTemplate::getStaticCollidableFloraHeight(int floraTileX, int floraTileY) const
{
	return (m_staticCollidableFloraHeightMap) ? m_staticCollidableFloraHeightMap->getValue(floraTileX, floraTileY) : float(0);
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearanceTemplate::_setStaticCollidableFloraInfo(FloraGroup::Info *infoMap, float *heightMap)
{
	const int mapWidthInFlora = getMapWidthInFlora();
	const int numElements = sqr(mapWidthInFlora);

	if (infoMap)
	{
		if (m_staticCollidableFloraMap)
		{
			delete m_staticCollidableFloraMap;
			m_staticCollidableFloraMap=0;
		}

		int *familiyIds = new int[numElements];
		for (int i=0;i<numElements;i++)
		{
			familiyIds[i] = infoMap[i].getFamilyId();
		}

		m_staticCollidableFloraMap = new PackedIntegerMap(mapWidthInFlora, mapWidthInFlora, familiyIds);

		/*
		for (int z=0;z<mapWidthInFlora;z++)
		{
			for (int x=0;x<mapWidthInFlora;x++)
			{
				int v = pim->getValue(x, z);
				DEBUG_FATAL(v!=temp[z*mapWidthInFlora + x], (""));
			}
		}
		*/

		delete [] familiyIds;

	}

	if (heightMap)
	{
		if (m_staticCollidableFloraHeightMap)
		{
			delete m_staticCollidableFloraHeightMap;
			m_staticCollidableFloraHeightMap=0;
		}

		const float fixedPointResolution = 1.0f / 32.0f;
		m_staticCollidableFloraHeightMap = new PackedFixedPointMap(mapWidthInFlora, mapWidthInFlora, fixedPointResolution, heightMap);

		/*
		for (int z=0;z<mapWidthInFlora;z++)
		{
			for (int x=0;x<mapWidthInFlora;x++)
			{
				float pv = pfpm->getValue(x, z);
				float ov = m_staticCollidableFloraHeightSamples[z*mapWidthInFlora + x];

				float diff = fabsf(pv - ov);
				DEBUG_FATAL(diff>.0101, (""));

			}
		}
		*/
	}
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearanceTemplate::_destroyStaticCollidableFloraMaps()
{
	if (m_staticCollidableFloraMap)
	{
		delete m_staticCollidableFloraMap;
		m_staticCollidableFloraMap=0;
	}
	if (m_staticCollidableFloraHeightMap)
	{
		delete m_staticCollidableFloraHeightMap;
		m_staticCollidableFloraHeightMap=0;
	}
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearanceTemplate::_createDefaultStaticCollidableFloraMaps()
{
	_destroyStaticCollidableFloraMaps();
	const int floraWidth=getMapWidthInFlora();
	m_staticCollidableFloraMap = new PackedIntegerMap(floraWidth, floraWidth, 0);
	m_staticCollidableFloraHeightMap = new PackedFixedPointMap(floraWidth, floraWidth, 1, 0);
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearanceTemplate::load (Iff& iff)
{
	if (iff.getCurrentName () != TAG (M,P,T,A) && iff.getCurrentName () != TAG (P,T,A,T))
		return;

	iff.enterForm ();

		//DEBUG_WARNING (iff.getCurrentName () < TAG_0015, ("ProceduralTerrainAppearanceTemplate [%s]: loading older version.  Please load and save this file in the TerrainEditor.", getCrcName ().getString ()));

		const Tag version = iff.getCurrentName();
		if (version>=TAG_0013 && version<=TAG_0015)
		{
			_load(iff, version);
		}
		else
		{
			char tagBuffer [5];
			ConvertTagToString(version, tagBuffer);

			char buffer [128];
			iff.formatLocation(buffer, sizeof (buffer));
			
			FATAL(true, ("ProceduralTerrainAppearanceTemplate::load - unsupported version tag %s/%s", buffer, tagBuffer));
		}

	iff.exitForm (false);

	// ------------------------------------
	m_mapWidthInFlora = (m_floraTileWidthInMeters) ? 
		  int(m_mapWidthInMeters / m_floraTileWidthInMeters)
		: int(0);

	if ( !m_staticCollidableFloraMap 
		|| m_staticCollidableFloraMap->getWidth()<getMapWidthInFlora()
		)
	{
		//DEBUG_WARNING(true, ("Create default empty flora maps for %s.\n", m_name));
		_createDefaultStaticCollidableFloraMaps();
	}
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearanceTemplate::_load(Iff& iff, Tag version)
{
	iff.enterForm(version);

		iff.enterChunk (TAG_DATA);

			m_legacyMap = true;

			//----------------------

			m_name                               = iff.read_string ();
			m_mapWidthInMeters                   = iff.read_float ();
			m_chunkWidthInMeters                 = iff.read_float ();
			m_numberOfTilesPerChunk              = iff.read_int32 ();
			m_useGlobalWaterTable                = iff.read_int32 () != 0;
			m_globalWaterTableHeight             = iff.read_float ();
			m_globalWaterTableShaderSize         = iff.read_float ();
			m_globalWaterTableShaderTemplateName = iff.read_string ();
			m_environmentCycleTime               = iff.read_float ();

			if (version==TAG_0013)
			{
				std::string dummyString;
				iff.read_string(dummyString);
				iff.read_string(dummyString);
				IGNORE_RETURN(iff.read_float());
				iff.read_string(dummyString);
				IGNORE_RETURN(iff.read_float());
				iff.read_string(dummyString);
				IGNORE_RETURN(iff.read_float());
				iff.read_string(dummyString);
				IGNORE_RETURN(iff.read_float());
				IGNORE_RETURN(iff.read_int32());
				iff.read_string(dummyString);
			}

			m_collidableMinimumDistance          = iff.read_float ();
			m_collidableMaximumDistance          = iff.read_float ();
			m_collidableTileSize                 = iff.read_float ();
			m_collidableTileBorder               = iff.read_float ();
			m_collidableSeed                     = iff.read_uint32 ();
			m_nonCollidableMinimumDistance       = iff.read_float ();
			m_nonCollidableMaximumDistance       = iff.read_float ();
			m_nonCollidableTileSize              = iff.read_float ();
			m_nonCollidableTileBorder            = iff.read_float ();
			m_nonCollidableSeed                  = iff.read_uint32 ();
			m_radialMinimumDistance              = iff.read_float ();
			m_radialMaximumDistance              = iff.read_float ();
			m_radialTileSize                     = iff.read_float ();
			m_radialTileBorder                   = iff.read_float ();
			m_radialSeed                         = iff.read_uint32 ();
			m_farRadialMinimumDistance           = iff.read_float ();
			m_farRadialMaximumDistance           = iff.read_float ();
			m_farRadialTileSize                  = iff.read_float ();
			m_farRadialTileBorder                = iff.read_float ();
			m_farRadialSeed                      = iff.read_uint32 ();

			// --------------------
			if (version>=TAG_0015)
			{
				m_legacyMap = iff.read_bool8();
			}

		iff.exitChunk (TAG_DATA, true);

		m_tileWidthInMeters = m_chunkWidthInMeters / m_numberOfTilesPerChunk;

		//-- get the generator from the loader
		NOT_NULL (!m_terrainGenerator);
		m_terrainGenerator = new TerrainGenerator ();
		m_terrainGenerator->load (iff);

		NOT_NULL (!m_bakedTerrain);
		m_bakedTerrain = new BakedTerrain ();
		m_bakedTerrain->load (iff);

		if (version>=TAG_0015)
		{
			m_staticCollidableFloraMap       = new PackedIntegerMap(iff);
			m_staticCollidableFloraHeightMap = new PackedFixedPointMap(iff);
		}

	iff.exitForm (version);
}

//===================================================================

ProceduralTerrainAppearanceTemplate::WriterData::WriterData () :
	name (0),
	mapWidthInMeters (1024),
	chunkWidthInMeters (32),
	numberOfTilesPerChunk (8),
	useGlobalWaterTable (false),
	globalWaterTableHeight (0),
	globalWaterTableShaderSize (2.f),
	globalWaterTableShaderTemplateName (0),
	environmentCycleTime (60.f),
	collidableMinimumDistance (0.f),
	collidableMaximumDistance (0.f),
	collidableTileSize (0.f),
	collidableTileBorder (0.f),
	collidableSeed (0),
	nonCollidableMinimumDistance (0.f),
	nonCollidableMaximumDistance (0.f),
	nonCollidableTileSize (0.f),
	nonCollidableTileBorder (0.f),
	nonCollidableSeed (0),
	radialMinimumDistance (0.f),
	radialMaximumDistance (0.f),
	radialTileSize (0.f),
	radialTileBorder (0.f),
	radialSeed (0),
	farRadialMinimumDistance (0.f),
	farRadialMaximumDistance (0.f),
	farRadialTileSize (0.f),
	farRadialTileBorder (0.f),
	farRadialSeed (0),
	terrainGenerator(0),
	bakedTerrain(0),
	staticCollidableFloraMap(0),
	staticCollidableFloraHeightMap(0)
{
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearanceTemplate::prepareWriterData(ProceduralTerrainAppearanceTemplate::WriterData &data)
{
	data.name                                 = m_name;
	data.mapWidthInMeters                     = m_mapWidthInMeters;
	data.chunkWidthInMeters                   = m_chunkWidthInMeters;
	data.numberOfTilesPerChunk                = m_numberOfTilesPerChunk;
	data.useGlobalWaterTable                  = m_useGlobalWaterTable;
	data.globalWaterTableHeight               = m_globalWaterTableHeight;
	data.globalWaterTableShaderSize           = m_globalWaterTableShaderSize;
	data.globalWaterTableShaderTemplateName   = m_globalWaterTableShaderTemplateName;
	data.environmentCycleTime                 = m_environmentCycleTime;
	data.collidableMinimumDistance            = m_collidableMinimumDistance;
	data.collidableMaximumDistance            = m_nonCollidableMaximumDistance;
	data.collidableTileSize                   = m_collidableTileSize;
	data.collidableTileBorder                 = m_collidableTileBorder;
	data.collidableSeed                       = m_collidableSeed;
	data.nonCollidableMinimumDistance         = m_nonCollidableMinimumDistance;
	data.nonCollidableMaximumDistance         = m_nonCollidableMaximumDistance;
	data.nonCollidableTileSize                = m_nonCollidableTileSize;
	data.nonCollidableTileBorder              = m_nonCollidableTileBorder;
	data.nonCollidableSeed                    = m_nonCollidableSeed;
	data.radialMinimumDistance                = m_radialMinimumDistance;
	data.radialMaximumDistance                = m_radialMaximumDistance;
	data.radialTileSize                       = m_radialTileSize;
	data.radialTileBorder                     = m_radialTileBorder;
	data.radialSeed                           = m_radialSeed;
	data.farRadialMinimumDistance             = m_farRadialMinimumDistance;
	data.farRadialMaximumDistance             = m_farRadialMaximumDistance;
	data.farRadialTileSize                    = m_farRadialTileSize;
	data.farRadialTileBorder                  = m_farRadialTileBorder;
	data.farRadialSeed                        = m_farRadialSeed;
	data.legacyMap                            = m_legacyMap;
	data.terrainGenerator                     = getTerrainGenerator();
	data.bakedTerrain                         = getBakedTerrain();
	data.staticCollidableFloraMap             = m_staticCollidableFloraMap;
	data.staticCollidableFloraHeightMap       = m_staticCollidableFloraHeightMap;
}

//-------------------------------------------------------------------

void ProceduralTerrainAppearanceTemplate::write(Iff& iff, const ProceduralTerrainAppearanceTemplate::WriterData& data)
{
	iff.insertForm(TAG (P,T,A,T));

		iff.insertForm(TAG_0015);

			iff.insertChunk(TAG_DATA);

				iff.insertChunkString (data.name ? data.name : "");
				iff.insertChunkData (data.mapWidthInMeters);
				iff.insertChunkData (data.chunkWidthInMeters);
				iff.insertChunkData (data.numberOfTilesPerChunk);
				iff.insertChunkData (data.useGlobalWaterTable ? static_cast<int32> (1) : static_cast<int32> (0));
				iff.insertChunkData (data.globalWaterTableHeight);
				iff.insertChunkData (data.globalWaterTableShaderSize);
				iff.insertChunkString (data.globalWaterTableShaderTemplateName ? data.globalWaterTableShaderTemplateName : "");
				iff.insertChunkData (data.environmentCycleTime);
				iff.insertChunkData (data.collidableMinimumDistance);
				iff.insertChunkData (data.collidableMaximumDistance);
				iff.insertChunkData (data.collidableTileSize);
				iff.insertChunkData (data.collidableTileBorder);
				iff.insertChunkData (data.collidableSeed);
				iff.insertChunkData (data.nonCollidableMinimumDistance);
				iff.insertChunkData (data.nonCollidableMaximumDistance);
				iff.insertChunkData (data.nonCollidableTileSize);
				iff.insertChunkData (data.nonCollidableTileBorder);
				iff.insertChunkData (data.nonCollidableSeed);
				iff.insertChunkData (data.radialMinimumDistance);
				iff.insertChunkData (data.radialMaximumDistance);
				iff.insertChunkData (data.radialTileSize);
				iff.insertChunkData (data.radialTileBorder);
				iff.insertChunkData (data.radialSeed);
				iff.insertChunkData (data.farRadialMinimumDistance);
				iff.insertChunkData (data.farRadialMaximumDistance);
				iff.insertChunkData (data.farRadialTileSize);
				iff.insertChunkData (data.farRadialTileBorder);
				iff.insertChunkData (data.farRadialSeed);
				iff.insertChunkData (uint8(data.legacyMap ? 1 : 0));

			iff.exitChunk(TAG_DATA);

			// -------------------------------
			NOT_NULL(data.terrainGenerator);
			data.terrainGenerator->save(iff);
			// -------------------------------

			// -------------------------------
			NOT_NULL(data.bakedTerrain);
			data.bakedTerrain->save(iff);
			// -------------------------------

			// -------------------------------
			if (data.staticCollidableFloraMap)
			{
				data.staticCollidableFloraMap->save(iff);
			}
			else
			{
				PackedIntegerMap fm(0, 0, 0);
				fm.save(iff);
			}
			// -------------------------------

			// -------------------------------
			if (data.staticCollidableFloraHeightMap)
			{
				data.staticCollidableFloraHeightMap->save(iff);
			}
			else
			{
				PackedFixedPointMap hm(0, 0, 0, 0);
				hm.save(iff);
			}
			// -------------------------------

		iff.exitForm(TAG_0015);

	iff.exitForm(TAG(P,T,A,T), false);
}

//===================================================================
