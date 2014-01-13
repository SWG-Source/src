// ======================================================================
//
// IsVendorOwnerMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/IsVendorOwnerMessage.h"

// ======================================================================

IsVendorOwnerMessage::IsVendorOwnerMessage(NetworkId containerId) :
	GameNetworkMessage("IsVendorOwnerMessage"),
	m_containerId(containerId)
{
	AutoByteStream::addVariable(m_containerId);
}

// ======================================================================

IsVendorOwnerMessage::IsVendorOwnerMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("IsVendorOwnerMessage"),
	m_containerId()
{
	AutoByteStream::addVariable(m_containerId);
	unpack(source);
}

// ======================================================================

IsVendorOwnerMessage::~IsVendorOwnerMessage()
{
}
