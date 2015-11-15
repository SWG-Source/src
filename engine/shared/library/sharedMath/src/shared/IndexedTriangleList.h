// ======================================================================
//
// IndexedTriangleList.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_IndexedTriangleList_H
#define INCLUDED_IndexedTriangleList_H

// ======================================================================

class Iff;
class Vector;

// ======================================================================

class IndexedTriangleList
{
public:

	IndexedTriangleList();
	explicit IndexedTriangleList(Iff &iff);
	~IndexedTriangleList();

	const stdvector<Vector>::fwd &getVertices() const;
	const stdvector<int>::fwd    &getIndices() const;

	stdvector<Vector>::fwd &getVertices();
	stdvector<int>::fwd    &getIndices();

	void load(Iff &iff);
	void write(Iff &iff) const;

	void allowVertexMerging(bool mergeVertices);
	void setVertexMergeEpsilon(float epsilon);

	void clear();
	void addTriangleList(const Vector *vertices, int numberOfVertices);
	void addTriangleStrip(const Vector *vertices, int numberOfVertices);
	void addTriangleFan(const Vector *vertices, int numberOfVertices);
	void addIndexedTriangleList(const Vector *vertices, int numberOfVertices, const int *indices, int numberOfIndices);
	void addIndexedTriangleStrip(const Vector *vertices, int numberOfVertices, const int *indices, int numberOfIndices);
	void addIndexedTriangleFan(const Vector *vertices, int numberOfVertices, const int *indices, int numberOfIndices);
	
	IndexedTriangleList* clone() const;
	void copy(const IndexedTriangleList *tris);
	
	bool collide(Vector const & start, Vector const & end, Vector & result) const;
	bool collide(Vector const & start, Vector const & end, stdvector<int>::fwd const & indices, Vector & result) const;

private:

	// disabled
	IndexedTriangleList(const IndexedTriangleList &);
	IndexedTriangleList &operator =(const IndexedTriangleList &);

private:

	void load_0000(Iff &iff);

	void addVertices(const Vector *vertices, int numberOfVertices, stdvector<int>::fwd &indices);

private:

	bool                    m_mergeVertices;
	float                   m_epsilon;
	stdvector<Vector>::fwd * const m_vertices;
	stdvector<int>::fwd * const m_indices;
};

// ======================================================================

inline void IndexedTriangleList::allowVertexMerging(bool mergeVertices)
{
	m_mergeVertices = mergeVertices;
}

// ----------------------------------------------------------------------

inline void IndexedTriangleList::setVertexMergeEpsilon(float epsilon)
{
	m_epsilon = epsilon;
}

// ======================================================================

#endif
