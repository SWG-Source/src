// ======================================================================
//
// CollisionBuckets.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CollisionBuckets_H
#define INCLUDED_CollisionBuckets_H

// ======================================================================

#include "sharedCollision/CollisionSurface.h"

#include "sharedMath/AxialBox.h"
#include "sharedMath/Triangle3d.h"

#include <vector>

class DebugShapeRenderer;
class IndexedTriangleList;

// ======================================================================

class CollisionBuckets
{
public:

	explicit CollisionBuckets(IndexedTriangleList * const indexedTriangleList);
	~CollisionBuckets();

	void build(Vector const & minimumBounds, Vector const & maximumBounds);
	void destroy();
	void drawDebugShapes(DebugShapeRenderer * const renderer) const;
	bool intersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, float * time) const;

private:

	CollisionBuckets();
	CollisionBuckets(const CollisionBuckets & source);
	CollisionBuckets & operator=(const CollisionBuckets & source);

	struct Node;
	typedef std::vector<Node *> BucketNodeList;
	void findCandidateBucketsToTestClosestToFarthest(Vector const & begin, Vector const & end, BucketNodeList & bucketNodeList) const;

private:

	IndexedTriangleList const * const m_indexedTriangleList;
	AxialBox m_bounds;
	Vector m_spanOfBounds;
	Vector m_sizeOfNode;

	unsigned int m_bucketsAlongX;
	unsigned int m_bucketsAlongY;
	unsigned int m_bucketsAlongZ;

	struct Node
	{
		Node(AxialBox const & bounds);

		typedef std::vector<int> Indices;
		Indices m_indices;
		AxialBox m_bounds;
	};

	typedef std::vector<Node *> NodeRow;
	typedef std::vector<NodeRow> NodeColumn;
	typedef std::vector<NodeColumn> NodeMatrix;
	NodeMatrix * m_nodeMatrix;

#ifdef _DEBUG
	mutable unsigned int m_renderXX;
	mutable unsigned int m_renderYY;
	mutable unsigned int m_renderZZ;
	mutable unsigned int m_timesRendered;
	mutable BucketNodeList m_lastTestedNodes;
	mutable Triangle3d m_hitTriangle;
#endif

};


// ======================================================================

#endif
