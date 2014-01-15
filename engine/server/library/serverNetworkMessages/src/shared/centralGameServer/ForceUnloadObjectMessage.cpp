//========================================================================
//
// ForceUnloadObjectMessage.cpp - tells  CentralServer to unload all instances of
// an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ForceUnloadObjectMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id			id of object we want to unload
 */
ForceUnloadObjectMessage::ForceUnloadObjectMessage(const NetworkId& id, bool permaDelete) :
	GameNetworkMessage("ForceUnloadObjectMessage"),
	m_id(id),
        m_permaDelete(permaDelete)
{
	addVariable(m_id);
        addVariable(m_permaDelete);
}	// ForceUnloadObjectMessage::ForceUnloadObjectMessage

//-----------------------------------------------------------------------

ForceUnloadObjectMessage::ForceUnloadObjectMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ForceUnloadObjectMessage"),
m_id(NetworkId::cms_invalid),
m_permaDelete(false)
{
	addVariable(m_id);
        addVariable(m_permaDelete);
        unpack(source);
}	// ForceUnloadObjectMessage::ForceUnloadObjectMessage

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
ForceUnloadObjectMessage::~ForceUnloadObjectMessage()
{
}	// ForceUnloadObjectMessage::~ForceUnloadObjectMessage

//-----------------------------------------------------------------------
