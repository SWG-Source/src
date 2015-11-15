// ======================================================================
//
// PersistableWaypoint.cpp
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/PersistableWaypoint.h"

#include "Archive/Archive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedUtility/LocationArchive.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

PersistableWaypoint::PersistableWaypoint() :
		m_appearanceNameCrc(0),
		m_location(),
		m_name(),
		m_networkId(NetworkId::cms_invalid),
		m_color(0),
		m_active(false),
		m_detached(false)
{
}

#ifdef WIN32
bool PersistableWaypoint::operator!=(const PersistableWaypoint& rhs)
{
	return m_networkId != rhs.m_networkId ||
		m_location != rhs.m_location || 
		m_name != rhs.m_name ||
		m_appearanceNameCrc != rhs.m_appearanceNameCrc ||
		m_color != rhs.m_color ||
		m_active != rhs.m_active ||
		m_detached != rhs.m_detached;
}
#endif

// ======================================================================

namespace Archive
{
	void get (Archive::ReadIterator & source, PersistableWaypoint & target)
	{
		get(source, target.m_appearanceNameCrc);
		get(source, target.m_location);
		get(source, target.m_name);
		get(source, target.m_networkId);
		get(source, target.m_color);
		get(source, target.m_active);
	}
	
//-----------------------------------------------------------------------

	void put (Archive::ByteStream & target, const PersistableWaypoint & source)
	{
		put(target, source.m_appearanceNameCrc);
		put(target, source.m_location);
		put(target, source.m_name);
		put(target, source.m_networkId);
		put(target, source.m_color);
		put(target, source.m_active);
	}

}

// ======================================================================
