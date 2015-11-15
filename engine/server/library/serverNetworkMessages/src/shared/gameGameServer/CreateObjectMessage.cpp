// ======================================================================
//
// CreateObjectMessage.cpp - tells Gameserver to create a new object.
//
// Copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CreateObjectMessage.h"

// ======================================================================

CreateObjectByCrcMessage::CreateObjectByCrcMessage(NetworkId const &id, uint32 crc, Tag objectType, bool createAuthoritative, NetworkId const &container) :
	GameNetworkMessage("CreateObjectByCrcMessage"),
	m_id(id),
	m_crc(crc),
	m_objType(objectType),
	m_createAuthoritative(createAuthoritative),
	m_container(container)
{
	addVariable(m_id);
	addVariable(m_crc);
	addVariable(m_objType);
	addVariable(m_createAuthoritative);
	addVariable(m_container);
}

// ----------------------------------------------------------------------

CreateObjectByCrcMessage::CreateObjectByCrcMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("CreateObjectByCrcMessage"),
	m_id(),
	m_crc(),
	m_objType(),
	m_createAuthoritative(),
	m_container()
{
	addVariable(m_id);
	addVariable(m_crc);
	addVariable(m_objType);
	addVariable(m_createAuthoritative);
	addVariable(m_container);
	unpack(source);
}

// ----------------------------------------------------------------------

CreateObjectByCrcMessage::~CreateObjectByCrcMessage()
{
}

// ======================================================================

