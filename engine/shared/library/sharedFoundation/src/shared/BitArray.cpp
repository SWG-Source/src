// ======================================================================
//
// BitArray.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/BitArray.h"

#include "Archive/Archive.h"
#include "sharedFoundation/FormattedString.h"

#include <algorithm>

// ======================================================================

BitArray::BitArray() :
m_arrayData(m_defaultData),
m_numAllocatedBytes(sizeof(m_defaultData)),
m_numInUseBytes(0),
m_numInUseBits(0)
{
	memset(m_arrayData, 0, m_numAllocatedBytes);
}

// -----------------------------------------------------------------------

BitArray::BitArray(int numBits) :
m_arrayData(m_defaultData),
m_numAllocatedBytes(sizeof(m_defaultData)),
m_numInUseBytes(0),
m_numInUseBits(0)
{
	if (numBits > (m_numAllocatedBytes << 3))
	{
		m_arrayData = nullptr;
		m_numAllocatedBytes = 0;
		reserve(numBits);
		m_numInUseBytes = 0;
		m_numInUseBits = 0;
	}
	else
	{
		memset(m_arrayData, 0, m_numAllocatedBytes);
	}
}

// -----------------------------------------------------------------------

BitArray::BitArray(const BitArray &orig) :
m_arrayData(m_defaultData),
m_numAllocatedBytes(sizeof(m_defaultData)),
m_numInUseBytes(0),
m_numInUseBits(0)
{
	if (orig.m_numInUseBytes > m_numAllocatedBytes)
	{
		m_arrayData = new signed char[static_cast<size_t>(orig.m_numInUseBytes)];
		m_numAllocatedBytes = orig.m_numInUseBytes;
	}

	m_numInUseBytes = orig.m_numInUseBytes;
	m_numInUseBits = orig.m_numInUseBits;

	if (m_numInUseBytes > 0)
	{
		memcpy(m_arrayData, orig.m_arrayData, m_numInUseBytes);

		if (m_numAllocatedBytes > m_numInUseBytes)
			memset(&(m_arrayData[m_numInUseBytes]), 0, static_cast<size_t>(m_numAllocatedBytes - m_numInUseBytes));
	}
	else
	{
		memset(m_arrayData, 0, m_numAllocatedBytes);
	}
}

// -----------------------------------------------------------------------

BitArray::~BitArray()
{
	if (m_arrayData != m_defaultData)
		delete []m_arrayData;
}

// -----------------------------------------------------------------------

BitArray & BitArray::operator= (const BitArray &rhs)
{
	if (this != &rhs)
	{
		// trivial case, rhs is empty
		if ((rhs.m_numInUseBits <= 0) || (rhs.m_numInUseBytes <= 0))
		{
			clear();
			return *this;
		}

		// char array is too small, reallocate new char array
		if (m_numAllocatedBytes < rhs.m_numInUseBytes)
		{
			if (m_arrayData != m_defaultData)
				delete []m_arrayData;

			m_arrayData = new signed char[static_cast<size_t>(rhs.m_numInUseBytes)];
			m_numAllocatedBytes = rhs.m_numInUseBytes;
		}
		// char array is large enough, no reallocation required,
		// but clear out char array, if necessary
		else if (m_numInUseBytes > rhs.m_numInUseBytes)
		{
			memset(&(m_arrayData[rhs.m_numInUseBytes]), 0, static_cast<size_t>(m_numInUseBytes - rhs.m_numInUseBytes));
		}

		m_numInUseBytes = rhs.m_numInUseBytes;
		m_numInUseBits = rhs.m_numInUseBits;
		memcpy(m_arrayData, rhs.m_arrayData, static_cast<size_t>(m_numInUseBytes));
	}

	return *this;
}

// -----------------------------------------------------------------------
bool BitArray::operator== (const BitArray &rhs) const
{
	//-- Check for self equality
	if (this == &rhs)
		return true;

	// compare the individual chars in the 2 array up to the number
	// of chars needed to hold the smaller of the 2 m_numInUseBits
	int const smallerArraySize = std::min(m_numInUseBytes, rhs.m_numInUseBytes);
	for (int i = 0; i < smallerArraySize; ++i)
		if (m_arrayData[i] != rhs.m_arrayData[i])
			return false;

	// if there are any additional set bits (i.e. non-zero char)
	// in the larger of the 2 arrays, then equality is false
	if (m_numInUseBytes != rhs.m_numInUseBytes)
	{
		signed char const * largerArrayData;
		int largerArraySize;

		if (m_numInUseBytes > rhs.m_numInUseBytes)
		{
			largerArraySize = m_numInUseBytes;
			largerArrayData = m_arrayData;
		}
		else
		{
			largerArraySize = rhs.m_numInUseBytes;
			largerArrayData = rhs.m_arrayData;
		}

		for (int j = smallerArraySize; j < largerArraySize; ++j)
			if (largerArrayData[j])
				return false;
	}

	return true;
}

// -----------------------------------------------------------------------

bool BitArray::operator!=(const BitArray &rhs) const
{
	return !operator==(rhs);
}

// -----------------------------------------------------------------------

void BitArray::setBit(int const index)
{
	DEBUG_FATAL(index < 0, ("BitArray::setBit index [%d] out of range", index));

	if (index >= 0)
	{
		if (index >= m_numInUseBits)
			reserve(index+1);

		int const arrayPos = index >> 3;  //lint !e702  shift right of signed quantity (DEBUG_FATALs if index < 0)
		int const bitPos = index & 7;
		m_arrayData[arrayPos] |= (1 << bitPos);
	}
}

// -----------------------------------------------------------------------

void BitArray::setValue(int const beginIndex, int const endIndex, unsigned long value)
{
	DEBUG_FATAL(beginIndex < 0, ("BitArray::setValue beginIndex [%d] out of range", beginIndex));
	DEBUG_FATAL(endIndex < 0, ("BitArray::setValue endIndex [%d] out of range", endIndex));
	DEBUG_FATAL(beginIndex > endIndex, ("BitArray::setValue beginIndex [%d] > endIndex [%d]", beginIndex, endIndex));

	if ((beginIndex >= 0) && (endIndex >= 0) && (beginIndex <= endIndex))
	{
		int currentIndex = beginIndex;
		while (currentIndex <= endIndex)
		{
			if (value == 0)
				break;

			if (value & 0x1)
				setBit(currentIndex++);
			else
				clearBit(currentIndex++);

			value >>= 1;
		}

		for (int i = currentIndex; i <= endIndex; ++i)
			clearBit(i);
	}
}

// -----------------------------------------------------------------------

void BitArray::clearBit(int const index)
{
	DEBUG_FATAL(index < 0, ("BitArray::clearBit index [%d] out of range", index));

	if ((index >= 0) && (index < m_numInUseBits))
	{
		int const arrayPos = index >> 3;  //lint !e702  shift right of signed quantity (DEBUG_FATALs if index < 0)
		int const bitPos = index & 7;
		m_arrayData[static_cast<size_t>(arrayPos)] &= ~(1 << bitPos);
	}
}

// -----------------------------------------------------------------------

bool BitArray::testBit(int const index) const
{
	DEBUG_FATAL(index < 0, ("BitArray::testBit index [%d] out of range", index));

	if ((index < 0) || (index >= m_numInUseBits))
		return false;

	int const arrayPos = index >> 3;  //lint !e702  shift right of signed quantity (DEBUG_FATALs if index < 0)
	int const bitPos = index & 7;
	return ((m_arrayData[arrayPos] & (1 << bitPos)) != 0);
}

// -----------------------------------------------------------------------

unsigned long BitArray::getValue(int const beginIndex, int const endIndex) const
{
	DEBUG_FATAL(beginIndex < 0, ("BitArray::setValue beginIndex [%d] out of range", beginIndex));
	DEBUG_FATAL(endIndex < 0, ("BitArray::setValue endIndex [%d] out of range", endIndex));
	DEBUG_FATAL(beginIndex > endIndex, ("BitArray::setValue beginIndex [%d] > endIndex [%d]", beginIndex, endIndex));

	unsigned long value = 0;
	if ((beginIndex >= 0) && (endIndex >= 0) && (beginIndex <= endIndex))
	{
		for (int i = endIndex; i >= beginIndex; --i)
		{
			if (value)
				value <<= 1;

			if (testBit(i))
				++value;
		}
	}

	return value;
}

// -----------------------------------------------------------------------

void BitArray::reserve(int const numBits)
{
	DEBUG_FATAL(numBits < 0, ("BitArray::reserve number of bits [%d] out of range", numBits));

	if (numBits > 0)
	{
		// if shrinking, make sure to clear all the bits between
		// the new size and the old size to to maintain requirement
		// that every bit past 0-based index (m_numInUseBits - 1) is
		// guaranteed to be unset
		if (numBits < m_numInUseBits)
		{
			for (int i = numBits; i < m_numInUseBits; ++i)
				clearBit(i);
		}

		m_numInUseBits = numBits;

		int const oldNumInUseBytes = m_numInUseBytes;
		m_numInUseBytes = (m_numInUseBits + 7) >> 3;

		if (m_numInUseBytes > m_numAllocatedBytes)
		{
			signed char * tmp = new signed char[static_cast<size_t>(m_numInUseBytes)];
			memset(&(tmp[oldNumInUseBytes]), 0, static_cast<size_t>(m_numInUseBytes - oldNumInUseBytes));
			if ((oldNumInUseBytes > 0) && (m_arrayData != nullptr))
				memcpy(tmp, m_arrayData, static_cast<size_t>(oldNumInUseBytes)); //lint !e671 !e670 logically, you can't enter this code block unless oldNumInUseBytes is smaller.

			m_numAllocatedBytes = m_numInUseBytes;
			
			if (m_arrayData != m_defaultData)
				delete []m_arrayData;

			m_arrayData = tmp;
		}
	}
	else if (numBits == 0)
	{
		if (m_arrayData != m_defaultData)
			delete []m_arrayData;

		m_arrayData = m_defaultData;
		m_numAllocatedBytes = sizeof(m_defaultData);
		m_numInUseBytes = 0;
		m_numInUseBits = 0;

		memset(m_arrayData, 0, m_numAllocatedBytes);
	}
}

// -----------------------------------------------------------------------
bool BitArray::match(BitArray const & rhs) const
{
	return operator==(rhs);
}

// -----------------------------------------------------------------------

bool BitArray::matchAnyBit(BitArray const & rhs) const
{
	// compare the individual chars in the 2 array up to the number
	// of chars needed to hold the smaller of the 2 m_numInUseBits
	int const smallerArraySize = std::min(m_numInUseBytes, rhs.m_numInUseBytes);
	for (int i = 0; i < smallerArraySize; ++i)
		if (m_arrayData[i] & rhs.m_arrayData[i])
			return true;

	return false;
}

// -----------------------------------------------------------------------

namespace Archive
{
	void get(ReadIterator &source, BitArray &target)
	{
		int numInUseBytes;
		int numInUseBits;

		get(source, numInUseBytes);
		get(source, numInUseBits);

		// trivial case, rhs is empty
		if ((numInUseBits <= 0) || (numInUseBytes <= 0))
		{
			target.clear();
			return;
		}

		// char array is too small, reallocate new char array
		if (target.m_numAllocatedBytes < numInUseBytes)
		{
			if (target.m_arrayData != target.m_defaultData)
				delete []target.m_arrayData;

			target.m_arrayData = new signed char[static_cast<size_t>(numInUseBytes)];
			target.m_numAllocatedBytes = numInUseBytes;
		}
		// char array is large enough, no reallocation required,
		// but clear out char array, if necessary
		else if (target.m_numInUseBytes > numInUseBytes)
		{
			memset(&(target.m_arrayData[numInUseBytes]), 0, static_cast<size_t>(target.m_numInUseBytes - numInUseBytes));
		}

		target.m_numInUseBytes = numInUseBytes;
		target.m_numInUseBits = numInUseBits;

		for (int i = 0; i < target.m_numInUseBytes; ++i)
		{
			get(source, target.m_arrayData[i]);
		}
	}

	void put(ByteStream &target, const BitArray &source)
	{
		put(target, source.m_numInUseBytes);
		put(target, source.m_numInUseBits);
		if ((source.m_numInUseBits > 0) && (source.m_numInUseBytes > 0))
		{
			put(target, source.m_arrayData, source.m_numInUseBytes);
		}
	}
}

// -----------------------------------------------------------------------

void BitArray::clear()
{
	if (m_numInUseBytes > 0)
	{
		memset(m_arrayData, 0, static_cast<size_t>(m_numInUseBytes));
		m_numInUseBytes = 0;
		m_numInUseBits = 0;
	}
}

// -----------------------------------------------------------------------

void BitArray::setMultipleBits(int const count)
{
	if (count <= 0)
		return;

	if (count > m_numInUseBits)
		reserve(count);

	int const charCount = (count >> 3);
	if (charCount)
		memset(m_arrayData, static_cast<unsigned>(0xFFFFFFFF), static_cast<size_t>(charCount));

	for (int i = (charCount << 3); i < count; ++i)
		setBit(i);
}

// -----------------------------------------------------------------------

void BitArray::insertBit(int const index, bool const value)
{
	DEBUG_FATAL(index < 0, ("BitArray::insertBit index [%d] out of range", index));

	// nothing to do if inserting a new bit past
	// the end and the new bit will not be set
	if (!value && (index >= m_numInUseBits))
		return;

	// if inserting a new bit in the middle, then shift all
	// bits at and after the insertion point "backward"
	if (index >= 0 && index < m_numInUseBits)
	{
		for(int i = (m_numInUseBits - 1); i >= index; --i)
		{
			if (testBit(i))
				setBit(i + 1);
			else
				clearBit(i + 1);
		}
	}

	// set/clear the inserted bit
	if (value)
		setBit(index);
	else
		clearBit(index);
}

// -----------------------------------------------------------------------

void BitArray::removeBit(int const index)
{
	DEBUG_FATAL(index < 0, ("BitArray::removeBit index [%d] out of range", index));
	
	// shift all bits after the removal point "forward"
	if (index >= 0 && index < m_numInUseBits)
	{
		int const lastBitIndexToShift = m_numInUseBits - 1;
		for(int i = index; i < lastBitIndexToShift; ++i)
		{
			if (testBit(i + 1))
				setBit(i);
			else
				clearBit(i);
		}

		// shrink the array by 1
		reserve(m_numInUseBits - 1);
	}
}

// -----------------------------------------------------------------------

int BitArray::getNumberOfSetBits() const
{
	int count = 0;
	for (int i = 0; i < m_numInUseBits; ++i)
		if (testBit(i))
			++count;

	return count;
}

// -----------------------------------------------------------------------

bool BitArray::empty() const
{
	for (size_t i = 0; i < static_cast<size_t>(m_numInUseBytes); ++i)
	{
		if (m_arrayData[i])
			return false;
	}

	return true;
}

// -----------------------------------------------------------------------

void BitArray::getAsDbTextString(std::string &result, int const maxNibbleCount /*= 32767*/) const
{
	result.clear();

	static const char hexDigits[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	
	bool anyBitSet = false;
	unsigned char nibbleIntValue;
	int const nibbleCount = std::min(maxNibbleCount, (m_numInUseBytes << 1));
	for (int i = (nibbleCount - 1); i >= 0; --i)
	{
		nibbleIntValue = static_cast<unsigned char>(m_arrayData[i >> 1]);
		if (i % 2)
			nibbleIntValue >>= 4;
		else
			nibbleIntValue &= static_cast<unsigned char>(0xF);

		if (nibbleIntValue && !anyBitSet)
		{
			anyBitSet = true;
			result.resize(i + 1);
		}

		if (anyBitSet)
			result[i] = hexDigits[nibbleIntValue];
	}
}

// -----------------------------------------------------------------------

void BitArray::setFromDbTextString(const char * text)
{
	clear();

	int const nibbleCount = (text ? strlen(text) : 0);
	if (nibbleCount > 0)
	{
		bool anyBitSet = false;
		unsigned char nibbleIntValue;

		// set the highest index bit first to avoid constant reallocation
		for (int i = (nibbleCount - 1); i >= 0; --i)
		{
			if ((text[i] >= '0') && (text[i] <= '9'))
				nibbleIntValue = static_cast<unsigned char>(text[i] - '0');
			else if ((text[i] >= 'A') && (text[i] <= 'F'))
				nibbleIntValue = static_cast<unsigned char>(text[i] - 'A' + 10);
			else if ((text[i] >= 'a') && (text[i] <= 'f'))
				nibbleIntValue = static_cast<unsigned char>(text[i] - 'a' + 10);
			else
				nibbleIntValue = static_cast<unsigned char>(0);

			if (nibbleIntValue)
			{
				if (!anyBitSet)
				{
					anyBitSet = true;
					reserve(((i >> 1) + 1) << 3);
				}

				if (i % 2)
					m_arrayData[i >> 1] |= static_cast<signed char>(nibbleIntValue << 4);
				else
					m_arrayData[i >> 1] |= static_cast<signed char>(nibbleIntValue);
			}
		}
	}
}

// -----------------------------------------------------------------------

std::string BitArray::getDebugString() const
{
	std::string s = FormattedString<512>().sprintf("%6d: ", m_numInUseBits);
	int i;
	for (i = 0; i < m_numInUseBits; ++i)
	{
		if ((i > 0) && ((i % 8) == 0))
			s += " ";

		if (testBit(i))
			s += "1";
		else
			s += "0";
	}

	s += " (";

	std::string baString;
	getAsDbTextString(baString);
	s += baString;

	s += ") (";

	for (int i = 0; i < m_numAllocatedBytes; ++i)
	{
		if (i > 0)
			s += " ";

		if (i == m_numInUseBytes)
			s += "   ";

		s += FormattedString<512>().sprintf("%02X", static_cast<unsigned char>(m_arrayData[i]));
	}

	s += ")";

	if (m_arrayData == m_defaultData)
		s += FormattedString<512>().sprintf(" (%d/%d default storage)", m_numInUseBytes, m_numAllocatedBytes);
	else
		s += FormattedString<512>().sprintf(" (%d/%d heap storage)", m_numInUseBytes, m_numAllocatedBytes);

	return s;
}

// ======================================================================
