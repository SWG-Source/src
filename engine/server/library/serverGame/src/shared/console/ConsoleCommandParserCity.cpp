// ======================================================================
//
// ConsoleCommandParserCity.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserCity.h"

#include "UnicodeUtils.h"
#include "serverGame/CitizenInfo.h"
#include "serverGame/CityInfo.h"
#include "serverGame/CityInterface.h"
#include "serverGame/CityStructureInfo.h"
#include "serverGame/GameServer.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/PvpData.h"

// ======================================================================

namespace ConsoleCommandParserCityNamespace
{
	std::string const getCityPermissionsText(int permissions);
	std::string const getStructureTypeText(int type);
	std::string const getFactionString(uint32 factionId);
}
using namespace ConsoleCommandParserCityNamespace;

static const CommandParser::CmdInfo cmds[] =
{
	{"listById",                          0, "", "List all cities by city id."},
	{"listByName",                        0, "", "List all cities by city name."},
	{"listByRank",                        0, "", "List all cities by city rank."},
	{"listByPlanet",                      0, "", "List all cities by planet."},
	{"listByCitizenCount",                0, "", "List all cities by citizen count."},
	{"listByStructureCount",              0, "", "List all cities by structure count."},
	{"listByCreationTime",                0, "", "List all cities by city creation time."},
	{"listCitiesWithIllegalShuttleport",  0, "", "List all cities that are not rank 4 or 5 that have a shuttleport."},
	{"showCityGcwRegionDefender",         0, "", "Display GCW region defender data for cities."},
	{"showCityDetails",                   1, "<city id>", "Display city details for a city."},
	{"setCityCreationTime",               7, "<city id> <yyyy> <mm> <dd> <h> <m> <s>", "Sets a city's creation time."},
#ifdef _DEBUG
	{"showCityRetroactiveCreationTime",   2, "<city id> <cluster>", "Show the retroactive city creation time for the specified city on the specified cluster."},
#else
	{"showCityRetroactiveCreationTime",   1, "<city id>", "Show the retroactive city creation time for the specified city."},
#endif
	{"",                                  0, "", ""} // this must be last
};

//-----------------------------------------------------------------

ConsoleCommandParserCity::ConsoleCommandParserCity (void) :
CommandParser ("city", 0, "...", "City related commands.", 0)
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------

bool ConsoleCommandParserCity::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	NOT_NULL (node);
	UNREF (userId);

	UNREF(originalCommand);

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

	//-----------------------------------------------------------------

	if (isCommand(argv [0], "listById"))
	{
		std::map<int, CityInfo> const & allCities = CityInterface::getAllCityInfo();

		std::multimap<int, std::string> cityInfo;
		for (std::map<int, CityInfo>::const_iterator iter = allCities.begin(); iter != allCities.end(); ++iter)
		{
			CityInfo const & ci = iter->second;
			if (ci.getCityName().empty())
				continue;

			CitizenInfo const * const mayor = CityInterface::getCitizenInfo(iter->first, ci.getLeaderId());
			cityInfo.insert(std::make_pair(iter->first, std::string(FormattedString<2048>().sprintf("%d, %s, %s (%s), %s, %s (%d, %d), %dm, %lu (%s), %d, %d, %d (%s)\n", iter->first, ci.getCityName().c_str(), ci.getLeaderId().getValueString().c_str(), (mayor ? mayor->m_citizenName.c_str() : ""), ci.getCityHallId().getValueString().c_str(), ci.getPlanet().c_str(), ci.getX(), ci.getZ(), ci.getRadius(), ci.getFaction(), getFactionString(ci.getFaction()).c_str(), ci.getCitizenCount(), ci.getStructureCount(), ci.getCreationTime(), ((ci.getCreationTime() > 0) ? CalendarTime::convertEpochToTimeStringLocal(ci.getCreationTime()).c_str() : "N/A")))));
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = cityInfo.begin(); iter2 != cityInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		result += Unicode::narrowToWide(FormattedString<2048>().sprintf("%d cities listed\n", cityInfo.size()));

		if (!cityInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, mayor, cityHallId, location, radius, faction, number of citizens, number of structures, creationTime\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByName"))
	{
		std::map<int, CityInfo> const & allCities = CityInterface::getAllCityInfo();

		std::multimap<std::string, std::string> cityInfo;
		for (std::map<int, CityInfo>::const_iterator iter = allCities.begin(); iter != allCities.end(); ++iter)
		{
			CityInfo const & ci = iter->second;
			if (ci.getCityName().empty())
				continue;

			CitizenInfo const * const mayor = CityInterface::getCitizenInfo(iter->first, ci.getLeaderId());
			cityInfo.insert(std::make_pair(ci.getCityName(), std::string(FormattedString<2048>().sprintf("%d, %s, %s (%s), %s, %s (%d, %d), %dm, %lu (%s), %d, %d, %d (%s)\n", iter->first, ci.getCityName().c_str(), ci.getLeaderId().getValueString().c_str(), (mayor ? mayor->m_citizenName.c_str() : ""), ci.getCityHallId().getValueString().c_str(), ci.getPlanet().c_str(), ci.getX(), ci.getZ(), ci.getRadius(), ci.getFaction(), getFactionString(ci.getFaction()).c_str(), ci.getCitizenCount(), ci.getStructureCount(), ci.getCreationTime(), ((ci.getCreationTime() > 0) ? CalendarTime::convertEpochToTimeStringLocal(ci.getCreationTime()).c_str() : "N/A")))));
		}

		for (std::multimap<std::string, std::string>::const_iterator iter2 = cityInfo.begin(); iter2 != cityInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		result += Unicode::narrowToWide(FormattedString<2048>().sprintf("%d cities listed\n", cityInfo.size()));

		if (!cityInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, mayor, cityHallId, location, radius, faction, number of citizens, number of structures, creationTime\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByRank"))
	{
		std::map<int, CityInfo> const & allCities = CityInterface::getAllCityInfo();

		std::multimap<int, std::string> cityInfo;
		for (std::map<int, CityInfo>::const_iterator iter = allCities.begin(); iter != allCities.end(); ++iter)
		{
			CityInfo const & ci = iter->second;
			if (ci.getCityName().empty())
				continue;

			CitizenInfo const * const mayor = CityInterface::getCitizenInfo(iter->first, ci.getLeaderId());
			cityInfo.insert(std::make_pair(-ci.getRadius(), std::string(FormattedString<2048>().sprintf("%d, %s, %s (%s), %s, %s (%d, %d), %dm, %lu (%s), %d, %d, %d (%s)\n", iter->first, ci.getCityName().c_str(), ci.getLeaderId().getValueString().c_str(), (mayor ? mayor->m_citizenName.c_str() : ""), ci.getCityHallId().getValueString().c_str(), ci.getPlanet().c_str(), ci.getX(), ci.getZ(), ci.getRadius(), ci.getFaction(), getFactionString(ci.getFaction()).c_str(), ci.getCitizenCount(), ci.getStructureCount(), ci.getCreationTime(), ((ci.getCreationTime() > 0) ? CalendarTime::convertEpochToTimeStringLocal(ci.getCreationTime()).c_str() : "N/A")))));
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = cityInfo.begin(); iter2 != cityInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		result += Unicode::narrowToWide(FormattedString<2048>().sprintf("%d cities listed\n", cityInfo.size()));

		if (!cityInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, mayor, cityHallId, location, radius, faction, number of citizens, number of structures, creationTime\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByPlanet"))
	{
		std::map<int, CityInfo> const & allCities = CityInterface::getAllCityInfo();

		std::multimap<std::string, std::string> cityInfo;
		for (std::map<int, CityInfo>::const_iterator iter = allCities.begin(); iter != allCities.end(); ++iter)
		{
			CityInfo const & ci = iter->second;
			if (ci.getCityName().empty())
				continue;

			CitizenInfo const * const mayor = CityInterface::getCitizenInfo(iter->first, ci.getLeaderId());
			cityInfo.insert(std::make_pair(ci.getPlanet(), std::string(FormattedString<2048>().sprintf("%d, %s, %s (%s), %s, %s (%d, %d), %dm, %lu (%s), %d, %d, %d (%s)\n", iter->first, ci.getCityName().c_str(), ci.getLeaderId().getValueString().c_str(), (mayor ? mayor->m_citizenName.c_str() : ""), ci.getCityHallId().getValueString().c_str(), ci.getPlanet().c_str(), ci.getX(), ci.getZ(), ci.getRadius(), ci.getFaction(), getFactionString(ci.getFaction()).c_str(), ci.getCitizenCount(), ci.getStructureCount(), ci.getCreationTime(), ((ci.getCreationTime() > 0) ? CalendarTime::convertEpochToTimeStringLocal(ci.getCreationTime()).c_str() : "N/A")))));
		}

		for (std::multimap<std::string, std::string>::const_iterator iter2 = cityInfo.begin(); iter2 != cityInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		result += Unicode::narrowToWide(FormattedString<2048>().sprintf("%d cities listed\n", cityInfo.size()));

		if (!cityInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, mayor, cityHallId, location, radius, faction, number of citizens, number of structures, creationTime\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByCitizenCount"))
	{
		std::map<int, CityInfo> const & allCities = CityInterface::getAllCityInfo();

		std::multimap<int, std::string> cityInfo;
		for (std::map<int, CityInfo>::const_iterator iter = allCities.begin(); iter != allCities.end(); ++iter)
		{
			CityInfo const & ci = iter->second;
			if (ci.getCityName().empty())
				continue;

			CitizenInfo const * const mayor = CityInterface::getCitizenInfo(iter->first, ci.getLeaderId());
			cityInfo.insert(std::make_pair(-ci.getCitizenCount(), std::string(FormattedString<2048>().sprintf("%d, %s, %s (%s), %s, %s (%d, %d), %dm, %lu (%s), %d, %d, %d (%s)\n", iter->first, ci.getCityName().c_str(), ci.getLeaderId().getValueString().c_str(), (mayor ? mayor->m_citizenName.c_str() : ""), ci.getCityHallId().getValueString().c_str(), ci.getPlanet().c_str(), ci.getX(), ci.getZ(), ci.getRadius(), ci.getFaction(), getFactionString(ci.getFaction()).c_str(), ci.getCitizenCount(), ci.getStructureCount(), ci.getCreationTime(), ((ci.getCreationTime() > 0) ? CalendarTime::convertEpochToTimeStringLocal(ci.getCreationTime()).c_str() : "N/A")))));
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = cityInfo.begin(); iter2 != cityInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		result += Unicode::narrowToWide(FormattedString<2048>().sprintf("%d cities listed\n", cityInfo.size()));

		if (!cityInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, mayor, cityHallId, location, radius, faction, number of citizens, number of structures, creationTime\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByStructureCount"))
	{
		std::map<int, CityInfo> const & allCities = CityInterface::getAllCityInfo();

		std::multimap<int, std::string> cityInfo;
		for (std::map<int, CityInfo>::const_iterator iter = allCities.begin(); iter != allCities.end(); ++iter)
		{
			CityInfo const & ci = iter->second;
			if (ci.getCityName().empty())
				continue;

			CitizenInfo const * const mayor = CityInterface::getCitizenInfo(iter->first, ci.getLeaderId());
			cityInfo.insert(std::make_pair(-ci.getStructureCount(), std::string(FormattedString<2048>().sprintf("%d, %s, %s (%s), %s, %s (%d, %d), %dm, %lu (%s), %d, %d, %d (%s)\n", iter->first, ci.getCityName().c_str(), ci.getLeaderId().getValueString().c_str(), (mayor ? mayor->m_citizenName.c_str() : ""), ci.getCityHallId().getValueString().c_str(), ci.getPlanet().c_str(), ci.getX(), ci.getZ(), ci.getRadius(), ci.getFaction(), getFactionString(ci.getFaction()).c_str(), ci.getCitizenCount(), ci.getStructureCount(), ci.getCreationTime(), ((ci.getCreationTime() > 0) ? CalendarTime::convertEpochToTimeStringLocal(ci.getCreationTime()).c_str() : "N/A")))));
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = cityInfo.begin(); iter2 != cityInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		result += Unicode::narrowToWide(FormattedString<2048>().sprintf("%d cities listed\n", cityInfo.size()));

		if (!cityInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, mayor, cityHallId, location, radius, faction, number of citizens, number of structures, creationTime\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listByCreationTime"))
	{
		std::map<int, CityInfo> const & allCities = CityInterface::getAllCityInfo();

		std::multimap<int, std::string> cityInfo;
		for (std::map<int, CityInfo>::const_iterator iter = allCities.begin(); iter != allCities.end(); ++iter)
		{
			CityInfo const & ci = iter->second;
			if (ci.getCityName().empty())
				continue;

			CitizenInfo const * const mayor = CityInterface::getCitizenInfo(iter->first, ci.getLeaderId());
			cityInfo.insert(std::make_pair(ci.getCreationTime(), std::string(FormattedString<2048>().sprintf("%d, %s, %s (%s), %s, %s (%d, %d), %dm, %lu (%s), %d, %d, %d (%s)\n", iter->first, ci.getCityName().c_str(), ci.getLeaderId().getValueString().c_str(), (mayor ? mayor->m_citizenName.c_str() : ""), ci.getCityHallId().getValueString().c_str(), ci.getPlanet().c_str(), ci.getX(), ci.getZ(), ci.getRadius(), ci.getFaction(), getFactionString(ci.getFaction()).c_str(), ci.getCitizenCount(), ci.getStructureCount(), ci.getCreationTime(), ((ci.getCreationTime() > 0) ? CalendarTime::convertEpochToTimeStringLocal(ci.getCreationTime()).c_str() : "N/A")))));
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = cityInfo.begin(); iter2 != cityInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		result += Unicode::narrowToWide(FormattedString<2048>().sprintf("%d cities listed\n", cityInfo.size()));

		if (!cityInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, mayor, cityHallId, location, radius, faction, number of citizens, number of structures, creationTime\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "listCitiesWithIllegalShuttleport"))
	{
		std::map<int, CityInfo> const & allCities = CityInterface::getAllCityInfo();

		std::multimap<int, std::string> cityInfo;
		for (std::map<int, CityInfo>::const_iterator iter = allCities.begin(); iter != allCities.end(); ++iter)
		{
			CityInfo const & ci = iter->second;
			if (ci.getCityName().empty())
				continue;

			if (ci.getRadius() >= 400)
				continue;

			if (ci.getTravelCost() > 0)
			{
				CitizenInfo const * const mayor = CityInterface::getCitizenInfo(iter->first, ci.getLeaderId());
				cityInfo.insert(std::make_pair(iter->first, std::string(FormattedString<2048>().sprintf("%d, %s, %s (%s), %s, %s (%d, %d), %dm, %lu (%s), %d, %d, %d (%s)\n", iter->first, ci.getCityName().c_str(), ci.getLeaderId().getValueString().c_str(), (mayor ? mayor->m_citizenName.c_str() : ""), ci.getCityHallId().getValueString().c_str(), ci.getPlanet().c_str(), ci.getX(), ci.getZ(), ci.getRadius(), ci.getFaction(), getFactionString(ci.getFaction()).c_str(), ci.getCitizenCount(), ci.getStructureCount(), ci.getCreationTime(), ((ci.getCreationTime() > 0) ? CalendarTime::convertEpochToTimeStringLocal(ci.getCreationTime()).c_str() : "N/A")))));
			}
		}

		for (std::multimap<int, std::string>::const_iterator iter2 = cityInfo.begin(); iter2 != cityInfo.end(); ++iter2)
			result += Unicode::narrowToWide(iter2->second);

		result += Unicode::narrowToWide(FormattedString<2048>().sprintf("%d cities with illegal shuttleport listed\n", cityInfo.size()));

		if (!cityInfo.empty())
			result += Unicode::narrowToWide("Output format is: \"id, name, mayor, cityHallId, location, radius, faction, number of citizens, number of structures, creationTime\"\n");

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "showCityGcwRegionDefender"))
	{
		result += Unicode::narrowToWide(FormattedString<2048>().sprintf("Version (%d)\n\n", CityInterface::getGcwRegionDefenderCitiesVersion()));

		std::map<std::pair<std::string, int>, uint32> const & gcwRegionDefenderCities = CityInterface::getGcwRegionDefenderCities();
		for (std::map<std::pair<std::string, int>, uint32>::const_iterator iterCity = gcwRegionDefenderCities.begin(); iterCity != gcwRegionDefenderCities.end(); ++iterCity)
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("(%s) (%d, %s) (%lu) (%s)\n", iterCity->first.first.c_str(), iterCity->first.second, CityInterface::getCityInfo(iterCity->first.second).getCityName().c_str(), iterCity->second, getFactionString(iterCity->second).c_str()));

		result += Unicode::narrowToWide("\n");

		std::map<std::string, std::pair<int, int> > const & gcwRegionDefenderCitiesCount = CityInterface::getGcwRegionDefenderCitiesCount();
		for (std::map<std::string, std::pair<int, int> >::const_iterator iterCount = gcwRegionDefenderCitiesCount.begin(); iterCount != gcwRegionDefenderCitiesCount.end(); ++iterCount)
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("(%s) (Imperial=%d) (Rebel=%d)\n", iterCount->first.c_str(), iterCount->second.first, iterCount->second.second));

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv [0], "showCityDetails"))
	{
		int const cityId = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		CityInfo const & ci = CityInterface::getCityInfo(cityId);

		if (ci.getCityName().empty())
		{
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("no city with city id %d\n", cityId));
			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
		{
			// id
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("id: %d\n", cityId));

			// name
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("name: %s\n", ci.getCityName().c_str()));

			// mayor
			CitizenInfo const * const mayor = CityInterface::getCitizenInfo(cityId, ci.getLeaderId());
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("mayor: %s (%s)\n", ci.getLeaderId().getValueString().c_str(), (mayor ? mayor->m_citizenName.c_str() : "")));

			// cityHallId
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("city hall id: %s\n", ci.getCityHallId().getValueString().c_str()));

			// location
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("location: %s (%d, %d)\n", ci.getPlanet().c_str(), ci.getX(), ci.getZ()));

			// radius
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("radius: %dm\n", ci.getRadius()));

			// faction
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("faction: %lu (%s)\n", ci.getFaction(), getFactionString(ci.getFaction()).c_str()));

			// GCW region defender
			if (ci.getGcwDefenderRegion().empty())
				result += Unicode::narrowToWide("GCW defender region: (NONE)\n");
			else
				result += Unicode::narrowToWide(FormattedString<2048>().sprintf("GCW defender region: %s [joined at %d (%s)]\n", ci.getGcwDefenderRegion().c_str(), ci.getTimeJoinedGcwDefenderRegion(), CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(ci.getTimeJoinedGcwDefenderRegion())).c_str()));

			// creationTime
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("creation time: %d (%s)\n", ci.getCreationTime(), ((ci.getCreationTime() > 0) ? CalendarTime::convertEpochToTimeStringLocal(ci.getCreationTime()).c_str() : "N/A")));

			// taxes
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("taxes: income %d, property %d, sales %d\n", ci.getIncomeTax(), ci.getPropertyTax(), ci.getSalesTax()));

			// travel
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("travel: location (%.2f, %.2f, %.2f), cost %d, interplanetary %s\n", ci.getTravelLoc().x, ci.getTravelLoc().y, ci.getTravelLoc().z, ci.getTravelCost(), (ci.getTravelInterplanetary() ? "yes" : "no")));

			// clone
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("clone: cloner id %s, cloner location (%.2f, %.2f, %.2f), clone respawn location (%.2f, %.2f, %.2f), clone respawn cell %s\n", ci.getCloneId().getValueString().c_str(), ci.getCloneLoc().x, ci.getCloneLoc().y, ci.getCloneLoc().z, ci.getCloneRespawn().x, ci.getCloneRespawn().y, ci.getCloneRespawn().z, ci.getCloneRespawnCell().getValueString().c_str()));

			// citizens
			result += Unicode::narrowToWide("\nCitizens:\n");

			std::map<std::pair<int, NetworkId>, CitizenInfo> const & allCitizens = CityInterface::getAllCitizensInfo();
			std::string citizenRankInfo;
			int citizenCount = 0;
			for (std::map<std::pair<int, NetworkId>, CitizenInfo>::const_iterator iterCitizen = allCitizens.lower_bound(std::make_pair(cityId, NetworkId::cms_invalid)); iterCitizen != allCitizens.end(); ++iterCitizen)
			{
				if (iterCitizen->first.first != cityId)
					break;

				++citizenCount;

				// citizen rank
				iterCitizen->second.m_citizenRank.getAsDbTextString(citizenRankInfo);
				citizenRankInfo += " (";

				std::vector<std::string> ranks;
				CityInterface::getCitizenRank(cityId, iterCitizen->first.second, ranks);
				for (std::vector<std::string>::const_iterator iterRank = ranks.begin(); iterRank != ranks.end(); ++iterRank)
				{
					if (iterRank != ranks.begin())
						citizenRankInfo += ", ";

					citizenRankInfo += *iterRank;
				}

				citizenRankInfo += ")";

				result += Unicode::narrowToWide(FormattedString<2048>().sprintf("%s, %s, %s, %d, %d (%s), %s, %s, %s\n", iterCitizen->first.second.getValueString().c_str(), iterCitizen->second.m_citizenName.c_str(), iterCitizen->second.m_citizenProfessionSkillTemplate.c_str(), iterCitizen->second.m_citizenLevel, iterCitizen->second.m_citizenPermissions, getCityPermissionsText(iterCitizen->second.m_citizenPermissions).c_str(), citizenRankInfo.c_str(), iterCitizen->second.m_citizenTitle.c_str(), iterCitizen->second.m_citizenAllegiance.getValueString().c_str()));
			}

			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("%d citizens listed\n", citizenCount));

			if (citizenCount > 0)
				result += Unicode::narrowToWide("Output format is: \"id, name, profession, level, permissions, rank, title, allegiance\"\n");

			// structures
			result += Unicode::narrowToWide("\nStructures:\n");

			std::map<std::pair<int, NetworkId>, CityStructureInfo> const & allStructures = CityInterface::getAllCityStructuresInfo();
			int structureCount = 0;
			for (std::map<std::pair<int, NetworkId>, CityStructureInfo>::const_iterator iterStructure = allStructures.lower_bound(std::make_pair(cityId, NetworkId::cms_invalid)); iterStructure != allStructures.end(); ++iterStructure)
			{
				if (iterStructure->first.first != cityId)
					break;

				++structureCount;

				result += Unicode::narrowToWide(FormattedString<2048>().sprintf("%s, %s, %d (%s)\n", iterStructure->first.second.getValueString().c_str(), (iterStructure->second.getStructureValid() ? "valid" : "no valid"), iterStructure->second.getStructureType(), getStructureTypeText(iterStructure->second.getStructureType()).c_str()));
			}

			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("%d structures listed\n", structureCount));

			if (structureCount > 0)
				result += Unicode::narrowToWide("Output format is: \"id, valid, type\"\n");

			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
	}
	else if (isCommand(argv [0], "setCityCreationTime"))
	{
		int const cityId = atoi(Unicode::wideToNarrow(argv[1]).c_str());
		CityInfo const & ci = CityInterface::getCityInfo(cityId);

		if (ci.getCityName().empty())
		{
			result += Unicode::narrowToWide(FormattedString<2048>().sprintf("no city with city id %d\n", cityId));
			result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
		{
			time_t const rawtime = ::time(nullptr);
			struct tm * timeinfo = ::localtime(&rawtime);
			timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1900;
			timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[3]).c_str()) - 1;
			timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[4]).c_str());
			timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[5]).c_str());
			timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[6]).c_str());
			timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[7]).c_str());
			time_t const specifiedTime = ::mktime(timeinfo);

			if (specifiedTime <= 0)
			{
				result += Unicode::narrowToWide("specified time is invalid\n");
				result += getErrorMessage(argv[0], ERR_FAIL);
			}
			else
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("setting city creation time for city %d (%s):\n", cityId, ci.getCityName().c_str()));

				if (ci.getCreationTime() <= 0)
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("***FROM*** %d (N/A)\n", ci.getCreationTime()));
				else
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("***FROM*** %d (%s, %s)\n", ci.getCreationTime(), CalendarTime::convertEpochToTimeStringLocal(ci.getCreationTime()).c_str(), CalendarTime::convertEpochToTimeStringGMT(ci.getCreationTime()).c_str()));

				result += Unicode::narrowToWide(FormattedString<512>().sprintf("****TO**** %ld (%s, %s)\n", static_cast<long>(specifiedTime), CalendarTime::convertEpochToTimeStringLocal(specifiedTime).c_str(), CalendarTime::convertEpochToTimeStringGMT(specifiedTime).c_str()));

				CityInterface::setCityCreationTime(cityId, static_cast<int>(specifiedTime));

				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
		}
	}
	else if (isCommand(argv [0], "showCityRetroactiveCreationTime"))
	{
		int const cityId = atoi(Unicode::wideToNarrow(argv[1]).c_str());

#ifdef _DEBUG
		std::string const clusterName = Unicode::wideToNarrow(argv[2]);
#else
		std::string const clusterName = GameServer::getInstance().getClusterName();
#endif

		time_t const retroactiveCreationTime = GameServer::getRetroactivePlayerCityCreationTime(clusterName, cityId);

		if (retroactiveCreationTime <= 0)
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("no retroactive creation time for player city (%s, %d)\n", clusterName.c_str(), cityId));
		else
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("retroactive creation time for player city (%s, %d) is %ld, %s\n", clusterName.c_str(), cityId, retroactiveCreationTime, CalendarTime::convertEpochToTimeStringLocal(retroactiveCreationTime).c_str()));
	}

	return true;
}	// ConsoleCommandParserCity::performParsing

// ----------------------------------------------------------------------

std::string const ConsoleCommandParserCityNamespace::getCityPermissionsText(int permissions)
{
	std::string result;
	if (permissions == static_cast<int>(CitizenPermissions::Citizen))
	{
		result = "Citizen";
	}
	else
	{
		if (permissions & static_cast<int>(CitizenPermissions::Militia))
			result += "Militia ";

		if (permissions & static_cast<int>(CitizenPermissions::AbsentWeek1))
			result += "AbsentWeek1 ";

		if (permissions & static_cast<int>(CitizenPermissions::AbsentWeek2))
			result += "AbsentWeek2 ";

		if (permissions & static_cast<int>(CitizenPermissions::AbsentWeek3))
			result += "AbsentWeek3 ";

		if (permissions & static_cast<int>(CitizenPermissions::AbsentWeek4))
			result += "AbsentWeek4 ";

		if (permissions & static_cast<int>(CitizenPermissions::AbsentWeek5))
			result += "AbsentWeek5 ";

		if (permissions & static_cast<int>(CitizenPermissions::InactiveProtected))
			result += "InactiveProtected ";
	}

	return result;
}

// ----------------------------------------------------------------------

std::string const ConsoleCommandParserCityNamespace::getStructureTypeText(int type)
{
	// from city.scriptlib

	// Structure Flags
	static const int			SF_PM_REGISTER					= 1;
	static const int			SF_COST_CITY_HALL				= 2;	// 5000 (datatable entry 1)
	static const int			SF_COST_CITY_HI					= 4;	// 1000
	static const int			SF_COST_CITY_MED				= 8;	// 500
	static const int			SF_COST_CITY_LOW				= 16;	// 250  (datatable entry 4)
	static const int			SF_MISSION_TERMINAL				= 32;
	static const int			SF_SKILL_TRAINER				= 64;
	static const int			SF_DECORATION					= 128;

	// BEGIN: City Hall ONLY flags.
	// Must be synchronized with datatables/city/specializations.iff
	static const int			SF_SPEC_SAMPLE_RICH				= 256;		/*Sample Rich*/
	static const int			SF_SPEC_FARMING					= 512; 		/*removed*/
	static const int			SF_SPEC_INDUSTRY				= 1024;		/*Manufacturing Center*/
	static const int			SF_SPEC_RESEARCH				= 2048;		/*Research Center*/
	static const int			SF_SPEC_CLONING					= 4096;		/*Clone Lab*/
	static const int			SF_SPEC_MISSIONS				= 8192;		/*Improved Job Market*/	
	static const int			SF_SPEC_ENTERTAINER				= 16384;	/*Entertainment Districtu*/
	static const int			SF_SPEC_DOCTOR					= 32768; 	/*removed*/
	static const int			SF_SPEC_STRONGHOLD				= 65536; 	/*removed*/
	static const int			SF_SPEC_MASTER_MANUFACTURING	= 131072; 	/*removed*/
	static const int			SF_SPEC_MASTER_HEALING			= 262144; 	/*removed*/
	static const int			SF_SPEC_DECOR_INCREASE			= 524288; 	/*Outdoor Enhancements*/
	static const int			SF_SPEC_STORYTELLER				= 1048576;	/*Encore Performance*/
	static const int			SF_SPEC_INCUBATOR				= 2097152;	/*incubation-BMs*/
	static const int			SF_SPEC_UNKNOWN_6				= 4194304;
	static const int			SF_REQUIRE_ZONE_RIGHTS				= 8388608;
	// END: City Hall ONLY flags.

	static const int			SF_COST_CITY_GARDEN_SMALL		= 16777216;
	static const int			SF_COST_CITY_GARDEN_LARGE		= 33554432;

	std::string result;

	if (type & SF_PM_REGISTER)
		result += "SF_PM_REGISTER ";

	if (type & SF_COST_CITY_HALL)
		result += "SF_COST_CITY_HALL ";

	if (type & SF_COST_CITY_HI)
		result += "SF_COST_CITY_HI ";

	if (type & SF_COST_CITY_MED)
		result += "SF_COST_CITY_MED ";

	if (type & SF_COST_CITY_LOW)
		result += "SF_COST_CITY_LOW ";

	if (type & SF_MISSION_TERMINAL)
		result += "SF_MISSION_TERMINAL ";

	if (type & SF_SKILL_TRAINER)
		result += "SF_SKILL_TRAINER ";

	if (type & SF_DECORATION)
		result += "SF_DECORATION ";

	if (type & SF_SPEC_SAMPLE_RICH)
		result += "SF_SPEC_SAMPLE_RICH ";

	if (type & SF_SPEC_FARMING)
		result += "SF_SPEC_FARMING ";

	if (type & SF_SPEC_INDUSTRY)
		result += "SF_SPEC_INDUSTRY ";

	if (type & SF_SPEC_RESEARCH)
		result += "SF_SPEC_RESEARCH ";

	if (type & SF_SPEC_CLONING)
		result += "SF_SPEC_CLONING ";

	if (type & SF_SPEC_MISSIONS)
		result += "SF_SPEC_MISSIONS ";

	if (type & SF_SPEC_ENTERTAINER)
		result += "SF_SPEC_ENTERTAINER ";

	if (type & SF_SPEC_DOCTOR)
		result += "SF_SPEC_DOCTOR ";

	if (type & SF_SPEC_STRONGHOLD)
		result += "SF_SPEC_STRONGHOLD ";

	if (type & SF_SPEC_MASTER_MANUFACTURING)
		result += "SF_SPEC_MASTER_MANUFACTURING ";

	if (type & SF_SPEC_MASTER_HEALING)
		result += "SF_SPEC_MASTER_HEALING ";

	if (type & SF_SPEC_DECOR_INCREASE)
		result += "SF_SPEC_DECOR_INCREASE ";

	if (type & SF_SPEC_STORYTELLER)
		result += "SF_SPEC_STORYTELLER ";

	if (type & SF_SPEC_INCUBATOR)
		result += "SF_SPEC_INCUBATOR ";

	if (type & SF_SPEC_UNKNOWN_6)
		result += "SF_SPEC_UNKNOWN_6 ";

	if (type & SF_REQUIRE_ZONE_RIGHTS)
		result += "SF_REQUIRE_ZONE_RIGHTS ";

	if (type & SF_COST_CITY_GARDEN_SMALL)
		result += "SF_COST_CITY_GARDEN_SMALL ";

	if (type & SF_COST_CITY_GARDEN_LARGE)
		result += "SF_COST_CITY_GARDEN_LARGE ";

	return result;
}

// ----------------------------------------------------------------------

std::string const ConsoleCommandParserCityNamespace::getFactionString(uint32 factionId)
{
	if (PvpData::isNeutralFactionId(factionId))
		return "Neutral";
	else if (PvpData::isImperialFactionId(factionId))
		return "Imperial";
	else if (PvpData::isRebelFactionId(factionId))
		return "Rebel";

	return "Unknown";
}

// ======================================================================
