// ======================================================================
//
// AiCreatureData.h
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_AiCreatureData_H
#define INCLUDED_AiCreatureData_H

#include "sharedFoundation/PersistentCrcString.h"
#include "serverGame/CreatureObject.h"

class CrcString;

// ======================================================================
class AiCreatureData
{
public:

	AiCreatureData();

	static void install();

	static AiCreatureData const & getCreatureData(CrcString const & creatureName);
	static AiCreatureData const & getDefaultCreatureData();
	static std::string getWeaponTemplateName(CrcString const & weaponName);

public:

	enum DeathBlow
	{
		  DB_no
		, DB_yes
		, DB_instant
	};

	PersistentCrcString const * m_name;
	float m_movementSpeedPercent;
	PersistentCrcString m_primaryWeapon;
	PersistentCrcString m_secondaryWeapon;
	float m_aggressive;
	bool m_stalker;
	float m_assist;
	DeathBlow m_deathBlow;
	PersistentCrcString m_primarySpecials;
	PersistentCrcString m_secondarySpecials;
	CreatureObject::Difficulty m_difficulty;
};

// ======================================================================

#endif // INCLUDED_AiCreatureData_H
