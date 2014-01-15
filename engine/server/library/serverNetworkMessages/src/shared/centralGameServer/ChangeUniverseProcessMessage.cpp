//========================================================================
//
// ChangeUniverseProcessMessage.cpp - tells  CentralServer to unload all instances of
// an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ChangeUniverseProcessMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id			id of object we want to unload
 */
ChangeUniverseProcessMessage::ChangeUniverseProcessMessage(int id) :
	GameNetworkMessage("ChangeUniverseProcessMessage"),
	m_id(id)
{
	addVariable(m_id);
}	

//-----------------------------------------------------------------------

ChangeUniverseProcessMessage::ChangeUniverseProcessMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ChangeUniverseProcessMessage"),
m_id(0)
{
	addVariable(m_id);
    unpack(source);
}

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
ChangeUniverseProcessMessage::~ChangeUniverseProcessMessage()
{
}	

//-----------------------------------------------------------------------
