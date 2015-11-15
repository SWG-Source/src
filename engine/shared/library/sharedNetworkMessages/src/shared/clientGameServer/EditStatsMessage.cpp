// ======================================================================
//
// EditStatsMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/EditStatsMessage.h"

// ======================================================================

const char * const EditStatsMessage::MessageType = "EditStatsMessage";

//----------------------------------------------------------------------

EditStatsMessage::EditStatsMessage(const NetworkId & target) :
GameNetworkMessage(MessageType),
m_target(target)
{
	addVariable(m_target);
}

//----------------------------------------------------------------------

EditStatsMessage::EditStatsMessage(Archive::ReadIterator &source) :
GameNetworkMessage(MessageType),
m_target()
{
	addVariable(m_target);
	unpack(source);
}

//----------------------------------------------------------------------
