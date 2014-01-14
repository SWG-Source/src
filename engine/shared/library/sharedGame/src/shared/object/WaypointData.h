//======================================================================
//
// WaypointData.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_WaypointData_H
#define INCLUDED_WaypointData_H

//======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedUtility/Location.h"
#include "Unicode.h"

class Waypoint;

namespace MessageDispatch
{
	template <typename MessageType, typename IdentifierType> class Transceiver;
	class Callback;
}

//----------------------------------------------------------------------

class WaypointDataBase
{
public:
	WaypointDataBase();
	virtual ~WaypointDataBase() {};

	void setName(Unicode::String const &name);

	unsigned int                m_appearanceNameCrc;
	Location                    m_location;
	Unicode::String             m_name;
	uint8                       m_color;
	bool                        m_active;
};

//----------------------------------------------------------------------

namespace Archive
{
	class ByteStream;
	class ReadIterator;

	void get(Archive::ReadIterator &source, WaypointDataBase &target);
	void put(Archive::ByteStream &target, WaypointDataBase const &source);
};

//----------------------------------------------------------------------

class WaypointData: public WaypointDataBase
{
public:
	typedef MessageDispatch::Transceiver<const Waypoint &, void *> ChangeNotification;

	WaypointData  ();
	WaypointData  (const NetworkId & networkId);
	~WaypointData ();
	WaypointData  (const WaypointData &);
	WaypointData & operator = (const WaypointData &);

	static WaypointData &       getInvalidWaypoint();
	
	ChangeNotification *        m_changeNotification;
	int                         m_refCount;
	NetworkId                   m_networkId;
};

//======================================================================

#endif
