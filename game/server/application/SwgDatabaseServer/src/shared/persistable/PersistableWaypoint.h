// ======================================================================
//
// PersistableWaypoint.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PersistableWaypoint_H
#define INCLUDED_PersistableWaypoint_H

// ======================================================================

#include "sharedUtility/Location.h"
#include "Unicode.h"

// ======================================================================

class PersistableWaypoint
{
  public:
	unsigned int                m_appearanceNameCrc;
	Location                    m_location;
	Unicode::String             m_name;
	NetworkId                   m_networkId;
	unsigned char               m_color;
	bool                        m_active;
	bool                        m_detached;

  public:
	PersistableWaypoint();
#ifdef WIN32
	bool operator!=(const PersistableWaypoint& rhs);
#endif
};

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get (Archive::ReadIterator & source, PersistableWaypoint & target);
	void put (ByteStream & target, const PersistableWaypoint & source);
}

// ======================================================================

#endif
