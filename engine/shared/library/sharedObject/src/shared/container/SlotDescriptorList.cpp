// ======================================================================
//
// SlotDescriptorList.cpp
// Copyright 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/SlotDescriptorList.h"

#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedObject/SlotDescriptor.h"

#include <map>
#include <string>

// ======================================================================

bool                               SlotDescriptorList::ms_installed;
SlotDescriptorList::DescriptorMap *SlotDescriptorList::ms_descriptors;

// ======================================================================
// class SlotDescriptorList
// ======================================================================

void SlotDescriptorList::install()
{
	DEBUG_FATAL(ms_installed, ("SlotDescriptorList already installed"));

	ms_descriptors = new DescriptorMap();

	ms_installed = true;
	ExitChain::add(remove, "SlotDescriptorList");
}

// ----------------------------------------------------------------------

const SlotDescriptor *SlotDescriptorList::fetch(const CrcLowerString &filename)
{
	DEBUG_FATAL(!ms_installed, ("SlotDescriptorList not installed"));

	SlotDescriptor *descriptor = 0;

	//-- check if we've got this one cached
	DescriptorMap::iterator lowerBoundResult = ms_descriptors->lower_bound(&filename);
	if ((lowerBoundResult != ms_descriptors->end()) && !ms_descriptors->key_comp()(&filename, lowerBoundResult->first))
	{
		// it's cached, return it
		descriptor = lowerBoundResult->second;
	}
	else
	{
		// it's not available

		//-- ensure sure file exists
		if (!TreeFile::exists(filename.getString()))
		{
			WARNING(true, ("specified SlotDescriptor file [%s] does not exist", filename.getString()));
			return 0;
		}

#ifdef _DEBUG
		DataLint::pushAsset(filename.getString());
#endif // _DEBUG

		//-- load it
		Iff iff(filename.getString());
		descriptor = new SlotDescriptor(iff, filename);

#ifdef _DEBUG
		DataLint::popAsset();
#endif // _DEBUG

		//-- add to managed container
		IGNORE_RETURN(ms_descriptors->insert(lowerBoundResult, DescriptorMap::value_type(&descriptor->getName(), descriptor)));
	}

	// we should have a valid slot descriptor here
	NOT_NULL(descriptor);

	//-- bump up reference count
	descriptor->fetch();

	return descriptor;
}

// ----------------------------------------------------------------------
/**
 * Return the SlotDescriptor instance contained in the given file.
 *
 * Prefer the variant of this function taking a CrcLowerString if you
 * already have the filename in that format.
 */

const SlotDescriptor *SlotDescriptorList::fetch(const std::string &filename)
{
	DEBUG_FATAL(!ms_installed, ("SlotDescriptorList not installed"));

	return fetch(CrcLowerString(filename.c_str()));
}

// ======================================================================

void SlotDescriptorList::remove()
{
	DEBUG_FATAL(!ms_installed, ("SlotDescriptorList not installed"));

	//-- handle leaks
	if (!ms_descriptors->empty())
	{
		DEBUG_WARNING(true, ("SlotDescriptorList: %u named SlotDescriptor leaks", ms_descriptors->size()));

		const DescriptorMap::iterator endIt = ms_descriptors->end();
		for (DescriptorMap::iterator it = ms_descriptors->begin(); it != endIt; ++it)
		{
			SlotDescriptor *const descriptor = it->second;

			DEBUG_WARNING(descriptor, ("-- leaked SlotDescriptor [%s]", descriptor->getName().getString()));
			delete descriptor;
		}
	}

	delete ms_descriptors;

	ms_installed = false;
}

// ----------------------------------------------------------------------

void SlotDescriptorList::stopTracking(const SlotDescriptor &descriptor)
{
	DEBUG_FATAL(!ms_installed, ("SlotDescriptorList not installed"));

	const CrcLowerString &filename = descriptor.getName();

	DescriptorMap::iterator lowerBoundResult = ms_descriptors->lower_bound(&filename);
	if ((lowerBoundResult == ms_descriptors->end()) || ms_descriptors->key_comp()(&filename, lowerBoundResult->first))
	{
		// we don't manage this descriptor
		return;
	}

	//-- remove the SlotDescriptor from the map
	ms_descriptors->erase(lowerBoundResult);
}

// ======================================================================
