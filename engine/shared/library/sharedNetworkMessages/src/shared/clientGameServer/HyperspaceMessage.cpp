// ======================================================================
//
// HyperspaceMessage.cpp
//
// Copyright 2004 Sony Online Entertainment
// tford
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/HyperspaceMessage.h"

// ======================================================================

HyperspaceMessage::HyperspaceMessage(NetworkId const & ownerId)
: GameNetworkMessage("HyperspaceMessage")
, m_ownerId(ownerId)
{
	addVariable(m_ownerId);
}

// ----------------------------------------------------------------------

HyperspaceMessage::HyperspaceMessage(Archive::ReadIterator & source)
: GameNetworkMessage("HyperspaceMessage")
, m_ownerId()
{
	addVariable(m_ownerId);
	unpack(source);
}

// ----------------------------------------------------------------------

NetworkId const & HyperspaceMessage::getOwnerId() const
{
	return m_ownerId.get();
}

// ======================================================================

