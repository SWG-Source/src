// ======================================================================
//
// LocationRequest.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LocationRequest.h"

#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

LocationRequest::LocationRequest (uint32 const processId, NetworkId const & networkId, std::string const & locationId, float const searchX, float const searchZ, float const searchRadius, float const locationReservationRadius, bool const checkWater, bool const checkSlope) :
	GameNetworkMessage ("LocationRequest"),
	m_processId (processId),
	m_networkId (networkId),
	m_locationId (locationId),
	m_searchX (searchX),
	m_searchZ (searchZ),
	m_searchRadius (searchRadius),
	m_locationReservationRadius (locationReservationRadius),
	m_checkWater (checkWater),
	m_checkSlope (checkSlope)
{
	addVariable (m_processId);
	addVariable (m_networkId);
	addVariable (m_locationId);
	addVariable (m_searchX);
	addVariable (m_searchZ);
	addVariable (m_searchRadius);
	addVariable (m_locationReservationRadius);
	addVariable (m_checkWater);
	addVariable (m_checkSlope);
}

// ----------------------------------------------------------------------

LocationRequest::LocationRequest (Archive::ReadIterator & source) :
	GameNetworkMessage ("LocationRequest"),
	m_processId (0),
	m_networkId (),
	m_locationId (),
	m_searchX (0.f),
	m_searchZ (0.f),
	m_searchRadius (0.f),
	m_locationReservationRadius (0.f),
	m_checkWater (false),
	m_checkSlope (false)
{
	addVariable (m_processId);
	addVariable (m_networkId);
	addVariable (m_locationId);
	addVariable (m_searchX);
	addVariable (m_searchZ);
	addVariable (m_searchRadius);
	addVariable (m_locationReservationRadius);
	addVariable (m_checkWater);
	addVariable (m_checkSlope);
	
	unpack (source);
}

// ----------------------------------------------------------------------

LocationRequest::~LocationRequest ()
{
}

// ----------------------------------------------------------------------

uint32 LocationRequest::getProcessId () const
{
	return m_processId.get ();
}

// ----------------------------------------------------------------------

NetworkId const & LocationRequest::getNetworkId () const
{
	return m_networkId.get ();
}

// ----------------------------------------------------------------------

std::string const & LocationRequest::getLocationId () const
{
	return m_locationId.get ();
}

// ----------------------------------------------------------------------

float LocationRequest::getSearchX () const
{
	return m_searchX.get ();
}

// ----------------------------------------------------------------------

float LocationRequest::getSearchZ () const
{
	return m_searchZ.get ();
}

// ----------------------------------------------------------------------

float LocationRequest::getSearchRadius () const
{
	return m_searchRadius.get ();
}

// ----------------------------------------------------------------------

float LocationRequest::getLocationReservationRadius () const
{
	return m_locationReservationRadius.get ();
}

// ----------------------------------------------------------------------

bool LocationRequest::getCheckWater () const
{
	return m_checkWater.get ();
}

// ----------------------------------------------------------------------

bool LocationRequest::getCheckSlope () const
{
	return m_checkSlope.get ();
}

// ======================================================================
