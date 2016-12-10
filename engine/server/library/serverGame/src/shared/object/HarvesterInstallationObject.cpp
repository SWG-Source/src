//========================================================================
//
// HarvesterInstallationObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/HarvesterInstallationObject.h"

#include "serverGame/Chat.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/HarvesterInstallationController.h"
#include "serverGame/InstallationSynchronizedUi.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ResourcePoolObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerHarvesterInstallationObjectTemplate.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/MessageToPayload.h"
#include "serverUtility/ServerClock.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/MessageQueueGenericResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueHarvesterResourceData.h"
#include "sharedNetworkMessages/MessageQueueResourceEmptyHopper.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedUtility/InstallationResourceData.h"
#include "swgServerNetworkMessages/MessageQueueResourceFinishHarvest.h"
#include "swgServerNetworkMessages/MessageQueueResourceSetInstalledEfficiency.h"

#include <stdio.h>
#include <vector>

//-----------------------------------------------------------------------

namespace
{
	namespace ObjVars
	{
		const std::string resource = "resource";
	}
}

//----------------------------------------------------------------------


HarvesterInstallationObject::HarvesterInstallationObject(const ServerHarvesterInstallationObjectTemplate* newTemplate) :
	InstallationObject(newTemplate),
	m_installedEfficiency(0.0f),
	m_resourceType(NetworkId::cms_invalid),
	m_maxExtractionRate(newTemplate->getMaxExtractionRate()),
	m_currentExtractionRate(static_cast<float>(newTemplate->getCurrentExtractionRate())),
	m_maxHopperAmount(newTemplate->getMaxHopperSize()),
	m_hopperResource(NetworkId::cms_invalid),
	m_hopperAmount(0.0f),
	m_survey(nullptr),
	m_surveyTime(0)
{
	//Installation objects have real time updated UI
	addMembersToPackages();
}

//-----------------------------------------------------------------------

HarvesterInstallationObject::~HarvesterInstallationObject()
{
	delete m_survey;
	m_survey = nullptr;
	// 	m_synchronizedUi deleted by superclass
}

//-----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
Controller* HarvesterInstallationObject::createDefaultController(void)
{
	Controller* _controller = new HarvesterInstallationController(this);

	setController(_controller);
	return _controller;
}

// ----------------------------------------------------------------------

/**
 * Activate an object as a harvester.  Assumes the object has already been
 * placed in its intended location.
 */
void HarvesterInstallationObject::activate(const NetworkId &actorId)
{
	const ServerObject *actor = safe_cast<const ServerObject*>(NetworkIdManager::getObjectById(actorId));

	if (getHopperContents() >= m_maxHopperAmount.get())
	{
		if (actor)
			Chat::sendSystemMessage(*actor,SharedStringIds::harvester_hopper_full,Unicode::String());
		return;
	}

	bool active = isActive();
	InstallationObject::activate(actorId);
	if (!active && isActive())
	{
		ResourcePoolObject const * const resourcePool = getSelectedResourcePool();

		if (!resourcePool)
		{
			if (actor)
				Chat::sendSystemMessage(*actor,SharedStringIds::harvester_no_resource,Unicode::String());
			deactivate();
			return;
		}

		if (resourcePool->isDepleted())
		{
			if (actor)
				Chat::sendSystemMessage(*actor,SharedStringIds::harvester_resource_depleted,Unicode::String());
			deactivate();
			return;
		}

		if (ConfigServerGame::getDeactivateHarvesterIfDamaged() && (getDamageTaken() > 0))
		{
			if (actor)
			{
				// Send cute breakdown message

				float damageScale = 1.0f-(static_cast<float>(getDamageTaken())/static_cast<float>(getMaxHitPoints()));

				if (damageScale < 0.25)
					Chat::sendSystemMessage(*actor,SharedStringIds::harvester_no_maint_1,Unicode::String());
				else if (damageScale < 0.5)
					Chat::sendSystemMessage(*actor,SharedStringIds::harvester_no_maint_2,Unicode::String());
				else if (damageScale < 0.75)
					Chat::sendSystemMessage(*actor,SharedStringIds::harvester_no_maint_3,Unicode::String());
				else
					Chat::sendSystemMessage(*actor,SharedStringIds::harvester_no_maint_4,Unicode::String());
			}
			deactivate();
		}

		Vector location(getTransform_o2p().getPosition_p());
		//apply current extraction rate multplier for things like harvester bonus
		m_installedEfficiency = (m_currentExtractionRate.get() * ConfigServerGame::getHarvesterExtractionRateMultiplier()) * resourcePool->getEfficiencyAtLocation(location.x,location.z);
		setTickCount(static_cast<float>(ServerClock::getInstance().getGameTimeSeconds()));
	}
}

// ----------------------------------------------------------------------

/**
 * Deactivate an object as a harvester.
 */
void HarvesterInstallationObject::deactivate()
{
	bool active = isActive();
	InstallationObject::deactivate();
	if (active && !isActive())
	{
		m_installedEfficiency=0.0f;
		roundHopperContents(); // fractional units are discarded when harvester is deactivated
	}
}

// ----------------------------------------------------------------------

void HarvesterInstallationObject::harvest()
{
	if (isAuthoritative() && isActive())
	{
		if (m_installedEfficiency.get() == 0)
			return;

		if (ConfigServerGame::getDeactivateHarvesterIfDamaged() && (getDamageTaken() > 0))
			deactivate();
		else
		{
			ResourcePoolObject const * const pool=getSelectedResourcePool();
			if (pool)
			{
				float condition = static_cast<float>(getMaxHitPoints() - getDamageTaken()) / static_cast<float>(getMaxHitPoints());

				DEBUG_FATAL(condition < 0 || condition > 1, ("Condition was %f, should have been between 0 and 1\n",condition));
				if (condition< 0 || condition > 1)
					condition = 0;

				float oldTickCount = getTickCount();
				float amountCollected = (pool->harvest(m_installedEfficiency.get(), static_cast<uint32>(getTickCount()))) * condition;
				float newTickCount = static_cast<float>(ServerClock::getInstance().getGameTimeSeconds());
				setTickCount(newTickCount);

				// Check for out of power
				if ((getOutOfPowerTime() > 0) && (getOutOfPowerTime() < newTickCount))
				{
					if (getOutOfPowerTime() < oldTickCount)
					{
						amountCollected = 0;
					}
					else
					{
						float runRatio = (getOutOfPowerTime() - oldTickCount) / (newTickCount - oldTickCount);
						DEBUG_REPORT_LOG(true,("Scaling output by %f because power ran out.\n",runRatio));
						amountCollected *= runRatio;
					}
					deactivate();
					LOG("CustomerService", ("Power:shut down installation %s owned by %s due to 0 power", getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(getOwnerId()).c_str()));
				}

				int hopperContents=static_cast<int>(getHopperContents());
				if (hopperContents + amountCollected > m_maxHopperAmount.get())
				{
					addResource(m_resourceType.get(),static_cast<float>(m_maxHopperAmount.get()-hopperContents));
					deactivate();
				}
				else
				{
					addResource(m_resourceType.get(),amountCollected);
				}
			}

			if (!pool || pool->isDepleted())
			{
				deactivate();
			}
		}
	}
	InstallationObject::harvest();
}

//----------------------------------------------------------------------

void HarvesterInstallationObject::selectResource (const NetworkId & resourceTypeId, const NetworkId &actor)
{
	if (isAuthoritative())
	{
		bool found=false;

		if (!m_survey)
			takeSurvey();
		NOT_NULL(m_survey);
		for (std::vector<ResourceTypeObject const *>::iterator i=m_survey->begin(); i!=m_survey->end(); ++i)
		{
			if (*i && (*i)->getNetworkId()==resourceTypeId)
				found=true;
		}

		if (!found)
		{
			return;
		}

		if (isActive())
		{
			deactivate();
			m_resourceType = resourceTypeId;
			activate(actor);
		}
		else
			m_resourceType = resourceTypeId;
	
		scheduleForAlter();
	}
	else
	{
		//TODO:  network message
	}
}

// ----------------------------------------------------------------------

/**
 * Record all the available resource pools.  This data is cached but not persisted.
 */
void HarvesterInstallationObject::takeSurvey()
{
	if (m_survey)
		m_survey->clear();
	else
		m_survey = new std::vector<ResourceTypeObject const *>;

	PlanetObject *planet=ServerUniverse::getInstance().getCurrentPlanet();

	if (!planet)
	{
		WARNING (true, ("Cannot take survey if no planet object exists."));
		return;
	}

	const ResourceClassObject *resClass = getMasterClass();
	if (resClass)
		planet->getAvailableResourceList(*m_survey, *resClass);

	m_surveyTime = ServerClock::getInstance().getGameTimeSeconds();

	InstallationSynchronizedUi * const syncUi = dynamic_cast<InstallationSynchronizedUi *>(getSynchronizedUi ());
	if (syncUi)
		syncUi->resetResourcePools ();
}

//----------------------------------------------------------------------

ServerSynchronizedUi * HarvesterInstallationObject::createSynchronizedUi ()
{
	return new InstallationSynchronizedUi(*this);
}

//----------------------------------------------------------------------

void HarvesterInstallationObject::getResourceData (ResourceDataVector & data)
{
	InstallationSynchronizedUi * const syncUi = dynamic_cast<InstallationSynchronizedUi *>(getSynchronizedUi ());

	if (syncUi)
		syncUi->getResourceData (data);
}

// ----------------------------------------------------------------------

int HarvesterInstallationObject::getMaxExtractionRate() const
{
	return m_maxExtractionRate.get();
}

// ----------------------------------------------------------------------

float HarvesterInstallationObject::getCurrentExtractionRate() const
{
	return m_currentExtractionRate.get();
}

// ----------------------------------------------------------------------

float HarvesterInstallationObject::getInstalledExtractionRate() const
{
	return m_installedEfficiency.get(); //TODO:  rename ?
}

// ----------------------------------------------------------------------

int HarvesterInstallationObject::getMaxHopperAmount() const
{
	return m_maxHopperAmount.get();
}

// ----------------------------------------------------------------------

/**
 * Add up the amounts of all the resources in the hopper
 */
float HarvesterInstallationObject::getCurrentHopperAmount() const
{
	if (m_hopperAmount.get() >= 0)
		return m_hopperAmount.get();
	else
	{
		float total=0;
		DynamicVariableList::NestedList l(getObjVars(),ObjVars::resource);
		{
			for (DynamicVariableList::NestedList::const_iterator i=l.begin(); i!=l.end(); ++i)
			{
				float var=0;
				if (i.getValue(var))
					total+=var;
			}
		}
	
		return total;
	}
}

// ----------------------------------------------------------------------

/**
 * Return all the resource pools known to the current survey.  If the
 * survey is more than an hour old, refresh it.
 */
std::vector<ResourceTypeObject const *> const & HarvesterInstallationObject::getSurveyTypes()
{
	if (!m_survey || ServerClock::getInstance().getGameTimeSeconds() - m_surveyTime > (60*60))
	{
		delete m_survey;
		m_survey = nullptr;
		takeSurvey();
	}

	NOT_NULL(m_survey);
	return *m_survey;
}

//----------------------------------------------------------------------

float HarvesterInstallationObject::alter(float time)
{
	DEBUG_REPORT_LOG(true,("HarvesterInstallationObject::alter\n"));

	float const result = InstallationObject::alter (time);

	PROFILER_AUTO_BLOCK_DEFINE("HarvesterInstallationObject::alter");

	if (isAuthoritative() && isActive())
		harvest ();

	InstallationSynchronizedUi * const syncUi = dynamic_cast<InstallationSynchronizedUi *>(getSynchronizedUi ());
	if (syncUi)
	{
		syncUi->refresh();
	}

	addObjectToConcludeList();

	return getNextAlterTime(result);
}

//----------------------------------------------------------------------

float HarvesterInstallationObject::getNextAlterTime(float baseAlterTime) const
{
	if (baseAlterTime == AlterResult::cms_kill || baseAlterTime == AlterResult::cms_alterNextFrame) //lint !e777 // == with floats
		return baseAlterTime;

	float result = baseAlterTime;

	InstallationSynchronizedUi const *syncUi = dynamic_cast<InstallationSynchronizedUi const *>(getSynchronizedUi());

    //If GUI active update harvester more quickly
	if (syncUi)
		AlterResult::incorporateAlterResult(result, ConfigServerGame::getWatchedHarvesterAlterTime());
    
	return result;
}

//----------------------------------------------------------------------

/**
 * Return the contents of the hopper.
 * @data Vector to store the contents (optional)
 * @returns The sum of the resources in the hopper, i.e. total hopper
 * space consumed.
 */
float HarvesterInstallationObject::getHopperContents(HopperContentsVector * data) const
{
	if (m_hopperAmount.get() >= 0)
	{
		if (data && m_hopperAmount.get() != 0)
			data->push_back(std::make_pair(m_hopperResource.get(), m_hopperAmount.get()));
		return m_hopperAmount.get();
	}
	else
	{
		float total = 0;
		const DynamicVariableList::NestedList l(getObjVars(),ObjVars::resource);
		for (DynamicVariableList::NestedList::const_iterator pos=l.begin(); pos!=l.end(); ++pos)
		{
			float amount = 0;
			if (pos.getValue(amount))
			{
				NetworkId networkIdValue(pos.getName());
				if (data)
				{
					data->push_back (std::make_pair (networkIdValue, amount));
				}
				total += amount;
			}
		}
		return total;
	}
}

// ----------------------------------------------------------------------

/**
 * Throws away the contents of the hopper.
 */
void HarvesterInstallationObject::discardAllHopperContents()
{
	if (isAuthoritative())
	{
		removeObjVarItem(ObjVars::resource);
		m_hopperAmount=0.0f;
		m_hopperResource=NetworkId::cms_invalid;
	}
	else
	{
		//TODO: messages
	}
}

//----------------------------------------------------------------------

void HarvesterInstallationObject::emptyHopper(const NetworkId & playerId, const NetworkId & resourceId, int amount, bool discard, uint8 sequenceId)
{
	//TODO:  what if not authoritative?

	if (isAuthoritative())
	{
		CreatureObject * const player = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId (playerId));
		if (!player)
		{
			WARNING (true, ("bad player"));
			return;
		}

		bool success = false;

		ServerObject * const inventory = (player && !discard) ? player->getInventory () : 0;

		if (!discard && !inventory)
		{
			WARNING (true, ("no inventory"));
		}
		else
		{
			ResourceTypeObject const * const resType = ServerUniverse::getInstance().getResourceTypeById(resourceId);
			if (!resType)
			{
				WARNING (true, ("bad resource specified"));
			}
			else
			{
				int amountToRemove=0;
				if (amount < 0)
					amountToRemove = static_cast<int>(getAmountOfResource(resourceId));
				else
					amountToRemove = std::min(amount,static_cast<int>(getAmountOfResource(resourceId)));

				std::string crateTemplateName;
				resType->getCrateTemplate(crateTemplateName);
				ResourceContainerObject * crate = 0;
						
				if (inventory)
				{
					crate = dynamic_cast<ResourceContainerObject*>(ServerWorld::createNewObject(crateTemplateName, *inventory, true));
					if (!crate)
					{
						DEBUG_REPORT_LOG (!crate, ("error making crate %s.  (Inventory was probably full.)\n", crateTemplateName.c_str()));	
						amountToRemove = 0;
					}
				}
				
				if (crate)
				{
					amountToRemove = std::min (amountToRemove, crate->getMaxQuantity ());
					if (!crate->addResource (CachedNetworkId (resourceId), amountToRemove, playerId))
					{
						WARNING (true, ("error adding resource to crate"));
						amountToRemove = 0;
					}
				}
						
				if (amountToRemove == 0)
					DEBUG_REPORT_LOG (true, ("removed zero from harvester\n"));
				else
					addResource(resourceId, static_cast<float>(-amountToRemove));
				
				success = true;
			}
		}

		InstallationSynchronizedUi * const syncUi = dynamic_cast<InstallationSynchronizedUi *>(getSynchronizedUi ());
		if (syncUi)
			syncUi->refresh ();

		{
			MessageQueueGenericResponse * const resp = new MessageQueueGenericResponse (static_cast<int>(CM_clientResourceHarvesterEmptyHopper), success, sequenceId);

			Controller *const controller = player->getController();
			NOT_NULL(controller);
			controller->appendMessage(static_cast<int>(CM_clientResourceHarvesterEmptyHopperResponse),0.0f,resp,
				GameControllerMessageFlags::SEND |
				GameControllerMessageFlags::RELIABLE |
				GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}

		return;
	}

	//----------------------------------------------------------------------

	else
	{

		//TODO: multiserver messages
	}
}

// ----------------------------------------------------------------------

/**
 * Round the contents of the hopper to the nearest integer, discarding
 * any fractional amounts.
 */
void HarvesterInstallationObject::roundHopperContents()
{
	DEBUG_FATAL(!isAuthoritative(),("Programmer bug:  roundHopperContents() is a helper function that should only be called on an authoritative object.\n"));

	m_hopperAmount = static_cast<float>(static_cast<int>(m_hopperAmount.get()));
   
	std::vector<std::string> itemsToRemove;

	DynamicVariableList::NestedList resources(getObjVars(),ObjVars::resource);
	for (DynamicVariableList::NestedList::const_iterator i=resources.begin(); i!=resources.end(); ++i)
	{
		float floatVal = 0;
		if (i.getValue(floatVal))
		{
			int intVal = static_cast<int>(floatVal);
			if (intVal == 0)
				itemsToRemove.push_back(i.getNameWithPath());
			else
			{
				IGNORE_RETURN(setObjVarItem(i.getNameWithPath(),static_cast<float>(intVal)));
			}
		}
	}

	for (std::vector<std::string>::const_iterator j=itemsToRemove.begin(); j!=itemsToRemove.end(); ++j)
		removeObjVarItem(*j);
}

// ----------------------------------------------------------------------

void HarvesterInstallationObject::setCurrentExtractionRate(float currentExtractionRate)
{
	if (isAuthoritative())
	{ 
		if (currentExtractionRate > m_maxExtractionRate.get())
		{
			DEBUG_WARNING(true,("Ignoring setCurrentExtractionRate(%f) because it was above maxExtractionRate %i.\n",
								currentExtractionRate,m_maxExtractionRate.get()));
			return;
		}

		if (m_currentExtractionRate.get() == currentExtractionRate)
			return;  

		if (isActive())
		{
			//make sure we update the harvester contents with the old extraction rate
			//before changing to the new extraction rate value
			harvest();

			deactivate();
			m_currentExtractionRate = currentExtractionRate;
			activate(NetworkId::cms_invalid);
		}
		else
		{
			m_currentExtractionRate = currentExtractionRate;
		}
	}
	else
	{
		//TODO: messages
	}
}

// ----------------------------------------------------------------------

/**
 * Sets the maximum extraction rate.
 * If the new max rate is less than the current rate, sets the current
 * rate to the max rate.
 * @todo Is that how we want to handle that case?
 */
void HarvesterInstallationObject::setMaxExtractionRate(int maxExtractionRate)
{
	if (isAuthoritative())
	{
		m_maxExtractionRate=maxExtractionRate;
		if (maxExtractionRate > m_currentExtractionRate.get())
		{
			DEBUG_REPORT_LOG(true,("Reducing current extraction rate to %i because max was changed.\n",maxExtractionRate));
			setCurrentExtractionRate(static_cast<float>(maxExtractionRate));
		}
	}
	else
	{
		//TODO: messages
	}
}

// ----------------------------------------------------------------------

/**
 * Sets the maximum hopper amount.
 * If the new max hopper amount is greater than the amount in the hopper,
 * it sets the max but does not discard any resources.
 * @todo Is that how we want to handle that case?
 */
void HarvesterInstallationObject::setMaxHopperAmount(int maxHopperAmount)
{
	if (isAuthoritative())
	{
		m_maxHopperAmount=maxHopperAmount;
	}
	else
	{
		//TODO: messages
	}
}

// ----------------------------------------------------------------------

void HarvesterInstallationObject::handleCMessageTo (const MessageToPayload &message)
{
	InstallationObject::handleCMessageTo (message);
}

// ----------------------------------------------------------------------

ResourceClassObject *HarvesterInstallationObject::getMasterClass() const
{
	ResourceClassObject *obj = nullptr;
	const ServerHarvesterInstallationObjectTemplate *harvesterTemplate = dynamic_cast<const ServerHarvesterInstallationObjectTemplate*>(getObjectTemplate());
	if (harvesterTemplate)
		obj=ServerUniverse::getInstance().getResourceClassByName(harvesterTemplate->getMasterClassName());
	if (!obj)
		obj=ServerUniverse::getInstance().getResourceTreeRoot();
	return obj;
}

// ----------------------------------------------------------------------

void HarvesterInstallationObject::sendResourceDataToClient(CreatureObject &clientObject)
{
	ResourceDataVector dv;
	getResourceData (dv);

	if (!dv.empty ())
	{
		MessageQueueHarvesterResourceData * const newData = new MessageQueueHarvesterResourceData (getNetworkId (), dv);
		CreatureController *controller = safe_cast<CreatureController*>(clientObject.getController());
		controller->appendMessage(static_cast<int>(CM_clientResourceHarvesterResourceData), 0.0f, newData, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}
}

// ----------------------------------------------------------------------

void HarvesterInstallationObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	InstallationObject::getAttributes(data);
}

// ----------------------------------------------------------------------

/**
 * Add (or subtract) resources to the hopper.  In the case of subtraction,
 * rounds a negative result to 0.
 */
void HarvesterInstallationObject::addResource(const NetworkId &resourceId, float amount)
{
	if(isAuthoritative())
	{
		if (amount >= 0)
			LOG("CustomerService", ("Harvest:adding %.2f of resource %s to harvester %s owned by %s", amount, resourceId.getValueString().c_str(), getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(getOwnerId()).c_str()));
		else
			LOG("CustomerService", ("Harvest:removing %.2f of resource %s from harvester %s owned by %s", -amount, resourceId.getValueString().c_str(), getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(getOwnerId()).c_str()));
		if (m_hopperAmount.get() == 0 || m_hopperResource.get()==resourceId)
		{
			m_hopperResource = resourceId;
			float newValue = m_hopperAmount.get() + amount;
			m_hopperAmount = newValue > 0 ? newValue : 0;
		}	
		else
		{
			if (m_hopperAmount.get() > 0) // splitting a single-resource harvester into a multi-resource harvester
			{
				std::string varName=std::string("resource.")+m_hopperResource.get().getValueString();
				IGNORE_RETURN(setObjVarItem(varName,m_hopperAmount.get()));
				m_hopperResource = NetworkId::cms_invalid;
				m_hopperAmount = -1.0f; // flag that hopper amount is not meaningful and object variables should be used instead
			}
			
			std::string varName=std::string("resource.")+resourceId.getValueString(); // The name of the object variable is resource.XXX, where XXX is the networkId.
			float oldValue = 0.0f;
			IGNORE_RETURN(getObjVars().getItem(varName, oldValue));
			float newValue = oldValue + amount;
			if (newValue <= 0.0f)
			{
				removeObjVarItem (varName);
				if (!getObjVars().hasItem(std::string("resource")))
					m_hopperAmount = 0.0f; // back to being a single-resource harvester
			}
			else
				IGNORE_RETURN(setObjVarItem(varName,newValue));
		}
	}
	else
	{
		sendControllerMessageToAuthServer(CM_addResource, new MessageQueueGenericValueType<std::pair<NetworkId, float> >(std::make_pair(resourceId, amount)));
	}
}

// ----------------------------------------------------------------------

/**
 * Returns the amount of a particular resource in the hopper
 */
float HarvesterInstallationObject::getAmountOfResource(const NetworkId &resourceId) const
{
	if (m_hopperAmount.get() >= 0)
	{
		if (resourceId == m_hopperResource.get())
			return m_hopperAmount.get();
		else
			return 0;
	}
	else
	{
		// multiple resources in the hopper

		float currentAmount = 0;
		std::string objVarName(ObjVars::resource+'.'+resourceId.getValueString());
		if (!getObjVars().getItem(objVarName, currentAmount))
			return 0;
		else
			return currentAmount;
	}
}

// ----------------------------------------------------------------------

bool HarvesterInstallationObject::isEmpty() const
{
	return (getHopperContents(0) == 0.f);
}

// ----------------------------------------------------------------------

NetworkId const &HarvesterInstallationObject::getSelectedResourceTypeId() const
{
	return m_resourceType.get();
}

// ----------------------------------------------------------------------

ResourcePoolObject const * HarvesterInstallationObject::getSelectedResourcePool() const
{
	if (getSelectedResourceTypeId() == NetworkId::cms_invalid)
		return nullptr;
	
	ResourceTypeObject const * const typeObject = ServerUniverse::getInstance().getResourceTypeById(getSelectedResourceTypeId());
	if (typeObject)
	{
		return typeObject->getPoolForCurrentPlanet();
	}
	else
		return nullptr;
}

// ======================================================================
