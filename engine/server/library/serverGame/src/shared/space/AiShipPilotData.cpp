// ======================================================================
//
// AiShipPilotData.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiShipPilotData.h"

#include "serverGame/ConfigServerGame.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedLog/Log.h"
#include "sharedUtility/DataTable.h"

#include <algorithm>
#include <map>

// ======================================================================

namespace AiShipPilotDataNamespace
{
	char const * const cs_pilotTypeFileName = "datatables/ship/pilottype.iff";
	std::string const cs_defaultPilotType = "default";

	typedef std::map<std::string, AiShipPilotData *> PilotTypePilotDataMap;
	PilotTypePilotDataMap ms_pilotTypePilotDataMap;

	AiShipPilotData ms_defaultPilotData;
}

using namespace AiShipPilotDataNamespace;

// ----------------------------------------------------------------------

AiShipPilotData::AiShipPilotData() :
	m_name(),
	m_skill(0.0f),
	m_aggression(0.5f),
	m_leashRadius(500.0f),
	m_aggroRadius(200.0f),
	m_nonCombatMaxSpeedPercent(0.5f),
	m_projectileFireDelay(0.0f),
	m_projectileFireAngle(0.0),
	m_projectileMissAngle(0.0f),
	m_projectileMissChance(0.0f),
	m_countermeasureReactionTime(2.0f),
	m_turretMissAngle(0.0f),
	m_turretMissChance(.5f),
	m_bomberMissilesPerBombingRun(1),
	m_fighterMissileLockOnTime(0.0f),
	m_fighterMissileLockOnAngle(0.0f),
	m_fighterMissileChanceToFirePercent(1.0f),
	m_fighterMissileFireDelay(0.0f),
	m_fighterChaseMaxTime(0.0f),
	m_fighterChaseMinThrottle(0.0f),
	m_fighterChaseSeperationTime(0.0f),
	m_fighterChaseMaxShots(0),
	m_fighterChaseMaxOnTailTime(0.0f),
	m_fighterEvadeMaxTime(0.0f),
	m_fighterEvadePositionUpdateDelay(0.0f),
	m_fighterEvadeAngle(0.0f)
{
}

// ----------------------------------------------------------------------

AiShipPilotData::~AiShipPilotData()
{
}

// ----------------------------------------------------------------------

void AiShipPilotData::install()
{
	DEBUG_FATAL(!ms_pilotTypePilotDataMap.empty(), ("AiShipPilotData::install - already installed"));
	ExitChain::add(AiShipPilotData::remove, "AiShipPilotDataNamespace::remove");

	ms_pilotTypePilotDataMap.clear();
	reload();
}

// ----------------------------------------------------------------------

void AiShipPilotData::remove()
{
	//-- Delete the pilot type to pilot data map
	std::for_each(ms_pilotTypePilotDataMap.begin(), ms_pilotTypePilotDataMap.end(), PointerDeleterPairSecond());
	ms_pilotTypePilotDataMap.clear();
}

// ----------------------------------------------------------------------

void AiShipPilotData::reload()
{
	//-- Load the pilot type data table, do not call ms_pilotTypePilotDataMap.clear() in this function, it breaks the reload.

	Iff iff;
	if (iff.open(cs_pilotTypeFileName, true))
	{
		DataTable dataTable;
		dataTable.load(iff);

		int const numberOfRows = dataTable.getNumRows();
		for (int row = 0; row < numberOfRows; ++row)
		{
			std::string const name = dataTable.getStringValue("name", row);

			if (ms_pilotTypePilotDataMap.find(name) == ms_pilotTypePilotDataMap.end())
			{
				AiShipPilotData * const pilotData = new AiShipPilotData;

				std::pair<PilotTypePilotDataMap::iterator, bool> result = ms_pilotTypePilotDataMap.insert(std::make_pair(name, pilotData));

				DEBUG_FATAL(!result.second, ("AiShipPilotData::reload: Failed to insert %s", name.c_str()));
				UNREF(result);
			}

			PilotTypePilotDataMap::iterator iterPilotData = ms_pilotTypePilotDataMap.find(name);

			if (iterPilotData != ms_pilotTypePilotDataMap.end())
			{
				AiShipPilotData & pilotData = *NON_NULL(iterPilotData->second);

				pilotData.m_name = name;
				pilotData.m_skill = dataTable.getFloatValue("skill", row);
				pilotData.m_aggression = dataTable.getFloatValue("aggression", row);
				pilotData.m_leashRadius = dataTable.getFloatValue("leashRadius", row);
				pilotData.m_aggroRadius = dataTable.getFloatValue("aggroRadius", row);
				pilotData.m_nonCombatMaxSpeedPercent = std::max(0.0f, dataTable.getFloatValue("nonCombatMaxSpeedPercent", row));
				pilotData.m_projectileFireDelay = std::max(0.1f, dataTable.getFloatValue("projectileFireDelay", row));
				pilotData.m_projectileFireAngle = convertDegreesToRadians(clamp(0.0f, dataTable.getFloatValue("projectileFireAngleDegrees", row), 360.0f));
				pilotData.m_projectileMissAngle = tan(convertDegreesToRadians(clamp(0.0f, dataTable.getFloatValue("projectileMissAngleDegrees", row), 10.0f)));
				pilotData.m_projectileMissChance = std::max(0.0f, dataTable.getFloatValue("projectileMissChance", row));
				pilotData.m_countermeasureReactionTime = dataTable.getFloatValue("countermeasureReactionTime", row);
				pilotData.m_turretMissAngle = tan(convertDegreesToRadians(clamp(0.0f, dataTable.getFloatValue("turretMissAngleDegrees", row), 20.0f)));
				pilotData.m_turretMissChance = dataTable.getFloatValue("turretMissChance", row);
				pilotData.m_bomberMissilesPerBombingRun = dataTable.getIntValue("bomberMissilesPerBombingRun", row);
				pilotData.m_fighterMissileLockOnTime = std::max(0.0f, dataTable.getFloatValue("fighterMissileLockOnTime", row));
				pilotData.m_fighterMissileLockOnAngle = convertDegreesToRadians(clamp(1.0f, dataTable.getFloatValue("fighterMissileLockOnAngleDegrees", row), 360.0f));
				pilotData.m_fighterMissileChanceToFirePercent = clamp(0.0f, dataTable.getFloatValue("fighterMissileChanceToFirePercent", row), 1.0f);
				pilotData.m_fighterMissileFireDelay = std::max(1.0f, dataTable.getFloatValue("fighterMissileFireDelay", row));
				pilotData.m_fighterChaseMaxTime = std::max(5.0f, dataTable.getFloatValue("fighterChaseMaxTime", row));
				pilotData.m_fighterChaseMinThrottle = clamp(0.0f, dataTable.getFloatValue("fighterChaseMinThrottle", row), 1.0f);
				pilotData.m_fighterChaseSeperationTime = std::max(0.0f, dataTable.getFloatValue("fighterChaseSeperationTime", row));
				pilotData.m_fighterChaseMaxShots = std::max(1, dataTable.getIntValue("fighterChaseMaxShots", row));
				pilotData.m_fighterChaseMaxOnTailTime = std::max(0.0f, dataTable.getFloatValue("fighterChaseMaxOnTailTime", row));
				pilotData.m_fighterEvadeMaxTime = std::max(5.0f, dataTable.getFloatValue("fighterEvadeMaxTime", row));
				pilotData.m_fighterEvadePositionUpdateDelay = std::max(0.0f, dataTable.getFloatValue("fighterEvadePositionUpdateDelay", row));
				pilotData.m_fighterEvadeAngle = convertDegreesToRadians(clamp(0.0f, dataTable.getFloatValue("fighterEvadeAngleDegrees", row), 360.0f));

				DEBUG_FATAL(!WithinRangeInclusiveInclusive(0.0f, pilotData.m_skill, 1.0f), ("AiShipPilotData::m_skill (%.2f) is out of range (0 to 1).", pilotData.m_skill));
				DEBUG_FATAL(!WithinRangeInclusiveInclusive(0.0f, pilotData.m_aggression, 1.0f), ("AiShipPilotData::m_aggression (%.2f) is out of range (0 to 1).", pilotData.m_aggression));
			}
		}

		//-- Extract the default pilot data
		PilotTypePilotDataMap::iterator iter = ms_pilotTypePilotDataMap.find(cs_defaultPilotType);
		if (iter != ms_pilotTypePilotDataMap.end())
			ms_defaultPilotData = *iter->second;
		else
			DEBUG_WARNING(true, ("AiShipPilotData::install: file %s does not specify pilotType %s", cs_pilotTypeFileName, cs_defaultPilotType.c_str()));
	}
	else
		DEBUG_WARNING(true, ("AiShipPilotData::install: could not open file %s", cs_pilotTypeFileName));
}

// ----------------------------------------------------------------------

AiShipPilotData const & AiShipPilotData::getPilotData(std::string const & pilotType)
{
	//-- Find the pilot data corresponding to the pilot type
	PilotTypePilotDataMap::iterator iter = ms_pilotTypePilotDataMap.find(pilotType);
	if (iter != ms_pilotTypePilotDataMap.end())
	{
		return *iter->second;
	}
	else
	{
		//-- Use default pilot data if pilot type was not found
		DEBUG_WARNING(true, ("Could not find pilotType %s in %s, using default", pilotType.c_str(), cs_pilotTypeFileName));
		return ms_defaultPilotData;
	}
}

// ----------------------------------------------------------------------

AiShipPilotData const & AiShipPilotData::getDefaultPilotData()
{
	return ms_defaultPilotData;
}

// ======================================================================
