// ======================================================================
//
// ServerPathfindingMessaging.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverPathfinding/FirstServerPathfinding.h"
#include "serverPathfinding/ServerPathfindingMessaging.h"

#include "serverGame/AiLocation.h"
#include "serverGame/CellObject.h"
#include "serverGame/Client.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverPathfinding/CityPathGraph.h"
#include "serverPathfinding/CityPathGraphManager.h"
#include "serverPathfinding/CityPathNode.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/CommandTable.h"
#include "sharedNetworkMessages/AIDebuggingMessages.h"
#include <map>
#include <set>

#include "sharedFoundation/CrcConstexpr.hpp"

ServerPathfindingMessaging * g_messaging = nullptr;

// ======================================================================

void ServerPathfindingMessaging::install ( void )
{
	g_messaging = new ServerPathfindingMessaging();

	g_messaging->connectToMessage(RequestWatchObjectPath::MESSAGE_TYPE);
	g_messaging->connectToMessage(RequestWatchPathMap::MESSAGE_TYPE);
	g_messaging->connectToMessage(RequestUnstick::MESSAGE_TYPE);
}

void ServerPathfindingMessaging::remove ( void )
{
	g_messaging->disconnectFromMessage(RequestWatchObjectPath::MESSAGE_TYPE);
	g_messaging->disconnectFromMessage(RequestWatchPathMap::MESSAGE_TYPE);
	g_messaging->disconnectFromMessage(RequestUnstick::MESSAGE_TYPE);

	delete g_messaging;
	g_messaging = nullptr;
}

ServerPathfindingMessaging & ServerPathfindingMessaging::getInstance ( void )
{
	return *g_messaging;
}

// ----------------------------------------------------------------------

ServerPathfindingMessaging::ServerPathfindingMessaging()
: m_clientList( new ClientList() ),
  m_callback( new MessageDispatch::Callback )
{
}

ServerPathfindingMessaging::~ServerPathfindingMessaging()
{
	for(ClientList::iterator it = m_clientList->begin(); it != m_clientList->end(); ++it)
	{
		ignorePathMap(*it);
	}

	delete m_clientList;
	m_clientList = nullptr;

	delete m_callback;
	m_callback = nullptr;
}

// ----------

void ServerPathfindingMessaging::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	UNREF(source);

	if(message.isType(RequestWatchObjectPath::MESSAGE_TYPE))
	{
		const RequestWatchObjectPath &m = dynamic_cast<const RequestWatchObjectPath &>(message);
		CachedNetworkId cid(m.getClientId());
		ServerObject * s = static_cast<ServerObject *>(cid.getObject());
		Client * client = s->getClient();
		if (m.getEnable())
		{
			watchObjectPath(client, m.getObjectId());
		}
		else
		{
			ignoreObjectPath(client, m.getObjectId());
		}
	}
	else if(message.isType(RequestWatchPathMap::MESSAGE_TYPE))
	{
		const RequestWatchPathMap &m = dynamic_cast<const RequestWatchPathMap &>(message);
		CachedNetworkId cid(m.getClientId());
		ServerObject * s = static_cast<ServerObject *>(cid.getObject());
		Client * client = s->getClient();
		if (m.getEnable())
		{
			watchPathMap(client);
		}
		else
		{
			ignorePathMap(client);
		}
	}
	else if(message.isType(RequestUnstick::MESSAGE_TYPE))
	{
		const RequestUnstick &m = dynamic_cast<const RequestUnstick &>(message);
		CachedNetworkId cid(m.getClientId());
		ServerObject * s = static_cast<ServerObject *>(cid.getObject());
		NOT_NULL(s);

		// Give the scripts a chance to handle the "unsticking"
		GameScriptObject * const gso = s->getScriptObject();
		if (gso)
		{
			ScriptParams scriptParams;
			if (SCRIPT_OVERRIDE == gso->trigAllScripts(Scripting::TRIG_UNSTICKING, scriptParams))
			{
				//-- script has handled the unstick
				return;
			}
		}

		// unsticking a player character in Restuss always takes the character
		// to the imperial cloning center if he is imperial, rebel cloning
		// center if he is rebel, or starport if he is neutral
		if (ServerWorld::getSceneId() == std::string("rori"))
		{
			CreatureObject * co = s->asCreatureObject();
			if (co && PlayerCreatureController::getPlayerObject(co))
			{
				Vector const location = s->getPosition_w();

				// restuss_nobuild_1 region
				if ((location.x >= 4518.0f) && (location.x <= 6118.0f) && (location.z >= 4880.0f) && (location.z <= 6480.0f))
				{
					Vector unstickPoint;

					if (PvpData::isImperialFactionId(co->getPvpFaction()))
					{
						// imperial installation
						unstickPoint.x = 5878.0f;
						unstickPoint.y = 81.0f;
						unstickPoint.z = 5581.0f;
					}
					else if (PvpData::isRebelFactionId(co->getPvpFaction()))
					{
						// rebel installation
						unstickPoint.x = 4863.0f;
						unstickPoint.y = 77.0f;
						unstickPoint.z = 5876.0f;
					}
					else
					{
						// starport
						unstickPoint.x = 5304.0f;
						unstickPoint.y = 80.0f;
						unstickPoint.z = 6185.0f;
					}

					CreatureController * controller = co->getCreatureController();
					if (controller)
					{
						Transform newTransform = Transform::identity;
						newTransform.setPosition_p(unstickPoint);
						controller->teleport(newTransform, nullptr);
					}

					return;
				}
			}
		}

		Vector unstickPoint;
		CellObject * cell = ContainerInterface::getContainingCellObject(*s);
		if (cell != nullptr && s->asCreatureObject() != nullptr)
		{
			// try finding a waypoint in the cell first
			if (!cell->getClosestPathNodePos(*s, unstickPoint))
			{
				// if there are no waypoints, use the /eject command
				s->asCreatureObject()->commandQueueEnqueue(CommandTable::getCommand(
					constcrc("eject")), NetworkId::cms_invalid, Unicode::String());
			}
		}
		else
		{
			// teleport the object to a close-by waypoint
			if (!CityPathGraphManager::getClosestPathNodePos(s,unstickPoint))
			{
				// try using ServerWorld::getGoodLocation to find a spot
				static const float SEARCH_SIZE = 32.0f;
				const Vector center(s->getPosition_w());
				const Vector llArea(center.x - SEARCH_SIZE, center.y, center.z - SEARCH_SIZE);
				const Vector urArea(center.x + SEARCH_SIZE, center.y, center.z + SEARCH_SIZE);
				unstickPoint = ServerWorld::getGoodLocation(5.0f, 5.0f, llArea, urArea, false, true);
				
				// unstick failed - warp them to Mos Eisley starport
				if(unstickPoint.x == 0 && unstickPoint.y == 0 && unstickPoint.z == 0)
				{
					// only warp them to mos eisley if they are on one of the 10 original planets
					const std::string mySceneId = s->getSceneId();
					if(mySceneId == "corellia"
						|| mySceneId == "dantooine"
						|| mySceneId == "dathomir"
						|| mySceneId == "endor"
						|| mySceneId == "lok"
						|| mySceneId == "naboo"
						|| mySceneId == "rori"
						|| mySceneId == "talus"
						|| mySceneId == "tatooine"
						|| mySceneId == "yavin4"
						)
					{
						GameServer::getInstance().requestSceneWarp(CachedNetworkId(*s),"tatooine",Vector(3528,5,-4804),NetworkId::cms_invalid,Vector(3528,5,-4804));
						return;
					}
				}
			}
		}
		if (unstickPoint.x != 0 || unstickPoint.y != 0 || unstickPoint.z != 0)
		{
			CreatureController * controller = dynamic_cast<CreatureController *>(s->getController());
			if (controller)
			{
				Transform newTransform = Transform::identity;
				newTransform.setPosition_p(unstickPoint);
				controller->teleport(newTransform, cell);
			}
		}
	}
}

// ----------------------------------------------------------------------

void ServerPathfindingMessaging::watchObjectPath(Client * client, const NetworkId &object)
{
	UNREF(client);
	UNREF(object);
}

// ----------

void ServerPathfindingMessaging::ignoreObjectPath(Client * client, const NetworkId &object)
{
	UNREF(client);
	UNREF(object);
}

// ----------------------------------------------------------------------

void ServerPathfindingMessaging::watchPathMap(Client * client)
{
	if(client == nullptr) return;

	// Add the client to our client list

	m_clientList->insert(client);

	// and send it all the graphs we have

	int count = CityPathGraphManager::getGraphCount();

	for(int i = 0; i < count; i++)
	{
		CityPathGraph const * graph = CityPathGraphManager::getGraph(i);

		sendGraphInfo(graph,client);
	}

	m_callback->connect(client->getDestroyNotifier(), *this, &ServerPathfindingMessaging::onClientDestroy);
}

// ----------

void ServerPathfindingMessaging::ignorePathMap(Client * client)
{
	if(client == nullptr) return;

	m_clientList->erase(client);

	int count = CityPathGraphManager::getGraphCount();

	for(int i = 0; i < count; i++)
	{
		CityPathGraph const * graph = CityPathGraphManager::getGraph(i);

		sendEraseGraph(graph,client);
	}

	m_callback->disconnect(client->getDestroyNotifier(), *this, &ServerPathfindingMessaging::onClientDestroy);
}

// ----------------------------------------------------------------------

void ServerPathfindingMessaging::onClientDestroy(ClientDestroy & d)
{
	m_clientList->erase(d.client);

	m_callback->disconnect(d.client->getDestroyNotifier(), *this, &ServerPathfindingMessaging::onClientDestroy);
}

// ----------------------------------------------------------------------

void ServerPathfindingMessaging::sendGraphInfo ( CityPathGraph const * graph )
{
	if(graph == nullptr) return;

	for(ClientList::iterator it = m_clientList->begin(); it != m_clientList->end(); ++it)
	{
		sendGraphInfo( graph, (*it) );
	}
}

// ----------

void ServerPathfindingMessaging::sendGraphInfo ( CityPathGraph const * graph, Client * client )
{
	if(client == nullptr) return;
	if(graph == nullptr) return;

	int nodeCount = graph->getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		CityPathNode const * node = graph->_getNode(i);

		if(node) sendNodeInfo(node,client);
	}
}

// ----------------------------------------------------------------------

void ServerPathfindingMessaging::sendEraseGraph ( CityPathGraph const * graph )
{
	if(graph == nullptr) return;

	for(ClientList::iterator it = m_clientList->begin(); it != m_clientList->end(); ++it)
	{
		sendEraseGraph( graph, (*it) );
	}
}

// ----------

void ServerPathfindingMessaging::sendEraseGraph ( CityPathGraph const * graph, Client * client )
{
	if(client == nullptr) return;
	if(graph == nullptr) return;

	int nodeCount = graph->getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		CityPathNode const * node = graph->_getNode(i);

		if(node) sendEraseNode(node,client);
	}
}

// ----------------------------------------------------------------------

void ServerPathfindingMessaging::sendNodeInfo ( CityPathNode const * node )
{
	if(node == nullptr) return;

	for(ClientList::iterator it = m_clientList->begin(); it != m_clientList->end(); ++it)
	{
		sendNodeInfo( node, (*it) );
	}
}

// ----------

void ServerPathfindingMessaging::sendNodeInfo ( CityPathNode const * node, Client * client )
{
	if(node == nullptr) return;
	if(client == nullptr) return;

	AINodeInfo m;

	m.setNodeId(node->getDebugId());
	m.setLocation(node->getPosition_p());
	m.setLevel(0);
	m.setParent(0);

	std::vector<int> children;
	m.setChildren(children);

	std::vector<int> siblings;

	int edgeCount = node->getEdgeCount();

	for(int i = 0; i < edgeCount; i++)
	{
		int neighborIndex = node->getNeighbor(i);

		CityPathNode const * neighbor = node->_getGraph()->_getNode(neighborIndex);

		if(neighbor) siblings.push_back(neighbor->getDebugId());
	}

	m.setSiblings(siblings);

	m.setType(0);

	// ----------

	client->send(m, true);
}

// ----------------------------------------------------------------------

void ServerPathfindingMessaging::sendNeighborInfo ( CityPathNode const * node )
{
	if(node == nullptr) return;

	for(ClientList::iterator it = m_clientList->begin(); it != m_clientList->end(); ++it)
	{
		sendNeighborInfo( node, (*it) );
	}
}

// ----------

void ServerPathfindingMessaging::sendNeighborInfo ( CityPathNode const * node, Client * client )
{
	if(node == nullptr) return;
	if(client == nullptr) return;

	int edgeCount = node->getEdgeCount();

	for(int i = 0; i < edgeCount; i++)
	{
		int neighborIndex = node->getNeighbor(i);

		CityPathNode const * neighbor = node->_getGraph()->_getNode(neighborIndex);

		if(neighbor) sendNodeInfo(neighbor,client);
	}
}

// ----------------------------------------------------------------------

void ServerPathfindingMessaging::sendEraseNode ( CityPathNode const * node )
{
	if(node == nullptr) return;

	for(ClientList::iterator it = m_clientList->begin(); it != m_clientList->end(); ++it)
	{
		sendEraseNode( node, (*it) );
	}
}

// ----------

void ServerPathfindingMessaging::sendEraseNode ( CityPathNode const * node, Client * client )
{
	if(node == nullptr) return;
	if(client == nullptr) return;

	AINodeInfo m;

	m.setNodeId(node->getDebugId());
	m.setLocation(Vector::zero);
	m.setLevel(0);
	m.setParent(0);
	m.setType(-1);

	client->send(m, true);
}

// ----------------------------------------------------------------------

void ServerPathfindingMessaging::sendWaypointInfo ( AiLocation const & loc )
{
	for(ClientList::iterator it = m_clientList->begin(); it != m_clientList->end(); ++it)
	{
		sendWaypointInfo( loc, (*it) );
	}
}

void ServerPathfindingMessaging::sendWaypointInfo ( AiLocation const & loc, Client * client )
{
	if(client == nullptr) return;

	AINodeInfo m;

	m.setNodeId(loc.getDebugId());
	m.setLocation(loc.getPosition_w());
	m.setLevel(0);
	m.setParent(0);

	std::vector<int> children;
	m.setChildren(children);

	std::vector<int> siblings;
	m.setSiblings(siblings);

	m.setType(1);

	// ----------

	client->send(m, true);
}

// ----------------------------------------------------------------------

void ServerPathfindingMessaging::sendEraseWaypoint ( AiLocation const & loc )
{
	for(ClientList::iterator it = m_clientList->begin(); it != m_clientList->end(); ++it)
	{
		sendEraseWaypoint( loc, (*it) );
	}
}

void ServerPathfindingMessaging::sendEraseWaypoint ( AiLocation const & loc, Client * client )
{
	if(client == nullptr) return;

	AINodeInfo m;

	m.setNodeId(loc.getDebugId());
	m.setLocation(Vector::zero);
	m.setLevel(0);
	m.setParent(0);
	m.setType(-1);

	client->send(m, true);
}


// ======================================================================

