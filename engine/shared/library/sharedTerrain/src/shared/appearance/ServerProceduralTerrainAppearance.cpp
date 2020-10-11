//===================================================================
//
// ServerProceduralTerrainAppearance.cpp
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/ServerProceduralTerrainAppearance.h"

#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedMath/Plane.h"
#include "sharedMath/Line2d.h"
#include "sharedMath/Vector2d.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"
#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"
#include "sharedTerrain/ServerProceduralTerrainAppearance_Cache.h"

#include <algorithm>
#include <cstdio>
#include <map>
#include <vector>
#include <set>

//===================================================================

namespace ServerProceduralTerrainAppearanceNamespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct TriangleIndirectionData 
	{
		int offsetX0;
		int offsetZ0;
		int offsetX1;
		int offsetZ1;
		int offsetX2;
		int offsetZ2;
	};

	static const int TRIANGLE_INDIRECTION_ARRAY_SIZE = 8;

	static const TriangleIndirectionData constructionTriangleIndirectionArray [TRIANGLE_INDIRECTION_ARRAY_SIZE] =
	{
		// x1, z1, x2, z2, x3, z3
		{   1,  1,  0,  2,  1,  2 },
		{   1,  1,  1,  2,  2,  2 },
		{   1,  1,  2,  2,  2,  1 },
		{   1,  1,  2,  1,  2,  0 },
		{   1,  1,  2,  0,  1,  0 },
		{   1,  1,  1,  0,  0,  0 },
		{   1,  1,  0,  0,  0,  1 },
		{   1,  1,  0,  1,  0,  2 }
	};

	static int chunkIndirectionArray [16] =
	{
		-1,  1,
		 0,  1,
		 1,  1,
		-1,  0,
		 1,  0,
		-1, -1,
		 0, -1,
		 1, -1
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class DenormalizedLine2d
	{
	public:

		DenormalizedLine2d (const Vector2d& point0, const Vector2d& point1) :
			m_normal (-point1.y + point0.y, point1.x - point0.x),
			m_c (-m_normal.dot (point0))
		{
		}

		float computeDistanceTo (const Vector2d& point) const
		{
			return m_normal.dot (point) + m_c;
		}

	private:

		DenormalizedLine2d ();

	private:

		Vector2d   m_normal;
		float      m_c;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace ServerProceduralTerrainAppearanceNamespace;

// ----------------------------------------------------------------------

class ServerProceduralTerrainAppearance::CollisionChunkSorter
{
public:

	explicit CollisionChunkSorter(Vector const & position_o);
	~CollisionChunkSorter();
	CollisionChunkSorter(CollisionChunkSorter const &);
	CollisionChunkSorter & operator=(CollisionChunkSorter const &);

	bool operator()(Chunk const * Chunk1, Chunk const * Chunk2) const;

private:

	CollisionChunkSorter();

private:

	Vector m_position_o;
};

// ----------------------------------------------------------------------

ServerProceduralTerrainAppearance::CollisionChunkSorter::CollisionChunkSorter(Vector const & position_o) :
	m_position_o(position_o)
{
}

// ----------------------------------------------------------------------

ServerProceduralTerrainAppearance::CollisionChunkSorter::~CollisionChunkSorter()
{
}

// ----------------------------------------------------------------------

ServerProceduralTerrainAppearance::CollisionChunkSorter::CollisionChunkSorter(CollisionChunkSorter const & rhs) :
	m_position_o(rhs.m_position_o)
{
}

// ----------------------------------------------------------------------

ServerProceduralTerrainAppearance::CollisionChunkSorter & ServerProceduralTerrainAppearance::CollisionChunkSorter::operator=(CollisionChunkSorter const & rhs)
{
	if (this != &rhs)
		m_position_o = rhs.m_position_o;

	return *this;
}

// ----------------------------------------------------------------------

bool ServerProceduralTerrainAppearance::CollisionChunkSorter::operator()(Chunk const * const chunk1, Chunk const * const chunk2) const
{
	float const distanceSquaredToChunk1 = chunk1 ? m_position_o.magnitudeBetweenSquared(chunk1->getExtent()->getCenter()) : 0.f;
	float const distanceSquaredToChunk2 = chunk2 ? m_position_o.magnitudeBetweenSquared(chunk2->getExtent()->getCenter()) : 0.f;

	return distanceSquaredToChunk1 < distanceSquaredToChunk2;
}

//===================================================================
// PUBLIC STATIC ServerProceduralTerrainAppearance::ServerChunk
//===================================================================

ArrayList<int>* ServerProceduralTerrainAppearance::ServerChunk::ms_indexList;

//-------------------------------------------------------------------

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL (ServerProceduralTerrainAppearance::ServerChunk, false, 256, 4, 0);

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::ServerChunk::install ()
{
	installMemoryBlockManager ();
}

//----------------------------------------------------------------------

bool ServerProceduralTerrainAppearance::ServerChunk::compareTimestampsGreater(ServerChunk const * const lhs, ServerChunk const * const rhs)
{	
	return lhs->m_timeStamp > rhs->m_timeStamp;
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::ServerChunk::createIndexList (const int numberOfTilesPerChunk, const int actualNumberOfPoles)
{
	const int size = sqr (numberOfTilesPerChunk) * TRIANGLE_INDIRECTION_ARRAY_SIZE * 3;
	ms_indexList = new ArrayList<int> (size);
	ms_indexList->preallocate (size, true);

	int indexIndex = 0;

	int x;
	int z;
	for (z = 0; z < numberOfTilesPerChunk; z++)
	{
		for (x = 0; x < numberOfTilesPerChunk; x++)
		{
			const int tileOriginX = x * 2;
			const int tileOriginZ = z * 2;

			int i;
			for (i = 0; i < TRIANGLE_INDIRECTION_ARRAY_SIZE; i++)
			{
				const int i0x = tileOriginX + constructionTriangleIndirectionArray [i].offsetX0;
				const int i0z = tileOriginZ + constructionTriangleIndirectionArray [i].offsetZ0;
				const int i0  = i0x + i0z * (actualNumberOfPoles);

				const int i1x = tileOriginX + constructionTriangleIndirectionArray [i].offsetX1;
				const int i1z = tileOriginZ + constructionTriangleIndirectionArray [i].offsetZ1;
				const int i1  = i1x + i1z * (actualNumberOfPoles);

				const int i2x = tileOriginX + constructionTriangleIndirectionArray [i].offsetX2;
				const int i2z = tileOriginZ + constructionTriangleIndirectionArray [i].offsetZ2;
				const int i2  = i2x + i2z * (actualNumberOfPoles);

				 (*ms_indexList) [indexIndex++] = i0;
				 (*ms_indexList) [indexIndex++] = i1;
				 (*ms_indexList) [indexIndex++] = i2;
			}
		}
	}
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::ServerChunk::remove ()
{
	removeMemoryBlockManager ();

	delete ms_indexList;
	ms_indexList = 0;
}

//===================================================================
// PUBLIC ServerProceduralTerrainAppearance::ServerChunk
//===================================================================

ServerProceduralTerrainAppearance::ServerChunk::ServerChunk (ProceduralTerrainAppearance& proceduralTerrainAppearance) :
	Chunk (proceduralTerrainAppearance),
	m_vertexList (0),
	m_planeList (0),
	m_timeStamp(time(0))
{
}

//-------------------------------------------------------------------

ServerProceduralTerrainAppearance::ServerChunk::~ServerChunk ()
{
	ServerProceduralTerrainAppearance::Cache::destroyVertexList (m_vertexList);
	m_vertexList = 0;

	ServerProceduralTerrainAppearance::Cache::destroyPlaneList (m_planeList);
	m_planeList = 0;

	ServerProceduralTerrainAppearance::Cache::destroyShaderMap (shaderMap);
	shaderMap = 0;

	if (m_floraStaticCollidableMap)
	{
		ServerProceduralTerrainAppearance::Cache::destroyFloraMap(m_floraStaticCollidableMap);
		m_floraStaticCollidableMap = 0;
	}
}

//-------------------------------------------------------------------

bool ServerProceduralTerrainAppearance::ServerChunk::getHeightAt (const Vector& pos, float* const height) const
{
	return getHeightAt (pos, height, 0);
}

//-------------------------------------------------------------------

bool ServerProceduralTerrainAppearance::ServerChunk::getHeightAt (const Vector& pos, float* const height, Vector* const normal) const
{
	const Vector vmin = m_boxExtent.getMin ();
	const Vector vmax = m_boxExtent.getMax ();
	if (pos.x < vmin.x || pos.x > vmax.x || pos.z < vmin.z || pos.z > vmax.z)
	{
		DEBUG_WARNING (true, ("called getHeightAt for position not within chunk"));
		return false;
	}

	int tileX;
	int tileZ;
	_findTileXz(pos, tileX, tileZ);
	if (isExcluded(tileX, tileZ))
		return false;

	//-- find out which tile this intersects
	const Vector start (pos.x, vmax.y + 0.1f, pos.z);
	const Vector end (pos.x, vmin.y - 0.1f, pos.z);
	const Vector dir = end - start;

	//-- collide with the 8 polygons in the tile
	bool found = false;

	CollisionInfo result;
	result.setPoint (end);

	Vector intersection;

	int const numberOfTilesPerChunk = m_proceduralTerrainAppearance.getNumberOfTilesPerChunk();
	int const tileIndex = tileZ * numberOfTilesPerChunk + tileX;
	const int offset = tileIndex * 8;

	int k;
	for (k = offset; k < offset + 8; ++k)
	{
		const Plane& plane = (*m_planeList) [k];
		const Vector& normal = plane.getNormal ();

		if ((dir.dot (normal) < 0.f) && (plane.findIntersection (start, end, intersection)))
		{
			const int i0 = (*ms_indexList) [k * 3 + 0];
			const int i1 = (*ms_indexList) [k * 3 + 1];
			const int i2 = (*ms_indexList) [k * 3 + 2];

			const Vector& v0 = (*m_vertexList) [i0];
			const Vector& v1 = (*m_vertexList) [i1];
			const Vector& v2 = (*m_vertexList) [i2];

			DenormalizedLine2d const line01 (Vector2d (v0.x, v0.z), Vector2d (v1.x, v1.z));
			DenormalizedLine2d const line12 (Vector2d (v1.x, v1.z), Vector2d (v2.x, v2.z));
			DenormalizedLine2d const line20 (Vector2d (v2.x, v2.z), Vector2d (v0.x, v0.z));

			if (line01.computeDistanceTo (Vector2d (start.x, start.z)) <= 0 &&
				line12.computeDistanceTo (Vector2d (start.x, start.z)) <= 0 &&
				line20.computeDistanceTo (Vector2d (start.x, start.z)) <= 0)
			{
				found = true;

				result.setPoint (intersection);
				result.setNormal (normal);
			}
		}
	}

	if (found)
	{
		if (height)
			*height = result.getPoint ().y;

		if (normal)
			*normal = result.getNormal ();

		return true;
	}

	return false;
}

//-------------------------------------------------------------------

#define ALLOW_BACKFACING_COLLISION 0

bool ServerProceduralTerrainAppearance::ServerChunk::collide (const Vector& start, const Vector& end, CollideParameters const & /*collideParameters*/, CollisionInfo& result) const
{
	bool found = false;

	//-- test the line against the extent
	if (m_boxExtent.testSphereOnly (start, end))
	{
		NOT_NULL (m_vertexList);
		NOT_NULL (m_planeList);

#if ALLOW_BACKFACING_COLLISION == 0
		const Vector dir = end - start;
#endif

		Vector intersection;

		int const numberOfTilesPerChunk = m_proceduralTerrainAppearance.getNumberOfTilesPerChunk();
		for (int z = 0; z < numberOfTilesPerChunk; z++)
		{
			for (int x = 0; x < numberOfTilesPerChunk; x++)
			{
				if (isExcluded(x, z))
					continue;

				int const tileIndex = z * numberOfTilesPerChunk + x;
				const int offset = tileIndex * 8;

				for (int k = offset; k < offset + 8; ++k)
				{
					const Plane&  plane  = (*m_planeList) [k];
					const Vector& normal = plane.getNormal ();

#if ALLOW_BACKFACING_COLLISION == 0
					if ((dir.dot (normal) < 0.f) && (plane.findIntersection (start, end, intersection)))
#else
					if (plane.findIntersection (start, end, intersection))
#endif
					{
						const int i0 = (*ms_indexList) [k * 3 + 0];
						const int i1 = (*ms_indexList) [k * 3 + 1];
						const int i2 = (*ms_indexList) [k * 3 + 2];

						const Vector& v0 = (*m_vertexList) [i0];
						const Vector& v1 = (*m_vertexList) [i1];
						const Vector& v2 = (*m_vertexList) [i2];

						if ((start.magnitudeBetweenSquared (intersection) < start.magnitudeBetweenSquared (result.getPoint ())) && intersection.inPolygon (v0, v1, v2))
						{
							found = true;

							result.setPoint (intersection);
							result.setNormal (normal);
						}
					}
				}
			}
		}
	}

	return found;
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::ServerChunk::create (const ProceduralTerrainAppearance::CreateChunkData& createChunkData)
{
	createChunkData.validate ();

	chunkX                = createChunkData.chunkX;
	chunkZ                = createChunkData.chunkZ;
	chunkWidthInMeters    = createChunkData.chunkWidthInMeters;
	originOffset          = createChunkData.originOffset;
	numberOfPoles         = createChunkData.numberOfPoles;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- cache and verify all variables from createChunkData 
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   const int numberOfTilesPerChunk = createChunkData.numberOfTilesPerChunk; 
	const int actualNumberOfPoles   = (numberOfTilesPerChunk * 2) + 1;

	const Array2d<ShaderGroup::Info>* const ccd_shaderMap = &createChunkData.createChunkBuffer->shaderMap;
	const Array2d<FloraGroup::Info>* const  ccd_floraStaticCollidableMap = &createChunkData.createChunkBuffer->floraStaticCollidableMap;
	const Array2d<bool>* const              ccd_excludeMap = &createChunkData.createChunkBuffer->excludeMap;
	const Array2d<bool>* const              ccd_passableMap = &createChunkData.createChunkBuffer->passableMap;
	const Array2d<Vector>* const            vertexPositionMap = &createChunkData.createChunkBuffer->vertexPositionMap;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- create the shared indexed triangle list if needed
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	if (!ms_indexList)
	{
		createIndexList (numberOfTilesPerChunk, actualNumberOfPoles);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- initialization
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	m_boxExtent.setMin(Vector::maxXYZ);
	m_boxExtent.setMax(Vector::negativeMaxXYZ);

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- create and copy appropriate maps
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	m_vertexList             = ServerProceduralTerrainAppearance::Cache::createVertexList (sqr (actualNumberOfPoles));
	m_planeList              = ServerProceduralTerrainAppearance::Cache::createPlaneList (sqr (numberOfTilesPerChunk) * TRIANGLE_INDIRECTION_ARRAY_SIZE);

	shaderMap                = ServerProceduralTerrainAppearance::Cache::createShaderMap (numberOfPoles, numberOfPoles);
	shaderMap->makeCopy (*ccd_shaderMap);

	if (getAppearanceTemplate()->getLegacyMode())
	{
		m_floraStaticCollidableMap = ServerProceduralTerrainAppearance::Cache::createFloraMap (numberOfPoles, numberOfPoles);
		m_floraStaticCollidableMap->makeCopy (*ccd_floraStaticCollidableMap);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- create vertex data
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	{
		m_vertexList->clear ();

		for (int z = originOffset; z < originOffset + actualNumberOfPoles; z++)
		{
			for (int x = originOffset; x < originOffset + actualNumberOfPoles; x++)
			{
				m_vertexList->add(vertexPositionMap->getData(x,z));

				m_boxExtent.updateMinAndMax(vertexPositionMap->getData(x,z));
			}
		}
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- update extents
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	m_boxExtent.calculateCenterAndRadius();

	Sphere boundingSphere = m_boxExtent.getSphere();
	boundingSphere.setRadius(boundingSphere.getRadius() + 0.001f);
	m_boxExtent.setSphere(boundingSphere);

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- create triangles and planes
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	{
		//-- todo index list is the same for every chunk
		m_planeList->preallocate (sqr (numberOfTilesPerChunk) * TRIANGLE_INDIRECTION_ARRAY_SIZE, true);

		int indexIndex = 0;
		int planeIndex = 0;

		int x;
		int z;
		for (z = 0; z < numberOfTilesPerChunk; z++)
		{
			for (x = 0; x < numberOfTilesPerChunk; x++)
			{
				const int tileOriginX = x * 2;
				const int tileOriginZ = z * 2;

				if (!ccd_passableMap->getData (tileOriginX + originOffset, tileOriginZ + originOffset))
				{
					setPassable(x, z, false);
				}

				if (ccd_excludeMap->getData (tileOriginX + originOffset, tileOriginZ + originOffset))
				{
					_setExcluded(x, z);
					continue;
				}

				int i;
				for (i = 0; i < TRIANGLE_INDIRECTION_ARRAY_SIZE; i++)
				{
					const int i0 = (*ms_indexList) [indexIndex++];
					const int i1 = (*ms_indexList) [indexIndex++];
					const int i2 = (*ms_indexList) [indexIndex++];

					 (*m_planeList) [planeIndex++].set ((*m_vertexList) [i0], (*m_vertexList) [i1], (*m_vertexList) [i2]);
				}
			}
		}
	}
}

//-------------------------------------------------------------------

int ServerProceduralTerrainAppearance::ServerChunk::getChunkMemorySize () const
{
	const int geometrySize = sizeof (Vector) * m_vertexList->getNumberOfElements ();
	const int planeSize = sizeof (Plane) * m_planeList->getNumberOfElements ();

#ifdef _DEBUG
	DEBUG_REPORT_PRINT (true, ("  geometrySize = %i\n", geometrySize));
	DEBUG_REPORT_PRINT (true, ("     planeSize = %i\n", planeSize));
#endif

	return Chunk::getChunkMemorySize () + sizeof (*this) + geometrySize + planeSize;
}

//===================================================================
// PUBLIC ServerProceduralTerrainAppearance::ChunkSphereExtentAccessor
//===================================================================

Sphere const ServerProceduralTerrainAppearance::ChunkSphereExtentAccessor::getExtent (ServerProceduralTerrainAppearance::Chunk const * const chunk)
{
	return chunk->getBoxExtent ().getSphere ();
}

//-------------------------------------------------------------------

char const * ServerProceduralTerrainAppearance::ChunkSphereExtentAccessor::getDebugName (ServerProceduralTerrainAppearance::Chunk const * const /*chunk*/)
{
	return 0;
}

//===================================================================
// PUBLIC ServerProceduralTerrainAppearance
//===================================================================

ServerProceduralTerrainAppearance::ServerProceduralTerrainAppearance (const ProceduralTerrainAppearanceTemplate* appearanceTemplate) :
	ProceduralTerrainAppearance (appearanceTemplate),
	m_chunkMap (new ChunkMap),
	m_chunkMapKeyOffset (0),
	m_highUsedChunkList (new ServerChunkList),
	m_lowUsedChunkList (new ServerChunkList),
	m_unusedChunkList (new ServerChunkList),
	m_invalidateChunkList (new ChunkList),
	m_chunksCreatedThisFrame (new ChunkSet),
	m_sphereTree ()
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- to ensure that memory isn't claimed on the client for server terrain (this must be first)
	{
		Cache::install ();
		ServerChunk::install ();
	}
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const_cast<ShaderGroup&> (appearanceTemplate->getTerrainGenerator ()->getShaderGroup ()).loadSurfaceProperties ();

	const float mapWidthInMeters    = proceduralTerrainAppearanceTemplate->getMapWidthInMeters ();
	const float chunkWidthInMeters  = proceduralTerrainAppearanceTemplate->getChunkWidthInMeters ();
	const int numberOfTilesPerChunk = proceduralTerrainAppearanceTemplate->getNumberOfTilesPerChunk ();
	const int actualNumberOfPoles   = (numberOfTilesPerChunk * 2) + 1;
	DEBUG_FATAL(appearanceTemplate->getLegacyMode(), ("ServerProceduralTerrainAppearance should never run in legacy mode.\n"));
	Cache::warm (sqr (actualNumberOfPoles), sqr (numberOfTilesPerChunk) * TRIANGLE_INDIRECTION_ARRAY_SIZE, numberOfPoles);

	m_chunkMapKeyOffset = static_cast<int> (mapWidthInMeters / chunkWidthInMeters);
}

//-------------------------------------------------------------------

ServerProceduralTerrainAppearance::~ServerProceduralTerrainAppearance ()
{
	while (!m_chunkMap->empty ())
	{
		ChunkMap::iterator iter = m_chunkMap->begin ();

		delete iter->second;
		m_chunkMap->erase (iter);
	}

	delete m_chunkMap;
	delete m_highUsedChunkList;
	delete m_lowUsedChunkList;
	delete m_unusedChunkList;
	delete m_invalidateChunkList;
	delete m_chunksCreatedThisFrame;

	DEBUG_WARNING (m_sphereTree.getObjectCount () != 0, ("ServerProceduralTerrainAppearance::m_sphereTree is not empty (%i)", m_sphereTree.getObjectCount ()));

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//-- to ensure that memory isn't claimed on the client for server terrain (this must be last)
	{
		ServerChunk::remove ();
		Cache::remove ();
	}
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

//-------------------------------------------------------------------

float ServerProceduralTerrainAppearance::alter (float time)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerProceduralTerrainAppearance::alter");
	m_chunksGeneratedThisFrame = 0;
	m_chunksCreatedThisFrame->clear();

	int i;
	for (i = 0; i < getNumberOfReferenceObjects (); i++)
	{
		const Object* const object = getReferenceObject (i);
		if (object->isInWorldCell ())
			ProceduralTerrainAppearance::createChunk (getOwner ()->rotateTranslate_w2o (object->getPosition_w ()), 1);
	}

	IGNORE_RETURN (ProceduralTerrainAppearance::alter (time));

	// @todo figure out what this should floatly return.
	return AlterResult::cms_alterNextFrame;
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::render () const
{
	DEBUG_FATAL (true, ("ServerProceduralTerrainAppearance::render should not be called"));
}

//-------------------------------------------------------------------

bool ServerProceduralTerrainAppearance::hasHighLevelOfDetailTerrain (const Vector& position_o) const
{
	return ProceduralTerrainAppearance::findChunk (position_o, 1) != 0;
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::generateBetween(Vector const & start_o, Vector const & end_o, ChunkList & chunkList)
{
	int startX = calculateChunkX(start_o.x);
	int startZ = calculateChunkZ(start_o.z);
	int endX = calculateChunkX(end_o.x);
	int endZ = calculateChunkZ(end_o.z);

	// if both sets of indices are invalid, we don't want to waste time trying to generate chunks
	// if only one is invalid, we still want to check the valid chunks on the terrain from the valid point
	if (!areValidChunkIndices(startX, startZ) && !areValidChunkIndices(endX, endZ))
		return;

	const float chunkWidthInMeters = proceduralTerrainAppearanceTemplate->getChunkWidthInMeters ();

	typedef std::vector<std::pair<int, int> > GenerateList;
	GenerateList generateList;

	if (startX == endX && startZ == endZ)
	{
		// if we are in the same chunk, just add it and be done with it
		generateList.push_back(GenerateList::value_type(startX, startZ));
	}
	else if (fabs(start_o.x - end_o.x) < 0.00001f && fabs(start_o.z - end_o.z) < 0.00001f)
	{
		// this is to not cause a debug_fatal on line2d construction
		generateList.push_back(GenerateList::value_type(startX, startZ));
		generateList.push_back(GenerateList::value_type(endX, endZ));
	}
	// we only want to do the complicated calculations on the shorter side
	else if (abs(endX - startX) < abs(endZ - startZ))
	{
		Line2d const line(start_o.x, start_o.z, end_o.x, end_o.z);

		// in order to make the boundary checks, we reorder the points so that startX <= endX
		if (startX > endX)
		{
			std::swap(startX, endX);
			std::swap(startZ, endZ);
		}

		// this is the X value of the edge (parallel to the X axis) we will cross on our way to endX, this changes as we go through the loop
		float boundingChunkEdgeX = (startX+1)*chunkWidthInMeters;
		int subStartZ = startZ;
		int subEndZ = endZ;

		for (int x = startX; x <= endX; ++x)
		{
			// if we are on the last X, we need to generate until endZ
			if (x == endX)
				subEndZ = endZ;
			else
			{
				// we want to find out how far along on the X axis we need to generate chunks (subStartZ -> subEndZ)
				// in these vectors, the y is our z here, since we are working in a 2d space
				Vector2d intersect;
				if (line.findIntersection(Vector2d(boundingChunkEdgeX, start_o.z), Vector2d(boundingChunkEdgeX, end_o.z), intersect))
					subEndZ = calculateChunkZ(intersect.y);
				else
					DEBUG_WARNING (true, ("ServerProceduralTerrainAppearance::did not intersect with boundingChunkX while generating chunks: startX %i, endX %i, startZ %i, endZ %i, boundingEdgeX %f", startX, endX, startZ, endZ, boundingChunkEdgeX));
			}

			// generate all chunks along the x axis that we need (inclusive of subStartZ and subEndZ)
			int deltaZ = (subStartZ < subEndZ) ? 1 : -1;
			for (int z = subStartZ; z != subEndZ; z += deltaZ)
				generateList.push_back(GenerateList::value_type(x, z));
			generateList.push_back(GenerateList::value_type(x, subEndZ));

			// along the next X axis (moves b/c of the for loop) we start at the endZ of this run
			subStartZ = subEndZ;

			// move the boundingEdge to where it should be for the next loop
			boundingChunkEdgeX += chunkWidthInMeters;
		}
	}
	else
	{
		Line2d const line(start_o.x, start_o.z, end_o.x, end_o.z);

		// in order to make the boundary checks, we reorder the points so that startZ <= endZ
		if (startZ > endZ)
		{
			std::swap(startX, endX);
			std::swap(startZ, endZ);
		}

		// this is the Z value of the edge (parallel to the Z axis) we will cross on our way to endZ, this changes as we go through the loop
		float boundingChunkEdgeZ = (startZ+1)*chunkWidthInMeters;
		int subStartX = startX;
		int subEndX = endX;

		for (int z = startZ; z <= endZ; ++z)
		{
			// if we are on the last Z, we need to generate until endZ
			if (z == endZ)
				subEndX = endX;
			else
			{
				// we want to find out how far along on the Z axis we need to generate chunks (subStartX -> subEndX)
				Vector2d intersect;
				if (line.findIntersection(Vector2d(start_o.x, boundingChunkEdgeZ), Vector2d(end_o.x, boundingChunkEdgeZ), intersect))
					subEndX = calculateChunkX(intersect.x);
				else
					DEBUG_WARNING (true, ("ServerProceduralTerrainAppearance::did not intersect with boundingChunkZ while generating chunks: startX %i, endX %i, startZ %i, endZ %i, boundingEdgeZ %f", startX, endX, startZ, endZ, boundingChunkEdgeZ));
			}

			// generate all chunks along this z axis that we need (inclusive of subStartX and subEndX)
			int deltaX = (subStartX < subEndX) ? 1 : -1;
			for (int x = subStartX; x != subEndX; x += deltaX)
				generateList.push_back(GenerateList::value_type(x, z));
			generateList.push_back(GenerateList::value_type(subEndX, z));

			// along the next Z axis (moves b/c of the for loop) we start at the endX of this run
			subStartX = subEndX;

			// move the boundingEdge to where it should be for the next loop
			boundingChunkEdgeZ += chunkWidthInMeters;
		}
	}

	// add on all chunks to be generated to chunkList. if we encounter invalid chunks (nullptr), don't include them
	const Chunk* chunk;
	for (GenerateList::iterator iter = generateList.begin(); iter != generateList.end(); ++iter)
	{
		createChunk(iter->first, iter->second, 1, 0);
		chunk = findChunk(iter->first, iter->second, 1);

		if (chunk)
			chunkList.push_back(chunk);
	}
}

//===================================================================
// PROTECTED ServerProceduralTerrainAppearance
//===================================================================

void ServerProceduralTerrainAppearance::createChunk (const int x, const int z, const int chunkSize, unsigned /*hasLargerNeighborFlags*/)
{
	//-- make sure indices are valid
	if (!areValidChunkIndices (x, z))
		return;

	//-- see if the chunk already exists
	if (findChunk (x, z, chunkSize) != 0)
		return;

	const TerrainGenerator* terrainGenerator      = proceduralTerrainAppearanceTemplate->getTerrainGenerator ();
	const int               numberOfTilesPerChunk = proceduralTerrainAppearanceTemplate->getNumberOfTilesPerChunk ();
	const float             chunkWidthInMeters    = proceduralTerrainAppearanceTemplate->getChunkWidthInMeters ();
	const float             tileWidthInMeters     = proceduralTerrainAppearanceTemplate->getTileWidthInMeters ();

	//-- chunk does not exist -- it needs to be created. find out what map data i'll need to ask the generator for
	const float  distanceBetweenPoles =  tileWidthInMeters * 0.5f;

	const Vector start (
		static_cast<float>(x)*chunkWidthInMeters - static_cast<float>(originOffset)*distanceBetweenPoles, 
		0.0f,
		static_cast<float>(z)*chunkWidthInMeters - static_cast<float>(originOffset)*distanceBetweenPoles);

	ServerChunk* chunk = new ServerChunk (*this);

	//-- setup data needed to create a chunk
	ProceduralTerrainAppearance::CreateChunkData createChunkData (&createChunkBuffer);

	createChunkData.chunkX                     = x;
	createChunkData.chunkZ                     = z;
	createChunkData.start                      = start;
	createChunkData.numberOfTilesPerChunk      = numberOfTilesPerChunk;
	createChunkData.chunkWidthInMeters         = chunkWidthInMeters;
	createChunkData.tileWidthInMeters          = tileWidthInMeters;
	createChunkData.shaderGroup                = &terrainGenerator->getShaderGroup ();
	createChunkData.floraGroup                 = &terrainGenerator->getFloraGroup ();
	createChunkData.radialGroup                = &terrainGenerator->getRadialGroup ();
	createChunkData.environmentGroup           = &terrainGenerator->getEnvironmentGroup ();
	createChunkData.fractalGroup               = &terrainGenerator->getFractalGroup ();
	createChunkData.bitmapGroup                = &terrainGenerator->getBitmapGroup ();
	createChunkData.originOffset               = originOffset;
	createChunkData.numberOfPoles              = numberOfPoles;

	//-- ask the generator to fill out this area
	TerrainGenerator::GeneratorChunkData generatorChunkData(proceduralTerrainAppearanceTemplate->getLegacyMode());

	generatorChunkData.heightMap            = &createChunkData.createChunkBuffer->heightMap;
	generatorChunkData.colorMap             = &createChunkData.createChunkBuffer->colorMap;
	generatorChunkData.shaderMap            = &createChunkData.createChunkBuffer->shaderMap;
	generatorChunkData.floraStaticCollidableMap    = &createChunkData.createChunkBuffer->floraStaticCollidableMap;
	generatorChunkData.floraStaticNonCollidableMap = &createChunkData.createChunkBuffer->floraStaticNonCollidableMap;
	generatorChunkData.floraDynamicNearMap  = &createChunkData.createChunkBuffer->floraDynamicNearMap;
	generatorChunkData.floraDynamicFarMap   = &createChunkData.createChunkBuffer->floraDynamicFarMap;
	generatorChunkData.environmentMap       = &createChunkData.createChunkBuffer->environmentMap;
	generatorChunkData.vertexPositionMap    = &createChunkData.createChunkBuffer->vertexPositionMap;
	generatorChunkData.vertexNormalMap      = &createChunkData.createChunkBuffer->vertexNormalMap;
	generatorChunkData.excludeMap           = &createChunkData.createChunkBuffer->excludeMap;
	generatorChunkData.passableMap          = &createChunkData.createChunkBuffer->passableMap;
	generatorChunkData.start                = start;
	generatorChunkData.originOffset         = originOffset;
	generatorChunkData.numberOfPoles        = numberOfPoles;
	generatorChunkData.upperPad             = upperPad;
	generatorChunkData.distanceBetweenPoles = distanceBetweenPoles;
	generatorChunkData.shaderGroup          = &terrainGenerator->getShaderGroup ();
	generatorChunkData.floraGroup           = &terrainGenerator->getFloraGroup ();
	generatorChunkData.radialGroup          = &terrainGenerator->getRadialGroup ();
	generatorChunkData.environmentGroup     = &terrainGenerator->getEnvironmentGroup ();
	generatorChunkData.fractalGroup         = &terrainGenerator->getFractalGroup ();
	generatorChunkData.bitmapGroup          = &terrainGenerator->getBitmapGroup ();

	terrainGenerator->generateChunk (generatorChunkData);

	//-- create the chunk using the data the generator created
	chunk->create (createChunkData);
	createFlora (chunk);

	addChunk (chunk, chunkSize);

}  //lint !e429  //-- chunk has not been freed or returned

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::ServerChunk::writeChunkData (const char* filename) const
{
	FILE* outfile = fopen (filename, "wt");
	if (!outfile)
		return;

	NOT_NULL (m_vertexList);

	int i;
	for (i = 0; i < m_vertexList->getNumberOfElements (); i++)
	{
		Vector v = (*m_vertexList) [i];

		fprintf (outfile, "%4i  <%5.1f, %5.1f, %5.1f>\n", i, v.x, v.y, v.z);
	}

	fclose (outfile);
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::writeChunkData (int chunkX, int chunkZ) const
{
	//-- create chunk
	const_cast<ServerProceduralTerrainAppearance*> (this)->createChunk (chunkX, chunkZ, 1, 0);

	//-- get chunk
	const ServerChunk* chunk = dynamic_cast<const ServerChunk*> (findChunk (chunkX, chunkZ, 1));
	NOT_NULL (chunk);

	//-- write chunk
	char filename [100];
	sprintf (filename, "%3i_%3i.txt", chunkX, chunkZ);
	chunk->writeChunkData (filename);
}

//-------------------------------------------------------------------

int ServerProceduralTerrainAppearance::getTerrainType (const Vector& position_o) const
{
	Vector result = position_o;
	float waterHeight;
	if (getHeight (position_o, result.y) && getWaterHeight (position_o, waterHeight) && result.y < waterHeight)
		return 1;

	return 0;
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::invalidateRegion (const Rectangle2d& extent2d)
{
	m_invalidateChunkList->clear ();

	//-- figure out which chunks we need to invalidate
	{
		ChunkMap::iterator iter = m_chunkMap->begin ();
		ChunkMap::iterator end = m_chunkMap->end ();
		for (; iter != end; ++iter)
		{	
			const Chunk* const chunk = iter->second;
			const BoxExtent& boxExtent = chunk->getBoxExtent ();
			const Rectangle2d chunkExtent2d (boxExtent.getMin ().x, boxExtent.getMin ().z,  boxExtent.getMax ().x, boxExtent.getMax ().z);
			if (extent2d.intersects (chunkExtent2d))
				m_invalidateChunkList->push_back (chunk);
		}
	}

	//-- delete those chunks
	{
		while (!m_invalidateChunkList->empty ())
		{
			const Chunk* const chunk = m_invalidateChunkList->back ();
			ChunkMap::iterator iter = m_chunkMap->find (computeChunkMapKey (chunk->getChunkX (), chunk->getChunkZ ()));

			ChunkMap::iterator end = m_chunkMap->end ();
			if (iter != end)
			{
				delete iter->second;
				m_chunkMap->erase (iter);
				m_chunksCreatedThisFrame->erase(iter->first);
			}
			else
				DEBUG_WARNING (true, ("trying to delete non-existant chunk"));

			m_invalidateChunkList->pop_back ();
		}
	}
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::removeUnnecessaryChunk ()
{
	//-- figure out which high chunks are in use
	{
		m_highUsedChunkList->clear ();
		m_highUsedChunkList->reserve (static_cast<size_t> (getNumberOfReferenceObjects ()));

		int i;
		for (i = 0; i < getNumberOfReferenceObjects (); ++i)
		{
			const Object* const object = getReferenceObject (i);
			if (object->isInWorld () && object->isInWorldCell ())
			{
				const Vector& position = object->getPosition_w ();
				const ServerChunk* const chunk = safe_cast<ServerChunk const *>(ProceduralTerrainAppearance::findChunk (position, 1));
				if (chunk)
					m_highUsedChunkList->push_back (chunk);
			}
		}

		std::sort (m_highUsedChunkList->begin (), m_highUsedChunkList->end ());
		ServerChunkList::iterator iter = std::unique (m_highUsedChunkList->begin (), m_highUsedChunkList->end ());
		IGNORE_RETURN (m_highUsedChunkList->erase (iter, m_highUsedChunkList->end ()));
	}

	//-- figure out which low chunks are in use
	{
		m_lowUsedChunkList->clear ();
		m_lowUsedChunkList->reserve (m_highUsedChunkList->size () * 8);

		uint i;
		for (i = 0; i < m_highUsedChunkList->size (); ++i)
		{
			const ServerChunk* const chunk = (*m_highUsedChunkList) [i];
			const int chunkX = chunk->getChunkX ();
			const int chunkZ = chunk->getChunkZ ();

			int j;
			for (j = 0; j < 8; ++j)
			{
				const ServerChunk* const lowChunk = safe_cast<ServerChunk const *>(findChunk (chunkX + chunkIndirectionArray [j * 2 + 0], chunkZ + chunkIndirectionArray [j * 2 + 1], 1));
				if (lowChunk && !std::binary_search (m_highUsedChunkList->begin (), m_highUsedChunkList->end (), lowChunk))
					m_lowUsedChunkList->push_back (lowChunk);
			}
		}

		std::sort (m_lowUsedChunkList->begin (), m_lowUsedChunkList->end ());
		ServerChunkList::iterator iter = std::unique (m_lowUsedChunkList->begin (), m_lowUsedChunkList->end ());
		IGNORE_RETURN (m_lowUsedChunkList->erase (iter, m_lowUsedChunkList->end ()));
	}

	//-- iterate through the tree and delete chunks not in use
	{
		m_unusedChunkList->clear ();
		m_unusedChunkList->reserve (static_cast<size_t> (getNumberOfChunks ()));

		ChunkMap::iterator iter = m_chunkMap->begin ();
		ChunkMap::iterator end = m_chunkMap->end ();
		for (; iter != end; ++iter)
		{
			const ServerChunk* const chunk = safe_cast<ServerChunk const *>(iter->second);

			if (!std::binary_search (m_highUsedChunkList->begin (), m_highUsedChunkList->end (), chunk) &&
				!std::binary_search (m_lowUsedChunkList->begin (), m_lowUsedChunkList->end (), chunk) &&
				!m_chunksCreatedThisFrame->count(computeChunkMapKey(chunk->getChunkX(), chunk->getChunkZ())))
				m_unusedChunkList->push_back (chunk);
		}
	}

	//-- sort the unused chunks by timestamp so the oldest ones get deleted first
	std::sort(m_unusedChunkList->begin(), m_unusedChunkList->end(), ServerChunk::compareTimestampsGreater);

	//-- can I delete at least 10 unused Chunks?
	int chunksToDelete = 128;
	while (chunksToDelete > 0 && !m_unusedChunkList->empty ())
	{
		const Chunk* const chunk = m_unusedChunkList->back ();
		NOT_NULL (chunk);

		ChunkMap::iterator iter = m_chunkMap->find (computeChunkMapKey (chunk->getChunkX (), chunk->getChunkZ ()));
		ChunkMap::iterator end = m_chunkMap->end ();
		if (iter != end)
		{
			DEBUG_FATAL (chunk != iter->second, ("chunk in unused chunklist does not match chunk found in chunkmap"));

			// we don't need to check if this is in the chunksCreatedThisFrame,
			// the unusedChunkList was created w/ the condition that it's elements were not created this frame (see above)
			delete iter->second;
			m_chunkMap->erase (iter);
		}
		else
			DEBUG_WARNING (true, ("trying to delete non-existant chunk"));

		m_unusedChunkList->pop_back ();
		--chunksToDelete;
	}

	if (chunksToDelete > 0)
	{
		//-- sort the low chunks by timestamp so the oldest ones get deleted first
		std::sort(m_lowUsedChunkList->begin(), m_lowUsedChunkList->end(), ServerChunk::compareTimestampsGreater);

		while (chunksToDelete > 0 && !m_lowUsedChunkList->empty ())
		{
			const Chunk* const chunk = m_lowUsedChunkList->back ();
			NOT_NULL (chunk);

			// only delete this chunk if it was not created in this frame
			if (!m_chunksCreatedThisFrame->count(computeChunkMapKey(chunk->getChunkX(), chunk->getChunkZ())))
			{
				ChunkMap::iterator iter = m_chunkMap->find (computeChunkMapKey (chunk->getChunkX (), chunk->getChunkZ ()));
				ChunkMap::iterator end = m_chunkMap->end ();
				if (iter != end)
				{
					DEBUG_FATAL (chunk != iter->second, ("chunk in unused chunklist does not match chunk found in chunkmap"));

					delete iter->second;
					m_chunkMap->erase (iter);
				}
				else
					DEBUG_WARNING (true, ("trying to delete non-existant chunk"));

				--chunksToDelete;
			}

			m_lowUsedChunkList->pop_back ();
		}
	}
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::addChunk (Chunk* const chunk, const int /*chunkSize*/)
{
	const uint32 key = computeChunkMapKey (chunk->getChunkX (), chunk->getChunkZ ());

	bool result = m_chunkMap->insert (std::make_pair (key, chunk)).second;
	UNREF (result);
	DEBUG_FATAL (!result, (""));
	result = m_chunksCreatedThisFrame->insert (key).second;
	DEBUG_WARNING(!result, ("Failed to insert chunk into map for chunks created per frame"));
	++m_chunksGeneratedThisFrame;

	//-- add to sphere tree
	chunk->setSpatialSubdivisionHandle (m_sphereTree.addObject (chunk));
}

//-------------------------------------------------------------------

bool ServerProceduralTerrainAppearance::collideChunkList(ChunkList const & chunkList, Vector const & start_o, Vector const & end_o, CollisionInfo& result) const
{
	bool collided = false;

	result.setPoint (end_o);

	//-- fire ray through chunks
	for (ChunkList::const_iterator iter = chunkList.begin (); iter != chunkList.end (); ++iter)
	{	
		Chunk const * const chunk = *iter;

		CollisionInfo info;
		if (chunk->collide (start_o, result.getPoint (), CollideParameters::cms_default, info))
		{
			collided = true;
			result   = info;
		}
	}

	return collided;
}

//-------------------------------------------------------------------

bool ServerProceduralTerrainAppearance::collide(Vector const & start_o, Vector const & end_o, CollideParameters const & /*collideParameters*/, CollisionInfo & result) const
{
	ChunkList chunkList;
	m_sphereTree.findOnSegment (start_o, end_o, chunkList);
	std::stable_sort(chunkList.begin(), chunkList.end(), CollisionChunkSorter(start_o));	

	return collideChunkList(chunkList, start_o, end_o, result);
}

//-------------------------------------------------------------------

bool ServerProceduralTerrainAppearance::collideForceChunkCreation(Vector const & start_o, Vector const & end_o, CollisionInfo & result)
{
	ChunkList chunkList;
	generateBetween(start_o, end_o, chunkList);
	std::stable_sort(chunkList.begin(), chunkList.end(), CollisionChunkSorter(start_o));	

	return collideChunkList(chunkList, start_o, end_o, result);
}

//-------------------------------------------------------------------

int ServerProceduralTerrainAppearance::getNumberOfChunks () const
{
	return static_cast<uint> (m_chunkMap->size ());
}

//-------------------------------------------------------------------

const ProceduralTerrainAppearance::Chunk* ServerProceduralTerrainAppearance::findChunk (const int x, const int z, const int /*chunkSize*/) const
{
	ChunkMap::iterator iter = m_chunkMap->find (computeChunkMapKey (x, z));
	if (iter != m_chunkMap->end ())
		return iter->second;

	return 0;
}

//-------------------------------------------------------------------

const ProceduralTerrainAppearance::Chunk* ServerProceduralTerrainAppearance::findFirstRenderableChunk (const int x, const int z) const
{
	return findChunk (x, z, 1);
}

//-------------------------------------------------------------------

const ProceduralTerrainAppearance::Chunk* ServerProceduralTerrainAppearance::findAnyChunk () const
{
	if (m_chunkMap->empty ())
		return 0;

	return m_chunkMap->begin ()->second;
}

//-------------------------------------------------------------------

uint32 ServerProceduralTerrainAppearance::computeChunkMapKey (const int x, const int z) const
{
	return ((x + m_chunkMapKeyOffset) & 0x0FFFF) << 16 | ((z + m_chunkMapKeyOffset) & 0x0FFFF);
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::prepareForDelete (Chunk const * const chunk)
{
	m_sphereTree.removeObject (chunk->getSpatialSubdivisionHandle ());
	chunk->setSpatialSubdivisionHandle (0);

	ProceduralTerrainAppearance::prepareForDelete (chunk);
}

//-------------------------------------------------------------------

void ServerProceduralTerrainAppearance::purgeChunks()
{
	while (!m_chunkMap->empty ())
	{
		ChunkMap::iterator iter = m_chunkMap->begin ();

		delete iter->second;
		m_chunkMap->erase (iter);
		m_chunksCreatedThisFrame->erase(iter->first);
	}
}

//===================================================================
