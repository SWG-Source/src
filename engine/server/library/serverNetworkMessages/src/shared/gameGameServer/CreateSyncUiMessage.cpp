//========================================================================
//
// CreateSyncUiMessage.cpp - tells Gameserver to create a new object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CreateSyncUiMessage.h"

#include "serverNetworkMessages/GameGameServerMessages.h"

//========================================================================

/**
 * Class constructor.
 *
 * @param id           id of the object
 */
CreateSyncUiMessage::CreateSyncUiMessage(const NetworkId& id, const std::vector<NetworkId> & clients) :
	GameNetworkMessage("CreateSyncUiMessage"),
	m_id(id),
	m_clients()
{
	addVariable(m_id);
	addVariable(m_clients);

	for (std::vector<NetworkId>::const_iterator i = clients.begin(); i != clients.end(); ++i)
		m_clients.push_back(*i);
}	// CreateSyncUiMessage::CreateSyncUiMessage

//-----------------------------------------------------------------------

CreateSyncUiMessage::CreateSyncUiMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("CreateSyncUiMessage"),
	m_id(),
	m_clients()
{
	addVariable(m_id);
	addVariable(m_clients);
	unpack(source);
}	// CreateSyncUiMessage::CreateSyncUiMessage

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
CreateSyncUiMessage::~CreateSyncUiMessage()
{
}	// CreateSyncUiMessage::~CreateSyncUiMessage


