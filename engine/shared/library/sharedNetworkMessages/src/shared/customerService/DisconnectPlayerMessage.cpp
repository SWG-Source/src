// ======================================================================
//
// DisconnectPlayerMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/DisconnectPlayerMessage.h"

// ======================================================================

DisconnectPlayerMessage::DisconnectPlayerMessage() :
	GameNetworkMessage("DisconnectPlayerMessage")
{
}

// ======================================================================

DisconnectPlayerMessage::DisconnectPlayerMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("DisconnectPlayerMessage")
{
	unpack(source);
}

// ======================================================================

DisconnectPlayerMessage::~DisconnectPlayerMessage()
{
}
