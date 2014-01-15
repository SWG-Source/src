// ======================================================================
//
// EnableNewJediTrackingMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/EnableNewJediTrackingMessage.h"

// ======================================================================

EnableNewJediTrackingMessage::EnableNewJediTrackingMessage(bool enableTracking) :
	GameNetworkMessage("EnableNewJediTrackingMessage"),
	m_enableTracking(enableTracking)
{
	addVariable(m_enableTracking);
}

// ----------------------------------------------------------------------

EnableNewJediTrackingMessage::EnableNewJediTrackingMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("EnableNewJediTrackingMessage"),
	m_enableTracking()
{
	addVariable(m_enableTracking);
	unpack(source);
}

// ----------------------------------------------------------------------

EnableNewJediTrackingMessage::~EnableNewJediTrackingMessage()
{
}

// ======================================================================

