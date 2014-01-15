// ======================================================================
//
// AiCreatureCombatProfile_Action.cpp
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiCreatureCombatProfile_Action.h"

// ======================================================================
//
// AiCreatureCombatProfile_Action
//
// ======================================================================

// ----------------------------------------------------------------------
AiCreatureCombatProfile::Action::Action(PersistentCrcString const & name, PersistentCrcString const & baseName, time_t useTime, float useChance, UseCount useCount)
 : m_name(name)
 , m_baseName(baseName)
 , m_useTime(useTime)
 , m_useChance(useChance)
 , m_useCount(useCount)
{
}

// ======================================================================
