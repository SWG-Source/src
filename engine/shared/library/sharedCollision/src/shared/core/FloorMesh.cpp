// ======================================================================
//
// FloorMesh.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/FloorMesh.h"

#include "sharedCollision/BaseClass.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/FloorLocator.h"
#include "sharedCollision/FloorTri.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/FloorManager.h"

#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Overlap2d.h"
#include "sharedCollision/Overlap3d.h"
#include "sharedCollision/Distance3d.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedCollision/Intersect2d.h"
#include "sharedCollision/Containment2d.h"
#include "sharedCollision/Distance2d.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/SimpleExtent.h"
#include "sharedCollision/BoxTree.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/BoxExtent.h"

#include "sharedDebug/DataLint.h"

#include "sharedMath/Triangle3d.h"
#include "sharedMath/Line3d.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/Plane3d.h"
#include "sharedMath/Ribbon3d.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Circle.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/Sphere.h"

#include "sharedFoundation/Tag.h"
#include "sharedFoundation/DataResourceList.h"

#include "sharedFile/Iff.h"

#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Transform.h"

#include "sharedObject/Appearance.h"

#include "sharedRandom/Random.h"

#include <vector>
#include <stack>
#include <deque>
#include <algorithm>
#include <queue>

typedef std::deque<int> FloorTriIdQueue;
typedef std::stack<FloorTri*, std::vector<FloorTri*> > FloorTriStack;

float wallEpsilon = 0.01f;

float gs_hopTolerance = 0.3f;
float gs_clearTolerance = 0.001f;
float gs_hitPastTolerance = 0.2f; // how far (meters) in the past a hit can occur and still be considered a hit

const Tag TAG_VERT = TAG(V, E, R, T);
const Tag TAG_TRIS = TAG(T, R, I, S);
const Tag TAG_FLOR = TAG(F, L, O, R);
const Tag TAG_PNOD = TAG(P, N, O, D);
const Tag TAG_PEDG = TAG(P, E, D, G);
const Tag TAG_BTRE = TAG(B, T, R, E);
const Tag TAG_BEDG = TAG(B, E, D, G);
const Tag TAG_PGRF = TAG(P, G, R, F);

template <> FloorMeshList::CreateDataResourceMap *FloorMeshList::ms_bindings = nullptr;
template <> FloorMeshList::LoadedDataResourceMap *FloorMeshList::ms_loaded = nullptr;

// ----------------------------------------------------------------------

FloorMesh::FloorMesh(const std::string & filename)
	: CollisionMesh(),
	DataResource(filename.c_str()),
	m_vertices(new VectorVector()),
	m_floorTris(new FloorTriVec()),
	m_crossableEdges(new FloorEdgeIdVec()),
	m_uncrossableEdges(new FloorEdgeIdVec()),
	m_wallBaseEdges(new FloorEdgeIdVec()),
	m_wallTopEdges(new FloorEdgeIdVec()),
	m_pathGraph(nullptr),
	m_appearance(nullptr),
	m_triMarkCounter(1000),    // just some random number
	m_objectFloor(false)
{
#ifdef _DEBUG

	m_crossableLines = nullptr;
	m_uncrossableLines = nullptr;
	m_interiorLines = nullptr;
	m_portalLines = nullptr;
	m_rampLines = nullptr;
	m_fallthroughTriLines = nullptr;
	m_solidTriLines = nullptr;

#endif
}

// ----------

FloorMesh::FloorMesh(VectorVector const & vertices, IntVector const & indices)
	: CollisionMesh(),
	DataResource(""),
	m_vertices(new VectorVector()),
	m_floorTris(new FloorTriVec()),
	m_crossableEdges(new FloorEdgeIdVec()),
	m_uncrossableEdges(new FloorEdgeIdVec()),
	m_wallBaseEdges(new FloorEdgeIdVec()),
	m_wallTopEdges(new FloorEdgeIdVec()),
	m_pathGraph(nullptr),
	m_appearance(nullptr),
	m_triMarkCounter(1000),
	m_objectFloor(false)
{
	build(vertices, indices);

#ifdef _DEBUG

	m_crossableLines = nullptr;
	m_uncrossableLines = nullptr;
	m_interiorLines = nullptr;
	m_portalLines = nullptr;
	m_rampLines = nullptr;
	m_fallthroughTriLines = nullptr;
	m_solidTriLines = nullptr;

#endif
}

// ----------

FloorMesh::~FloorMesh()
{
	delete m_vertices;
	m_vertices = nullptr;

	delete m_floorTris;
	m_floorTris = nullptr;

	delete m_crossableEdges;
	m_crossableEdges = nullptr;

	delete m_uncrossableEdges;
	m_crossableEdges = nullptr;

	delete m_wallBaseEdges;
	m_wallBaseEdges = nullptr;

	delete m_wallTopEdges;
	m_wallTopEdges = nullptr;

	delete m_pathGraph;
	m_pathGraph = nullptr;

	m_appearance = nullptr;

#ifdef _DEBUG

	delete m_crossableLines;
	m_crossableLines = nullptr;

	delete m_uncrossableLines;
	m_uncrossableLines = nullptr;

	delete m_interiorLines;
	m_interiorLines = nullptr;

	delete m_portalLines;
	m_portalLines = nullptr;

	delete m_rampLines;
	m_rampLines = nullptr;

	delete m_fallthroughTriLines;
	m_fallthroughTriLines = nullptr;

	delete m_solidTriLines;
	m_solidTriLines = nullptr;

#endif
}

// ----------------------------------------------------------------------

void FloorMesh::calcBounds(void) const
{
	AxialBox bounds;

	for (int i = 0; i < getVertexCount(); i++)
	{
		bounds.add(getVertex(i));
	}

	// ----------
	// floor collision will catch when a player is over/under a floor
	// mesh

	bounds.setMax(bounds.getMax() + Vector(0.0f, 2.0f, 0.0f));
	bounds.setMin(bounds.getMin() - Vector(0.0f, 1.0f, 0.0f));

	// ----------

	if (m_appearance)
	{
		Extent const * extent = m_appearance->getExtent();

		BoxExtent const * boxExtent = dynamic_cast<BoxExtent const *>(extent);

		AxialBox const & box = boxExtent->getBox();

		if (boxExtent)
		{
			bounds.add(box.getCorner(0));
			bounds.add(box.getCorner(1));
			bounds.add(box.getCorner(2));
			bounds.add(box.getCorner(3));
		}
	}

	// ----------

	m_extent->setShape(MultiShape(bounds));

	setBoundsDirty(false);
}

// ======================================================================
// Basic interface inherited from CollisionMesh - accessors

int FloorMesh::getVertexCount(void) const
{
	return m_vertices->size();
}

Vector const & FloorMesh::getVertex(int whichVertex) const
{
	return m_vertices->at(whichVertex);
}

void FloorMesh::setVertex(int whichVertex, Vector const & newPoint)
{
	m_vertices->at(whichVertex) = newPoint;
}

// ----------

int FloorMesh::getTriCount(void) const
{
	return m_floorTris->size();
}

IndexedTri const & FloorMesh::getIndexedTri(int whichTri) const
{
	return m_floorTris->at(whichTri);
}

void FloorMesh::setIndexedTri(int whichTri, IndexedTri const & newTri)
{
	IndexedTri & I = m_floorTris->at(whichTri);

	I = newTri;
}

// ----------

Triangle3d FloorMesh::getTriangle(int whichTri) const
{
	FloorTri const & F = getFloorTri(whichTri);

	Vector const & A = getVertex(F.getCornerIndex(0));
	Vector const & B = getVertex(F.getCornerIndex(1));
	Vector const & C = getVertex(F.getCornerIndex(2));

	return Triangle3d(A, B, C);
}

// ----------------------------------------------------------------------

Transform const & FloorMesh::getTransform_o2p(void) const
{
	return Transform::identity;
}

// ----------------------------------------------------------------------

void FloorMesh::addTriangle(Triangle3d const & t)
{
	int index = static_cast<int>(m_vertices->size());

	m_vertices->push_back(t.getCornerA());
	m_vertices->push_back(t.getCornerB());
	m_vertices->push_back(t.getCornerC());

	addTriangle(index + 0, index + 1, index + 2);
}

// ----------

void FloorMesh::addTriangle(int iA, int iB, int iC)
{
	FloorTri F;

	F.setCornerIndex(0, iA);
	F.setCornerIndex(1, iB);
	F.setCornerIndex(2, iC);

	F.setNeighborIndex(0, -1);
	F.setNeighborIndex(1, -1);
	F.setNeighborIndex(2, -1);

	Vector A = getVertex(iA);
	Vector B = getVertex(iB);
	Vector C = getVertex(iC);

	Vector temp = (C - A).cross(B - A);
	IGNORE_RETURN(temp.normalize());

	F.setNormal(temp);

	F.setIndex(static_cast<int>(getTriCount()));

	m_floorTris->push_back(F);
}

// ----------------------------------------------------------------------

void FloorMesh::deleteVertex(int whichVertex)
{
	UNREF(whichVertex);

	FATAL(true, ("can't delete floor vertices yet..."));
}

void FloorMesh::deleteVertices(IntVector const & vertIndices)
{
	UNREF(vertIndices);

	FATAL(true, ("can't delete floor vertices yet..."));
}

// ----------------------------------------------------------------------

void FloorMesh::deleteTri(int whichTriangle)
{
	IGNORE_RETURN(m_floorTris->erase(m_floorTris->begin() + whichTriangle));

	//@todo - This really doesn't need to be called after every erase,
	// it ought to be called after all the erasing is done.

	assignIndices();
}

// ----------
// Mark-n-sweep delete

void FloorMesh::deleteTris(IntVector const & triIndices)
{
	if (triIndices.empty()) return;

	// ----------

	clearMarks(0);

	// ----------

	int indexCount = triIndices.size();

	for (int iBad = 0; iBad < indexCount; iBad++)
	{
		getFloorTri(triIndices[iBad]).setMark(1);
	}

	// ----------

	FloorTriVec * pGoodTris = new FloorTriVec;

	for (int iTri = 0; iTri < getTriCount(); iTri++)
	{
		FloorTri const & F = getFloorTri(iTri);

		if (F.getMark() != 1)
		{
			pGoodTris->push_back(F);
		}
	}

	// ----------

	FloorTriVec * pOldTris = m_floorTris;

	m_floorTris = pGoodTris;

	delete pOldTris;

	assignIndices();
}

// ----------------------------------------------------------------------

void FloorMesh::clear(void)
{
	CollisionMesh::clear();

	m_floorTris->clear();
	m_vertices->clear();

	m_crossableEdges->clear();
	m_uncrossableEdges->clear();
	m_wallBaseEdges->clear();
	m_wallTopEdges->clear();
}

// ----------------------------------------------------------------------

void FloorMesh::assignIndices(void)
{
	for (int i = 0; i < getTriCount(); i++)
	{
		getFloorTri(i).setIndex(static_cast<int>(i));
	}
}

// ----------------------------------------------------------------------

void FloorMesh::clearMarks(int clearMarkValue)
{
	for (int i = 0; i < getTriCount(); i++)
	{
		FloorTri & F = getFloorTri(i);

		F.setMark(clearMarkValue);
		F.setEdgeMark(0, clearMarkValue);
		F.setEdgeMark(1, clearMarkValue);
		F.setEdgeMark(2, clearMarkValue);
	}
}

// ----------------------------------------------------------------------

void FloorMesh::clearPortalEdges(void)
{
	for (int i = 0; i < getTriCount(); i++)
	{
		FloorTri & F = getFloorTri(i);

		F.setPortalId(0, -1);
		F.setPortalId(1, -1);
		F.setPortalId(2, -1);
	}
}

// ----------------------------------------------------------------------

void FloorMesh::setAppearance(Appearance const * appearance)
{
	m_appearance = appearance;

	if (m_appearance && m_extent)
	{
		Extent const * extent = appearance->getExtent();

		BoxExtent const * boxExtent = dynamic_cast<BoxExtent const *>(extent);

		if (boxExtent)
		{
			AxialBox bounds = m_extent->getShape().getAxialBox();

			AxialBox box = boxExtent->getBox();

			bounds.add(box.getCorner(0));
			bounds.add(box.getCorner(1));
			bounds.add(box.getCorner(2));
			bounds.add(box.getCorner(3));

			m_extent->setShape(MultiShape(bounds));
		}
	}
}

// ----------------------------------------------------------------------

void FloorMesh::build(VectorVector const & verts, IntVector const & indices)
{
	(*m_vertices) = verts;
	m_floorTris->clear();

	// ----------

	int nTris = indices.size() / 3;

	for (int i = 0; i < nTris; i++)
	{
		int A = indices[i * 3 + 0];
		int B = indices[i * 3 + 1];
		int C = indices[i * 3 + 2];

		addTriangle(A, B, C);
	}
}

// ----------------------------------------------------------------------
// Flag all edges of all floor tris referenced by edgePairs as crossable

//@todo - Uses brute-force search, may be slow for big meshes.

void FloorMesh::addCrossableEdges(IntVector const & edgePairs)
{
	for (int iTri = 0; iTri < getTriCount(); iTri++)
	{
		FloorTri & F = getFloorTri(iTri);

		// ----------

		int nPairs = edgePairs.size() / 2;

		for (int iPair = 0; iPair < nPairs; iPair++)
		{
			int A = edgePairs[iPair * 2 + 0];
			int B = edgePairs[iPair * 2 + 1];

			int whichEdge = FindEdgeUndirected(F, A, B);

			if (whichEdge != -1)
			{
				F.setEdgeType(whichEdge, FET_Crossable);
			}
		}
	}
}

// ----------------------------------------------------------------------
// Flag all edges of all floor tris referenced by edgePairs as crossable

//@todo - Uses brute-force search, may be slow for big meshes.

void FloorMesh::addRampEdges(IntVector const & edgePairs)
{
	for (int iTri = 0; iTri < getTriCount(); iTri++)
	{
		FloorTri & F = getFloorTri(iTri);

		// ----------

		int nPairs = edgePairs.size() / 2;

		for (int iPair = 0; iPair < nPairs; iPair++)
		{
			int A = edgePairs[iPair * 2 + 0];
			int B = edgePairs[iPair * 2 + 1];

			int whichEdge = FindEdgeUndirected(F, A, B);

			if (whichEdge != -1)
			{
				F.setEdgeType(whichEdge, FET_WallBase);
			}
		}
	}
}

// ----------------------------------------------------------------------
// Flag the tris in the floor we want to be able to fall through as
// fall-through-able

void FloorMesh::addFallthroughTris(IntVector const & triIDs)
{
	int idCount = triIDs.size();

	for (int iTri = 0; iTri < idCount; iTri++)
	{
		int index = triIDs[iTri];

		FloorTri & F = getFloorTri(index);

		F.setFallthrough(true);
	}
}

// ----------------------------------------------------------------------
// All edges of all floor tris start out unlinked and non-crossable.
// Once we've linked all the tris up, go through and flag any edges that
// now have neighbors as crossable.

void FloorMesh::flagCrossableEdges(void)
{
	for (int iTri = 0; iTri < getTriCount(); iTri++)
	{
		FloorTri & F = getFloorTri(iTri);

		// ----------

		for (int iEdge = 0; iEdge < 3; iEdge++)
		{
			if (F.hasNeighbor(iEdge))
			{
				F.setEdgeType(iEdge, FET_Crossable);
			}
		}
	}
}

// ----------------------------------------------------------------------

FloorTri & FloorMesh::getFloorTri(int whichTri)
{
	FATAL(whichTri < 0, ("FloorMesh::getFloorTri - tried to get an invalid tri"));
	FATAL(whichTri >= getTriCount(), ("FloorMesh::getFloorTri - tried to get an invalid tri"));

	return m_floorTris->at(whichTri);
}

FloorTri const & FloorMesh::getFloorTri(int whichTri) const
{
	FATAL(whichTri < 0, ("FloorMesh::getFloorTri - tried to get an invalid tri"));
	FATAL(whichTri >= getTriCount(), ("FloorMesh::getFloorTri - tried to get an invalid tri"));

	return m_floorTris->at(whichTri);
}

// ----------

FloorEdgeIdVec const & FloorMesh::getEdgeList(FloorEdgeType edgeType) const
{
	switch (edgeType)
	{
	case FET_Crossable:   return *m_crossableEdges;
	case FET_Uncrossable: return *m_uncrossableEdges;
	case FET_WallBase:    return *m_wallBaseEdges;
	case FET_WallTop:     return *m_wallTopEdges;
	default: {
		DEBUG_FATAL(true, ("FloorMesh::getEdgeList - requesting invalid list $d\n", edgeType));
		static FloorEdgeIdVec dummy;
		return dummy;
	}
	}
}

// ----------------------------------------------------------------------

BaseClass * FloorMesh::getPathGraph(void)
{
	return m_pathGraph;
}

BaseClass const * FloorMesh::getPathGraph(void) const
{
	return m_pathGraph;
}

void FloorMesh::attachPathGraph(BaseClass * newGraph)
{
	if (m_pathGraph != newGraph)
	{
		delete m_pathGraph;
		m_pathGraph = newGraph;
	}
}

// ----------------------------------------------------------------------
// Clean out any unused vertices and update the vertex indices in the
// floor tris to match

void FloorMesh::sweep(void)
{
	int nVerts = getVertexCount();
	int nTris = getTriCount();

	// ----------
	// Build our vertex reference count table

	IntVector vertRefcount(nVerts, 0);

	for (int iTri = 0; iTri < nTris; iTri++)
	{
		IndexedTri const & tri = getIndexedTri(iTri);

		int A = tri.getCornerIndex(0);
		int B = tri.getCornerIndex(1);
		int C = tri.getCornerIndex(2);

		vertRefcount[A]++;
		vertRefcount[B]++;
		vertRefcount[C]++;
	}

	// ----------
	// Use the refcount table to build a remap table that will pack
	// all the used vertices at the front of the array

	IntVector vertRemap(nVerts);

	int nUsedVerts = 0;
	int cursor = 0;

	for (int iRemap = 0; iRemap < nVerts; iRemap++)
	{
		if (vertRefcount[iRemap] != 0)
		{
			nUsedVerts++;

			vertRemap[iRemap] = cursor;

			cursor++;
		}
		else
		{
			vertRemap[iRemap] = -1;
		}
	}

	// ----------
	// Use the remap table to pack all vertex data at the beginning of the array

	for (int iVert = 0; iVert < nVerts; iVert++)
	{
		int newIndex = vertRemap[iVert];

		// Skip unused vertices & vertices that aren't moving

		if (newIndex == -1) continue;
		if (newIndex == static_cast<int>(iVert)) continue;

		// Move the used vertices to their new place in the array

		setVertex(newIndex, getVertex(iVert));
	}

	// ----------
	// Use the remap table to update the floor tri's vertex indices

	for (int iTri2 = 0; iTri2 < nTris; iTri2++)
	{
		IndexedTri temp = getIndexedTri(iTri2);

		int iA = vertRemap[temp.getCornerIndex(0)];
		int iB = vertRemap[temp.getCornerIndex(1)];
		int iC = vertRemap[temp.getCornerIndex(2)];

		temp.setCornerIndex(0, iA);
		temp.setCornerIndex(1, iB);
		temp.setCornerIndex(2, iC);

		setIndexedTri(iTri2, temp);
	}

	// ----------
	// Data is repacked, shrink the array to hold only the used verts

	m_vertices->resize(nUsedVerts);
}

// ----------------------------------------------------------------------
// Brute-force, ON^2 adjacency calc - try and link every tri with every
// other tri

void FloorMesh::link(void)
{
	int nTris = getTriCount();

	for (int A = 0; A < nTris - 1; A++)
	{
		for (int B = A + 1; B < nTris; B++)
		{
			LinkTris(getFloorTri(A), getFloorTri(B));
		}
	}
}

// ----------------------------------------------------------------------

void FloorMesh::compile(void)
{
	IGNORE_RETURN(scrub());  // get rid of any degenerate triangles
	merge();                   // merge the vertices for the triangles
	sweep();                   // sweep out any unused vertices
	link();                    // build adjacency information for the triangles
	flagCrossableEdges();      // use the adjacency information to mark internal floor edges as crossable
	calcBounds();              // build the bounding box for the floor
	setPartTags();
	buildBoundaryEdgeList();
}

// ----------------------------------------------------------------------

void FloorMesh::write(Iff & iff)
{
	iff.insertForm(TAG_FLOR);
	iff.insertForm(TAG_0006);

	// ----------

	iff.insertChunk(TAG_VERT);
	{
		int vertCount = getVertexCount();

		iff.insertChunkData(vertCount);

		for (int i = 0; i < vertCount; i++)
		{
			iff.insertChunkFloatVector(getVertex(i));
		}
	}
	iff.exitChunk(TAG_VERT);

	// ----------

	iff.insertChunk(TAG_TRIS);
	{
		int triCount = getTriCount();

		iff.insertChunkData(triCount);

		for (int i = 0; i < triCount; i++)
		{
			getFloorTri(i).write(iff);
		}
	}
	iff.exitChunk(TAG_TRIS);

	// ----------

	if (m_boxTree != nullptr)
	{
		m_boxTree->write(iff);
	}

	// ----------

	if (!m_crossableEdges->empty() || !m_uncrossableEdges->empty() || !m_wallBaseEdges->empty())
	{
		iff.insertChunk(TAG_BEDG);
		{
			int totalEdgeCount = m_crossableEdges->size() + m_uncrossableEdges->size() + m_wallBaseEdges->size();

			iff.insertChunkData(totalEdgeCount);

			{
				int edgeCount = m_crossableEdges->size();

				for (int i = 0; i < edgeCount; i++)
				{
					FloorEdgeId & id = m_crossableEdges->at(i);

					bool crossable = getFloorTri(id.triId).isCrossable(id.edgeId);

					iff.insertChunkData(id.triId);
					iff.insertChunkData(id.edgeId);
					iff.insertChunkData(crossable);
				}
			}

			{
				int edgeCount = m_uncrossableEdges->size();

				for (int i = 0; i < edgeCount; i++)
				{
					FloorEdgeId & id = m_uncrossableEdges->at(i);

					bool crossable = getFloorTri(id.triId).isCrossable(id.edgeId);

					iff.insertChunkData(id.triId);
					iff.insertChunkData(id.edgeId);
					iff.insertChunkData(crossable);
				}
			}

			{
				int edgeCount = m_wallBaseEdges->size();

				for (int i = 0; i < edgeCount; i++)
				{
					FloorEdgeId & id = m_wallBaseEdges->at(i);

					bool crossable = getFloorTri(id.triId).isCrossable(id.edgeId);

					iff.insertChunkData(id.triId);
					iff.insertChunkData(id.edgeId);
					iff.insertChunkData(crossable);
				}
			}
		}
		iff.exitChunk(TAG_BEDG);
	}

	// ----------

	ObjectWriter graphWriter = FloorManager::getPathGraphWriter();

	if (m_pathGraph && graphWriter)
	{
		graphWriter(m_pathGraph, iff);
	}

	// ----------

	iff.exitForm(TAG_0006);
	iff.exitForm(TAG_FLOR);
}

// ----------------------------------------------------------------------

void FloorMesh::read(Iff & iff)
{
	clear();

	// ----------

	int version = -1;

	iff.enterForm(TAG_FLOR);

	switch (iff.getCurrentName())
	{
	case TAG_0000:
		version = 0;
		read_0000(iff);
		break;

	case TAG_0001:
		version = 1;
		read_0001(iff);
		break;

	case TAG_0002:
		version = 2;
		read_0002(iff);
		break;

	case TAG_0003:
		version = 3;
		read_0003(iff);
		break;

	case TAG_0004:
		version = 4;
		read_0004(iff);
		break;

	case TAG_0005:
		version = 5;
		read_0005(iff);
		break;

	case TAG_0006:
		version = 6;
		read_0006(iff);
		break;

	default:
		version = -1;
		FATAL(true, ("FloorMesh::Invalid version"));
		break;
	}

	iff.exitForm(TAG_FLOR);

	// ----------
	// Post-load stuff

	// floor versions 6 and earlier calculate their floor tri normals upside down. fix that here.

	if (version <= 6)
	{
		int triCount = getTriCount();

		for (int i = 0; i < triCount; i++)
		{
			FloorTri & T = getFloorTri(i);

			T.setNormal(-T.getNormal());
		}
	}

	// Check for inverted floor tris

	if (DataLint::isInstalled())
	{
		int triCount = getTriCount();

		for (int i = 0; i < triCount; i++)
		{
			FloorTri & T = getFloorTri(i);

			if (T.getNormal().y <= 0.0f)
			{
				DEBUG_WARNING(true, ("FloorMesh::read - Floor %s contains a triangle that's not facing up (art error) [%1.2f, %1.2f, %1.2f]", iff.getFileName(), T.getNormal().x, T.getNormal().y, T.getNormal().z));
				break;
			}
		}
	}

	calcBounds();

	calcHeightFuncs();

	if (version < 5) buildBoundaryEdgeList();

#ifdef _DEBUG

	if (ConfigSharedCollision::getBuildDebugData())
	{
		buildDebugData();
	}

#endif
}

// ----------------------------------------------------------------------

void FloorMesh::read_0000(Iff & iff)
{
	iff.enterForm(TAG_0000);

	// ----------

	iff.enterChunk(TAG_VERT);
	{
		while (iff.getChunkLengthLeft())
		{
			Vector V = iff.read_floatVector();

			m_vertices->push_back(V);
		}
	}
	iff.exitChunk(TAG_VERT);

	// ----------

	iff.enterChunk(TAG_TRIS);
	{
		while (iff.getChunkLengthLeft())
		{
			FloorTri F;

			F.setIndex(iff.read_int32());

			F.setCornerIndex(0, iff.read_int32());
			F.setCornerIndex(1, iff.read_int32());
			F.setCornerIndex(2, iff.read_int32());

			F.setNeighborIndex(0, iff.read_int32());
			F.setNeighborIndex(1, iff.read_int32());
			F.setNeighborIndex(2, iff.read_int32());

			F.setNormal(iff.read_floatVector());

			F.setEdgeType(0, FET_Uncrossable);
			F.setEdgeType(1, FET_Uncrossable);
			F.setEdgeType(2, FET_Uncrossable);

			F.setFallthrough(false);

			m_floorTris->push_back(F);
		}
	}
	iff.exitChunk(TAG_TRIS);

	// ----------

	iff.exitForm(TAG_0000);

	// Gotta fix up marking edges as crossable if this is an old floormesh

	flagCrossableEdges();
}

// ----------------------------------------------------------------------

void FloorMesh::read_0001(Iff & iff)
{
	iff.enterForm(TAG_0001);

	// ----------

	iff.enterChunk(TAG_VERT);
	{
		while (iff.getChunkLengthLeft())
		{
			Vector V = iff.read_floatVector();

			m_vertices->push_back(V);
		}
	}
	iff.exitChunk(TAG_VERT);

	// ----------

	iff.enterChunk(TAG_TRIS);
	{
		while (iff.getChunkLengthLeft())
		{
			FloorTri F;

			F.setIndex(iff.read_int32());

			F.setCornerIndex(0, iff.read_int32());
			F.setCornerIndex(1, iff.read_int32());
			F.setCornerIndex(2, iff.read_int32());

			F.setNeighborIndex(0, iff.read_int32());
			F.setNeighborIndex(1, iff.read_int32());
			F.setNeighborIndex(2, iff.read_int32());

			F.setNormal(iff.read_floatVector());

			F.setCrossable(0, iff.read_bool8());
			F.setCrossable(1, iff.read_bool8());
			F.setCrossable(2, iff.read_bool8());

			F.setFallthrough(iff.read_bool8());

			m_floorTris->push_back(F);
		}
	}
	iff.exitChunk(TAG_TRIS);

	// ----------

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void FloorMesh::read_0002(Iff & iff)
{
	iff.enterForm(TAG_0002);

	// ----------

	iff.enterChunk(TAG_VERT);
	{
		while (iff.getChunkLengthLeft())
		{
			Vector V = iff.read_floatVector();

			m_vertices->push_back(V);
		}
	}
	iff.exitChunk(TAG_VERT);

	// ----------

	iff.enterChunk(TAG_TRIS);
	{
		while (iff.getChunkLengthLeft())
		{
			FloorTri F;

			F.read_0000(iff);

			m_floorTris->push_back(F);
		}
	}
	iff.exitChunk(TAG_TRIS);

	// ----------

	iff.exitForm(TAG_0002);
}

// ----------------------------------------------------------------------

void FloorMesh::read_0003(Iff & iff)
{
	iff.enterForm(TAG_0003);

	// ----------

	iff.enterChunk(TAG_VERT);
	{
		while (iff.getChunkLengthLeft())
		{
			Vector V = iff.read_floatVector();

			m_vertices->push_back(V);
		}
	}
	iff.exitChunk(TAG_VERT);

	// ----------

	iff.enterChunk(TAG_TRIS);
	{
		FloorTri F;

		while (iff.getChunkLengthLeft())
		{
			F.read_0001(iff);

			m_floorTris->push_back(F);
		}
	}
	iff.exitChunk(TAG_TRIS);

	// ----------

	iff.exitForm(TAG_0003);
}

// ----------------------------------------------------------------------

void FloorMesh::read_0004(Iff & iff)
{
	iff.enterForm(TAG_0004);

	// ----------

	iff.enterChunk(TAG_VERT);
	{
		while (iff.getChunkLengthLeft())
		{
			Vector V = iff.read_floatVector();

			m_vertices->push_back(V);
		}
	}
	iff.exitChunk(TAG_VERT);

	// ----------

	iff.enterChunk(TAG_TRIS);
	{
		FloorTri F;

		while (iff.getChunkLengthLeft())
		{
			F.read_0001(iff);

			m_floorTris->push_back(F);
		}
	}
	iff.exitChunk(TAG_TRIS);

	// ----------
	// Version 4 floor meshes had two chunks of pathfinding data, one for nodes
	// and one for edges, but they weren't in a containing form.

	ObjectFactory pathGraphFactory = FloorManager::getPathGraphFactory();

	if (pathGraphFactory)
	{
		// The factory knows how to build a path graph from the old
		// data format.

		BaseClass * pathGraph = pathGraphFactory(iff);

		attachPathGraph(pathGraph);
	}
	else
	{
		// We have a path graph to load but no factory to load it
		// with, so skip the form

		iff.enterChunk(TAG_PNOD);
		iff.exitChunk(TAG_PNOD, true);

		iff.enterChunk(TAG_PEDG);
		iff.exitChunk(TAG_PEDG, true);
	}

	// ----------

	iff.exitForm(TAG_0004);
}

// ----------------------------------------------------------------------

void FloorMesh::read_0005(Iff & iff)
{
	iff.enterForm(TAG_0005);

	// ----------

	iff.enterChunk(TAG_VERT);
	{
		int vertexCount = iff.getChunkLengthTotal() / sizeof(Vector);

		m_vertices->clear();
		m_vertices->reserve(vertexCount);

		while (iff.getChunkLengthLeft())
		{
			Vector V = iff.read_floatVector();

			m_vertices->push_back(V);
		}
	}
	iff.exitChunk(TAG_VERT);

	// ----------

	iff.enterChunk(TAG_TRIS);
	{
		int triCount = iff.getChunkLengthTotal() / FloorTri::getOnDiskSize_0001();

		m_floorTris->clear();
		m_floorTris->reserve(triCount);

		FloorTri F;

		while (iff.getChunkLengthLeft())
		{
			F.read_0001(iff);

			m_floorTris->push_back(F);
		}
	}
	iff.exitChunk(TAG_TRIS);

	// ----------

	if (iff.getCurrentName() == TAG_BTRE)
	{
		m_boxTree = new BoxTree();

		m_boxTree->read(iff);
	}

	// ----------

	if (iff.getCurrentName() == TAG_BEDG)
	{
		iff.enterChunk(TAG_BEDG);
		{
			int edgeCount = iff.read_int32();

			for (int i = 0; i < edgeCount; i++)
			{
				int triId = iff.read_int32();
				int edgeId = iff.read_int32();

				FloorEdgeId id(triId, edgeId);

				bool crossable = iff.read_bool8();

				UNREF(crossable); // redundant now that we've got an edge type

				switch (getFloorTri(id.triId).getEdgeType(id.edgeId))
				{
				case FET_Crossable: { m_crossableEdges->push_back(id); break; }
				case FET_Uncrossable: { m_uncrossableEdges->push_back(id); break; }
				case FET_WallBase: { m_wallBaseEdges->push_back(id); break; }
				default: { break; }
				}
			}
		}
		iff.exitChunk(TAG_BEDG);
	}

	// ----------

	if (!iff.atEndOfForm() && iff.getCurrentName() == TAG_PGRF)
	{
		ObjectFactory pathGraphFactory = FloorManager::getPathGraphFactory();

		if (pathGraphFactory && CollisionWorld::isServerSide())
		{
			BaseClass * pathGraph = pathGraphFactory(iff);

			attachPathGraph(pathGraph);
		}
		else
		{
			// We have a path graph to load but no factory to load it
			// with, so skip the form

			iff.enterForm();
			iff.exitForm(true);
		}
	}

	// ----------

	iff.exitForm(TAG_0005);
}

// ----------------------------------------------------------------------

void FloorMesh::read_0006(Iff & iff)
{
	iff.enterForm(TAG_0006);

	// ----------

	iff.enterChunk(TAG_VERT);
	{
		int vertexCount = iff.read_int32();

		m_vertices->clear();
		m_vertices->reserve(vertexCount);

		for (int i = 0; i < vertexCount; i++)
		{
			Vector V = iff.read_floatVector();

			m_vertices->push_back(V);
		}
	}
	iff.exitChunk(TAG_VERT);

	// ----------

	iff.enterChunk(TAG_TRIS);
	{
		int triCount = iff.read_int32();

		m_floorTris->clear();
		m_floorTris->reserve(triCount);

		FloorTri F;

		for (int i = 0; i < triCount; i++)
		{
			F.read_0002(iff);

			m_floorTris->push_back(F);
		}
	}
	iff.exitChunk(TAG_TRIS);

	// ----------

	if (iff.getCurrentName() == TAG_BTRE)
	{
		m_boxTree = new BoxTree();

		m_boxTree->read(iff);
	}

	// ----------

	if (iff.getCurrentName() == TAG_BEDG)
	{
		iff.enterChunk(TAG_BEDG);
		{
			int edgeCount = iff.read_int32();

			for (int i = 0; i < edgeCount; i++)
			{
				int triId = iff.read_int32();
				int edgeId = iff.read_int32();

				FloorEdgeId id(triId, edgeId);

				bool crossable = iff.read_bool8();

				UNREF(crossable); // redundant now that we've got an edge type

				switch (getFloorTri(id.triId).getEdgeType(id.edgeId))
				{
				case FET_Crossable: { m_crossableEdges->push_back(id); break; }
				case FET_Uncrossable: { m_uncrossableEdges->push_back(id); break; }
				case FET_WallBase: { m_wallBaseEdges->push_back(id); break; }
				default: { break; }
				}
			}
		}
		iff.exitChunk(TAG_BEDG);
	}

	// ----------

	if (!iff.atEndOfForm() && iff.getCurrentName() == TAG_PGRF)
	{
		ObjectFactory pathGraphFactory = FloorManager::getPathGraphFactory();

		if (pathGraphFactory && CollisionWorld::isServerSide())
		{
			BaseClass * pathGraph = pathGraphFactory(iff);

			attachPathGraph(pathGraph);
		}
		else
		{
			// We have a path graph to load but no factory to load it
			// with, so skip the form

			iff.enterForm();
			iff.exitForm(true);
		}
	}

	// ----------

	iff.exitForm(TAG_0006);
}

// ======================================================================
// DataResource implementation

void FloorMesh::load(Iff & iff)
{
	read(iff);
}

// ----------

Tag FloorMesh::getId(void) const
{
	return TAG_FLOR;
}

// ----------------------------------------------------------------------

FloorMesh * FloorMesh::create(const std::string & filename)
{
	return new FloorMesh(filename);
}

// ----------

void FloorMesh::release(void) const
{
	FloorMeshList::release(*this);
}

// ----------------------------------------------------------------------

void FloorMesh::install(void)
{
	FloorMeshList::registerTemplate(TAG_FLOR, FloorMesh::create);
}

void FloorMesh::remove(void)
{
	FloorMeshList::remove();
}

// ----------------------------------------------------------------------

bool FloorMesh::findFloorTri(FloorLocator const & testLoc,
	float heightTolerance,
	bool bAllowJump,
	FloorLocator & outLoc) const
{
	Vector testPoint = testLoc.getPosition_l();

	// ----------
	// If the line through the test point doesn't intersect the floor's bounding box,
	// the point can't land on the floor

	Line3d line(testPoint, -Vector::unitY);

	bool bHitsBounds = Overlap3d::TestLineABox(line, getBoundingABox());

	if (!bHitsBounds)
	{
		outLoc = FloorLocator::invalid;
		return false;
	}

	// ----------
	// Search the triangles for one that is below the test point

	FloorLocator closest;

	bool bFound = intersectClosest(line, closest);

	// ----------

	if (!bFound || (closest.getId() == -1))
	{
		outLoc = FloorLocator::invalid;
		return false;
	}

	// ----------

	float dist = closest.getOffset();

	if (dist > heightTolerance)
	{
		// Floor tri is too far below the test point

		outLoc = FloorLocator::invalid;
		return false;
	}

	if (dist > 0.0f)
	{
		// Floor tri is below the test point, we don't need a jump
		// or hop to get on it

		outLoc = closest;
		return true;
	}

	// ----------

	if (bAllowJump)
	{
		// Floor tri is above the test point, but we dont' care
		// since we're allowing jumps

		outLoc = closest;
		return true;
	}

	// ----------

	if (std::abs(dist) < gs_hopTolerance)
	{
		// Floor tri is above the test point but within the hop
		// tolerance and we're allowing hops - we can hop up onto it.

		DEBUG_REPORT_LOG_PRINT(ConfigSharedCollision::getReportMessages(), ("FloorMesh::findFloorTri - Found triangle we can hop onto\n"));
		outLoc = closest;
		return true;
	}

	// ----------

	if (getFloorTri(closest.getId()).isFallthrough())
	{
		// Floor tri is above the test point, out of hop range, and we're not allowing jumps
		// BUT, since it's marked as fallthrough we can be on it anyway.

		DEBUG_REPORT_LOG_PRINT(ConfigSharedCollision::getReportMessages(), ("FloorMesh::findFloorTri - Found fallthrough triangle we can jump onto\n"));
		outLoc = closest;
		return true;
	}

	// ----------
	// No valid tri found

	outLoc = FloorLocator::invalid;
	return false;
}

// ----------

bool FloorMesh::findFloorTri(FloorLocator const & testLoc,
	bool bAllowJump,
	FloorLocator & outLoc) const
{
	return findFloorTri(testLoc, REAL_MAX, bAllowJump, outLoc);
}

// ----------------------------------------------------------------------

void FloorMesh::makeHitResult(Vector const & begin, Vector const & delta, int hitTriId, int hitEdgeId, float hitTime, FloorLocator & result) const
{
	Vector endPoint = begin + delta * hitTime;

	Vector contactNormal = getTriangle(hitTriId).getEdgeDir(hitEdgeId).cross(-Vector::unitY);

	contactNormal.y = 0.0f;

	IGNORE_RETURN(contactNormal.normalize());

	// ----------

	result = FloorLocator(this, endPoint);

	result.setTriId(hitTriId);
	result.setEdgeId(hitEdgeId);

	result.setHitTriId(hitTriId);
	result.setHitEdgeId(hitEdgeId);

	result.setTime(hitTime);
	result.setContactNormal(contactNormal);

	result.snapToFloor();
}

// ----------

void FloorMesh::makeHitResult2(Vector const & begin, Vector const & delta, int centerTriId, int hitTriId, int hitEdgeId, float hitTime, FloorLocator & result) const
{
	Segment3d edge = getTriangle(hitTriId).getEdgeSegment(hitEdgeId);

	Vector endPoint = begin + delta * hitTime;

	Vector contactPoint = Distance2d::ClosestPointSeg(endPoint, edge);
	Vector contactNormal = endPoint - contactPoint;

	contactNormal.y = 0.0f;

	IGNORE_RETURN(contactNormal.normalize());

	// ----------

	result = FloorLocator(this, endPoint);

	result.setTriId(centerTriId);
	result.setEdgeId(-1);

	result.setHitTriId(hitTriId);
	result.setHitEdgeId(hitEdgeId);

	result.setTime(hitTime);
	result.setContactNormal(contactNormal);

	result.snapToFloor();
}

// ----------

void FloorMesh::makeExitResult(Vector const & begin, Vector const & delta, int hitTriId, int hitEdgeId, float hitTime, FloorLocator & result) const
{
	Segment3d edge = getTriangle(hitTriId).getEdgeSegment(hitEdgeId);

	Vector endPoint = begin + delta * hitTime;

	Vector contactPoint = Distance2d::ClosestPointSeg(endPoint, edge);
	Vector contactNormal = endPoint - contactPoint;

	contactNormal.y = 0.0f;

	IGNORE_RETURN(contactNormal.normalize());

	// ----------

	result = FloorLocator(this, endPoint);

	result.setTriId(hitTriId);
	result.setEdgeId(hitEdgeId);

	result.setHitTriId(hitTriId);
	result.setHitEdgeId(hitEdgeId);

	result.setTime(hitTime);
	result.setContactNormal(contactNormal);

	result.snapToFloor();
}

// ----------

void FloorMesh::makeSuccessResult(FloorLocator const & loc, FloorLocator & result) const
{
	result = loc;

	result.setEdgeId(-1);
	result.setTime(1.0f);
}

// ----------

void FloorMesh::makeFailureResult(FloorLocator & result) const
{
	result = FloorLocator::invalid;
}

// ----------------------------------------------------------------------
// Triangles facing down cannot be entered

// Crossable edges of fallthrough tris can be entered from anywhere
// uncrossable edges cannot be entered

// Crossable edges of non-fallthrough tris can be entered from above
// Ramp edges can be entered from above

bool FloorMesh::canEnterEdge(FloorLocator const & enterLoc) const
{
	if (enterLoc.getEdgeId() == -1) return false;
	if (enterLoc.getTriId() == -1) return false;
	if (enterLoc.getFloorMesh() == nullptr) return false;

	// Can't enter the tri if it's facing down

	if (getTriangle(enterLoc.getTriId()).getNormal().y < 0.0f)
	{
		return false;
	}

	// ----------

	FloorTri const & F = enterLoc.getFloorTri();

	FloorEdgeType edgeType = F.getEdgeType(enterLoc.getEdgeId());

	if (edgeType == FET_Uncrossable)
	{
		return false;
	}
	else if (edgeType == FET_Crossable)
	{
		if (F.isFallthrough())
		{
			return true;
		}
		else
		{
			return enterLoc.getOffset() >= -1.0f;
		}
	}
	else if (edgeType == FET_WallBase)
	{
		return enterLoc.getOffset() >= 0.0f;
	}
	else
	{
		DEBUG_WARNING(true, ("FloorMesh::canEnterEdge - bad edge type"));
		return false;
	}
}

bool FloorMesh::canEnterEdge(FloorLocator const & startLoc, Vector const & delta, FloorEdgeId const & id, bool useRadius) const
{
	FloorLocator entryLoc;

	if (intersectEdge(startLoc, delta, id, useRadius, false, entryLoc))
	{
		return canEnterEdge(entryLoc);
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------
// Crossable edges can be exited from anywhere
// uncrossable edges cannot be exited

// Ramp edges can be exited only if doing so causes a valid reentrance

bool FloorMesh::canExitEdge(FloorLocator const & exitLoc, Vector const & delta, bool useRadius) const
{
	if (exitLoc.getTriId() == -1) return false;
	if (exitLoc.getEdgeId() == -1) return false;
	if (exitLoc.getFloorMesh() == nullptr) return false;

	FloorEdgeType edgeType = exitLoc.getFloorTri().getEdgeType(exitLoc.getEdgeId());

	Vector exitPos = exitLoc.getPosition_p();

	UNREF(exitPos);

	if (edgeType == FET_Uncrossable)
	{
		return false;
	}
	else if (edgeType == FET_Crossable)
	{
		return true;
	}
	else if (edgeType == FET_WallBase)
	{
		// Can only enter through a non-fallthrough tri if the path crosses over
		// the boundary

		FloorLocator entryLoc;

		// this is a bit messy - we want to see if there's a crossable edge immediately after
		// the ramp edge that the locator can enter through, so we chop the move delta down
		// to 10 cm long and use that for the test.

		Vector testDelta = delta;

		if (testDelta.magnitudeSquared() > 0.01f)
		{
			testDelta.normalize();

			testDelta *= 0.1f;
		}

		if (findEntrance(exitLoc, testDelta, useRadius, entryLoc))
		{
			Vector enterPos = entryLoc.getPosition_p();

			UNREF(enterPos);

			return true;
		}
		else
		{
			// if we're moving away from the edge, we consider it to be exitable
			// (needed to do this to prevent a bug, but what was the bug?)

			Vector edgeDir = getTriangle(exitLoc.getTriId()).getEdgeDir(exitLoc.getEdgeId());

			Vector edgeNormal(edgeDir.z, 0.0f, -edgeDir.x);

			if (delta.dot(edgeNormal) > 0.0f)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		DEBUG_WARNING(true, ("FloorMesh::canExitEdge - bad edge type"));

		return false;
	}
}

// ----------

bool FloorMesh::canExitEdge(FloorLocator const & startLoc, Vector const & delta, FloorEdgeId const & id, bool useRadius) const
{
	if (id.triId == -1) return false;
	if (id.edgeId == -1) return false;

	FloorEdgeType edgeType = getFloorTri(id.triId).getEdgeType(id.edgeId);

	if (edgeType == FET_Uncrossable)
	{
		return false;
	}
	else if (edgeType == FET_Crossable)
	{
		return true;
	}
	else
	{
		FloorLocator exitLoc;

		if (intersectEdge(startLoc, delta, id, useRadius, true, exitLoc))
		{
			Vector newDelta = delta - (delta * exitLoc.getTime());

			return canExitEdge(exitLoc, newDelta, useRadius);
		}
		else
		{
			return false;
		}
	}
}

// ----------------------------------------------------------------------

PathWalkResult FloorMesh::findStartingTri(FloorLocator const & startLoc, Vector const & delta, bool useRadius, int & outTriId) const
{
	FloorLocator entryLoc;

	if (findEntrance(startLoc, delta, useRadius, entryLoc))
	{
		outTriId = entryLoc.getId();

		return PWR_WalkOk;
	}
	else
	{
		return PWR_DoesntEnter;
	}
}

// ----------------------------------------------------------------------

bool FloorMesh::findClosestLocation(FloorLocator const & testLoc, FloorLocator & result) const
{
	Vector point = testLoc.getPosition_l();

	result = FloorLocator::invalid;

	real minDist = REAL_MAX;
	Vector minPoint = Vector::zero;
	int minId = -1;

	for (int i = 0; i < getTriCount(); i++)
	{
		Triangle3d T = getTriangle(i);

		Vector triPoint = Distance3d::ClosestPointTri(point, T);

		real dist = (point - triPoint).magnitudeSquared();

		if (dist < minDist)
		{
			if (T.getNormal().y < 0.0f)
			{
				// Triangle's facing down, skip it.
				continue;
			}

			minDist = dist;
			minPoint = triPoint;
			minId = i;
		}
	}

	if (minId == -1)
	{
		result = FloorLocator::invalid;

		return false;
	}

	// ----------

	Vector scooted = Collision3d::MoveIntoTriangle(minPoint, getTriangle(minId), ConfigSharedCollision::getWallEpsilon());

	result = FloorLocator(this, scooted, minId, 0.0f, 0.0f);

	return true;
}

// ----------------------------------------------------------------------

bool FloorMesh::calcHitTime(Segment3d const & moveSeg, int triId, int edgeId, float & outHitTime) const
{
	Line3d moveLine = moveSeg.getLine();

	Segment3d edge = getTriangle(triId).getEdgeSegment(edgeId);

	float hitTime;

	float param;

	bool hit = Intersect2d::IntersectLineSeg(moveLine, edge, hitTime, param);

	if (hit)
	{
		outHitTime = hitTime;

		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

bool FloorMesh::attach(FloorLocator & loc) const
{
	if (loc.getFloorMesh() != this)
	{
		loc.setFloorMesh(this);

		FloorLocator tempLoc;

		bool dropOK = dropTest(loc, tempLoc);

		if (dropOK) loc = tempLoc;

		return dropOK;
	}
	else
	{
		return true;
	}
}

// ----------------------------------------------------------------------

PathWalkResult FloorMesh::pathWalkPoint(FloorLocator const & startLoc, Vector const & delta,
	int & outHitTriId, int & outHitEdge, float & outHitTime) const
{
	Vector begin = startLoc.getPosition_l();
	Vector end = begin + delta;

	Segment3d pathSegment(begin, end);
	Ribbon3d pathRibbon(pathSegment, -Vector::unitY);

	outHitTriId = -1;
	outHitEdge = -1;
	outHitTime = REAL_MAX;

	// ----------
	// Figure out the starting tri if we don't know it already

	int startTriId = startLoc.getId();

	if (startTriId == -1)
	{
		PathWalkResult entryResult = findStartingTri(startLoc, delta, false, startTriId);

		if (entryResult == PWR_DoesntEnter)
		{
			return PWR_DoesntEnter;
		}
		else if (entryResult == PWR_CantEnter)
		{
			return PWR_CantEnter;
		}
	}
	else
	{
		Triangle3d startTri = getTriangle(startTriId);

		if (!Overlap2d::TestPointTri(begin, startTri))
		{
			int enterTriEdge = Collision3d::TestEntranceTri(pathRibbon, startTri);

			if (enterTriEdge == -1)
			{
				return PWR_DoesntEnter;
			}
			else
			{
				FloorEdgeId id(startTriId, enterTriEdge);

				if (!canEnterEdge(startLoc, delta, id, false))
				{
					return PWR_CantEnter;
				}
			}
		}
	}

	// ----------

	int currentTriId = startTriId;

	// trap for an infinite loop - fan
	int numIterations = 0;

	const int maxIterations = 100;

	while (currentTriId != -1 && numIterations < maxIterations)
	{
		Triangle3d currentTri = getTriangle(currentTriId);

		if (Overlap2d::TestPointTri(end, currentTri))
		{
			outHitTriId = currentTriId;
			outHitEdge = -1;
			outHitTime = REAL_MAX;

			return PWR_WalkOk;
		}

		// ----------

		int hitEdgeId = Collision3d::TestExitTri(pathRibbon, currentTri);

		if (hitEdgeId == -1)
		{
			outHitTriId = currentTriId;
			outHitEdge = -1;
			outHitTime = REAL_MAX;

			return PWR_WalkFailed;
		}

		float tempHitTime = 0.0f;

		bool calcHitOK = calcHitTime(pathSegment, currentTriId, hitEdgeId, tempHitTime);

		if (!calcHitOK)
		{
			FLOOR_WARNING("FloorMesh::pathWalkPoint - Couldn't calc exit time");

			return PWR_WalkFailed;
		}
		else
		{
			FloorEdgeId id(currentTriId, hitEdgeId);

			if (!canExitEdge(startLoc, delta, id, false))
			{
				Vector contactDelta = delta * tempHitTime;

				float contactDist = contactDelta.magnitude();

				if (contactDist <= wallEpsilon)
				{
					// The starting point is extremely close to the contact point, close enough that
					// the starting point can be considered to be in contact with its hit edge

					outHitTriId = currentTriId;
					outHitEdge = hitEdgeId;
					outHitTime = tempHitTime;

					return PWR_InContact;
				}
				else
				{
					outHitTriId = currentTriId;
					outHitEdge = hitEdgeId;
					outHitTime = tempHitTime;

					return PWR_HitEdge;
				}
			}
		}

		// ----------

		int neighborTriId = getFloorTri(currentTriId).getNeighborIndex(hitEdgeId);

		if (neighborTriId == -1)
		{
			float tempHitTime(0);

			bool calcHitOK = calcHitTime(pathSegment, currentTriId, hitEdgeId, tempHitTime);

			if (!calcHitOK)
			{
				FLOOR_WARNING("FloorMesh::pathWalkPoint - Couldn't calc exit time");

				return PWR_WalkFailed;
			}
			else
			{
				FloorTri const & floorTri = getFloorTri(currentTriId);

				int portalId = floorTri.getPortalId(hitEdgeId);

				if (portalId == -1)
				{
					outHitTriId = currentTriId;
					outHitEdge = hitEdgeId;
					outHitTime = tempHitTime;

					return PWR_ExitedMesh;
				}
				else
				{
					outHitTriId = currentTriId;
					outHitEdge = hitEdgeId;
					outHitTime = tempHitTime;

					return PWR_HitPortalEdge;
				}
			}
		}

		if (neighborTriId == currentTriId)
		{
			outHitTriId = currentTriId;
			outHitEdge = hitEdgeId;
			outHitTime = REAL_MAX;

			return PWR_WalkFailed;
		}

		// ----------
		// Path didn't clip against this tri, step to the adjacent tri

		currentTriId = neighborTriId;
		++numIterations;
	}

	// Should never get here, but...
	if (numIterations >= maxIterations)
		FLOOR_WARNING("FloorMesh::pathWalkPoint - numInterations exceeded maxInterations");
	else
		FLOOR_WARNING("FloorMesh::pathWalkPoint - couldn't find a neighbortTriId");

	outHitTriId = -1;
	outHitEdge = -1;
	outHitTime = REAL_MAX;

	return PWR_WalkFailed; //lint !e527
}

// ----------------------------------------------------------------------

void FloorMesh::setPartTags(void)
{
	FloorTriStack unprocessed;

	for (int i = 0; i < getTriCount(); i++)
	{
		FloorTri * tri = &getFloorTri(i);

		tri->setPartTag(-1);

		unprocessed.push(tri);
	}

	FloorTriStack processing;

	int currentTag = 0;

	while (!unprocessed.empty())
	{
		FloorTri * currentTri = unprocessed.top();
		unprocessed.pop();

		if (currentTri->getPartTag() != -1) continue;

		processing.push(currentTri);

		while (!processing.empty())
		{
			FloorTri * tri = processing.top();
			processing.pop();

			if (tri->getPartTag() != -1) continue;

			tri->setPartTag(currentTag);

			int neighborA = tri->getNeighborIndex(0);
			int neighborB = tri->getNeighborIndex(1);
			int neighborC = tri->getNeighborIndex(2);

			if (neighborA != -1) processing.push(&getFloorTri(neighborA));
			if (neighborB != -1) processing.push(&getFloorTri(neighborB));
			if (neighborC != -1) processing.push(&getFloorTri(neighborC));
		}

		currentTag++;
	}
}

// ----------

int FloorMesh::getPartCount(void) const
{
	int temp = 0;

	for (int i = 0; i < getTriCount(); i++)
	{
		int tag = getFloorTri(i).getPartTag();

		if (tag > temp) temp = tag;
	}

	return temp + 1;
}

// ----------------------------------------------------------------------
// compute the height functions for each tri in the floor

void FloorMesh::calcHeightFuncs(void)
{
	int triCount = getTriCount();

	for (int i = 0; i < triCount; i++)
	{
		FloorTri & T = getFloorTri(i);

		Vector A = getVertex(T.getCornerIndex(0));

		Vector N = T.getNormal();

		if (N.y == 0.0f) continue;

		float da = A.dot(N);

		float dydx = -(N.x / N.y);
		float dydz = -(N.z / N.y);
		float c = (da / N.y);

		Vector heightFunc(dydx, c, dydz);

		T.setHeightFunc(heightFunc);
	}
}

// ----------------------------------------------------------------------
// Given an array of points representing a portal, flag edges of the
// floor mesh adjacent to the portal as being connected to it. Returns
// true if the portal was matched to any edge of the floor

bool FloorMesh::matchSegmentToPoly(Vector const & a, Vector const & b, VectorVector const & polyVerts)
{
	Plane3d polyPlane(polyVerts[0], polyVerts[1], polyVerts[2]);

	// ----------
	// Test 1 - Match the segment to the poly if it projects onto the interior
	// of the poly and is less than 10 centimeters away from the poly's plane.

	//ClosestPointPlane can't handle a zero length normal vector
	if (polyPlane.getNormal().magnitudeSquared() > Vector::NORMALIZE_THRESHOLD)
	{
		Vector a2 = Distance3d::ClosestPointPlane(a, polyPlane);
		Vector b2 = Distance3d::ClosestPointPlane(b, polyPlane);

		if (a2.inPolygon(polyVerts) && b2.inPolygon(polyVerts))
		{
			real distA = (a2 - a).magnitude();
			real distB = (b2 - b).magnitude();

			if ((distA < 0.1f) && (distB < 0.1f)) return true;
		}
	}

	// ----------
	// Test 2 - Match the segment to the poly if the vertices match a portal edge
	// to within 5 centimeters

	int vertCount = polyVerts.size();

	for (int i = 0; i < vertCount; i++)
	{
		Vector pa = polyVerts[i + 0];
		Vector pb = polyVerts[(i + 1) % vertCount];

		Segment3d S(pa, pb);

		Vector a2 = Distance3d::ClosestPointSeg(a, S);
		Vector b2 = Distance3d::ClosestPointSeg(b, S);

		// See if the distance between the verts is less than 5 centimeters for
		// both ends

		real distA = (a2 - a).magnitude();
		real distB = (b2 - b).magnitude();

		if ((distA < 0.05f) && (distB < 0.05f)) return true;
	}

	// ----------
	// Test 3 - Match the segment to the poly if the vertices are very very close
	// to the poly

	{
		Vector a2 = Distance3d::ClosestPointPoly(a, polyVerts);
		Vector b2 = Distance3d::ClosestPointPoly(b, polyVerts);

		real distA = (a2 - a).magnitude();
		real distB = (b2 - b).magnitude();

		if ((distA < 0.01f) && (distB < 0.01f)) return true;
	}

	return false;
}

// ----------------------------------------------------------------------

void FloorMesh::findAdjacentBoundaryEdges(VectorVector const & polyVerts, EdgeIdVec & outIds) const
{
	if (polyVerts.size() < 3) return;

	for (int currentTri = 0; currentTri < getTriCount(); currentTri++)
	{
		FloorTri const & F = getFloorTri(currentTri);
		Triangle3d T = getTriangle(currentTri);

		for (int currentEdge = 0; currentEdge < 3; currentEdge++)
		{
			// don't try and match internal edges

			if (F.getNeighborIndex(currentEdge) != -1)
			{
				continue;
			}

			Vector a = T.getCorner(currentEdge);
			Vector b = T.getCorner(currentEdge + 1);

			if (matchSegmentToPoly(a, b, polyVerts))
			{
				outIds.push_back(EdgeId(currentTri, currentEdge));
			}
		}
	}
}

// ----------------------------------------------------------------------

bool FloorMesh::flagPortalEdges(VectorVector const & portalVerts, int portalId)
{
	EdgeIdVec tempIds;

	findAdjacentBoundaryEdges(portalVerts, tempIds);

	int idCount = tempIds.size();

	for (int i = 0; i < idCount; i++)
	{
		EdgeId const & id = tempIds[i];

		getFloorTri(id.first).setPortalId(id.second, portalId);
	}

	return !tempIds.empty();
}

// ----------------------------------------------------------------------

bool FloorMesh::validate(FloorLocator const & loc) const
{
	if (loc.getId() == -1)
	{
		return true;
	}

	FloorLocator temp;

	if (!testIntersect(Line3d(loc.getPosition_l(), -Vector::unitY), loc.getId(), temp))
	{
		FLOOR_WARNING("FloorMesh::validate - Validate locator failed");
		return false;
	}
	else
	{
		return true;
	}
}

// ----------------------------------------------------------------------

PathWalkResult FloorMesh::pathWalkCircle(FloorLocator const & startLoc,
	Vector const & delta,
	int ignoreTriId,
	int ignoreEdge,
	FloorLocator & result) const
{
	float radius = startLoc.getRadius();

	// ----------
	// run the path walk

	Vector begin = startLoc.getPosition_l();

	float hitTime = REAL_MAX;
	int hitId = -1;
	int hitEdgeId = -1;
	int centerTriId = -1;

	PathWalkResult walkResult = pathWalkCircleGetIds(startLoc, delta, ignoreTriId, ignoreEdge, hitTime, hitId, hitEdgeId, centerTriId);

	// ----------
	// use the results to build a FloorLocator

	if (walkResult == PWR_DoesntEnter)
	{
		FloorLocator endLoc(this, begin + delta, -1, 0.0f, radius);

		makeSuccessResult(endLoc, result);

		return walkResult;
	}
	else if (walkResult == PWR_HitBeforeEnter)
	{
		makeHitResult2(begin, delta, centerTriId, hitId, hitEdgeId, hitTime, result);

		result.setRadius(radius);

		return walkResult;
	}
	else if ((walkResult == PWR_MissedStartTri) || (walkResult == PWR_CantEnter) || (walkResult == PWR_StartLocInvalid) || (walkResult == PWR_WalkFailed) || (walkResult == PWR_HitPast))
	{
		makeFailureResult(result);

		result.setRadius(radius);

		return walkResult;
	}
	else if ((walkResult == PWR_HitEdge) || (walkResult == PWR_InContact))
	{
		makeHitResult2(begin, delta, centerTriId, hitId, hitEdgeId, hitTime, result);

		result.setRadius(radius);

		return walkResult;
	}
	else if ((walkResult == PWR_CenterHitEdge) || (walkResult == PWR_CenterInContact))
	{
		makeHitResult(begin, delta, hitId, hitEdgeId, hitTime, result);

		result.setRadius(radius);

		return walkResult;
	}
	else if (walkResult == PWR_ExitedMesh)
	{
		makeExitResult(begin, delta, hitId, hitEdgeId, hitTime, result);

		result.setRadius(radius);

		return walkResult;
	}
	else if (walkResult == PWR_HitPortalEdge)
	{
		makeExitResult(begin, delta, hitId, hitEdgeId, hitTime, result);

		result.setRadius(radius);

		return walkResult;
	}
	else if (walkResult == PWR_WalkOk)
	{
		Line3d endLine(begin + delta, -Vector::unitY);
		FloorLocator dropLoc;

		if (centerTriId != -1)
		{
			if (!testIntersect(endLine, centerTriId, dropLoc))
			{
				FLOOR_WARNING("FloorMesh::pathWalkCircle - Walk was OK, but end point isn't in the end triangle");

				makeFailureResult(result);

				result.setRadius(radius);

				return PWR_WalkFailed;
			}
		}

		dropLoc.setRadius(radius);

		makeSuccessResult(dropLoc, result);

		return walkResult;
	}
	else
	{
		FLOOR_WARNING("FloorMesh::pathWalkCircle - Got an invalid return value from pathWalkCircle");

		makeFailureResult(result);

		result.setRadius(radius);

		return PWR_WalkFailed;
	}
}

// ----------------------------------------------------------------------
// Sweep a circle along the floor and return the first time of collision
// of the circle with any non-crossable edge in the floor.

// This method is a little tricky - when we're sliding the circle across
// the floor, the circle can hit multiple edge of each triangle it touches.
// We have to keep track of the neighbor triangles of each edge it touches
// amd check all their edges also, but we don't want to check any triangle's
// edges twice. So, we keep a deque of the triangles the circle has hit and
// use mark values to keep from visiting triangles twice. This method ends
// up being quite speedy.

// Returns true if something was hit.

struct EdgeQueueEntry
{
	EdgeQueueEntry(int id, float time)
		: triId(id), hitTime(time)
	{
	}

	// This compare returns A.hitTime > B.hitTime so that the queue
	// will be ordered earliest-to-latest

	static bool compare(EdgeQueueEntry const & A, EdgeQueueEntry const & B)
	{
		return A.hitTime > B.hitTime;
	}

	bool operator < (EdgeQueueEntry const & A) const
	{
		return hitTime > A.hitTime;
	}

	int    triId;
	float  hitTime;
};

typedef std::priority_queue< EdgeQueueEntry > FloorEdgeQueue;
typedef std::stack<int> FloorTriIdStack;

// ----------

PathWalkResult FloorMesh::pathWalkCircleGetIds(FloorLocator const & inStartLoc, Vector const & delta,
	int ignoreTriId, int ignoreEdge,
	float & outHitTime, int & outHitTriId, int & outHitEdge, int & outCenterTriId) const
{
	FloorLocator startLoc = inStartLoc;

	Vector begin = startLoc.getPosition_l();
	Vector end = begin + delta;
	Line3d beginLine(begin, -Vector::unitY);
	Ribbon3d pathRibbon(begin, end, -Vector::unitY);

	Circle circle(begin, startLoc.getRadius());

	outHitTime = REAL_MAX;
	outHitTriId = -1;
	outHitEdge = -1;
	outCenterTriId = -1;

	// ----------
	// Figure out the starting tri if we don't know it already

	int startTriId = startLoc.getId();

	if (startTriId == -1)
	{
		PathWalkResult entryResult = findStartingTri(startLoc, delta, true, startTriId);

		if (entryResult == PWR_DoesntEnter)
		{
			return PWR_DoesntEnter;
		}
		else if (entryResult == PWR_CantEnter)
		{
			return PWR_CantEnter;
		}

		startLoc.setId(startTriId);
	}
	else
	{
		Triangle3d startTri = getTriangle(startTriId);

		if (!Overlap3d::TestLineTriSided(beginLine, startTri))
		{
			int enterTriEdge = Collision3d::TestEntranceTri(pathRibbon, startTri);

			if (enterTriEdge == -1)
			{
				return PWR_DoesntEnter;
			}
			else
			{
				FloorEdgeId id(startTriId, enterTriEdge);

				if (!canEnterEdge(startLoc, delta, id, true))
				{
					return PWR_CantEnter;
				}
			}
		}
	}

	// ----------
	// First things first - Find out if and when the center of the circle will
	// exit the floor during this timestep

	PathWalkResult centerWalkResult;
	float centerHitTime = REAL_MAX;
	int centerHitTriId = -1;
	int centerHitEdge = -1;

	{
		centerWalkResult = pathWalkPoint(startLoc, delta, centerHitTriId, centerHitEdge, centerHitTime);

		if ((centerWalkResult == PWR_MissedStartTri)
			|| (centerWalkResult == PWR_CantEnter)
			|| (centerWalkResult == PWR_WalkFailed)
			|| (centerWalkResult == PWR_StartLocInvalid))
		{
			return centerWalkResult;
		}
		else if (centerWalkResult == PWR_InContact)
		{
			// The center of the circle is in contact with an edge and will stay in contact during this timestep.

			outHitTime = centerHitTime;
			outHitTriId = centerHitTriId;
			outHitEdge = centerHitEdge;
			outCenterTriId = centerHitTriId;

			return PWR_CenterInContact;
		}
		else if ((centerWalkResult == PWR_ExitedMesh) || (centerWalkResult == PWR_HitEdge) || (centerWalkResult == PWR_HitPortalEdge))
		{
			// The center of the circle will hit an edge during this timestep
		}
		else
		{
		}
	}

	// ----------

	bool hitAnything = false;
	float circleHitTime = 1.0f;
	int circleHitTriId = -1;
	int circleHitEdge = -1;

	int markValue = getTriMarkValue();

	FloorEdgeQueue unprocessed;

	unprocessed.push(EdgeQueueEntry(startTriId, 0.0f));

	while (!unprocessed.empty())
	{
		int currentTriId = unprocessed.top().triId;
		unprocessed.pop();

		FloorTri const & currentTri = getFloorTri(currentTriId);

		if (currentTri.getMark() == markValue) continue;

		currentTri.setMark(markValue);

		// ----------
		// Test for hits with each edge of the tri

		for (int i = 0; i < 3; i++)
		{
			// See if it's even possible to hit this edge

			if ((ignoreTriId == currentTriId) && (ignoreEdge == i))
			{
				continue;
			}

			Segment3d edge = getTriangle(currentTriId).getEdgeSegment(i);

			Vector edgeDir = edge.getDelta();

			Vector edgeNormal(edgeDir.z, 0.0f, -edgeDir.x);

			Vector V(delta.x, 0.0f, delta.z);

			if (edgeNormal.dot(V) > 0.0f)
			{
				// circle is moving away from the edge

				// ramp edges can only be hit from the front

				FloorEdgeType edgeType = getFloorTri(currentTriId).getEdgeType(i);

				if (edgeType == FET_WallBase)
				{
					continue;
				}
			}

			// Compute the time of intersection between the circle and the edge

			Range hitRange = Intersect2d::IntersectCircleSeg(circle, V, edge);

			// Ignore the hit if it ended in the past, starts further in the future than our
			// earliest hit, or starts after the circle has already left the floor

			if (hitRange.isEmpty()) continue;
			if (hitRange.getMax() < 0.0f) continue;
			if (hitRange.getMin() > circleHitTime) continue;

			if ((centerWalkResult == PWR_ExitedMesh) || (centerWalkResult == PWR_HitPortalEdge) || (centerWalkResult == PWR_HitEdge))
			{
				if (hitRange.getMin() > centerHitTime) continue;
			}

			// This hit needs processing.

			FloorEdgeId id(currentTriId, i);

			if (canExitEdge(startLoc, delta, id, true))
			{
				// Crossable edge hit - push the adjacent triangle onto the stack

				int neighbor = currentTri.getNeighborIndex(i);

				if (neighbor != -1)
				{
					unprocessed.push(EdgeQueueEntry(neighbor, hitRange.getMin()));
				}
			}
			else
			{
				// If the circle is moving away from its contact point with the segment, there's no contact

				Vector endPoint = begin + V * hitRange.getMin();

				Vector contactPoint = Distance2d::ClosestPointSeg(endPoint, edge);
				Vector contactNormal = endPoint - contactPoint;

				contactNormal.y = 0.0f;

				if (contactNormal.dot(V) >= 0.0f)
				{
					continue;
				}

				if (hitRange.getMin() == hitRange.getMax())
				{
					// Degenerate hit - circle grazes one end of the segment. Ignore the hit.

					continue;
				}
				else if (hitRange.getMin() < 0.0f)
				{
					float stepDistance = delta.magnitude();

					float contactDistance = stepDistance * -hitRange.getMin();

					if (contactDistance < wallEpsilon)
					{
						circleHitTime = hitRange.getMin();
						circleHitTriId = currentTriId;
						circleHitEdge = i;
						hitAnything = true;
					}
					else
					{
						continue;
					}
				}
				else
				{
					// Non-crossable edge hit - record the hit

					circleHitTime = hitRange.getMin();
					circleHitTriId = currentTriId;
					circleHitEdge = i;
					hitAnything = true;
				}
			}
		}
	}

	// ----------

	if (hitAnything)
	{
		float stepDistance = delta.magnitude();

		float contactDist = std::abs(stepDistance * circleHitTime);

		if ((circleHitTime < 0) && (contactDist > wallEpsilon))
		{
			// The circle hit an edge some time in the past.

			outHitTime = circleHitTime;
			outHitTriId = circleHitTriId;
			outHitEdge = circleHitEdge;
			outCenterTriId = startTriId;

			return PWR_HitPast;
		}
		else
		{
			if (contactDist <= wallEpsilon)
			{
				// The circle is currently in contact with an edge

				outHitTime = circleHitTime;
				outHitTriId = circleHitTriId;
				outHitEdge = circleHitEdge;
				outCenterTriId = startTriId;

				return PWR_InContact;
			}
			else
			{
				// The circle hits an edge during this timestep.
				// Walk the center of the circle to where it will be at the contact time

				Vector newDelta = delta * circleHitTime;

				int newCenterTriId = -1;
				int newCenterHitEdge = -1;
				float newCenterHitTime = REAL_MAX;

				PathWalkResult result = pathWalkPoint(startLoc, newDelta, newCenterTriId, newCenterHitEdge, newCenterHitTime);

				if (result == PWR_HitEdge)
				{
					outHitTime = newCenterHitTime;
					outHitTriId = newCenterTriId;
					outHitEdge = newCenterHitEdge;
					outCenterTriId = newCenterTriId;

					return PWR_CenterHitEdge;
				}
				else if (result == PWR_InContact)
				{
					outHitTime = newCenterHitTime;
					outHitTriId = newCenterTriId;
					outHitEdge = newCenterHitEdge;
					outCenterTriId = newCenterTriId;

					return PWR_CenterInContact;
				}
				else if (result == PWR_DoesntEnter)
				{
					outHitTime = circleHitTime;
					outHitTriId = circleHitTriId;
					outHitEdge = circleHitEdge;
					outCenterTriId = -1;

					return PWR_HitBeforeEnter;
				}
				else if (result != PWR_WalkOk)
				{
					FLOOR_WARNING("FloorMeshpathWalkCircleGetIds - Walking the center to the time of the circle's contact failed");
					return PWR_WalkFailed;
				}

				outHitTime = circleHitTime;
				outHitTriId = circleHitTriId;
				outHitEdge = circleHitEdge;
				outCenterTriId = newCenterTriId;

				return PWR_HitEdge;
			}
		}
	}
	else
	{
		if (centerWalkResult == PWR_HitEdge)
		{
			outHitTime = centerHitTime;
			outHitTriId = centerHitTriId;
			outHitEdge = centerHitEdge;
			outCenterTriId = centerHitTriId;

			return PWR_CenterHitEdge;
		}
		else if (centerWalkResult == PWR_InContact)
		{
			outHitTime = centerHitTime;
			outHitTriId = centerHitTriId;
			outHitEdge = centerHitEdge;
			outCenterTriId = centerHitTriId;

			return PWR_CenterInContact;
		}
		else if (centerWalkResult == PWR_ExitedMesh)
		{
			outHitTime = centerHitTime;
			outHitTriId = centerHitTriId;
			outHitEdge = centerHitEdge;
			outCenterTriId = centerHitTriId;

			return PWR_ExitedMesh;
		}
		else if (centerWalkResult == PWR_HitPortalEdge)
		{
			outHitTime = centerHitTime;
			outHitTriId = centerHitTriId;
			outHitEdge = centerHitEdge;
			outCenterTriId = centerHitTriId;

			return PWR_HitPortalEdge;
		}
		else
		{
			// The circle doesn't hit anything, and the center doesn't exit the floor

			outHitTime = REAL_MAX;
			outHitTriId = -1;
			outHitEdge = -1;
			outCenterTriId = centerHitTriId;

			return PWR_WalkOk;
		}
	}

	WARNING_STRICT_FATAL(true, ("pathWalkCircleGetIds failed to compute a result. Return PWR_WalkOk"));
	outHitTime = REAL_MAX;
	outHitTriId = -1;
	outHitEdge = -1;
	outCenterTriId = centerHitTriId;

	return PWR_WalkOk;
}

// ----------------------------------------------------------------------

bool FloorMesh::testAboveCrossables(FloorLocator const & testLoc) const
{
	Vector offset(0.0f, gs_hopTolerance, 0.0f);

	Circle circle(testLoc.getOffsetPosition_l() + offset, testLoc.getRadius() - gs_clearTolerance);

	int edgeCount = m_crossableEdges->size();

	for (int i = 0; i < edgeCount; i++)
	{
		FloorEdgeId id = m_crossableEdges->at(i);

		if (getFloorTri(id.triId).isFallthrough()) continue;

		Segment3d edge = getTriangle(id.triId).getEdgeSegment(id.edgeId);

		if (Overlap3d::TestSegCircle_Below(edge, circle))
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------

bool FloorMesh::testAboveCrossables(FloorLocator const & testLoc, Vector const & inDelta) const
{
	DEBUG_FATAL(inDelta.y != 0.0f, ("FloorMesh::testCrossables - can't handle Y movement yet\n"));

	Vector delta(inDelta.x, 0.0f, inDelta.z);

	Vector offset(0.0f, gs_hopTolerance, 0.0f);

	Circle circle(testLoc.getOffsetPosition_l() + offset, testLoc.getRadius() - gs_clearTolerance);

	int edgeCount = m_crossableEdges->size();

	for (int i = 0; i < edgeCount; i++)
	{
		FloorEdgeId id = m_crossableEdges->at(i);

		if (getFloorTri(id.triId).isFallthrough()) continue;

		Segment3d edge = getTriangle(id.triId).getEdgeSegment(id.edgeId);

		Range hitRange = Intersect3d::IntersectCircleSeg_Below(circle, delta, edge);

		if (hitRange.isEmpty()) continue;

		if ((hitRange.getMin() <= 1.0f) && (hitRange.getMax() >= 0.0f)) return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool FloorMesh::testClear(FloorLocator const & inTestLoc) const
{
	FloorLocator testLoc = inTestLoc;

	attach(testLoc);

	int startTriId = testLoc.getId();

	if (startTriId == -1)
	{
		return true;
	}

	Circle testCircle(testLoc.getOffsetPosition_l(), testLoc.getRadius());

	// ----------

	int markValue = getTriMarkValue();

	static FloorTriIdQueue unprocessed;

	unprocessed.clear();
	unprocessed.push_back(startTriId);

	bool testedAboveCrossables = false;

	while (!unprocessed.empty())
	{
		int currentTriId = unprocessed.front();
		unprocessed.pop_front();

		FloorTri const & currentTri = getFloorTri(currentTriId);

		if (currentTri.getMark() == markValue) continue;

		currentTri.setMark(markValue);

		// ----------

		for (int i = 0; i < 3; i++)
		{
			Segment3d edge = getTriangle(currentTriId).getEdgeSegment(i);

			if (!Overlap2d::TestSegCircle(edge, testCircle)) continue;

			// ----------

			FloorEdgeType type = currentTri.getEdgeType(i);

			if (type == FET_Crossable)
			{
				// circle overlaps a crossable edge, push the neighbor onto the stack

				int neighbor = currentTri.getNeighborIndex(i);

				if (neighbor != -1)
				{
					unprocessed.push_back(neighbor);
				}

				continue;
			}
			else if (type == FET_Uncrossable)
			{
				// circle overlaps an uncrossable edge, circle is not clear

				return false;
			}
			else if (type == FET_WallBase)
			{
				// circle overlaps a wall base. circle is not clear if it's under a crossable
				// edge of a solid tri. we only need to run this test once.

				if (testedAboveCrossables) continue;

				if (!testAboveCrossables(testLoc))
				{
					return false;
				}

				testedAboveCrossables = true;
			}
			else
			{
				continue;
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------

bool FloorMesh::getClosestCollidableEdge(FloorLocator const & loc, int & outTriId, int & outEdgeId, float & outDist) const
{
	FloorLocator testLoc = loc;

	attach(testLoc);

	int startTriId = testLoc.getId();

	if (startTriId == -1)
	{
		return false;
	}

	Circle testCircle(testLoc.getPosition_l(), testLoc.getRadius());

	int minTriId = -1;
	int minEdgeId = -1;

	// ----------

	int markValue = getTriMarkValue();

	static FloorTriIdQueue unprocessed;

	unprocessed.clear();
	unprocessed.push_back(startTriId);

	bool testedAboveCrossables = false;
	bool isAboveCrossables = false;

	while (!unprocessed.empty())
	{
		int currentTriId = unprocessed.front();
		unprocessed.pop_front();

		FloorTri const & currentTri = getFloorTri(currentTriId);

		if (currentTri.getMark() == markValue) continue;

		currentTri.setMark(markValue);

		// ----------

		for (int i = 0; i < 3; i++)
		{
			Segment3d edge = getTriangle(currentTriId).getEdgeSegment(i);

			float dist = Distance2d::DistancePointSeg(testCircle.getCenter(), edge);

			if (dist >= testCircle.getRadius()) continue;

			// ----------

			FloorEdgeType type = currentTri.getEdgeType(i);

			if (type == FET_Crossable)
			{
				int neighbor = currentTri.getNeighborIndex(i);

				if (neighbor != -1)
				{
					unprocessed.push_back(neighbor);
				}
			}
			else if (type == FET_Uncrossable)
			{
				testCircle.setRadius(dist);

				minTriId = currentTriId;
				minEdgeId = i;
			}
			else if (type == FET_WallBase)
			{
				// circle overlaps a wall base. circle is not clear if it's under a crossable
				// edge of a solid tri. we only need to run this test once.

				if (testedAboveCrossables) continue;

				isAboveCrossables = testAboveCrossables(testLoc);
				testedAboveCrossables = true;

				if (!isAboveCrossables)
				{
					testCircle.setRadius(dist);

					minTriId = currentTriId;
					minEdgeId = i;
				}
			}
			else
			{
				continue;
			}
		}
	}

	if (minTriId != -1)
	{
		outTriId = minTriId;
		outEdgeId = minEdgeId;
		outDist = testCircle.getRadius();

		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

bool FloorMesh::testConnectable(FloorLocator const & locA, FloorLocator const & locB) const
{
	Vector posA = locA.getPosition_l();
	Vector posB = locB.getPosition_l();

	Vector deltaA = posB - posA;
	Vector deltaB = posA - posB;

	FloorLocator moveResultA;
	FloorLocator moveResultB;

	PathWalkResult resultA = pathWalkCircle(locA, deltaA, -1, -1, moveResultA);
	PathWalkResult resultB = pathWalkCircle(locB, deltaB, -1, -1, moveResultB);

	if ((resultA == PWR_ExitedMesh) || (resultA == PWR_HitPortalEdge))
	{
		// If the move from A didn't exit the floor at B, we can't connect the locators

		if (moveResultA.getTriId() != locB.getTriId())
		{
			return false;
		}
	}

	if ((resultB == PWR_ExitedMesh) || (resultB == PWR_HitPortalEdge))
	{
		// If the move from B didn't exit the floor at A, we can't connect the locators

		if (moveResultB.getTriId() != locA.getTriId())
		{
			return false;
		}
	}

	if ((resultA != PWR_WalkOk) && (resultA != PWR_ExitedMesh) && (resultA != PWR_HitPortalEdge)) return false;
	if ((resultB != PWR_WalkOk) && (resultB != PWR_ExitedMesh) && (resultB != PWR_HitPortalEdge)) return false;

	if (resultA == PWR_WalkOk)
	{
		if (moveResultA.getId() != locB.getId())
		{
			return false;
		}
	}

	if (resultB == PWR_WalkOk)
	{
		if (moveResultB.getId() != locA.getId())
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------
// The points returned by getGoodLocation won't be evenly distributed over
// the floor, but I don't think that's a problem.

bool FloorMesh::getGoodLocation(float radius, Vector & outLoc) const
{
	int triCount = getTriCount();

	int attempts = 10;

	while (attempts)
	{
		// Pick a random triangle in the floor

		int triIndex = Random::random(triCount - 1);

		Triangle3d const & tri = getTriangle(triIndex);

		// Pick a random barycentric coordinate

		float baryX = Random::randomReal(0.0f, 1.0f);
		float baryY = Random::randomReal(0.0f, 1.0f);

		if ((baryX + baryY) > 1.0f)
		{
			baryX = 1.0f - baryX;
			baryY = 1.0f - baryY;
		}

		// See if the point on the triangle at that
		// barycentric coordinate is clear

		Vector const & A = tri.getCornerA();
		Vector const & B = tri.getCornerB();
		Vector const & C = tri.getCornerC();

		Vector point = A + (B - A) * baryX + (C - A) * baryY;

		FloorLocator testLoc(this, point, -1, 0.0f, radius);

		if (testClear(testLoc))
		{
			outLoc = point;

			return true;
		}

		// ----------

		attempts--;
	}

	return false;
}

// ----------------------------------------------------------------------

int FloorMesh::getTriMarkValue(void) const
{
	return m_triMarkCounter++;
}

// ----------------------------------------------------------------------

void FloorMesh::buildBoundaryEdgeList(void)
{
	m_crossableEdges->clear();
	m_uncrossableEdges->clear();
	m_wallBaseEdges->clear();
	m_wallTopEdges->clear();

	for (int i = 0; i < getTriCount(); i++)
	{
		FloorTri const & F = getFloorTri(i);

		for (int j = 0; j < 3; j++)
		{
			if (F.getNeighborIndex(j) == -1)
			{
				FloorEdgeId id(i, j);

				switch (F.getEdgeType(j))
				{
				case FET_Crossable: { m_crossableEdges->push_back(id); break; }
				case FET_Uncrossable: { m_uncrossableEdges->push_back(id); break; }
				case FET_WallBase: { m_wallBaseEdges->push_back(id); break; }
				default: { break; }
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

bool FloorMesh::intersectEdge(FloorLocator const & startLoc, Vector const & delta, FloorEdgeId const & id, bool useRadius, bool testFront, FloorLocator & outLoc) const
{
	Vector point = startLoc.getPosition_l();

	Segment3d triSeg = getTriangle(id.triId).getEdgeSegment(id.edgeId);
	Segment3d edge(triSeg.getBegin(), triSeg.getEnd());

	float hitTime;

	if (useRadius)
	{
		Circle circle(point, startLoc.getRadius());

		Range hitRange = Intersect2d::IntersectCircleSeg(circle, delta, edge);

		if (hitRange.isEmpty()) return false;
		if (hitRange.getMax() < 0.0f) return false;
		if (hitRange.getMin() > 1.0f) return false;

		hitTime = hitRange.getMin();
	}
	else
	{
		float param;

		Line3d moveLine(point, delta);

		bool hitSeg = Intersect2d::IntersectLineSeg(moveLine, edge, hitTime, param);

		if (!hitSeg) return false;
		if (hitTime > 1.0f) return false;

		if (hitTime < 0.0f)
		{
			float dist = -delta.magnitude() * hitTime;

			if (dist > 0.2f)
			{
				return false;
			}
			else
			{
				int blah = 2;

				blah += (int)hitTime;

				UNREF(blah);
			}
		}
	}

	Vector cross = delta.cross(edge.getDelta());

	if (testFront)
	{
		if (cross.y < 0.0f) return false;
	}
	else
	{
		if (cross.y > 0.0f) return false;
	}

	Vector centerHitPos = point + delta * hitTime;
	Vector crossPoint = Distance3d::ClosestPointYLine(edge, centerHitPos);

	float offset = centerHitPos.y - crossPoint.y + startLoc.getOffset();

	Vector newPos(centerHitPos.x, crossPoint.y, centerHitPos.z);

	outLoc = startLoc;

	outLoc.setTriId(id.triId);
	outLoc.setEdgeId(id.edgeId);
	outLoc.setPosition_l(newPos);
	outLoc.setOffset(offset);
	outLoc.setTime(hitTime);

	return true;
}

// ----------------------------------------------------------------------

bool FloorMesh::intersectBoundary(FloorLocator const & startLoc, Vector const & delta, bool useRadius, FloorLocatorVec & results) const
{
	results.clear();

	bool result = false;

	result |= intersectBoundary(m_crossableEdges, startLoc, delta, useRadius, results);
	result |= intersectBoundary(m_wallBaseEdges, startLoc, delta, useRadius, results);

	return result;
}

// ----------

bool FloorMesh::intersectBoundary(FloorEdgeIdVec * edgeList, FloorLocator const & startLoc, Vector const & delta, bool useRadius, FloorLocatorVec & results) const
{
	bool hit = false;

	int edgeCount = edgeList->size();

	for (int i = 0; i < edgeCount; i++)
	{
		FloorEdgeId & id = edgeList->at(i);

		Vector V(delta.x, 0.0f, delta.z);

		FloorLocator hitLoc;

		bool hitEdgeId = intersectEdge(startLoc, V, id, useRadius, false, hitLoc);

		if (hitEdgeId)
		{
			hit = true;

			results.push_back(hitLoc);
		}
	}

	return hit;
}

// ----------------------------------------------------------------------

bool FloorMesh::dropTest(FloorLocator const & testLoc, FloorLocator & outLoc) const
{
	return dropTest(testLoc, ConfigSharedCollision::getHopHeight(), outLoc);
}

// ----------

bool FloorMesh::dropTest(FloorLocator const & testLoc, float hopHeight, FloorLocator & outLoc) const
{
	Vector point = testLoc.getPosition_l();

	Line3d line(point, -Vector::unitY);

	FloorLocator closestAbove;
	FloorLocator closestBelow;

	float fallHeight = m_objectFloor ? REAL_MAX : 3.0f;

	// Our drop dir points down, so the closestBelow locator is in 'front' of the line,
	// and the closestAbove locator is 'behind' it.

	if (findClosestPair(line, -1, closestBelow, closestAbove))
	{
		float distBelow = std::abs(closestBelow.getOffset());
		float distAbove = std::abs(closestAbove.getOffset());

		closestAbove.setSurface(this);
		closestBelow.setSurface(this);

		if (closestAbove.isAttached() && closestBelow.isAttached())
		{
			// Handle test points on or slightly below the floor correctly

			if ((distAbove < distBelow) && (distAbove < 1.0f))
			{
				outLoc = closestAbove;
			}
			else
			{
				outLoc = closestBelow;
			}
		}
		else if (closestBelow.isAttached())
		{
			if (distBelow <= fallHeight)
			{
				outLoc = closestBelow;
			}
			else
			{
				if (closestBelow.getFloorTri().isFallthrough())
				{
					outLoc = closestBelow;
				}
				else
				{
					outLoc = FloorLocator::invalid;
				}
			}
		}
		else if (closestAbove.isAttached())
		{
			if (distAbove <= hopHeight)
			{
				outLoc = closestAbove;
			}
			else
			{
				if (closestAbove.getFloorTri().isFallthrough())
				{
					outLoc = closestAbove;
				}
				else
				{
					outLoc = FloorLocator::invalid;
				}
			}
		}
		else
		{
			outLoc = FloorLocator::invalid;
		}
	}
	else
	{
		outLoc = FloorLocator::invalid;
	}

	// ----------

	return outLoc.isAttached();
}

// ----------------------------------------------------------------------

bool FloorMesh::dropTest(FloorLocator const & testLoc, int triID, FloorLocator & outLoc) const
{
	Vector point = testLoc.getPosition_l();

	Line3d line(point, -Vector::unitY);

	FloorLocator tempLoc;

	if (testIntersect(line, triID, tempLoc))
	{
		float absDist = std::abs(tempLoc.getOffset());

		if (absDist <= ConfigSharedCollision::getHopHeight())
		{
			outLoc = tempLoc;
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool FloorMesh::findEntrance(FloorLocator const & startLoc, Vector const & delta, bool useRadius, FloorLocator & outLoc) const
{
	static FloorLocatorVec results;

	results.clear();

	bool intersected = intersectBoundary(startLoc, delta, useRadius, results);

	if (!intersected) return false;

	// ----------

	float minDistA = REAL_MAX; // distance from the point on the boundary to the start of the segment
	float minDistB = REAL_MAX; // distance from the point on the boundary to the X-Z cross point

	int minIndex = -1;

	int resultCount = results.size();

	for (int i = 0; i < resultCount; i++)
	{
		FloorLocator const & entryLoc = results[i];

		float hitTime = entryLoc.getTime();

		Vector startOffsetPoint = startLoc.getOffsetPosition_l();
		Vector hitPoint = entryLoc.getPosition_l();

		Vector crossOffsetPoint = startOffsetPoint + delta * hitTime;

		Vector temp = hitPoint - startOffsetPoint;
		temp.y = 0.0f;

		float distA = temp.magnitude();
		float distB = hitPoint.magnitudeBetween(crossOffsetPoint);

		// For a crossing to be considered an entrance, it has to be within half a meter
		// (fudge factor) of the earliest crossing, and it has to be closer to the move
		// segment than the earliest crossing.

		if (distA >= (minDistA + 0.5f)) continue;
		if (distB >= minDistB) continue;

		float offset = entryLoc.getOffset();

		float hopHeight = -ConfigSharedCollision::getHopHeight();
		float fallHeight = m_objectFloor ? REAL_MAX : 3.0f;

		bool fallthrough = entryLoc.getFloorTri().isFallthrough();

		if ((offset < hopHeight) && (!fallthrough))
		{
			// crosses too far below solid tri

			continue;
		}

		if ((offset > fallHeight) && (!fallthrough))
		{
			// crosses too far above solid tri

			continue;
		}

		if (canEnterEdge(entryLoc))
		{
			minDistA = distA;
			minDistB = distB;
			minIndex = i;
		}
	}

	if (minIndex != -1)
	{
		outLoc = results[minIndex];

		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

void FloorMesh::drawDebugShapes(DebugShapeRenderer * renderer, bool drawExtent) const
{
	UNREF(renderer);
	UNREF(drawExtent);

#ifdef _DEBUG

	if (renderer == nullptr) return;

	if (ConfigSharedCollision::getDrawFloors())
	{
		if (m_crossableLines)      renderer->drawLineList(*m_crossableLines, VectorArgb::solidGreen);
		if (m_uncrossableLines)    renderer->drawLineList(*m_uncrossableLines, VectorArgb::solidRed);
		if (m_interiorLines)       renderer->drawLineList(*m_interiorLines, VectorArgb::solidWhite);
		if (m_portalLines)         renderer->drawLineList(*m_portalLines, PackedArgb(255, 220, 255, 0));
		if (m_rampLines)           renderer->drawLineList(*m_rampLines, PackedArgb(255, 255, 175, 0));
		if (m_fallthroughTriLines) renderer->drawLineList(*m_fallthroughTriLines, VectorArgb::solidRed);
		if (m_solidTriLines)       renderer->drawLineList(*m_solidTriLines, VectorArgb::solidWhite);

		if (drawExtent)
		{
			getExtent_l()->drawDebugShapes(renderer);
		}
	}

	if (ConfigSharedCollision::getDrawPathNodes())
	{
		ObjectRenderer pathRenderer = FloorManager::getPathGraphRenderer();

		if (m_pathGraph && pathRenderer)
		{
			pathRenderer(m_pathGraph, renderer);
		}
	}

	if (m_boxTree && ConfigSharedCollision::getDrawBoxTrees())
	{
		m_boxTree->drawDebugShapes(renderer);
	}

#endif
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void FloorMesh::buildDebugData(void)
{
	Vector scootUp(0.0f, 0.04f, 0.0f);

	// ----------
	// Create our line arrays for the debug info

	delete m_crossableLines;
	delete m_uncrossableLines;
	delete m_interiorLines;
	delete m_portalLines;
	delete m_rampLines;
	delete m_fallthroughTriLines;
	delete m_solidTriLines;

	m_crossableLines = new VectorVector();
	m_uncrossableLines = new VectorVector();
	m_interiorLines = new VectorVector();
	m_portalLines = new VectorVector();
	m_rampLines = new VectorVector();
	m_fallthroughTriLines = new VectorVector();
	m_solidTriLines = new VectorVector();

	// ----------

	for (int i = 0; i < getTriCount(); i++)
	{
		FloorTri const & F = getFloorTri(i);

		// Create border edges for the tri

		for (int j = 0; j < 3; j++)
		{
			Vector a = getVertex(F.getCornerIndex(j)) + scootUp;
			Vector b = getVertex(F.getCornerIndex(j + 1)) + scootUp;

			if (F.getNeighborIndex(j) != -1)
			{
				m_interiorLines->push_back(a);
				m_interiorLines->push_back(b);

				continue;
			}

			// Pick which container the edge should go in.

			FloorEdgeType edgeType = F.getEdgeType(j);

			VectorVector * container = m_uncrossableLines;

			if (edgeType == FET_Crossable) container = m_crossableLines;

			if (edgeType == FET_WallBase) container = m_rampLines;

			if (F.getPortalId(j) != -1) container = m_portalLines;

			// Edge indicators are duplicated 3 times to make them more visible

			for (int i = 0; i < 3; i++)
			{
				container->push_back(a + scootUp * float(i));
				container->push_back(b + scootUp * float(i));
			}
		}

		// Create the 'y's that are used to indicate fallthrough
		// tris

		VectorVector * container = F.isFallthrough() ? m_fallthroughTriLines : m_solidTriLines;

		{
			float lerp = 0.3f;

			Vector a = getVertex(F.getCornerIndex(0)) + scootUp;
			Vector b = getVertex(F.getCornerIndex(1)) + scootUp;
			Vector c = getVertex(F.getCornerIndex(2)) + scootUp;

			Vector center = (a + b + c) / 3.0f;

			a = a * lerp + center * (1.0f - lerp);
			b = b * lerp + center * (1.0f - lerp);
			c = c * lerp + center * (1.0f - lerp);

			container->push_back(a); container->push_back(center);
			container->push_back(b); container->push_back(center);
			container->push_back(c); container->push_back(center);
		}
	}
}

#endif

// ----------------------------------------------------------------------

bool FloorMesh::getDistanceUncrossable2d(Vector const & V, float maxDistance, float & outDistance, FloorEdgeId & outEdgeId) const
{
	if (!m_uncrossableEdges) return false;

	// ----------

	int count = m_uncrossableEdges->size();

	float minDistance = maxDistance;

	FloorEdgeId minId(-1, -1);

	for (int i = 0; i < count; i++)
	{
		FloorEdgeId const & id = m_uncrossableEdges->at(i);

		Segment3d edge = getTriangle(id.triId).getEdgeSegment(id.edgeId);

		float dist = Distance2d::DistancePointSeg(V, edge);

		if (dist < minDistance)
		{
			minDistance = dist;
			minId = id;
		}
	}

	if (minDistance != maxDistance)
	{
		outDistance = minDistance;
		outEdgeId = minId;

		return true;
	}
	else
	{
		return false;
	}
}

// ======================================================================
