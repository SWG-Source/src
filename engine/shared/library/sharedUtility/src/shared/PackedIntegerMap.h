// ======================================================================
//
// PackedIntegerMap.h
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PackedIntegerMap_H
#define INCLUDED_PackedIntegerMap_H

// ======================================================================

class Iff;

#include "sharedFoundationTypes/FoundationTypes.h"

// ======================================================================

class PackedIntegerMap
{
public:

	PackedIntegerMap(int width, int height, const int *values);
	PackedIntegerMap(Iff &iff);
	PackedIntegerMap(const PackedIntegerMap &copy);
	~PackedIntegerMap();

	void save(Iff &iff) const;

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }
	int getValue(int x, int y) const;

	void setValue(int x, int y, int value);

protected:

	void _construct(int width, int height, int minimumValue, int maximumValue);
	void _destroy();

	void _setValue(int index, int value);
	int _byteSize() const 
	{ 
		const int s = (m_width*m_height*m_bitsPerElement + 7)>>3;
		return (s==0) ? 1 : s;
	}

	int m_width, m_height, m_bitsPerElement, m_minimumValue;
	uint8 *map;

};

// ======================================================================

#endif // INCLUDED_PackedIntegerMap_H
