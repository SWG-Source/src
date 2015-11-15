// ======================================================================
//
// AiPilotManager.cpp
//
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiPilotManager.h"


#include "serverGame/AiShipPilotData.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Range.h"
#include "sharedUtility/DataTable.h"

#include <algorithm>
#include <map>

// ======================================================================

namespace AiPilotManagerNamespace
{
	enum ColumnData
	{
		CD_property,
		CD_isDegrees,
		CD_useSkill,
		CD_linearScale,
		CD_novice,
		CD_expert,
		CD_autoAdjust
	};

	enum RowIndex
	{
		RI_weaponBaseFireRate,
		RI_weaponMissileFireRate,
		RI_weaponSelectionSkill,
		RI_targetReactionTime,
		RI_targetAimZoneUpdate,
		RI_targetDesiredHitsPerSecond,
		RI_targetBehind,
		RI_targetCollisionCourse,
		RI_hitsToEvadeBase,
		RI_hitsDecayTimer,
		RI_boosterUseMinimumRange,
		RI_maneuverPaths,
		RI_maneuverTimeMax,
		RI_maneuverPathComplexity,
		RI_minimumThrottle,
		RI_count
	};

	struct AiPilotManagerData
	{
		AiPilotManagerData(RowIndex const rowIndex, const std::string & propertyName) :
			m_row(rowIndex),
			m_property(propertyName),
			m_isDegrees(false),
			m_useSkill(true),
			m_linearScale(true),
			m_skillRange(0.0f, 1.0f),
			m_autoAdjust(false)
		{
		}

		AiPilotManagerData() :
			m_row(RI_count),
			m_property("fix me"),
			m_isDegrees(false),
			m_useSkill(true),
			m_linearScale(true),
			m_skillRange(0.0f, 1.0f),
			m_autoAdjust(false)
		{
		}

		void convertRangeToRadians()
		{
			m_skillRange.set(convertDegreesToRadians(m_skillRange.getMin()), convertDegreesToRadians(m_skillRange.getMax()));
		}

		RowIndex m_row;
		std::string m_property;
		bool m_isDegrees;
		bool m_useSkill;
		bool m_linearScale;
		Range m_skillRange;
		bool m_autoAdjust;
	};

	#define DEFINEROW( rowName ) AiPilotManagerData(RI_##rowName, #rowName)

	static AiPilotManagerData ms_pilotManagerData[RI_count] =
	{
		DEFINEROW(weaponBaseFireRate),
		DEFINEROW(weaponMissileFireRate),
		DEFINEROW(weaponSelectionSkill),
		DEFINEROW(targetReactionTime),
		DEFINEROW(targetAimZoneUpdate),
		DEFINEROW(targetDesiredHitsPerSecond),
		DEFINEROW(targetBehind),
		DEFINEROW(targetCollisionCourse),
		DEFINEROW(hitsToEvadeBase),
		DEFINEROW(hitsDecayTimer),
		DEFINEROW(boosterUseMinimumRange),
		DEFINEROW(maneuverPaths),
		DEFINEROW(maneuverTimeMax),
		DEFINEROW(maneuverPathComplexity),
		DEFINEROW(minimumThrottle)
	};

#define GETRANGE(pilot, pilotData, rowName) { \
	RowIndex row = static_cast<RowIndex>(RI_##rowName); \
	DEBUG_FATAL(ms_pilotManagerData[row].m_property != #rowName, ("AiPilotManager - GETRANGE( %s != %s)", ms_pilotManagerData[row].m_property.c_str(), #rowName)); \
	pilotData.m_##rowName = (ms_pilotManagerData[row].m_skillRange.linearInterpolate(ms_pilotManagerData[row].m_useSkill ? pilot.m_skill : pilot.m_aggression)); }
	
	char const * const cs_pilotTypeFileName = "datatables/ship/pilotmanager.iff";

	bool ms_pilotManagerInstalled = false;
}

using namespace AiPilotManagerNamespace;

// ----------------------------------------------------------------------

AiPilotManager::AiPilotManager() :
m_weaponBaseFireRate(1.0f),
m_weaponMissileFireRate(1.0f),
m_weaponSelectionSkill(1.0f),
m_targetReactionTime(0.2f),
m_targetAimZoneUpdate(1.0f),
m_targetDesiredHitsPerSecond(1.0f),
m_targetBehind(0.2f),
m_targetCollisionCourse(0.2f),
m_hitsToEvadeBase(5.0f),
m_hitsDecayTimer(25.0f),
m_boosterUseMinimumRange(50.0f),
m_maneuverPaths(2),
m_maneuverTimeMax(30.0f),
m_maneuverPathComplexity(1.0f),
m_minimumThrottle(0.5f)
{
}

// ----------------------------------------------------------------------

AiPilotManager::~AiPilotManager()
{
}

// ----------------------------------------------------------------------

void AiPilotManager::install()
{
	DEBUG_FATAL(ms_pilotManagerInstalled, ("AiPilotManager::install - already installed"));
	ExitChain::add(AiPilotManager::remove, "AiPilotManager::remove");

	//-- Load the pilot type data table
	{
		Iff iff;
		if (iff.open(cs_pilotTypeFileName, true))
		{
			ms_pilotManagerInstalled = true;

			DataTable dataTable;
			dataTable.load(iff);

			int const numberOfRows = dataTable.getNumRows();
			for (int row = 0; row < numberOfRows; ++row)
			{
				AiPilotManagerData & managedData = ms_pilotManagerData[row];
				
				// Verify the property names.
#ifdef _DEBUG
				std::string const & property = dataTable.getStringValue(static_cast<int>(CD_property), row);
				DEBUG_FATAL(managedData.m_property != property, ("AiPilotManager::install: Table rows in [%s] do not match at row %d. I expected %s and received %s.", cs_pilotTypeFileName, row, managedData.m_property.c_str(), property.c_str()));
#endif
				
				// Get the skill range.
				float const noviceSkillValue = dataTable.getFloatValue(static_cast<int>(CD_novice), row);
				float const expertSkillValue = dataTable.getFloatValue(static_cast<int>(CD_expert), row);
				managedData.m_skillRange.set(noviceSkillValue, expertSkillValue);
				
				// Check to see if we should auto adjust. (not used)
				managedData.m_autoAdjust = dataTable.getIntValue(static_cast<int>(CD_autoAdjust), row) > 0;
				
				// Should we use a linear or non-linear scale.
				managedData.m_linearScale = dataTable.getIntValue(static_cast<int>(CD_linearScale), row) > 0;
				
				// Should we use the skill percentage or the aggression.
				managedData.m_useSkill = dataTable.getIntValue(static_cast<int>(CD_useSkill), row) > 0;
				
				// Should we use the skill percentage or the aggression.
				managedData.m_isDegrees = dataTable.getIntValue(static_cast<int>(CD_isDegrees), row) > 0;
				
				if (managedData.m_isDegrees)
				{
					managedData.convertRangeToRadians();
				}
			}
		}
		else
			DEBUG_WARNING(true, ("AiPilotManager::install: could not open file %s", cs_pilotTypeFileName));
	}
}

// ----------------------------------------------------------------------

void AiPilotManager::remove()
{
	ms_pilotManagerInstalled = false;
}

// ----------------------------------------------------------------------

void AiPilotManager::getPilotData(AiShipPilotData const & pilot, AiPilotManager & pilotManagerData)
{
	GETRANGE(pilot, pilotManagerData, weaponBaseFireRate);
	GETRANGE(pilot, pilotManagerData, weaponMissileFireRate);
	GETRANGE(pilot, pilotManagerData, weaponSelectionSkill);
	GETRANGE(pilot, pilotManagerData, targetReactionTime);
	GETRANGE(pilot, pilotManagerData, targetAimZoneUpdate);
	GETRANGE(pilot, pilotManagerData, targetDesiredHitsPerSecond);
	GETRANGE(pilot, pilotManagerData, targetBehind);
	GETRANGE(pilot, pilotManagerData, targetCollisionCourse);
	GETRANGE(pilot, pilotManagerData, hitsToEvadeBase);
	GETRANGE(pilot, pilotManagerData, hitsDecayTimer);
	GETRANGE(pilot, pilotManagerData, boosterUseMinimumRange);
	GETRANGE(pilot, pilotManagerData, maneuverPaths);
	GETRANGE(pilot, pilotManagerData, maneuverTimeMax);
	GETRANGE(pilot, pilotManagerData, maneuverPathComplexity);
	GETRANGE(pilot, pilotManagerData, minimumThrottle);
}


// ======================================================================
