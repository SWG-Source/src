// ======================================================================
// 
// Squad.h
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#ifndef INCLUDED_Squad_H
#define INCLUDED_Squad_H

#include "serverGame/Formation.h"
#include "sharedFoundation/Timer.h"
#include "sharedMath/Vector.h"
#include "sharedObject/CachedNetworkId.h"

class NetworkId;
class PersistentCrcString;

// ----------------------------------------------------------------------
class Squad
{
public:

	friend class SpaceSquadManager;

	typedef std::map<CachedNetworkId, PersistentCrcString const *> UnitMap;
	typedef std::vector<CachedNetworkId> UnitList;

	static void install();

public:

	virtual char const * getClassName() const = 0;
	virtual void alter(float deltaSeconds);

	bool isEmpty() const;
	int getUnitCount() const;
	UnitMap const & getUnitMap() const; // faster
	void getUnitList(UnitList & unitList); // slower
	int getId() const;

	void addUnit(NetworkId const & unit);
	void removeUnit(NetworkId const & unit);

	Formation & getFormation();

	bool setLeader(NetworkId const & unit, bool rebuildFormation);
	CachedNetworkId const & getLeader() const;

	Vector const & getLeaderOffsetPosition_l() const;

	void combineWith(Squad & squad);
	void setUnitFormationPosition_l(NetworkId const & unit, Vector const & position_l);

	Vector const & getSquadPosition_w();

	virtual void assignNewLeader() = 0;

	bool contains(NetworkId const & unit) const;


protected:

	Squad();
	virtual ~Squad() = 0;

	virtual void onAddUnit(NetworkId const & unit) = 0;
	virtual void onRemoveUnit() = 0;
	virtual void onNewLeader(NetworkId const & oldLeader) = 0;
	virtual void onSetUnitFormationPosition_l(NetworkId const & unit, Vector const & position_l) = 0;
	void calculateSquadPosition_w();
	void setId(int const id);

	Vector m_leaderOffsetPosition_l;

private:

	static void remove();

	void buildFormation();

	int m_id;
	UnitMap * const m_unitMap;
	Formation m_formation;
	CachedNetworkId m_leader;
	Vector m_squadPosition_w;
	Timer m_squadPositionUpdateTimer;

	// Disabled

	Squad(Squad const &);
	Squad & operator =(Squad const &);
};

// ======================================================================

#endif // INCLUDED_Squad_H
