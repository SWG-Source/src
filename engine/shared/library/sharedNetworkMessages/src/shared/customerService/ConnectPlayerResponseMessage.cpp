// ======================================================================
//
// ConnectPlayerResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ConnectPlayerResponseMessage.h"

// ======================================================================

ConnectPlayerResponseMessage::ConnectPlayerResponseMessage(int32 result) :
	GameNetworkMessage("ConnectPlayerResponseMessage"),
	m_result(result)
{
	AutoByteStream::addVariable(m_result);
}

// ======================================================================

ConnectPlayerResponseMessage::ConnectPlayerResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("ConnectPlayerResponseMessage"),
	m_result()
{
	AutoByteStream::addVariable(m_result);
	unpack(source);
}

// ======================================================================

ConnectPlayerResponseMessage::~ConnectPlayerResponseMessage()
{
}
