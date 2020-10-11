//===================================================================
//
// ProceduralTerrainAppearance.h
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_ProceduralTerrainAppearance_H
#define INCLUDED_ProceduralTerrainAppearance_H

//===================================================================

#include "sharedCollision/BoxExtent.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Vector.h"
#include "sharedTerrain/TerrainAppearance.h"
#include "sharedTerrain/TerrainGenerator.h"

class Appearance;
class Camera;
class Iff;
class Object;
class ObjectList;
class ObjectTemplate;
class ProceduralTerrainAppearanceTemplate;
class Rectangle2d;
class SpatialSubdivisionHandle;

//-------------------------------------------------------------------
//
// ProceduralTerrainAppearance holds a list of generated terrain chunks 
//
// data for the template is as follows:
//
//     mapWidthInMeters        supplied by designers/artists (i.e. 4096m x 4096m)
//
//     chunkWidthInMeters      supplied by designers/artists (i.e. 32m)
//
//     tileWidthInMeters       width of a terrain tile in meters
//                             tiles are composed of a fan of 8 polygons
//
//     numberOfTilesPerChunk   number of tiles per chunk for width and height (square)
//                             total number of tiles per chunk is numberOfTilesPerChunk^2
//
//     numberOfTiles           along width = numberOfChunks * numberOfTilesPerChunk
//                             total number of tiles = numberOfTiles^2
// 
// tiles are organized in the following vertex structure. the purpose of
// indirection is to allow me to create the tile vertex arrays in a loop
//
//      2     3     4
//      x-----x-----x
//      |\    |    /|
//      |  \  |  /  |
//      |   0\|/    |
//    1 x-----x-----x 5
//      |    /|\    |
//      |  /  |  \  |
//      |/    |    \|
//      x-----x-----x
//      8     7     6
//

class ProceduralTerrainAppearance : public TerrainAppearance
{
public:

	typedef void (*CreateFloraHookFunction) (Object& object);

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// CreateChunkData defines the data needed to generate a chunk in the terrain system
	//
	struct CreateChunkData
	{
	public:

		//-- scratchpad (using TerrainGenerator::createChunkBuffer)
		TerrainGenerator::CreateChunkBuffer* createChunkBuffer;

		//-- coordinate of chunk (x,z) in chunk space
		int                                  chunkX;
		int                                  chunkZ;

		//-- number of shader tiles
		int                                  numberOfTilesPerChunk;

		//-- chunk width
		float                                chunkWidthInMeters;
		float                                tileWidthInMeters;

		Vector                               start;

		int                                  originOffset;
		int                                  numberOfPoles;

		//--
		const ShaderGroup*                   shaderGroup;
		const FloraGroup*                    floraGroup;
		const RadialGroup*                   radialGroup;
		const EnvironmentGroup*              environmentGroup;
		const FractalGroup*                  fractalGroup;
		const BitmapGroup*                   bitmapGroup;

		uint8                                hasLargerNeighborFlags;

	private:

		CreateChunkData ();
		CreateChunkData (const CreateChunkData& rhs);
		CreateChunkData& operator= (const CreateChunkData& rhs);

	public:

		explicit CreateChunkData (TerrainGenerator::CreateChunkBuffer* newCreateChunkBuffer);
		virtual ~CreateChunkData ();

		virtual void validate () const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// SurfaceData returns the object template at a given tile of the terrain system
	//
	struct SurfaceData
	{
	public:

		const ObjectTemplate* objectTemplate;

	public:

		SurfaceData ();
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// StaticFloraData defines information about what flora exists at a point
	//
	struct StaticFloraData
	{
	public:

		//-- family data
		bool        floats;
		float       childChoice;

		//-- family child data
		FloraGroup::FamilyChildData const * familyChildData;

	public:

		StaticFloraData ();
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// Chunk defines a chunk of terrain in the terrain system
	//
	class Chunk : public Appearance
	{
	public:

		explicit Chunk (ProceduralTerrainAppearance& proceduralTerrainAppearance);
		virtual ~Chunk ();

		const ProceduralTerrainAppearanceTemplate *getAppearanceTemplate() const { return m_proceduralTerrainAppearance.proceduralTerrainAppearanceTemplate; }

		virtual bool              collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const = 0;
		virtual bool              getHeightAt (const Vector& pos, float* height) const = 0;
		virtual bool              getHeightAt (const Vector& pos, float* height, Vector* normal) const = 0;

		const BoxExtent&          getBoxExtent() const                     { return m_boxExtent; }

		//-- coordinate of chunk (x,z) in chunk space
		int                       getChunkX() const                        { return chunkX; }
		int                       getChunkZ() const                        { return chunkZ; }

		float                     getChunkWidthInMeters () const           { return chunkWidthInMeters; }

		bool                      getHasLargerNeighbor (int dir) const;
		bool                      getHasAnyLargerNeighbors () const        { return hasLargerNeighborFlags!=0; }

		bool                      findSurface (const Vector& position, SurfaceData& data) const;
		void                      setPassable(int tileX, int tileZ, bool isPassable);
		bool                      isPassable(const Vector& position) const;

		bool                      isExcluded(const Vector& position) const;
		bool                      isExcluded(int tileX, int tileZ) const;

		bool                      floraAllowed(const Vector& position) const  { return _floraAllowed(position.x, position.z); }
		bool                      findStaticCollidableFlora (const Vector& position, StaticFloraData& data, bool& floraAllowed) const;
		bool                      findStaticCollidableFlora (const Vector& position, FloraGroup::Info& data, bool& floraAllowed) const;

		virtual int               getChunkMemorySize () const;

#ifdef _DEBUG
		virtual bool              debugRenderingEnabled () const;
#endif

		SpatialSubdivisionHandle *getSpatialSubdivisionHandle() const                             { return m_spatialSubdivisionHandle; }
		void                      setSpatialSubdivisionHandle(SpatialSubdivisionHandle *h) const  { m_spatialSubdivisionHandle = h; }

	public:

		static void               setDrawVertexNormals (bool drawVertexNormals);
		static bool               getDrawVertexNormals ();
		static void               setDrawExtent (bool drawExtent);
		static bool               getDrawExtent ();

	protected:

		void                      _findMapXz (const Vector& position, int& x, int& z) const;
		void                      _findTileXz(Vector const & position, int & x, int & z) const;

		void                      _makeStaticFloraData(StaticFloraData& o_data, const FloraGroup::Info &i_groupInfo) const;
		bool                      _floraAllowed(float positionX, float positionZ) const;
		bool                      _findStaticFlora(const Array2d<FloraGroup::Info>& floraMap, float positionX, float positionZ, StaticFloraData& data, bool& floraAllowed) const;
		bool                      _findStaticFlora(const Array2d<FloraGroup::Info>& floraMap, float positionX, float positionZ, FloraGroup::Info &data, bool& floraAllowed) const;
		void                      _prepareForDelete();

		void                      _setExcluded(int tileX, int tileZ);

	protected:

		ProceduralTerrainAppearance & m_proceduralTerrainAppearance;

		//-- coordinate of chunk (x,z) in chunk space
		int                       chunkX;
		int                       chunkZ;

		float                     chunkWidthInMeters;
		uint8                     hasLargerNeighborFlags;

		int                       originOffset;
		int                       numberOfPoles;

		BoxExtent                 m_boxExtent;

		Array2d<ShaderGroup::Info>* shaderMap;
		Array2d<FloraGroup::Info> * m_floraStaticCollidableMap;

		unsigned m_excluded, m_passable;

		mutable SpatialSubdivisionHandle * m_spatialSubdivisionHandle;

	private:

		Chunk ();
		Chunk (const Chunk& rhs);
		Chunk& operator= (const Chunk& rhs);

	private:

		static bool               ms_drawVertexNormals;
		static bool               ms_drawExtent;
	};

	friend class Chunk;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	ProceduralTerrainAppearance (const ProceduralTerrainAppearanceTemplate* appearanceTemplate);
	virtual ~ProceduralTerrainAppearance () = 0;

	virtual const Sphere& getSphere () const;
	virtual bool          getHeight (const Vector& position_o, float& height) const;
	virtual bool          getHeight (const Vector& position_o, float& height, Vector& normal) const;
	virtual bool          getHeightForceChunkCreation (const Vector& position_o, float& height) const;
	virtual const ObjectTemplate* getSurfaceProperties (const Vector& position_o) const;
	virtual bool          getWaterHeight (const Vector& position_o, float& height) const;
	virtual bool          getWaterHeight (const Vector& position_o, float& height, TerrainGeneratorWaterType& waterType, bool ignoreNonTransparentWater=false) const;
	TerrainGeneratorWaterType getWaterType (const Vector& position_o) const;
	virtual bool          getWater (int chunkX, int chunkZ) const;
	virtual bool          getSlope (int chunkX, int chunkZ) const;
	virtual bool          getWater (const Rectangle2d& rectangle) const;
	virtual bool          getSlope (const Rectangle2d& rectangle) const;
	virtual float         getChunkWidthInMeters () const;
	virtual float         getMapWidthInMeters () const;
	virtual float         getEnvironmentCycleTime () const;
	virtual int           getNumberOfChunks () const = 0;
	virtual int           calculateChunkX (float positionX) const;
	virtual int           calculateChunkZ (float positionZ) const;
	virtual float         getChunkHeight (int chunkX, int chunkZ) const;
	virtual const BoxExtent* getChunkExtent (const Vector& position_o) const;
	virtual const BoxExtent* getChunkExtentForceChunkCreation (const Vector& position_o) const;
	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParamters, CollisionInfo & result) const = 0;
	virtual float         alter (float time) = 0;

	int                   getNumberOfTilesPerChunk () const;

	bool                  findSurface (const Vector& position, SurfaceData& data) const;
	bool                  findStaticCollidableFlora (const Vector& position, StaticFloraData& data, bool& floraAllowed) const;
	virtual bool          isPassable(const Vector& position) const;
	virtual bool          isPassableForceChunkCreation(const Vector& position) const;

	//-- run-time rule addition interface
	void                  addLayer (const TerrainGenerator::Layer* layer);
	void                  removeLayer (const TerrainGenerator::Layer* layer);
	void                  prepareGenerator ();
	float                 generateHeight_expensive (const Vector2d& position_w);

	virtual void          addClearCollidableFloraObject (const Object* object, const Vector& position_w, float radius);
	virtual void          removeClearCollidableFloraObject (const Object* object);

	virtual void          debugDump () const;

	ShaderGroup const & getShaderGroup() const;
	FloraGroup const & getFloraGroup() const;
	RadialGroup const & getRadialGroup() const;
	EnvironmentGroup const & getEnvironmentGroup() const;
	virtual bool hasPassableAffectors() const;

public:

	static void           install ();

	static void           setCreateFloraHookFunction (CreateFloraHookFunction createFloraHookFunction);
	static void           setMaximumNumberOfChunksAllowed (int newMaximumNumberOfChunksAllowed);

protected:

	class ClearCollidableFloraEntry
	{
	public:

		Vector position;
		float  radius;
	};

protected:

	typedef std::map<const Object*, ClearCollidableFloraEntry> ClearCollidableFloraMap;

protected:

	virtual const Chunk*  findChunk (int x, int z, int chunkSize) const = 0;
	virtual const Chunk*  findFirstRenderableChunk (int x, int z) const = 0;
	virtual const Chunk*  findAnyChunk () const;
	virtual void          addChunk (Chunk* chunk, int chunkSize) = 0;
	virtual void          createChunk (int x, int z, int chunkSize, unsigned hasLargerNeighborFlags) = 0;
	virtual void          removeUnnecessaryChunk () = 0;

	bool                  areValidChunkIndices (int x, int z) const;
	const Chunk*          findChunk (const Vector& position, int chunkSize) const;
	const Chunk*          findFirstRenderableChunk (const Vector& position) const;
	const Chunk*          findFirstRenderableChunk2D(float positionX, float positionZ) const;

	void                  createChunk (const Vector& position, int chunkSize);
	virtual void          prepareForDelete (const Chunk* chunk);

	bool                  _legacyGetStaticCollidableFloraData(StaticFloraData &o_data, const Chunk* const chunk, const Vector &floraPosition);
	void                  _legacyCreateFlora (const Chunk* chunk);
	void                  createFlora (const Chunk* chunk);
	void                  destroyFlora (const Chunk* chunk);

	void                  verifyChunk (const Chunk* chunk) const;

	void                  setClient ();

	const ClearCollidableFloraMap* getClearCollidableFloraMap () const;

protected:

	//-- 
	const int             originOffset;
	const int             upperPad;
	const int             numberOfPoles;

	//-- culling sphere
	mutable Sphere        sphere;

	//-- scratchpad for creating chunks
	TerrainGenerator::CreateChunkBuffer createChunkBuffer;

	//-- scratchpad for [expensively] creating just height data
	TerrainGenerator::CreateChunkBuffer generateHeightChunkBuffer;

	//-- maximum number of chunks to hold
	int                   maximumNumberOfChunksAllowed;
	int                   maximumNumberOfChunksAlongSide;

	const ProceduralTerrainAppearanceTemplate* proceduralTerrainAppearanceTemplate;

	typedef std::vector<TerrainGenerator::Layer*> RunTimeRuleList;
	RunTimeRuleList*      m_runTimeRuleList;

	int                   m_chunksGeneratedThisFrame;

protected:

	static CreateFloraHookFunction ms_createFloraHookFunction;

private:

	static void remove ();

private:

	bool shouldClearCollidableFlora (const Vector& position) const;

private:

	ProceduralTerrainAppearance ();
	ProceduralTerrainAppearance (const ProceduralTerrainAppearance&);
	ProceduralTerrainAppearance& operator= (const ProceduralTerrainAppearance&);

private:

	bool                 m_server;

	typedef std::map<uint32, Object*> FloraMap;
	FloraMap* const      m_floraMap;
	FloraMap* const      m_cachedFloraMap;

	ClearCollidableFloraMap* const m_clearCollidableFloraMap;

	bool m_hasPassableAffectors;
}; 

//-------------------------------------------------------------------

inline
bool ProceduralTerrainAppearance::Chunk::getHasLargerNeighbor (int direction) const 
{ 
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, direction, 4);

	return 0!=(hasLargerNeighborFlags & (1<<direction)); 
}

//-------------------------------------------------------------------

inline
bool ProceduralTerrainAppearance::Chunk::findStaticCollidableFlora (const Vector& position, StaticFloraData& data, bool& floraAllowed) const
{
	NOT_NULL (m_floraStaticCollidableMap);
	return _findStaticFlora(*m_floraStaticCollidableMap, position.x, position.z, data, floraAllowed);
}

//-------------------------------------------------------------------

inline
bool ProceduralTerrainAppearance::Chunk::findStaticCollidableFlora (const Vector& position, FloraGroup::Info& data, bool& floraAllowed) const
{
	NOT_NULL (m_floraStaticCollidableMap);
	return _findStaticFlora(*m_floraStaticCollidableMap, position.x, position.z, data, floraAllowed);
}


//===================================================================

#endif
