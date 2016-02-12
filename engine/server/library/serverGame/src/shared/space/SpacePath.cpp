// ======================================================================
// 
// SpacePath.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/SpacePath.h"

#include "serverGame/ConfigServerGame.h"
#include "sharedCollision/SpaceAvoidanceManager.h"
#include "sharedDebug/Profiler.h"
#include "sharedLog/Log.h"

#include <algorithm>
#include <vector>
#include <map>
#include <cstdio>

// ======================================================================
//
// SpacePath
//
// ======================================================================

// ----------------------------------------------------------------------
namespace SpacePathNamespace
{
	// default collision radius.
	float const s_minimumCollisionRadius = 7.0f;

	// maximum number of nodes in the path.
	int const s_maximumPathNodes = 256;
}

// ----------------------------------------------------------------------
using namespace SpacePathNamespace;


// ----------------------------------------------------------------------
SpacePath::SpacePath()
 : m_transformList()
 , m_transformDirty()
 , m_transformOriginal()
 , m_referenceCount(0)
 , m_updateFlag(false)
 , m_collisionRadius(s_minimumCollisionRadius)
 , m_collisionRadiusMap(new ObjectSizeMap)
{
}

// ----------------------------------------------------------------------
SpacePath::~SpacePath()
{
	DEBUG_FATAL(!m_collisionRadiusMap->empty(), ("Object size map has outstanding references!"));
	delete m_collisionRadiusMap;
}

// ----------------------------------------------------------------------
void SpacePath::addTransform(Transform const & transform)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpacePath::addTransform() path(0x%p) pathSize(%u) x(%.2f) y(%.2f) z(%.2f)", this, m_transformList.size() + 1, transform.getPosition_p().x, transform.getPosition_p().y, transform.getPosition_p().z));

	DEBUG_FATAL(m_transformList.size() > static_cast<size_t>(s_maximumPathNodes), ("The transform list exceeds %d transforms.", s_maximumPathNodes));

	m_transformDirty.setBit(static_cast<int>(m_transformList.size()));
	m_transformOriginal.setBit(static_cast<int>(m_transformList.size()));

	m_transformList.push_back(transform);
}
	

// ----------------------------------------------------------------------
void SpacePath::clear()
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpacePath::clearPath() path(0x%p)", this));

	m_transformDirty.clear();
	m_transformOriginal.clear();
	m_transformList.clear();
}

// ----------------------------------------------------------------------
SpacePath::TransformList const & SpacePath::getTransformList() const
{
	return m_transformList;
}

// ----------------------------------------------------------------------
bool SpacePath::isEmpty() const
{
	return m_transformList.empty();
}

// ----------------------------------------------------------------------
void SpacePath::addReference(void const * const object, float const objectSize)
{
	DEBUG_FATAL(object == nullptr, ("Passing a nullptr object into path 0x%p.", this));

	++m_referenceCount;

	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpacePath::addReference() path(0x%p) pathSize(%u) referenceCount(%d) objectSize(%3.2f)", this, m_transformList.size(), m_referenceCount, objectSize));

	// Add the object to the list here.
	IGNORE_RETURN(m_collisionRadiusMap->insert(std::make_pair(object, objectSize)));

	// Resize this path if necessary.
	requestPathResize();
}

// ----------------------------------------------------------------------
void SpacePath::releaseReference(void const * const object)
{
	DEBUG_FATAL(object == nullptr, ("Passing a nullptr object into path 0x%p.", this));

	if (--m_referenceCount < 0)
	{
		DEBUG_FATAL(true, ("Negative reference count, this means there is a reference counting problem."));
	}

	// Remove the object from the list.
	IGNORE_RETURN(m_collisionRadiusMap->erase(object));

	// Resize this path if necessary.
	requestPathResize();

	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpacePath::releaseReference() path(0x%p) pathSize(%u) referenceCount(%d)", this, m_transformList.size(), m_referenceCount));
}

// ----------------------------------------------------------------------
int SpacePath::getReferenceCount() const
{
	return m_referenceCount;
}

// ----------------------------------------------------------------------
bool SpacePath::refine(int & pathRefinementsAvailable)
{
	PROFILER_AUTO_BLOCK_DEFINE("SpacePath::refine");

	bool isComplete = true;

	// Create a scratch xform.
	Transform newTransform;

	float const collisionRadius = getCollisionRadius();
	float const collisionRadiusSquared = sqr(collisionRadius);

	// Keep track of the transform list size.
	int transformListSize = static_cast<int>(m_transformList.size());

	if (!m_transformList.empty() && !m_transformDirty.empty() && transformListSize < s_maximumPathNodes)
	{
		TransformList::iterator iteratorTransform = m_transformList.begin();

		for(; iteratorTransform != m_transformList.end() && (iteratorTransform + 1) != m_transformList.end() && pathRefinementsAvailable > 0 && transformListSize < s_maximumPathNodes; /*++iteratorTransform*/)
		{
			int const currentOffset = iteratorTransform - m_transformList.begin();

			if (m_transformDirty.testBit(currentOffset))
			{
				m_transformDirty.clearBit(currentOffset);

				// Update refinement counters.
				pathRefinementsAvailable--;

				TransformList::iterator iteratorTransformNext = iteratorTransform + 1;
				Transform const & currentTransform = *iteratorTransform;
				Transform const & nextTransform = *iteratorTransformNext;
				Vector const & currentPosition_w = currentTransform.getPosition_p();
				Vector const & nextPosition_w = nextTransform.getPosition_p();
				Vector const sweepVector(nextPosition_w - currentPosition_w);

				if (sweepVector.magnitudeSquared() > collisionRadiusSquared)
				{
					PROFILER_AUTO_BLOCK_DEFINE("SpacePath::refine(){...getAvoidancePosition()...}");

					Vector avoidancePosition_w;
					
					bool const pathAdjusted = SpaceAvoidanceManager::getAvoidancePosition(currentTransform, collisionRadius, sweepVector, nextPosition_w, avoidancePosition_w, nullptr);
					if (pathAdjusted)
					{
						newTransform.setPosition_p(avoidancePosition_w);
						
						iteratorTransform = m_transformList.insert(iteratorTransformNext, newTransform);
						
						int const transformIndex = iteratorTransform - m_transformList.begin();
						m_transformDirty.insertBit(transformIndex, true);
						m_transformOriginal.insertBit(transformIndex, false);

						// Update transform list size.
						++transformListSize;

						// Continue as all iterators are now invalid.
						continue;
					}
				}
				else
				{
					int const transformIndex = iteratorTransform - m_transformList.begin();

					// Do not remove original bits.
					if (m_transformOriginal.testBit(transformIndex)) 
					{
						// The sweep distance is too small, remove the point from the list.
						m_transformDirty.removeBit(transformIndex);
						m_transformOriginal.removeBit(transformIndex);

						iteratorTransform = m_transformList.erase(iteratorTransform);
						
						// Update transform list size.
						--transformListSize;
						
						// Continue as all iterators are now invalid.
						continue;
					}
				}
			}

			++iteratorTransform;
		}

		// See if we completed the scan. 
		isComplete = (iteratorTransform == m_transformList.end() || (iteratorTransform + 1) == m_transformList.end());
	}

	return isComplete;
}


// ----------------------------------------------------------------------
void SpacePath::setUpdateFlag(bool const flag)
{
	m_updateFlag = flag;
}

// ----------------------------------------------------------------------
bool SpacePath::getUpdateFlag() const
{
	return m_updateFlag;
}

// ----------------------------------------------------------------------
int SpacePath::getLength() const
{
	return static_cast<int>(m_transformList.size());
}

// ----------------------------------------------------------------------
float SpacePath::getCollisionRadius() const 
{
	return m_collisionRadius;
}

// ----------------------------------------------------------------------
void SpacePath::requestPathResize()
{
	float maximumObjectSize = s_minimumCollisionRadius;

	for (ObjectSizeMap::iterator itObject = m_collisionRadiusMap->begin(); itObject != m_collisionRadiusMap->end(); ++itObject)
	{
		maximumObjectSize = std::max(maximumObjectSize, itObject->second);
	}

	// Always update if the new max object is larger or if the new max object size is smaller by at least the minimum collision radius.
	if (maximumObjectSize > m_collisionRadius || !WithinEpsilonInclusive(m_collisionRadius, maximumObjectSize, s_minimumCollisionRadius)) 
	{
		// Mark all nodes as dirty to refine paths.
		m_transformDirty.clear();
		m_transformDirty.setMultipleBits(m_transformList.size());

		// Set the max path granularity to the maximum object size.
		m_collisionRadius = maximumObjectSize;
	}
}

// ----------------------------------------------------------------------
bool SpacePath::updateCollisionRadius(void const * const object, float const objectSize)
{
	DEBUG_FATAL(object == nullptr, ("Passing a nullptr object into path(0x%p) updateCollisionRadius.", this));

	bool requiresPathUpdate = false;

	// Update all the object sizes.
	std::pair<ObjectSizeMap::iterator, ObjectSizeMap::iterator> const & collisionRadiiToUpdate = m_collisionRadiusMap->equal_range(object);
	for (ObjectSizeMap::iterator itRadii = collisionRadiiToUpdate.first; itRadii != collisionRadiiToUpdate.second; ++itRadii)
	{
		float const currentSize = itRadii->second;
		if (!WithinEpsilonInclusive(currentSize, objectSize, s_minimumCollisionRadius)) 
		{
			itRadii->second = objectSize;
			requiresPathUpdate = true;
		}
	}
	
	// Update the path refinement if necessary.
	if (requiresPathUpdate) 
	{
		requestPathResize();
	}

	return requiresPathUpdate;
}


// ======================================================================
