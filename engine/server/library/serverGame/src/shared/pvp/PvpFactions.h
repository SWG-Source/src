// ======================================================================
//
// PvpFactions.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _PvpFactions_H_
#define _PvpFactions_H_

// ======================================================================

#include "Pvp.h"

// ======================================================================

class PvpFactions
{
public:
	static bool isNonaggressiveFaction(Pvp::FactionId factionId);
	static bool isUnattackableFaction(Pvp::FactionId factionId);
	static bool isBountyTargetFaction(Pvp::FactionId factionId);
	static bool isBubbleFaction(Pvp::FactionId factionId);
	static std::vector<Pvp::FactionId> const &getOpposingFactions(Pvp::FactionId factionId);
	static Pvp::FactionId getBattlefieldFactionId();
	static Pvp::FactionId getDuelFactionId();
	static Pvp::FactionId getBountyDuelFactionId();
	static Pvp::FactionId getGuildWarCoolDownPeriodFactionId();
	static Pvp::FactionId getBubbleCombatFactionId();
};

// ======================================================================

#endif // _PvpFactions_H_

