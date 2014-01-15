//========================================================================
//
// SetPlanetServerMessage.cpp - tells Centralserver we want all the objects in a chunk.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/SetPlanetServerMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

SetPlanetServerMessage::SetPlanetServerMessage(const std::string &address, uint16 port) :
	GameNetworkMessage("SetPlanetServerMessage"),
	m_address(address),
	m_port(port)
{
	addVariable(m_address);
	addVariable(m_port);
}

//-----------------------------------------------------------------------

SetPlanetServerMessage::SetPlanetServerMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("SetPlanetServerMessage"),
		m_address(),
		m_port()
{
	addVariable(m_address);
	addVariable(m_port);
	
	unpack(source);
}

//-----------------------------------------------------------------------

SetPlanetServerMessage::~SetPlanetServerMessage()
{
}

// ======================================================================
