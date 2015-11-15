// ======================================================================
//
// DisconnectPlayerResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/DisconnectPlayerResponseMessage.h"

// ======================================================================

DisconnectPlayerResponseMessage::DisconnectPlayerResponseMessage(int32 result) :
	GameNetworkMessage("DisconnectPlayerResponseMessage"),
	m_result(result)
{
	AutoByteStream::addVariable(m_result);
}

// ======================================================================

DisconnectPlayerResponseMessage::DisconnectPlayerResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("DisconnectPlayerResponseMessage"),
	m_result()
{
	AutoByteStream::addVariable(m_result);
	unpack(source);
}

// ======================================================================

DisconnectPlayerResponseMessage::~DisconnectPlayerResponseMessage()
{
}
