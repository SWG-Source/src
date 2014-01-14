// ======================================================================
//
// PaletteArgbList.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/PaletteArgbList.h"

#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedSynchronization/Mutex.h"

#include <map>

// ======================================================================

namespace PaletteArgbListNamespace
{
	Mutex  s_criticalSection;
}

using namespace PaletteArgbListNamespace;

// ======================================================================

bool                          PaletteArgbList::ms_installed;
PaletteArgbList::ResourceMap *PaletteArgbList::ms_resourceMap;

// ======================================================================
/**
 * Install the PaletteArgbList.
 *
 * This function must be invoked prior to using any
 * other aspect of the PaletteArgbList class.  As of this writing,
 * PaletteArgbList::install() is invoked via SetupSharedMath::install().
 *
 * @see SetupSharedMath.
 */

void PaletteArgbList::install()
{
	DEBUG_FATAL(ms_installed, ("PaletteArgbList already installed"));

	ms_resourceMap = new ResourceMap();

	ms_installed = true;
	ExitChain::add(remove, "PaletteArgbList");
}

// ----------------------------------------------------------------------
/**
 * Retrieve a PaletteArgb instance loaded from the specified filename.
 *
 * The specified pathName is the path to the Microsoft Palette (r) (.PAL)
 * file to load via the TreeFile System.  If the given file currently
 * is loaded, the same instance with a bumped up reference count will
 * be returned.
 *
 * This function bumps up the reference count on the returned instance.
 * When the caller is done with the PaletteArgb, it should call
 * PaletteArgb::release().
 *
 * If the specfied filename can not be found, a default palette with
 * one all-zero entry will be returned along with a WARNING.
 *
 * @param pathName  TreeFile-accessible pathname to a Microsoft Palette (r)
 *                  file.
 *
 * @return          the contents of the Palette file if the palette file
 *                  exists; otherwise, a default palette instance.
 *
 * @see PaletteArgb::release().
 */

const PaletteArgb *PaletteArgbList::fetch(const CrcString &pathName)
{
	return fetch(pathName, true);
}

// ----------------------------------------------------------------------

void PaletteArgbList::assignAsynchronousLoaderFunctions()
{
	if (AsynchronousLoader::isInstalled())
		AsynchronousLoader::bindFetchReleaseFunctions("pal", &asynchronousLoaderFetchNoCreate, &asynchronousLoaderRelease);
}

// ======================================================================

void PaletteArgbList::remove()
{
	DEBUG_FATAL(!ms_installed, ("PaletteArgbList not installed"));

	//-- release exisitng palettes, report memory leaks
	//DEBUG_REPORT_LOG(!ms_resourceMap->empty(), ("PaletteArgbList: loaded [%u] palettes:", ms_resourceMap->size()));

	const ResourceMap::iterator endIt = ms_resourceMap->end();
	for (ResourceMap::iterator it = ms_resourceMap->begin(); it != endIt; ++it)
	{
		PaletteArgb const * palette = it->second;

		// print leak info
		NOT_NULL(palette);
		DEBUG_WARNING(palette->getReferenceCount() > 0, ("  palette [%s]: %d references outstanding", it->first->getString(), palette->getReferenceCount()));

		// delete the resource
		delete palette;
	}

	//-- delete the map
	delete ms_resourceMap;
	ms_resourceMap = 0;

	ms_installed = false;
}

// ----------------------------------------------------------------------

void PaletteArgbList::stopTracking(const PaletteArgb &palette)
{
	DEBUG_FATAL(!ms_installed, ("PaletteArgbList not installed"));

	s_criticalSection.enter();

		//-- check if this palette is named.  if not, ignore it.
		const char *const paletteName = palette.getName().getString();
		if (!paletteName || !*paletteName)
		{
			// palette has no name, so the list isn't tracking it.
			s_criticalSection.leave();
			return;
		}

		//-- find it in our list.
		const ResourceMap::iterator findIt = ms_resourceMap->find(&palette.getName());
		if (findIt == ms_resourceMap->end())
		{
			// not found
			DEBUG_WARNING(true, ("named palette [%s] not tracked, shouldn't happen.", palette.getName().getString()));
		}
		else
		{
			// found it
			ms_resourceMap->erase(findIt);
		}

	s_criticalSection.leave();
}

// ----------------------------------------------------------------------

const void *PaletteArgbList::asynchronousLoaderFetchNoCreate(char const *fileName)
{
	TemporaryCrcString cfn(fileName, true);
	return PaletteArgbList::fetch(cfn, false);
}

// ----------------------------------------------------------------------

void PaletteArgbList::asynchronousLoaderRelease(void const *palette)
{
	static_cast<PaletteArgb const *>(palette)->release();
}

// ----------------------------------------------------------------------

const PaletteArgb *PaletteArgbList::fetch(const CrcString &pathName, bool create)
{
	DEBUG_FATAL(!ms_installed, ("PaletteArgbList not installed"));

	s_criticalSection.enter();

		//-- check if the pathName is cached
		const ResourceMap::iterator lowerBoundResult = ms_resourceMap->lower_bound(&pathName);
		const bool                  haveResource     = ((lowerBoundResult != ms_resourceMap->end()) && !ms_resourceMap->key_comp()(&pathName, lowerBoundResult->first));

		if (haveResource)
		{
			NOT_NULL(lowerBoundResult->second);
			lowerBoundResult->second->fetch();

			PaletteArgb const *const palette = lowerBoundResult->second;

			s_criticalSection.leave();
			return palette;
		}

		// resource doesn't exist
		
		//-- Skip if not creating.
		if (!create)
		{
			s_criticalSection.leave();
			return 0;
		}

		PaletteArgb *palette = 0;

		//-- check if referenced filename exists
		if (!TreeFile::exists(pathName.getString()))
		{
			//-- palette file can't be found, return a new default palette.
			//   note: this palette doesn't get mapped, so every call for this
			//         non-existent palette will generate a new one.  That's okay,
			//         this should be a rare occurrence.  If it is not rare and acceptable,
			//         we'll want a single default palette allocated and returned.
			WARNING(true, ("palette [%s] not found, using default palette.", pathName.getString()));
			palette = new PaletteArgb(1);

			// Set only color to something we can tell is a bug.
			bool error = false;
			PackedArgb &entry = palette->getEntry(0, error);
			WARNING(error, ("PaletteArgbList::fetch error"));
			entry.setArgb(255, 0, 255, 255);
		}
		else
		{
			//-- palette file found, load it
			palette = new PaletteArgb(pathName);

			//-- map resource to name
			IGNORE_RETURN(ms_resourceMap->insert(lowerBoundResult, ResourceMap::value_type(&palette->getName(), palette)));
		}

		//-- bump up the reference count
		NOT_NULL(palette);
		palette->fetch();

	s_criticalSection.leave();

	return palette;
}

// ======================================================================
