//========================================================================
//
// InstallationObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/InstallationObject.h"

#include "serverGame/Chat.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/InstallationController.h"
#include "serverGame/InstallationSynchronizedUi.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ResourcePoolObject.h"
#include "serverGame/ServerInstallationObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include "serverUtility/ServerClock.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/MessageQueueGenericResponse.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "swgServerNetworkMessages/MessageQueueActivateInstallation.h"
#include "swgServerNetworkMessages/MessageQueueDeactivateInstallation.h"
#include "swgServerNetworkMessages/MessageQueueInstallationHarvest.h"
#include "swgServerNetworkMessages/MessageQueueResourceFinishHarvest.h"
#include "swgServerNetworkMessages/MessageQueueResourceSetInstalledEfficiency.h"

#include <stdio.h>
#include <vector>

//----------------------------------------------------------------------

static const std::string OBJVAR_ACCUMULATED_TIME("_installation.acclTime");

const SharedObjectTemplate * InstallationObject::m_defaultSharedTemplate = nullptr;

namespace InstallationObjectNamespace
{
	InstallationObject::AllInstallationsSet g_installationList;
}

using namespace InstallationObjectNamespace;

//----------------------------------------------------------------------


InstallationObject::InstallationObject(const ServerInstallationObjectTemplate* newTemplate) :
	TangibleObject(newTemplate),
	m_installationType(0),
	m_tickCount(0.0f),
	m_activated(false),
	m_power(0),
	m_powerRate(0),
	m_activateStartTime(0)
{
	addMembersToPackages();

	IGNORE_RETURN(g_installationList.insert(this));

	ObjectTracker::addInstallation();
}

//-----------------------------------------------------------------------

InstallationObject::~InstallationObject()
{
	IGNORE_RETURN(g_installationList.erase(this));

	ObjectTracker::removeInstallation();
}

//-----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * InstallationObject::getDefaultSharedTemplate(void) const
{
static const ConstCharCrcLowerString templateName("object/installation/base/shared_installation_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(
			ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create "
			"default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "InstallationObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// InstallationObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void InstallationObject::removeDefaultTemplate(void)
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// InstallationObject::removeDefaultTemplate

//-----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
Controller* InstallationObject::createDefaultController(void)
{
	Controller* _controller = new InstallationController(this);

	setController(_controller);
	return _controller;
}

//------------------------------------------------------------------------------------------

void InstallationObject::getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const
{
	if(name == "powerValue")
	{
		m_power.pack(target);
	}
	else if(name == "powerRate")
	{
		m_powerRate.pack(target);
	}
	else
	{
		TangibleObject::getByteStreamFromAutoVariable(name, target);
	}
}

//------------------------------------------------------------------------------------------

void InstallationObject::setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source)
{
	Archive::ReadIterator ri(source);
	if(name == "powerValue")
	{
		m_power.unpackDelta(ri);
	}
	else if(name == "powerRate")
	{
		m_powerRate.unpackDelta(ri);
	}
	else
	{
		TangibleObject::setAutoVariableFromByteStream(name, source);
	}
	addObjectToConcludeList();
}

// ----------------------------------------------------------------------

/**
 * Activate an object as an installation.  Assumes the object has already been
 * placed in its intended location.
 */
void InstallationObject::activate(const NetworkId &actorId)
{
	if (isAuthoritative())
	{
		const ServerObject *actor = safe_cast<const ServerObject*>(NetworkIdManager::getObjectById(actorId));

		if (m_activated.get())
			return;

		float currentPower = getPower();
		if (currentPower <= 0 && getPowerRate() > 0)
		{
			if (actor)
				Chat::sendSystemMessage(*actor, SharedStringIds::harvester_no_power, Unicode::String());
			return;
		}
	
		m_activated = true;
		setCondition(ServerTangibleObjectTemplate::C_onOff);
		setObjVarItem(OBJVAR_ACCUMULATED_TIME, 0.0f);
		m_activateStartTime = static_cast<float>(ServerClock::getInstance().getGameTimeSeconds());
		m_power = currentPower;
		scheduleForAlter();
	}
	else
	{
		Controller *const controller = getController();
		NOT_NULL(controller);
		controller->appendMessage(static_cast<int>(CM_activateInstallation),0.0f,new MessageQueueActivateInstallation(),
								  GameControllerMessageFlags::SEND | 
								  GameControllerMessageFlags::RELIABLE |
								  GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

/**
 * Deactivate an object as an installation.
 */
void InstallationObject::deactivate()
{
	if (isAuthoritative())
	{
		if (!m_activated.get())
			return;

		// adjust power for time active
		float currentTime = static_cast<float>(ServerClock::getInstance().getGameTimeSeconds());
		setActivateStartTime(currentTime);
		
		// turn the station off
		m_tickCount = 0.0f;
		m_activated = false;
		clearCondition(ServerTangibleObjectTemplate::C_onOff);
		scheduleForAlter();
	}
	else
	{
		Controller *const controller = getController();
		NOT_NULL(controller);
		controller->appendMessage(static_cast<int>(CM_deactivateInstallation),0.0f,new MessageQueueDeactivateInstallation(),
								  GameControllerMessageFlags::SEND | 
								  GameControllerMessageFlags::RELIABLE |
								  GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void InstallationObject::harvest()
{
	if (isAuthoritative())
	{
	}
	else
	{
		Controller *const controller = getController();
		NOT_NULL(controller);
		controller->appendMessage(static_cast<int>(CM_installationHarvest),0.0f,new MessageQueueInstallationHarvest(),
								  GameControllerMessageFlags::SEND | 
								  GameControllerMessageFlags::RELIABLE |
								  GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void InstallationObject::finishActivate(float installedEfficiency, float tickCount)
{
	if (isAuthoritative())
	{
		if (m_activated.get())
		{
			m_tickCount = tickCount;
		}
		else
		{
			DEBUG_REPORT_LOG(true,("Got setInstalledEfficiency for installation %s, which was not activated.\n",getNetworkId().getValueString().c_str()));
		}
	}
	else
	{
		MessageQueueResourceSetInstalledEfficiency* const message = new MessageQueueResourceSetInstalledEfficiency (installedEfficiency,tickCount);
		Controller *const controller = getController();
		NOT_NULL(controller);
		controller->appendMessage(static_cast<int>(CM_resourceSetInstalledEfficiency),0.0f,message,
								  GameControllerMessageFlags::SEND | 
								  GameControllerMessageFlags::RELIABLE |
								  GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

//----------------------------------------------------------------------

float InstallationObject::alter(float time)
{
	PROFILER_AUTO_BLOCK_DEFINE("InstallationObject::alter");
	return TangibleObject::alter(time);
}

// ----------------------------------------------------------------------

void InstallationObject::handleCMessageTo (const MessageToPayload &message)
{
	if (message.getMethod() == "C++deactivate")
	{
		deactivate();
	}
	else if (message.getMethod() == "C++finishActivate")
	{
		float installedEfficiency;
		float tickCount;
		std::string packedData(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
		sscanf(
			packedData.c_str(),
			"%f %f",
			&installedEfficiency,
			&tickCount);
		finishActivate(installedEfficiency,tickCount);
	}
	else
		TangibleObject::handleCMessageTo(message);
}

// ----------------------------------------------------------------------

void InstallationObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	TangibleObject::getAttributes(data);
}

// ----------------------------------------------------------------------

/**
 * Returns the amount of time the installation has been powered, based on the 
 * current time.
 *
 * @return the time powered, in secs
 */
float InstallationObject::getTimePowered(void) const
{
	if (isActive())
	{
		float currentTime = static_cast<float>(ServerClock::getInstance().getGameTimeSeconds());
		const_cast<InstallationObject*>(this)->setActivateStartTime(currentTime);
	}

	float accumulatedTime = 0.0f;
	getObjVars().getItem(OBJVAR_ACCUMULATED_TIME, accumulatedTime);
	return accumulatedTime;
}	// InstallationObject::getTimePowered()

// ----------------------------------------------------------------------

/**
 * Returns the time at which the installation will run out of power.
 *
 * @return the time in secs, -1 if it never runs out, and 0 if the installation 
 *         is inactive
 */
float InstallationObject::getOutOfPowerTime(void) const
{
	if (isActive() && m_powerRate.get() > 0)
		return m_activateStartTime.get() + (m_power.get() / m_powerRate.get()) * 3600.0f;
	else if (m_powerRate.get() <= 0)
		return -1.0f;
	return 0.0f;
}	// InstallationObject::getOutOfPowerTime

// ----------------------------------------------------------------------

/**
 * Sets the power consumption rate of the installation, in units/hour.
 *
 * @param rate		the power rate
 */
void InstallationObject::setPowerRate(float rate)
{
	// don't do anything if we're not really changing the rate
	if (std::abs(rate - m_powerRate.get()) < 0.01f)
		return;
	
	if (isAuthoritative())
	{
		if (isActive())
		{
			// adjust the power for our current rate by changing the active time to our current time
			setActivateStartTime(static_cast<float>(ServerClock::getInstance().getGameTimeSeconds()));
		}
		m_powerRate = rate;	
	}
}

// ----------------------------------------------------------------------

/**
 * Changes the activation time to a given value. Will adjust the power available 
 * based on the difference between the current active time and the new one.
 *
 * @param time		the new activation time for the installation
 */
void InstallationObject::setActivateStartTime(float time)
{
	// if we're not active, there's no point in changing the time
	if (!isActive())
		return;

	// see if we need to update the count of how long we've been powered
	if (time > m_activateStartTime.get() && 
		(m_power.get() > 0 || m_powerRate.get() <= 0))
	{
		// figure out how much power we used to get to the time value
		float deltaTime = time - m_activateStartTime.get();
		if (m_powerRate.get() > 0)
		{
			float deltaPower = (deltaTime / 3600.0f) * m_powerRate.get();
			if (m_power.get() - deltaPower < 0.0f)
			{
				// we don't have enough power to make it to time; figure out
				// when we ran out of power
				deltaTime = (m_power.get() * 3600.0f / m_powerRate.get());
			}
		}
		
		float accumulatedTime = 0.0f;
		getObjVars().getItem(OBJVAR_ACCUMULATED_TIME, accumulatedTime);
		setObjVarItem(OBJVAR_ACCUMULATED_TIME, accumulatedTime + deltaTime);
	}
	m_activateStartTime = time;
}	// InstallationObject::setActivateStartTime

// ----------------------------------------------------------------------

/**
 * Returns the amount of power the installation currently has left.
 */
float InstallationObject::getPower(void) const
{
	float result = m_power.get();
	if (result > 0.0f && m_powerRate.get() > 0.0f)
	{
		// return the power minus the amount that would be drained now
		float powerUsed = (getTimePowered() / 3600.0f) * m_powerRate.get();
		result = m_power.get() - powerUsed;
	}
	if (result <= 0.0f)
		result = 0.0f;
	return result;
}	// InstallationObject::getPower

// ----------------------------------------------------------------------

/**
 * Adds or removes power from the installation.
 *
 * @param delta		amount to change the power by
 */
void InstallationObject::changePower(float delta)
{
	if (isAuthoritative())
	{
		if (delta >= 0)
			LOG("CustomerService", ("Power:adding %.2f power to installation %s owned by %s", delta, getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(getOwnerId()).c_str()));
		else
			LOG("CustomerService", ("Power:removing %.2f power from installation %s owned by %s", -delta, getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(getOwnerId()).c_str()));
		float currentPower = getPower();
		float newPower = currentPower + delta;
		if (newPower > 0)
		{
			if (m_powerRate.get() > 0.0f)
				newPower += (getTimePowered() / 3600.0f) * m_powerRate.get();
			m_power = newPower;
		}
		else
		{
			// turn the installation off, and set the power so getPower will
			// return exactly 0
			if (isActive())
			{
				deactivate();
				LOG("CustomerService", ("Power:shut down installation %s owned by %s due to 0 power", getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(getOwnerId()).c_str()));
			}
			float powerUsed = (getTimePowered() / 3600.0f) * m_powerRate.get();
			m_power = powerUsed;
		}
	}
	else
	{
		WARNING(true, ("InstallationObject::changePower called for non-auth "
			"object %s", getNetworkId().getValueString().c_str()));
	}
}	// InstallationObject::changePower

//-----------------------------------------------------------------------

const InstallationObject::AllInstallationsSet & InstallationObject::getAllInstallations()
{
	return g_installationList;
}

// ======================================================================
