// ======================================================================
//
// PvpUpdateObserver.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _PvpUpdateObserver_H_
#define _PvpUpdateObserver_H_

#include "Archive/AutoDeltaObserverOps.h"
#include "sharedGame/PvpData.h"

// ======================================================================

class Client;
class NetworkId;
class TangibleObject;

// ======================================================================

class PvpUpdateObserver
{
public:
	PvpUpdateObserver(TangibleObject const *who, Archive::AutoDeltaObserverOp operation);
	~PvpUpdateObserver();

	static void updatePvpStatusCache(Client const *client, TangibleObject const &who, uint32 flags, uint32 factionId);
	static void removeClientFromPvpStatusCache(Client const *client);
	static void stopObservingPvpSyncNotification(Client const *client, NetworkId const & who);
	static void startObservingPvpSyncNotification(Client const *client, TangibleObject const &who);
	static void sendUpdateAndRemoveClientFromPvpStatusCache(Client const *client);
	static bool satisfyPvpSyncCondition(bool isNonPvpObject, bool isInvulnerable, bool isCreatureObject, uint32 factionId);
	static void update();

private:
	PvpUpdateObserver(PvpUpdateObserver const &);
	PvpUpdateObserver& operator=(PvpUpdateObserver const &);

	TangibleObject const *m_obj;
	uint32 m_pvpFaction;
};

//----------------------------------------------------------------------

inline bool PvpUpdateObserver::satisfyPvpSyncCondition(bool isNonPvpObject, bool isInvulnerable, bool isCreatureObject, uint32 factionId)
{
	return ((!isNonPvpObject && !isInvulnerable) || (isCreatureObject && (PvpData::isImperialFactionId(factionId) || PvpData::isRebelFactionId(factionId))));
}

// ======================================================================

#endif // _PvpUpdateObserver_H_
