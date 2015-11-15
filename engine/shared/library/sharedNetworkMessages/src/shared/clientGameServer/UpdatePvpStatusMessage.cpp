// ======================================================================
//
// UpdatePvpStatusMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/UpdatePvpStatusMessage.h"

// ======================================================================

const char * const UpdatePvpStatusMessage::MessageType = "UpdatePvpStatusMessage";

UpdatePvpStatusMessage::UpdatePvpStatusMessage(const NetworkId & sourceObject, uint32 flags, uint32 factionId) :
	GameNetworkMessage(MessageType),
	m_flags(flags),
	m_factionId(factionId),
	m_target(sourceObject)
{
	AutoByteStream::addVariable(m_flags);
	AutoByteStream::addVariable(m_factionId);
	AutoByteStream::addVariable(m_target);
}

// ----------------------------------------------------------------------

UpdatePvpStatusMessage::UpdatePvpStatusMessage(Archive::ReadIterator & source) :
	GameNetworkMessage(MessageType),
	m_flags(0),
	m_factionId(0),
	m_target(NetworkId::cms_invalid)
{
	AutoByteStream::addVariable(m_flags);
	AutoByteStream::addVariable(m_factionId);
	AutoByteStream::addVariable(m_target);
	unpack(source);
}

// ----------------------------------------------------------------------

UpdatePvpStatusMessage::~UpdatePvpStatusMessage()
{
}

// ======================================================================

