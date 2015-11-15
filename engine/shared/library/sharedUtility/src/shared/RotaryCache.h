// ======================================================================
//
// RotaryCache.h
// Copyright 2007 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_RotaryCache_H
#define INCLUDED_RotaryCache_H


//===================================================================

class CrcLowerString;

//===================================================================

class RotaryCache
{
	// Member Functions
public:
	RotaryCache();

	virtual ~RotaryCache();

	bool initialize(unsigned int maxCache);

	void* add(const CrcLowerString& keyString, void* valuePtr);

	void* fetch(const CrcLowerString& keyString);

	void* remove(const CrcLowerString& keyString);

	void* getFirst();

	void* getNext();

	void clear();
protected:

	// Member Data
public:

protected:
	struct CacheListEntry
	{
		CrcLowerString			key;
		void*					value;
	};

	typedef stdlist<CacheListEntry>::fwd	RotaryList;

	struct CacheMapEntry
	{
		RotaryList::iterator				iter;
	};

	typedef stdmap<unsigned int,CacheMapEntry>::fwd	RotaryMap;

	RotaryList		mList;
	RotaryMap		mMap;
	unsigned int	mMaxEntries,
					mNumEntries;

	RotaryList::iterator	mIterList;

};

#endif	// INCLUDED_RotaryCache_H
