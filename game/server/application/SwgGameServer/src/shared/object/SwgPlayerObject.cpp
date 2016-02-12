//========================================================================
//
// SwgCreatureObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "FirstSwgGameServer.h"
#include "SwgGameServer/SwgCreatureObject.h"
#include "SwgGameServer/SwgPlayerObject.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerObject.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "SwgGameServer/JediManagerObject.h"
#include "SwgGameServer/SwgServerUniverse.h"
#include "serverGame/PlayerCreatureController.h"


//----------------------------------------------------------------------

// jedi/force power objvars
const static std::string OBJVAR_JEDI("jedi");
const static std::string OBJVAR_JEDI_STATE("jedi.state");
const static std::string OBJVAR_FORCE_REGEN_RATE("jedi.regenrate");
const static std::string OBJVAR_FORCE_REGEN_VALUE("jedi.regenvalue");
const static std::string OBJVAR_JEDI_VISIBILITY("jedi.visibility");
const static std::string OBJVAR_JEDI_BOUNTIES("jedi.objKillers");


//----------------------------------------------------------------------

/**
 * Class constructor.
 */
SwgPlayerObject::SwgPlayerObject(const ServerPlayerObjectTemplate* newTemplate) :
	PlayerObject(newTemplate),
	m_updateJediLocationTime(0),
	m_jediState(0)
{
	addMembersToPackages();
}	// SwgCreatureObject::SwgCreatureObject

//----------------------------------------------------------------------

/**
 * Class destructor.
 */
SwgPlayerObject::~SwgPlayerObject()
{
}	// SwgPlayerObject::~SwgPlayerObject

//----------------------------------------------------------------------

/**
 * Sets up auto-sync data.
 */
void SwgPlayerObject::addMembersToPackages()
{
	//@todo: do we want to make a game equivalent of package.txt and Packager.cpp?
	addFirstParentAuthClientServerVariable_np (m_jediState);
}	// SwgPlayerObject::addMembersToPackages

//----------------------------------------------------------------------

/**
 * Initializes data for a proxy object.
 */
void SwgPlayerObject::endBaselines()
{
	PlayerObject::endBaselines();

	if (isAuthoritative())
	{
		// get the jediState from the objvar
		int jediState = 0;
		if (getObjVars().getItem(OBJVAR_JEDI_STATE, jediState))
			m_jediState = static_cast<JediState>(jediState);
	}
}	// SwgPlayerObject::endBaselines

//----------------------------------------------------------------------

/**
 * Called when we are made authoritative.
 */
void SwgPlayerObject::virtualOnSetAuthority()
{
	PlayerObject::virtualOnSetAuthority();

	const SwgCreatureObject * owner = safe_cast<const SwgCreatureObject *>(getCreatureObject());
	if ((owner != nullptr) && owner->isPlayerControlled() && (owner->getBountyValue() > 0))
	{
		// tell the Jedi manager our position
		JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
			ServerUniverse::getInstance()).getJediManager();
		if (jediManager != nullptr)
		{
			jediManager->updateJediLocation(owner->getNetworkId(), 
				owner->getPosition_w(), owner->getSceneId());
		}
		m_updateJediLocationTime = 0;
	}
}	// SwgPlayerObject::virtualOnSetAuthority

//----------------------------------------------------------------------

/**
 * Sets the Jedi state for the player.
 *
 * @param state		the new state
 */
void SwgPlayerObject::setJediState(JediState state)
{
	if (state == JS_none || 
		state == JS_forceSensitive ||
		state == JS_jedi ||
		state == JS_forceRankedLight ||
		state == JS_forceRankedDark
		)
	{
		if (isAuthoritative())
		{
			// get our current state for comparison
			JediState oldState = getJediState();
			bool oldIsJedi = isJedi();
			
			m_jediState = state;
			if (state == JS_none)
			{
				if (oldState != JS_none)
				{
					// we are no longer force sensitive
					removeObjVarItem(OBJVAR_JEDI);
					setMaxForcePower(0);
				}
			}
			else
			{
				setObjVarItem(OBJVAR_JEDI_STATE, state);
				if (oldState == JS_none)
				{
					// we are newly force sensitive
					// note that our max force power needs to be set separately
					WARNING(getForcePowerRegenRate() > 0, ("SwgPlayerObject::setJediState, "
						"resetting force power regen for player %s that has a non-zero "
						"regen value %f", getAccountDescription(getCreatureObject()).c_str(), 
						getForcePowerRegenRate()));
					setForcePowerRegenRate(ConfigServerGame::getMinForcePowerRegenRate());
				}
			}

			// check if our Jedi status has turned on or off
			if (!oldIsJedi && isJedi())
			{
				setJediVisibility(0);
			}
		}
		else
		{
			sendControllerMessageToAuthServer(CM_setJediState, 
				new MessageQueueGenericValueType<int>(static_cast<int>(state)));
		}
	}
	else
	{
		WARNING(true, ("setJediState for player %s invalid state %d", 
			getAccountDescription(getCreatureObject()).c_str(), state));
	}
}

//----------------------------------------------------------------------

/**
 * Updates the time from when we sent the Jedi manager our position.
 *
 * @param time		time since the last call
 */
void SwgPlayerObject::updateJediLocationTime(float time)
{
	if (isAuthoritative())
	{
		m_updateJediLocationTime += time;
		if (m_updateJediLocationTime > static_cast<float>(
			ConfigServerGame::getJediUpdateLocationTimeSeconds()))
		{
			m_updateJediLocationTime = 0;

			// tell the Jedi manager our position
			JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
				ServerUniverse::getInstance()).getJediManager();
			if (jediManager != nullptr)
			{
				const CreatureObject * owner = getCreatureObject();
				if (owner->isInWorld())
				{
					jediManager->updateJediLocation(owner->getNetworkId(), 
						owner->getPosition_w(), owner->getSceneId());
				}
			}
		}
	}
}	// SwgPlayerObject::updateJediLocationTime

//----------------------------------------------------------------------

/**
 * Sets the visibility of a Jedi.
 *
 * @param visibility		the visibility value
 */
void SwgPlayerObject::setJediVisibility(int visibility)
{
	if (!isJedi())
		return;

	if (visibility < 0)
		visibility = 0;

	setObjVarItem(OBJVAR_JEDI_VISIBILITY, visibility);
}	// SwgPlayerObject::setJediVisibility

//----------------------------------------------------------------------

/**
 * Returns the visibility of a Jedi.
 *
 * @return the visibility of the Jedi
 */
int SwgPlayerObject::getJediVisibility(void) const
{
	if (!isJedi())
		return 0;

	int visibility;
	if (!getObjVars().getItem(OBJVAR_JEDI_VISIBILITY,visibility))
	{
		WARNING(true, ("Jedi %s doesn't have a visibility objvar, adding one", 
			getNetworkId().getValueString().c_str()));
		const_cast<SwgPlayerObject *>(this)->setJediVisibility(0);
		return 0;
	}
	return visibility;
}	// SwgPlayerObject::getJediVisibility

//----------------------------------------------------------------------

/**
 * Adds a bounty hunter to the list of players hunting this Jedi.
 *
 * @param hunter		the bounty hunter's id
 */
/*
void SwgPlayerObject::addJediBounty(const NetworkId & hunter)
{
	if (!isJedi())
		return;

	std::vector<NetworkId> bounties;
	if (getObjVars().getItem(OBJVAR_JEDI_BOUNTIES, bounties))
	{
		// make sure we don't have the hunter already on our list
		if (std::find(bounties.begin(), bounties.end(), hunter) == bounties.end())
		{
			bounties.push_back(hunter);
			setObjVarItem(OBJVAR_JEDI_BOUNTIES, bounties);
		}
	}

	// note that since this function should only be called due to the Jedi
	// manager being updated, so we don't forward this to the manager like
	// we do with setJediBounties
}	// SwgPlayerObject::addJediBounty
*/

//----------------------------------------------------------------------

/**
 * Removes a bounty hunter from the list of players hunting this Jedi.
 *
 * @param hunter		the bounty hunter's id
 */
/*
void SwgPlayerObject::removeJediBounty(const NetworkId & hunter)
{
	if (!isJedi())
		return;

	std::vector<NetworkId> bounties;
	if (getObjVars().getItem(OBJVAR_JEDI_BOUNTIES, bounties))
	{
		std::vector<NetworkId>::iterator i = std::find(bounties.begin(), 
			bounties.end(), hunter);
		if (i != bounties.end())
		{
			bounties.erase(i);
			setObjVarItem(OBJVAR_JEDI_BOUNTIES, bounties);
		}
	}

	// note that since this function should only be called due to the Jedi
	// manager being updated, so we don't forward this to the manager like
	// we do with setJediBounties
}	// SwgPlayerObject::removeJediBounty
*/

//----------------------------------------------------------------------

/**
 * Sets the bounties on a Jedi.
 *
 * @param bounties		the number of bounties
 *
void SwgPlayerObject::setJediBounties(const std::vector<NetworkId> & bounties)
{
	if (!isJedi())
		return;

	setObjVarItem(OBJVAR_JEDI_BOUNTIES, bounties);

	// update the Jedi manager
	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
		ServerUniverse::getInstance()).getJediManager();
	if (jediManager != nullptr)
	{
		const CreatureObject * owner = getCreatureObject();
		jediManager->updateJedi(owner->getNetworkId(), bounties);
	}
}	// SwgPlayerObject::setJediBounties
*/
//----------------------------------------------------------------------

/**
 * Returns the bounties on a Jedi once for old characters. Once this function is
 * called for a player, assume the Jedi manager is authoritative for who
 * has bounties on this Jedi.
 *
 * @param bounties		vector to be filled in with the ids of the players 
 *						hunting the Jedi
 *
 * @return true if bounties has good data, false if not
 */
/*
bool SwgPlayerObject::getJediBounties(std::vector<NetworkId> & bounties)
{
	if (!isJedi())
		return false;

	bounties.clear();
	SwgCreatureObject * owner = safe_cast<SwgCreatureObject *>(getCreatureObject());
	if (owner != nullptr)
	{
		if (owner->getObjVars().getItem(OBJVAR_JEDI_BOUNTIES, bounties))
		{
			// remove the objvar, the Jedi manager is authoritative for this data
			owner->removeObjVarItem(OBJVAR_JEDI_BOUNTIES);
		}
	}
	return true;
}	// SwgPlayerObject::getJediBounties
*/
/**
 * Calculates whether a player is a jedi
 *
 * @return true if the player is a jedi
 */
bool SwgPlayerObject::isJedi() const
{
	return (getJediState() & (JS_jedi | JS_forceRankedLight | JS_forceRankedDark)) != 0;
}

