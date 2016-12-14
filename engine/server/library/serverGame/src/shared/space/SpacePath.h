// ======================================================================
// 
// SpacePath.h
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#ifndef INCLUDED_SpacePath_H
#define INCLUDED_SpacePath_H

#include "sharedMath/Transform.h"
#include "sharedFoundation/BitArray.h"

// ----------------------------------------------------------------------
class SpacePath
{
public:
	friend class SpacePathManager;

	typedef std::vector<Transform> TransformList;

	void addTransform(Transform const & transform);
	TransformList const & getTransformList() const;
	bool isEmpty() const;
	void clear();
	int getLength() const;

	float getCollisionRadius() const;
	bool updateCollisionRadius(void const * const object, float const objectSize);

private:
	// accessed by SpacePathManager
	bool refine(int & pathRefinementsAvailable);

	void setUpdateFlag(bool update);
	bool getUpdateFlag() const;

private:
	
	TransformList m_transformList;
	
	BitArray m_transformDirty;
	BitArray m_transformOriginal;

	int m_referenceCount;
	bool m_updateFlag;


	float m_collisionRadius;

	typedef std::multimap<void const * /*object*/, float /*size of object */> ObjectSizeMap;
	ObjectSizeMap * const m_collisionRadiusMap;

	void addReference(void const * const object, float const objectSize);
	void releaseReference(void const * const object);
	int getReferenceCount() const;

	void requestPathResize();

	SpacePath(SpacePath const & rhs);
	SpacePath();
	~SpacePath();
	SpacePath & operator=(SpacePath const & rhs);
};

// ======================================================================

#endif // INCLUDED_SpacePath_H
