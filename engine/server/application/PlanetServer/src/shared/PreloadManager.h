// ======================================================================
//
// PreloadManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PreloadManager_H
#define INCLUDED_PreloadManager_H

// ======================================================================

#include "Singleton/Singleton2.h"

// ======================================================================

class PopulationList;

typedef uint32 PreloadServerId; // each server has a id from the preload list, which isn't the same as its processid.  Typedef to make the code easier to read.

class PreloadListData
{
  public:
	int       m_topLeftChunkX;
	int       m_topLeftChunkZ;
	int       m_bottomRightChunkX;
	int       m_bottomRightChunkZ;
	PreloadServerId m_cityServerId;
	PreloadServerId m_wildernessServerId;
};

class PreloadServerInformation
{
  public:
	uint32    m_actualServerId;
	time_t    m_timeLoadStarted;
	int       m_loadTime;
};

// ======================================================================

class PreloadManager : public Singleton2<PreloadManager>
{
  public:
	PreloadManager();
	virtual ~PreloadManager();

  public:
	bool onGameServerReady       (uint32 serverId, PreloadServerId preloadServerId);
	void handlePreloadList       ();
	void removeGameServer        (uint32 serverId);
	void preloadCompleteOnServer (uint32 serverId);
	uint32 getClosestGameServer  (int x, int z);
	PreloadServerId getPreloadGameServer (int x, int z);
	void updatePopulationList    (PopulationList &theList) const;
	bool isPreloadComplete       () const;
	PreloadServerId getPreloadServerId (uint32 realServerId) const;
	uint32 getRealServerId       (PreloadServerId preloadServerId) const;
	void getDebugString          (std::string &output) const;

	typedef std::map<PreloadServerId, PreloadServerInformation> ServerMapType;
	ServerMapType const * getServerMap () const;
	
  private:
	void            sendPreloadsToGameServer  (uint32 realServerId, PreloadServerId preloadServerId) const;
		
  private:
	ServerMapType *m_serverMap; // map of server id in preload list ---> actual server id

	typedef std::vector<PreloadListData> PreloadListType;
	PreloadListType *m_preloadList;

	typedef std::set<PreloadServerId> ServersWaitingType;
	ServersWaitingType *m_serversWaiting; // set of servers (by list id, not actual id) that still need preloads.

	bool m_listReceived;
};

//-----------------------------------------------------------------------

inline PreloadManager::ServerMapType const * PreloadManager::getServerMap () const
{
	return m_serverMap;
}

// ======================================================================

#endif
