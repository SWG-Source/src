// ======================================================================
//
// Waypoint.h
//
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall
// ======================================================================

#ifndef	_INCLUDED_Waypoint_H
#define	_INCLUDED_Waypoint_H

// ======================================================================

#include "sharedMessageDispatch/Transceiver.h"

namespace MessageDispatch
{
	template <typename MessageType, typename IdentifierType> class Transceiver;
}

class WaypointData;
class WaypointDataBase;
class Location;
class NetworkId;

// ======================================================================

class Waypoint
{
public:
	enum
	{
		Invisible,
		Blue,
		Green,
		Orange,
		Yellow,
		Purple,
		White,
		Space,
		Small,
		Entrance,
		NumColors
	};

	typedef MessageDispatch::Transceiver<Waypoint const &,     void *> ChangeNotification;
	typedef MessageDispatch::Transceiver<ChangeNotification &, void *> LoadNotification;

	static void install();

	Waypoint();
	Waypoint(Waypoint const &source);
	Waypoint(NetworkId const &);
	~Waypoint();

	Waypoint &operator=(Waypoint const &rhs);
	bool operator!=(Waypoint const &rhs) const;

	static uint8                  getColorIdByName        (std::string const &colorName);
	static std::string const &    getColorNameById        (uint8 color);
	static Waypoint const         getWaypointById         (NetworkId const &id);

	unsigned int                  getAppearanceNameCrc    () const;
	ChangeNotification &          getChangeNotification   ();
	LoadNotification &            getLoadNotification     ();
	Location const &              getLocation             () const;
	Unicode::String const &       getName                 () const;
	NetworkId const &             getNetworkId            () const;
	uint8                         getColor                () const;
	bool                          isActive                () const;
	bool                          isValid                 () const;
	bool                          isVisible               () const;
	WaypointDataBase const &      getWaypointDataBase     () const;

	void                          set                     (WaypointDataBase const &wd);
	void                          setAppearanceNameCrc    (unsigned int crc);
	void                          setLocation             (Location const &location);
	void                          setName                 (Unicode::String const &name);
	void                          setColor                (uint8 color);
	void                          setActive               (bool active);

private:
	friend void releaseDataAccessor(Waypoint &target, WaypointData *data);
	void releaseData(WaypointData *replacementData);

	WaypointData *        m_data;
	LoadNotification      m_loadNotification;
};

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(Archive::ReadIterator &source, Waypoint &target);
	void put(Archive::ByteStream &target, Waypoint const &source);
}

// ======================================================================

#endif	// _INCLUDED_Waypoint_H

