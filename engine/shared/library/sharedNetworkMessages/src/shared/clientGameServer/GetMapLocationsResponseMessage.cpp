// ======================================================================
//
// GetMapLocationsResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GetMapLocationsResponseMessage.h"

// ======================================================================

const char * const GetMapLocationsResponseMessage::MessageType = "GetMapLocationsResponseMessage";

//----------------------------------------------------------------------

GetMapLocationsResponseMessage::GetMapLocationsResponseMessage(const std::string &planetName, 
															   const MapLocationVector & locStatic, 
															   const MapLocationVector & locDynamic, 
															   const MapLocationVector & locPersist, 
															   int versionStatic, 
															   int versionDynamic, 
															   int versionPersist) :
GameNetworkMessage     (MessageType),
m_planetName           (planetName),
m_mapLocationsStatic   (),
m_mapLocationsDynamic  (),
m_mapLocationsPersist  (),
m_versionStatic        (versionStatic),
m_versionDynamic       (versionDynamic),
m_versionPersist       (versionPersist)
{
	m_mapLocationsStatic.set   (locStatic);
	m_mapLocationsDynamic.set  (locDynamic);
	m_mapLocationsPersist.set  (locPersist);
	
	AutoByteStream::addVariable(m_planetName);
	AutoByteStream::addVariable(m_mapLocationsStatic);
	AutoByteStream::addVariable(m_mapLocationsDynamic);
	AutoByteStream::addVariable(m_mapLocationsPersist);
	AutoByteStream::addVariable(m_versionStatic);
	AutoByteStream::addVariable(m_versionDynamic);
	AutoByteStream::addVariable(m_versionPersist);
}

// ======================================================================

GetMapLocationsResponseMessage::GetMapLocationsResponseMessage(Archive::ReadIterator &source) :
GameNetworkMessage(MessageType),
m_planetName           (),
m_mapLocationsStatic   (),
m_mapLocationsDynamic  (),
m_mapLocationsPersist  (),
m_versionStatic        (),
m_versionDynamic       (),
m_versionPersist       ()
{
	AutoByteStream::addVariable(m_planetName);
	AutoByteStream::addVariable(m_mapLocationsStatic);
	AutoByteStream::addVariable(m_mapLocationsDynamic);
	AutoByteStream::addVariable(m_mapLocationsPersist);
	AutoByteStream::addVariable(m_versionStatic);
	AutoByteStream::addVariable(m_versionDynamic);
	AutoByteStream::addVariable(m_versionPersist);
	
	unpack(source);
}

// ======================================================================

GetMapLocationsResponseMessage::~GetMapLocationsResponseMessage()
{
}

//----------------------------------------------------------------------
