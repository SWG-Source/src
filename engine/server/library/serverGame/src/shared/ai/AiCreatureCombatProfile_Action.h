// ======================================================================
//
// AiCreatureCombatProfile_Action.h
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_AiCreatureCombatProfile_Action_H
#define INCLUDED_AiCreatureCombatProfile_Action_H

class PersistentCrcString;

#include "serverGame/AiCreatureCombatProfile.h"

// ======================================================================
struct AiCreatureCombatProfile::Action
{
	enum UseCount
	{
		UC_once,
		UC_infinite
	};

	Action(PersistentCrcString const & name, PersistentCrcString const & baseName, time_t useTime, float useChance, UseCount useCount);

	PersistentCrcString const & m_name;
	//@TODO The following variable, m_baseName, is no longer used and should probably be removed -- ARH
	PersistentCrcString const & m_baseName; // This is the name of the action without any underscores in it '_'
	time_t const m_useTime;
	float const m_useChance;
	UseCount const m_useCount;

private:

	Action();
	Action(Action const &);
	Action & operator =(Action const &);
};

// ======================================================================

#endif // INCLUDED_AiCreatureCombatProfile_Action_H
