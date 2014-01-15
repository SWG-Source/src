// ============================================================================
// 
// ConsentManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsentManager.h"

#include "sharedNetworkMessages/ConsentResponseMessage.h"

#include "sharedGame/ProsePackage.h"

#include "serverGame/Client.h"
#include "serverGame/GameServer.h"

#include "swgSharedNetworkMessages/ConsentRequestMessage.h"

#include "StringId.h"

#include <list>
#include <list>

// ============================================================================

ConsentManager::ConsentManager()
: m_consentMap(),
  m_nextId(0)
{
}

//-----------------------------------------------------------------------------

int ConsentManager::askConsent(const NetworkId& player, const ProsePackage& question, CallbackFunction callback)
{
	int newId = m_nextId++;

	std::pair<int, CallbackFunction> p = std::make_pair(newId, callback);

	std::map<NetworkId, std::list<std::pair<int, CallbackFunction> > >::iterator i = m_consentMap.find(player);
	if(i == m_consentMap.end())
	{
		std::list<std::pair<int, CallbackFunction> > l;
		l.push_back(p);
		m_consentMap.insert(std::make_pair(player, l));
	}
	else
	{
		i->second.push_back(p);
	}
	
	Client *client = GameServer::getInstance().getClient(player);
	ConsentRequestMessage msg(question, newId);
	if(client)
		client->send(msg, true);
	return newId;
}

//-----------------------------------------------------------------------------

void ConsentManager::removePlayerConsentRequests(const NetworkId& player)
{
	m_consentMap.erase(player);
}

//-----------------------------------------------------------------------------

void ConsentManager::handleResponse (const ConsentResponseMessage& msg)
{
	const NetworkId& player = msg.getNetworkId();
	int  id = msg.getId();
	bool response = msg.getResponse();

	//look for the player in the mapping
	std::map<NetworkId, std::list<std::pair<int, CallbackFunction> > >::iterator i = m_consentMap.find(player);
	if(i != m_consentMap.end())
	{
		//look for the unique consent id in the mapping
		std::list<std::pair<int, CallbackFunction> > l = i->second;
		for(std::list<std::pair<int, CallbackFunction> >::iterator i2 = l.begin(); i2 != l.end(); ++i2)
		{
			if(i2->first == id)
			{
				//call the callback function
				CallbackFunction func = i2->second;
				if(func)
					func(player, id, response);
				//remove the request from the list now that it's been processed
				l.erase(i2);
				//remove the player from the mapping if they have no more consent requests pending
				if(l.empty())
				{
					m_consentMap.erase(player);
				}
				break;
			}
		}
	}
}

// ============================================================================
