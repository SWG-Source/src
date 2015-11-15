// ======================================================================
//
// CommandChecks.h
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CommandChecks_H
#define INCLUDED_CommandChecks_H

// ======================================================================


// ======================================================================

class Command;
class NetworkId;

//----------------------------------------------------------------------

class CommandChecks // static class
{
public:
	static void getRangeForCommand(Command const * command, float minWeaponRange, float maxWeaponRange, bool weaponCheckLast, float & min, float & max);
	static bool isLocomotionValidForCommand(Command const * command, int8 locomotion, float speed);
	static bool isEnoughActionPointsForCommand(Command const * command, int actionPoints);
	static bool isEnoughMindPointsForCommand(Command const * command, int mindPoints);

	static bool isMob(NetworkId const & targetId);
	static bool isVehicle(NetworkId const & targetId);
	static bool isMonster(NetworkId const & targetId);
	static bool isNpc(NetworkId const & targetId);
	static bool isDroid(NetworkId const & targetId);
};

//----------------------------------------------------------------------

#endif // INCLUDED_CommandChecks_H

