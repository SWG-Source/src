//===================================================================
//
// ProceduralTerrainAppearanceTemplate.h
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_ProceduralTerrainAppearanceTemplate_H
#define INCLUDED_ProceduralTerrainAppearanceTemplate_H

//===================================================================

#include "sharedMath/Vector2d.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedTerrain/TerrainGenerator.h"
#include "sharedTerrain/Boundary.h"

class BakedTerrain;
class Iff;
class PackedIntegerMap;
class PackedFixedPointMap;

//===================================================================

class ProceduralTerrainAppearanceTemplate : public AppearanceTemplate
{
public:

	explicit ProceduralTerrainAppearanceTemplate(
		const char* filename, 
		Iff* iff, 
		bool legacyMode, 
		int  chunkOriginOffset, // size of sample pad on lower side of chunk.
		int  chunkUpperPad,     // size of sample pad on upper side of chunk.
		bool samplingMode=false
	);

	virtual ~ProceduralTerrainAppearanceTemplate ()=0;

	const char*             getName () const;
	float                   getMapWidthInMeters () const;
	float                   getChunkWidthInMeters () const;
	int                     getNumberOfTilesPerChunk () const;
	bool                    getUseGlobalWaterTable () const;
	float                   getGlobalWaterTableHeight () const;
	const char*             getGlobalWaterTableShaderTemplateName () const;
	float                   getGlobalWaterTableShaderSize () const;
	float                   getEnvironmentCycleTime () const;
	float                   getCollidableMinimumDistance () const;
	float                   getCollidableMaximumDistance () const;
	float                   getCollidableTileSize () const;
	float                   getCollidableTileBorder () const;
	uint32                  getCollidableSeed () const;
	float                   getNonCollidableMinimumDistance () const;
	float                   getNonCollidableMaximumDistance () const;
	float                   getNonCollidableTileSize () const;
	float                   getNonCollidableTileBorder () const;
	uint32                  getNonCollidableSeed () const;
	float                   getRadialMinimumDistance () const;
	float                   getRadialMaximumDistance () const;
	float                   getRadialTileSize () const;
	float                   getRadialTileBorder () const;
	uint32                  getRadialSeed () const;
	float                   getFarRadialMinimumDistance () const;
	float                   getFarRadialMaximumDistance () const;
	float                   getFarRadialTileSize () const;
	float                   getFarRadialTileBorder () const;
	uint32                  getFarRadialSeed () const;
	float                   getTileWidthInMeters () const;

	bool                    getLegacyMap() const { return m_legacyMap; }
	bool                    getLegacyMode() const { return m_legacyMode; }

	int                     getChunkOriginOffset() const { return m_chunkOriginOffset; }
	int                     getChunkUpperPad() const { return m_chunkUpperPad; }

	const TerrainGenerator* getTerrainGenerator () const;
	const BakedTerrain*     getBakedTerrain () const;

	bool                    getWaterHeight (const Vector& position, float& height) const;
	bool					getWaterHeight (const Vector& position, float& height, TerrainGeneratorWaterType& waterType, bool ignoreNonTransparentWater = false) const;
	TerrainGeneratorWaterType getWaterType (const Vector& position_w) const;

	// --

	float                   getFloraTileWidthInMeters() const { return m_floraTileWidthInMeters; }
	int                     getMapWidthInFlora() const { return m_mapWidthInFlora; }
	int                     getStaticCollidableFloraFamily(int floraTileX, int floraTileY) const;
	float                   getStaticCollidableFloraHeight(int floraTileX, int floraTileY) const;

protected:

	struct RibbonQuad
	{
		TerrainGeneratorWaterType waterType;
		Vector points[4];
	};

	struct RibbonEndCap
	{
		TerrainGeneratorWaterType waterType;
		Vector2d points[8];
		Rectangle2d extent;
		float height;
	};

	struct WaterTable
	{
		TerrainGeneratorWaterType waterType;
		const TerrainGenerator::Boundary* boundary;
		float                             height;
	};

protected:

	char*                   m_name;
	float                   m_mapWidthInMeters;
	float                   m_chunkWidthInMeters;
	int                     m_numberOfTilesPerChunk;
	bool                    m_useGlobalWaterTable;
	float                   m_globalWaterTableHeight;
	char*                   m_globalWaterTableShaderTemplateName;
	float                   m_globalWaterTableShaderSize;
	float                   m_environmentCycleTime;
	float                   m_collidableMinimumDistance;
	float                   m_collidableMaximumDistance;
	float                   m_collidableTileSize;
	float                   m_collidableTileBorder;
	uint32                  m_collidableSeed;
	float                   m_nonCollidableMinimumDistance;
	float                   m_nonCollidableMaximumDistance;
	float                   m_nonCollidableTileSize;
	float                   m_nonCollidableTileBorder;
	uint32                  m_nonCollidableSeed;
	float                   m_radialMinimumDistance;
	float                   m_radialMaximumDistance;
	float                   m_radialTileSize;
	float                   m_radialTileBorder;
	uint32                  m_radialSeed;
	float                   m_farRadialMinimumDistance;
	float                   m_farRadialMaximumDistance;
	float                   m_farRadialTileSize;
	float                   m_farRadialTileBorder;
	uint32                  m_farRadialSeed;
	bool                    m_legacyMap;

	bool                    m_legacyMode;
	const bool              m_samplingMode;

	float                   m_tileWidthInMeters;

	int                     m_chunkOriginOffset;
	int                     m_chunkUpperPad;

	TerrainGenerator*       m_terrainGenerator;
	BakedTerrain*           m_bakedTerrain;

	typedef std::vector<WaterTable> WaterTableList;
	WaterTableList*         m_waterTableList;

	typedef std::vector<RibbonQuad> RibbonQuadList;
	RibbonQuadList*         m_ribbonQuadList;

	typedef std::vector<RibbonEndCap> RibbonEndCapList;
	RibbonEndCapList*         m_ribbonEndCapList;

	float                   m_floraTileWidthInMeters;
	int                     m_mapWidthInFlora;
	PackedIntegerMap       *m_staticCollidableFloraMap;
	PackedFixedPointMap    *m_staticCollidableFloraHeightMap;

protected:

	void        load (Iff& iff);  //lint !e1511 //-- member hides non-virtual member
	void        _load(Iff& iff, Tag version);
	void        createWaterTableAndRibbonQuadLists ();
	void        createWaterTableAndRibbonQuadLists (const TerrainGenerator::Layer* layer);
	void        _setSamplingParameters(bool legacyMode, int chunkOriginOffset, int chunkUpperPad);
	void        _setStaticCollidableFloraInfo(FloraGroup::Info *infoMap, float *heightMap);
	void        _destroyStaticCollidableFloraMaps();
	void        _createDefaultStaticCollidableFloraMaps();

private:

	ProceduralTerrainAppearanceTemplate ();
	ProceduralTerrainAppearanceTemplate (const ProceduralTerrainAppearanceTemplate&);
	ProceduralTerrainAppearanceTemplate& operator= (const ProceduralTerrainAppearanceTemplate&);

	// ------------------------------------------------------------------

public:

	struct WriterData
	{
	public:

		const char* name;
		float       mapWidthInMeters;
		float       chunkWidthInMeters;
		int         numberOfTilesPerChunk;
		bool        useGlobalWaterTable;
		float       globalWaterTableHeight;
		float       globalWaterTableShaderSize;
		const char* globalWaterTableShaderTemplateName;
		float       environmentCycleTime;
		float       collidableMinimumDistance;
		float       collidableMaximumDistance;
		float       collidableTileSize;
		float       collidableTileBorder;
		uint32      collidableSeed;
		float       nonCollidableMinimumDistance;
		float       nonCollidableMaximumDistance;
		float       nonCollidableTileSize;
		float       nonCollidableTileBorder;
		uint32      nonCollidableSeed;
		float       radialMinimumDistance;
		float       radialMaximumDistance;
		float       radialTileSize;
		float       radialTileBorder;
		uint32      radialSeed;
		float       farRadialMinimumDistance;
		float       farRadialMaximumDistance;
		float       farRadialTileSize;
		float       farRadialTileBorder;
		uint32      farRadialSeed;
		bool        legacyMap;

		// ------------------------------------

		const TerrainGenerator *terrainGenerator;
		const BakedTerrain *bakedTerrain;
		const PackedIntegerMap *staticCollidableFloraMap;
		const PackedFixedPointMap *staticCollidableFloraHeightMap;

	public:

		WriterData();
	};

	void  prepareWriterData(WriterData &writerData);

	static void write (Iff& iff, const WriterData& data);
}; 

//===================================================================

#endif
