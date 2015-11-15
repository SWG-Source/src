//========================================================================
//
// LoadStructureMessage.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LoadStructureMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

LoadStructureMessage::LoadStructureMessage(const NetworkId & structureId, const std::string &whoRequested) :
	GameNetworkMessage("LoadStructureMessage"),
	m_structureId(structureId),
	m_whoRequested(whoRequested)
{
	AutoByteStream::addVariable(m_structureId);
	AutoByteStream::addVariable(m_whoRequested);
}

//-----------------------------------------------------------------------

LoadStructureMessage::LoadStructureMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("LoadStructureMessage"),
	m_structureId(),
	m_whoRequested()
{
	AutoByteStream::addVariable(m_structureId);
	AutoByteStream::addVariable(m_whoRequested);
	unpack(source);
}

//-----------------------------------------------------------------------

LoadStructureMessage::~LoadStructureMessage()
{
}

//-----------------------------------------------------------------------

