// ======================================================================
//
// ArrangementDescriptorList.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/ArrangementDescriptorList.h"

#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedObject/ArrangementDescriptor.h"

#include <map>
#include <string>

// ======================================================================

bool                                      ArrangementDescriptorList::ms_installed;
ArrangementDescriptorList::DescriptorMap *ArrangementDescriptorList::ms_descriptors;

ConstCharCrcLowerString                   ArrangementDescriptorList::ms_defaultFilename("arrangement/arrangement_none.iff");


// ======================================================================

void ArrangementDescriptorList::install()
{
	DEBUG_FATAL(ms_installed, ("ArrangementDescriptorList already installed"));

	ms_descriptors = new DescriptorMap();
	ms_installed   = true;

	ExitChain::add(remove, "ArrangementDescriptorList");
}

// ----------------------------------------------------------------------

const ArrangementDescriptor *ArrangementDescriptorList::fetch(const CrcLowerString &filename)
{
	DEBUG_FATAL(!ms_installed, ("ArrangementDescriptorList not installed"));

	ArrangementDescriptor *descriptor = 0;

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
		Iff  iff;

		if (!iff.open(filename.getString(), true))
		{
			//-- make sure we didn't just fail on the default filename
			if (filename != ms_defaultFilename)
			{
				WARNING(true, ("ArrangementDescriptor file [%s] does not exist, using default", filename.getString()));
				return fetch(ms_defaultFilename);
			}
			else
			{
				WARNING(true, ("default ArrangementDescriptor [%s] could not be loaded, returning nullptr ArrangementDescriptor", filename.getString()));
				return 0;
			}
		}

#ifdef _DEBUG
		DataLint::pushAsset(filename.getString());
#endif // _DEBUG

		//-- load it
		descriptor = new ArrangementDescriptor(iff, filename);

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

const ArrangementDescriptor *ArrangementDescriptorList::fetch(const std::string &filename)
{
	DEBUG_FATAL(!ms_installed, ("ArrangementDescriptorList not installed"));

	return fetch(CrcLowerString(filename.c_str()));
}

// ======================================================================

void ArrangementDescriptorList::remove()
{
	DEBUG_FATAL(!ms_installed, ("ArrangementDescriptorList not installed"));

	//-- handle leaks
	if (!ms_descriptors->empty())
	{
		DEBUG_WARNING(true, ("ArrangementDescriptorList detected %u ArrangementDescriptor leaks:", ms_descriptors->size()));

		const DescriptorMap::iterator endIt = ms_descriptors->end();
		for (DescriptorMap::iterator it = ms_descriptors->begin(); it != endIt; ++it)
		{
			ArrangementDescriptor *const descriptor = it->second;

			DEBUG_WARNING(descriptor, ("-- leaked ArrangementDescriptor [%s]", descriptor->getName().getString()));
			delete descriptor;
		}
	}

	delete ms_descriptors;

	ms_installed = false;
}

// ----------------------------------------------------------------------

void ArrangementDescriptorList::stopTracking(const ArrangementDescriptor &descriptor)
{
	DEBUG_FATAL(!ms_installed, ("ArrangementDescriptorList not installed"));

	//-- find the descriptor within our managed container
	const CrcLowerString &filename = descriptor.getName();

	DescriptorMap::iterator lowerBoundResult = ms_descriptors->lower_bound(&filename);
	if ((lowerBoundResult == ms_descriptors->end()) || ms_descriptors->key_comp()(&filename, lowerBoundResult->first))
	{
		// Specified descriptor not managed.  It could be unnamed (i.e. anonymous).
		return;
	}

	//-- remove from managed container
	ms_descriptors->erase(lowerBoundResult);
}

// ======================================================================
