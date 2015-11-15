//======================================================================
//
// BuffManager.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_BuffManager_H
#define INCLUDED_BuffManager_H

#include "sharedFoundation/NetworkId.h"

//======================================================================

class CreatureObject;

//----------------------------------------------------------------------

class BuffManager
{
public:
	static void install();
	static void remove();

	static bool applyBuffEffects(CreatureObject *creatureObj, uint32 buffNameCrc, float duration, float dynamicValue, uint32 stackCount, NetworkId caster);
	static bool removeBuffEffects(CreatureObject *creatureObj, uint32 buffNameCrc, NetworkId caster);
	static bool reapplyBuffEffects(CreatureObject *creatureObj, uint32 buffNameCrc, float duration, float dynamicValue, uint32 stackCount, NetworkId caster);

	static bool getIsBuffPersisted(uint32 buffNameCrc);
	static bool getIsBuffRemovedOnDeath(uint32 buffNameCrc);
	static bool getIsBuffPlayerRemovable(uint32 buffNameCrc);
	static bool getDoesBuffDecayOnPvPDeath(uint32 buffNameCrc);

	static int  getBuffState(uint32 buffNameCrc);
	static float getBuffDefaultDuration(uint32 buffNameCrc);
	static std::string const & getBuffCallback(uint32 buffNameCrc);
	static uint32 getBuffMaxStacks(uint32 buffNameCrc);
	static bool getBuffIsCelestial(uint32 buffNameCrc);
	
protected:
	
	static bool applyBuffEffectsInternal(CreatureObject *creatureObj, uint32 buffNameCrc, float duration, float dynamicValue, bool deleteFirst, uint32 stackCount, NetworkId caster);
	
};


//======================================================================

#endif
