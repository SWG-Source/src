//===================================================================
//
// ServerProceduralTerrainAppearance.h
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_ServerProceduralTerrainAppearance_h
#define INCLUDED_ServerProceduralTerrainAppearance_h

//===================================================================

#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedMath/SphereTree.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"

class Plane;
class MemoryBlockManager;

//===================================================================

class ServerProceduralTerrainAppearance : public ProceduralTerrainAppearance
{
public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// Chunk defines a chunk of terrain in the terrain system
	//
	class ServerChunk : public ProceduralTerrainAppearance::Chunk
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

		ServerChunk ();
		ServerChunk (const ServerChunk& rhs);
		ServerChunk& operator= (const ServerChunk& rhs);

	public:

		static void install ();
		static void remove ();
		static bool compareTimestampsGreater(ServerChunk const * lhs, ServerChunk const * rhs);

	public:

		explicit ServerChunk (ProceduralTerrainAppearance& proceduralTerrainAppearance);
		virtual ~ServerChunk ();

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

	ServerProceduralTerrainAppearance (const ProceduralTerrainAppearanceTemplate* appearanceTemplate);
	virtual ~ServerProceduralTerrainAppearance ();

	virtual float alter (float time);
	virtual void  render () const;
	virtual void  writeChunkData (int chunkX, int chunkZ) const;
	virtual int   getTerrainType (const Vector& position_o) const;
	virtual void  invalidateRegion (const Rectangle2d& extent2d);
	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;
	virtual bool  collideForceChunkCreation (Vector const & start_o, Vector const & end_o, CollisionInfo & result);
	virtual int   getNumberOfChunks () const;
	virtual bool  hasHighLevelOfDetailTerrain (const Vector& position_o) const;
	virtual void  purgeChunks();

protected:

	typedef std::map<uint32, Chunk const *> ChunkMap;
	typedef std::vector<Chunk const *> ChunkList;
	typedef std::vector<ServerChunk const *> ServerChunkList;
	typedef std::set<uint32> ChunkSet;

protected:
	
	virtual void  createChunk (int x, int z, int chunkSize, unsigned hasLargerNeighborFlags);
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

	ChunkMap* const m_chunkMap;

private:

	class ChunkSphereExtentAccessor: public BaseSphereTreeAccessor<Chunk const *, ChunkSphereExtentAccessor>
	{
	public:

		static Sphere const getExtent (Chunk const * chunk);
		static char const * getDebugName (Chunk const * chunk);
	};

	class CollisionChunkSorter;

private:

	ServerProceduralTerrainAppearance ();
	ServerProceduralTerrainAppearance (const ServerProceduralTerrainAppearance&);
	ServerProceduralTerrainAppearance& operator= (const ServerProceduralTerrainAppearance&);

private:

	int m_chunkMapKeyOffset;

	ServerChunkList* const m_highUsedChunkList;
	ServerChunkList* const m_lowUsedChunkList;
	ServerChunkList* const m_unusedChunkList;
	ChunkList* const m_invalidateChunkList;
	ChunkSet*  const m_chunksCreatedThisFrame;

	SphereTree<Chunk const *, ChunkSphereExtentAccessor> m_sphereTree;
}; 

//===================================================================

#endif
