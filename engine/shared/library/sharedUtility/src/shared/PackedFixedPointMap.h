// ======================================================================
//
// PackedFixedPointMap.h
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PackedFixedPointMap_H
#define INCLUDED_PackedFixedPointMap_H

// ======================================================================

class Iff;

#include "sharedUtility/PackedIntegerMap.h"

// ======================================================================

class PackedFixedPointMap
{
public:

	PackedFixedPointMap(int width, int height, float resolution, const float *values);
	PackedFixedPointMap(Iff &iff);
	PackedFixedPointMap(const PackedFixedPointMap &copy);
	~PackedFixedPointMap();

	void save(Iff &iff) const;

	int getWidth()  const { return m_integerMap.getWidth(); }
	int getHeight() const { return m_integerMap.getHeight(); }

	float getValue(int x, int y) const;
	void setValue(int x, int y, float value);

protected:

	static int *_convertFloatMap(const float *values, int size, float resolution);
	static Iff &_beginLoad(Iff &iff);

	PackedIntegerMap m_integerMap;
	float m_resolution;
};

// ======================================================================

#endif // INCLUDED_PackedFixedPointMap_H
