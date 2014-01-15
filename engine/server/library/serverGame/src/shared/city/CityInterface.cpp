// ======================================================================
//
// CityInterface.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CityInterface.h"

#include "UnicodeUtils.h"
#include "serverGame/Chat.h"
#include "serverGame/CityObject.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerUniverse.h"
#include "sharedNetworkMessages/ChatRoomData.h"

// ======================================================================

namespace CityInterfaceNamespace
{
	// ----------------------------------------------------------------------

	std::string const &getChatRoomPrefix()
	{ //lint !e1929 // returning a reference makes sense here.
		static std::string const prefix = std::string("SWG.") + GameServer::getInstance().getClusterName().c_str() + ".city.";
		return prefix;
	}

	// ----------------------------------------------------------------------

	std::string const &getChatRoomSuffix()
	{ //lint !e1929 // returning a reference makes sense here.
		static std::string const suffix("." + ChatRoomTypes::ROOM_CITY);
		return suffix;
	}

	// ----------------------------------------------------------------------

	std::string getChatRoomPath(int cityId)
	{
		char buf[256];
		snprintf(buf, 256, "%s%d%s", getChatRoomPrefix().c_str(), cityId, getChatRoomSuffix().c_str());
		return std::string(buf);
	}

	// ----------------------------------------------------------------------

	std::string getChatRoomTitle(int cityId)
	{
		char buf[32];
		snprintf(buf, 32, "%d", cityId);
		return std::string(buf);
	}
}
using namespace CityInterfaceNamespace;

// ======================================================================

std::vector<int> const & CityInterface::getCitizenOfCityId(NetworkId const &citizenId)
{
	return ServerUniverse::getInstance().getMasterCityObject()->getCitizenOfCityId(citizenId);
}

// ----------------------------------------------------------------------

int CityInterface::getMilitiaOfCityId(NetworkId const &citizenId)
{
	std::vector<int> const & cityId = getCitizenOfCityId(citizenId);
	if (!cityId.empty())
	{
		int const firstCityId = cityId.front();
		if (isCityMilitia(firstCityId, citizenId))
			return firstCityId;
	}

	return 0;
}

// ----------------------------------------------------------------------

void CityInterface::getAllCityIds(std::vector<int> &results)
{
	ServerUniverse::getInstance().getMasterCityObject()->getAllCityIds(results);
}

// ----------------------------------------------------------------------

int CityInterface::findCityByName(std::string const &name)
{
	return ServerUniverse::getInstance().getMasterCityObject()->findCityByName(name);
}

// ----------------------------------------------------------------------

int CityInterface::findCityByCityHall(NetworkId const &cityHallId)
{
	return ServerUniverse::getInstance().getMasterCityObject()->findCityByCityHall(cityHallId);
}

// ----------------------------------------------------------------------

int CityInterface::getCityAtLocation(std::string const &planetName, int x, int z, int radius)
{
	return ServerUniverse::getInstance().getMasterCityObject()->getCityAtLocation(planetName, x, z, radius);
}

// ----------------------------------------------------------------------

bool CityInterface::cityExists(int cityId)
{
	return ServerUniverse::getInstance().getMasterCityObject()->cityExists(cityId);
}

// ----------------------------------------------------------------------

void CityInterface::getCitizenIds(int cityId, std::vector<NetworkId> &results)
{
	ServerUniverse::getInstance().getMasterCityObject()->getCitizenIds(cityId, results);
}

// ----------------------------------------------------------------------

int CityInterface::getCitizenCount(int cityId)
{
	return getCityInfo(cityId).getCitizenCount();
}

// ----------------------------------------------------------------------

void CityInterface::getCityStructureIds(int cityId, std::vector<NetworkId> &results)
{
	ServerUniverse::getInstance().getMasterCityObject()->getCityStructureIds(cityId, results);
}

// ----------------------------------------------------------------------

CityInfo const &CityInterface::getCityInfo(int cityId)
{
	return ServerUniverse::getInstance().getMasterCityObject()->getCityInfo(cityId);
}

// ----------------------------------------------------------------------

std::map<int, CityInfo> const &CityInterface::getAllCityInfo()
{
	return ServerUniverse::getInstance().getMasterCityObject()->getAllCityInfo();
}

// ----------------------------------------------------------------------

CitizenInfo const *CityInterface::getCitizenInfo(int cityId, NetworkId const &citizenId)
{
	return ServerUniverse::getInstance().getMasterCityObject()->getCitizenInfo(cityId, citizenId);
}

// ----------------------------------------------------------------------

std::map<std::pair<int, NetworkId>, CitizenInfo> const &CityInterface::getAllCitizensInfo()
{
	return ServerUniverse::getInstance().getMasterCityObject()->getAllCitizensInfo();
}

// ----------------------------------------------------------------------

CityStructureInfo const *CityInterface::getCityStructureInfo(int cityId, NetworkId const &structureId)
{
	return ServerUniverse::getInstance().getMasterCityObject()->getCityStructureInfo(cityId, structureId);
}

// ----------------------------------------------------------------------

std::map<std::pair<int, NetworkId>, CityStructureInfo> const & CityInterface::getAllCityStructuresInfo()
{
	return ServerUniverse::getInstance().getMasterCityObject()->getAllCityStructuresInfo();
}

// ----------------------------------------------------------------------

std::map<std::pair<std::string, int>, uint32> const & CityInterface::getGcwRegionDefenderCities()
{
	return ServerUniverse::getInstance().getMasterCityObject()->getGcwRegionDefenderCities();
}

// ----------------------------------------------------------------------

std::map<std::string, std::pair<int, int> > const & CityInterface::getGcwRegionDefenderCitiesCount()
{
	return ServerUniverse::getInstance().getMasterCityObject()->getGcwRegionDefenderCitiesCount();
}

// ----------------------------------------------------------------------

int CityInterface::getGcwRegionDefenderCitiesVersion()
{
	return ServerUniverse::getInstance().getMasterCityObject()->getGcwRegionDefenderCitiesVersion();
}

// ----------------------------------------------------------------------

bool CityInterface::isValidCityName(std::string const &cityName)
{
	if (cityName.empty() || cityName.length() > 25 || cityName.find(':') != std::string::npos)
		return false;
	return true;
}

// ----------------------------------------------------------------------

bool CityInterface::isCityMilitia(int cityId, NetworkId const &citizenId)
{
	CitizenInfo const *ci = getCitizenInfo(cityId, citizenId);
	if (ci && (ci->m_citizenPermissions & CitizenPermissions::Militia))
		return true;
	return false;
}

// ----------------------------------------------------------------------

int CityInterface::createCity(std::string const &cityName, NetworkId const &cityHallId, std::string const &cityPlanet, int x, int z, int radius, NetworkId const &leaderId, int incomeTax, int propertyTax, int salesTax, Vector const &travelLoc, int travelCost, bool travelInterplanetary, Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId)
{
	if (   !isValidCityName(cityName)
	    || cityPlanet.empty()
	    || cityHallId == NetworkId::cms_invalid
	    || getCityAtLocation(cityPlanet, x, z, radius))
		return 0;
	return ServerUniverse::getInstance().getMasterCityObject()->createCity(cityName, cityHallId, cityPlanet, x, z, radius, leaderId, incomeTax, propertyTax, salesTax, travelLoc, travelCost, travelInterplanetary, cloneLoc, cloneRespawn, cloneRespawnCell, cloneId);
}

// ----------------------------------------------------------------------
void CityInterface::removeCity(int cityId)
{
	ServerUniverse::getInstance().getMasterCityObject()->removeCity(cityId);
}

// ----------------------------------------------------------------------

void CityInterface::removeCitizen(int cityId, NetworkId const &citizenId, bool sendUpdateNotification /*= true*/)
{
	UNREF(sendUpdateNotification);
	ServerUniverse::getInstance().getMasterCityObject()->removeCitizen(cityId, citizenId);
}

// ----------------------------------------------------------------------

void CityInterface::removeCityStructure(int cityId, NetworkId const &structureId)
{
	ServerUniverse::getInstance().getMasterCityObject()->removeCityStructure(cityId, structureId);
}

// ----------------------------------------------------------------------

void CityInterface::setCityName(int cityId, std::string const &cityName)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCityName(cityId, cityName);
}

// ----------------------------------------------------------------------

void CityInterface::setCityHall(int cityId, NetworkId const &cityHallId)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCityHall(cityId, cityHallId);
}

// ----------------------------------------------------------------------

void CityInterface::setCityLocation(int cityId, std::string const &cityPlanet, int x, int z)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCityLocation(cityId, cityPlanet, x, z);
}

// ----------------------------------------------------------------------

void CityInterface::setCityRadius(int cityId, int radius)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCityRadius(cityId, radius);
}

// ----------------------------------------------------------------------

void CityInterface::setCityFaction(int cityId, uint32 faction, bool notifyCitizens)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCityFaction(cityId, faction, notifyCitizens);
}

// ----------------------------------------------------------------------

void CityInterface::setCityGcwDefenderRegion(int cityId, std::string const &gcwDefenderRegion, int timeJoined, bool notifyCitizens)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCityGcwDefenderRegion(cityId, gcwDefenderRegion, timeJoined, notifyCitizens);
}

// ----------------------------------------------------------------------

void CityInterface::setCityCreationTime(int cityId, int creationTime)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCityCreationTime(cityId, creationTime);
}

// ----------------------------------------------------------------------

void CityInterface::setCityLeader(int cityId, NetworkId const &leaderId)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCityLeader(cityId, leaderId);
}

// ----------------------------------------------------------------------

void CityInterface::setCityIncomeTax(int cityId, int incomeTax)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCityIncomeTax(cityId, incomeTax);
}

// ----------------------------------------------------------------------

void CityInterface::setCityPropertyTax(int cityId, int propertyTax)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCityPropertyTax(cityId, propertyTax);
}

// ----------------------------------------------------------------------

void CityInterface::setCitySalesTax(int cityId, int salesTax)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCitySalesTax(cityId, salesTax);
}

// ----------------------------------------------------------------------

void CityInterface::setCityTravelInfo(int cityId, Vector const &travelLoc, int travelCost, bool travelInterplanetary)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCityTravelInfo(cityId, travelLoc, travelCost, travelInterplanetary);
}

// ----------------------------------------------------------------------

void CityInterface::setCityCloneInfo(int cityId, Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCityCloneInfo(cityId, cloneLoc, cloneRespawn, cloneRespawnCell, cloneId);
}

// ----------------------------------------------------------------------

void CityInterface::setCitizenInfo(int cityId, NetworkId const &citizenId, std::string const &citizenName, NetworkId const &allegiance, int permissions)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCitizen(cityId, citizenId, citizenName, allegiance, permissions);
}

// ----------------------------------------------------------------------

void CityInterface::setCitizenProfessionInfo(int cityId, NetworkId const &citizenId, std::string const &citizenProfessionSkillTemplate, int citizenLevel)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCitizenProfessionInfo(cityId, citizenId, citizenProfessionSkillTemplate, citizenLevel);
}

// ----------------------------------------------------------------------

void CityInterface::addCitizenRank(int cityId, NetworkId const &citizenId, std::string const &rankName)
{
	ServerUniverse::getInstance().getMasterCityObject()->addCitizenRank(cityId, citizenId, rankName);
}

// ----------------------------------------------------------------------

void CityInterface::removeCitizenRank(int cityId, NetworkId const &citizenId, std::string const &rankName)
{
	ServerUniverse::getInstance().getMasterCityObject()->removeCitizenRank(cityId, citizenId, rankName);
}

// ----------------------------------------------------------------------

bool CityInterface::hasCitizenRank(int cityId, NetworkId const &citizenId, std::string const &rankName)
{
	return ServerUniverse::getInstance().getMasterCityObject()->hasCitizenRank(cityId, citizenId, rankName);
}

// ----------------------------------------------------------------------

void CityInterface::getCitizenRank(int cityId, NetworkId const &citizenId, std::vector<std::string> &ranks)
{
	ServerUniverse::getInstance().getMasterCityObject()->getCitizenRank(cityId, citizenId, ranks);
}

// ----------------------------------------------------------------------

void CityInterface::setCitizenTitle(int cityId, NetworkId const &citizenId, std::string const &citizenTitle)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCitizenTitle(cityId, citizenId, citizenTitle);
}

// ----------------------------------------------------------------------

void CityInterface::setCityStructureInfo(int cityId, NetworkId const &structureId, int structureType, bool structureValid)
{
	ServerUniverse::getInstance().getMasterCityObject()->setCityStructure(cityId, structureId, structureType, structureValid);
}

// ----------------------------------------------------------------------

void CityInterface::enterCityChatRoom(int cityId, CreatureObject const &who)
{
	if (cityId > 0)
	{
		Chat::createRoom("System", false, getChatRoomPath(cityId), getChatRoomTitle(cityId));
		std::string firstName;
		size_t pos = 0;
		IGNORE_RETURN(Unicode::getFirstToken(Unicode::wideToNarrow(who.getObjectName()), 0, pos, firstName));
		Chat::invite(firstName, getChatRoomPath(cityId));
		Chat::enterRoom(firstName, getChatRoomPath(cityId), false, false);
	}
}

// ----------------------------------------------------------------------

void CityInterface::reenterCityChatRoom(CreatureObject const &who)
{
	std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(who.getNetworkId());
	if (!cityIds.empty())
	{
		int const cityId = cityIds.front();

		std::string firstName;
		size_t pos = 0;
		IGNORE_RETURN(Unicode::getFirstToken(Unicode::wideToNarrow(who.getObjectName()), 0, pos, firstName));

		std::string const cityChatRoomPath = getChatRoomPath(cityId);
		Chat::exitRoom(firstName, cityChatRoomPath);
		Chat::uninvite(firstName, cityChatRoomPath);
		Chat::invite(firstName, cityChatRoomPath);
		Chat::enterRoom(firstName, cityChatRoomPath, false, false);
	}
}

// ----------------------------------------------------------------------

void CityInterface::leaveCityChatRoom(int cityId, CreatureObject const &who)
{
	if (cityId > 0)
	{
		std::string firstName;
		size_t pos = 0;
		IGNORE_RETURN(Unicode::getFirstToken(Unicode::wideToNarrow(who.getObjectName()), 0, pos, firstName));
		Chat::exitRoom(firstName, getChatRoomPath(cityId));
		Chat::uninvite(firstName, getChatRoomPath(cityId));
	}
}

// ----------------------------------------------------------------------

void CityInterface::sendCityChat(CreatureObject const &who, Unicode::String const &text)
{
	std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(who.getNetworkId());
	if (!cityIds.empty())
	{
		int const cityId = cityIds.front();

		std::string firstName;
		size_t pos = 0;
		IGNORE_RETURN(Unicode::getFirstToken(Unicode::wideToNarrow(who.getObjectName()), 0, pos, firstName));
		Chat::sendToRoom(firstName, getChatRoomPath(cityId), text, Unicode::String());
	}
}

// ---------------------------------------------------------------------

void CityInterface::onChatRoomCreate(std::string const &path)
{
	std::string const &prefix = getChatRoomPrefix();
	if (!path.compare(0, prefix.length(), prefix))
	{
		int cityId = atoi(path.c_str()+prefix.length());
		if (cityId)
		{
			std::vector<ServerObject *> players;
			GameServer::getInstance().getObjectsWithClients(players);
			for (std::vector<ServerObject *>::const_iterator i = players.begin(); i != players.end(); ++i)
			{
				if ((*i)->isAuthoritative() && (*i)->isPlayerControlled())
				{
					CreatureObject const *playerCreature = (*i)->asCreatureObject();
					if (playerCreature)
					{
						std::vector<int> const & cityIds = CityInterface::getCitizenOfCityId(playerCreature->getNetworkId());
						if (!cityIds.empty())
						{
							if (cityIds.front() == cityId)
								enterCityChatRoom(cityId, *playerCreature);
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void CityInterface::createAllCityChatRooms()
{
	std::set<int> cityIds;
	std::vector<ServerObject *> players;
	GameServer::getInstance().getObjectsWithClients(players);
	for (std::vector<ServerObject *>::const_iterator i = players.begin(); i != players.end(); ++i)
	{
		if ((*i)->isAuthoritative() && (*i)->isPlayerControlled())
		{
			CreatureObject const *playerCreature = (*i)->asCreatureObject();
			if (playerCreature)
			{
				std::vector<int> const & playerCityIds = CityInterface::getCitizenOfCityId(playerCreature->getNetworkId());
				if (!playerCityIds.empty())
					IGNORE_RETURN(cityIds.insert(playerCityIds.front()));
			}
		}
	}
	for (std::set<int>::const_iterator j = cityIds.begin(); j != cityIds.end(); ++j)
	{
		Chat::createRoom("System", false, getChatRoomPath(*j), getChatRoomTitle(*j));
	}
}

// ----------------------------------------------------------------------
// returns the number of citizens and city name that the mail was
// sent to or -1 if insufficient permission to send mail to citizens
std::pair<int, std::string> CityInterface::mailToCitizens(CreatureObject const &who, Unicode::String const &subject, Unicode::String const &message, Unicode::String const &oob)
{
	int count = -1;
	std::string cityName;

	// because of the bug that allows a player to be a citizen of more than one cities, we
	// cannot just get the city that the player is a citizen of and check to see if he's the
	// mayor of that city; we have to check every city and find one that he's mayor of
	std::map<int, CityInfo> const & allCities = ServerUniverse::getInstance().getMasterCityObject()->getAllCityInfo();
	for (std::map<int, CityInfo>::const_iterator j = allCities.begin(); j != allCities.end(); ++j)
	{
		CityInfo const &cityInfo = j->second;
		if (cityInfo.getLeaderId() == who.getNetworkId())
		{
			cityName = cityInfo.getCityName();
			count = 0;
			std::map<std::pair<int, NetworkId>, CitizenInfo> const & allCitizens = ServerUniverse::getInstance().getMasterCityObject()->getAllCitizensInfo();
			for (std::map<std::pair<int, NetworkId>, CitizenInfo>::const_iterator i = allCitizens.lower_bound(std::make_pair(j->first, NetworkId::cms_invalid)); i != allCitizens.end(); ++i)
			{
				if (i->first.first != j->first)
					break;

				Chat::sendPersistentMessage(Unicode::wideToNarrow(who.getEncodedObjectName()), i->second.m_citizenName, subject, message, oob);
				++count;
			}

			break;
		}
	}

	return std::make_pair(count, cityName);
}

// ----------------------------------------------------------------------
// returns the number of citizens and city name that the mail was
// sent to or -1 if insufficient permission to send mail to citizens
std::pair<int, std::string> CityInterface::mailToCitizens(int cityId, Unicode::String const &subject, Unicode::String const &message, Unicode::String const &oob)
{
	std::string const cityName = CityInterface::getCityInfo(cityId).getCityName();
	if (cityName.empty())
		return std::make_pair(-1, std::string());

	int count = 0;
	std::map<std::pair<int, NetworkId>, CitizenInfo> const & allCitizens = ServerUniverse::getInstance().getMasterCityObject()->getAllCitizensInfo();
	for (std::map<std::pair<int, NetworkId>, CitizenInfo>::const_iterator i = allCitizens.lower_bound(std::make_pair(cityId, NetworkId::cms_invalid)); i != allCitizens.end(); ++i)
	{
		if (i->first.first != cityId)
			break;

		Chat::sendPersistentMessage("City Hall", i->second.m_citizenName, subject, message, oob);
		++count;
	}

	return std::make_pair(count, cityName);
}

// ----------------------------------------------------------------------

void CityInterface::verifyCitizenName(NetworkId const &citizenId, std::string const &citizenName)
{
	std::vector<int> const & cityId = getCitizenOfCityId(citizenId);
	for (std::vector<int>::const_iterator iterCityId = cityId.begin(); iterCityId != cityId.end(); ++iterCityId)
	{
		CitizenInfo const * const ci = getCitizenInfo(*iterCityId, citizenId);
		if (!ci)
			continue;

		if (ci->m_citizenName == citizenName)
			continue;

		setCitizenInfo(*iterCityId, citizenId, citizenName, ci->m_citizenAllegiance, ci->m_citizenPermissions);
	}
}

// ----------------------------------------------------------------------

void CityInterface::checkForDualCitizenship()
{
	ServerUniverse::getInstance().getMasterCityObject()->checkForDualCitizenship();
}

// ----------------------------------------------------------------------

PgcRatingInfo const * CityInterface::getPgcRating(NetworkId const &chroniclerId)
{
	return ServerUniverse::getInstance().getMasterCityObject()->getPgcRating(chroniclerId);
}

// ----------------------------------------------------------------------

void CityInterface::getPgcChroniclerId(std::string const &chroniclerName, std::vector<NetworkId> &chroniclerIds)
{
	ServerUniverse::getInstance().getMasterCityObject()->getPgcChroniclerId(chroniclerName, chroniclerIds);
}

// ----------------------------------------------------------------------

void CityInterface::adjustPgcRating(NetworkId const &chroniclerId, std::string const &chroniclerName, int adjustment)
{
	ServerUniverse::getInstance().getMasterCityObject()->adjustPgcRating(chroniclerId, chroniclerName, adjustment);
}

// ----------------------------------------------------------------------

void CityInterface::adjustPgcRatingData(NetworkId const &chroniclerId, std::string const &chroniclerName, int index, int adjustment)
{
	ServerUniverse::getInstance().getMasterCityObject()->adjustPgcRatingData(chroniclerId, chroniclerName, index, adjustment);
}

// ----------------------------------------------------------------------

void CityInterface::setPgcRatingData(NetworkId const &chroniclerId, std::string const &chroniclerName, int index, int value)
{
	ServerUniverse::getInstance().getMasterCityObject()->setPgcRatingData(chroniclerId, chroniclerName, index, value);
}

// ----------------------------------------------------------------------

void CityInterface::verifyPgcChroniclerName(NetworkId const &chroniclerId, std::string const &chroniclerName)
{
	ServerUniverse::getInstance().getMasterCityObject()->verifyPgcChroniclerName(chroniclerId, chroniclerName);
}

// ======================================================================
