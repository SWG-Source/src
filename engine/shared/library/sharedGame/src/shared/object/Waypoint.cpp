// ======================================================================
//
// Waypoint.cpp
//
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/Waypoint.h"

#include "Archive/Archive.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedUtility/LocationArchive.h"
#include "sharedUtility/Location.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/WaypointData.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "UnicodeUtils.h"
#include <unordered_map>

// ======================================================================

namespace WaypointNamespace
{
	typedef std::unordered_map<NetworkId, WaypointData *> WaypointMapById;
	WaypointMapById s_waypointMapById;
	bool s_installed;

	const std::string ColorNames [static_cast<size_t>(Waypoint::NumColors)] =
	{
		"invisible",
		"blue",
		"green",
		"orange",
		"yellow",
		"purple",
		"white",
		"space",
		"small",
		"entrance"
	};

	void remove();
}

using namespace WaypointNamespace;

// ======================================================================

void releaseDataAccessor(Waypoint &target, WaypointData *data)
{
	target.releaseData(data);
}

// ======================================================================

namespace Archive
{

	// ----------------------------------------------------------------------

	void get(Archive::ReadIterator &source, WaypointDataBase &target)
	{
		get(source, target.m_appearanceNameCrc);
		get(source, target.m_location);
		get(source, target.m_name);
		NetworkId hackNetworkId;
		get(source, hackNetworkId); // preserve format of old persisted bytestreams
		get(source, target.m_color);
		get(source, target.m_active);
	}

	// ----------------------------------------------------------------------

	void put(Archive::ByteStream &target, WaypointDataBase const &source)
	{
		put(target, source.m_appearanceNameCrc);
		put(target, source.m_location);
		put(target, source.m_name);
		put(target, NetworkId::cms_invalid); // preserve format of old persisted bytestreams
		put(target, source.m_color);
		put(target, source.m_active);
	}

	// ----------------------------------------------------------------------

	void get(Archive::ReadIterator &source, Waypoint &target)
	{
		unsigned int    appearanceNameCrc;
		Location        location;
		Unicode::String name;
		NetworkId       networkId;
		uint8           color;
		bool            active;

		get(source, appearanceNameCrc);
		get(source, location);
		get(source, name);
		get(source, networkId);
		get(source, color);
		get(source, active);

		if (networkId.isValid())
		{
			WaypointData *data = 0;
			std::unordered_map<NetworkId, WaypointData *>::iterator f = s_waypointMapById.find(networkId);
			if (f == s_waypointMapById.end())
				data = new WaypointData(networkId);
			else
				data = f->second;

			releaseDataAccessor(target, data);
			target.setAppearanceNameCrc(appearanceNameCrc);
			target.setLocation(location);
			target.setName(name);
			target.setColor(color);
			target.setActive(active);
		}
		else
		{
			DEBUG_FATAL(true, ("Trying to unpack an invalid waypoint.  The caller should probably be using WaypointDataBase instead."));

			// We're unpacking a waypoint without a valid networkId, so just point it at the invalid waypoint
			releaseDataAccessor(target, &WaypointData::getInvalidWaypoint());
		}
	}

	// ----------------------------------------------------------------------

	void put(Archive::ByteStream &target, Waypoint const &source)
	{
		put(target, source.getAppearanceNameCrc());
		put(target, source.getLocation());
		put(target, source.getName());
		put(target, source.getNetworkId());
		put(target, source.getColor());
		put(target, source.isActive());
	}

	// ----------------------------------------------------------------------

}

// ======================================================================

WaypointDataBase::WaypointDataBase() :
	m_appearanceNameCrc(0),
	m_location(),
	m_name(),
	m_color(Waypoint::Blue),
	m_active(false)
{
}

// ----------------------------------------------------------------------

void WaypointDataBase::setName(Unicode::String const &name)
{
	//This magical number (250) is chosen because the waypoint datatable has VARCHAR2(512) in this column,
	//and we're assuming 2 bytes per char (which is not quite true actually) plus an extra one for null plus
	//a few extra for good measure and because nobody needs 251-character waypoint names.
	if (name.length() > 250)
	{
		WARNING(true, ("Warning: truncating waypoint name, not valid if string id: %s", Unicode::wideToUTF8(name).c_str()));
		m_name = name.substr(0, 250);
	}
	else
	{
		m_name = name;
	}
}

// ======================================================================

WaypointData::WaypointData() :
	WaypointDataBase(),
	m_changeNotification(new ChangeNotification),
	m_refCount(0),
	m_networkId()
{
}

// ----------------------------------------------------------------------

WaypointData::WaypointData(NetworkId const &networkId) :
	WaypointDataBase(),
	m_changeNotification(new ChangeNotification),
	m_refCount(0),
	m_networkId(networkId)
{
	if (m_networkId != NetworkId::cms_invalid)
		s_waypointMapById[m_networkId] = this;
}

// ----------------------------------------------------------------------

WaypointData::~WaypointData()
{
	if (this != &getInvalidWaypoint())
	{
		DEBUG_FATAL(m_refCount > 0, ("WaypointData deleted while it still has references!"));

		IGNORE_RETURN(s_waypointMapById.erase(m_networkId));
	}

	delete m_changeNotification;
	m_changeNotification = 0;
}

// ----------------------------------------------------------------------

WaypointData & WaypointData::getInvalidWaypoint()
{
	// we have a static function to return a static rather
	// than have a static member because we want to prevent
	// another static from referencing this static before
	// it has been created (i.e. the problem with ordering
	// when statics are initialized)
	static WaypointData invalidWaypoint;

	return invalidWaypoint;
}

// ======================================================================

void Waypoint::install()
{
	s_installed = true;

	// prevent anyone from deleting
	WaypointData::getInvalidWaypoint().m_refCount++;

	ExitChain::add(remove, "Waypoint::remove");
}

void WaypointNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("s_installed is false"));

	s_installed = false;
}

Waypoint::Waypoint() :
	m_data(0),
	m_loadNotification()
{
	DEBUG_FATAL(!s_installed, ("Waypoint::install not called"));

	m_data = &WaypointData::getInvalidWaypoint();
	m_data->m_refCount++;
}

// ----------------------------------------------------------------------

Waypoint::Waypoint(NetworkId const &id) :
	m_data(0),
	m_loadNotification()
{
	DEBUG_FATAL(!s_installed, ("Waypoint::install not called"));

	if (id == NetworkId::cms_invalid)
	{
		m_data = &WaypointData::getInvalidWaypoint();
	}
	else
	{
		m_data = new WaypointData(id);
	}
	m_data->m_refCount++;
}

// ----------------------------------------------------------------------

Waypoint::Waypoint(Waypoint const &rhs) :
	m_data(rhs.m_data),
	m_loadNotification()
{
	m_data->m_refCount++;
}

// ----------------------------------------------------------------------

Waypoint::~Waypoint()
{
	releaseData(0);
}

// ----------------------------------------------------------------------

Waypoint &Waypoint::operator=(Waypoint const &rhs)
{
	if (&rhs != this)
		releaseData(rhs.m_data);
	return *this;
}

// ----------------------------------------------------------------------

bool Waypoint::operator!=(Waypoint const &) const
{
	return true; // force deltas
}

// ----------------------------------------------------------------------

uint8 Waypoint::getColorIdByName(std::string const &colorName) // static
{
	for (int i = 0; i < static_cast<int>(NumColors); ++i)
		if (colorName == ColorNames[i])
			return static_cast<uint8>(i);

	return static_cast<uint8>(Invisible);
}

// ----------------------------------------------------------------------

std::string const &Waypoint::getColorNameById(uint8 id) // static
{
	if (id < static_cast<uint8>(NumColors))
		return ColorNames[static_cast<int>(id)];

	static const std::string empty;
	return empty;
}

// ----------------------------------------------------------------------

Waypoint const Waypoint::getWaypointById(NetworkId const &id) // static
{
	Waypoint result;
	std::unordered_map<NetworkId, WaypointData *>::iterator f = s_waypointMapById.find(id);
	if (f != s_waypointMapById.end())
		result.releaseData(f->second);
	return result;
}

// ----------------------------------------------------------------------

unsigned int Waypoint::getAppearanceNameCrc() const
{
	return m_data->m_appearanceNameCrc;
}

// ----------------------------------------------------------------------

Waypoint::ChangeNotification &Waypoint::getChangeNotification()
{
	return *m_data->m_changeNotification;
}

// ----------------------------------------------------------------------

Waypoint::LoadNotification &Waypoint::getLoadNotification()
{
	return m_loadNotification;
}

// ----------------------------------------------------------------------

Location const &Waypoint::getLocation() const
{
	return m_data->m_location;
}

// ----------------------------------------------------------------------

Unicode::String const &Waypoint::getName() const
{
	return m_data->m_name;
}

// ----------------------------------------------------------------------

NetworkId const &Waypoint::getNetworkId() const
{
	return m_data->m_networkId;
}

// ----------------------------------------------------------------------

uint8 Waypoint::getColor() const
{
	return m_data->m_color;
}

// ----------------------------------------------------------------------

bool Waypoint::isActive() const
{
	return m_data->m_active;
}

// ----------------------------------------------------------------------

bool Waypoint::isValid() const
{
	return m_data->m_networkId != NetworkId::cms_invalid;
}

// ----------------------------------------------------------------------

bool Waypoint::isVisible() const
{
	return m_data->m_color != Waypoint::Invisible;
}

// ----------------------------------------------------------------------

WaypointDataBase const &Waypoint::getWaypointDataBase() const
{
	NOT_NULL(m_data);
	return *m_data;
}

// ----------------------------------------------------------------------

void Waypoint::set(WaypointDataBase const &wd)
{
	DEBUG_FATAL(!isValid(), ("Programmer bug:  Attempted to change data on an invalid waypoint"));

	m_data->m_appearanceNameCrc = wd.m_appearanceNameCrc;
	m_data->m_location = wd.m_location;
	m_data->setName(wd.m_name);
	m_data->m_color = std::min(wd.m_color, static_cast<uint8>(NumColors-1));
	m_data->m_active = wd.m_active;

	m_data->m_changeNotification->emitMessage(*this);
}

// ----------------------------------------------------------------------

void Waypoint::setAppearanceNameCrc(unsigned int appearanceNameCrc)
{
	DEBUG_FATAL(!isValid(), ("Programmer bug:  Attempted to change data on an invalid waypoint"));

	m_data->m_appearanceNameCrc = appearanceNameCrc;
	m_data->m_changeNotification->emitMessage(*this);
}

// ----------------------------------------------------------------------

void Waypoint::setLocation(Location const &location)
{
	DEBUG_FATAL(!isValid(), ("Programmer bug:  Attempted to change data on an invalid waypoint"));

	m_data->m_location = location;
	m_data->m_changeNotification->emitMessage(*this);
}

// ----------------------------------------------------------------------

void Waypoint::setName(Unicode::String const &name)
{
	DEBUG_FATAL(!isValid(), ("Programmer bug:  Attempted to change data on an invalid waypoint"));

	m_data->setName(name);

	m_data->m_changeNotification->emitMessage(*this);
}

// ----------------------------------------------------------------------

void Waypoint::setColor(uint8 color)
{
	DEBUG_FATAL(!isValid(), ("Programmer bug:  Attempted to change data on an invalid waypoint"));

	m_data->m_color = std::min(color, static_cast<uint8>(NumColors-1));
	m_data->m_changeNotification->emitMessage(*this);
}

// ----------------------------------------------------------------------

void Waypoint::setActive(bool active)
{
	DEBUG_FATAL(!isValid(), ("Programmer bug:  Attempted to change data on an invalid waypoint"));

	m_data->m_active = active;
	m_data->m_changeNotification->emitMessage(*this);
}

// ----------------------------------------------------------------------

void Waypoint::releaseData(WaypointData *replacement)
{
	if (replacement != m_data)
	{
		if (replacement)
		{
			m_loadNotification.emitMessage(*replacement->m_changeNotification);
			replacement->m_refCount++;
		}

		m_data->m_refCount--;
		if (m_data->m_refCount < 1)
		{
			if (m_data != &WaypointData::getInvalidWaypoint())
				delete m_data;
		}

		m_data = replacement;
	}
}

// ======================================================================

