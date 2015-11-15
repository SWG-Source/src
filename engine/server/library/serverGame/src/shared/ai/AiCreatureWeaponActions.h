// ======================================================================
//
// AiCreatureWeaponActions.h
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_AiCreatureWeaponActions_H
#define INCLUDED_AiCreatureWeaponActions_H

#include "Archive/AutoDeltaByteStream.h"
#include "Archive/AutoDeltaVector.h"

class AiCreatureCombatProfile;
class CreatureObject;
class PersistentCrcString;

// ======================================================================
class AiCreatureWeaponActions
{
public:

	AiCreatureWeaponActions();

	void addServerNpAutoDeltaVariables(Archive::AutoDeltaByteStream & stream);
	
	/**
	 * Reset all the timers for this weapon's actions. This needs to be called
	 * each time the AI enters combat.
	 */

	void reset();

	/**
	 * Sets the combat profile which defines the actions for this weapon. This
	 * additionally grants the actions to the owner which allows them to use
	 * all the actions in combat.
	 */
	
	void setCombatProfile(CreatureObject & owner, AiCreatureCombatProfile const & combatProfile);

	PersistentCrcString const & getCombatAction();

public:

	typedef uint32 Time;

	Archive::AutoDeltaVector<Time> m_singleUseActionList;
	Archive::AutoDeltaVector<Time> m_delayRepeatActionList;
	Archive::AutoDeltaVector<Time> m_instantRepeatActionList;
	AiCreatureCombatProfile const * m_combatProfile; // Not owned by this class

private:

	AiCreatureWeaponActions(AiCreatureWeaponActions const &);
	AiCreatureWeaponActions & operator =(AiCreatureWeaponActions const &);
};

// ======================================================================

#endif // INCLUDED_AiCreatureWeaponActions_H
