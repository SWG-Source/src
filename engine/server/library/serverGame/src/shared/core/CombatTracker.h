// ======================================================================
//
// CombatTracker.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef CombatTracker_H
#define CombatTracker_H

// ======================================================================

class ServerObject;
class TangibleObject;

// ======================================================================

class CombatTracker
{
public:
	static void install();
	static void remove();
	static void update();

	static void addDefender(TangibleObject *defender);
	static void removeDefender(TangibleObject *defender);

private:
	static void getInterestedViewers(TangibleObject const *defender, std::vector<ServerObject *> &combatViewers);
};

// ======================================================================

#endif // CombatTracker_H

