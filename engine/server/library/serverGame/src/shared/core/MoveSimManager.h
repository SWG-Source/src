//======================================================================
//
// MoveSimManager.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MoveSimManager_H
#define INCLUDED_MoveSimManager_H

//======================================================================

class NetworkId;
class CreatureObject;

//----------------------------------------------------------------------

class MoveSimManager
{
public:

	static void start (const NetworkId & userId, int numNpcToSpawn, int numPcToSpawn, float moveScale, float moveSpeed);
	static void clear ();
	static void checkApplySimulation (CreatureObject & creature);
};

//======================================================================

#endif
