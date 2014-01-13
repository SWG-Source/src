// ======================================================================
//
// GetMapLocationsMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GetMapLocationsMessage.h"

// ======================================================================

const char * const GetMapLocationsMessage::MessageType = "GetMapLocationsMessage";

//----------------------------------------------------------------------

GetMapLocationsMessage::GetMapLocationsMessage(const std::string &planetName, int cacheVersionStatic, int cacheVersionDynamic, int cacheVersionPersist) :
GameNetworkMessage    (MessageType),
m_planetName          (planetName),
m_cacheVersionStatic  (cacheVersionStatic),
m_cacheVersionDynamic (cacheVersionDynamic),
m_cacheVersionPersist (cacheVersionPersist)
{
	AutoByteStream::addVariable(m_planetName);
	AutoByteStream::addVariable(m_cacheVersionStatic);
	AutoByteStream::addVariable(m_cacheVersionDynamic);
	AutoByteStream::addVariable(m_cacheVersionPersist);
}

// ======================================================================

GetMapLocationsMessage::GetMapLocationsMessage(Archive::ReadIterator &source) :
GameNetworkMessage    (MessageType),
m_planetName          (),
m_cacheVersionStatic  (0),
m_cacheVersionDynamic (0),
m_cacheVersionPersist (0)
{
	AutoByteStream::addVariable(m_planetName);
	AutoByteStream::addVariable(m_cacheVersionStatic);
	AutoByteStream::addVariable(m_cacheVersionDynamic);
	AutoByteStream::addVariable(m_cacheVersionPersist);
	unpack(source);
}

// ======================================================================

GetMapLocationsMessage::~GetMapLocationsMessage()
{
}

//----------------------------------------------------------------------

