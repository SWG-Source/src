// ==================================================================
//
// EnterTicketPurchaseModeMessage.cpp
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/EnterTicketPurchaseModeMessage.h"

// ==================================================================

const char* const EnterTicketPurchaseModeMessage::cms_name = "EnterTicketPurchaseModeMessage";

// ==================================================================

EnterTicketPurchaseModeMessage::EnterTicketPurchaseModeMessage (const std::string& planetName, const std::string& travelPointName, bool instantTravel) :
	GameNetworkMessage ("EnterTicketPurchaseModeMessage"),
	m_planetName (planetName),
	m_travelPointName (travelPointName),
	m_instantTravel (instantTravel)
{
	addVariable (m_planetName);
	addVariable (m_travelPointName);
	addVariable (m_instantTravel);
}

// ------------------------------------------------------------------

EnterTicketPurchaseModeMessage::EnterTicketPurchaseModeMessage (Archive::ReadIterator& source) :
	GameNetworkMessage ("EnterTicketPurchaseModeMessage"),
	m_planetName (),
	m_travelPointName (),
	m_instantTravel ()
{
	addVariable (m_planetName);
	addVariable (m_travelPointName);
	addVariable (m_instantTravel);
	unpack (source);
}

// ------------------------------------------------------------------

EnterTicketPurchaseModeMessage::~EnterTicketPurchaseModeMessage ()
{
}

// ------------------------------------------------------------------

const std::string& EnterTicketPurchaseModeMessage::getPlanetName () const
{
	return m_planetName.get ();
}

// ------------------------------------------------------------------

const std::string& EnterTicketPurchaseModeMessage::getTravelPointName () const
{
	return m_travelPointName.get ();
}

// ------------------------------------------------------------------

bool EnterTicketPurchaseModeMessage::getInstantTravel () const
{
	return m_instantTravel.get ();
}

// ==================================================================
