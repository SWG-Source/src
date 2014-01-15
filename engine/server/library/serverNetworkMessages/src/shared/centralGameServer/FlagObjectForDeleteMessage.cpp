//========================================================================
//
// FlagObjectForDeleteMessage.cpp - tells  CentralServer to unload all instances of
// an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/FlagObjectForDeleteMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param id			id of object we want to unload
 */
FlagObjectForDeleteMessage::FlagObjectForDeleteMessage(const NetworkId& id, int reason, bool immediate, bool demandLoadedContainer, bool cascadeReason) :
	GameNetworkMessage("FlagObjectForDeleteMessage"),
	m_id(id),
	m_reason(reason),
	m_immediate(immediate),
	m_demandLoadedContainer(demandLoadedContainer),
	m_cascadeReason(cascadeReason)
{
	addVariable(m_id);
	addVariable(m_reason);
	addVariable(m_immediate);
	addVariable(m_demandLoadedContainer);
	addVariable(m_cascadeReason);

	DEBUG_FATAL(cascadeReason && !demandLoadedContainer,("Programmer bug:  using cascadeReason with FlagObjectForDeleteMessage is only supported if the object is a demand-loaded container."));
}	

//-----------------------------------------------------------------------

FlagObjectForDeleteMessage::FlagObjectForDeleteMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("FlagObjectForDeleteMessage"),
		m_id(NetworkId::cms_invalid),
		m_reason(0),
		m_immediate(false),
		m_cascadeReason(false)
{
	addVariable(m_id);
	addVariable(m_reason);
	addVariable(m_immediate);
	addVariable(m_demandLoadedContainer);
	addVariable(m_cascadeReason);
    unpack(source);
}

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
FlagObjectForDeleteMessage::~FlagObjectForDeleteMessage()
{
}	

//-----------------------------------------------------------------------
