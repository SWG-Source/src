// ======================================================================
//
// PvpUpdateObserver.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PvpUpdateObserver.h"
#include "serverGame/Client.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/Pvp.h"
#include "serverGame/TangibleObject.h"
#include "serverGame/ServerTangibleObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "sharedDebug/Profiler.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/UpdatePvpStatusMessage.h"

#include <unordered_map>
#include <unordered_set>

// ======================================================================

namespace PvpUpdateObserverNamespace
{
	struct ClientObjectPointerHash
	{
		size_t operator()(Client const * const ptr) const
		{
			return (reinterpret_cast<const size_t>(ptr) >> 4);
		};
	};

	struct TangibleObjectPointerHash
	{
		size_t operator()(TangibleObject const * const ptr) const
		{
			return (reinterpret_cast<const size_t>(ptr) >> 4);
		};
	};

	std::unordered_set<TangibleObject const *, TangibleObjectPointerHash> s_activeUpdaters;

	typedef std::unordered_map<Client const *, std::unordered_map<NetworkId, std::pair<uint32, uint32> >, ClientObjectPointerHash> PvpUpdateObserverCache;
	PvpUpdateObserverCache s_pvpUpdateObserverCache;

	typedef std::unordered_map<Client const *, std::unordered_set<NetworkId>, ClientObjectPointerHash> PvpUpdateObserverRequestsThisFrame;
	PvpUpdateObserverRequestsThisFrame s_pvpUpdateObserverRequestsThisFrame;

	// PvpUpdateObserver can be called multiple times per frame for the same object,
	// and in the constructor, we iterate over the clients observing the object,
	// and then interate over the objects being observed by this client; these
	// 2 lists can be quite large on a crowded game server, and repeated iteration
	// of these 2 lists doesn't gain us anything, but does use up lots of loop time,
	// so as an optimization, we'll only iterate over these 2 lists once per frame,
	// and then just watch on individual updates to these 2 lists afterwards, and
	// process the individual updates to these 2 lists when the updates occur
	std::unordered_set<NetworkId> s_objectsProcessedThisFrame;
	std::unordered_set<Client const *, ClientObjectPointerHash> s_clientsProcessedThisFrame;

	void sendUpdatePvpStatusMessage(Client const & client, std::unordered_set<NetworkId> const & requestedNetworkId, std::unordered_map<NetworkId, std::pair<uint32, uint32> > & cachedNetworkId);
}

using namespace PvpUpdateObserverNamespace;

// ======================================================================

PvpUpdateObserver::PvpUpdateObserver(TangibleObject const *who, Archive::AutoDeltaObserverOp) :
	m_obj(0),
	m_pvpFaction(0)
{
	PROFILER_AUTO_BLOCK_DEFINE("PvpUpdateObserver_init");

	if (!who || !who->getNetworkId().isValid() || s_activeUpdaters.find(who) != s_activeUpdaters.end())
		return;

	IGNORE_RETURN(s_activeUpdaters.insert(who));
	m_obj = who;
	m_pvpFaction = who->getPvpFaction();

	// get client visible status for everyone observing this object (including itself)
	if ((s_objectsProcessedThisFrame.count(who->getNetworkId()) == 0) && satisfyPvpSyncCondition(who->isNonPvpObject(), who->hasCondition(ServerTangibleObjectTemplate::C_invulnerable), (who->asCreatureObject() != nullptr), who->getPvpFaction()))
	{
		std::set<Client *> const &clients = who->getObservers();
		for (std::set<Client *>::const_iterator i = clients.begin(); i != clients.end(); ++i)
		{
			std::unordered_map<NetworkId, std::pair<uint32, uint32> > & pvpUpdateObserverCache = s_pvpUpdateObserverCache[*i];
			if (pvpUpdateObserverCache.count(who->getNetworkId()) == 0)
			{
				uint32 flags, factionId;
				Pvp::getClientVisibleStatus(**i, *who, flags, factionId);
				pvpUpdateObserverCache[who->getNetworkId()] = std::make_pair(flags, factionId);
			}

			IGNORE_RETURN(s_pvpUpdateObserverRequestsThisFrame[*i].insert(who->getNetworkId()));
		}

		IGNORE_RETURN(s_objectsProcessedThisFrame.insert(who->getNetworkId()));
	}
	// get client visible status for everyone this object is observing except itself
	{
		Client * const client = who->getClient();
		if (client && (s_clientsProcessedThisFrame.count(client) == 0))
		{
			std::unordered_map<NetworkId, std::pair<uint32, uint32> > & pvpUpdateObserverCache = s_pvpUpdateObserverCache[client];
			std::unordered_set<NetworkId> & pvpUpdateObserverRequestsThisFrame = s_pvpUpdateObserverRequestsThisFrame[client];

			Client::ObservingListPvpSync const &objs = client->getObservingPvpSync();
			for (Client::ObservingListPvpSync::const_iterator i = objs.begin(); i != objs.end(); ++i)
			{
				if (*i != who)
				{
					if (pvpUpdateObserverCache.count((*i)->getNetworkId()) == 0)
					{
						uint32 flags, factionId;
						Pvp::getClientVisibleStatus(*client, **i, flags, factionId);
						pvpUpdateObserverCache[(*i)->getNetworkId()] = std::make_pair(flags, factionId);
					}

					IGNORE_RETURN(pvpUpdateObserverRequestsThisFrame.insert((*i)->getNetworkId()));
				}
			}

			IGNORE_RETURN(s_clientsProcessedThisFrame.insert(client));
		}
	}
}

// ----------------------------------------------------------------------

PvpUpdateObserver::~PvpUpdateObserver()
{
	if (!m_obj)
		return;

	IGNORE_RETURN(s_activeUpdaters.erase(m_obj));

	// see if faction changed to or from imperial or rebel
	if ((m_pvpFaction != m_obj->getPvpFaction()) && !m_obj->getObservers().empty())
	{
		if (PvpData::isImperialFactionId(m_pvpFaction) ||
			PvpData::isRebelFactionId(m_pvpFaction) ||
			PvpData::isImperialFactionId(m_obj->getPvpFaction()) ||
			PvpData::isRebelFactionId(m_obj->getPvpFaction()))
		{
			// did the object's "pvp sync" status change because of the faction change?
			bool const wasPvpSync = PvpUpdateObserver::satisfyPvpSyncCondition(m_obj->isNonPvpObject(), m_obj->hasCondition(ServerTangibleObjectTemplate::C_invulnerable), (m_obj->asCreatureObject() != nullptr), m_pvpFaction);
			bool const isPvpSync = PvpUpdateObserver::satisfyPvpSyncCondition(m_obj->isNonPvpObject(), m_obj->hasCondition(ServerTangibleObjectTemplate::C_invulnerable), (m_obj->asCreatureObject() != nullptr), m_obj->getPvpFaction());

			if (wasPvpSync != isPvpSync)
			{
				// force pvp status update
				Pvp::forceStatusUpdate(*(const_cast<TangibleObject *>(m_obj)));

				const std::set<Client *> &observers = m_obj->getObservers();
				for (std::set<Client *>::const_iterator i = observers.begin(); i != observers.end(); ++i)
				{
					if (isPvpSync)
						(*i)->addObservingPvpSync(const_cast<TangibleObject *>(m_obj));
					else
						(*i)->removeObservingPvpSync(const_cast<TangibleObject *>(m_obj));
				}
			}
		}
	}

	m_obj = 0;
}

// ----------------------------------------------------------------------

void PvpUpdateObserver::updatePvpStatusCache(Client const *client, TangibleObject const &who, uint32 flags, uint32 factionId)
{
	if (!client || !who.getNetworkId().isValid() || !PvpUpdateObserver::satisfyPvpSyncCondition(who.isNonPvpObject(), who.hasCondition(ServerTangibleObjectTemplate::C_invulnerable), (who.asCreatureObject() != nullptr), who.getPvpFaction()))
		return;

	s_pvpUpdateObserverCache[client][who.getNetworkId()] = std::make_pair(flags, factionId);
}

// ----------------------------------------------------------------------

void PvpUpdateObserver::removeClientFromPvpStatusCache(Client const *client)
{
	if (!client)
		return;

	// the Client * is not guaranteed to be valid, so don't dereference it
	IGNORE_RETURN(s_pvpUpdateObserverCache.erase(client));
	IGNORE_RETURN(s_pvpUpdateObserverRequestsThisFrame.erase(client));
	IGNORE_RETURN(s_clientsProcessedThisFrame.erase(client));
}

// ----------------------------------------------------------------------

void PvpUpdateObserver::stopObservingPvpSyncNotification(Client const *client, NetworkId const & who)
{
	if (!client || !who.isValid())
		return;

	PvpUpdateObserverCache::iterator iterCacheClient = s_pvpUpdateObserverCache.find(client);
	PvpUpdateObserverRequestsThisFrame::iterator iterRequestedClient = s_pvpUpdateObserverRequestsThisFrame.find(client);

	if (iterCacheClient != s_pvpUpdateObserverCache.end())
		IGNORE_RETURN(iterCacheClient->second.erase(who));

	if (iterRequestedClient != s_pvpUpdateObserverRequestsThisFrame.end())
		IGNORE_RETURN(iterRequestedClient->second.erase(who));
}

// ----------------------------------------------------------------------

void PvpUpdateObserver::startObservingPvpSyncNotification(Client const *client, TangibleObject const &who)
{
	if (!client || !who.getNetworkId().isValid())
		return;

	if ((s_objectsProcessedThisFrame.count(who.getNetworkId()) != 0) || (s_clientsProcessedThisFrame.count(client) != 0))
	{
		std::unordered_map<NetworkId, std::pair<uint32, uint32> > & pvpUpdateObserverCache = s_pvpUpdateObserverCache[client];
		if (pvpUpdateObserverCache.count(who.getNetworkId()) == 0)
		{
			uint32 flags, factionId;
			Pvp::getClientVisibleStatus(*client, who, flags, factionId);
			pvpUpdateObserverCache[who.getNetworkId()] = std::make_pair(flags, factionId);
		}

		IGNORE_RETURN(s_pvpUpdateObserverRequestsThisFrame[client].insert(who.getNetworkId()));
	}
}

// ----------------------------------------------------------------------

void PvpUpdateObserver::sendUpdateAndRemoveClientFromPvpStatusCache(Client const *client)
{
	if (!client)
		return;

	PvpUpdateObserverCache::iterator iterCacheClient = s_pvpUpdateObserverCache.find(client);
	PvpUpdateObserverRequestsThisFrame::iterator iterRequestedClient = s_pvpUpdateObserverRequestsThisFrame.find(client);

	if ((iterCacheClient != s_pvpUpdateObserverCache.end()) && (iterRequestedClient != s_pvpUpdateObserverRequestsThisFrame.end()))
		sendUpdatePvpStatusMessage(*client, iterRequestedClient->second, iterCacheClient->second);

	if (iterCacheClient != s_pvpUpdateObserverCache.end())
		s_pvpUpdateObserverCache.erase(iterCacheClient);

	if (iterRequestedClient != s_pvpUpdateObserverRequestsThisFrame.end())
		s_pvpUpdateObserverRequestsThisFrame.erase(iterRequestedClient);

	IGNORE_RETURN(s_clientsProcessedThisFrame.erase(client));
}

// ----------------------------------------------------------------------

void PvpUpdateObserver::update()
{
	PvpUpdateObserverCache::iterator iterCacheClient;

	for (PvpUpdateObserverRequestsThisFrame::const_iterator iterRequestedClient = s_pvpUpdateObserverRequestsThisFrame.begin(); iterRequestedClient != s_pvpUpdateObserverRequestsThisFrame.end(); ++iterRequestedClient)
	{
		iterCacheClient = s_pvpUpdateObserverCache.find(iterRequestedClient->first);
		if (iterCacheClient == s_pvpUpdateObserverCache.end())
			continue;

		sendUpdatePvpStatusMessage(*(iterRequestedClient->first), iterRequestedClient->second, iterCacheClient->second);
	}

	s_pvpUpdateObserverRequestsThisFrame.clear();
	s_objectsProcessedThisFrame.clear();
	s_clientsProcessedThisFrame.clear();
}

// ----------------------------------------------------------------------

void PvpUpdateObserverNamespace::sendUpdatePvpStatusMessage(Client const & client, std::unordered_set<NetworkId> const & requestedNetworkId, std::unordered_map<NetworkId, std::pair<uint32, uint32> > & cachedNetworkId)
{
	uint32 flags, factionId;
	ServerObject const * serverObject;
	TangibleObject const * tangibleObject;
	std::unordered_map<NetworkId, std::pair<uint32, uint32> >::iterator iterCacheNetworkId;

	for (std::unordered_set<NetworkId>::const_iterator iterRequestedNetworkId = requestedNetworkId.begin(); iterRequestedNetworkId != requestedNetworkId.end(); ++iterRequestedNetworkId)
	{
		iterCacheNetworkId = cachedNetworkId.find(*iterRequestedNetworkId);
		if (iterCacheNetworkId == cachedNetworkId.end())
			continue;

		serverObject = ServerWorld::findObjectByNetworkId(*iterRequestedNetworkId);
		if (serverObject)
		{
			tangibleObject = serverObject->asTangibleObject();
			if (tangibleObject)
			{
				Pvp::getClientVisibleStatus(client, *tangibleObject, flags, factionId);
				if (flags != iterCacheNetworkId->second.first || factionId != iterCacheNetworkId->second.second)
				{
					if (ConfigServerGame::getLogPvpUpdates())
					{
						LOG("PvpUpdate", ("sending UpdatePvpStatus for %s to client %s, flags=%d, factionId=%d",
							tangibleObject->getNetworkId().getValueString().c_str(),
							client.getCharacterObjectId().getValueString().c_str(),
							flags,
							factionId));
					}
					UpdatePvpStatusMessage message(tangibleObject->getNetworkId(), flags, factionId);
					client.send(message, true);
					iterCacheNetworkId->second.first = flags;
					iterCacheNetworkId->second.second = factionId;
				}
			}
		}
	}
}

// ======================================================================
