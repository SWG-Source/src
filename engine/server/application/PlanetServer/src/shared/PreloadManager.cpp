// ======================================================================
//
// PreloadManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstPlanetServer.h"
#include "PreloadManager.h"

#include "ConfigPlanetServer.h"
#include "GameServerData.h"
#include "PlanetProxyObject.h"
#include "QuadtreeNode.h"
#include "Scene.h"
#include "serverUtility/PopulationList.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <cstdio>

// ======================================================================

PreloadManager::PreloadManager() :
		Singleton2<PreloadManager>(),
		m_serverMap(new ServerMapType),
		m_preloadList(new PreloadListType),
		m_serversWaiting(new ServersWaitingType),
		m_listReceived(false)
{
	ExitChain::add(&remove, "PreloadManager::remove");
}

// ----------------------------------------------------------------------

PreloadManager::~PreloadManager()
{
	delete m_serverMap;
	delete m_preloadList;
	delete m_serversWaiting;
	
	m_serverMap=0;
	m_preloadList=0;
	m_serversWaiting=0;
}

// ----------------------------------------------------------------------

/**
 * Set up the preload list, based on the scene and config options.
 */
void PreloadManager::handlePreloadList()
{
	m_listReceived=true;

	WARNING(ConfigPlanetServer::getLoadWholePlanet() && ConfigPlanetServer::getLoadWholePlanetMultiserver(),("You have specified both loadWholePlanet and loadWholePlanetMultiserver.  LoadWholePlanet (single-server) will take precedence."));
	
	if (ConfigPlanetServer::getLoadWholePlanet() || PlanetServer::getInstance().isInSpaceMode() || !PlanetServer::getInstance().getEnablePreload())
	{
		// Set up a single server to handle the whole planet
		PreloadListData fakeBeacon;
		fakeBeacon.m_topLeftChunkX = 0;
		fakeBeacon.m_topLeftChunkZ = 0;
		fakeBeacon.m_bottomRightChunkX = 100;
		fakeBeacon.m_bottomRightChunkZ = 100;
		fakeBeacon.m_cityServerId = 1;
		fakeBeacon.m_wildernessServerId = 1;

		m_preloadList->clear();
		m_preloadList->push_back(fakeBeacon);
		
		m_serversWaiting->insert(1);
		PreloadServerInformation & psi = (*m_serverMap)[1];
		psi.m_actualServerId = 0;
		psi.m_timeLoadStarted = time(0);
		psi.m_loadTime = -1;
	}
	// WTF is this ... the tutorial planet bypasses the preload multiserver setting in favor of this pie slice algorithm
	// We should not need this anymore with the NPE tutorial
	// NPE will use the preload tables for multiserver and the 'NewbieTutorial' manager for placing the tutotial instances 
	/*
	else if(PlanetServer::getInstance().isInTutorialMode())
	{
		m_preloadList->clear();

		int n=ConfigPlanetServer::getNumTutorialServers();
		if ((ConfigPlanetServer::getMaxGameServers() != 0) && (n > ConfigPlanetServer::getMaxGameServers()))
			n=ConfigPlanetServer::getMaxGameServers();
		for (int i=0 ; i<n; ++i)
		{
			float angle = PI_TIMES_2 * (static_cast<float>(i)/static_cast<float>(n));

			// Make a circle of load beacons.  The effect will be to divide the map into wedges
			PreloadListData fakeBeacon;
			fakeBeacon.m_topLeftChunkX = Node::roundToNode(static_cast<int>(cos(angle) * 1000.0f));
			fakeBeacon.m_topLeftChunkZ = Node::roundToNode(static_cast<int>(sin(angle) * 1000.0f));
			fakeBeacon.m_bottomRightChunkX = fakeBeacon.m_topLeftChunkX + Node::getNodeSize();
			fakeBeacon.m_bottomRightChunkZ = fakeBeacon.m_topLeftChunkZ + Node::getNodeSize();
			fakeBeacon.m_cityServerId = i+1;
			fakeBeacon.m_wildernessServerId = i+1;

			DEBUG_REPORT_LOG(true,("Loadbeacon at %i, %i\n",fakeBeacon.m_topLeftChunkX, fakeBeacon.m_topLeftChunkZ));
			
			m_preloadList->push_back(fakeBeacon);
			PreloadServerInformation & psi = (*m_serverMap)[i+1];
			psi.m_actualServerId = 0;
			psi.m_timeLoadStarted = time(0);
			psi.m_loadTime = -1;
			m_serversWaiting->insert(i+1);
		}
	}*/
	else
	{
		if (PlanetServer::getInstance().getEnablePreload())
		{
			DataTable * data = DataTableManager::getTable(ConfigPlanetServer::getPreloadDataTableName(),true);
			FATAL(data==nullptr,("Could not find data table %s, needed for preload",ConfigPlanetServer::getPreloadDataTableName()));
			
			int numRows = data->getNumRows();
			for (int row=0; row<numRows; ++row)
			{
				if (data->getStringValue("Scene",row)==Scene::getInstance().getSceneId())
				{
					PreloadListData item;
					item.m_topLeftChunkX=data->getIntValue("Top Left Chunk X",row);
					item.m_topLeftChunkZ=data->getIntValue("Top Left Chunk Z",row);
					item.m_bottomRightChunkX=data->getIntValue("Bottom Right Chunk X",row);
					item.m_bottomRightChunkZ=data->getIntValue("Bottom Right Chunk Z",row);
					item.m_cityServerId=data->getIntValue("City Server Id",row);
					item.m_wildernessServerId=data->getIntValue("Wilderness Server Id",row);

					if (ConfigPlanetServer::getMaxGameServers() != 0)
					{
						// Reduce the total number of game servers
						item.m_cityServerId = ((item.m_cityServerId - 1) % ConfigPlanetServer::getMaxGameServers()) + 1;

						if (item.m_wildernessServerId != 0)
							item.m_wildernessServerId = ((item.m_wildernessServerId - 1) % ConfigPlanetServer::getMaxGameServers()) + 1;
					}
					
					m_preloadList->push_back(item);
				
					ServerMapType::iterator mapIter = m_serverMap->find(item.m_cityServerId);
					if (mapIter == m_serverMap->end())
					{
						PreloadServerInformation & psi = (*m_serverMap)[item.m_cityServerId];
						psi.m_actualServerId = 0;
						psi.m_timeLoadStarted = time(0);
						psi.m_loadTime = -1;
						m_serversWaiting->insert(item.m_cityServerId);
					}

					if (item.m_wildernessServerId != 0)
					{
						mapIter = m_serverMap->find(item.m_wildernessServerId);
						if (mapIter == m_serverMap->end())
						{
							PreloadServerInformation & psi = (*m_serverMap)[item.m_wildernessServerId];
							psi.m_actualServerId = 0;
							psi.m_timeLoadStarted = time(0);
							psi.m_loadTime = -1;
							m_serversWaiting->insert(item.m_wildernessServerId);
						}
					}
				}
			}
		}
	}

	FATAL(PlanetServer::getInstance().getEnablePreload() && m_serversWaiting->size()==0,("Planet server started for scene %s, which was not listed in the preload data table",
																						 Scene::getInstance().getSceneId().c_str()));

	std::set<PreloadServerId> startGameServerList;
	if (m_serversWaiting->size() < 1)
	{
		IGNORE_RETURN(startGameServerList.insert(1));
	}
	else
	{
		for (ServersWaitingType::const_iterator i = m_serversWaiting->begin(); i != m_serversWaiting->end(); ++i)
			IGNORE_RETURN(startGameServerList.insert(*i));
	}

	Scene::getInstance().assignPreloadsToNodes();
	PlanetServer::getInstance().startGameServer(startGameServerList, 0);
}

// ----------------------------------------------------------------------

/**
 * Called when a game server reports that it is ready to receive objects
 */
bool PreloadManager::onGameServerReady(uint32 serverId, PreloadServerId preloadServerId)
{
	bool result = true;
	bool noPreload = true;

	// attempt to assign the game server the specified role in the preload list, if possible
	bool assigned = false;

	for (ServerMapType::iterator i=m_serverMap->begin(); i!=m_serverMap->end(); ++i)
	{
		if (i->first == preloadServerId)
		{
			if (i->second.m_actualServerId == 0)
			{
				i->second.m_actualServerId = serverId;
				sendPreloadsToGameServer(serverId, i->first);
				noPreload = false;
				assigned = true;
			}

			break;
		}
	}

	if (!assigned)
	{
		for (ServerMapType::iterator j=m_serverMap->begin(); j!=m_serverMap->end(); ++j)
		{
			if (j->second.m_actualServerId == 0)
			{
				// assign the new game server to this role in the preload list
				j->second.m_actualServerId = serverId;
				sendPreloadsToGameServer(serverId, j->first);
				noPreload = false;
				break;
			}
		}
	}

	if (noPreload)
	{
		// this server has nothing to preload, so advance its status right away
		GameServerData *data = PlanetServer::getInstance().getGameServerData(serverId);
		if(! PlanetServer::getInstance().getEnablePreload())
		{
			if (data)
				data->preloadComplete();
			
			// send PreloadRequestCompleteMessage, because things on the game server wait for it
			PlanetServer::getInstance().sendPreloadRequestCompleteMessage(serverId, 0);
		}
		else
		{
			if(! PlanetServer::getInstance().isInTutorialMode())
			{
				result = false; // disconnect the game server, it's extraneous.
			}
		}
	}
	
	// if the preload list is empty, preloading is finished as soon as at least 1 game server is up
	if (m_serverMap->empty())
	{
		DEBUG_REPORT_LOG(ConfigPlanetServer::getLogPreloading(),("Preloading is finished on %s because there was nothing to preload.\n",Scene::getInstance().getSceneId().c_str()));
		PlanetServer::getInstance().preloadCompleteOnAllServers();
	}
	return result;
}

// ----------------------------------------------------------------------

void PreloadManager::sendPreloadsToGameServer(uint32 realServerId, PreloadServerId preloadServerId) const
{
	if (PlanetServer::getInstance().getEnablePreload())
		Scene::getInstance().loadAllNodesForServer(realServerId, preloadServerId);

	PlanetServer::getInstance().sendPreloadRequestCompleteMessage(realServerId, preloadServerId);
}

// ----------------------------------------------------------------------

/**
 * Called when a game server drops connection.  Remove it from the preload assignments.
 */
void PreloadManager::removeGameServer(uint32 serverId)
{
	for (ServerMapType::iterator i=m_serverMap->begin(); i!=m_serverMap->end(); ++i)
	{
		if (i->second.m_actualServerId == serverId)
		{
			i->second.m_actualServerId = 0;
			i->second.m_timeLoadStarted = time(0);
			i->second.m_loadTime = -1;
			m_serversWaiting->insert(i->first);
			break;
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Called when a game server indicates it has all the preload objects
 */
void PreloadManager::preloadCompleteOnServer(uint32 serverId)
{
	DEBUG_REPORT_LOG(ConfigPlanetServer::getLogPreloading(),("Preload is finished on server %lu (planet %s)\n",serverId, Scene::getInstance().getSceneId().c_str()));

	PreloadServerId const preloadId = getPreloadServerId(serverId);
	m_serversWaiting->erase(preloadId);

	// mark how long it took the server to load
	ServerMapType::iterator i = m_serverMap->find(preloadId);
	if ((i != m_serverMap->end()) && (i->second.m_loadTime == -1))
		i->second.m_loadTime = std::max(static_cast<int>(0), static_cast<int>(time(0) - i->second.m_timeLoadStarted));

	if (isPreloadComplete())
	{
		if (ConfigPlanetServer::getEnableStartupCreateProxies())
		{
			Scene::getInstance().createAllProxies();
		}
		
		DEBUG_REPORT_LOG(ConfigPlanetServer::getLogPreloading(),("Preloading is finished on all servers for %s.\n", Scene::getInstance().getSceneId().c_str()));
		PlanetServer::getInstance().preloadCompleteOnAllServers();
	}
}

// ----------------------------------------------------------------------

/**
 * Finds the preload game server for the specified spot.
 */
PreloadServerId PreloadManager::getPreloadGameServer(int x, int z)
{
	PreloadServerId bestSoFar = 0;
	int bestDistance = 0;
	for (std::vector<PreloadListData>::const_iterator item=m_preloadList->begin(); item!=m_preloadList->end(); ++item)
	{
		if (item->m_cityServerId != 0)
		{
			int minX=Node::roundToNode(item->m_topLeftChunkX);
			int minZ=Node::roundToNode(item->m_topLeftChunkZ);
			int maxX=Node::roundToNode(item->m_bottomRightChunkX);
			int maxZ=Node::roundToNode(item->m_bottomRightChunkZ);
						
			if ((x >= minX) && (x < maxX) && (z >= minZ) && (z < maxZ))
				return item->m_cityServerId;
		}
		if (item->m_wildernessServerId != 0)
		{
			// Pretend there is a "beacon" in the middle of the preload area
			int beaconX=(item->m_topLeftChunkX + item->m_bottomRightChunkX) / 2;
			int beaconZ=(item->m_topLeftChunkZ + item->m_bottomRightChunkZ) / 2;
			int distanceToBeacon = ((x-beaconX)*(x-beaconX) + (z-beaconZ)*(z-beaconZ));

			if (bestSoFar == 0 || distanceToBeacon < bestDistance)
			{
				bestDistance = distanceToBeacon;
				bestSoFar = item->m_wildernessServerId;
			}
		}
	}

	return bestSoFar;
}

// ----------------------------------------------------------------------

/**
 * Finds the closest operational game server to the specified spot.  Used
 * when the server specified in the preload list isn't available, because
 * either it crashed or it hasn't started yet.
 * Considers wilderness first, because city servers are probably more
 * heavily loaded.
 */
uint32 PreloadManager::getClosestGameServer (int x, int z)
{
	uint32 bestSoFar = 0;
	int bestDistance = 0;
	std::vector<PreloadListData>::const_iterator item;
	for (item=m_preloadList->begin(); item!=m_preloadList->end(); ++item)
	{
		if (item->m_wildernessServerId != 0)
		{
			int beaconX=(item->m_topLeftChunkX + item->m_bottomRightChunkX) / 2;
			int beaconZ=(item->m_topLeftChunkZ + item->m_bottomRightChunkZ) / 2;
			int distanceToBeacon = ((x-beaconX)*(x-beaconX) + (z-beaconZ)*(z-beaconZ));
			uint32 realServerId = getRealServerId(item->m_wildernessServerId);
			if ((realServerId != 0) && (bestSoFar == 0 || distanceToBeacon < bestDistance))
			{
				bestDistance = distanceToBeacon;
				bestSoFar = realServerId;
			}
		}
	}

	if (bestSoFar==0)
	{
		// No wilderness servers.  Try city servers
		for (item=m_preloadList->begin(); item!=m_preloadList->end(); ++item)
		{
			if (item->m_cityServerId != 0)
			{
				int beaconX=(item->m_topLeftChunkX + item->m_bottomRightChunkX) / 2;
				int beaconZ=(item->m_topLeftChunkZ + item->m_bottomRightChunkZ) / 2;
				int distanceToBeacon = ((x-beaconX)*(x-beaconX) + (z-beaconZ)*(z-beaconZ));
				uint32 realServerId = getRealServerId(item->m_cityServerId);
				if ((realServerId != 0) && (bestSoFar == 0 || distanceToBeacon < bestDistance))
				{
					bestDistance = distanceToBeacon;
					bestSoFar = realServerId;
				}
			}
		}
	}
	
	return bestSoFar; // returning 0 is possible
}

// ----------------------------------------------------------------------

/**
 * Adds the loadbeacon locations to a PopulationList structure.
 */
void PreloadManager::updatePopulationList(PopulationList &theList) const
{
	for (std::vector<PreloadListData>::const_iterator item=m_preloadList->begin(); item!=m_preloadList->end(); ++item)
	{
		int chunkX=(item->m_topLeftChunkX + item->m_bottomRightChunkX) / 2;
		int chunkZ=(item->m_topLeftChunkZ + item->m_bottomRightChunkZ) / 2;
		theList.setPopulation(Scene::getInstance().getSceneId(),chunkX, chunkZ, Scene::getInstance().getServerPopulationByLocation(chunkX, chunkZ));
	}
}

// ----------------------------------------------------------------------

PreloadServerId PreloadManager::getPreloadServerId(uint32 realServerId) const
{
	for (ServerMapType::iterator i=m_serverMap->begin(); i!=m_serverMap->end(); ++i)
		if (i->second.m_actualServerId == realServerId)
			return i->first;

	return 0;
}

// ----------------------------------------------------------------------

bool PreloadManager::isPreloadComplete() const
{
	return (m_listReceived && m_serversWaiting->empty());
}

// ----------------------------------------------------------------------

void PreloadManager::getDebugString(std::string &output) const
{
	if (isPreloadComplete())
		output="finished";
	else
	{
		output="loading";
		for (ServersWaitingType::const_iterator i=m_serversWaiting->begin(); i!=m_serversWaiting->end(); ++i)
		{
			uint32 realServerId=0;
			int loadTime=0;
			ServerMapType::const_iterator j=m_serverMap->find(*i);
			if (j!=m_serverMap->end())
			{
				realServerId=j->second.m_actualServerId;

				if (j->second.m_loadTime != -1)
					loadTime = j->second.m_loadTime;
				else
					loadTime = time(0) - j->second.m_timeLoadStarted;
			}
			
			char temp[100];
			sprintf(temp," %lu-%ds (%lu)",*i,loadTime,realServerId);
			output+=temp;
		}
	}
}

// ----------------------------------------------------------------------

uint32 PreloadManager::getRealServerId(PreloadServerId preloadServerId) const
{
	ServerMapType::iterator i=m_serverMap->find(preloadServerId);
	if (i!=m_serverMap->end())
		return i->second.m_actualServerId;

	return 0;
}

// ======================================================================
