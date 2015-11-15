// ======================================================================
//
// PackedIntegerMap.cpp
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/PackedIntegerMap.h"
#include "sharedFile/Iff.h"

// ======================================================================

namespace
{
	const Tag TAG_PIMP = TAG(P,I,M,P);
	const Tag TAG_CNTL = TAG(C,N,T,L);
}

PackedIntegerMap::PackedIntegerMap(int width, int height, const int *values)
{
	int i;
	const int size = width*height;
	int minimumValue, maximumValue;

	if (values)
	{
		minimumValue=values[0];
		maximumValue=values[0];
		for (i=1;i<size;i++)
		{
			const int v = values[i];
			if (v<minimumValue)
			{
				minimumValue=v;
			}
			else if (v>maximumValue)
			{
				maximumValue=v;
			}
		}
	}
	else
	{
		minimumValue=maximumValue=0;
	}

	_construct(width, height, minimumValue, maximumValue);

	if (values)
	{
		for (i=0;i<size;i++)
		{
			_setValue(i, values[i]);
		}
	}
	else
	{
		memset(map, 0, _byteSize());
	}
}

PackedIntegerMap::PackedIntegerMap(Iff &iff)
{
	iff.enterForm(TAG_PIMP);

		iff.enterForm(TAG_0000);

			iff.enterChunk(TAG_CNTL);
				m_width              = iff.read_int32();
				m_height             = iff.read_int32();
				m_bitsPerElement     = iff.read_int32();
				m_minimumValue       = iff.read_int32();
			iff.exitChunk(TAG_CNTL);

			const int size = _byteSize();
			map = new uint8[size];

			iff.enterChunk(TAG_DATA);
				iff.readRest_uint8(map, size);
			iff.exitChunk(TAG_DATA);

		iff.exitForm(TAG_0000);

	iff.exitForm(TAG_PIMP);
}

PackedIntegerMap::PackedIntegerMap(const PackedIntegerMap &copy)
:	m_width(copy.m_width),
	m_height(copy.m_height),
	m_bitsPerElement(copy.m_bitsPerElement),
	m_minimumValue(copy.m_minimumValue),
	map(0)
{
	const int size = _byteSize();
	map = new uint8[size];
	memcpy(map, copy.map, size);
}

PackedIntegerMap::~PackedIntegerMap()
{
	delete [] map;
}

void PackedIntegerMap::_construct(int width, int height, int minimumValue, int maximumValue)
{
	m_width=width;
	m_height=height;

	const unsigned range = unsigned(maximumValue-minimumValue+1);
	for (m_bitsPerElement=0;unsigned(1<<m_bitsPerElement)<range;m_bitsPerElement++);

	m_minimumValue=minimumValue;

	const int size = _byteSize();
	map = new uint8[size];
}

void PackedIntegerMap::save(Iff &iff) const
{
	iff.insertForm(TAG_PIMP);

		iff.insertForm(TAG_0000);

			iff.insertChunk(TAG_CNTL);
				iff.insertChunkData(m_width);
				iff.insertChunkData(m_height);
				iff.insertChunkData(m_bitsPerElement);
				iff.insertChunkData(m_minimumValue);
			iff.exitChunk(TAG_CNTL);

			iff.insertChunk(TAG_DATA);
				iff.insertChunkData(map, _byteSize());
			iff.exitChunk(TAG_DATA);

		iff.exitForm(TAG_0000);

	iff.exitForm (TAG_PIMP);
}

// ======================================================================

int PackedIntegerMap::getValue(int x, int y) const
{
	DEBUG_FATAL(x<0 || y<0, (""));
	const unsigned elementOffset = y*m_width + x;
	const unsigned bitOffset     = elementOffset * m_bitsPerElement;
	const unsigned byteOffset    = bitOffset>>3;
	const unsigned bitIndex      = bitOffset&7;

	unsigned bits=0;
	unsigned destMask=1;
	uint8 srcMask=uint8(1<<bitIndex);
	const uint8 *srcByte = map + byteOffset;
	for (int i=m_bitsPerElement;i>0;i--)
	{
		if (*srcByte & srcMask)
		{
			bits|=destMask;
		}
		destMask<<=1;
		srcMask<<=1;
		if (!srcMask)
		{
			srcMask=1;
			srcByte++;
		}
	}

	return int(bits) + m_minimumValue;
}

void PackedIntegerMap::setValue(int x, int y, int value)
{
	DEBUG_FATAL(x<0 || y<0, (""));
	_setValue(y*m_width + x, value);
}

void PackedIntegerMap::_setValue(int elementIndex, int value)
{
	const unsigned bitOffset     = elementIndex * m_bitsPerElement;
	const unsigned byteOffset    = bitOffset>>3;
	const unsigned bitIndex      = bitOffset&7;

	unsigned srcBits=unsigned(value-m_minimumValue);
	unsigned srcMask=1;
	uint8 destMask=uint8(1<<bitIndex);
	uint8 *destByte = map + byteOffset;
	for (int i=m_bitsPerElement;i>0;i--)
	{
		if (srcBits & srcMask)
		{
			*destByte |= destMask;
		}
		else
		{
			*destByte &= ~destMask;
		}

		srcMask<<=1;
		destMask<<=1;
		if (!destMask)
		{
			destMask=1;
			destByte++;
		}
	}
}

// ======================================================================
