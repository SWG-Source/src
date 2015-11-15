// ======================================================================
//
// RetrievedItemLoadMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/RetrievedItemLoadMessage.h"

// ======================================================================

RetrievedItemLoadMessage::RetrievedItemLoadMessage(NetworkId const &ownerId, NetworkId const &itemId) :
	GameNetworkMessage("RetrievedItemLoadMessage"),
	m_ownerId(ownerId),
	m_itemId(itemId)
{
	addVariable(m_ownerId);
	addVariable(m_itemId);
}

// ----------------------------------------------------------------------

RetrievedItemLoadMessage::RetrievedItemLoadMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("RetrievedItemLoadMessage"),
	m_ownerId(),
	m_itemId()
{
	addVariable(m_ownerId);
	addVariable(m_itemId);
	unpack(source);
}

// ----------------------------------------------------------------------

RetrievedItemLoadMessage::~RetrievedItemLoadMessage()
{
}

// ======================================================================

