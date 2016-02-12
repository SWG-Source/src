// ======================================================================
//
// RotaryCache.cpp
// Copyright 2007 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedFoundationTypes/FoundationTypes.h"
#include "sharedFoundation/CrcLowerString.h"

#include <map>
#include <list>

#include "sharedUtility/RotaryCache.h"


RotaryCache::RotaryCache() 
	: mMaxEntries(0)
	, mNumEntries(0)
{

}

RotaryCache::~RotaryCache()
{
	clear();
}

bool 
RotaryCache::initialize(unsigned int maxCache)
{
	clear();
	
	mMaxEntries = maxCache;

	return true;
}

void* 
RotaryCache::add(const CrcLowerString& keyString, void* valuePtr)
{
	CacheListEntry	listEntry;
	void*			returnVal = nullptr;

	listEntry.key = keyString;
	listEntry.value = valuePtr;
	mList.push_front(listEntry);

	RotaryList::iterator	iter = mList.begin();

	CacheMapEntry	mapEntry;

	mapEntry.iter = iter;

	mMap.insert(RotaryMap::value_type(listEntry.key.getCrc(),mapEntry));

	mNumEntries++;

	if(mNumEntries > mMaxEntries)
	{
		CacheListEntry&	backEntry = mList.back();
		returnVal = backEntry.value;

		mMap.erase(backEntry.key.getCrc());

		mNumEntries--;

		mList.pop_back();
	}

	return returnVal;
}

void* 
RotaryCache::fetch(const CrcLowerString& keyString)
{
	RotaryMap::iterator	iterFind = mMap.find(keyString.getCrc());

	if(iterFind != mMap.end())
	{
		CacheMapEntry&	entryFind = (*iterFind).second;

		RotaryList::iterator	iterList = entryFind.iter;

		CacheListEntry entryList = (*iterList);

		mList.erase(iterList);
		mList.push_front(entryList);
		entryFind.iter = mList.begin();

		return entryList.value;
	}

	return nullptr;
}

void* 
RotaryCache::remove(const CrcLowerString& keyString)
{
	RotaryMap::iterator	iterFind = mMap.find(keyString.getCrc());

	if(iterFind != mMap.end())
	{
		void*	retVal = nullptr;
		CacheMapEntry&	entryFind = (*iterFind).second;

		RotaryList::iterator	iterList = entryFind.iter;

		CacheListEntry& entryList = (*iterList);


		retVal = entryList.value;

		mList.erase(iterList);
		mMap.erase(iterFind);

		return retVal;
	}

	return nullptr;

}

void* 
RotaryCache::getFirst()
{
	mIterList = mList.begin();

	return getNext();
}

void* 
RotaryCache::getNext()
{
	if(mIterList != mList.end())
	{
		void*	retVal = (*mIterList).value;

		mIterList++;

		return retVal;
	}

	return nullptr;
}

void 
RotaryCache::clear()
{
	mMap.clear();
	mList.clear();
}
