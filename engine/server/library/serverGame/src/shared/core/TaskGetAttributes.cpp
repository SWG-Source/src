// ======================================================================
//
// TaskGetAttributes.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/TaskGetAttributes.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/StaticLootItemManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedNetworkMessages/AttributeListMessage.h"
#include "sharedObject/NetworkIdManager.h"

#include "UnicodeUtils.h"

// ======================================================================

TaskGetAttributes::TaskGetAttributes(NetworkId const & playerId, NetworkId const & targetId, int const clientRevision) :
	m_playerId(playerId),
	m_targetId(targetId),
	m_staticItemName(),
	m_clientRevision(clientRevision)
{
}

// ----------------------------------------------------------------------

TaskGetAttributes::TaskGetAttributes(NetworkId const & playerId, std::string const & staticItemName, int const clientRevision) :
	m_playerId(playerId),
	m_targetId(),
	m_staticItemName(staticItemName),
	m_clientRevision(clientRevision)
{
}

// ----------------------------------------------------------------------

bool TaskGetAttributes::run()
{
	CreatureObject * const playerCreature = CreatureObject::getCreatureObject(m_playerId);
	if (playerCreature)
	{
		PlayerObject * const player = PlayerCreatureController::getPlayerObject(playerCreature);
		if (player) 
		{
			Client const * const client = GameServer::getInstance().getClient(m_playerId);
			if (client)
			{
				static ServerObject::AttributeVector data;
				data.clear();
				int serverRevision = -127; // an arbitrary value that is not the same as the initial client revision on the game client (ObjectAttributesManager.cpp).
				bool forceAttributeUpdate = false;

				if(!m_staticItemName.empty())
				{
					StaticLootItemManager::getAttributes(m_playerId, m_staticItemName, data);
					AttributeListMessage const msg(m_staticItemName, data, serverRevision);
					client->send(msg, true);
				}
				else
				{
					ServerObject const * const obj = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(m_targetId));
					if (obj)
					{
						serverRevision = obj->getAttributeRevision();

						if (obj->hasAttributeCaching() && !player->isCrafting()) 
						{
							if (serverRevision != m_clientRevision) 
								obj->getAttributes(m_playerId, data);
						}
						else
						{
							obj->getAttributes(m_playerId, data);
							forceAttributeUpdate = true;
						}
					}
					else
					{
						ResourceTypeObject const * const resourceType = ServerUniverse::getInstance().getResourceTypeById(m_targetId);
						if (resourceType)
						{
							serverRevision = 0; // rls - these values don't change (at this time).
							if (serverRevision != m_clientRevision) 
								resourceType->getResourceAttributes(data);
						}
						else
						{
							serverRevision = static_cast<int>(player->getWaypoints().size());
							if (serverRevision != m_clientRevision)
								player->getWaypointAttributes(m_targetId, data);
						}
					}

					if (serverRevision != m_clientRevision || forceAttributeUpdate == true)
					{
						AttributeListMessage const msg(m_targetId, data, serverRevision);
						client->send(msg, true);
					}
				}
			}
		}
	}
	return true;
}

// ======================================================================
