// ======================================================================
//
// AiShipAttackTargetList.h
// Copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiShipAttackTargetList_H
#define	INCLUDED_AiShipAttackTargetList_H

#include "sharedObject/CachedNetworkId.h"

class NetworkId;
class ShipObject;

// ======================================================================

class AiShipAttackTargetList
{
public:

	class AiShipAttackTargetEntry
	{
	public:

		AiShipAttackTargetEntry(float const initialDamage);

		float m_damage;
		time_t m_lastDamageTime;

	private:

		// Disabled

		AiShipAttackTargetEntry();
	};

	typedef std::map<CachedNetworkId, AiShipAttackTargetEntry> TargetList;
	typedef std::vector<std::pair<CachedNetworkId, AiShipAttackTargetEntry> > SortedTargetList;

public:

	AiShipAttackTargetList(ShipObject * const owner);
	~AiShipAttackTargetList();

	bool isEmpty() const;
	void add(ShipObject & unit, float const damage);
	bool remove(NetworkId const & unit, bool const inDestructorHack = false);
	void purge(time_t const maxAge);
	void clear();
	void verify();

	CachedNetworkId const & getPrimaryTarget() const;
	TargetList const & getUnSortedTargetList() const; // Faster
	void getSortedTargetList(SortedTargetList & destTargetList) const;   // Slower,  don't call every frame

private:

	void findNewPrimaryTarget();
	void setNewPrimaryTarget(CachedNetworkId const & newPrimaryTarget, float const damage);

	ShipObject * const m_owner;
	TargetList * const m_targetList;
	CachedNetworkId m_primaryTarget;
	float m_primaryTargetDamage;
	
	// Disabled

	AiShipAttackTargetList(AiShipAttackTargetList const &);
	AiShipAttackTargetList &operator =(AiShipAttackTargetList const &);
};

// ======================================================================

#endif // INCLUDED_AiShipAttackTargetList_H
