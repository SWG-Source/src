// ==================================================================
//
// PlanetTravelPointListRequest.cpp
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/PlanetTravelPointListRequest.h"

// ==================================================================

const char* const PlanetTravelPointListRequest::cms_name = "PlanetTravelPointListRequest";

// ==================================================================

PlanetTravelPointListRequest::PlanetTravelPointListRequest (const NetworkId& networkId, const std::string& planetName, int sequenceId) :
GameNetworkMessage (cms_name),
m_networkId        (networkId),
m_planetName       (planetName),
m_sequenceId       (sequenceId)
{
	addVariable (m_networkId);
	addVariable (m_planetName);
//	addVariable (m_sequenceId);
}

// ------------------------------------------------------------------

PlanetTravelPointListRequest::PlanetTravelPointListRequest (Archive::ReadIterator& source) :
GameNetworkMessage (cms_name),
m_networkId  (),
m_planetName (),
m_sequenceId (0)
{
	addVariable (m_networkId);
	addVariable (m_planetName);
//	addVariable (m_sequenceId);
	unpack (source);
}

// ------------------------------------------------------------------

PlanetTravelPointListRequest::~PlanetTravelPointListRequest ()
{
}

// ------------------------------------------------------------------

const NetworkId& PlanetTravelPointListRequest::getNetworkId () const
{
	return m_networkId.get ();
}

// ------------------------------------------------------------------

const std::string& PlanetTravelPointListRequest::getPlanetName () const
{
	return m_planetName.get ();
}

//----------------------------------------------------------------------

int PlanetTravelPointListRequest::getSequenceId () const
{
	return m_sequenceId.get ();
}

// ==================================================================
