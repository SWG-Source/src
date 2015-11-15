//========================================================================
//
// CentralPingMessage.cpp - tells Centralserver we want all the objects in a chunk.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CentralPingMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

CentralPingMessage::CentralPingMessage  () :
		GameNetworkMessage("CentralPingMessage")
{
}

//-----------------------------------------------------------------------

CentralPingMessage::CentralPingMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("CentralPingMessage")
{
	unpack(source);
}

//-----------------------------------------------------------------------

CentralPingMessage::~CentralPingMessage()
{
}

// ======================================================================
