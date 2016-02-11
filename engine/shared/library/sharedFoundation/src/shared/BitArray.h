// ======================================================================
//
// BitArray.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_BitArray_H
#define INCLUDED_BitArray_H

//----------------------------------------------------------------------

class BitArray;

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get (ReadIterator &source, BitArray &target);
	void put (ByteStream &target, const BitArray &source);
}

//----------------------------------------------------------------------

class BitArray
{
public:
	         BitArray      ();
	explicit BitArray      (int numBits);
	         BitArray      (const BitArray&);
	virtual ~BitArray      ();

	void     setBit        (int index);
	void     setValue      (int beginIndex, int endIndex, unsigned long value);
	void     setMultipleBits(int count);
	void     clearBit      (int index);
	bool     testBit       (int index) const;
	unsigned long getValue (int beginIndex, int endIndex) const;
	void clear();
	int getNumberOfSetBits() const;
	bool empty() const;
	void insertBit(int const index, bool value);
	void removeBit(int const index);
	std::string getDebugString() const;

	// Equivalent to operator==()
	bool     match         (const BitArray &rhs) const;

	// Returns true if array has any bits set which this array also has set.
	bool     matchAnyBit   (const BitArray &rhs) const;

	//operators
	BitArray & operator =  (const BitArray &rhs);
	bool       operator == (const BitArray &rhs) const;
	bool       operator!=  (const BitArray &rhs) const;

	// for DB persistence purpose, returns the BitArray as a
	// std::string where each 4 bits are printed as a hex nibble;
	// and the converse that takes the nullptr-terminated string
	// and converts it back into the BitArray
	void getAsDbTextString(std::string &result, int maxNibbleCount = 32767) const;
	void setFromDbTextString(const char * text);

	friend void Archive::get (Archive::ReadIterator &source, BitArray &target);
	friend void Archive::put (Archive::ByteStream &target, const BitArray &source);

private:
	// *****WARNING***** *****WARNING***** *****WARNING***** *****WARNING*****
	// do not expose the "size" (either m_numInUseBits, m_numInUseBytes, or
	// m_numAllocatedBytes) to the outside world, since it is not guaranteed
	// to be accurate in terms of what the outside world expects the "size"
	// to be; for optimization, BitArray doesn't necessarily change the "size"
	// value(s) if a particular bit index is not set; for example, if you do
	// setBit(1000000), and then do clearBit(1000000), the "size" is not
	// necessarily 1000000; the only guarantee we make here is that
	// testBit(1000000) will return false; the bottom line is that BitArray
	// does not maintain the "size" of the BitArray, so no function should
	// ever be added to return the "size" of the BitArray
	// *****WARNING***** *****WARNING***** *****WARNING***** *****WARNING*****

	// to avoid constant reallocation for small BitArray, small BitArray will
	// use this as storage; only when the BitArray is larger than 128 (16*8)
	// will we need to allocate off the heap
	signed char m_defaultData[16];

	// points to the storage for the BitArray, which will be either m_defaultData
	// or heap allocated storage for BitArray larger than 128 (16*8)
	signed char *m_arrayData;

	// how many chars allocated in m_arrayData
	int         m_numAllocatedBytes;

	// how many chars in m_arrayData that's actually used for the BitArray;
	// this will always be <= m_numAllocatedBytes;
	// this will always be 0 or (m_numInUseBits+7) >> 3, but we cache it here
	// so we don't constantly have to calculate (m_numInUseBits+7) >> 3
	int         m_numInUseBytes;

	// storage (i.e. m_arrayData) may be allocated to handle more than m_numInUseBits,
	// because its expensive to be constantly expanding and shrinking m_arrayData;
	// however, every bit past 0-based index (m_numInUseBits - 1) is guaranteed to be unset
	int         m_numInUseBits;

	void     reserve       (int numBits);
};

//----------------------------------------------------------------------

#endif
