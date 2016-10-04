// ======================================================================
//
// SpaceSquadManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/SpaceSquadManager.h"

#include "serverGame/AiShipController.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/SpaceSquad.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedLog/Log.h"

#include <map>

// ======================================================================
//
// SpaceSquadManagerNamespace
//
// ======================================================================

namespace SpaceSquadManagerNamespace
{
	typedef std::map<int, SpaceSquad *> SquadList;

	SquadList s_squadList;
	int s_nextSquadId = 1;

#ifdef _DEBUG
	void verifySquads();
	typedef std::vector<NetworkId> NetworkIdList;
	NetworkIdList s_debugUnitList;
#endif // _DEBUG
}

using namespace SpaceSquadManagerNamespace;

#ifdef _DEBUG
// ----------------------------------------------------------------------
void SpaceSquadManagerNamespace::verifySquads()
{
	{
		// Build a list of all the units

		s_debugUnitList.clear();
		SquadList::const_iterator iterSquadList = s_squadList.begin();

		for (; iterSquadList != s_squadList.end(); ++iterSquadList)
		{
			SpaceSquad const & squad = *NON_NULL(iterSquadList->second);
			SpaceSquad::UnitMap const & unitMap = squad.getUnitMap();
			SpaceSquad::UnitMap::const_iterator iterUnitMap = unitMap.begin();
			
			for (; iterUnitMap != unitMap.end(); ++iterUnitMap)
			{
				CachedNetworkId const & unit = iterUnitMap->first;

				s_debugUnitList.push_back(unit);
			}
		}
	}

	{
		// Make sure each unit is in exactly 1 squad

		NetworkIdList::const_iterator iterDebugUnitList = s_debugUnitList.begin();

		for (; iterDebugUnitList != s_debugUnitList.end(); ++iterDebugUnitList)
		{
			NetworkId const & unit = (*iterDebugUnitList);
			int squadCount = 0;

			SquadList::const_iterator iterSquadList = s_squadList.begin();

			for (; iterSquadList != s_squadList.end(); ++iterSquadList)
			{
				SpaceSquad const & squad = *NON_NULL(iterSquadList->second);

				if (squad.contains(unit))
				{
					++squadCount;
				}
			}

			DEBUG_WARNING((squadCount > 1), ("SpaceSquadManagerNamespace::verifySquads() unit(%s) ERROR: Unit is in more than one squad! attackSquadCount(%d)", unit.getValueString().c_str()));
			DEBUG_WARNING((squadCount != 1), ("SpaceSquadManagerNamespace::verifySquads() unit(%s) ERROR: Unit is not in an squad!", unit.getValueString().c_str()));
		}
	}
}
#endif // _DEBUG

// ======================================================================
//
// SpaceSquadManager
//
// ======================================================================

// ----------------------------------------------------------------------
void SpaceSquadManager::install()
{
	ExitChain::add(&remove, "SpaceSquadManager::remove");
}

// ----------------------------------------------------------------------
void SpaceSquadManager::remove()
{
	// Make sure all the squads were cleaned up

	SquadList::const_iterator iterSquadList = s_squadList.begin();

	for (; iterSquadList != s_squadList.end(); ++iterSquadList)
	{
		WARNING(!iterSquadList->second->isEmpty(), ("SpaceSquadManager::remove() The squad(%d) is not empty(%u), this is a sign of a reference counting problem.", iterSquadList->first, iterSquadList->second->getUnitCount()));
	}

	WARNING(!s_squadList.empty(), ("SpaceSquadManager::remove() The squad list is not empty(%u), this is a sign of a reference counting problem.", s_squadList.size()));
}

// ----------------------------------------------------------------------
void SpaceSquadManager::alter(float const deltaTime)
{
	SquadList::iterator iterSquadList = s_squadList.begin();

	for (; iterSquadList != s_squadList.end();)
	{
		// Purge squads with no units

		if (iterSquadList->second->isEmpty())
		{
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquadManager::alter() Purging empty squad(%d) totalSquadCount(%u)", iterSquadList->first, s_squadList.size() - 1));

			delete iterSquadList->second;

			s_squadList.erase(iterSquadList++);
		}
		else
		{
			iterSquadList->second->alter(deltaTime);
			++iterSquadList;
		}
	}

#ifdef _DEBUG
	verifySquads();
#endif // _DEBUG
}

// ----------------------------------------------------------------------
int SpaceSquadManager::createSquadId()
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquadManager::createSquadId() newSquadId(%d)", s_nextSquadId));

	int const newId = s_nextSquadId;

	if (++s_nextSquadId == 0)
	{
		s_nextSquadId = 1;
	}

	return newId;
}

// ----------------------------------------------------------------------
SpaceSquad * SpaceSquadManager::createSquad()
{
	int const squadId = createSquadId();

	return createSquad(squadId);
}

// ----------------------------------------------------------------------
SpaceSquad * SpaceSquadManager::createSquad(int const squadId)
{
	FATAL((s_squadList.find(squadId) != s_squadList.end()), ("Trying to create a squad(%d) that already exists.", squadId));

	SpaceSquad * const squad = new SpaceSquad;

	squad->setId(squadId);

	IGNORE_RETURN(s_squadList.insert(std::make_pair(squadId, squad)));

	return squad;
}

// ----------------------------------------------------------------------
bool SpaceSquadManager::setSquadId(NetworkId const & unit, int const newSquadId)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquadManager::setSquadId() unit(%s) newSquadId(%d)", unit.getValueString().c_str(), newSquadId));

	bool result = false;
	AiShipController * const aiShipController = AiShipController::getAiShipController(unit);

	if (aiShipController != nullptr)
	{
		result = true;

		//-- See if the unit is already in the requested squad

		SpaceSquad const & oldSquad = aiShipController->getSquad();

		if (oldSquad.getId() != newSquadId)
		{
			SquadList::iterator iterSquadList = s_squadList.find(newSquadId);

			if (iterSquadList != s_squadList.end())
			{
				//-- The squad already existed, add the unit

				SpaceSquad * const newSquad = iterSquadList->second;

				newSquad->addUnit(unit);
			}
			else
			{
				//-- The squad did not exist, create it and add the unit

				SpaceSquad * const newSquad = SpaceSquadManager::createSquad(newSquadId);

				newSquad->addUnit(unit);
			}
		}
		else
		{
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquadManager::setSquadId() unit(%s) newSquadId(%d) ERROR: The unit is already in the requested squad.", unit.getValueString().c_str(), newSquadId));
		}
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpaceSquadManager::setSquadId() unit(%s) newSquadId(%d) ERROR: Unable to resolve unit to an AiShipController.", unit.getValueString().c_str(), newSquadId));
	}

	return result;
}

// ----------------------------------------------------------------------
SpaceSquad * SpaceSquadManager::getSquad(int const squadId)
{
	SpaceSquad * result = nullptr;
	SquadList::const_iterator iterSpaceSquadList = s_squadList.find(squadId);

	if (iterSpaceSquadList != s_squadList.end())
	{
		result = iterSpaceSquadList->second;
	}

	return result;
}

// ======================================================================
