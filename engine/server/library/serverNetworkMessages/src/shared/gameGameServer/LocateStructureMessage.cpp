//========================================================================
//
// LocateStructureMessage.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LocateStructureMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

LocateStructureMessage::LocateStructureMessage(const NetworkId & structureId, const int x, const int z, const std::string &sceneId, const std::string &whoRequested):
	GameNetworkMessage("LocateStructureMessage"),
	m_structureId(structureId),
	m_x(x),
	m_z(z),
	m_sceneId(sceneId),
	m_whoRequested(whoRequested)
{
	AutoByteStream::addVariable(m_structureId);
	AutoByteStream::addVariable(m_x);
	AutoByteStream::addVariable(m_z);
	AutoByteStream::addVariable(m_sceneId);
	AutoByteStream::addVariable(m_whoRequested);
}

//-----------------------------------------------------------------------

LocateStructureMessage::LocateStructureMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("LocateStructureMessage"),
	m_structureId(),
	m_x(),
	m_z(),
	m_sceneId(),
	m_whoRequested()
{
	AutoByteStream::addVariable(m_structureId);
	AutoByteStream::addVariable(m_x);
	AutoByteStream::addVariable(m_z);
	AutoByteStream::addVariable(m_sceneId);
	AutoByteStream::addVariable(m_whoRequested);
	unpack(source);
}

//-----------------------------------------------------------------------

LocateStructureMessage::~LocateStructureMessage()
{
}

//-----------------------------------------------------------------------

