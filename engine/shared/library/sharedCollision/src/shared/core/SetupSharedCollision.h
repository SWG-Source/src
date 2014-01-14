// ======================================================================
//
// SetupSharedCollision.h
// copyright 2002 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_SetupSharedCollision_H
#define INCLUDED_SetupSharedCollision_H

class CrcLowerString;
class Object;

typedef bool (*PlayEffectHook)(CrcLowerString const & effectName, Object * object, CrcLowerString const & hardpoint );
typedef bool (*IsPlayerHouseHook)(Object const * object);

// ======================================================================

class SetupSharedCollision
{
public:

	struct Data
	{
		bool              installExtents;
		bool              installCollisionWorld;
		PlayEffectHook    playEffect;
		IsPlayerHouseHook isPlayerHouse;
		bool              serverSide;
	};

	static void install ( Data const & data );
};

// ======================================================================

#endif
