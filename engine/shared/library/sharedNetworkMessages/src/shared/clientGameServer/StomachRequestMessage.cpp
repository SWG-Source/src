// ======================================================================
//
// StomachRequestMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/StomachRequestMessage.h"

// ======================================================================

StomachRequestMessage::StomachRequestMessage()
: GameNetworkMessage("StomachRequestMessage")
{
}

//-----------------------------------------------------------------------

StomachRequestMessage::StomachRequestMessage(Archive::ReadIterator & source)
: GameNetworkMessage("StomachRequestMessage")
{
	UNREF(source);
}

// ----------------------------------------------------------------------

StomachRequestMessage::~StomachRequestMessage()
{
}

// ======================================================================
