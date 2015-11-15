// ======================================================================
//
// FactionalSystemMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/FactionalSystemMessage.h"

#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

FactionalSystemMessage::FactionalSystemMessage(Unicode::String const & prosePackage, Vector const & location, float radius, bool notifyImperial, bool notifyRebel) :
	GameNetworkMessage("FactionalSystemMessage"),
	m_prosePackage(prosePackage),
	m_location(location),
	m_radius(radius),
	m_notifyImperial(notifyImperial),
	m_notifyRebel(notifyRebel)
{
	addVariable(m_prosePackage);
	addVariable(m_location);
	addVariable(m_radius);
	addVariable(m_notifyImperial);
	addVariable(m_notifyRebel);
}

// ----------------------------------------------------------------------

FactionalSystemMessage::FactionalSystemMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("FactionalSystemMessage"),
	m_prosePackage(),
	m_location(),
	m_radius(0.0f),
	m_notifyImperial(false),
	m_notifyRebel(false)
{
	addVariable(m_prosePackage);
	addVariable(m_location);
	addVariable(m_radius);
	addVariable(m_notifyImperial);
	addVariable(m_notifyRebel);

	unpack(source);
}

//-----------------------------------------------------------------------

FactionalSystemMessage::~FactionalSystemMessage()
{
}

// ======================================================================
