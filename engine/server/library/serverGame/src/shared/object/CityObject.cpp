// ======================================================================
//
// CityObject.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CityObject.h"

#include "serverGame/GameServer.h"
#include "serverGame/CityController.h"
#include "serverGame/CityInterface.h"
#include "serverGame/CityStringParser.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GuildObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NameManager.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ServerCityObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "sharedGame/CitizenRankDataTable.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/Controller.h"

#include "UnicodeUtils.h"

// ======================================================================

namespace CityObjectNamespace
{
	std::string const getFactionString(uint32 factionId)
	{
		if (PvpData::isNeutralFactionId(factionId))
			return "Neutral";
		else if (PvpData::isImperialFactionId(factionId))
			return "Imperial";
		else if (PvpData::isRebelFactionId(factionId))
			return "Rebel";

		return "Unknown";
	}
};

using namespace CityObjectNamespace;

// ======================================================================

static void replaceSetIfNeeded(char const *label, Archive::AutoDeltaSet<std::string> &oldSet, std::set<std::string> const &newSet, bool force)
{
	if (force || (oldSet.get() != newSet))
	{
		WARNING(true, ("City set %s was inconsistant on load.", label));

		#ifdef _DEBUG
			DEBUG_REPORT_LOG(true, ("Old set:\n"));
			{
				for (std::set<std::string>::const_iterator i = oldSet.begin(); i != oldSet.end(); ++i)
					DEBUG_REPORT_LOG(true, ("%s\n", (*i).c_str()));
			}
			DEBUG_REPORT_LOG(true, ("New set:\n"));
			{
				for (std::set<std::string>::const_iterator i = newSet.begin(); i != newSet.end(); ++i)
					DEBUG_REPORT_LOG(true, ("%s\n", (*i).c_str()));
			}
		#endif

		oldSet.clear();
		for (std::set<std::string>::const_iterator i = newSet.begin(); i != newSet.end(); ++i)
			oldSet.insert(*i);
	}
}

// ======================================================================

CityObject::CityObject(ServerCityObjectTemplate const *newTemplate) :
	UniverseObject(newTemplate),
	m_cities(),
	m_citizens(),
	m_structures(),
	m_citiesInfo(),
	m_citizensInfo(),
	m_structuresInfo(),
	m_citizenToCityId(),
	m_pgcRatingInfo(),
	m_pgcRatingChroniclerId(),
	m_gcwRegionDefenderCities(),
	m_gcwRegionDefenderCitiesCount(),
	m_gcwRegionDefenderCitiesVersion(0),
	m_lowFreeCityId(1000*GameServer::getInstance().getProcessId())
{
	addMembersToPackages();

	m_citizensInfo.setOnErase(this, &CityObject::onCitizensInfoErase);
	m_citizensInfo.setOnInsert(this, &CityObject::onCitizensInfoInsert);
	m_citizensInfo.setOnSet(this, &CityObject::onCitizensInfoSet);
}

// ----------------------------------------------------------------------

CityObject::~CityObject()
{
	if (isInWorld())
		removeFromWorld();
}

// ----------------------------------------------------------------------

Controller* CityObject::createDefaultController()
{
	Controller* controller = new CityController(this);

	setController(controller);
	return controller;
}

// ----------------------------------------------------------------------

void CityObject::setupUniverse()
{
	ServerUniverse::getInstance().registerMasterCityObject(*this);

	if (isAuthoritative())
	{
		// build the city info from data read in from DB

		// disable notification while building the initial list
		m_citizensInfo.setOnErase(nullptr, nullptr);
		m_citizensInfo.setOnInsert(nullptr, nullptr);
		m_citizensInfo.setOnSet(nullptr, nullptr);

		// build cities
		std::map<int, CityInfo> tempCities;
		{
			std::set<std::string> const &cities = m_cities.get();
			for (std::set<std::string>::const_iterator i = cities.begin(); i != cities.end(); ++i)
			{
				int cityId;
				std::string cityName;
				NetworkId cityHallId;
				std::string cityPlanet;
				int x, z, radius;
				int creationTime;
				NetworkId leaderId;
				int incomeTax, propertyTax, salesTax;
				Vector travelLoc;
				int travelCost;
				bool travelInterplanetary;
				Vector cloneLoc;
				Vector cloneRespawn;
				NetworkId cloneRespawnCell;
				NetworkId cloneId;
				if (CityStringParser::parseCityInfo(*i, cityId, cityName, cityHallId, cityPlanet, x, z, radius, creationTime, leaderId, incomeTax, propertyTax, salesTax, travelLoc, travelCost, travelInterplanetary, cloneLoc, cloneRespawn, cloneRespawnCell, cloneId))
				{
					CityInfo & ci = tempCities[cityId];
					ci.setCityInfo(
						cityName, cityHallId,
						cityPlanet, x, z, radius,
						leaderId,
						incomeTax, propertyTax, salesTax,
						travelLoc, travelCost, travelInterplanetary,
						cloneLoc, cloneRespawn, cloneRespawnCell, cloneId);

					ci.setCityCreationTime(creationTime);
				}
			}
		}

		// build citizens
		{
			NetworkId pgcChroniclerId;
			PgcRatingInfo pgcRatingInfo;

			std::set<std::string> const &citizens = m_citizens.get();
			for (std::set<std::string>::const_iterator i = citizens.begin(); i != citizens.end(); ++i)
			{
				int cityId;
				NetworkId citizenId;
				std::string citizenName;
				std::string citizenProfessionSkillTemplate;
				int citizenLevel;
				int citizenPermissions;
				BitArray citizenRank;
				std::string citizenTitle;
				NetworkId citizenAllegiance;
				if (CityStringParser::parseCitizenInfo(*i, cityId, citizenId, citizenName, citizenProfessionSkillTemplate, citizenLevel, citizenPermissions, citizenRank, citizenTitle, citizenAllegiance))
				{
					std::map<int, CityInfo>::iterator j = tempCities.find(cityId);
					if (j != tempCities.end())
					{
						if (m_citizensInfo.find(std::make_pair(cityId, citizenId)) == m_citizensInfo.end())
							j->second.setCitizenCount(j->second.getCitizenCount() + 1);

						CitizenInfo ci;
						ci.m_citizenName = citizenName;
						ci.m_citizenProfessionSkillTemplate = citizenProfessionSkillTemplate;
						ci.m_citizenLevel = citizenLevel;
						ci.m_citizenPermissions = citizenPermissions;
						ci.m_citizenRank = citizenRank;
						ci.m_citizenTitle = citizenTitle;
						ci.m_citizenAllegiance = citizenAllegiance;

						m_citizensInfo.set(std::make_pair(cityId, citizenId), ci);

						// if someone has the dual citizenship issue, the city
						// that he is a mayor of should take priority
						std::map<NetworkId, std::vector<int> >::const_iterator const iterCityOfCitizen = m_citizenToCityId.find(citizenId);
						if (iterCityOfCitizen == m_citizenToCityId.end())
						{
							std::vector<int> citizenCity;
							citizenCity.push_back(cityId);
							m_citizenToCityId.set(citizenId, citizenCity);
						}
						else
						{
							std::vector<int> citizenCity = iterCityOfCitizen->second;
							if (j->second.getLeaderId() == citizenId)
							{
								citizenCity.push_back(citizenCity.front());
								citizenCity[0] = cityId;
							}
							else
							{
								citizenCity.push_back(cityId);
							}

							m_citizenToCityId.set(citizenId, citizenCity);
						}
					}
				}
				else if (CityStringParser::parsePgcRatingInfo(*i, pgcChroniclerId, pgcRatingInfo))
				{
					m_pgcRatingInfo.set(pgcChroniclerId, pgcRatingInfo);
					m_pgcRatingChroniclerId.insert(std::make_pair(NameManager::normalizeName(pgcRatingInfo.m_chroniclerName), pgcChroniclerId));
				}
			}
		}

		// build structures
		{
			std::set<std::string> const &structures = m_structures.get();
			for (std::set<std::string>::const_iterator i = structures.begin(); i != structures.end(); ++i)
			{
				int cityId;
				NetworkId structureId;
				int structureType;
				bool structureValid;
				if (CityStringParser::parseCityStructureInfo(*i, cityId, structureId, structureType, structureValid))
				{
					std::map<int, CityInfo>::iterator j = tempCities.find(cityId);
					if (j != tempCities.end())
					{
						if (m_structuresInfo.find(std::make_pair(cityId, structureId)) == m_structuresInfo.end())
							j->second.setStructureCount(j->second.getStructureCount() + 1);

						CityStructureInfo csi;
						csi.setStructureType(structureType);
						csi.setStructureValid(structureValid);

						m_structuresInfo.set(std::make_pair(cityId, structureId), csi);
					}
				}
			}
		}

		// save off final city info
		std::vector<std::pair<int, int> > cityIds;
		{
			for (std::map<int, CityInfo>::const_iterator iterCity = tempCities.begin(); iterCity != tempCities.end(); ++iterCity)
			{
				CityInfo const &ci = iterCity->second;
				m_citiesInfo.set(iterCity->first, ci);

				// get city ids, excluding any citys with empty names or no city hall id
				if (!ci.getCityName().empty() && ci.getCityHallId() != NetworkId::cms_invalid && !ci.getPlanet().empty())
					cityIds.push_back(std::make_pair(iterCity->first, ci.getCreationTime()));
			}
		}

		// ***DO NOT*** add city travel points here, since we are not guaranteed that
		// all the planet master objects have loaded and initialized yet; we'll wait
		// until the cluster completes loading, and add the city travel points then

		// rebuild cities
		{
			std::set<std::string> cities;
			std::list<std::pair<std::string, std::string> > citiesToRetroactiveSetCreationTime;

			for (std::vector<std::pair<int, int> >::const_iterator i = cityIds.begin(); i != cityIds.end(); ++i)
			{
				std::string citySpec;
				getCitySpec(i->first, citySpec);
				cities.insert(citySpec);

				// see if city needs to have its creation time retroactively set
				if (i->second <= 0)
				{
					time_t const retroactiveCreationTime = GameServer::getRetroactivePlayerCityCreationTime(GameServer::getInstance().getClusterName(), i->first);
					if (retroactiveCreationTime > 0)
					{
						std::string updatedCitySpec;
						CityInfo const &ci = getCityInfo(i->first);
						CityStringParser::buildCitySpec(
							i->first, ci.getCityName(), ci.getCityHallId(),
							ci.getPlanet(), ci.getX(), ci.getZ(), ci.getRadius(),
							static_cast<int>(retroactiveCreationTime),
							ci.getLeaderId(),
							ci.getIncomeTax(), ci.getPropertyTax(), ci.getSalesTax(),
							ci.getTravelLoc(), ci.getTravelCost(), ci.getTravelInterplanetary(),
							ci.getCloneLoc(), ci.getCloneRespawn(), ci.getCloneRespawnCell(), ci.getCloneId(),
							updatedCitySpec);

						citiesToRetroactiveSetCreationTime.push_back(std::make_pair(citySpec, updatedCitySpec));
					}
				}
			}

			if (m_cities.get() != cities)
			{
				// since we're going to be replacing everything, take the opportunity
				// to set the retroactive creation time for those cities that need it
				for (std::list<std::pair<std::string, std::string> >::const_iterator iter = citiesToRetroactiveSetCreationTime.begin(); iter != citiesToRetroactiveSetCreationTime.end(); ++iter)
				{
					cities.erase(iter->first);
					cities.insert(iter->second);
				}

				replaceSetIfNeeded("cities", m_cities, cities, true);
			}
			else
			{
				// set the retroactive creation time for those cities that need it
				for (std::list<std::pair<std::string, std::string> >::const_iterator iter = citiesToRetroactiveSetCreationTime.begin(); iter != citiesToRetroactiveSetCreationTime.end(); ++iter)
				{
					m_cities.erase(iter->first);
					m_cities.insert(iter->second);
				}
			}
		}

		// rebuild citizens
		{
			std::set<std::string> citizens;
			for (std::vector<std::pair<int, int> >::const_iterator i = cityIds.begin(); i != cityIds.end(); ++i)
			{
				std::vector<NetworkId> citizenIds;
				getCitizenIds(i->first, citizenIds);
				for (std::vector<NetworkId>::const_iterator j = citizenIds.begin(); j != citizenIds.end(); ++j)
				{
					std::string citizenSpec;
					getCitizenSpec(i->first, *j, citizenSpec);
					citizens.insert(citizenSpec);
				}
			}

			for (std::map<NetworkId, PgcRatingInfo>::const_iterator iter = m_pgcRatingInfo.begin(); iter != m_pgcRatingInfo.end(); ++iter)
			{
				std::string pgcRatingSpec;
				CityStringParser::buildPgcRatingSpec(iter->first, iter->second, pgcRatingSpec);
				citizens.insert(pgcRatingSpec);
			}

			replaceSetIfNeeded("citizens", m_citizens, citizens, false);
		}

		// rebuild structures
		{
			std::set<std::string> structures;
			for (std::vector<std::pair<int, int> >::const_iterator i = cityIds.begin(); i != cityIds.end(); ++i)
			{
				std::vector<NetworkId> structureIds;
				getCityStructureIds(i->first, structureIds);
				for (std::vector<NetworkId>::const_iterator j = structureIds.begin(); j != structureIds.end(); ++j)
				{
					std::string structureSpec;
					getCityStructureSpec(i->first, *j, structureSpec);
					structures.insert(structureSpec);
				}
			}
			replaceSetIfNeeded("structures", m_structures, structures, false);
		}

		// re-enable notification
		m_citizensInfo.setOnErase(this, &CityObject::onCitizensInfoErase);
		m_citizensInfo.setOnInsert(this, &CityObject::onCitizensInfoInsert);
		m_citizensInfo.setOnSet(this, &CityObject::onCitizensInfoSet);
	}
}

// ----------------------------------------------------------------------

void CityObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	UniverseObject::getAttributes(data);
}

// ----------------------------------------------------------------------

int CityObject::nextFreeCityId()
{
	// Note: this must return a city id which is not only unused, but also one
	// that cannot have collisions with a free cityId generated by another
	// server, even during the same frame.  This is because we need immediate
	// feedback of what the newly created city's ID is, regardless of whether
	// the master city object is authoritative on this server.
	// In order to do this, we find the lowest unused id starting from
	// 1000 * our process id + 1.  We also need to assure that if multiple citys
	// are created from the same process, they do not collide in the time before
	// the creation actually takes place, since we may not have up to date
	// city info if we are not authoritative.

	do
	{
		++m_lowFreeCityId;
	}
	while (cityExists(m_lowFreeCityId));
	return m_lowFreeCityId;
}

// ----------------------------------------------------------------------

int CityObject::createCity(std::string const &cityName, NetworkId const &cityHallId, std::string const &cityPlanet, int x, int z, int radius, NetworkId const &leaderId, int incomeTax, int propertyTax, int salesTax, Vector const &travelLoc, int travelCost, bool travelInterplanetary, Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId, int cityId)
{
	if (cityId == 0)
		cityId = nextFreeCityId();

	if (!isAuthoritative())
	{
		CityInfo ci;
		ci.setCityInfo(
			cityName, cityHallId,
			cityPlanet, x, z, radius,
			leaderId,
			incomeTax, propertyTax, salesTax,
			travelLoc, travelCost, travelInterplanetary,
			cloneLoc, cloneRespawn, cloneRespawnCell, cloneId);

		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_cityCreate,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, CityInfo> >(std::make_pair(cityId, ci)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		CityInfo ci;
		ci.setCityInfo(
			cityName, cityHallId,
			cityPlanet, x, z, radius,
			leaderId,
			incomeTax, propertyTax, salesTax,
			travelLoc, travelCost, travelInterplanetary,
			cloneLoc, cloneRespawn, cloneRespawnCell, cloneId);
		ci.setCityCreationTime(static_cast<int>(::time(nullptr)));
		m_citiesInfo.set(cityId, ci);

		std::string citySpec;
		getCitySpec(cityId, citySpec);
		m_cities.insert(citySpec);

		// add travel point if the new city can support it
		if ((radius >= 400) && (travelCost > 0))
		{
			PlanetObject *planet = ServerUniverse::getInstance().getPlanetByName(cityPlanet);
			if (planet)
			{
				planet->addTravelPoint(
					cityName,
					travelLoc,
					travelCost,
					travelInterplanetary,
					TravelPoint::TPT_PC_Shuttleport);
			}
		}
	}

	return cityId;
}

// ----------------------------------------------------------------------

void CityObject::removeCity(int cityId)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_cityRemove,
				0.0f,
				new MessageQueueGenericValueType<int>(cityId),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		// remove citizens
		{
			std::vector<NetworkId> citizenIds;
			getCitizenIds(cityId, citizenIds);
			for (std::vector<NetworkId>::const_iterator i = citizenIds.begin(); i != citizenIds.end(); ++i)
			{
				std::string citizenSpec;
				getCitizenSpec(cityId, *i, citizenSpec);
				m_citizens.erase(citizenSpec);

				m_citizensInfo.erase(std::make_pair(cityId, *i));

				std::map<NetworkId, std::vector<int> >::const_iterator const iterCitizenCityId = m_citizenToCityId.find(*i);
				if (iterCitizenCityId != m_citizenToCityId.end())
				{
					std::vector<int> citizenCityId = iterCitizenCityId->second;
					std::vector<int>::iterator const iterFindCityId = std::find(citizenCityId.begin(), citizenCityId.end(), cityId);
					if (iterFindCityId != citizenCityId.end())
					{
						citizenCityId.erase(iterFindCityId);

						if (citizenCityId.empty())
							m_citizenToCityId.erase(*i);
						else
							m_citizenToCityId.set(*i, citizenCityId);
					}
				}

				// notify (now former) citizen
				char buffer[32];
				snprintf(buffer, sizeof(buffer)-1, "%d", cityId);
				buffer[sizeof(buffer)-1] = '\0';

				MessageToQueue::getInstance().sendMessageToC(*i,
					"C++CityRemoveCitizen",
					buffer,
					0,
					false);
			}
		}
		// remove structures
		{
			std::vector<NetworkId> structureIds;
			getCityStructureIds(cityId, structureIds);
			for (std::vector<NetworkId>::const_iterator i = structureIds.begin(); i != structureIds.end(); ++i)
			{
				std::string structureSpec;
				getCityStructureSpec(cityId, *i, structureSpec);
				m_structures.erase(structureSpec);

				m_structuresInfo.erase(std::make_pair(cityId, *i));
			}
		}
		// remove city
		{
			// remove travel point if the city has one
			std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
			if (iterCityInfo != m_citiesInfo.end())
			{
				if ((iterCityInfo->second.getRadius() >= 400) && (iterCityInfo->second.getTravelCost() > 0))
				{
					PlanetObject *planet = ServerUniverse::getInstance().getPlanetByName(iterCityInfo->second.getPlanet());
					if (planet)
					{
						planet->removeTravelPoint(iterCityInfo->second.getCityName());
					}
				}

				std::string const & gcwDefenderRegion = iterCityInfo->second.getGcwDefenderRegion();
				if (!gcwDefenderRegion.empty())
				{
					removeGcwRegionDefenderCity(gcwDefenderRegion, cityId, iterCityInfo->second.getFaction());
				}
			}

			// Note: because city specs contain float values, we need to be sure to pull them from
			// the actual string list of cities rather than building it when erasing.
			std::string citySpec;
			if (findCitySpec(cityId, citySpec))
				m_cities.erase(citySpec);

			m_citiesInfo.erase(cityId);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::removeCitizen(int cityId, NetworkId const &citizenId)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_cityRemoveCitizen,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, NetworkId> >(std::make_pair(cityId, citizenId)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		std::string citizenSpec;
		if (getCitizenSpec(cityId, citizenId, citizenSpec) && !citizenSpec.empty())
		{
			m_citizens.erase(citizenSpec);

			m_citizensInfo.erase(std::make_pair(cityId, citizenId));

			std::map<NetworkId, std::vector<int> >::const_iterator const iterCitizenCityId = m_citizenToCityId.find(citizenId);
			if (iterCitizenCityId != m_citizenToCityId.end())
			{
				std::vector<int> citizenCityId = iterCitizenCityId->second;
				std::vector<int>::iterator const iterFindCityId = std::find(citizenCityId.begin(), citizenCityId.end(), cityId);
				if (iterFindCityId != citizenCityId.end())
				{
					citizenCityId.erase(iterFindCityId);

					if (citizenCityId.empty())
						m_citizenToCityId.erase(citizenId);
					else
						m_citizenToCityId.set(citizenId, citizenCityId);

					// notify (now former) citizen
					char buffer[32];
					snprintf(buffer, sizeof(buffer)-1, "%d", cityId);
					buffer[sizeof(buffer)-1] = '\0';

					MessageToQueue::getInstance().sendMessageToC(citizenId,
						"C++CityRemoveCitizen",
						buffer,
						0,
						false);
				}
			}

			std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
			if (iterCityInfo != m_citiesInfo.end())
			{
				CityInfo updatedCi = iterCityInfo->second;
				updatedCi.setCitizenCount(std::max(0, (updatedCi.getCitizenCount() - 1)));
				m_citiesInfo.set(cityId, updatedCi);
			}
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::removeCityStructure(int cityId, NetworkId const &structureId)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_cityRemoveStructure,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, NetworkId> >(std::make_pair(cityId, structureId)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		std::string structureSpec;
		if (getCityStructureSpec(cityId, structureId, structureSpec) && !structureSpec.empty())
		{
			m_structures.erase(structureSpec);

			m_structuresInfo.erase(std::make_pair(cityId, structureId));

			std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
			if (iterCityInfo != m_citiesInfo.end())
			{
				CityInfo updatedCi = iterCityInfo->second;
				updatedCi.setStructureCount(std::max(0, (updatedCi.getStructureCount() - 1)));
				m_citiesInfo.set(cityId, updatedCi);
			}
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCityName(int cityId, std::string const &cityName)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetName,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, std::string> >(std::make_pair(cityId, cityName)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (cityExists(cityId))
	{
		CityInfo const &ci = getCityInfo(cityId);
		std::string newCitySpec;
		CityStringParser::buildCitySpec(
			cityId, cityName, ci.getCityHallId(),
			ci.getPlanet(), ci.getX(), ci.getZ(), ci.getRadius(),
			ci.getCreationTime(),
			ci.getLeaderId(),
			ci.getIncomeTax(), ci.getPropertyTax(), ci.getSalesTax(),
			ci.getTravelLoc(), ci.getTravelCost(), ci.getTravelInterplanetary(),
			ci.getCloneLoc(), ci.getCloneRespawn(), ci.getCloneRespawnCell(), ci.getCloneId(),
			newCitySpec);
		setCitySpec(cityId, newCitySpec);

		std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
		if (iterCityInfo != m_citiesInfo.end())
		{
			CityInfo updatedCi = iterCityInfo->second;
			updatedCi.setCityName(cityName);
			m_citiesInfo.set(cityId, updatedCi);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCityHall(int cityId, NetworkId const &cityHallId)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetCityHall,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, NetworkId> >(std::make_pair(cityId, cityHallId)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (cityExists(cityId))
	{
		CityInfo const &ci = getCityInfo(cityId);
		std::string newCitySpec;
		CityStringParser::buildCitySpec(
			cityId, ci.getCityName(), cityHallId,
			ci.getPlanet(), ci.getX(), ci.getZ(), ci.getRadius(),
			ci.getCreationTime(),
			ci.getLeaderId(),
			ci.getIncomeTax(), ci.getPropertyTax(), ci.getSalesTax(),
			ci.getTravelLoc(), ci.getTravelCost(), ci.getTravelInterplanetary(),
			ci.getCloneLoc(), ci.getCloneRespawn(), ci.getCloneRespawnCell(), ci.getCloneId(),
			newCitySpec);
		setCitySpec(cityId, newCitySpec);

		std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
		if (iterCityInfo != m_citiesInfo.end())
		{
			CityInfo updatedCi = iterCityInfo->second;
			updatedCi.setCityHall(cityHallId);
			m_citiesInfo.set(cityId, updatedCi);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCityLocation(int cityId, std::string const &cityPlanet, int x, int z)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetLocation,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, std::pair<std::string, std::pair<int, int> > > >(std::make_pair(cityId, std::make_pair(cityPlanet, std::make_pair(x, z)))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (cityExists(cityId))
	{
		CityInfo const &ci = getCityInfo(cityId);
		std::string newCitySpec;
		CityStringParser::buildCitySpec(
			cityId, ci.getCityName(), ci.getCityHallId(),
			cityPlanet, x, z, ci.getRadius(),
			ci.getCreationTime(),
			ci.getLeaderId(),
			ci.getIncomeTax(), ci.getPropertyTax(), ci.getSalesTax(),
			ci.getTravelLoc(), ci.getTravelCost(), ci.getTravelInterplanetary(),
			ci.getCloneLoc(), ci.getCloneRespawn(), ci.getCloneRespawnCell(), ci.getCloneId(),
			newCitySpec);
		setCitySpec(cityId, newCitySpec);

		std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
		if (iterCityInfo != m_citiesInfo.end())
		{
			CityInfo updatedCi = iterCityInfo->second;
			updatedCi.setCityLocation(cityPlanet, x, z);
			m_citiesInfo.set(cityId, updatedCi);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCityRadius(int cityId, int radius)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetRadius,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, int> >(std::make_pair(cityId, radius)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (cityExists(cityId))
	{
		CityInfo const &ci = getCityInfo(cityId);

		// add or remove travel point if the city can support it or can no longer support it
		if ((ci.getRadius() >= 400) && (radius < 400) && (ci.getTravelCost() > 0))
		{
			PlanetObject *planet = ServerUniverse::getInstance().getPlanetByName(ci.getPlanet());
			if (planet)
			{
				planet->removeTravelPoint(ci.getCityName());
			}
		}
		else if ((ci.getRadius() < 400) && (radius >= 400) && (ci.getTravelCost() > 0))
		{
			PlanetObject *planet = ServerUniverse::getInstance().getPlanetByName(ci.getPlanet());
			if (planet)
			{
				planet->addTravelPoint(
					ci.getCityName(),
					ci.getTravelLoc(),
					ci.getTravelCost(),
					ci.getTravelInterplanetary(),
					TravelPoint::TPT_PC_Shuttleport);
			}
		}

		std::string newCitySpec;
		CityStringParser::buildCitySpec(
			cityId, ci.getCityName(), ci.getCityHallId(),
			ci.getPlanet(), ci.getX(), ci.getZ(), radius,
			ci.getCreationTime(),
			ci.getLeaderId(),
			ci.getIncomeTax(), ci.getPropertyTax(), ci.getSalesTax(),
			ci.getTravelLoc(), ci.getTravelCost(), ci.getTravelInterplanetary(),
			ci.getCloneLoc(), ci.getCloneRespawn(), ci.getCloneRespawnCell(), ci.getCloneId(),
			newCitySpec);
		setCitySpec(cityId, newCitySpec);

		std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
		if (iterCityInfo != m_citiesInfo.end())
		{
			CityInfo updatedCi = iterCityInfo->second;
			updatedCi.setCityRadius(radius);
			m_citiesInfo.set(cityId, updatedCi);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCityFaction(int cityId, uint32 faction, bool notifyCitizens)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetFaction,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, std::pair<uint32, bool> > >(std::make_pair(cityId, std::make_pair(faction, notifyCitizens))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (cityExists(cityId))
	{
		std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
		if (iterCityInfo != m_citiesInfo.end())
		{
			CityInfo updatedCi = iterCityInfo->second;
			uint32 const oldFaction = updatedCi.getFaction();
			updatedCi.setCityFaction(faction);
			m_citiesInfo.set(cityId, updatedCi);

			// if city faction has changed, send mail to citizens
			if ((oldFaction != faction) && notifyCitizens)
			{
				static const Unicode::String subject = Unicode::narrowToWide("@" + StringId("city/city", "mail_subject_city_faction_changed").getCanonicalRepresentation());

				ProsePackage pp;
				pp.stringId = StringId("city/city", "mail_body_city_faction_changed");
				pp.actor.str = Unicode::narrowToWide(updatedCi.getCityName());
				pp.target.str = Unicode::narrowToWide(getFactionString(oldFaction));
				pp.other.str = Unicode::narrowToWide(getFactionString(faction));

				Unicode::String oob;
				OutOfBandPackager::pack(pp, -1, oob);

				CityInterface::mailToCitizens(cityId, subject, Unicode::emptyString, oob);
			}

			std::string const & gcwDefenderRegion = updatedCi.getGcwDefenderRegion();
			if ((oldFaction != faction) && !gcwDefenderRegion.empty())
			{
				removeGcwRegionDefenderCity(gcwDefenderRegion, cityId, oldFaction);
				addGcwRegionDefenderCity(gcwDefenderRegion, cityId, faction);
			}
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCityGcwDefenderRegion(int cityId, std::string const &gcwDefenderRegion, int timeJoined, bool notifyCitizens)
{
	if (!gcwDefenderRegion.empty())
	{
		Pvp::GcwScoreCategory const * const gcwScoreCategory = Pvp::getGcwScoreCategory(gcwDefenderRegion);
		if (!gcwScoreCategory || !gcwScoreCategory->gcwRegionDefender)
			return;
	}

	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetGcwDefenderRegion,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::pair<int, std::string>, std::pair<int, bool> > >(std::make_pair(std::make_pair(cityId, gcwDefenderRegion), std::make_pair(timeJoined, notifyCitizens))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (cityExists(cityId))
	{
		std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
		if (iterCityInfo != m_citiesInfo.end())
		{
			CityInfo updatedCi = iterCityInfo->second;
			std::string const oldGcwDefenderRegion = updatedCi.getGcwDefenderRegion();
			updatedCi.setCityGcwDefenderRegion(gcwDefenderRegion);
			updatedCi.setCityTimeJoinedGcwDefenderRegion(timeJoined);
			m_citiesInfo.set(cityId, updatedCi);

			// if city GCW defender region has changed, send mail to citizens
			if ((oldGcwDefenderRegion != gcwDefenderRegion) && notifyCitizens)
			{
				static const Unicode::String subject = Unicode::narrowToWide("@" + StringId("city/city", "mail_subject_city_gcw_defender_region_changed").getCanonicalRepresentation());

				ProsePackage pp;
				pp.stringId = StringId("city/city", "mail_body_city_gcw_defender_region_changed");
				pp.actor.str = Unicode::narrowToWide(updatedCi.getCityName());
				pp.target.str = Unicode::narrowToWide(oldGcwDefenderRegion.empty() ? "(None)" : ("@" + StringId("gcw_regions", oldGcwDefenderRegion).getCanonicalRepresentation()));
				pp.other.str = Unicode::narrowToWide(gcwDefenderRegion.empty() ? "(None)" : ("@" + StringId("gcw_regions", gcwDefenderRegion).getCanonicalRepresentation()));

				Unicode::String oob;
				OutOfBandPackager::pack(pp, -1, oob);

				CityInterface::mailToCitizens(cityId, subject, Unicode::emptyString, oob);
			}

			if (oldGcwDefenderRegion != gcwDefenderRegion)
			{
				removeGcwRegionDefenderCity(oldGcwDefenderRegion, cityId, updatedCi.getFaction());
				addGcwRegionDefenderCity(gcwDefenderRegion, cityId, updatedCi.getFaction());
			}
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCityCreationTime(int cityId, int creationTime)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetCreationTime,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, int> >(std::make_pair(cityId, creationTime)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (cityExists(cityId))
	{
		CityInfo const &ci = getCityInfo(cityId);
		std::string newCitySpec;
		CityStringParser::buildCitySpec(
			cityId, ci.getCityName(), ci.getCityHallId(),
			ci.getPlanet(), ci.getX(), ci.getZ(), ci.getRadius(),
			creationTime,
			ci.getLeaderId(),
			ci.getIncomeTax(), ci.getPropertyTax(), ci.getSalesTax(),
			ci.getTravelLoc(), ci.getTravelCost(), ci.getTravelInterplanetary(),
			ci.getCloneLoc(), ci.getCloneRespawn(), ci.getCloneRespawnCell(), ci.getCloneId(),
			newCitySpec);
		setCitySpec(cityId, newCitySpec);

		std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
		if (iterCityInfo != m_citiesInfo.end())
		{
			CityInfo updatedCi = iterCityInfo->second;
			updatedCi.setCityCreationTime(creationTime);
			m_citiesInfo.set(cityId, updatedCi);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCityLeader(int cityId, NetworkId const &leaderId)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetLeader,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, NetworkId> >(std::make_pair(cityId, leaderId)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (cityExists(cityId))
	{
		CityInfo const &ci = getCityInfo(cityId);
		std::string newCitySpec;
		CityStringParser::buildCitySpec(
			cityId, ci.getCityName(), ci.getCityHallId(),
			ci.getPlanet(), ci.getX(), ci.getZ(), ci.getRadius(),
			ci.getCreationTime(),
			leaderId,
			ci.getIncomeTax(), ci.getPropertyTax(), ci.getSalesTax(),
			ci.getTravelLoc(), ci.getTravelCost(), ci.getTravelInterplanetary(),
			ci.getCloneLoc(), ci.getCloneRespawn(), ci.getCloneRespawnCell(), ci.getCloneId(),
			newCitySpec);
		setCitySpec(cityId, newCitySpec);

		std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
		if (iterCityInfo != m_citiesInfo.end())
		{
			CityInfo updatedCi = iterCityInfo->second;
			updatedCi.setCityLeader(leaderId);
			m_citiesInfo.set(cityId, updatedCi);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCityIncomeTax(int cityId, int incomeTax)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetIncomeTax,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, int> >(std::make_pair(cityId, incomeTax)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (cityExists(cityId))
	{
		CityInfo const &ci = getCityInfo(cityId);
		std::string newCitySpec;
		CityStringParser::buildCitySpec(
			cityId, ci.getCityName(), ci.getCityHallId(),
			ci.getPlanet(), ci.getX(), ci.getZ(), ci.getRadius(),
			ci.getCreationTime(),
			ci.getLeaderId(),
			incomeTax, ci.getPropertyTax(), ci.getSalesTax(),
			ci.getTravelLoc(), ci.getTravelCost(), ci.getTravelInterplanetary(),
			ci.getCloneLoc(), ci.getCloneRespawn(), ci.getCloneRespawnCell(), ci.getCloneId(),
			newCitySpec);
		setCitySpec(cityId, newCitySpec);

		std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
		if (iterCityInfo != m_citiesInfo.end())
		{
			CityInfo updatedCi = iterCityInfo->second;
			updatedCi.setCityIncomeTax(incomeTax);
			m_citiesInfo.set(cityId, updatedCi);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCityPropertyTax(int cityId, int propertyTax)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetPropertyTax,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, int> >(std::make_pair(cityId, propertyTax)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (cityExists(cityId))
	{
		CityInfo const &ci = getCityInfo(cityId);
		std::string newCitySpec;
		CityStringParser::buildCitySpec(
			cityId, ci.getCityName(), ci.getCityHallId(),
			ci.getPlanet(), ci.getX(), ci.getZ(), ci.getRadius(),
			ci.getCreationTime(),
			ci.getLeaderId(),
			ci.getIncomeTax(), propertyTax, ci.getSalesTax(),
			ci.getTravelLoc(), ci.getTravelCost(), ci.getTravelInterplanetary(),
			ci.getCloneLoc(), ci.getCloneRespawn(), ci.getCloneRespawnCell(), ci.getCloneId(),
			newCitySpec);
		setCitySpec(cityId, newCitySpec);

		std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
		if (iterCityInfo != m_citiesInfo.end())
		{
			CityInfo updatedCi = iterCityInfo->second;
			updatedCi.setCityPropertyTax(propertyTax);
			m_citiesInfo.set(cityId, updatedCi);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCitySalesTax(int cityId, int salesTax)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetSalesTax,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, int> >(std::make_pair(cityId, salesTax)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (cityExists(cityId))
	{
		CityInfo const &ci = getCityInfo(cityId);
		std::string newCitySpec;
		CityStringParser::buildCitySpec(
			cityId, ci.getCityName(), ci.getCityHallId(),
			ci.getPlanet(), ci.getX(), ci.getZ(), ci.getRadius(),
			ci.getCreationTime(),
			ci.getLeaderId(),
			ci.getIncomeTax(), ci.getPropertyTax(), salesTax,
			ci.getTravelLoc(), ci.getTravelCost(), ci.getTravelInterplanetary(),
			ci.getCloneLoc(), ci.getCloneRespawn(), ci.getCloneRespawnCell(), ci.getCloneId(),
			newCitySpec);
		setCitySpec(cityId, newCitySpec);

		std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
		if (iterCityInfo != m_citiesInfo.end())
		{
			CityInfo updatedCi = iterCityInfo->second;
			updatedCi.setCitySalesTax(salesTax);
			m_citiesInfo.set(cityId, updatedCi);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCityTravelInfo(int cityId, Vector const &travelLoc, int travelCost, bool travelInterplanetary)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetTravelInfo,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, std::pair<Vector, std::pair<int, bool> > > >(std::make_pair(cityId, std::make_pair(travelLoc, std::make_pair(travelCost, travelInterplanetary)))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (cityExists(cityId))
	{
		CityInfo const &ci = getCityInfo(cityId);

		// add or remove travel point if the city can support it or can no longer support it
		bool travelPointAdded = false;
		bool const travelPointChanged = ((ci.getTravelLoc() != travelLoc) || (ci.getTravelCost() != travelCost) || (ci.getTravelInterplanetary() != travelInterplanetary));
		if ((ci.getRadius() >= 400) && (ci.getTravelCost() > 0) && (travelCost <= 0))
		{
			PlanetObject *planet = ServerUniverse::getInstance().getPlanetByName(ci.getPlanet());
			if (planet)
			{
				planet->removeTravelPoint(ci.getCityName());
			}
		}
		else if ((ci.getRadius() >= 400) && (ci.getTravelCost() <= 0) && (travelCost > 0))
		{
			PlanetObject *planet = ServerUniverse::getInstance().getPlanetByName(ci.getPlanet());
			if (planet)
			{
				planet->addTravelPoint(
					ci.getCityName(),
					travelLoc,
					travelCost,
					travelInterplanetary,
					TravelPoint::TPT_PC_Shuttleport);

				travelPointAdded = true;
			}
		}

		// see if we need to update the travel point
		if (travelPointChanged && !travelPointAdded && (ci.getRadius() >= 400) && (travelCost > 0))
		{
			PlanetObject *planet = ServerUniverse::getInstance().getPlanetByName(ci.getPlanet());
			if (planet)
			{
				planet->removeTravelPoint(ci.getCityName());

				planet->addTravelPoint(
					ci.getCityName(),
					travelLoc,
					travelCost,
					travelInterplanetary,
					TravelPoint::TPT_PC_Shuttleport);
			}
		}

		std::string newCitySpec;
		CityStringParser::buildCitySpec(
			cityId, ci.getCityName(), ci.getCityHallId(),
			ci.getPlanet(), ci.getX(), ci.getZ(), ci.getRadius(),
			ci.getCreationTime(),
			ci.getLeaderId(),
			ci.getIncomeTax(), ci.getPropertyTax(), ci.getSalesTax(),
			travelLoc, travelCost, travelInterplanetary,
			ci.getCloneLoc(), ci.getCloneRespawn(), ci.getCloneRespawnCell(), ci.getCloneId(),
			newCitySpec);
		setCitySpec(cityId, newCitySpec);

		std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
		if (iterCityInfo != m_citiesInfo.end())
		{
			CityInfo updatedCi = iterCityInfo->second;
			updatedCi.setCityTravelInfo(travelLoc, travelCost, travelInterplanetary);
			m_citiesInfo.set(cityId, updatedCi);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCityCloneInfo(int cityId, Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetCloneInfo,
				0.0f,
				new MessageQueueGenericValueType<std::pair<int, std::pair<Vector, std::pair<Vector, std::pair<NetworkId, NetworkId> > > > >(std::make_pair(cityId, std::make_pair(cloneLoc, std::make_pair(cloneRespawn, std::make_pair(cloneRespawnCell, cloneId))))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else if (cityExists(cityId))
	{
		CityInfo const &ci = getCityInfo(cityId);
		std::string newCitySpec;
		CityStringParser::buildCitySpec(
			cityId, ci.getCityName(), ci.getCityHallId(),
			ci.getPlanet(), ci.getX(), ci.getZ(), ci.getRadius(),
			ci.getCreationTime(),
			ci.getLeaderId(),
			ci.getIncomeTax(), ci.getPropertyTax(), ci.getSalesTax(),
			ci.getTravelLoc(), ci.getTravelCost(), ci.getTravelInterplanetary(),
			cloneLoc, cloneRespawn, cloneRespawnCell, cloneId,
			newCitySpec);
		setCitySpec(cityId, newCitySpec);

		std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
		if (iterCityInfo != m_citiesInfo.end())
		{
			CityInfo updatedCi = iterCityInfo->second;
			updatedCi.setCityCloneInfo(cloneLoc, cloneRespawn, cloneRespawnCell, cloneId);
			m_citiesInfo.set(cityId, updatedCi);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCitizen(int cityId, NetworkId const &citizenId, std::string const &citizenName, NetworkId const &allegiance, int permissions)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetCitizen,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<NetworkId, int> > > >(std::make_pair(std::make_pair(cityId, citizenId), std::make_pair(citizenName, std::make_pair(allegiance, permissions)))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		std::string oldCitizenSpec, newCitizenSpec;
		CitizenInfo const * existingInfo = getCitizenSpec(cityId, citizenId, oldCitizenSpec);

		CityStringParser::buildCitizenSpec(cityId, citizenId, citizenName, (existingInfo ? existingInfo->m_citizenProfessionSkillTemplate : std::string()), (existingInfo ? existingInfo->m_citizenLevel : 0), permissions, (existingInfo ? existingInfo->m_citizenRank : BitArray()), (existingInfo ? existingInfo->m_citizenTitle : std::string()), allegiance, newCitizenSpec);
		if (oldCitizenSpec != newCitizenSpec)
			setCitizen(oldCitizenSpec, newCitizenSpec);

		if (!existingInfo)
		{
			std::map<NetworkId, std::vector<int> >::const_iterator const iterCityOfCitizen = m_citizenToCityId.find(citizenId);
			if (iterCityOfCitizen == m_citizenToCityId.end())
			{
				std::vector<int> citizenCity;
				citizenCity.push_back(cityId);
				m_citizenToCityId.set(citizenId, citizenCity);
			}
			else
			{
				std::vector<int> citizenCity = iterCityOfCitizen->second;

				// if someone has the dual citizenship issue, the city
				// that he is a mayor of should take priority
				bool mayor = false;
				{
					std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
					if ((iterCityInfo != m_citiesInfo.end()) && (iterCityInfo->second.getLeaderId() == citizenId))
						mayor = true;
				}

				if (mayor)
				{
					citizenCity.push_back(citizenCity.front());
					citizenCity[0] = cityId;
				}
				else
				{
					citizenCity.push_back(cityId);
				}

				m_citizenToCityId.set(citizenId, citizenCity);
			}

			std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
			if (iterCityInfo != m_citiesInfo.end())
			{
				CityInfo updatedCi = iterCityInfo->second;
				updatedCi.setCitizenCount(updatedCi.getCitizenCount() + 1);
				m_citiesInfo.set(cityId, updatedCi);
			}

			// notify new citizen
			char buffer[32];
			snprintf(buffer, sizeof(buffer)-1, "%d", cityId);
			buffer[sizeof(buffer)-1] = '\0';

			MessageToQueue::getInstance().sendMessageToC(citizenId,
				"C++CityAddCitizen",
				buffer,
				0,
				false);
		}

		CitizenInfo updatedInfo;
		updatedInfo.m_citizenName = citizenName;
		updatedInfo.m_citizenProfessionSkillTemplate = (existingInfo ? existingInfo->m_citizenProfessionSkillTemplate : std::string());
		updatedInfo.m_citizenLevel = (existingInfo ? existingInfo->m_citizenLevel : 0);
		updatedInfo.m_citizenPermissions = permissions;
		updatedInfo.m_citizenRank = (existingInfo ? existingInfo->m_citizenRank : BitArray());
		updatedInfo.m_citizenTitle = (existingInfo ? existingInfo->m_citizenTitle : std::string());
		updatedInfo.m_citizenAllegiance = allegiance;

		m_citizensInfo.set(std::make_pair(cityId, citizenId), updatedInfo);
	}
}

// ----------------------------------------------------------------------

void CityObject::setCitizenProfessionInfo(int cityId, NetworkId const &citizenId, std::string const &citizenProfessionSkillTemplate, int citizenLevel)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetCitizenProfessionInfo,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > >(std::make_pair(std::make_pair(cityId, citizenId), std::make_pair(citizenProfessionSkillTemplate, citizenLevel))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		CitizenInfo const *info = getCitizenInfo(cityId, citizenId);
		if (info)
		{
			std::string oldCitizenSpec, newCitizenSpec;
			CityStringParser::buildCitizenSpec(cityId, citizenId, info->m_citizenName, info->m_citizenProfessionSkillTemplate, info->m_citizenLevel, info->m_citizenPermissions, info->m_citizenRank, info->m_citizenTitle, info->m_citizenAllegiance, oldCitizenSpec);
			CityStringParser::buildCitizenSpec(cityId, citizenId, info->m_citizenName, citizenProfessionSkillTemplate, citizenLevel, info->m_citizenPermissions, info->m_citizenRank, info->m_citizenTitle, info->m_citizenAllegiance, newCitizenSpec);
			if (oldCitizenSpec != newCitizenSpec)
				setCitizen(oldCitizenSpec, newCitizenSpec);

			CitizenInfo updatedInfo = *info;
			updatedInfo.m_citizenProfessionSkillTemplate = citizenProfessionSkillTemplate;
			updatedInfo.m_citizenLevel = citizenLevel;

			m_citizensInfo.set(std::make_pair(cityId, citizenId), updatedInfo);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::addCitizenRank(int cityId, NetworkId const &citizenId, std::string const &rankName)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetCitizenAddRank,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > >(std::make_pair(rankName, std::make_pair(citizenId, cityId))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		CitizenInfo const *info = getCitizenInfo(cityId, citizenId);
		if (info)
		{
			CitizenRankDataTable::CitizenRank const * rankData = CitizenRankDataTable::getRank(rankName);
			if (rankData && !info->m_citizenRank.testBit(rankData->slotId))
			{
				BitArray rank = info->m_citizenRank;
				rank.setBit(rankData->slotId);

				std::string oldCitizenSpec, newCitizenSpec;
				CityStringParser::buildCitizenSpec(cityId, citizenId, info->m_citizenName, info->m_citizenProfessionSkillTemplate, info->m_citizenLevel, info->m_citizenPermissions, info->m_citizenRank, info->m_citizenTitle, info->m_citizenAllegiance, oldCitizenSpec);
				CityStringParser::buildCitizenSpec(cityId, citizenId, info->m_citizenName, info->m_citizenProfessionSkillTemplate, info->m_citizenLevel, info->m_citizenPermissions, rank, info->m_citizenTitle, info->m_citizenAllegiance, newCitizenSpec);
				if (oldCitizenSpec != newCitizenSpec)
					setCitizen(oldCitizenSpec, newCitizenSpec);

				CitizenInfo updatedInfo = *info;
				updatedInfo.m_citizenRank = rank;

				m_citizensInfo.set(std::make_pair(cityId, citizenId), updatedInfo);
			}
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::removeCitizenRank(int cityId, NetworkId const &citizenId, std::string const &rankName)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetCitizenRemoveRank,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > >(std::make_pair(rankName, std::make_pair(citizenId, cityId))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		CitizenInfo const *info = getCitizenInfo(cityId, citizenId);
		if (info)
		{
			CitizenRankDataTable::CitizenRank const * rankData = CitizenRankDataTable::getRank(rankName);
			if (rankData && info->m_citizenRank.testBit(rankData->slotId))
			{
				BitArray rank = info->m_citizenRank;
				rank.clearBit(rankData->slotId);

				std::string oldCitizenSpec, newCitizenSpec;
				CityStringParser::buildCitizenSpec(cityId, citizenId, info->m_citizenName, info->m_citizenProfessionSkillTemplate, info->m_citizenLevel, info->m_citizenPermissions, info->m_citizenRank, info->m_citizenTitle, info->m_citizenAllegiance, oldCitizenSpec);
				CityStringParser::buildCitizenSpec(cityId, citizenId, info->m_citizenName, info->m_citizenProfessionSkillTemplate, info->m_citizenLevel, info->m_citizenPermissions, rank, info->m_citizenTitle, info->m_citizenAllegiance, newCitizenSpec);
				if (oldCitizenSpec != newCitizenSpec)
					setCitizen(oldCitizenSpec, newCitizenSpec);

				CitizenInfo updatedInfo = *info;
				updatedInfo.m_citizenRank = rank;

				m_citizensInfo.set(std::make_pair(cityId, citizenId), updatedInfo);
			}
		}
	}
}

// ----------------------------------------------------------------------

bool CityObject::hasCitizenRank(int cityId, NetworkId const &citizenId, std::string const &rankName)
{
	CitizenInfo const *info = getCitizenInfo(cityId, citizenId);
	if (info)
	{
		CitizenRankDataTable::CitizenRank const * rankData = CitizenRankDataTable::getRank(rankName);
		if (rankData && info->m_citizenRank.testBit(rankData->slotId))
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------

void CityObject::getCitizenRank(int cityId, NetworkId const &citizenId, std::vector<std::string> &ranks)
{
	ranks.clear();

	CitizenInfo const *info = getCitizenInfo(cityId, citizenId);
	if (info)
	{
		std::vector<CitizenRankDataTable::CitizenRank const *> const & allRanks = CitizenRankDataTable::getAllRanks();
		for (std::vector<CitizenRankDataTable::CitizenRank const *>::const_iterator iter = allRanks.begin(); iter != allRanks.end(); ++iter)
		{
			if (info->m_citizenRank.testBit((*iter)->slotId))
				ranks.push_back((*iter)->name);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCitizenTitle(int cityId, NetworkId const &citizenId, std::string const &citizenTitle)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetCitizenTitle,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > >(std::make_pair(citizenTitle, std::make_pair(citizenId, cityId))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		CitizenInfo const *info = getCitizenInfo(cityId, citizenId);
		if (info)
		{
			std::string oldCitizenSpec, newCitizenSpec;
			CityStringParser::buildCitizenSpec(cityId, citizenId, info->m_citizenName, info->m_citizenProfessionSkillTemplate, info->m_citizenLevel, info->m_citizenPermissions, info->m_citizenRank, info->m_citizenTitle, info->m_citizenAllegiance, oldCitizenSpec);
			CityStringParser::buildCitizenSpec(cityId, citizenId, info->m_citizenName, info->m_citizenProfessionSkillTemplate, info->m_citizenLevel, info->m_citizenPermissions, info->m_citizenRank, citizenTitle, info->m_citizenAllegiance, newCitizenSpec);
			if (oldCitizenSpec != newCitizenSpec)
				setCitizen(oldCitizenSpec, newCitizenSpec);

			CitizenInfo updatedInfo = *info;
			updatedInfo.m_citizenTitle = citizenTitle;

			m_citizensInfo.set(std::make_pair(cityId, citizenId), updatedInfo);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::setCityStructure(int cityId, NetworkId const &structureId, int structureType, bool structureValid)
{
	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_citySetStructure,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<int, bool> > >(std::make_pair(std::make_pair(cityId, structureId), std::make_pair(structureType, structureValid))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		std::string oldStructureSpec, newStructureSpec;
		CityStructureInfo const * existingInfo = getCityStructureSpec(cityId, structureId, oldStructureSpec);
		CityStringParser::buildCityStructureSpec(cityId, structureId, structureType, structureValid, newStructureSpec);
		if (oldStructureSpec != newStructureSpec)
		{
			if (!oldStructureSpec.empty())
				m_structures.erase(oldStructureSpec);
			m_structures.insert(newStructureSpec);
		}

		if (!existingInfo)
		{
			std::map<int, CityInfo>::const_iterator const iterCityInfo = m_citiesInfo.find(cityId);
			if (iterCityInfo != m_citiesInfo.end())
			{
				CityInfo updatedCi = iterCityInfo->second;
				updatedCi.setStructureCount(updatedCi.getStructureCount() + 1);
				m_citiesInfo.set(cityId, updatedCi);
			}
		}

		CityStructureInfo updatedInfo;
		updatedInfo.setStructureType(structureType);
		updatedInfo.setStructureValid(structureValid);

		m_structuresInfo.set(std::make_pair(cityId, structureId), updatedInfo);
	}
}

// ----------------------------------------------------------------------

void CityObject::getCitySpec(int cityId, std::string &result) const
{
	CityInfo const &ci = getCityInfo(cityId);
	CityStringParser::buildCitySpec(
		cityId, ci.getCityName(), ci.getCityHallId(),
		ci.getPlanet(), ci.getX(), ci.getZ(), ci.getRadius(),
		ci.getCreationTime(),
		ci.getLeaderId(),
		ci.getIncomeTax(), ci.getPropertyTax(), ci.getSalesTax(),
		ci.getTravelLoc(), ci.getTravelCost(), ci.getTravelInterplanetary(),
		ci.getCloneLoc(), ci.getCloneRespawn(), ci.getCloneRespawnCell(), ci.getCloneId(),
		result);
}

// ----------------------------------------------------------------------

CitizenInfo const * CityObject::getCitizenSpec(int cityId, NetworkId const &citizenId, std::string &result) const
{
	CitizenInfo const *info = getCitizenInfo(cityId, citizenId);
	if (info)
	{
		CityStringParser::buildCitizenSpec(cityId, citizenId, info->m_citizenName, info->m_citizenProfessionSkillTemplate, info->m_citizenLevel, info->m_citizenPermissions, info->m_citizenRank, info->m_citizenTitle, info->m_citizenAllegiance, result);
		return info;
	}

	result.clear();
	return nullptr;
}

// ----------------------------------------------------------------------

CityStructureInfo const * CityObject::getCityStructureSpec(int cityId, NetworkId const &structureId, std::string &result) const
{
	CityStructureInfo const *info = getCityStructureInfo(cityId, structureId);
	if (info)
	{
		CityStringParser::buildCityStructureSpec(cityId, structureId, info->getStructureType(), info->getStructureValid(), result);
		return info;
	}

	result.clear();
	return nullptr;
}

// ----------------------------------------------------------------------

void CityObject::unload()
{
	FATAL(true, ("Tried to unload city object.\n"));
}

// ----------------------------------------------------------------------

void CityObject::setCitySpec(int cityId, std::string const &newCitySpec)
{
	FATAL(!isAuthoritative(), ("CityObject::setCitySpec is only valid on the auth server"));
	std::string oldCitySpec;
	if (findCitySpec(cityId, oldCitySpec))
	{
		if (oldCitySpec == newCitySpec)
			return;
		m_cities.erase(oldCitySpec);
	}
	m_cities.insert(newCitySpec);
}

// ----------------------------------------------------------------------

bool CityObject::findCitySpec(int cityId, std::string &citySpec) const
{
	for (std::set<std::string>::const_iterator i = m_cities.begin(); i != m_cities.end(); ++i)
	{
		if ((*i).find("v2:") == 0)
		{
			if (atoi((*i).c_str() + 3) == cityId)
			{
				citySpec = *i;
				return true;
			}
		}
		else if (atoi((*i).c_str()) == cityId)
		{
			citySpec = *i;
			return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------

void CityObject::setCitizen(std::string const &oldCitizenSpec, std::string const &newCitizenSpec)
{
	FATAL(!isAuthoritative(), ("CityObject::setCitizen is only valid on the auth server"));

	if (!oldCitizenSpec.empty())
			m_citizens.erase(oldCitizenSpec);

	m_citizens.insert(newCitizenSpec);
}

// ----------------------------------------------------------------------

void CityObject::onCitizensInfoErase(std::pair<int, NetworkId> const & key, CitizenInfo const & value)
{
	UNREF(value);

	// citizen has been removed from a city, so need to update CreatureObject militia city id changes
	ServerObject * const citizen = ServerWorld::findObjectByNetworkId(key.second);
	if (citizen)
	{
		CreatureObject * const citizenCreatureObject = citizen->asCreatureObject();
		if (citizenCreatureObject && citizenCreatureObject->isAuthoritative() && citizenCreatureObject->isInitialized() && (citizenCreatureObject->getMilitiaOfCityId() == key.first))
		{
			citizenCreatureObject->setMilitiaOfCityId(0);
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::onCitizensInfoInsert(std::pair<int, NetworkId> const & key, CitizenInfo const & value)
{
	// citizen has been added to a city, so need to update CreatureObject militia city id changes
	ServerObject * const citizen = ServerWorld::findObjectByNetworkId(key.second);
	if (citizen)
	{
		CreatureObject * const citizenCreatureObject = citizen->asCreatureObject();
		if (citizenCreatureObject && citizenCreatureObject->isAuthoritative() && citizenCreatureObject->isInitialized())
		{
			int const currentMilitiaOfCityId = citizenCreatureObject->getMilitiaOfCityId();
			if (currentMilitiaOfCityId == 0)
			{
				// gaining militia
				if (value.m_citizenPermissions & CitizenPermissions::Militia)
					citizenCreatureObject->setMilitiaOfCityId(key.first);
			}
			else if (currentMilitiaOfCityId == key.first)
			{
				// losing militia
				if (!(value.m_citizenPermissions & CitizenPermissions::Militia))
					citizenCreatureObject->setMilitiaOfCityId(0);
			}
		}
	}
}

// ----------------------------------------------------------------------

void CityObject::onCitizensInfoSet(std::pair<int, NetworkId> const & key, CitizenInfo const & oldValue, CitizenInfo const & newValue)
{
	UNREF(oldValue);

	// citizen info has been changed, so need to update CreatureObject militia city id changes
	ServerObject * const citizen = ServerWorld::findObjectByNetworkId(key.second);
	if (citizen)
	{
		CreatureObject * const citizenCreatureObject = citizen->asCreatureObject();
		if (citizenCreatureObject && citizenCreatureObject->isAuthoritative() && citizenCreatureObject->isInitialized())
		{
			int const currentMilitiaOfCityId = citizenCreatureObject->getMilitiaOfCityId();
			if (currentMilitiaOfCityId == 0)
			{
				// gaining militia
				if (newValue.m_citizenPermissions & CitizenPermissions::Militia)
					citizenCreatureObject->setMilitiaOfCityId(key.first);
			}
			else if (currentMilitiaOfCityId == key.first)
			{
				// losing militia
				if (!(newValue.m_citizenPermissions & CitizenPermissions::Militia))
					citizenCreatureObject->setMilitiaOfCityId(0);
			}
		}
	}
}

// ----------------------------------------------------------------------

bool CityObject::cityExists(int cityId) const
{
	return (m_citiesInfo.find(cityId) != m_citiesInfo.end());
}

// ----------------------------------------------------------------------

CityInfo const &CityObject::getCityInfo(int cityId) const
{
	static const CityInfo s_nullCityInfo;
	std::map<int, CityInfo>::const_iterator i = m_citiesInfo.find(cityId);
	if (i != m_citiesInfo.end())
		return i->second;
	return s_nullCityInfo;
}

// ----------------------------------------------------------------------

void CityObject::getAllCityIds(std::vector<int> &results) const
{
	results.clear();

	for (std::map<int, CityInfo>::const_iterator i = m_citiesInfo.begin(); i != m_citiesInfo.end(); ++i)
		results.push_back(i->first);
}

// ----------------------------------------------------------------------

std::map<int, CityInfo> const &CityObject::getAllCityInfo() const
{
	return m_citiesInfo.getMap();
}

// ----------------------------------------------------------------------

int CityObject::findCityByName(std::string const &name) const
{
	char const *nameText = name.c_str();
	for (std::map<int, CityInfo>::const_iterator i = m_citiesInfo.begin(); i != m_citiesInfo.end(); ++i)
		if (!_stricmp(i->second.getCityName().c_str(), nameText))
			return i->first;
	return 0;
}

// ----------------------------------------------------------------------

int CityObject::findCityByCityHall(NetworkId const &cityHallId) const
{
	for (std::map<int, CityInfo>::const_iterator i = m_citiesInfo.begin(); i != m_citiesInfo.end(); ++i)
		if (i->second.getCityHallId() == cityHallId)
			return i->first;
	return 0;
}

// ----------------------------------------------------------------------

int CityObject::getCityAtLocation(std::string const &planetName, int x, int z, int radius) const
{
	for (std::map<int, CityInfo>::const_iterator i = m_citiesInfo.begin(); i != m_citiesInfo.end(); ++i)
	{
		CityInfo const &ci = i->second;
		if (   ci.getPlanet() == planetName
			&& sqr(x-ci.getX())+sqr(z-ci.getZ()) <= sqr(radius+ci.getRadius()))
			return i->first;
	}
	return 0;
}

// ----------------------------------------------------------------------

void CityObject::getCitizenIds(int cityId, std::vector<NetworkId> &results) const
{
	results.clear();

	for (std::map<std::pair<int, NetworkId>, CitizenInfo>::const_iterator iter = m_citizensInfo.lower_bound(std::make_pair(cityId, NetworkId::cms_invalid)); iter != m_citizensInfo.end(); ++iter)
	{
		if (iter->first.first != cityId)
			break;

		results.push_back(iter->first.second);
	}
}

// ----------------------------------------------------------------------

std::map<std::pair<int, NetworkId>, CitizenInfo> const &CityObject::getAllCitizensInfo() const
{
	return m_citizensInfo.getMap();
}

// ----------------------------------------------------------------------

CitizenInfo const *CityObject::getCitizenInfo(int cityId, NetworkId const &citizenId) const
{
	std::map<std::pair<int, NetworkId>, CitizenInfo>::const_iterator const iterFind = m_citizensInfo.find(std::make_pair(cityId, citizenId));
	if (iterFind != m_citizensInfo.end())
		return &(iterFind->second);

	return nullptr;
}

// ----------------------------------------------------------------------

void CityObject::getCityStructureIds(int cityId, std::vector<NetworkId> &results) const
{
	results.clear();

	for (std::map<std::pair<int, NetworkId>, CityStructureInfo>::const_iterator iter = m_structuresInfo.lower_bound(std::make_pair(cityId, NetworkId::cms_invalid)); iter != m_structuresInfo.end(); ++iter)
	{
		if (iter->first.first != cityId)
			break;

		results.push_back(iter->first.second);
	}
}

// ----------------------------------------------------------------------

std::map<std::pair<int, NetworkId>, CityStructureInfo> const & CityObject::getAllCityStructuresInfo() const
{
	return m_structuresInfo.getMap();
}

// ----------------------------------------------------------------------

CityStructureInfo const *CityObject::getCityStructureInfo(int cityId, NetworkId const &structureId) const
{
	std::map<std::pair<int, NetworkId>, CityStructureInfo>::const_iterator const iterFind = m_structuresInfo.find(std::make_pair(cityId, structureId));
	if (iterFind != m_structuresInfo.end())
		return &(iterFind->second);

	return nullptr;
}

// ----------------------------------------------------------------------

std::vector<int> const & CityObject::getCitizenOfCityId(NetworkId const &citizenId) const
{
	static std::vector<int> emptyList;

	std::map<NetworkId, std::vector<int> >::const_iterator const iterFind = m_citizenToCityId.find(citizenId);
	if (iterFind != m_citizenToCityId.end())
		return iterFind->second;

	return emptyList;
}

// ----------------------------------------------------------------------

void CityObject::checkForDualCitizenship() const
{
	if (!isAuthoritative())
		return;

	// check for dual citizenship
	std::string cities;
	char buffer[32];
	for (std::map<NetworkId, std::vector<int> >::const_iterator iterCitizens = m_citizenToCityId.begin(); iterCitizens != m_citizenToCityId.end(); ++iterCitizens)
	{
		if (iterCitizens->second.size() > 1)
		{
			cities.clear();
			for (std::vector<int>::const_iterator iterCities = iterCitizens->second.begin(); iterCities != iterCitizens->second.end(); ++iterCities)
			{
				CityInfo const & ci = getCityInfo(*iterCities);

				if (!cities.empty())
					cities += std::string("|");

				snprintf(buffer, sizeof(buffer)-1, "%d", *iterCities);
				buffer[sizeof(buffer)-1] = '\0';

				if (ci.getLeaderId() == iterCitizens->first)
					cities += "*";

				cities += buffer;
				cities += ":";
				cities += ci.getCityName();
			}

			LOG("CityFixup", ("(%s, %s) is a citizen of %d cities (%s).", iterCitizens->first.getValueString().c_str(), NameManager::getInstance().getPlayerFullName(iterCitizens->first).c_str(), iterCitizens->second.size(), cities.c_str()));
			LOG("CustomerService", ("DualCitizenship:(%s, %s) is a citizen of %d cities (%s).", iterCitizens->first.getValueString().c_str(), NameManager::getInstance().getPlayerFullName(iterCitizens->first).c_str(), iterCitizens->second.size(), cities.c_str()));
		}
	}
}

// ----------------------------------------------------------------------

PgcRatingInfo const * CityObject::getPgcRating(NetworkId const &chroniclerId) const
{
	std::map<NetworkId, PgcRatingInfo>::const_iterator const iterFind = m_pgcRatingInfo.find(chroniclerId);
	if (iterFind != m_pgcRatingInfo.end())
		return &(iterFind->second);

	return nullptr;
}

// ----------------------------------------------------------------------

void CityObject::getPgcChroniclerId(std::string const &chroniclerName, std::vector<NetworkId> &chroniclerIds) const
{
	chroniclerIds.clear();

	std::string const normalizedName = NameManager::normalizeName(chroniclerName);
	std::set<std::pair<std::string, NetworkId> > const & pgcRatingChroniclerId = m_pgcRatingChroniclerId.get();
	for (std::set<std::pair<std::string, NetworkId> >::const_iterator iter = pgcRatingChroniclerId.lower_bound(std::make_pair(normalizedName, NetworkId::cms_invalid)); iter != pgcRatingChroniclerId.end(); ++iter)
	{
		if (iter->first != normalizedName)
			break;

		chroniclerIds.push_back(iter->second);
	}
}

// ----------------------------------------------------------------------

void CityObject::adjustPgcRating(NetworkId const &chroniclerId, std::string const &chroniclerName, int adjustment)
{
	if (!chroniclerId.isValid() || (adjustment == 0))
		return;

	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_adjustPgcRating,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > >(std::make_pair(chroniclerName, std::make_pair(chroniclerId, adjustment))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		PgcRatingInfo updatedPgcRating;
		PgcRatingInfo const * const existingPgcRating = getPgcRating(chroniclerId);
		if (existingPgcRating)
		{
			updatedPgcRating = *existingPgcRating;
			++updatedPgcRating.m_ratingCount;
			updatedPgcRating.m_ratingTotal += static_cast<int64>(adjustment);

			std::string oldPgcRatingSpec;
			CityStringParser::buildPgcRatingSpec(chroniclerId, *existingPgcRating, oldPgcRatingSpec);
			m_citizens.erase(oldPgcRatingSpec);
		}
		else
		{
			updatedPgcRating.m_chroniclerName = NameManager::getInstance().getPlayerFullName(chroniclerId);
			if (updatedPgcRating.m_chroniclerName.empty())
				updatedPgcRating.m_chroniclerName = chroniclerName;

			if (updatedPgcRating.m_chroniclerName.empty())
				return;

			updatedPgcRating.m_ratingCount = 1;
			updatedPgcRating.m_ratingTotal = static_cast<int64>(adjustment);

			m_pgcRatingChroniclerId.insert(std::make_pair(NameManager::normalizeName(updatedPgcRating.m_chroniclerName), chroniclerId));
		}

		updatedPgcRating.m_lastRatingTime = static_cast<int>(::time(nullptr));

		std::string updatedPgcRatingSpec;
		CityStringParser::buildPgcRatingSpec(chroniclerId, updatedPgcRating, updatedPgcRatingSpec);

		m_citizens.insert(updatedPgcRatingSpec);
		m_pgcRatingInfo.set(chroniclerId, updatedPgcRating);
	}
}

// ----------------------------------------------------------------------

void CityObject::adjustPgcRatingData(NetworkId const &chroniclerId, std::string const &chroniclerName, int index, int adjustment)
{
	if (!chroniclerId.isValid() || (adjustment == 0) || (index < 0) || (static_cast<size_t>(index) >= PgcRatingInfo::ms_PgcRatingDataSize))
		return;

	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_adjustPgcRatingData,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > >(std::make_pair(std::make_pair(index, chroniclerId), std::make_pair(chroniclerName, adjustment))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		PgcRatingInfo updatedPgcRating;
		PgcRatingInfo const * const existingPgcRating = getPgcRating(chroniclerId);
		if (existingPgcRating)
		{
			if (static_cast<size_t>(index) >= existingPgcRating->m_data.size())
				return;

			updatedPgcRating = *existingPgcRating;
			updatedPgcRating.m_data[index] += adjustment;

			std::string oldPgcRatingSpec;
			CityStringParser::buildPgcRatingSpec(chroniclerId, *existingPgcRating, oldPgcRatingSpec);
			m_citizens.erase(oldPgcRatingSpec);
		}
		else
		{
			if (static_cast<size_t>(index) >= updatedPgcRating.m_data.size())
				return;

			updatedPgcRating.m_chroniclerName = NameManager::getInstance().getPlayerFullName(chroniclerId);
			if (updatedPgcRating.m_chroniclerName.empty())
				updatedPgcRating.m_chroniclerName = chroniclerName;

			if (updatedPgcRating.m_chroniclerName.empty())
				return;

			updatedPgcRating.m_data[index] = adjustment;

			m_pgcRatingChroniclerId.insert(std::make_pair(NameManager::normalizeName(updatedPgcRating.m_chroniclerName), chroniclerId));
		}

		std::string updatedPgcRatingSpec;
		CityStringParser::buildPgcRatingSpec(chroniclerId, updatedPgcRating, updatedPgcRatingSpec);

		m_citizens.insert(updatedPgcRatingSpec);
		m_pgcRatingInfo.set(chroniclerId, updatedPgcRating);
	}
}

// ----------------------------------------------------------------------

void CityObject::setPgcRatingData(NetworkId const &chroniclerId, std::string const &chroniclerName, int index, int value)
{
	if (!chroniclerId.isValid() || (index < 0) || (static_cast<size_t>(index) >= PgcRatingInfo::ms_PgcRatingDataSize))
		return;

	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_setPgcRatingData,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > >(std::make_pair(std::make_pair(index, chroniclerId), std::make_pair(chroniclerName, value))),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		PgcRatingInfo updatedPgcRating;
		PgcRatingInfo const * const existingPgcRating = getPgcRating(chroniclerId);
		if (existingPgcRating)
		{
			if (static_cast<size_t>(index) >= existingPgcRating->m_data.size())
				return;

			if (existingPgcRating->m_data[index] == value)
				return;

			updatedPgcRating = *existingPgcRating;
			updatedPgcRating.m_data[index] = value;

			std::string oldPgcRatingSpec;
			CityStringParser::buildPgcRatingSpec(chroniclerId, *existingPgcRating, oldPgcRatingSpec);
			m_citizens.erase(oldPgcRatingSpec);
		}
		else
		{
			if (static_cast<size_t>(index) >= updatedPgcRating.m_data.size())
				return;

			updatedPgcRating.m_chroniclerName = NameManager::getInstance().getPlayerFullName(chroniclerId);
			if (updatedPgcRating.m_chroniclerName.empty())
				updatedPgcRating.m_chroniclerName = chroniclerName;

			if (updatedPgcRating.m_chroniclerName.empty())
				return;

			updatedPgcRating.m_data[index] = value;

			m_pgcRatingChroniclerId.insert(std::make_pair(NameManager::normalizeName(updatedPgcRating.m_chroniclerName), chroniclerId));
		}

		std::string updatedPgcRatingSpec;
		CityStringParser::buildPgcRatingSpec(chroniclerId, updatedPgcRating, updatedPgcRatingSpec);

		m_citizens.insert(updatedPgcRatingSpec);
		m_pgcRatingInfo.set(chroniclerId, updatedPgcRating);
	}
}

// ----------------------------------------------------------------------

void CityObject::verifyPgcChroniclerName(NetworkId const &chroniclerId, std::string const &chroniclerName)
{
	PgcRatingInfo const * const existingPgcRating = getPgcRating(chroniclerId);
	if (!existingPgcRating || (existingPgcRating->m_chroniclerName == chroniclerName) || chroniclerName.empty())
		return;

	if (!isAuthoritative())
	{
		Controller *controller = getController();
		if (controller)
		{
			controller->appendMessage(
				CM_fixPgcChroniclerName,
				0.0f,
				new MessageQueueGenericValueType<std::pair<std::string, NetworkId> >(std::make_pair(chroniclerName, chroniclerId)),
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_SERVER);
		}
	}
	else
	{
		PgcRatingInfo updatedPgcRating = *existingPgcRating;
		updatedPgcRating.m_chroniclerName = chroniclerName;

		std::string oldPgcRatingSpec;
		CityStringParser::buildPgcRatingSpec(chroniclerId, *existingPgcRating, oldPgcRatingSpec);

		m_citizens.erase(oldPgcRatingSpec);
		m_pgcRatingChroniclerId.erase(std::make_pair(NameManager::normalizeName(existingPgcRating->m_chroniclerName), chroniclerId));

		std::string updatedPgcRatingSpec;
		CityStringParser::buildPgcRatingSpec(chroniclerId, updatedPgcRating, updatedPgcRatingSpec);

		m_citizens.insert(updatedPgcRatingSpec);
		m_pgcRatingInfo.set(chroniclerId, updatedPgcRating);
		m_pgcRatingChroniclerId.insert(std::make_pair(NameManager::normalizeName(updatedPgcRating.m_chroniclerName), chroniclerId));
	}
}

// ----------------------------------------------------------------------

void CityObject::addGcwRegionDefenderCity(std::string const & gcwRegion, int cityId, uint32 faction)
{
	if (!gcwRegion.empty())
	{
		m_gcwRegionDefenderCities.set(std::make_pair(gcwRegion, cityId), faction);

		Archive::AutoDeltaMap<std::string, std::pair<int, int> >::const_iterator iter = m_gcwRegionDefenderCitiesCount.find(gcwRegion);
		if (iter != m_gcwRegionDefenderCitiesCount.end())
		{
			if (PvpData::isImperialFactionId(faction))
				m_gcwRegionDefenderCitiesCount.set(gcwRegion, std::make_pair(iter->second.first + 1, iter->second.second));
			else if (PvpData::isRebelFactionId(faction))
				m_gcwRegionDefenderCitiesCount.set(gcwRegion, std::make_pair(iter->second.first, iter->second.second + 1));
		}
		else
		{
			if (PvpData::isImperialFactionId(faction))
				m_gcwRegionDefenderCitiesCount.set(gcwRegion, std::make_pair(1, 0));
			else if (PvpData::isRebelFactionId(faction))
				m_gcwRegionDefenderCitiesCount.set(gcwRegion, std::make_pair(0, 1));
		}

		m_gcwRegionDefenderCitiesVersion.set(m_gcwRegionDefenderCitiesVersion.get() + 1);

		GuildObject * const go = ServerUniverse::getInstance().getMasterGuildObject();
		if (go)
			go->updateGcwRegionDefenderBonus(gcwRegion);
	}
}

// ----------------------------------------------------------------------

void CityObject::removeGcwRegionDefenderCity(std::string const & gcwRegion, int cityId, uint32 faction)
{
	if (!gcwRegion.empty())
	{
		m_gcwRegionDefenderCities.erase(std::make_pair(gcwRegion, cityId));

		Archive::AutoDeltaMap<std::string, std::pair<int, int> >::const_iterator iter = m_gcwRegionDefenderCitiesCount.find(gcwRegion);
		if (iter != m_gcwRegionDefenderCitiesCount.end())
		{
			std::pair<int, int> value = iter->second;

			if (PvpData::isImperialFactionId(faction))
				value.first = std::max(0, value.first - 1);
			else if (PvpData::isRebelFactionId(faction))
				value.second = std::max(0, value.second - 1);

			if ((value.first > 0) || (value.second > 0))
				m_gcwRegionDefenderCitiesCount.set(gcwRegion, value);
			else
				m_gcwRegionDefenderCitiesCount.erase(gcwRegion);
		}

		m_gcwRegionDefenderCitiesVersion.set(m_gcwRegionDefenderCitiesVersion.get() + 1);

		GuildObject * const go = ServerUniverse::getInstance().getMasterGuildObject();
		if (go)
			go->updateGcwRegionDefenderBonus(gcwRegion);
	}
}

// ======================================================================
