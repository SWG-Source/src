// ======================================================================
//
// PackedFixedPointMap.cpp
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/PackedFixedPointMap.h"
#include "sharedFile/Iff.h"

// ======================================================================

namespace
{
	const Tag TAG_PFPM = TAG(P,F,P,M);
	const Tag TAG_CNTL = TAG(C,N,T,L);

	class IntBufferCleanup
	{
	public:
		IntBufferCleanup(int *p) : m_p(p) {}
		~IntBufferCleanup() { delete [] m_p; }
		operator int *() { return m_p; }
		int *m_p;
	};
}

int *PackedFixedPointMap::_convertFloatMap(const float *values, int size, float resolution)
{
	if (!values || !size)
	{
		return 0;
	}
	else
	{
		int *returnValue = new int[size];
		for (int i=0;i<size;i++)
		{
			returnValue[i]=int(floorf(values[i]/resolution + 0.5f));
		}
		return returnValue;
	}
}

Iff &PackedFixedPointMap::_beginLoad(Iff &iff)
{
	iff.enterForm(TAG_PFPM);
	return iff;
}

PackedFixedPointMap::PackedFixedPointMap(int width, int height, float resolution, const float *values)
:  m_integerMap(width, height, IntBufferCleanup(_convertFloatMap(values, width*height, resolution))),
	m_resolution(resolution)
{
}

PackedFixedPointMap::PackedFixedPointMap(Iff &iff)
:  m_integerMap(_beginLoad(iff))
{
		iff.enterForm(TAG_0000);

			iff.enterChunk(TAG_CNTL);
				m_resolution            = iff.read_float();
			iff.exitChunk(TAG_CNTL);

		iff.exitForm(TAG_0000);

	iff.exitForm(TAG_PFPM);
}

PackedFixedPointMap::PackedFixedPointMap(const PackedFixedPointMap &copy)
:	m_integerMap(copy.m_integerMap),
	m_resolution(copy.m_resolution)
{
}

PackedFixedPointMap::~PackedFixedPointMap()
{
}

void PackedFixedPointMap::save(Iff &iff) const
{
	iff.insertForm(TAG_PFPM);

		m_integerMap.save(iff);

		iff.insertForm(TAG_0000);

			iff.insertChunk(TAG_CNTL);
				iff.insertChunkData(m_resolution);
			iff.exitChunk(TAG_CNTL);

		iff.exitForm(TAG_0000);

	iff.exitForm (TAG_PFPM);
}

// ======================================================================

float PackedFixedPointMap::getValue(int x, int y) const
{
	DEBUG_FATAL(x<0 || y<0, (""));
	int intValue = m_integerMap.getValue(x, y);
	return float(intValue)*m_resolution;
}

void PackedFixedPointMap::setValue(int x, int y, float value)
{
	DEBUG_FATAL(x<0 || y<0, (""));
	int intValue = int(floorf(value/m_resolution + 0.5f));
	m_integerMap.setValue(x, y, intValue);
}

// ======================================================================
