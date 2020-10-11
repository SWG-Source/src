//===================================================================
//
// SamplerProceduralTerrainAppearance.h
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_SamplerProceduralTerrainAppearance_h
#define INCLUDED_SamplerProceduralTerrainAppearance_h

//===================================================================

#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"

class Plane;
class MemoryBlockManager;

//===================================================================
class SamplerProceduralTerrainAppearanceTemplate;

class SamplerProceduralTerrainAppearance : public ProceduralTerrainAppearance
{
public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// Chunk defines a chunk of terrain in the terrain system
	//
	class SamplerChunk : public ProceduralTerrainAppearance::Chunk
	{
		MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

	protected:

		// indexList is a list of triplets indexing the vertex positions used in each triangle of the chunk.
		static ArrayList<int>* ms_indexList;

	protected:

		// vertexList is a list of 3D positions for the chunk's height poles.  size is (numberOfHeightPoles-2)^2
		ArrayList<Vector>* m_vertexList;

		// planeList is a list of planes corresponding to the above triangles
		ArrayList<Plane>*  m_planeList;

		time_t                m_timeStamp;

	private:

		static void createIndexList (int numberOfTilesPerChunk, int actualNumberOfPoles);

	private:

		SamplerChunk ();
		SamplerChunk (const SamplerChunk& rhs);
		SamplerChunk& operator= (const SamplerChunk& rhs);

	public:

		static void install ();
		static void remove ();
		static bool compareTimestampsGreater(SamplerChunk const * lhs, SamplerChunk const * rhs);

	public:

		explicit SamplerChunk (ProceduralTerrainAppearance& proceduralTerrainAppearance);
		virtual ~SamplerChunk ();

		virtual bool     getHeightAt (const Vector& worldPos, float* height) const;
		virtual bool     getHeightAt (const Vector& worldPos, float* height, Vector* normal) const;
		virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;
		virtual int      getChunkMemorySize () const;

		void             create (const ProceduralTerrainAppearance::CreateChunkData& newCreateChunkData);

		void             writeChunkData (const char* filename) const;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Cache;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	SamplerProceduralTerrainAppearance (SamplerProceduralTerrainAppearanceTemplate* appearanceTemplate);
	virtual ~SamplerProceduralTerrainAppearance ();

	virtual float alter (float time);
	virtual void  render () const;
	virtual void  writeChunkData (int chunkX, int chunkZ) const;
	virtual int   getTerrainType (const Vector& position_o) const;
	virtual void  invalidateRegion (const Rectangle2d& extent2d);
	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;
	virtual bool  collideForceChunkCreation (Vector const & start_o, Vector const & end_o, CollisionInfo & result);
	virtual int   getNumberOfChunks () const;
	virtual bool  hasHighLevelOfDetailTerrain (const Vector& position_o) const;

	bool isStaticCollidableFloraChunk(const int x, const int z, const int chunkSize);

	SamplerChunk *createChunk(const int x, const int z, const int chunkSize);
	virtual void  createChunk (int x, int z, int chunkSize, unsigned hasLargerNeighborFlags);
	virtual void  purgeChunks();

	void createFlora (const Chunk* const chunk);

protected:

	typedef std::map<uint32, Chunk const *> ChunkMap;
	typedef std::vector<Chunk const *> ChunkList;
	typedef std::vector<SamplerChunk const *> SamplerChunkList;
	typedef std::set<uint32> ChunkSet;

protected:
	
	virtual void  removeUnnecessaryChunk ();
	virtual void  addChunk (Chunk* chunk, int chunkSize);
	virtual const Chunk* findChunk (int x, int z, int chunkSize) const;
	virtual const Chunk* findFirstRenderableChunk (int x, int z) const;
	virtual const Chunk* findAnyChunk () const;
	virtual uint32 computeChunkMapKey (int x, int z) const;
	virtual void  prepareForDelete (Chunk const * chunk);
	void generateBetween(Vector const & start_o, Vector const & end_o, ChunkList & chunkList);
	bool collideChunkList(ChunkList const & chunkList, Vector const & start_o, Vector const & end_o, CollisionInfo & result) const;

protected:

	SamplerProceduralTerrainAppearanceTemplate &terrainSamplerTemplate;

	ChunkMap* const m_chunkMap;

private:

	class CollisionChunkSorter;

private:

	SamplerProceduralTerrainAppearance ();
	SamplerProceduralTerrainAppearance (const SamplerProceduralTerrainAppearance&);
	SamplerProceduralTerrainAppearance& operator= (const SamplerProceduralTerrainAppearance&);

private:

	int m_chunkMapKeyOffset;

	SamplerChunkList* const m_highUsedChunkList;
	SamplerChunkList* const m_lowUsedChunkList;
	SamplerChunkList* const m_unusedChunkList;
	ChunkList* const m_invalidateChunkList;
}; 

//===================================================================

#endif
