// ======================================================================
//
// IndexedTriangleList.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/IndexedTriangleList.h"

#include "sharedFile/Iff.h"
#include "sharedMath/Plane.h"
#include "sharedMath/Vector.h"

#include <vector>
#include <limits>

// ======================================================================

const Tag TAG_IDTL = TAG(I,D,T,L);
const Tag TAG_VERT = TAG(V,E,R,T);
const Tag TAG_INDX = TAG(I,N,D,X);

// ======================================================================

IndexedTriangleList::IndexedTriangleList() :
	m_mergeVertices(false),
	m_epsilon(0.0f),
	m_vertices(new std::vector<Vector>),
	m_indices(new std::vector<int>)
{
}

// ----------------------------------------------------------------------

IndexedTriangleList::IndexedTriangleList(Iff & iff) :
	m_mergeVertices(false),
	m_epsilon(0.0f),
	m_vertices(new std::vector<Vector>),
	m_indices(new std::vector<int>)
{
	load(iff);
}

// ----------------------------------------------------------------------

IndexedTriangleList::~IndexedTriangleList()
{
	delete m_vertices;
	delete m_indices;
}

// ----------------------------------------------------------------------

const std::vector<Vector> &IndexedTriangleList::getVertices() const
{
	return *m_vertices;
}

// ----------

const std::vector<int> &IndexedTriangleList::getIndices() const
{
	return *m_indices;
}

// ----------------------------------------------------------------------

std::vector<Vector> &IndexedTriangleList::getVertices()
{
	return *m_vertices;
}

// ----------

std::vector<int> &IndexedTriangleList::getIndices()
{
	return *m_indices;
}

// ----------------------------------------------------------------------

void IndexedTriangleList::load(Iff &iff)
{
	clear();

	iff.enterForm(TAG_IDTL);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
			{
				char buffer[512];
				iff.formatLocation(buffer, sizeof(buffer));
				DEBUG_FATAL(true, ("Unknown version number %s", buffer));
			}
		}

	iff.exitForm(TAG_IDTL);
}

// ----------------------------------------------------------------------

void IndexedTriangleList::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_VERT);
		{
			const uint numberOfVertices = static_cast<uint>( iff.getChunkLengthLeft(3 * sizeof(float)) );
			DEBUG_FATAL(!numberOfVertices, ("No vertices"));
			m_vertices->resize(numberOfVertices);
			iff.read_floatVector( static_cast<int>(numberOfVertices), &(*m_vertices)[0]);
		}
		iff.exitChunk(TAG_VERT);

		iff.enterChunk(TAG_INDX);
		{
			const uint numberOfIndices = static_cast<uint>( iff.getChunkLengthLeft(sizeof(int32)) );
			DEBUG_FATAL(!numberOfIndices, ("No indices"));
			m_indices->resize(numberOfIndices);
			for (uint i = 0; i < numberOfIndices; ++i)
				(*m_indices)[i] = iff.read_int32();
		}
		iff.exitChunk(TAG_INDX);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void IndexedTriangleList::write(Iff &iff) const
{
	iff.insertForm(TAG_IDTL);
		iff.insertForm(TAG_0000);

			iff.insertChunk(TAG_VERT);
			{
				const uint numberOfVertices = m_vertices->size();
				for (uint i = 0; i < numberOfVertices; ++i)
					iff.insertChunkFloatVector((*m_vertices)[i]);
			}
			iff.exitChunk(TAG_VERT);

			iff.insertChunk(TAG_INDX);
			{
				const uint numberOfIndices = m_indices->size();
				for (uint i = 0; i < numberOfIndices; ++i)
					iff.insertChunkData(static_cast<int32>((*m_indices)[i]));
			}
			iff.exitChunk(TAG_INDX);

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_IDTL);
}

// ----------------------------------------------------------------------

void IndexedTriangleList::clear()
{
	m_vertices->clear();
	m_indices->clear();
}

// ----------------------------------------------------------------------

void IndexedTriangleList::addVertices(const Vector *vertices, int numberOfVertices, std::vector<int> &indices)
{
	indices.reserve( static_cast<uint>(numberOfVertices) );

	if (m_mergeVertices)
	{
		for (int i = 0; i < numberOfVertices; ++i)
		{
			// look for a matching vertex
			uint j = 0;
			for ( ; j < m_vertices->size() && vertices[i] != (*m_vertices)[j] && vertices[i].magnitudeBetween((*m_vertices)[j]) > m_epsilon; ++j)
				{}

			if (j >= m_vertices->size())
			{
				indices.push_back(static_cast<int>(m_vertices->size()));
				m_vertices->push_back(vertices[i]);
			}
			else
				indices.push_back( static_cast<int>(j) );
		}
	}
	else
	{
		for (int i = 0; i < numberOfVertices; ++i)
		{
			indices.push_back(static_cast<int>(m_vertices->size()));
			m_vertices->push_back(vertices[i]);
		}
	}
}

// ----------------------------------------------------------------------

void IndexedTriangleList::addTriangleList(const Vector *vertices, int numberOfVertices)
{
	DEBUG_FATAL(numberOfVertices < 3 || numberOfVertices % 3 != 0, ("Invalid number of vertices for a triangle list %d", numberOfVertices));

	std::vector<int> vertexIndices;
	addVertices(vertices, numberOfVertices, vertexIndices);

	for (uint i = 0; i < static_cast<uint>(numberOfVertices); ++i)
		m_indices->push_back(vertexIndices[i]);
}

// ----------------------------------------------------------------------

void IndexedTriangleList::addTriangleStrip(const Vector *vertices, int numberOfVertices)
{
	DEBUG_FATAL(numberOfVertices < 3, ("Invalid number of vertices for a triangle strip %d", numberOfVertices));

	std::vector<int> vertexIndices;
	addVertices(vertices, numberOfVertices, vertexIndices);

	const uint triangleCount = static_cast<uint>(numberOfVertices) - 2;
	for (uint i = 0; i < triangleCount; ++i)
		if (i & 1)
		{
			m_indices->push_back(vertexIndices[i+0]);
			m_indices->push_back(vertexIndices[i+2]);
			m_indices->push_back(vertexIndices[i+1]);
		}
		else
		{
			m_indices->push_back(vertexIndices[i+0]);
			m_indices->push_back(vertexIndices[i+1]);
			m_indices->push_back(vertexIndices[i+2]);
		}
}

// ----------------------------------------------------------------------

void IndexedTriangleList::addTriangleFan(const Vector *vertices, int numberOfVertices)
{
	DEBUG_FATAL(numberOfVertices < 3, ("Invalid number of vertices for a triangle fan %d", numberOfVertices));

	std::vector<int> vertexIndices;
	addVertices(vertices, numberOfVertices, vertexIndices);

	const uint triangleCount = static_cast<uint>(numberOfVertices) - 2;
	for (uint i = 0; i < triangleCount; ++i)
	{
		m_indices->push_back(vertexIndices[0]);
		m_indices->push_back(vertexIndices[i+1]);
		m_indices->push_back(vertexIndices[i+2]);
	}
}

// ----------------------------------------------------------------------

void IndexedTriangleList::addIndexedTriangleList(const Vector *vertices, int numberOfVertices, const int *indices, int numberOfIndices)
{
	DEBUG_FATAL(numberOfVertices < 3, ("Invalid number of vertices for an indexed triangle list %d", numberOfVertices));
	DEBUG_FATAL(numberOfIndices < 3 || numberOfIndices % 3 != 0, ("Invalid number of indices for an indexed triangle list %d", numberOfIndices));

	std::vector<int> vertexIndices;
	addVertices(vertices, numberOfVertices, vertexIndices);

	for (uint i = 0; i < static_cast<uint>(numberOfIndices); ++i)
    {
        uint index = static_cast<uint>(indices[i]);

		m_indices->push_back(vertexIndices[index]);
    }
}

// ----------------------------------------------------------------------

void IndexedTriangleList::addIndexedTriangleStrip(const Vector *vertices, int numberOfVertices, const int *indices, int numberOfIndices)
{
	DEBUG_FATAL(numberOfVertices < 3, ("Invalid number of vertices for an indexed triangle list %d", numberOfVertices));
	DEBUG_FATAL(numberOfIndices < 3, ("Invalid number of indices for an indexed triangle strip %d", numberOfIndices));

	std::vector<int> vertexIndices;
	addVertices(vertices, numberOfVertices, vertexIndices);

	const uint triangleCount = static_cast<uint>(numberOfIndices) - 2;
	for (uint i = 0; i < triangleCount; ++i)
	{
		if (i & 1)
		{
			m_indices->push_back(vertexIndices[ static_cast<uint>(indices[i+0]) ]);
			m_indices->push_back(vertexIndices[ static_cast<uint>(indices[i+2]) ]);
			m_indices->push_back(vertexIndices[ static_cast<uint>(indices[i+1]) ]);
		}
		else
		{
			m_indices->push_back(vertexIndices[ static_cast<uint>(indices[i+0]) ]);
			m_indices->push_back(vertexIndices[ static_cast<uint>(indices[i+1]) ]);
			m_indices->push_back(vertexIndices[ static_cast<uint>(indices[i+2]) ]);
		}
	}
}

// ----------------------------------------------------------------------

void IndexedTriangleList::addIndexedTriangleFan(const Vector *vertices, int numberOfVertices, const int *indices, int numberOfIndices)
{
	DEBUG_FATAL(numberOfVertices < 3, ("Invalid number of vertices for an indexed triangle list %d", numberOfVertices));
	DEBUG_FATAL(numberOfIndices < 3, ("Invalid number of indices for an indexed triangle fan %d", numberOfIndices));

	std::vector<int> vertexIndices;
	addVertices(vertices, numberOfVertices, vertexIndices);

	const uint triangleCount = static_cast<uint>(numberOfIndices) - 2;
	for (uint i = 0; i < triangleCount; ++i)
	{
		m_indices->push_back(vertexIndices[ static_cast<uint>(indices[0])]);
		m_indices->push_back(vertexIndices[ static_cast<uint>(indices[i+1])]);
		m_indices->push_back(vertexIndices[ static_cast<uint>(indices[i+2])]);
	}
}

// ----------------------------------------------------------------------

IndexedTriangleList * IndexedTriangleList::clone() const
{
	IndexedTriangleList * const indexedTriangleList = new IndexedTriangleList();
	indexedTriangleList->copy(this);
	return indexedTriangleList;
}

// ----------------------------------------------------------------------

void IndexedTriangleList::copy(IndexedTriangleList const * const indexedTriangleList)
{
	m_mergeVertices = indexedTriangleList->m_mergeVertices;
	m_epsilon = indexedTriangleList->m_epsilon;
	*m_vertices = *indexedTriangleList->m_vertices;
	*m_indices = *indexedTriangleList->m_indices;
}

// ----------------------------------------------------------------------

bool IndexedTriangleList::collide(Vector const & start, Vector const & end, Vector & result) const
{
	return collide(start, end, *m_indices, result);
}

// ----------------------------------------------------------------------

bool IndexedTriangleList::collide(Vector const & start, Vector const & end, std::vector<int> const & indices, Vector & result) const
{
	bool found = false;

	Vector shortenedEnd(end);
	Vector const direction(end - start);

	Vector normal;
	Plane plane;
	Vector intersection;

	std::vector<Vector> const & vertices = *m_vertices;

	uint const numberOfIndices = indices.size();
	
	for (int index = 0; static_cast<int>(numberOfIndices - index) >= 3; /*increment in body*/)
	{
		Vector const & v0 = vertices[indices[index++]];
		Vector const & v1 = vertices[indices[index++]];
		Vector const & v2 = vertices[indices[index++]];

		//-- Compute normal
		normal = (v0 - v2).cross(v1 - v0);

		//-- Ignore backfaces. (Use float min to prevent precision errors.)
		if (direction.dot(normal) < std::numeric_limits<float>::min())
		{
			//-- It doesn't matter that the normal is not normalized
			plane.set(normal, v0);

			//-- See if the end points intersect the plane the polygon lies on, lies within the polygon, and is closer to start than the previous point
			if ((plane.findDirectedIntersection(start, shortenedEnd, intersection)) &&
				(start.magnitudeBetweenSquared(intersection) < start.magnitudeBetweenSquared(shortenedEnd)) &&
				(intersection.inPolygon(v0, v1, v2)))
			{
				found = true;
				result = intersection;
				shortenedEnd = intersection;
			}
		}
	}

	return found;
}

// ======================================================================
