//========================================================================
//
// RegionPvp.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_RegionPvp_H
#define INCLUDED_RegionPvp_H

#include "serverGame/Pvp.h"
#include "serverGame/Region.h"

// ======================================================================

class BattlefieldMarkerObject;

// ======================================================================

/**
 * A region used as a pvp area (battlefields).
 */
class RegionPvp : public RegionCircle
{
public:
	RegionPvp(float centerX, float centerY, float radius);
	RegionPvp(float centerX, float centerY, float radius, const CachedNetworkId & dynamicRegionId);

	virtual RegionPvp *asRegionPvp();
	virtual RegionPvp const *asRegionPvp() const;

	void setBattlefieldMarker(BattlefieldMarkerObject *marker);
	BattlefieldMarkerObject const *getBattlefieldMarker() const;
	void checkBattlefieldMarker();

	Pvp::FactionId getBattlefieldFactionId(const NetworkId & object) const;
	void setBattlefieldParticipant(const NetworkId & object, Pvp::FactionId factionId) const;
	void clearBattlefieldParticipants() const;

private:
	// disabled
	RegionPvp();
	RegionPvp(const RegionPvp &);
	RegionPvp & operator =(const RegionPvp &);

	void initializeBattlefield();

private:
	BattlefieldMarkerObject *m_battlefieldMarker;
};

// ----------------------------------------------------------------------

inline BattlefieldMarkerObject const *RegionPvp::getBattlefieldMarker() const
{
	return m_battlefieldMarker;
}

// ======================================================================

#endif	// INCLUDED_RegionPvp_H

