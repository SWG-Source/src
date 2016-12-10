// ======================================================================
//
// HateList.h
// Copyright 2005 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_HateList_H
#define	INCLUDED_HateList_H

#include "Archive/AutoDeltaMap.h"
#include "sharedObject/CachedNetworkId.h"

class HateList;
class NetworkId;
class PlayerObject;
class TangibleObject;

// ======================================================================
//
// HateList
//
// ======================================================================

class HateList
{
public:

	typedef std::map<CachedNetworkId, float> UnSortedList;
	typedef std::vector<std::pair<CachedNetworkId, float> > SortedList;
	typedef std::set<CachedNetworkId> RecentList;

	static float getMaxDistanceToTarget();

public:

	HateList();
	~HateList();

	void setOwner(TangibleObject * const owner);
	void addServerNpAutoDeltaVariables(Archive::AutoDeltaByteStream & stream);

	void alter();
	bool isEmpty() const;
	bool addHate(NetworkId const & target, float const hate);
	bool setHate(NetworkId const & target, float const hate);
	float getHate(NetworkId const & target) const;
	float getMaxHate() const;
	bool removeTarget(NetworkId const & target);
	void clear();
	bool isOnList(NetworkId const & target) const;
	int getTimeSinceLastUpdate() const;
	void resetHateTimer();
	void setAutoExpireTargetEnabled(bool enabled);
	bool isAutoExpireTargetEnabled() const;
	int getAutoExpireTargetDuration() const;

	CachedNetworkId const & getTarget() const;
	UnSortedList const & getUnSortedList() const; // Faster
	void getSortedList(SortedList & sortedHateList) const; // Slower, don't call every frame

	RecentList const & getRecentList() const;
	void clearRecentList();

	void setAILeashTime( float time );
	float getAILeashTime( void ) const;

	bool isOwnerPlayer() const;

	void forceHateTarget(NetworkId const & target);

private:

	void findNewTarget();
	void setTarget(CachedNetworkId const & target, float const hate);
	void triggerTargetChanged(NetworkId const & target);
	void triggerTargetAdded(NetworkId const & target);
	void triggerTargetRemoved(NetworkId const & target);
	bool isOwnerValid() const;
	bool isValidTarget(Object * const target);

	HateList(HateList const & hateList);
	bool operator ==(HateList const & rhs) const;
	bool operator !=(HateList const & rhs) const;
	HateList & operator =(HateList const & rhs);

	TangibleObject * m_owner;
	PlayerObject * m_playerObject;
	Archive::AutoDeltaMap<CachedNetworkId, float> m_hateList;
	Archive::AutoDeltaVariable<CachedNetworkId> m_target;
	Archive::AutoDeltaVariable<float> m_maxHate;
	Archive::AutoDeltaVariable<time_t> m_lastUpdateTime;
	Archive::AutoDeltaVariable<time_t> m_autoExpireTargetDuration;
	std::set<CachedNetworkId> m_recentHateList; // This is used for assist logic
};

//----------------------------------------------------------------------

inline bool HateList::isOwnerPlayer() const
{
	return (m_playerObject != nullptr);
}

// ======================================================================

#endif // INCLUDED_HateList_H
