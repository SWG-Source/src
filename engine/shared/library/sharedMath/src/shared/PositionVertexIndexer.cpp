// ======================================================================
//
// PositionVertexIndexer.cpp
// copyright 2002, Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/PositionVertexIndexer.h"

#include "sharedFoundation/Crc.h"

#include <algorithm>
#include <limits>
#include <vector>

// ======================================================================

namespace PositionVertexIndexerNamespace
{
	size_t const s_bucketSize = 149;
}

using namespace PositionVertexIndexerNamespace;

// ======================================================================

PositionVertexIndexer::PositionVertexIndexer() :
m_vertices(new VectorVector),
m_indexMap(new VertexIndexMap(s_bucketSize))
{
}

// ----------------------------------------------------------------------

PositionVertexIndexer::~PositionVertexIndexer()
{
	delete m_vertices;
	m_vertices = 0;

	delete m_indexMap;
	m_indexMap = 0;
}

// ----------------------------------------------------------------------

void PositionVertexIndexer::reserve(int const numberOfVertices)
{
	DEBUG_FATAL(numberOfVertices < 0, ("PositionVertexIndexer::reserve: numberOfVertices < 0"));
	m_vertices->reserve(static_cast<size_t>(numberOfVertices));
}

// ----------------------------------------------------------------------

int PositionVertexIndexer::addVertex(Vector const & vertex)
{
	uint32 const key = Crc::calculate(&vertex,sizeof(vertex));


	std::pair<VertexIndexMap::iterator, VertexIndexMap::iterator> collisions = m_indexMap->equal_range(key);

	bool insertVertex = true;
	int index = 0;

	if (collisions.first != m_indexMap->end())
	{
		// set insertVertex to false.
		insertVertex = false;
		
		// if so, look for collisions.		
		for (; collisions.first != collisions.second; ++collisions.first) 
		{
			index = collisions.first->second;
			Vector const & existingVertex = (*m_vertices)[static_cast<size_t>(index)];

			// if we find a collision, insert the vertex instead of returning the existing index.
			if (vertex != existingVertex)
			{
				insertVertex = true;
				break;
			}
		}
	}

	if (insertVertex) 
	{
		// add a unique vertex to the map.
		// get current index.
		index = static_cast<int>(m_vertices->size());

		// add to list.
		m_vertices->push_back(vertex);

		// insert into map.
		IGNORE_RETURN(m_indexMap->insert(std::make_pair(key, index)));
	}

	return index;
}

// ----------------------------------------------------------------------

int PositionVertexIndexer::getNumberOfVertices() const
{
	return static_cast<int>(m_vertices->size());
}

// ----------------------------------------------------------------------

void PositionVertexIndexer::clear()
{
	m_vertices->clear();
	m_indexMap->clear();
}

// ----------------------------------------------------------------------

Vector const & PositionVertexIndexer::getVertex(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfVertices());
	return (*m_vertices)[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

Vector & PositionVertexIndexer::getVertex(int const index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfVertices());
	return (*m_vertices)[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------

PositionVertexIndexer::VectorVector const & PositionVertexIndexer::getVertices() const
{
	return *m_vertices;
}

// ----------------------------------------------------------------------

PositionVertexIndexer::VectorVector & PositionVertexIndexer::getVertices()
{
	return *m_vertices;
}

// ======================================================================
