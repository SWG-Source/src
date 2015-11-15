//======================================================================
//
// MapLocationArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MapLocationArchive.h"

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/MapLocation.h"
#include "unicodeArchive/UnicodeArchive.h"

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator &source, MapLocation &target)
	{
		get (source, target.m_locationId);  
		get (source, target.m_locationName); 
		get (source, target.m_location.x);
		get (source, target.m_location.y);
		get (source, target.m_category);
		get (source, target.m_subCategory);
		get (source, target.m_flags);
	}

	void put (ByteStream &target, const MapLocation &source)
	{
		put (target, source.m_locationId);  
		put (target, source.m_locationName); 
		put (target, source.m_location.x);
		put (target, source.m_location.y);
		put (target, source.m_category);
		put (target, source.m_subCategory);
		put (target, source.m_flags);
	}
}

//----------------------------------------------------------------------
