// ======================================================================
//
// AiShipPilotData.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_AiShipPilotData_H
#define INCLUDED_AiShipPilotData_H

// ======================================================================

struct AiShipPilotData
{
public:

	AiShipPilotData();
	~AiShipPilotData();

	static void install();
	static void remove();
	static void reload();

	static AiShipPilotData const & getPilotData(std::string const & pilotType);
	static AiShipPilotData const & getDefaultPilotData();

public:

	std::string m_name;
	float m_skill;
	float m_aggression;
	float m_leashRadius;
	float m_aggroRadius;
	float m_nonCombatMaxSpeedPercent;
	float m_projectileFireDelay;
	float m_projectileFireAngle;
	float m_projectileMissAngle;
	float m_projectileMissChance;
	float m_countermeasureReactionTime;
	float m_turretMissAngle;
	float m_turretMissChance;
	int m_bomberMissilesPerBombingRun;
	float m_fighterMissileLockOnTime;
	float m_fighterMissileLockOnAngle;
	float m_fighterMissileChanceToFirePercent;
	float m_fighterMissileFireDelay;
	float m_fighterChaseMaxTime;
	float m_fighterChaseMinThrottle;
	float m_fighterChaseSeperationTime;
	int m_fighterChaseMaxShots;
	float m_fighterChaseMaxOnTailTime;
	float m_fighterEvadeMaxTime;
	float m_fighterEvadePositionUpdateDelay;
	float m_fighterEvadeAngle;
};

// ======================================================================

#endif
