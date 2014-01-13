// ======================================================================
//
// FactionRequestMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/FactionRequestMessage.h"

// ======================================================================

FactionRequestMessage::FactionRequestMessage()
: GameNetworkMessage("FactionRequestMessage")
{
}

//-----------------------------------------------------------------------

FactionRequestMessage::FactionRequestMessage(Archive::ReadIterator & source)
: GameNetworkMessage("FactionRequestMessage")
{
	UNREF(source);
}

// ----------------------------------------------------------------------

FactionRequestMessage::~FactionRequestMessage()
{
}

// ======================================================================
