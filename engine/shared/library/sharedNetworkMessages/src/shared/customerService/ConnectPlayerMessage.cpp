// ======================================================================
//
// ConnectPlayerMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ConnectPlayerMessage.h"

// ======================================================================

ConnectPlayerMessage::ConnectPlayerMessage() :
	GameNetworkMessage("ConnectPlayerMessage"),
	m_stationId(0)
{
	addVariable(m_stationId);
}

// ======================================================================

ConnectPlayerMessage::ConnectPlayerMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("ConnectPlayerMessage"),
	m_stationId(0)
{
	addVariable(m_stationId);
	unpack(source);
}

// ======================================================================

ConnectPlayerMessage::~ConnectPlayerMessage()
{
}
