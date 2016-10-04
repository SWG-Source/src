// ======================================================================
// 
// Squad.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/Squad.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/Formation.h"
#include "serverGame/SpaceSquad.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedLog/Log.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedUtility/DataTable.h"

#include <map>
#include <vector>

// ======================================================================
//
// SquadNamespace
//
// ======================================================================

namespace SquadNamespace
{
	typedef std::map<PersistentCrcString, int> FormationPriorityList;

	bool s_installed = false;
	FormationPriorityList s_formationPriorityList;
}

using namespace SquadNamespace;

// ======================================================================
//
// Squad
//
// ======================================================================

// ----------------------------------------------------------------------
void Squad::install()
{
	DEBUG_FATAL(s_installed, ("Already installed"));

	// Create the data table

	Iff iff;

	if (iff.open("datatables/space_mobile/space_mobile.iff", true))
	{
		DataTable dataTable;
		dataTable.load(iff);

		int const rowCount = dataTable.getNumRows();

		for (int row = 0; row < rowCount; ++row)
		{
			PersistentCrcString const shipName(dataTable.getStringValue("strIndex", row), false);
			int const formationPriority = dataTable.getIntValue("formationPriority", row);
			
			IGNORE_RETURN(s_formationPriorityList.insert(std::make_pair(shipName, formationPriority)));

			LOGC((ConfigServerGame::isSpaceAiLoggingEnabled() && formationPriority  <= 0), "space_debug_ai", ("Squad::install() ERROR: Invalid formationPriority(%d) specified for shipName(%s)", formationPriority, shipName.getString()));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to load space_mobile.iff to retrieve formation priorities!"));
	}

	ExitChain::add(&remove, "Squad::remove");

	s_installed = true;
}

// ----------------------------------------------------------------------
void Squad::remove()
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	s_formationPriorityList.clear();

	s_installed = false;
}

// ----------------------------------------------------------------------
Squad::Squad()
 : m_leaderOffsetPosition_l()
 , m_id(0)
 , m_unitMap(new UnitMap)
 , m_formation()
 , m_leader()
 , m_squadPosition_w()
 , m_squadPositionUpdateTimer(5.0f)
{
	m_squadPositionUpdateTimer.setElapsedTime(m_squadPositionUpdateTimer.getExpireTime());
}

// ----------------------------------------------------------------------
Squad::~Squad()
{
	delete m_unitMap;
}

// ----------------------------------------------------------------------
void Squad::alter(float const deltaSeconds)
{
	if (   m_formation.isDirty()
	    && (m_unitMap->size() > 1))
	{
		buildFormation();
	}

	if (m_squadPositionUpdateTimer.updateNoReset(deltaSeconds))
	{
		calculateSquadPosition_w();
	}
}

// ----------------------------------------------------------------------
bool Squad::isEmpty() const
{
	return m_unitMap->empty();
}

// ----------------------------------------------------------------------
int Squad::getUnitCount() const
{
	return static_cast<int>(m_unitMap->size());
}

// ----------------------------------------------------------------------
Squad::UnitMap const & Squad::getUnitMap() const
{
	return *m_unitMap;
}

// ----------------------------------------------------------------------
void Squad::getUnitList(UnitList & unitList)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("Squad::getUnitList() className(%s) squadId(%d)", getClassName(), m_id));
	LOGC((ConfigServerGame::isSpaceAiLoggingEnabled() && m_unitMap->empty()), "space_debug_ai", ("Squad::getUnitList() className(%s) squadId(%d) The current squad list should not be empty!", getClassName(), m_id));

	unitList.clear();
	unitList.reserve(m_unitMap->size());

	UnitMap::const_iterator iterUnitMap = m_unitMap->begin();

	for (; iterUnitMap != m_unitMap->end(); ++iterUnitMap)
	{
		unitList.push_back(iterUnitMap->first);
	}
}

// ----------------------------------------------------------------------
int Squad::getId() const
{
	return m_id;
}

// ----------------------------------------------------------------------
void Squad::addUnit(NetworkId const & unit)
{
	// If this unit is the only unit in the squad, it is assigned as the squad leader

	UnitMap::const_iterator iterUnitMap = m_unitMap->find(CachedNetworkId(unit));

	if (iterUnitMap == m_unitMap->end())
	{
		bool const leader = m_unitMap->empty();

		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("Squad::addUnit() className(%s) squadId(%d) unit(%s) leader(%s) squadSize(%d+1)", getClassName(), m_id, unit.getValueString().c_str(), leader ? "yes" : "no", m_unitMap->size()));

		//-- Add the unit to the new squad

		IGNORE_RETURN(m_unitMap->insert(std::make_pair(CachedNetworkId(unit), &PersistentCrcString::empty)));

		if (leader)
		{
			bool const rebuildFormation = true;
			IGNORE_RETURN(setLeader(unit, rebuildFormation));
		}

		m_formation.markDirty();

		onAddUnit(unit);
	}
	else
	{
#ifdef _DEBUG
		FormattedString<1024> fs;
		char const * const text = fs.sprintf("Squad::addUnit() className(%s) squadId(%d) unit(%s) is already in the squad, not re-adding", getClassName(), m_id, unit.getValueString().c_str());
		DEBUG_WARNING(true, (text));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
	}

//#ifdef _DEBUG
//	if (ConfigServerGame::isSpaceAiLoggingEnabled())
//	{
//		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("Squad::addUnit() squadId(%d) newSize(%u)", m_id, m_unitMap->size()));
//		iterUnitMap = m_unitMap->begin();
//		int index = 1;
//
//		for (; iterUnitMap != m_unitMap->end(); ++iterUnitMap)
//		{
//			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("Squad::addUnit() squadId(%d) [%2d] unit(%s)", m_id, index, iterUnitMap->getValueString().c_str()));
//			++index;
//		}
//	}
//#endif // _DEBUG
}

// ----------------------------------------------------------------------
void Squad::removeUnit(NetworkId const & unit)
{
	UnitMap::iterator iterUnitMap = m_unitMap->find(CachedNetworkId(unit));

	if (iterUnitMap != m_unitMap->end())
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("Squad::removeUnit() className(%s) squadId(%d) unit(%s) squadSize(%d-1) leader(%s)", getClassName(), m_id, unit.getValueString().c_str(),  m_unitMap->size(), m_leader.getValueString().c_str()));

		bool const wasLeader = (iterUnitMap->first == m_leader);

		m_unitMap->erase(iterUnitMap);

		// See if we need to assign a new squad leader

		if (wasLeader)
		{
			bool const rebuildFormation = false;

		    if (!isEmpty())
			{
				IGNORE_RETURN(setLeader(m_unitMap->begin()->first, rebuildFormation));
			}
			else
			{
				IGNORE_RETURN(setLeader(NetworkId::cms_invalid, rebuildFormation));
			}
		}

		m_formation.markDirty();

		onRemoveUnit();
	}
	else
	{
#ifdef _DEBUG
		FormattedString<1024> fs;
		char const * const text = fs.sprintf("Squad::removeUnit() ERROR: className(%s) squadId(%d) Unable to find the unit(%s)", getClassName(), m_id, unit.getValueString().c_str());
		DEBUG_WARNING(true, (text));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
	}

//#ifdef _DEBUG
//	if (ConfigServerGame::isSpaceAiLoggingEnabled())
//	{
//		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("Squad::removeUnit() squadId(%d) newSize(%u)", m_id, m_unitMap->size()));
//		iterUnitMap = m_unitMap->begin();
//		int index = 1;
//
//		for (; iterUnitMap != m_unitMap->end(); ++iterUnitMap)
//		{
//			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("Squad::removeUnit() squadId(%d) [%2d] unit(%s)", m_id, index, iterUnitMap->getValueString().c_str()));
//			++index;
//		}
//	}
//#endif // _DEBUG
}


// ----------------------------------------------------------------------
Formation & Squad::getFormation()
{
	return m_formation;
}

// ----------------------------------------------------------------------
bool Squad::setLeader(NetworkId const & unit, bool const rebuildFormation)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("Squad::setLeader() className(%s) squadId(%d) unit(%s) rebuildFormation(%s)", getClassName(), m_id, unit.getValueString().c_str(), rebuildFormation ? "yes" : "no"));

	if (   isEmpty()
	    && (unit == NetworkId::cms_invalid))
	{
		m_leader = CachedNetworkId::cms_cachedInvalid;

		return true;
	}

	bool result = false;
	UnitMap::iterator iterUnitMap = m_unitMap->find(CachedNetworkId(unit));

	if (iterUnitMap != m_unitMap->end())
	{
		result = true;

		NetworkId oldLeader(m_leader);
		m_leader = CachedNetworkId(unit);
		onNewLeader(oldLeader);

		if (rebuildFormation)
		{
			m_formation.markDirty();
		}
	}
	else
	{
		m_leader = CachedNetworkId::cms_cachedInvalid;

#ifdef _DEBUG
		FormattedString<1024> fs;
		char const * const text = fs.sprintf("Squad::setLeader() ERROR: className(%s) Trying to set a leader(%s) who is not a member of the squad.", getClassName(), unit.getValueString().c_str());
		DEBUG_WARNING(true, (text));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
	}

	return result;
}

// ----------------------------------------------------------------------
CachedNetworkId const & Squad::getLeader() const
{
	return m_leader;
}

// ----------------------------------------------------------------------
Vector const & Squad::getLeaderOffsetPosition_l() const
{
	return m_leaderOffsetPosition_l;
}

// ----------------------------------------------------------------------
void Squad::combineWith(Squad & squad)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("Squad::combineWith() className(%s) squadId(%d) combineWithSquadId(%d)", getClassName(), m_id, squad.getId()));
	LOGC((ConfigServerGame::isSpaceAiLoggingEnabled() && squad.isEmpty()), "space_debug_ai", ("Squad::combineWith() className(%s) squadId(%d) combineWithSquadId(%d) Why are we combining with an empty squad?", getClassName(), m_id, squad.getId()));

	//-- Add all the units to this squad

	UnitMap::const_iterator iterUnitMap = squad.getUnitMap().begin();

	for (; iterUnitMap != squad.getUnitMap().end(); ++iterUnitMap)
	{
		NetworkId const & unit = iterUnitMap->first;
		addUnit(unit);
	}
}

// ----------------------------------------------------------------------
void Squad::setId(int const id)
{
	DEBUG_FATAL((id == 0), ("0 is an invalid squad id."));

	m_id = id;
}

// ----------------------------------------------------------------------
void Squad::buildFormation()
{
	// Prioritize the ships to enter the formation first, the lower the priority
	// the more towards the center the ships are located

	typedef std::multimap<int, std::pair<NetworkId, PersistentCrcString const *> > SortedUnitList;
	SortedUnitList sortedUnitList;

	UnitMap::const_iterator iterUnitMap = m_unitMap->begin();

	for (; iterUnitMap != m_unitMap->end(); ++iterUnitMap)
	{
		NetworkId const & unit = iterUnitMap->first;
		PersistentCrcString const * unitName = iterUnitMap->second;
		DEBUG_FATAL((unitName == nullptr), ("The unit should have a non-nullptr name."));

		if (unit == m_leader)
		{
			IGNORE_RETURN(sortedUnitList.insert(std::make_pair(0, std::make_pair(unit, unitName))));
		}
		else
		{
			FormationPriorityList::const_iterator iterFormationPriorityList = s_formationPriorityList.find(*unitName);

			int priority = 10;

			if (iterFormationPriorityList != s_formationPriorityList.end())
			{
				priority = iterFormationPriorityList->second;
			}
			else
			{
#ifdef DEBUG
				Object * const unitObject = NetworkIdManager::getObjectById(unit);
				DEBUG_WARNING(true, ("className(%s) Unable to find the ship(%s) [%s] in the formation priority list.", 
					getClassName(), unitName->getString(), unitObject ? unitObject->getDebugInformation().c_str() : "nullptr"));
#endif
			}

			IGNORE_RETURN(sortedUnitList.insert(std::make_pair(priority, std::make_pair(unit, unitName))));
		}
	}

	// Store the sorted unit list

	m_unitMap->clear();
	SortedUnitList::reverse_iterator iterSortedUnitList = sortedUnitList.rbegin();

	for (; iterSortedUnitList != sortedUnitList.rend(); ++iterSortedUnitList)
	{
		NetworkId const & unit = iterSortedUnitList->second.first;
		PersistentCrcString const * unitName = iterSortedUnitList->second.second;

		IGNORE_RETURN(m_unitMap->insert(std::make_pair(CachedNetworkId(unit), unitName)));
	}

	m_formation.build(*this);
}

// ----------------------------------------------------------------------
void Squad::setUnitFormationPosition_l(NetworkId const & unit, Vector const & position_l)
{
	onSetUnitFormationPosition_l(unit, position_l);
}

// ----------------------------------------------------------------------
Vector const & Squad::getSquadPosition_w()
{
	if (m_squadPositionUpdateTimer.isExpired())
	{
		calculateSquadPosition_w();
	}

	return m_squadPosition_w;
}

// ----------------------------------------------------------------------
void Squad::calculateSquadPosition_w()
{
	if (m_squadPositionUpdateTimer.getElapsedRatio() > 0.0f)
	{
		// The position has already been calculated this frame
		return;
	}

	m_squadPositionUpdateTimer.reset();

	UnitMap::const_iterator iterUnitMap = getUnitMap().begin();

	m_squadPosition_w = Vector::zero;

	for (; iterUnitMap != getUnitMap().end(); ++iterUnitMap)
	{
		NetworkId const & unit = (*iterUnitMap).first;

		Object * const object = NetworkIdManager::getObjectById(unit);

		if (object != nullptr)
		{
			m_squadPosition_w += object->getPosition_w();
		}
		else
		{
#ifdef _DEBUG
			FormattedString<1024> fs;
			char const * const text = fs.sprintf("Squad::calculateSquadPosition_w() unit(%s) did not resolve to a valid object", unit.getValueString().c_str());
			DEBUG_WARNING(true, (text));
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
		}
	}

	if (getUnitMap().size() > 1)
	{
		m_squadPosition_w /= static_cast<float>(getUnitMap().size());
	}
	else
	{
#ifdef _DEBUG
		FormattedString<1024> fs;
		char const * const text = fs.sprintf("Squad::calculateSquadPosition_w() Why is there a zero size squad?");
		DEBUG_WARNING(true, (text));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", (text));
#endif // _DEBUG
	}
}

// ----------------------------------------------------------------------
bool Squad::contains(NetworkId const & unit) const
{
	return (m_unitMap->find(CachedNetworkId(unit)) != m_unitMap->end());
}

// ======================================================================
