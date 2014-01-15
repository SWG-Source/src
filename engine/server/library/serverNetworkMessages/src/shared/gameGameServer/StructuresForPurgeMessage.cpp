// ======================================================================
//
// StructuresForPurgeMessage.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/StructuresForPurgeMessage.h"

// ======================================================================

StructuresForPurgeMessage::StructuresForPurgeMessage(StationId stationId, std::vector<std::pair<NetworkId, NetworkId> > const & structures, std::vector<std::pair<NetworkId, std::pair<NetworkId, Unicode::String> > > const & vendors, bool warnOnly) :
		GameNetworkMessage("StructuresForPurgeMessage"),
		m_stationId(stationId),
		m_structures(),
		m_vendors(),
		m_warnOnly(warnOnly)
{
	m_structures.set(structures);
	m_vendors.set(vendors);
	addVariable(m_stationId);
	addVariable(m_structures);
	addVariable(m_vendors);
	addVariable(m_warnOnly);
}

//-----------------------------------------------------------------------

StructuresForPurgeMessage::StructuresForPurgeMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("StructuresForPurgeMessage"),
		m_stationId(),
		m_structures(),
		m_vendors(),
		m_warnOnly()
{
	addVariable(m_stationId);
	addVariable(m_structures);
	addVariable(m_vendors);
	addVariable(m_warnOnly);
	unpack(source); //lint !e1506 // virtual in constructor
}

//-----------------------------------------------------------------------

StructuresForPurgeMessage::~StructuresForPurgeMessage()
{
}

// ----------------------------------------------------------------------

StationId StructuresForPurgeMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

std::vector<std::pair<NetworkId, NetworkId> > const & StructuresForPurgeMessage::getStructures() const
{
	return m_structures.get();
}

// ----------------------------------------------------------------------

std::vector<std::pair<NetworkId, std::pair<NetworkId, Unicode::String> > > const & StructuresForPurgeMessage::getVendors() const
{
	return m_vendors.get();
}

// ----------------------------------------------------------------------

bool StructuresForPurgeMessage::getWarnOnly() const
{
	return m_warnOnly.get();
}

// ======================================================================
