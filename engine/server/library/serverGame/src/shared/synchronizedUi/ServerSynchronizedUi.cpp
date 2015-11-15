// ======================================================================
//
// ServerSynchronizedUi.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerSynchronizedUi.h"

#include "serverGame/Client.h"
#include "serverGame/GameServer.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObject.h"
#include "serverNetworkMessages/CreateSyncUiMessage.h"
#include "sharedNetworkMessages/BaselinesMessage.h"
#include "sharedNetworkMessages/DeltasMessage.h"
#include "sharedObject/NetworkIdManager.h"

#include <vector>

//-----------------------------------------------------------------------
ServerSynchronizedUi::ServerSynchronizedUi(ServerObject & obj) :
	m_clientObjects(0),
	m_owner(&obj),
	m_ownerChangingAuthority(false),
	m_uiPackage(),
	m_watchedByList()
{
	m_clientObjects = new std::vector<NetworkId>;
}
//-----------------------------------------------------------------------

ServerSynchronizedUi::~ServerSynchronizedUi()
{
	delete m_clientObjects;
	m_clientObjects = 0;
}

//-----------------------------------------------------------------------

void ServerSynchronizedUi::addClientObject(ServerObject & obj)
{
	if (!obj.getClient())
	{
		WARNING_STRICT_FATAL(true, ("Tried to add a client object %s that had no client\n", obj.getNetworkId().getValueString().c_str()));
		return;
	}
#ifdef _DEBUG
	if (std::find(m_clientObjects->begin(), m_clientObjects->end(), obj.getNetworkId()) != m_clientObjects->end())
	{
		WARNING_STRICT_FATAL(true, ("Trying to add a client %s to SynchUI object that is already there\n", obj.getNetworkId().getValueString().c_str()));
	}
#endif //_DEBUG
	else
		m_clientObjects->push_back(obj.getNetworkId());

	CreatureObject* co = dynamic_cast<CreatureObject*>(&obj);
	if(co)
	{
		Client* c = co->getClient();
		if(c)
		{
			c->addSynchronizedUi(this);
		}
	}

	sendBaselinesToClient(*obj.getClient());
}

//-----------------------------------------------------------------------

void ServerSynchronizedUi::addToUiPackage(Archive::AutoDeltaVariableBase & source)
{
	m_uiPackage.addVariable(source);
}

//-----------------------------------------------------------------------

void ServerSynchronizedUi::removeClientObject(const NetworkId & client)
{
	std::vector<NetworkId>::iterator i = std::find(m_clientObjects->begin(), m_clientObjects->end(),client);
	if (i == m_clientObjects->end())
	{
		WARNING_STRICT_FATAL(true, ("Trying to remove a client %s to SynchUI object that doesn't exist \n", client.getValueString().c_str()));
		return;
	}
	m_clientObjects->erase(i);

	CreatureObject* co = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(client));
	if(co)
	{
		Client* c = co->getClient();
		if(c)
		{
			c->removeSynchronizedUi(this);
		}
	}
}

//-----------------------------------------------------------------------

void ServerSynchronizedUi::removeAllClientObjects()
{
	m_clientObjects->clear();
}

//-----------------------------------------------------------------------

void ServerSynchronizedUi::applyBaselines(Archive::ReadIterator & source)
{
	m_uiPackage.unpack(source);
}

//-----------------------------------------------------------------------

void ServerSynchronizedUi::applyDeltas(const DeltasMessage& source)
{
	DEBUG_FATAL(source.getPackageId() != DeltasMessage::DELTAS_UI, ("Synchrnoized UI received bad package type"));
	Archive::ReadIterator bs = source.getPackage().begin();
	m_uiPackage.unpackDeltas(bs);
}

//-----------------------------------------------------------------------

void ServerSynchronizedUi::sendBaselinesToClient(const Client& client) const
{
	BaselinesMessage uiPackage(m_owner->getNetworkId(), m_owner->getObjectType(), m_uiPackage, BaselinesMessage::BASELINES_UI);
	client.send(uiPackage, true);
}

//-----------------------------------------------------------------------

void ServerSynchronizedUi::onAuthorityTransferred()
{
	CreateSyncUiMessage const createSyncUiMessage(m_owner->getNetworkId(), getClients());
	ServerMessageForwarding::send(createSyncUiMessage);

	BaselinesMessage const uiPackage(m_owner->getNetworkId(), m_owner->getObjectType(), m_uiPackage, BaselinesMessage::BASELINES_UI);
	ServerMessageForwarding::send(uiPackage);

	m_ownerChangingAuthority = true;

	removeAllClientObjects();
}

//-----------------------------------------------------------------------

void ServerSynchronizedUi::sendDeltas() const
{
	if (m_uiPackage.getItemCount() > 0)
	{
		DeltasMessage uiPackage(m_owner->getNetworkId(), m_owner->getObjectType(), m_uiPackage, DeltasMessage::DELTAS_UI);
		std::vector<NetworkId>::const_iterator i = m_clientObjects->begin();
		for(; i != m_clientObjects->end(); ++i)
		{
			ServerObject * object = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(*i));
			if (object && object->getClient())
			{
				object->getClient()->send(uiPackage, true);
			}
		}
	}
}

//----------------------------------------------------------------------

const std::vector<NetworkId> & ServerSynchronizedUi::getClients() const
{
static const std::vector<NetworkId> NO_CLIENTS;

	return m_clientObjects ? *m_clientObjects : NO_CLIENTS;
}

//----------------------------------------------------------------------

int ServerSynchronizedUi::getNumClients () const
{
	return m_clientObjects ? static_cast<int>(m_clientObjects->size ()) : 0;
}

//-----------------------------------------------------------------------
