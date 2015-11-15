// ======================================================================
//
// ShipAiReactionManager.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipAiReactionManager.h"

#include "serverGame/PvpUpdateObserver.h"
#include "serverGame/ShipObject.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedUtility/DataTable.h"

// ======================================================================

namespace ShipAiReactionManagerNamespace
{

	// ----------------------------------------------------------------------

	void remove();
	void loadSpaceMobileData();

	// ----------------------------------------------------------------------

	struct MobileData
	{
		MobileData() :
			m_aggro(false),
			m_faction(0),
			m_alliedFactionList(),
			m_enemyFactionList(),
			m_shipClass(ShipAiReactionManager::SC_invalid)
		{
		}

		bool m_aggro;
		uint32 m_faction;
		std::vector<uint32> m_alliedFactionList;
		std::vector<uint32> m_enemyFactionList;
		ShipAiReactionManager::ShipClass m_shipClass;

	private:
		MobileData &operator=(MobileData const &);
	};
				
	// ----------------------------------------------------------------------

	typedef std::map<PersistentCrcString, MobileData> MobileDataList;
	MobileDataList s_mobileDataList;

	// ----------------------------------------------------------------------

};

using namespace ShipAiReactionManagerNamespace;

// ======================================================================

void ShipAiReactionManager::install()
{
	s_mobileDataList.clear();
	loadSpaceMobileData();

	ExitChain::add(ShipAiReactionManagerNamespace::remove, "ShipAiReactionManagerNamespace::remove");
}

// ----------------------------------------------------------------------

void ShipAiReactionManagerNamespace::remove()
{
	s_mobileDataList.clear();
}

// ----------------------------------------------------------------------

void ShipAiReactionManagerNamespace::loadSpaceMobileData()
{
	// Create the data table

	Iff iff;

	if (iff.open("datatables/space_mobile/space_mobile.iff", true))
	{
		DataTable dataTable;
		dataTable.load(iff);

		int const rowCount = dataTable.getNumRows();

		for (int row = 0; row < rowCount; ++row)
		{
			MobileData mobileData;

			mobileData.m_aggro = (dataTable.getIntValue("isAggro", row) != 0);
			mobileData.m_faction = Crc::calculate(dataTable.getStringValue("space_faction", row));
			
			std::string const alliedFactions(dataTable.getStringValue("alliedFactions", row));

			if (alliedFactions.size() > 0)
			{
				int startPosition = 0;
				int endPosition = 0;
				do
				{
					endPosition = static_cast<int>(alliedFactions.find_first_of(",", static_cast<unsigned int>(startPosition)));
					std::string const alliedFaction(alliedFactions.substr(static_cast<unsigned int>(startPosition), static_cast<unsigned int>(endPosition)));
					uint32 const crc = Crc::calculate(alliedFaction.c_str());
					if (std::find(mobileData.m_alliedFactionList.begin(), mobileData.m_alliedFactionList.end(), crc) == mobileData.m_alliedFactionList.end())
						mobileData.m_alliedFactionList.push_back(crc);
					startPosition = endPosition + 1;

				} while (endPosition != static_cast<int>(std::string::npos));
			}

			std::string const enemyFactions(dataTable.getStringValue("enemyFactions", row));

			if (enemyFactions.size() > 0)
			{
				int startPosition = 0;
				int endPosition = 0;
				do
				{
					endPosition = static_cast<int>(enemyFactions.find_first_of(",", static_cast<unsigned int>(startPosition)));
					std::string const enemyFaction(enemyFactions.substr(static_cast<unsigned int>(startPosition), static_cast<unsigned int>(endPosition)));
					uint32 const crc = Crc::calculate(enemyFaction.c_str());
					if (std::find(mobileData.m_enemyFactionList.begin(), mobileData.m_enemyFactionList.end(), crc) == mobileData.m_enemyFactionList.end())
						mobileData.m_enemyFactionList.push_back(crc);
					startPosition = endPosition + 1;

				} while (endPosition != static_cast<int>(std::string::npos));
			}

			mobileData.m_shipClass = static_cast<ShipAiReactionManager::ShipClass>(dataTable.getIntValue("shipClass", row));
	
			IGNORE_RETURN(s_mobileDataList.insert(std::make_pair(PersistentCrcString(dataTable.getStringValue("strIndex", row), false), mobileData)));
		}
	}
	else
	{
		WARNING(true, ("Unable to load the AI space_mobile.iff, this means the AI is not going to work very well!"));
	}
}

// ----------------------------------------------------------------------

void ShipAiReactionManager::setShipSpaceMobileType(ShipObject &ship, PersistentCrcString const &spaceMobileType)
{
	PvpUpdateObserver(&ship, Archive::ADOO_generic);

	MobileDataList::const_iterator const i = s_mobileDataList.find(spaceMobileType);
	FATAL(i == s_mobileDataList.end(), ("ShipAiReactionManager::setShipSpaceMobileType: invalid space_mobile type '%s' for ship %s", spaceMobileType.getString(), ship.getNetworkId().getValueString().c_str()));

	ship.setSpaceFaction((*i).second.m_faction);
	ship.setSpaceFactionAllies((*i).second.m_alliedFactionList);
	ship.setSpaceFactionEnemies((*i).second.m_enemyFactionList);
	ship.setSpaceFactionIsAggro((*i).second.m_aggro);
}

// ----------------------------------------------------------------------

bool ShipAiReactionManager::isAlly(ShipObject const &actor, ShipObject const &target)
{
	// actors are never allies of unfactioned targets
	uint32 const targetFaction = target.getSpaceFaction();
	if (!targetFaction)
		return false;

	// if the target's faction is on the actor's ally list, they are allies
	std::vector<uint32> const &allyList = actor.getSpaceFactionAllies();
	return std::find(allyList.begin(), allyList.end(), targetFaction) != allyList.end();
}

// ----------------------------------------------------------------------

bool ShipAiReactionManager::isEnemy(ShipObject const &actor, ShipObject const &target)
{
	// allies are not enemies
	if (isAlly(actor, target) || isAlly(target, actor))
		return false;

	// if the target's faction is nonzero and on the actor's enemy list, they are enemies
	uint32 const targetFaction = target.getSpaceFaction();
	if (targetFaction)
	{
		std::vector<uint32> const &enemyList = actor.getSpaceFactionEnemies();
		if (std::find(enemyList.begin(), enemyList.end(), targetFaction) != enemyList.end())
			return true;
	}

	// aggro non-allies are enemies
	if (   (actor.isPlayerShip() && target.getSpaceFactionIsAggro())
	    || (target.isPlayerShip() && actor.getSpaceFactionIsAggro()))
		return true;

	return false;
}

// ----------------------------------------------------------------------

ShipAiReactionManager::ShipClass ShipAiReactionManager::getShipClass(PersistentCrcString const & spaceMobileType)
{
	MobileDataList::const_iterator const iterMobileDataList = s_mobileDataList.find(spaceMobileType);
	FATAL((iterMobileDataList == s_mobileDataList.end()), ("ShipAiReactionManager::getShipClass() invalid space_mobile type(%s)", spaceMobileType.getString()));

	return iterMobileDataList->second.m_shipClass;
}

// ----------------------------------------------------------------------

char const * ShipAiReactionManager::getShipClassString(ShipClass const shipClass)
{
	switch (shipClass)
	{
		default: { return "invalid"; } break;
		case SC_fighter: { return "fighter"; } break;
		case SC_bomber: { return "bomber"; } break;
		case SC_capitalShip: { return "capitol ship"; } break;
		case SC_transport: { return "transport"; } break;
	}

	return "";
}

// ======================================================================
