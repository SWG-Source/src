// ======================================================================
//
// CollisionBuckets.cpp
// copyright (c) 2001 Sony Online Entertainment
// tford
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/CollisionBuckets.h"

#include "sharedCollision/Bresenham.h"
#include "sharedCollision/FloorTri.h"	// for IndexedTri
#include "sharedCollision/Intersect2d.h"
#include "sharedCollision/Intersect3d.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Ray3d.h"

namespace CollisionBucketsNamespace
{
	float const cs_maximumNodeSpan = 100.0f;
	float const cs_minimumNodeSpan = 10.0f;
	unsigned int const cs_framesToRenderEachBucket = 50;
	bool s_renderTravelingNode = true;
	bool s_renderNodesAroundPath = true;

	// ----------------------------------------------------------------------

	class NodeChooser : public Bresenham::PointSetter3d
	{
	public:

		typedef std::vector<Bresenham::Point3D> PointVector;

		explicit NodeChooser(Bresenham::Point3D const & maximumDimension);
		PointVector const & getResultPointVector() const;

	private:

		NodeChooser();
		NodeChooser(NodeChooser const & copy);
		NodeChooser & operator=(NodeChooser const & copy);

		virtual void realSet(Bresenham::Point3D const & point);

	private:

		Bresenham::Point3D const m_maximumDimension;
		PointVector m_results;
	};
}

// ======================================================================

CollisionBucketsNamespace::NodeChooser::NodeChooser(Bresenham::Point3D const & maximumDimension)
: PointSetter3d()
, m_maximumDimension(maximumDimension)
, m_results()
{ }

// ----------------------------------------------------------------------

CollisionBucketsNamespace::NodeChooser::PointVector const & CollisionBucketsNamespace::NodeChooser::getResultPointVector() const
{
	return m_results;
}

// ----------------------------------------------------------------------

void CollisionBucketsNamespace::NodeChooser::realSet(Bresenham::Point3D const & point)
{
	if (point.x >= m_maximumDimension.x) return;
	if (point.y >= m_maximumDimension.y) return;
	if (point.z >= m_maximumDimension.z) return;
	if (point.x < 0) return;
	if (point.y < 0) return;
	if (point.z < 0) return;
	m_results.push_back(point);
}

// ======================================================================

using namespace CollisionBucketsNamespace;

// ======================================================================

CollisionBuckets::Node::Node(AxialBox const & bounds)
: m_indices()
, m_bounds(bounds)
{
}

// ======================================================================

CollisionBuckets::CollisionBuckets(IndexedTriangleList * const indexedTriangleList)
: m_indexedTriangleList(indexedTriangleList)
, m_bounds()
, m_spanOfBounds()
, m_sizeOfNode()
, m_bucketsAlongX(0)
, m_bucketsAlongY(0)
, m_bucketsAlongZ(0)
, m_nodeMatrix(0)
#ifdef _DEBUG
, m_renderXX(0)
, m_renderYY(0)
, m_renderZZ(0)
, m_timesRendered(0)
, m_lastTestedNodes()
, m_hitTriangle()
#endif
{
}

// ----------------------------------------------------------------------

CollisionBuckets::~CollisionBuckets()
{
	destroy();
	// destroy will delete m_nodeMatrix
	m_nodeMatrix = 0;
}

// ----------------------------------------------------------------------

void CollisionBuckets::build(Vector const & minimumBounds, Vector const & maximumBounds)
{
	if (m_nodeMatrix != 0)
	{
		return;
	}

	typedef std::vector<Vector> Vertices;
	typedef std::vector<int> Indices;
	Vertices const & vertices = m_indexedTriangleList->getVertices();
	Indices const & indices = m_indexedTriangleList->getIndices();

	m_bounds.setMin(minimumBounds);
	m_bounds.setMax(maximumBounds);
	m_spanOfBounds = m_bounds.getMax() - m_bounds.getMin();

	m_bucketsAlongX = ((m_spanOfBounds.x > cs_maximumNodeSpan)
						? static_cast<unsigned int>(m_spanOfBounds.x / cs_maximumNodeSpan)
						: static_cast<unsigned int>(m_spanOfBounds.x / cs_minimumNodeSpan)) + 1;
	m_bucketsAlongY = ((m_spanOfBounds.y > cs_maximumNodeSpan)
						? static_cast<unsigned int>(m_spanOfBounds.y / cs_maximumNodeSpan)
						: static_cast<unsigned int>(m_spanOfBounds.y / cs_minimumNodeSpan)) + 1;
	m_bucketsAlongZ = ((m_spanOfBounds.z > cs_maximumNodeSpan)
						? static_cast<unsigned int>(m_spanOfBounds.z / cs_maximumNodeSpan)
						: static_cast<unsigned int>(m_spanOfBounds.z / cs_minimumNodeSpan)) + 1;

	m_sizeOfNode.x = m_spanOfBounds.x / static_cast<float>(m_bucketsAlongX);
	m_sizeOfNode.y = m_spanOfBounds.y / static_cast<float>(m_bucketsAlongY);
	m_sizeOfNode.z = m_spanOfBounds.z / static_cast<float>(m_bucketsAlongZ);

	{	// size the matrix
		m_nodeMatrix = new NodeMatrix;
		(*m_nodeMatrix).resize(m_bucketsAlongX);
		for (unsigned int xx = 0; xx < m_bucketsAlongX; ++xx)
		{
			(*m_nodeMatrix)[xx].resize(m_bucketsAlongY);
			for (unsigned int yy = 0; yy < m_bucketsAlongY; ++yy)
			{
				// note that nodes are initialized to nullptr
				(*m_nodeMatrix)[xx][yy].resize(m_bucketsAlongZ, 0);
			}
		}
	}

	unsigned int numberOfFaces = indices.size() / 3;
	for (unsigned int i = 0, index = 0; i < numberOfFaces; ++i)
	{
		int const index1 = indices[index++];
		int const index2 = indices[index++];
		int const index3 = indices[index++];

		Vector const & point1 = vertices[static_cast<unsigned int>(index1)];
		Vector const & point2 = vertices[static_cast<unsigned int>(index2)];
		Vector const & point3 = vertices[static_cast<unsigned int>(index3)];

		for (unsigned int xx = 0; xx < m_bucketsAlongX; ++xx)
		{
			for (unsigned int yy = 0; yy < m_bucketsAlongY; ++yy)
			{
				for (unsigned int zz = 0; zz < m_bucketsAlongZ; ++zz)
				{
					Vector const minimumNodeOffset(m_sizeOfNode.x * xx, m_sizeOfNode.y * yy, m_sizeOfNode.z * zz);
					Vector const minimumNodeBounds(minimumNodeOffset + minimumBounds);
					Vector const maximumNodeBounds(minimumNodeBounds + m_sizeOfNode);

					AxialBox const bounds(minimumNodeBounds, maximumNodeBounds);

					if (Intersect3d::intersectTriangleBox(bounds, point1, point2, point3))
					{
						if ((*m_nodeMatrix)[xx][yy][zz] == 0)
						{
							(*m_nodeMatrix)[xx][yy][zz] = new Node(bounds);
						}

						Node * const node = (*m_nodeMatrix)[xx][yy][zz];
						node->m_indices.push_back(index1);
						node->m_indices.push_back(index2);
						node->m_indices.push_back(index3);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void CollisionBuckets::destroy()
{
	if (m_nodeMatrix != 0)
	{
		unsigned int const bucketsPerDimensionX = (*m_nodeMatrix).size();
		for (unsigned int xx = 0; xx < bucketsPerDimensionX; ++xx)
		{
			unsigned int const bucketsPerDimensionY = (*m_nodeMatrix)[xx].size();
			for (unsigned int yy = 0; yy < bucketsPerDimensionY; ++yy)
			{
				unsigned int const bucketsPerDimensionZ = (*m_nodeMatrix)[xx][yy].size();
				for (unsigned int zz = 0; zz < bucketsPerDimensionZ; ++zz)
				{
					delete (*m_nodeMatrix)[xx][yy][zz];
				}
			}
		}
		delete m_nodeMatrix;
		m_nodeMatrix = 0;
	}
}

// ----------------------------------------------------------------------

bool CollisionBuckets::intersect(Vector const & begin, Vector const & end, Vector * surfaceNormal, float * time) const
{
	Vector direction(end - begin);
	float const magnitude = direction.magnitude();

	bool hit = false;

	if (direction.normalize())
	{
		Ray3d const ray(begin, direction);
		Intersect3d::ResultData data;
		if ((m_bounds.contains(begin)) || (Intersect3d::intersectRayABoxWithData(ray, m_bounds, &data)))
		{
			float const lengthToFront = data.m_length;
			data.m_length = 0.0f;

			// the comment below explains the reason for doing the reverse test
			if ((m_bounds.contains(end)) || (Intersect3d::intersectRayABoxWithData(Ray3d(end, -direction), m_bounds, &data)))
			{
				float const lengthToBack = data.m_length;
				BucketNodeList nodesToTest;

				{
					// closeBegin and closeEnd need to be calculated for
					// extreme distances (~1000 units) to help keep the
					// Bresenham traversal from losing precision
					Vector const closeBegin(ray.atParam(lengthToFront));
					Vector const closeEnd(ray.atParam(magnitude - lengthToBack));
					findCandidateBucketsToTestClosestToFarthest(closeBegin, closeEnd, nodesToTest);
				}

#ifdef _DEBUG
				m_lastTestedNodes = nodesToTest;
				m_hitTriangle = Triangle3d();
#endif

				if (!nodesToTest.empty())
				{
					float bestTime = magnitude;
					Triangle3d bestTriangle;;

					typedef std::vector<Vector> Vertices;
					Vertices const & vertices = m_indexedTriangleList->getVertices();

					BucketNodeList::const_iterator ii = nodesToTest.begin();
					BucketNodeList::const_iterator iiEnd = nodesToTest.end();
					for (bool done = false; ((ii != iiEnd) && (!done)); ++ii)
					{
						Node const * const node = *ii;

						// pretest each bucket for early rejection
						if ((node != 0) && (Intersect3d::intersectRayABoxWithData(ray, node->m_bounds, &data)))
						{
							// no point testing beyond best time if the remaining
							// boxes are past this also
							done = (data.m_length > bestTime);
							if (!done)
							{
								Node::Indices const & indices = node->m_indices;

								unsigned int const numberOfFaces = indices.size() / 3;
								for (unsigned int i = 0, index = 0; i < numberOfFaces; ++i)
								{
									int const index1 = indices[index++];
									int const index2 = indices[index++];
									int const index3 = indices[index++];

									Vector const & point1 = vertices[static_cast<unsigned int>(index1)];
									Vector const & point2 = vertices[static_cast<unsigned int>(index2)];
									Vector const & point3 = vertices[static_cast<unsigned int>(index3)];

									Triangle3d const triangle(point1, point2, point3);
									Vector hitPoint;
									real t;

									if (Intersect3d::IntersectRayTriSided(ray, triangle, hitPoint, t))
									{
										if (t < bestTime)
										{
											bestTriangle = triangle;
#ifdef _DEBUG
											m_hitTriangle = bestTriangle;
#endif
											bestTime = t;
											hit = true;
										}
									}
								}
							}
						}
					}

					if (hit)
					{
						if (surfaceNormal != 0)
						{
							*surfaceNormal = bestTriangle.getNormal();
						}

						if (time != 0)
						{
							*time = bestTime / magnitude;
						}
					}
				}
			}
		}
	}
	return hit;
}

// ----------------------------------------------------------------------

void CollisionBuckets::findCandidateBucketsToTestClosestToFarthest(Vector const & begin, Vector const & end, BucketNodeList & bucketNodeList) const
{
	if (m_nodeMatrix != 0)
	{
		int const beginIndexX = static_cast<int>((begin.x - m_bounds.getMin().x) / m_sizeOfNode.x);
		int const beginIndexY = static_cast<int>((begin.y - m_bounds.getMin().y) / m_sizeOfNode.y);
		int const beginIndexZ = static_cast<int>((begin.z - m_bounds.getMin().z) / m_sizeOfNode.z);

		int const endIndexX = static_cast<int>((end.x - m_bounds.getMin().x) / m_sizeOfNode.x);
		int const endIndexY = static_cast<int>((end.y - m_bounds.getMin().y) / m_sizeOfNode.y);
		int const endIndexZ = static_cast<int>((end.z - m_bounds.getMin().z) / m_sizeOfNode.z);

		Bresenham::Point3D const beginIndex(beginIndexX, beginIndexY, beginIndexZ);
		Bresenham::Point3D const endIndex(endIndexX, endIndexY, endIndexZ);

		NodeChooser howToPick(Bresenham::Point3D(static_cast<int>(m_bucketsAlongX), static_cast<int>(m_bucketsAlongY), static_cast<int>(m_bucketsAlongZ)));

		Bresenham::FindPointsAlong(beginIndex, endIndex, true, howToPick);

		bucketNodeList.reserve(howToPick.getResultPointVector().size());

		NodeChooser::PointVector::const_iterator ii = howToPick.getResultPointVector().begin();
		NodeChooser::PointVector::const_iterator iiEnd = howToPick.getResultPointVector().end();

		for (; ii != iiEnd; ++ii)
		{
			Bresenham::Point3D const & point = *ii;

			unsigned int const xx = static_cast<unsigned int>(point.x);
			unsigned int const yy = static_cast<unsigned int>(point.y);
			unsigned int const zz = static_cast<unsigned int>(point.z);

			Node * node = (*m_nodeMatrix)[xx][yy][zz];
			if (node != 0)
			{
				bucketNodeList.push_back(node);
			}
		}
	}
}

// ----------------------------------------------------------------------

void CollisionBuckets::drawDebugShapes(DebugShapeRenderer * const renderer) const
{
	UNREF(renderer);
#ifdef _DEBUG
	if (renderer != 0)
	{
		typedef std::vector<Vector> Vertices;

		if (s_renderNodesAroundPath)
		{
			renderer->setColor(VectorArgb::solidYellow);

			BucketNodeList::const_iterator ii = m_lastTestedNodes.begin();
			BucketNodeList::const_iterator iiEnd = m_lastTestedNodes.end();

			for (; ii != iiEnd; ++ii)
			{
				Node const * const node = *ii;
				if (node != 0)
				{
					renderer->drawBox(node->m_bounds);
				}
			}

			if (!m_lastTestedNodes.empty())
			{
				Vertices lineList;
				lineList.push_back(m_hitTriangle.getCornerA());
				lineList.push_back(m_hitTriangle.getCornerB());
				lineList.push_back(m_hitTriangle.getCornerB());
				lineList.push_back(m_hitTriangle.getCornerC());
				lineList.push_back(m_hitTriangle.getCornerC());
				lineList.push_back(m_hitTriangle.getCornerA());
				renderer->setColor(VectorArgb::solidRed);
				renderer->drawLineList(lineList);
			}
		}

		if ((s_renderTravelingNode) && (m_nodeMatrix != 0))
		{
			// draw the current selected box
			renderer->setColor(VectorArgb::solidBlack);

			Node const * const node = (*m_nodeMatrix)[m_renderXX][m_renderYY][m_renderZZ];
			if (node != 0)
			{
				renderer->drawBox(node->m_bounds);

				// draw the triangles in the selected box
				renderer->setColor(VectorArgb::solidRed);

				Node::Indices const & indices = node->m_indices;
				Vertices const & vertices = m_indexedTriangleList->getVertices();

				Vertices lineList;
				unsigned int const numberOfFaces = indices.size() / 3;
				for (unsigned int i = 0, index = 0; i < numberOfFaces; ++i)
				{
					int const index1 = indices[index++];
					int const index2 = indices[index++];
					int const index3 = indices[index++];

					Vector const & point1 = vertices[static_cast<unsigned int>(index1)];
					Vector const & point2 = vertices[static_cast<unsigned int>(index2)];
					Vector const & point3 = vertices[static_cast<unsigned int>(index3)];
					lineList.push_back(point1);
					lineList.push_back(point2);
					lineList.push_back(point2);
					lineList.push_back(point3);
					lineList.push_back(point3);
					lineList.push_back(point1);
				}
				renderer->drawLineList(lineList);
			}
			else
			{
				m_timesRendered = cs_framesToRenderEachBucket;
			}

			// select the next box to show
			++m_timesRendered;
			if (m_timesRendered > cs_framesToRenderEachBucket)
			{
				m_timesRendered = 0;
				++m_renderZZ;

				if (m_renderZZ >= m_bucketsAlongZ)
				{
					m_renderZZ = 0;
					++m_renderYY;

					if (m_renderYY >= m_bucketsAlongY)
					{
						m_renderYY = 0;
						++m_renderXX;

						if (m_renderXX >= m_bucketsAlongX)
						{
							m_renderXX = 0;
						}
					}
				}
			}
		}
	}
#endif
}
