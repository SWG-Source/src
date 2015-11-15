// ======================================================================
//
// AddMapLocationResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/AddMapLocationResponseMessage.h"

// ======================================================================

const char * const AddMapLocationResponseMessage::MessageType = "AddMapLocationResponseMessage";

//----------------------------------------------------------------------

AddMapLocationResponseMessage::AddMapLocationResponseMessage(const NetworkId &locationId) :
	GameNetworkMessage(MessageType),
	m_locationId(locationId)
{
	AutoByteStream::addVariable(m_locationId);
}

// ======================================================================

AddMapLocationResponseMessage::AddMapLocationResponseMessage(Archive::ReadIterator &source) :
	GameNetworkMessage(MessageType),
	m_locationId()
{
	AutoByteStream::addVariable(m_locationId);
	unpack(source);
}

// ======================================================================

AddMapLocationResponseMessage::~AddMapLocationResponseMessage()
{
}
