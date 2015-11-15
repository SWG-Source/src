// ======================================================================
//
// LocationResponse.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LocationResponse.h"

#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

LocationResponse::LocationResponse (NetworkId const & networkId, bool const valid, std::string const & locationId, float const x, float const z, float const radius) :
	GameNetworkMessage ("LocationResponse"),
	m_networkId (networkId),
	m_valid (valid),
	m_locationId (locationId),
	m_x (x),
	m_z (z),
	m_radius (radius)
{
	addVariable (m_networkId);
	addVariable (m_valid);
	addVariable (m_locationId);
	addVariable (m_x);
	addVariable (m_z);
	addVariable (m_radius);
}

// ----------------------------------------------------------------------

LocationResponse::LocationResponse (Archive::ReadIterator & source) :
	GameNetworkMessage ("LocationResponse"),
	m_networkId (),
	m_valid (false),
	m_locationId (),
	m_x (0.f),
	m_z (0.f),
	m_radius (0.f)
{
	addVariable (m_networkId);
	addVariable (m_valid);
	addVariable (m_locationId);
	addVariable (m_x);
	addVariable (m_z);
	addVariable (m_radius);
	
	unpack (source);
}

// ----------------------------------------------------------------------

LocationResponse::~LocationResponse ()
{
}

// ----------------------------------------------------------------------

NetworkId const & LocationResponse::getNetworkId () const
{
	return m_networkId.get ();
}

// ----------------------------------------------------------------------

bool LocationResponse::getValid () const
{
	return m_valid.get ();
}

// ----------------------------------------------------------------------

std::string const & LocationResponse::getLocationId () const
{
	return m_locationId.get ();
}

// ----------------------------------------------------------------------

float LocationResponse::getX () const
{
	return m_x.get ();
}

// ----------------------------------------------------------------------

float LocationResponse::getZ () const
{
	return m_z.get ();
}

// ----------------------------------------------------------------------

float LocationResponse::getRadius () const
{
	return m_radius.get ();
}

// ======================================================================
