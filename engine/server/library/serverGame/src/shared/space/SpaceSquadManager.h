// ======================================================================
// 
// SpaceSquadManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#ifndef INCLUDED_SpaceSquadManager_H
#define INCLUDED_SpaceSquadManager_H

class NetworkId;
class SpaceSquad;

// ----------------------------------------------------------------------
class SpaceSquadManager
{
public:

	static void install();
	static void alter(float const deltaTime);

public:

	static int createSquadId();
	static SpaceSquad * createSquad();
	static bool setSquadId(NetworkId const & unit, int const newSquadId);
	static SpaceSquad * getSquad(int const squadId);

private:

	static void remove();
	static SpaceSquad * createSquad(int const squadId);

	// Disable

	SpaceSquadManager();
	~SpaceSquadManager();
	SpaceSquadManager(SpaceSquadManager const &);
	SpaceSquadManager & operator =(SpaceSquadManager const &);
};

// ======================================================================

#endif // INCLUDED_SpaceSquadManager_H
