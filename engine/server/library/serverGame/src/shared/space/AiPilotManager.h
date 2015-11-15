// ======================================================================
//
// AiPilotManager.h
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_AiPilotManager_H
#define INCLUDED_AiPilotManager_H

// ======================================================================

struct AiShipPilotData;

class AiPilotManager
{
public:

	AiPilotManager();
	~AiPilotManager();

	static void install();
	static void remove();


	static void getPilotData(AiShipPilotData const & pilot, AiPilotManager & pilotData);

	float m_weaponBaseFireRate;
	float m_weaponMissileFireRate;
	float m_weaponSelectionSkill;
	float m_targetReactionTime;
	float m_targetAimZoneUpdate;
	float m_targetDesiredHitsPerSecond;
	float m_targetBehind; // RLS - this is converted to radians as an optimization.
	float m_targetCollisionCourse; // RLS - this is converted to radians as an optimization.
	float m_hitsToEvadeBase;
	float m_hitsDecayTimer;
	float m_boosterUseMinimumRange;
	float m_maneuverPaths;
	float m_maneuverTimeMax;
	float m_maneuverPathComplexity;
   	float m_minimumThrottle;
};

// ======================================================================

#endif
