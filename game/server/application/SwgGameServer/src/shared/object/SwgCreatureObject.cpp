//========================================================================
//
// SwgCreatureObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "FirstSwgGameServer.h"
#include "SwgGameServer/SwgCreatureObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include "SwgGameServer/JediManagerObject.h"
#include "SwgGameServer/SwgPlayerCreatureController.h"
#include "SwgGameServer/SwgPlayerObject.h"
#include "SwgGameServer/SwgServerUniverse.h"


//----------------------------------------------------------------------

/**
 * Class constructor.
 */
SwgCreatureObject::SwgCreatureObject(const ServerCreatureObjectTemplate* newTemplate) :
	CreatureObject(newTemplate)
{
}	// SwgCreatureObject::SwgCreatureObject

//----------------------------------------------------------------------

/**
 * Class destructor.
 */
SwgCreatureObject::~SwgCreatureObject()
{
	//-- This must be the first line in the destructor to invalidate any watchers watching this object
	nullWatchers();

}	// SwgCreatureObject::~SwgCreatureObject

//----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
Controller* SwgCreatureObject::createDefaultController ()
{
	const ServerCreatureObjectTemplate * myTemplate = safe_cast<
		const ServerCreatureObjectTemplate *>(getObjectTemplate());

	Controller * controller = 0;
	if (myTemplate->getCanCreateAvatar())
	{
		controller = new SwgPlayerCreatureController(this);
	}
	else
	{
		controller = CreatureObject::createDefaultController();
	}

	setController(controller);
	return controller;
}	// CreatureObject::createDefaultController

//----------------------------------------------------------------------

/**
 * Called to alter the creature.
 */
float SwgCreatureObject::alter(float time)
{
	if (isAuthoritative() && isPlayerControlled() && (getBountyValue() > 0))
	{
		PlayerObject * const player = PlayerCreatureController::getPlayerObject(this);
		if (player)
		{
			SwgPlayerObject * swgPlayer = safe_cast<SwgPlayerObject *>(player);			
			swgPlayer->updateJediLocationTime(time);
		}
	}
	return CreatureObject::alter(time);
}	// SwgCreatureObject::alter

//----------------------------------------------------------------------

/**
 * Called when the creature is being removed from the world.
 */
void SwgCreatureObject::onRemovingFromWorld()
{
	// if we are a Jedi, flag ourself as offline
	if (isAuthoritative() && isPlayerControlled() && (getBountyValue() > 0))
	{
		JediManagerObject * const jediManager = static_cast<SwgServerUniverse &>(ServerUniverse::getInstance()).getJediManager();
		if (jediManager != nullptr)
		{
			jediManager->setJediOffline(getNetworkId(), getPosition_w(), getSceneId());
		}
	}

	CreatureObject::onRemovingFromWorld();
}	// SwgCreatureObject::onRemovingFromWorld

//----------------------------------------------------------------------

/**
 * Called when the creature is being deleted from the game.
 */
void SwgCreatureObject::onPermanentlyDestroyed()
{
	// if we are a Jedi, remove ourself from the Jedi manager
	if (isAuthoritative() && isPlayerControlled() && (getBountyValue() > 0))
	{
		JediManagerObject * const jediManager = static_cast<SwgServerUniverse &>(ServerUniverse::getInstance()).getJediManager();
		if (jediManager != nullptr)
		{
			jediManager->removeJedi(getNetworkId());
		}
	}

	CreatureObject::onPermanentlyDestroyed();
}	// SwgCreatureObject::onPermanentlyDestroyed

//----------------------------------------------------------------------

/**
 * Handles messages to this object.
 *
 * @param message		the message
 */
void SwgCreatureObject::handleCMessageTo (const MessageToPayload &message)
{
	CreatureObject::handleCMessageTo (message);
}	// SwgCreatureObject::handleCMessageTo

//----------------------------------------------------------------------

/**
* Checks if a player is a Jedi or not.
*/
bool SwgCreatureObject::isJedi(void) const
{
	if (!isPlayerControlled())
		return false;
	
	PlayerObject const * const player = PlayerCreatureController::getPlayerObject(this);
	if (player)
	{
		SwgPlayerObject const * const swgPlayer = safe_cast<SwgPlayerObject const *>(player);
		return swgPlayer->isJedi();
	}

	return false;
}	// SwgCreatureObject::isJedi

//-----------------------------------------------------------------------

/**
* Returns the number of skill points that have been spent on Jedi skills.
*/
const int SwgCreatureObject::getSpentJediSkillPoints() const
{
	int result = 0;

	/* skill points no longer exist
	const SkillList & skills = getSkillList();
	for (SkillList::const_iterator i = skills.begin(); i != skills.end(); ++i)
	{
		if (*i) 
		{
			// per Dave White, we only care about skills starting with "force_discipline"
			if ((*i)->getSkillName().find("force_discipline") == 0)
				result += (*i)->getSkillPointCost();
		}
		else
		{
			WARNING(true, ("Creature %s had a nullptr in their skill list", getNetworkId().getValueString().c_str()));
		}
	}
	*/
	return result;
}	// SwgCreatureObject::getSpentJediSkillPoints

//----------------------------------------------------------------------

/**
 * Tests if this creature has a bounty on another creature.
 *
 * @param target		the creature to test if we have a bounty on
 *
 * @return true if we have a bounty on the target, false if not
 */
bool SwgCreatureObject::hasBounty(const CreatureObject & target) const
{
	const SwgCreatureObject * swgTarget = dynamic_cast<const SwgCreatureObject *>(
		&target);
	if (swgTarget == nullptr)
		return false;

	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
		ServerUniverse::getInstance()).getJediManager();
	NOT_NULL(jediManager);

	return jediManager->hasBountyOnJedi(target.getNetworkId(), getNetworkId());
}	// SwgCreatureObject::hasBounty

// ----------------------------------------------------------------------

/**
 * Tests if this creature has a bounty on any other creature.
 *
 * @return true if we have a bounty, false if not
 */
bool SwgCreatureObject::hasBounty() const
{
	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
		ServerUniverse::getInstance()).getJediManager();
	NOT_NULL(jediManager);

	return jediManager->hasBountyOnJedi(getNetworkId());
}

// ----------------------------------------------------------------------

std::vector<NetworkId> const & SwgCreatureObject::getJediBountiesOnMe() const
{
	JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(
		ServerUniverse::getInstance()).getJediManager();
	NOT_NULL(jediManager);

	return jediManager->getJediBounties(getNetworkId());
}

// ----------------------------------------------------------------------

int SwgCreatureObject::getBountyValue() const
{
	int bountyValue;
	if (getObjVars().getItem("bounty.amount", bountyValue))
		return bountyValue;

	return 0;
}

//------------------------------------------------------------------------------------------

const bool SwgCreatureObject::grantSkill(const SkillObject & newSkill)
{
	return CreatureObject::grantSkill(newSkill);
}	// SwgCreatureObject::grantSkill

//-----------------------------------------------------------------------

void SwgCreatureObject::revokeSkill(const SkillObject & oldSkill)
{
	CreatureObject::revokeSkill(oldSkill);
}	// SwgCreatureObject::revokeSkill

//------------------------------------------------------------------------------------------

void SwgCreatureObject::endBaselines()
{
	CreatureObject::endBaselines();

	// if the player has a bounty value, register him with the bounty manager
	if (isAuthoritative() && isPlayerControlled())
	{
		int bountyValue = getBountyValue();
		if (bountyValue > 0)
		{
			SwgPlayerObject const * player = safe_cast<SwgPlayerObject const *>(PlayerCreatureController::getPlayerObject(this));
			if (player)
			{
				JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(ServerUniverse::getInstance()).getJediManager();
				if (jediManager)
				{
					jediManager->addJedi(getNetworkId(), getObjectName(),
						getPosition_w(), getSceneId(), player->getJediVisibility(),
						bountyValue, getLevel(),
						0, player->getJediState(), getSpentJediSkillPoints(), getPvpFaction());
				}
			}
		}
	}
}

//------------------------------------------------------------------------------------------

void SwgCreatureObject::onAddedToWorld()
{
	CreatureObject::onAddedToWorld();
	if (isAuthoritative() && isPlayerControlled() && (getBountyValue() > 0))
	{
		JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(ServerUniverse::getInstance()).getJediManager();
		if (jediManager != nullptr)
		{
			jediManager->updateJediLocation(getNetworkId(), getPosition_w(), getSceneId());
		}
	}
}

//------------------------------------------------------------------------------------------

void SwgCreatureObject::levelChanged() const
{
	CreatureObject::levelChanged();

	// If the player has a bounty value, update the level with the bounty manager
	if (isAuthoritative() && isPlayerControlled() && (getBountyValue() > 0))
	{
		JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(ServerUniverse::getInstance()).getJediManager();
		if (jediManager)
		{
			jediManager->updateJedi(getNetworkId(), -1, -1, getLevel(), -1);
		}
	}
}

//------------------------------------------------------------------------------------------

void SwgCreatureObject::setPvpFaction(Pvp::FactionId factionId)
{
	Pvp::FactionId oldId = getPvpFaction();
	CreatureObject::setPvpFaction(factionId);
	Pvp::FactionId newId = getPvpFaction();
	if ((oldId != newId) && isAuthoritative() && isPlayerControlled() && (getBountyValue() > 0))
	{
		JediManagerObject * jediManager = static_cast<SwgServerUniverse &>(ServerUniverse::getInstance()).getJediManager();
		if (jediManager != nullptr)
		{
			jediManager->updateJediFaction(getNetworkId(), newId);
		}
	}
}
