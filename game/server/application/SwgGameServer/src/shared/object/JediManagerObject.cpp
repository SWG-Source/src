// ======================================================================
//
// JediManagerObject.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstSwgGameServer.h"
#include "SwgGameServer/JediManagerObject.h"

#include "UnicodeUtils.h"
#include "boost/smart_ptr.hpp"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/GameServer.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/PlayerObject.h"
#include "serverNetworkMessages/BountyHunterTargetListMessage.h"
#include "serverNetworkMessages/BountyHunterTargetMessage.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/NetworkIdManager.h"
#include "SwgGameServer/JediManagerController.h"
#include "SwgGameServer/ServerJediManagerObjectTemplate.h"
#include "SwgGameServer/SwgCreatureObject.h"
#include "SwgGameServer/SwgServerUniverse.h"
#include "swgServerNetworkMessages/MessageQueueJediData.h"
#include "swgServerNetworkMessages/MessageQueueJediLocation.h"
#include "swgServerNetworkMessages/MessageQueueRequestJediBounty.h"

#include <climits>

static const int IGNORE_JEDI_STAT = INT_MAX;


// ======================================================================

namespace JediManagerObjectNamespace
{
	// the bounty hunter target list loaded from the DB; we store it here
	// and wait until the JediManagerObject object is created, and then
	// read it into the JediManagerObject object
	const BountyHunterTargetListMessage * s_queuedBountyHunterTargetListFromDB = nullptr;
}

using namespace JediManagerObjectNamespace;

// ----------------------------------------------------------------------

/**
 * Class constructor.
 */
JediManagerObject::JediManagerObject(const ServerJediManagerObjectTemplate *newTemplate) :
	UniverseObject(newTemplate),
	m_jediId(),
	m_holes(),
	m_jediName(),
	m_jediLocation(),
	m_jediScene(),
	m_jediVisibility(),
	m_jediBountyValue(),
	m_jediLevel(),
	m_jediBounties(),
	m_jediHoursAlive(),
	m_jediState(),
	m_jediOnline(),
	m_jediSpentJediSkillPoints(),
	m_jediFaction(),
	m_jediScriptData(),
	m_jediScriptDataNames(),
	m_bountyHunterTargetsReceivedFromDB(false)
{
	addMembersToPackages();
}	// JediManagerObject::JediManagerObject

// ----------------------------------------------------------------------

/**
 * Class destructor.
 */
JediManagerObject::~JediManagerObject()
{
}	// JediManagerObject::~JediManagerObject

// ----------------------------------------------------------------------

/**
 * Sets up auto-sync data.
 */
void JediManagerObject::addMembersToPackages()
{
	addServerVariable_np (m_jediId);
	addServerVariable_np (m_holes);
	addServerVariable_np (m_jediName);
	addServerVariable_np (m_jediLocation);
	addServerVariable_np (m_jediScene);
	addServerVariable_np (m_jediVisibility);
	addServerVariable_np (m_jediBountyValue);
	addServerVariable_np (m_jediLevel);
	addServerVariable_np (m_jediBounties);
	addServerVariable_np (m_bountyHunters);
	addServerVariable_np (m_jediHoursAlive);
	addServerVariable_np (m_jediState);
	addServerVariable_np (m_jediOnline);
	addServerVariable_np (m_jediSpentJediSkillPoints);
	addServerVariable_np (m_jediFaction);
	addServerVariable_np (m_jediScriptData);
	addServerVariable_np (m_jediScriptDataNames);
	addServerVariable_np (m_bountyHunterTargetsReceivedFromDB);
}	// JediManagerObject::addMembersToPackages

// ----------------------------------------------------------------------

/**
 * Initializes data for a newly created object.
 */
void JediManagerObject::initializeFirstTimeObject()
{
	UniverseObject::initializeFirstTimeObject();
}	// JediManagerObject::initializeFirstTimeObject

// ----------------------------------------------------------------------

/**
 * Initializes an object just proxied.
 */
void JediManagerObject::endBaselines()
{
	UniverseObject::endBaselines();
}	// JediManagerObject::endBaselines

// ----------------------------------------------------------------------

/**
 * Final setup for universe objects. Registers this object with the universe.
 */
void JediManagerObject::setupUniverse()
{
	static_cast<SwgServerUniverse&>(SwgServerUniverse::getInstance()).registerJediManager(*this);
}	// JediManagerObject::setupUniverse

// ----------------------------------------------------------------------

void JediManagerObject::onServerUniverseGainedAuthority()
{
	if (s_queuedBountyHunterTargetListFromDB)
	{
		addJediBounties(*s_queuedBountyHunterTargetListFromDB);

		delete s_queuedBountyHunterTargetListFromDB;
		s_queuedBountyHunterTargetListFromDB = nullptr;
	}
}

// ----------------------------------------------------------------------

/**
 * Creates the controller for this object.
 */
Controller * JediManagerObject::createDefaultController()
{
	Controller * controller = new JediManagerController(this);

	setController(controller);
	return controller;
}	// JediManagerObject::createDefaultController

// ----------------------------------------------------------------------

void JediManagerObject::conclude()
{
	if (ConfigServerGame::getLogJediConcludes())
		REPORT_LOG(true, ("Jedi manager concluding\n"));
	UniverseObject::conclude();
}

// ----------------------------------------------------------------------

/**
 * Adds a Jedi to the list of Jedi online.
 *
 * @param id			id of the Jedi
 * @param name			name of the Jedi
 * @param location		the world location of the Jedi
 * @param scene			the scene the Jedi is in
 * @param visibility	how visible the Jedi is
 * @param bountyValue   the bountyValue on the Jedi
 * @param level         the level of the Jedi
 * @param hoursAlive	how long the Jedi has been alive
 * @param state			the Jedi state
 * @param spentJediSkillPoints number of skills points the Jedi has spent on Jedi skills
 * @param faction       the Jedi's pvp faction
 */
void JediManagerObject::addJedi(const NetworkId & id, const Unicode::String & name, 
	const Vector & location, const std::string & scene, int visibility, int bountyValue, 
	int level, int hoursAlive, int state, int spentJediSkillPoints, int faction)
{
	WARNING_DEBUG_FATAL(!isInitialized(), ("JediManagerObject::addJedi when not "
		"initialized"));

	if (isAuthoritative())
	{
		// make sure we aren't adding a duplicate Jedi
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index == -1)
		{
			if (bountyValue > 0)
			{
				LOG("CustomerService", ("Jedi: Adding %s to the Jedi manager with a bounty value of %d", 
					PlayerObject::getAccountDescription(id).c_str(), bountyValue));

				// add the Jedi
				if (m_holes.empty())
				{
					m_jediName.push_back(name);
					m_jediLocation.push_back(location);
					m_jediScene.push_back(scene);
					m_jediVisibility.push_back(visibility);
					m_jediBountyValue.push_back(bountyValue);
					m_jediLevel.push_back(level);
					m_jediBounties.push_back(std::vector<NetworkId>());
					m_jediHoursAlive.push_back(hoursAlive);
					m_jediState.push_back(state);
					m_jediOnline.push_back(1);
					m_jediSpentJediSkillPoints.push_back(spentJediSkillPoints);
					m_jediFaction.push_back(faction);

					m_jediId.set(id, m_jediName.size() - 1);
				}
				else
				{
					const int hole = m_holes.back();
					m_holes.pop_back();

					m_jediName.set(hole, name);
					m_jediLocation.set(hole, location);
					m_jediScene.set(hole, scene);
					m_jediVisibility.set(hole, visibility);
					m_jediBountyValue.set(hole, bountyValue);
					m_jediLevel.set(hole, level);
					m_jediBounties.set(hole, std::vector<NetworkId>());
					m_jediHoursAlive.set(hole, hoursAlive);
					m_jediState.set(hole, state);
					m_jediOnline.set(hole, 1);
					m_jediSpentJediSkillPoints.set(hole, spentJediSkillPoints);
					m_jediFaction.set(hole, faction);

					m_jediId.set(id, hole);
				}

				if (location.x == 0 && location.y == 0 && location.z == 0)
				{
					LOG("CustomerService", ("Jedi: %s has a location of 0 0 0", 
						PlayerObject::getAccountDescription(id).c_str()));
				}
			}
		}
		else
		{
			// if bountyValue is 0 and Jedi doesn't have any
			// more bounty hunters hunting him, remove him
			if ((bountyValue <= 0) && (m_jediBounties[index].empty()))
			{
				removeJedi(id);
			}
			else
			{
				if (bountyValue != m_jediBountyValue[index])
				{
					LOG("CustomerService", ("Jedi: Updating Jedi %s bounty value from %d to %d", 
						PlayerObject::getAccountDescription(id).c_str(), m_jediBountyValue[index], bountyValue));
				}

				m_jediName.set(index, name);
				m_jediLocation.set(index, location);
				m_jediScene.set(index, scene);
				m_jediVisibility.set(index, visibility);
				m_jediBountyValue.set(index, bountyValue);
				m_jediLevel.set(index,level);
				m_jediState.set(index, state);
				m_jediOnline.set(index, 1);
				m_jediSpentJediSkillPoints.set(index, spentJediSkillPoints);
				m_jediFaction.set(index, faction);

				if (location.x == 0 && location.y == 0 && location.z == 0)
				{
					LOG("CustomerService", ("Jedi: %s has a location of 0 0 0", 
						PlayerObject::getAccountDescription(id).c_str()));
				}
			}
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_addJedi,
			new MessageQueueJediData(id, name, location, scene, visibility, bountyValue, 
			level, hoursAlive, state, spentJediSkillPoints, faction));
	}
	
	// in theory this should happen automatically via the auto delta code or when
	// we send the controller message, but on live this appears not to be happening
	if (ConfigServerGame::getForceJediConcludes())
		addObjectToConcludeList();
}	// JediManagerObject::addJedi

// ----------------------------------------------------------------------

/**
 * Tags a Jedi as being offline.
 *
 * @param id			id of the Jedi
 * @param location		final world location of the Jedi
 * @param scene			final scene of the Jedi
 */
void JediManagerObject::setJediOffline(const NetworkId & id, const Vector & location, 
	const std::string & scene)
{
	WARNING_DEBUG_FATAL(!isInitialized(), ("JediManagerObject::setJediOffline "
		"when not initialized"));

	if (isAuthoritative())
	{
		// make sure we aren't adding a duplicate Jedi
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index >= 0)
		{
			LOG("CustomerService", ("Jedi: Flagging Jedi %s as offline",
				PlayerObject::getAccountDescription(id).c_str()));
			m_jediLocation.set(index, location);
			m_jediScene.set(index, scene);
			m_jediOnline.set(index, 0);

			if (location.x == 0 && location.y == 0 && location.z == 0)
			{
				LOG("CustomerService", ("Jedi: %s has a location of 0 0 0", 
					PlayerObject::getAccountDescription(id).c_str()));
			}
		}
		else
		{
			LOG("CustomerService", ("Jedi: setJediOffline called "
				" for unknown Jedi %s", PlayerObject::getAccountDescription(id).c_str()));
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setJediOffline,
			new MessageQueueJediLocation(id, location, scene));
	}

	// in theory this should happen automatically via the auto delta code or when
	// we send the controller message, but on live this appears not to be happening
	if (ConfigServerGame::getForceJediConcludes())
		addObjectToConcludeList();
}	// JediManagerObject::setJediOffline

// ----------------------------------------------------------------------

/**
 * Removes a Jedi from the list of Jedi online.
 *
 * @param id			id of the Jedi
 */
void JediManagerObject::removeJedi(const NetworkId & id)
{
	WARNING_DEBUG_FATAL(!isInitialized(), ("JediManagerObject::removeJedi "
		"when not initialized"));

	if (isAuthoritative())
	{
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index >= 0)
		{
			LOG("CustomerService", ("Jedi: Removing %s from the Jedi manager", 
				PlayerObject::getAccountDescription(id).c_str()));

			{
			const std::vector<NetworkId> & bounties = m_jediBounties[index];
			for (std::vector<NetworkId>::const_iterator i = bounties.begin(); i != bounties.end(); ++i)
			{
				adjustBountyCount(*i, -1);

				// tell the db about the change
				GameServer::getInstance().sendToDatabaseServer(BountyHunterTargetMessage(*i, NetworkId::cms_invalid));

				// tell the hunter to update their pvp status
				MessageToQueue::getInstance().sendMessageToC(*i, "C++UpdatePvp", "", 0, false);
			}
			}

			// mark the location in the vectors as removed
			IGNORE_RETURN(m_jediId.erase(id));
			m_holes.push_back(index);

			// find and remove all the Jedi script data
			{
			char buffer[1024];
			for (Archive::AutoDeltaSet<std::string>::const_iterator i = m_jediScriptDataNames.begin();
				i != m_jediScriptDataNames.end(); ++i)
			{
				snprintf(buffer, sizeof(buffer), "%d|%s", index, (*i).c_str());
				m_jediScriptData.erase(buffer);
			}
			}
		}
		else
		{
			LOG("CustomerService", ("Jedi: removeJedi asked to "
				"remove unknown Jedi %s", PlayerObject::getAccountDescription(id).c_str()));
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeJedi, 
			new MessageQueueGenericValueType<NetworkId>(id));
	}

	// in theory this should happen automatically via the auto delta code or when
	// we send the controller message, but on live this appears not to be happening
	if (ConfigServerGame::getForceJediConcludes())
		addObjectToConcludeList();
}	// JediManagerObject::removeJedi

// ----------------------------------------------------------------------

/**
 * A character is being deleted
 *
 * @param id			id of the character
 */
void JediManagerObject::characterBeingDeleted(const NetworkId & id)
{
	WARNING_DEBUG_FATAL(!isInitialized(), ("JediManagerObject::jediBeingDeleted "
		"when not initialized"));

	// stop tracking the character
	Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
	const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
	if (index >= 0)
	{
		// tell all bounty hunters currently hunting the jedi to cancel their bounty mission
		ScriptParams params;
		params.addParam(id, "target");

		ScriptDictionaryPtr dictionary;
		GameScriptObject::makeScriptDictionary(params, dictionary);
		if (dictionary.get() != nullptr)
		{
			dictionary->serialize();

			const std::vector<NetworkId> & bounties = m_jediBounties[index];
			for (std::vector<NetworkId>::const_iterator i = bounties.begin(); i != bounties.end(); ++i)
			{
				MessageToQueue::getInstance().sendMessageToJava(*i, 
					"handleBountyMissionIncomplete", dictionary->getSerializedData(), 0, true);
			}
		}
		else
		{
			WARNING(true, ("JediManagerObject::characterBeingDeleted error converting "
				"params"));
		}

		// remove the jedi
		removeJedi(id);
	}

	// remove all bounties that the deleted character is hunting
	std::vector<NetworkId> jedis;
	getBountyHunterBounties(id, jedis);

	for (std::vector<NetworkId>::const_iterator iter = jedis.begin(); iter != jedis.end(); ++iter)
	{
		removeJediBounty(*iter, id);
	}
}	// JediManagerObject::characterBeingDeleted

// ----------------------------------------------------------------------

/**
 * Updates the data for a Jedi in the list of Jedi online. If a data param 
 * is < 0, it will be ignored.
 *
 * @param id			id of the Jedi
 * @param visibility	how visible the Jedi is
 * @param bountyValue   the bounty value of the Jedi
 * @param level         the level of the Jedi
 * @param hoursAlive	how long the Jedi has been alive
 */
void JediManagerObject::updateJedi(const NetworkId & id, int visibility,
	int bountyValue, int level, int hoursAlive)
{
	WARNING_DEBUG_FATAL(!isInitialized(), ("JediManagerObject::updateJedi(1) "
		"when not initialized"));

	if (isAuthoritative())
	{
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index >= 0)
		{
			if (visibility >= 0 && visibility != m_jediVisibility.get(index))
			{
				LOG("CustomerService", ("Jedi: Setting Jedi %s visibility to %d",
					PlayerObject::getAccountDescription(id).c_str(), visibility));
				m_jediVisibility.set(index, visibility);
			}
			if (bountyValue >= 0 && bountyValue != m_jediBountyValue.get(index))
			{
				LOG("CustomerService", ("Jedi: Setting Jedi %s bounty value to %d",
					PlayerObject::getAccountDescription(id).c_str(), bountyValue));
				m_jediBountyValue.set(index, bountyValue);
			}
			if (level >= 0 && level != m_jediLevel.get(index))
			{
				LOG("CustomerService", ("Jedi: Setting Jedi %s level to %d",
					PlayerObject::getAccountDescription(id).c_str(), level));
				m_jediLevel.set(index, level);
			}
			if (hoursAlive >= 0 && hoursAlive != m_jediHoursAlive.get(index))
			{
				LOG("CustomerService", ("Jedi: Setting Jedi %s hoursAlive to %d",
					PlayerObject::getAccountDescription(id).c_str(), hoursAlive));
				m_jediHoursAlive.set(index, hoursAlive);
			}
		}
		else
		{
			LOG("CustomerService", ("Jedi: updateJedi(1) called for "
				"unknown Jedi %s", PlayerObject::getAccountDescription(id).c_str()));
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_updateJedi, new MessageQueueJediData(
			id, Unicode::emptyString, Vector(), "", visibility, bountyValue, level, hoursAlive,
			0, 0, 0));
	}

	// in theory this should happen automatically via the auto delta code or when
	// we send the controller message, but on live this appears not to be happening
	if (ConfigServerGame::getForceJediConcludes())
		addObjectToConcludeList();
}	// JediManagerObject::updateJedi(int, int)

// ----------------------------------------------------------------------

/**
 * Updates the data for a Jedi in the list of Jedi online.
 *
 * @param id			id of the Jedi
 * @param state			the new state for the Jedi
 */
void JediManagerObject::updateJedi(const NetworkId & id, JediState state)
{
	WARNING_DEBUG_FATAL(!isInitialized(), ("JediManagerObject::updateJedi(2) "
		"when not initialized"));

	if (isAuthoritative())
	{
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index >= 0)
		{
			if (state != m_jediState.get(index))
			{
				LOG("CustomerService", ("Jedi: Setting Jedi %s state to %d",
					PlayerObject::getAccountDescription(id).c_str(), static_cast<int>(state)));
				m_jediState.set(index, state);
			}
		}
		else
		{
			LOG("CustomerService", ("Jedi: updateJedi(2) called for "
				"unknown Jedi %s", PlayerObject::getAccountDescription(id).c_str()));
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_updateJediState, 
			new MessageQueueGenericValueType<std::pair<NetworkId, int> >(
			std::make_pair(id, static_cast<int>(state))));
	}

	// in theory this should happen automatically via the auto delta code or when
	// we send the controller message, but on live this appears not to be happening
	if (ConfigServerGame::getForceJediConcludes())
		addObjectToConcludeList();
}	// JediManagerObject::updateJedi(JediState)

// ----------------------------------------------------------------------

/**
 * Updates the bounty data for a Jedi in the list of Jedi online.
 *
 * @param id			id of the Jedi
 * @param bounties		list of the bounty hunters tracking the Jedi
 *
void JediManagerObject::updateJedi(const NetworkId & id, 
	const std::vector<NetworkId> & bounties) const
{
	WARNING_DEBUG_FATAL(!isInitialized(), ("JediManagerObject::updateJedi(3) "
		"when not initialized"));

	if (isAuthoritative())
	{
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index >= 0)
		{
			LOG("CustomerService", ("Jedi: Updating bounties on Jedi %s",
				PlayerObject::getAccountDescription(id).c_str()));
			m_jediBounties.set(index, bounties);
		}
		else
		{
			LOG("CustomerService", ("Jedi: updateJedi(3) called for "
				"unknown Jedi %s", PlayerObject::getAccountDescription(id).c_str()));
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_updateJediBounties, 
			new MessageQueueGenericValueType<std::pair<NetworkId, std::vector<NetworkId> > >(
			std::make_pair(id, bounties)));
	}

	// in theory this should happen automatically via the auto delta code or when
	// we send the controller message, but on live this appears not to be happening
	if (ConfigServerGame::getForceJediConcludes())
		addObjectToConcludeList();
}	// JediManagerObject::updateJedi(const std::vector<NetworkId> &)
*/
// ----------------------------------------------------------------------

/**
 * Updates the location for a Jedi.
 * 
 * @param id			id of the Jedi
 * @param location		the Jedi location
 * @param scene			the Jedi scene
 */
void JediManagerObject::updateJediLocation(const NetworkId & id, const Vector & location, 
	const std::string & scene)
{
	WARNING_DEBUG_FATAL(!isInitialized(), ("JediManagerObject::updateJediLocation "
		"when not initialized"));

	if (isAuthoritative())
	{
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index >= 0)
		{
			if (scene != m_jediScene.get(index))
			{
				LOG("CustomerService", ("Jedi: Updating scene for Jedi %s to %s",
					PlayerObject::getAccountDescription(id).c_str(), scene.c_str()));
				m_jediScene.set(index, scene);
			}
			m_jediLocation.set(index, location);
			if (location.x == 0 && location.y == 0 && location.z == 0)
			{
				LOG("CustomerService", ("Jedi: %s has a location of 0 0 0", 
					PlayerObject::getAccountDescription(id).c_str()));
			}
			if (!m_jediOnline.get(index))
			{
				LOG("CustomerService", ("Jedi: Jedi %s was offline but we received a position update, so we are marking them online.",PlayerObject::getAccountDescription(id).c_str(), scene.c_str()));
				m_jediOnline.set(index, 1);
			}
		}
		else
		{
			LOG("CustomerService", ("Jedi: updateJediLocation called "
				"for unknown Jedi %s", PlayerObject::getAccountDescription(id).c_str()));
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_updateJediLocation, 
			new MessageQueueJediLocation(id, location, scene));
	}

	// in theory this should happen automatically via the auto delta code or when
	// we send the controller message, but on live this appears not to be happening
	if (ConfigServerGame::getForceJediConcludes())
		addObjectToConcludeList();
}	// JediManagerObject::updateJediLocation

// ----------------------------------------------------------------------

/**
 * Updates the number of skill points spent on Jedi skills for a Jedi.
 * 
 * @param id						id of the Jedi
 * @param spentJediSkillPoints		the number of skill points the Jedi has spent on Jedi skills
 */
void JediManagerObject::updateJediSpentJediSkillPoints(const NetworkId & id, int spentJediSkillPoints)
{
	WARNING_DEBUG_FATAL(!isInitialized(), ("JediManagerObject::updateJediSpentJediSkillPoints "
		"when not initialized"));

	if (isAuthoritative())
	{
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index >= 0)
		{
			if (spentJediSkillPoints != m_jediSpentJediSkillPoints.get(index))
			{
				LOG("CustomerService", ("Jedi: Updating spent Jedi skill points for Jedi %s from %d to %d",
					PlayerObject::getAccountDescription(id).c_str(), m_jediSpentJediSkillPoints.get(index), spentJediSkillPoints));
				m_jediSpentJediSkillPoints.set(index, spentJediSkillPoints);
			}
		}
		else
		{
			LOG("CustomerService", ("Jedi: updateJediSpentJediSkillPoints called "
				"for unknown Jedi %s", PlayerObject::getAccountDescription(id).c_str()));
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_updateJediSpentJediSkillPoints, 
			new MessageQueueGenericValueType<std::pair<NetworkId, int> >(
			std::make_pair(id, spentJediSkillPoints)));
	}

	// in theory this should happen automatically via the auto delta code or when
	// we send the controller message, but on live this appears not to be happening
	if (ConfigServerGame::getForceJediConcludes())
		addObjectToConcludeList();
}	// JediManagerObject::updateJediSpentJediSkillPoints

// ----------------------------------------------------------------------

/**
 * Updates the faction of a Jedi.
 * 
 * @param id		id of the Jedi
 * @param faction	the Jedi's faction
 */
void JediManagerObject::updateJediFaction(const NetworkId & id, int faction)
{
	WARNING_DEBUG_FATAL(!isInitialized(), ("JediManagerObject::updateJediFaction "
		"when not initialized"));

	if (isAuthoritative())
	{
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index >= 0)
		{
			if (faction != m_jediFaction.get(index))
			{
				LOG("CustomerService", ("Jedi: Updating Jedi faction for Jedi %s from %d to %d",
					PlayerObject::getAccountDescription(id).c_str(), m_jediFaction.get(index), faction));
				m_jediFaction.set(index, faction);
			}
		}
		else
		{
			LOG("CustomerService", ("Jedi: updateJediFaction called for unknown "
				"Jedi %s", PlayerObject::getAccountDescription(id).c_str()));
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_updateJediFaction, 
			new MessageQueueGenericValueType<std::pair<NetworkId, int> >(
			std::make_pair(id, faction)));
	}

	// in theory this should happen automatically via the auto delta code or when
	// we send the controller message, but on live this appears not to be happening
	if (ConfigServerGame::getForceJediConcludes())
		addObjectToConcludeList();
}	// JediManagerObject::updateJediFaction

// ----------------------------------------------------------------------

/**
 * Adds an arbitrary name->int pair to the info for a Jedi.
 *
 * @param id		the Jedi's id
 * @param name		the data name
 * @param value		the data value
 */
void JediManagerObject::updateJediScriptData(const NetworkId & id, const std::string & name, int value)
{
	WARNING_DEBUG_FATAL(!isInitialized(), ("JediManagerObject::updateJediScriptData "
		"when not initialized"));

	if (isAuthoritative())
	{
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index >= 0)
		{
			LOG("CustomerService", ("Jedi: Updating script data on Jedi %s. Data "
				"name = %s, value = %d", PlayerObject::getAccountDescription(id).c_str(),
				name.c_str(), value));
			char buffer[1024];
			if (snprintf(buffer, sizeof(buffer), "%d|%s", index, name.c_str()) > 0)
			{
				m_jediScriptData.set(buffer, value);
				m_jediScriptDataNames.insert(name);
			}
		}
		else
		{
			LOG("CustomerService", ("Jedi: updateJediScriptData called for "
				"unknown Jedi %s", PlayerObject::getAccountDescription(id).c_str()));
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_updateJediScriptData, 
			new MessageQueueGenericValueType<std::pair<NetworkId, std::pair<std::string, int> > >(
			std::make_pair(id, std::make_pair(name, value))));
	}

	// in theory this should happen automatically via the auto delta code or when
	// we send the controller message, but on live this appears not to be happening
	if (ConfigServerGame::getForceJediConcludes())
		addObjectToConcludeList();
}	// JediManagerObject::updateJediScriptData

// ----------------------------------------------------------------------

/**
 * Removes an arbitrary name->int pair to the info for a Jedi.
 *
 * @param id		the Jedi's id
 * @param name		the data name
 */
void JediManagerObject::removeJediScriptData(const NetworkId & id, const std::string & name)
{
	WARNING_DEBUG_FATAL(!isInitialized(), ("JediManagerObject::removeJediScriptData "
		"when not initialized"));

	if (isAuthoritative())
	{
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index >= 0)
		{
			LOG("CustomerService", ("Jedi: Removing script data on Jedi %s. Data "
				"name = %s", PlayerObject::getAccountDescription(id).c_str(),
				name.c_str()));
			char buffer[1024];
			snprintf(buffer, sizeof(buffer), "%d|%s", index, name.c_str());
			m_jediScriptData.erase(buffer);
		}
		else
		{
			LOG("CustomerService", ("Jedi: removeJediScriptData called for "
				"unknown Jedi %s", PlayerObject::getAccountDescription(id).c_str()));
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeJediScriptData, 
			new MessageQueueGenericValueType<std::pair<NetworkId, std::string> >(
			std::make_pair(id, name)));
	}

	// in theory this should happen automatically via the auto delta code or when
	// we send the controller message, but on live this appears not to be happening
	if (ConfigServerGame::getForceJediConcludes())
		addObjectToConcludeList();
}	// JediManagerObject::removeJediScriptData

// ----------------------------------------------------------------------

/**
 * Called by a script requesting Jedi with certain parameters. Limits are passed 
 * in for statistics that the caller is interested in; the Jedi returned will 
 * have the requested value or greater for the statistic if the value given is 
 * >= 0, or less than the statistic if the value given is < 0. If the statistic 
 * should be ignored, IGNORE_JEDI_STAT should be passed in as the limit value.
 * The exception to this is the minLevel and maxLevel parameters, which always
 * specify the (inclusive) minLevel and maxLevel filter
 *
 * @param visibility	how visible the Jedi is
 * @param bountyValue   bounty value of the Jedi
 * @param minLevel      minimum Jedi level (inclusive)
 * @param maxLevel      maximum Jedi level (inclusive)
 * @param bounties		how many bounties the Jedi has on him
 * @param hoursAlive	how long the Jedi has been alive
 * @param state         which Jedi states we're interested in
 * @param returnParams	params that will be filled in with the Jedi data
 */
void JediManagerObject::getJedi(int visibility, int bountyValue, int minLevel, int maxLevel, 
	int hoursAlive, int bounties, int state, ScriptParams & returnParams) const
{
	// unfortunately we have to create new data to store the Jedi info,
	// since the ScriptParams class wasn't set up to be passed as a return
	// value like we're using it.

	// the returnParams will be responsible for destructing these
	std::vector<NetworkId>                      * jediId = new std::vector<NetworkId>;
	std::vector<const Unicode::String *>        * jediName = new std::vector<const Unicode::String *>;
	std::vector<const Vector *>                 * jediLocation = new std::vector<const Vector *>;
	std::vector<const char *>                   * jediScene = new std::vector<const char *>;
	std::vector<int>                            * jediVisibility = new std::vector<int>;
	std::vector<int>                            * jediBountyValue = new std::vector<int>;
	std::vector<int>                            * jediLevel = new std::vector<int>;
	std::vector<const std::vector<NetworkId> *> * jediBounties = new std::vector<const std::vector<NetworkId> *>;
	std::vector<int>                            * jediHoursAlive = new std::vector<int>;
	std::vector<int>                            * jediState = new std::vector<int>;
	std::deque<bool>                            * jediOnline = new std::deque<bool>;
	std::vector<int>                            * jediSpentJediSkillPoints = new std::vector<int>;
	std::vector<int>                            * jediFaction = new std::vector<int>;
	std::map<std::string, std::vector<int> *>     jediScriptData;

	// initialize the jedi script data map
	{
	for (Archive::AutoDeltaSet<std::string>::const_iterator iter = m_jediScriptDataNames.begin();
		iter != m_jediScriptDataNames.end(); ++iter)
	{
		jediScriptData.insert(std::make_pair(*iter, new std::vector<int>));
	}
	}

	// check each Jedi to see if they meet the requirements
	for (Archive::AutoDeltaMap<NetworkId, int>::const_iterator i = m_jediId.begin(); i != m_jediId.end(); ++i)
	{
		if (m_jediName[i->second].empty())
			continue;

		// filter out Jedi who don't meet the requirements
		if (visibility != IGNORE_JEDI_STAT)
		{
			if (visibility >= 0 && m_jediVisibility[i->second] < visibility)
				continue;
			else if (visibility < 0 && m_jediVisibility[i->second] >= -visibility)
				continue;
		}
		if (bountyValue != IGNORE_JEDI_STAT)
		{
			if (bountyValue >= 0 && m_jediBountyValue[i->second] < bountyValue)
				continue;
			else if (bountyValue < 0 && m_jediBountyValue[i->second] >= -bountyValue)
				continue;
		}
		if ((m_jediLevel[i->second] <= 0) || (m_jediLevel[i->second] < minLevel) || (m_jediLevel[i->second] > maxLevel))
			continue;
		if (hoursAlive != IGNORE_JEDI_STAT)
		{
			if (hoursAlive >= 0 && m_jediHoursAlive[i->second] < hoursAlive)
				continue;
			else if (hoursAlive < 0 && m_jediHoursAlive[i->second] >= -hoursAlive)
				continue;
		}
		if (bounties != IGNORE_JEDI_STAT)
		{
			if (bounties >= 0 && m_jediBounties[i->second].size() < static_cast<size_t>(bounties))
				continue;
			else if (bounties < 0 && m_jediBounties[i->second].size() >= static_cast<size_t>(-bounties))
				continue;
		}
		if (state != IGNORE_JEDI_STAT)
		{
			if ((state & m_jediState[i->second]) == 0)
				continue;
		}
		// this Jedi is ok
		jediId->push_back(i->first);
		jediName->push_back(new Unicode::String(m_jediName[i->second]));
		jediLocation->push_back(new Vector(m_jediLocation[i->second]));
		char * scene = new char[m_jediScene[i->second].size() + 1];
		strcpy(scene, m_jediScene[i->second].c_str());
		jediScene->push_back(scene);
		jediVisibility->push_back(m_jediVisibility[i->second]);
		jediBountyValue->push_back(m_jediBountyValue[i->second]);
		jediLevel->push_back(m_jediLevel[i->second]);
		jediBounties->push_back(new std::vector<NetworkId>(m_jediBounties[i->second]));
		jediHoursAlive->push_back(m_jediHoursAlive[i->second]);
		jediState->push_back(m_jediState[i->second]);
		jediOnline->push_back(static_cast<bool>(m_jediOnline[i->second]));
		jediSpentJediSkillPoints->push_back(m_jediSpentJediSkillPoints[i->second]);
		jediFaction->push_back(m_jediFaction[i->second]);

		// find the script data for the Jedi
		char buffer[1024];
		for (Archive::AutoDeltaSet<std::string>::const_iterator j = m_jediScriptDataNames.begin();
			j != m_jediScriptDataNames.end(); ++j)
		{
			std::vector<int> * dataList = (*jediScriptData.find(*j)).second;
			NOT_NULL(dataList);
			snprintf(buffer, sizeof(buffer), "%d|%s", i->second, (*j).c_str());
			Archive::AutoDeltaMap<std::string, int>::const_iterator result = 
				m_jediScriptData.find(buffer);
			if (result != m_jediScriptData.end())
			{
				dataList->push_back((*result).second);
			}
			else
			{
				dataList->push_back(0);
			}
		}
	}
	
	returnParams.addParam(*jediId, "id", true);
	returnParams.addParam(*jediName, "name", true);
	returnParams.addParam(*jediLocation, "location", true);
	returnParams.addParam(*jediScene, "scene", true);
	returnParams.addParam(*jediVisibility, "visibility", true);
	returnParams.addParam(*jediBountyValue, "bountyValue", true);
	returnParams.addParam(*jediLevel, "level", true);
	returnParams.addParam(*jediBounties, "bounties", true);
	returnParams.addParam(*jediHoursAlive, "hoursAlive", true);
	returnParams.addParam(*jediState, "state", true);
	returnParams.addParam(*jediOnline, "online", true);
	returnParams.addParam(*jediSpentJediSkillPoints, "spentJediSkillPoints", true);
	returnParams.addParam(*jediFaction, "faction", true);

	{
	for (std::map<std::string, std::vector<int> *>::const_iterator iter = jediScriptData.begin();
		iter != jediScriptData.end(); ++iter)
	{
		returnParams.addParam(*((*iter).second), (*iter).first, true);
	}
	}
}	// JediManagerObject::getJedi

// ----------------------------------------------------------------------

/**
 * Called by a script requesting a specific Jedi.
 *
 * @param id			the id of the Jedi
 * @param returnParams	params that will be filled in with the Jedi data
 */
void JediManagerObject::getJedi(const NetworkId & id, ScriptParams & returnParams) const
{
	Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
	const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
	if (index < 0)
		return;

	returnParams.addParam(id, "id");
	returnParams.addParam(m_jediName[index], "name");
	returnParams.addParam(m_jediLocation[index], "location");
	returnParams.addParam(m_jediScene[index].c_str(), "scene");
	returnParams.addParam(m_jediVisibility[index], "visibility");
	returnParams.addParam(m_jediBountyValue[index], "bountyValue");
	returnParams.addParam(m_jediLevel[index], "level");
	returnParams.addParam(m_jediBounties[index], "bounties");
	returnParams.addParam(m_jediHoursAlive[index], "hoursAlive");
	returnParams.addParam(m_jediState[index], "state");
	returnParams.addParam(static_cast<bool>(m_jediOnline[index]), "online");
	returnParams.addParam(m_jediSpentJediSkillPoints[index], "spentJediSkillPoints");
	returnParams.addParam(m_jediFaction[index], "faction");
	

	// add the script data
	char buffer[1024];
	for (Archive::AutoDeltaSet<std::string>::const_iterator i = m_jediScriptDataNames.begin();
		i != m_jediScriptDataNames.end(); ++i)
	{
		snprintf(buffer, sizeof(buffer), "%d|%s", index, (*i).c_str());
		Archive::AutoDeltaMap<std::string, int>::const_iterator result = 
			m_jediScriptData.find(buffer);
		if (result != m_jediScriptData.end())
		{
			returnParams.addParam((*result).second, *i);
		}
		else
		{
			returnParams.addParam(0, *i);
		}
	}
}	// JediManagerObject::getJedi

// ----------------------------------------------------------------------

/**
 * Returns the location of a Jedi.
 *
 * @param id			id of the Jedi
 * @param location		filled with the Jedi location
 * @param scene			filled with the Jedi scene
 *
 * @return true if the location/scene data is valid, false if we couldn't find 
 * the Jedi
 */
bool JediManagerObject::getJediLocation(const NetworkId & id, Vector & location, 
	std::string & scene) const
{
	Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(id);
	const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
	if (index < 0)
		return false;

	location = m_jediLocation[index];
	scene = m_jediScene[index];
	return true;
}	// JediManagerObject::getJediLocation

// ----------------------------------------------------------------------

/**
 * Sets up the Jedi bounties from the database.
 *
 * @param msg		the message from the database listing all the Jedi and who 
 *					is hunting them
 */
void JediManagerObject::addJediBounties(const BountyHunterTargetListMessage & msg)
{
	if (!isAuthoritative())
		return;

	if (m_bountyHunterTargetsReceivedFromDB.get())
	{
		WARNING(true, ("JediManagerObject::addJediBounties() - attempting to apply BountyHunterTargetList from DB a second time"));
		return;
	}

	typedef std::vector< std::pair< NetworkId, NetworkId > > BountyList;
	const BountyList & bountyList = msg.getTargetList();
	for (BountyList::const_iterator i = bountyList.begin(); i != bountyList.end(); ++i)
	{
		const NetworkId & jediId = (*i).second;
		const NetworkId & hunterId = (*i).first;

		LOG("BountyHunterTargetList",("adding bounty from DB bh=%s jedi=%s", hunterId.getValueString().c_str(), jediId.getValueString().c_str()));

		if (jediId == NetworkId::cms_invalid || hunterId == NetworkId::cms_invalid)
			continue;
		
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(jediId);
		int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index < 0)
		{
			// we need to add the Jedi
			if (m_holes.empty())
			{
				m_jediName.push_back(Unicode::String());
				m_jediLocation.push_back(Vector());
				m_jediScene.push_back("");
				m_jediVisibility.push_back(0);
				m_jediBountyValue.push_back(0);
				m_jediLevel.push_back(0);
				m_jediBounties.push_back(std::vector<NetworkId>());
				m_jediHoursAlive.push_back(0);
				m_jediState.push_back(0);
				m_jediOnline.push_back(0);
				m_jediSpentJediSkillPoints.push_back(0);
				m_jediFaction.push_back(0);

				index = m_jediName.size() - 1;
			}
			else
			{
				index = m_holes.back();
				m_holes.pop_back();

				m_jediName.set(index, Unicode::String());
				m_jediLocation.set(index, Vector());
				m_jediScene.set(index, "");
				m_jediVisibility.set(index, 0);
				m_jediBountyValue.set(index, 0);
				m_jediLevel.set(index, 0);
				m_jediBounties.set(index, std::vector<NetworkId>());
				m_jediHoursAlive.set(index, 0);
				m_jediState.set(index, 0);
				m_jediOnline.set(index, 0);
				m_jediSpentJediSkillPoints.set(index, 0);
				m_jediFaction.set(index, 0);
			}

			m_jediId.set(jediId, index);
		}

		if (std::find(m_jediBounties[index].begin(), m_jediBounties[index].end(), hunterId) == m_jediBounties[index].end())
		{
			std::vector<NetworkId> hunters = m_jediBounties[index];
			hunters.push_back(hunterId);
			m_jediBounties.set(index, hunters);

			adjustBountyCount(hunterId, 1);
		}
	}

	m_bountyHunterTargetsReceivedFromDB = true;
}	// JediManagerObject::addJediBounties

// ----------------------------------------------------------------------

/**
 * Requests to assign a bounty hunter to track down a Jedi.
 *
 * @param target			the Jedi to hunt
 * @param hunter			the bounty hunter
 * @param successCallback	name of a messageHandler than will be called if the
 *							bounty was assigned
 * @param failCallback		name of a messageHandler than will be called if the
 *							bounty was not assigned
 * @param callbackObj		the object that will receive the callback
 */
void JediManagerObject::requestJediBounty(const NetworkId & targetId, 
	const NetworkId & hunterId, const std::string & successCallback, 
	const std::string & failCallback, const NetworkId & callbackObjectId)
{
	if (isAuthoritative())
	{
		bool success = false;
		int bounties = 0;

		// find the target and see how many bounties they have on them
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(targetId);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index >= 0)
		{
			bounties = m_jediBounties[index].size();
			if (bounties < ConfigServerGame::getMaxJediBounties())
			{
				success = true;
				// only add the hunter to the list if he isn't on there already
				if (std::find(m_jediBounties[index].begin(), m_jediBounties[index].end(), hunterId) == m_jediBounties[index].end())
				{
					std::vector<NetworkId> hunters = m_jediBounties[index];
					hunters.push_back(hunterId);
					m_jediBounties.set(index, hunters);
					++bounties;

					adjustBountyCount(hunterId, 1);
					
					// tell the db about the change
					GameServer::getInstance().sendToDatabaseServer(BountyHunterTargetMessage(hunterId, targetId));
				}
			}
		}

		if (callbackObjectId != NetworkId::cms_invalid)
		{
			// send the success or fail message
			ScriptParams params;
			params.addParam(targetId, "jedi");
			params.addParam(hunterId, "hunter");
			params.addParam(bounties, "bounties");

			ScriptDictionaryPtr dictionary;
			GameScriptObject::makeScriptDictionary(params, dictionary);
			if (dictionary.get() != nullptr)
			{
				dictionary->serialize();
				if (success)
				{
					MessageToQueue::getInstance().sendMessageToJava(callbackObjectId, 
						successCallback, dictionary->getSerializedData(), 0, true);
				}
				else
				{
					MessageToQueue::getInstance().sendMessageToJava(callbackObjectId, 
						failCallback, dictionary->getSerializedData(), 0, true);
				}
			}
			else
			{
				WARNING(true, ("JediManagerObject::requestJediBounty error converting "
					"params"));
			}
		}

		// tell the hunter to update their pvp status; this must be done after the
		// callback notification message above to give script a chance to finish
		// initializing the bounty mission so that we have all the information
		// required to calculate pvp status
		if (success)
			MessageToQueue::getInstance().sendMessageToC(hunterId, "C++UpdatePvp", "", 10, false);
	}
	else
	{
		sendControllerMessageToAuthServer(CM_requestJediBounty, 
			new MessageQueueRequestJediBounty(targetId, hunterId, successCallback, 
			failCallback, callbackObjectId));
	}
}	// JediManagerObject::requestJediBounty

// ----------------------------------------------------------------------

/**
 * Removes a bounty from a Jedi.
 *
 * @param targetId		the Jedi
 * @param hunterId      the id of the bounty hunter to remove
 */
void JediManagerObject::removeJediBounty(const NetworkId & targetId, 
	const NetworkId & hunterId)
{
	if (isAuthoritative())
	{
		// find the target and see how many bounties they have on them
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(targetId);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index >= 0)
		{
			std::vector<NetworkId> hunters = m_jediBounties[index];
			std::vector<NetworkId>::iterator i = std::find(hunters.begin(), hunters.end(), hunterId);
			if (i != hunters.end())
			{
				hunters.erase(i);

				// must always do this even if we end up calling removeJedi() below,
				// because removeJedi() also processes the hunters list, so we need
				// to remove the hunter from the hunters list or else removeJedi()
				// will remove the hunter again
				m_jediBounties.set(index, hunters);
				adjustBountyCount(hunterId, -1);

				// if bountyValue is 0 and Jedi doesn't have any
				// more bounty hunters hunting him, remove him
				if ((m_jediBountyValue[index] <= 0) && (hunters.empty()))
				{
					removeJedi(targetId);
				}

				// tell the db about the change
				GameServer::getInstance().sendToDatabaseServer(BountyHunterTargetMessage(hunterId, NetworkId::cms_invalid));

				// tell the hunter to update their pvp status
				MessageToQueue::getInstance().sendMessageToC(hunterId, "C++UpdatePvp", "", 0, false);
			}
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeJediBounty, 
			new MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> >(
			std::make_pair(targetId, hunterId)));
	}
}	// JediManagerObject::removeJediBounty

// ----------------------------------------------------------------------

/**
 * Removes all the bounties on a Jedi.
 *
 * @param targetId		the Jedi
 */
void JediManagerObject::removeAllJediBounties(const NetworkId & targetId)
{
	if (isAuthoritative())
	{
		// find the target and see how many bounties they have on them
		Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(targetId);
		const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
		if (index >= 0)
		{
			std::vector<NetworkId> hunters = m_jediBounties[index];
			for (std::vector<NetworkId>::const_iterator i = hunters.begin(); i != hunters.end(); ++i)
			{
				adjustBountyCount(*i,-1);
				
				// tell the db about the change
				GameServer::getInstance().sendToDatabaseServer(BountyHunterTargetMessage(*i, NetworkId::cms_invalid));

				// tell the hunter to update their pvp status
				MessageToQueue::getInstance().sendMessageToC(*i, "C++UpdatePvp", "", 0, false);
			}

			// must always do this even if we end up calling removeJedi() below,
			// because removeJedi() also processes the hunters list, so we need
			// to remove all the hunter from the hunters list or else removeJedi()
			// will remove the hunters again
			hunters.clear();
			m_jediBounties.set(index, hunters);

			// if bountyValue is 0 and Jedi doesn't have any
			// more bounty hunters hunting him, remove him
			if (m_jediBountyValue[index] <= 0)
			{
				removeJedi(targetId);
			}
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_removeAllJediBounties, 
			new MessageQueueGenericValueType<NetworkId>(targetId));
	}
}	// JediManagerObject::removeJediBounty

// ----------------------------------------------------------------------

/**
 * Tests if a Jedi is being hunted by a given bounty hunter.
 *
 * @param targetId		the id of the Jedi
 * @param hunterId		the id of the bounty hunter
 *
 * @return true if the Jedi is being hunted, false if not
 */
bool JediManagerObject::hasBountyOnJedi(const NetworkId & targetId, 
	const NetworkId & hunterId) const
{
	Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(targetId);
	const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
	if (index >= 0)
	{
		const std::vector<NetworkId> & hunters = m_jediBounties[index];
		return std::find(hunters.begin(), hunters.end(), hunterId) != hunters.end();
	}
	return false;
}	// JediManagerObject::hasBountyOnJedi

// ----------------------------------------------------------------------

/**
 * Tests if a given bounty hunter is hunting any Jedis
 *
 * @param hunterId		the id of the bounty hunter
 *
 * @return true if the bounty hunter is hunting someone, false if not
 */
bool JediManagerObject::hasBountyOnJedi(NetworkId const & hunterId) const
{
	Archive::AutoDeltaMap<NetworkId, int>::const_iterator i=m_bountyHunters.find(hunterId);
	if (i==m_bountyHunters.end() || i->second<=0)
		return false;
	else
		return true;
}

// ----------------------------------------------------------------------

void JediManagerObject::queueBountyHunterTargetListFromDB(const BountyHunterTargetListMessage * bhtlm)
{
	WARNING(s_queuedBountyHunterTargetListFromDB && bhtlm, ("JediManagerObject::queueBountyHunterTargetListFromDB() - attempting to queue another BountyHunterTargetList from DB"));

	delete s_queuedBountyHunterTargetListFromDB;
	s_queuedBountyHunterTargetListFromDB = bhtlm;
}

// ----------------------------------------------------------------------

const std::vector<NetworkId> & JediManagerObject::getJediBounties(const NetworkId & targetId) const
{
	const static std::vector<NetworkId> EMPTY_ARRAY;

	Archive::AutoDeltaMap<NetworkId, int>::const_iterator const indexIter = m_jediId.find(targetId);
	const int index = (indexIter == m_jediId.end() ? -1 : indexIter->second);
	if (index >= 0)
	{
		return m_jediBounties[index];
	}
	return EMPTY_ARRAY;
}	// JediManagerObject::getJediBounties

// ----------------------------------------------------------------------

void JediManagerObject::getBountyHunterBounties(const NetworkId & hunterId, std::vector<NetworkId> & jedis) const
{
	jedis.clear();

	for (Archive::AutoDeltaMap<NetworkId, int>::const_iterator iter = m_jediId.begin(); iter != m_jediId.end(); ++iter)
	{
		if (std::find(m_jediBounties[iter->second].begin(), m_jediBounties[iter->second].end(), hunterId) != m_jediBounties[iter->second].end())
			jedis.push_back(iter->first);
	}
}

// ----------------------------------------------------------------------

void JediManagerObject::adjustBountyCount(NetworkId const & hunterId, int const adjustment)
{
	Archive::AutoDeltaMap<NetworkId, int>::const_iterator i = m_bountyHunters.find(hunterId);			
	int const oldBountyCount = (i!=m_bountyHunters.end()) ? i->second : 0;
	int const newBountyCount = oldBountyCount + adjustment;
	WARNING_DEBUG_FATAL(newBountyCount < 0,("Programmer bug: the bounty count for bounty hunter %s when below 0.", hunterId.getValueString().c_str()));

	if (newBountyCount <= 0)
		m_bountyHunters.erase(hunterId);
	else
		m_bountyHunters.set(hunterId, newBountyCount);
}

// ======================================================================
