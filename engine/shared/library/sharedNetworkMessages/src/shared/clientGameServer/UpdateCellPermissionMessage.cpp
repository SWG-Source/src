// ======================================================================
//
// UpdateCellPermissionMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/UpdateCellPermissionMessage.h"

// ======================================================================

UpdateCellPermissionMessage::UpdateCellPermissionMessage(const NetworkId & sourceObject, bool allowed) : 
	GameNetworkMessage("UpdateCellPermissionMessage"),
	m_allowed(allowed),
	m_target(sourceObject)
{
	AutoByteStream::addVariable(m_allowed);
	AutoByteStream::addVariable(m_target);
}

// ----------------------------------------------------------------------

UpdateCellPermissionMessage::UpdateCellPermissionMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("UpdateCellPermissionMessage"),
	m_allowed(),
	m_target(NetworkId::cms_invalid)
{
	AutoByteStream::addVariable(m_allowed);
	AutoByteStream::addVariable(m_target);
	unpack(source);
}

// ----------------------------------------------------------------------

UpdateCellPermissionMessage::~UpdateCellPermissionMessage()
{
}

// ======================================================================

