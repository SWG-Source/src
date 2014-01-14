//======================================================================
//
// ShipHitEffectsManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipHitEffectsManager_H
#define INCLUDED_ShipHitEffectsManager_H

//======================================================================

class ShipHitEffectsManager
{
public:

	/**
	* These must be kept in sync with:
	* base_class.java
	* space/ship_hit_effects.tab
	* space/ship_hit_sounds.tab
	*/
	enum HitType
	{ 
		HT_shield,
		HT_armor,
		HT_component,
		HT_chassis,
		HT_numTypes
	};

private:
	                         ShipHitEffectsManager ();
	                         ShipHitEffectsManager (const ShipHitEffectsManager & rhs);
	ShipHitEffectsManager & operator= (const ShipHitEffectsManager & rhs);
};

//======================================================================

#endif
