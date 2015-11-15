// ======================================================================
//
// LoadObjectMessage.cpp - tells Gameserver another Gameserver wants to load a
// proxy from an authoritative object.
//
// Copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LoadObjectMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

/**
 * Class constructor.
 *
 * @param id			id of object we want to load
 * @param process		process id of Gameserver that wants to load the object
 */
LoadObjectMessage::LoadObjectMessage(NetworkId const &id, uint32 process) :
	GameNetworkMessage("LoadObjectMessage"),
	m_id(id),
	m_process(process)
{
	addVariable(m_id);
	addVariable(m_process);
}

// ----------------------------------------------------------------------

LoadObjectMessage::LoadObjectMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("LoadObjectMessage"),
	m_id(),
	m_process()
{
	addVariable(m_id);
	addVariable(m_process);
	unpack(source);
}

// ----------------------------------------------------------------------

/**
 * Class destructor.
 */
LoadObjectMessage::~LoadObjectMessage()
{
}

// ======================================================================


