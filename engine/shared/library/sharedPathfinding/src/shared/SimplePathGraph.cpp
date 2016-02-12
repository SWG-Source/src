// ======================================================================
//
// PathGraph.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedPathfinding/FirstSharedPathfinding.h"
#include "sharedPathfinding/SimplePathGraph.h"

#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedCollision/FloorLocator.h"

#include "sharedFile/Iff.h"

#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/Sphere.h"

#include "sharedPathfinding/PathNode.h"
#include "sharedPathfinding/PathEdge.h"

#include <vector>
#include <algorithm>

const Tag TAG_PGRF = TAG(P,G,R,F);
const Tag TAG_PNOD = TAG(P,N,O,D);
const Tag TAG_PEDG = TAG(P,E,D,G);
const Tag TAG_ECNT = TAG(E,C,N,T);
const Tag TAG_ESTR = TAG(E,S,T,R);
const Tag TAG_META = TAG(M,E,T,A);

// ----------------------------------------------------------------------

template<class T>
void readArray_Class ( Iff & iff, T * & array, Tag tag )
{
	iff.enterChunk(tag);
	{
		int count = iff.read_int32();

		if(array == nullptr) array = new T();

		array->resize(count);

		for(int i = 0; i < count; i++)
		{
			array->at(i).read(iff);
		}
	}
	iff.exitChunk(tag);
}

template<class T, class Reader>
void readArray_Class ( Iff & iff, T * & array, Tag tag, Reader R )
{
	iff.enterChunk(tag);
	{
		int count = iff.read_int32();

		if(array == nullptr) array = new T();

		array->resize(count);

		for(int i = 0; i < count; i++)
		{
			(array->at(i).*R)(iff);
		}
	}
	iff.exitChunk(tag);
}

template<class T>
void readArray_Struct ( Iff & iff, T * & array, Tag tag )
{
	iff.enterChunk(tag);
	{
		int count = iff.read_int32();

		if(array == nullptr) array = new T();

		array->resize(count);

		for(int i = 0; i < count; i++)
		{
			// bleh. hacky.

			uint8 * bytes = reinterpret_cast<uint8*>(&array->at(i));

			iff.read_uint8( sizeof(typename T::value_type), bytes );
		}
	}
	iff.exitChunk(tag);
}

// ----------------------------------------------------------------------

template<class T>
void writeArray_Class ( Iff & iff, T * array, Tag tag )
{
	iff.insertChunk(tag);
	{
		int count = array->size();

		iff.insertChunkData(count);

		for(int i = 0; i < count; i++)
		{
			array->at(i).write(iff);
		}
	}
	iff.exitChunk(tag);
}

template<class T, class Writer>
void writeArray_Class ( Iff & iff, T * array, Tag tag, Writer W )
{
	iff.insertChunk(tag);
	{
		int count = array->size();

		iff.insertChunkData(count);

		for(int i = 0; i < count; i++)
		{
			(array->at(i).*W)(iff);
		}
	}
	iff.exitChunk(tag);
}

template<class T>
void writeArray_Struct ( Iff & iff, T * array, Tag tag )
{
	iff.insertChunk(tag);
	{
		int count = array->size();

		iff.insertChunkData(count);

		for(int i = 0; i < count; i++)
		{
			iff.insertChunkData( array->at(i) );
		}
	}
	iff.exitChunk(tag);
}

// ======================================================================

SimplePathGraph::SimplePathGraph ( PathGraphType type )
: PathGraph( type ),
  m_nodes( new SimplePathGraph::NodeList() ),
  m_edges( new SimplePathGraph::EdgeList() ),
  m_edgeCounts( new IndexList() ),
  m_edgeStarts( new IndexList() )
{
#ifdef _DEBUG

	m_debugLines = nullptr;

#endif
}

SimplePathGraph::SimplePathGraph( SimplePathGraph::NodeList * nodes, SimplePathGraph::EdgeList * edges, PathGraphType type )
: PathGraph( type ),
  m_nodes( nodes ),
  m_edges( edges ),
  m_edgeCounts( new IndexList() ),
  m_edgeStarts( new IndexList() )
{
	int nodeCount = nodes->size();

	for(int i = 0; i < nodeCount; i++)
	{
		nodes->at(i).setGraph(this);
		nodes->at(i).setIndex(i);
	}

	buildIndexTables();

#ifdef _DEBUG

	m_debugLines = nullptr;

#endif
}

SimplePathGraph::~SimplePathGraph()
{
	delete m_nodes;
	m_nodes = nullptr;

	delete m_edges;
	m_edges = nullptr;

	delete m_edgeCounts;
	m_edgeCounts = nullptr;

	delete m_edgeStarts;
	m_edgeStarts = nullptr;

#ifdef _DEBUG

	delete m_debugLines;
	m_debugLines = nullptr;

#endif
}

// ----------

void SimplePathGraph::clear ( void )
{
	m_nodes->clear();
	m_edges->clear();
	m_edgeCounts->clear();
	m_edgeStarts->clear();

#ifdef _DEBUG

	if(m_debugLines) m_debugLines->clear();

#endif
}

// ----------------------------------------------------------------------

void SimplePathGraph::buildIndexTables ( void )
{
	std::sort( m_edges->begin(), m_edges->end() );

	m_edgeCounts->clear();
	m_edgeStarts->clear();

	m_edgeCounts->resize(m_nodes->size(),0);
	m_edgeStarts->resize(m_nodes->size(),-1);

	int edgeCount = m_edges->size();

	int i;

	for(i = 0; i < edgeCount; i++)
	{
		int node = m_edges->at(i).getIndexA();

		m_edgeCounts->at(node)++;

		if(m_edgeStarts->at(node) == -1)
		{
			m_edgeStarts->at(node) = i;
		}
	}
}

// ----------------------------------------------------------------------

int SimplePathGraph::getNodeCount ( void ) const
{
	return m_nodes->size();
}

PathNode * SimplePathGraph::getNode ( int nodeIndex )
{
	if (static_cast<unsigned int>(nodeIndex) < m_nodes->size())
		return &m_nodes->at(nodeIndex);

	return nullptr;
}

PathNode const * SimplePathGraph::getNode ( int nodeIndex ) const
{
	if (static_cast<unsigned int>(nodeIndex) < m_nodes->size())
		return &m_nodes->at(nodeIndex);

	return nullptr;
}

// ----------------------------------------------------------------------

int SimplePathGraph::getEdgeCount ( int nodeIndex ) const
{
	return m_edgeCounts->at(nodeIndex);
}

PathEdge * SimplePathGraph::getEdge ( int nodeIndex, int edgeIndex )
{
	return &m_edges->at( m_edgeStarts->at(nodeIndex) + edgeIndex );
}

PathEdge const * SimplePathGraph::getEdge ( int nodeIndex, int edgeIndex ) const
{
	return &m_edges->at( m_edgeStarts->at(nodeIndex) + edgeIndex );
}

// ----------------------------------------------------------------------

void SimplePathGraph::read ( Iff & iff )
{
	clear();

	// ----------

	iff.enterForm(TAG_PGRF);

	switch (iff.getCurrentName())
	{
		case TAG_0000:
			m_version = 0;
			read_0000(iff);
			break;

		case TAG_0001:
			m_version = 1;
			read_0001(iff);
			break;

		default:
			FATAL(true,("SimplePathGraph::read - Invalid version"));
			break;
	}

	iff.exitForm(TAG_PGRF);

	// ----------
	// Looks like the graph type isn't getting saved out. Patch it up at runtime

	if(getNodeCount() > 0)
	{
		PathNode const * node = getNode(0);

		PathNodeType type = node->getType();

		if(type == PNT_CellPortal || type == PNT_CellWaypoint || type == PNT_CellPOI)
		{
			setType(PGT_Cell);
		}
		else if(type == PNT_BuildingEntrance || type == PNT_BuildingCell || type == PNT_BuildingPortal || type == PNT_BuildingCellPart)
		{
			setType(PGT_Building);
		}
		else if(type == PNT_CityBuildingEntrance || type == PNT_CityWaypoint || type == PNT_CityPOI || type == PNT_CityBuilding || type == PNT_CityEntrance)
		{
			setType(PGT_City);
		}
		else
		{
			setType(PGT_None);
		}
	}

	// ----------

	int nodeCount = m_nodes->size();

	for(int i = 0; i < nodeCount; i++)
	{
		m_nodes->at(i).setGraph(this);
	}

	// ----------

#ifdef _DEBUG

	if(ConfigSharedCollision::getBuildDebugData())
	{
		buildDebugData();
	}

#endif
}

// ----------------------------------------------------------------------
// Version 4 floor meshes stored path information in an array of floor
// locators and an array of int pairs.

void SimplePathGraph::read_old ( Iff & iff )
{
	clear();

	// ----------

	iff.enterChunk(TAG_PNOD);
	{
		FloorLocator loc;

		while(iff.getChunkLengthLeft())
		{
			loc.read_0000(iff);

			m_nodes->push_back( PathNode(loc.getPosition_p()) );
		}
	}
	iff.exitChunk(TAG_PNOD);

	iff.enterChunk(TAG_PEDG);
	{
		while(iff.getChunkLengthLeft())
		{
			int a = iff.read_int32();
			int b = iff.read_int32();

			m_edges->push_back( PathEdge(a,b) );
			m_edges->push_back( PathEdge(b,a) );
		}
	}
	iff.exitChunk(TAG_PEDG);

	// ----------

	buildIndexTables();

#ifdef _DEBUG

	if(ConfigSharedCollision::getBuildDebugData())
	{
		buildDebugData();
	}

#endif
}

// ----------------------------------------------------------------------

void SimplePathGraph::read_0000 ( Iff & iff )
{
	iff.enterForm(TAG_0000);

	// ----------

	readArray_Class( iff, m_nodes, TAG_PNOD, &PathNode::read_0000 );

	readArray_Class( iff, m_edges, TAG_PEDG, &PathEdge::read_0000 );

	readArray_Struct( iff, m_edgeCounts, TAG_ECNT );

	readArray_Struct( iff, m_edgeStarts, TAG_ESTR );

	// ----------

	iff.exitForm(TAG_0000);
}

// ----------

void SimplePathGraph::read_0001 ( Iff & iff )
{
	iff.enterForm(TAG_0001);

	// ----------

	iff.enterChunk(TAG_META);
	{
		setType( static_cast<PathGraphType>(iff.read_int32()) );
	}
	iff.exitChunk(TAG_META);

	readArray_Class( iff, m_nodes, TAG_PNOD, &PathNode::read_0000 );

	readArray_Class( iff, m_edges, TAG_PEDG, &PathEdge::read_0000 );

	readArray_Struct( iff, m_edgeCounts, TAG_ECNT );

	readArray_Struct( iff, m_edgeStarts, TAG_ESTR );

	// ----------

	iff.exitForm(TAG_0001);
}

// ----------

void SimplePathGraph::write ( Iff & iff ) const
{
	iff.insertForm(TAG_PGRF);

	iff.insertForm(TAG_0001);

	// ----------

	iff.insertChunk(TAG_META);
	{
		iff.insertChunkData( static_cast<int>(getType()) );
	}
	iff.exitChunk(TAG_META);

	writeArray_Class( iff, m_nodes, TAG_PNOD );
	
	writeArray_Class( iff, m_edges, TAG_PEDG );

	writeArray_Struct( iff, m_edgeCounts, TAG_ECNT );

	writeArray_Struct( iff, m_edgeStarts, TAG_ESTR );

	// ----------

	iff.exitForm(TAG_0001);

	iff.exitForm(TAG_PGRF);
}

// ----------------------------------------------------------------------

void SimplePathGraph::drawDebugShapes ( DebugShapeRenderer * renderer ) const
{
	UNREF(renderer);

#ifdef _DEBUG

	if(renderer == nullptr) return;

	if( m_debugLines ) renderer->drawLineList( *m_debugLines, VectorArgb::solidYellow );

	int nodeCount = getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		PathNode const * node = getNode(i);

//		if(node->isPortalAdjacent())
//		{
//			renderer->setColor( VectorArgb::solidMagenta );
//		}
//		else
		{
			renderer->setColor( VectorArgb::solidCyan );
		}

		Vector center = node->getPosition_p() + Vector(0.0f,0.5f,0.0f);

		renderer->drawSphere( Sphere(center,0.2f) );
	}

#endif
}

// ----------------------------------------------------------------------

SimplePathGraph::NodeList const & SimplePathGraph::getNodes ( void ) const
{
	return *m_nodes;
}

// ----------

SimplePathGraph::EdgeList const & SimplePathGraph::getEdges ( void ) const
{
	return *m_edges;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void SimplePathGraph::buildDebugData ( void )
{
	m_debugLines = new VectorList();

	int edgeCount = m_edges->size();

	m_debugLines->reserve(edgeCount*2);

	for(int i = 0; i < edgeCount; i++)
	{
		PathEdge const & edge = m_edges->at(i);

		int A = edge.getIndexA();
		int B = edge.getIndexB();

		Vector a = getNode(A)->getPosition_p();
		Vector b = getNode(B)->getPosition_p();

		m_debugLines->push_back( a + Vector(0.0f,0.7f,0.0f) );
		m_debugLines->push_back( b + Vector(0.0f,0.3f,0.0f) );
	}
}

#endif

