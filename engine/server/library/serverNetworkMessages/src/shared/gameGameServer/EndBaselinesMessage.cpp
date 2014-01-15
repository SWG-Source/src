// ======================================================================
//
// EndBaselinesMessage.cpp - tells Gameserver object baseline data has ended.
//
// Copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/EndBaselinesMessage.h"

#include "serverNetworkMessages/GameGameServerMessages.h"

// ======================================================================

EndBaselinesMessage::EndBaselinesMessage(NetworkId const &id) :
	GameNetworkMessage("EndBaselinesMessage"),
	m_id(id)
{
	addVariable(m_id);
}

// ----------------------------------------------------------------------

EndBaselinesMessage::EndBaselinesMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("EndBaselinesMessage"),
	m_id()
{
	addVariable(m_id);
	unpack(source);
}

// ----------------------------------------------------------------------

EndBaselinesMessage::~EndBaselinesMessage()
{
}

// ======================================================================

