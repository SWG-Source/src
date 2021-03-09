//========================================================================
//
// ManufactureInstallationObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ManufactureInstallationObject.h"

#include "UnicodeUtils.h"
#include "serverGame/Chat.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/DraftSchematicObject.h"
#include "serverGame/FactoryObject.h"
#include "serverGame/ManufactureInstallationController.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/NonCriticalTaskQueue.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ProsePackageManagerServer.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerFactoryObjectTemplate.h"
#include "serverGame/ServerManufactureInstallationObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverUtility/ServerClock.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedGame/CraftingData.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"

//----------------------------------------------------------------------

// slot names
const static ConstCharCrcLowerString MANF_SCHEMATIC_SLOT_NAME("manf_schematic");
const static ConstCharCrcLowerString INPUT_HOPPER_SLOT_NAME("ingredient_hopper");
const static ConstCharCrcLowerString OUTPUT_HOPPER_SLOT_NAME("output_hopper");
const static ConstCharCrcLowerString NEW_OBJECT_SLOT_NAME("anythingnomod1");

const static std::string DEFAULT_FACTORY_OBJECT_TEMPLATE("object/factory/base/base_factory.iff");

// skill names
const static std::string FACTORY_SKILL_MOD("factory_speed");

// objvar names
const static std::string OBJVAR_CRAFTING_TYPE("crafting.type");
const static std::string OBJVAR_OWNER_SKILL("manf.owner_skill");
const static std::string OBJVAR_OWNER_NAME("manf.owner_name");
const static std::string OBJVAR_OWNER_STATION_ID("manf.owner_station_id");
const static std::string OBJVAR_NUM_ITEMS("manf.numItems");

//----------------------------------------------------------------------

namespace ManufactureInstallationObjectNamespace
{
	#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

	namespace StringIds
	{
		MAKE_STRING_ID(system_msg, manf_done);
		MAKE_STRING_ID(system_msg, manf_done_sub);
		MAKE_STRING_ID(system_msg, manf_error);
		MAKE_STRING_ID(system_msg, manf_error_1);
		MAKE_STRING_ID(system_msg, manf_error_2);
		MAKE_STRING_ID(system_msg, manf_error_3);
		MAKE_STRING_ID(system_msg, manf_error_4);
		MAKE_STRING_ID(system_msg, manf_error_5);
		MAKE_STRING_ID(system_msg, manf_error_6);
		MAKE_STRING_ID(system_msg, manf_error_7);
		MAKE_STRING_ID(system_msg, manf_error_8);
		MAKE_STRING_ID(system_msg, manf_error_9);
		MAKE_STRING_ID(system_msg, manf_no_component);
		MAKE_STRING_ID(system_msg, manf_no_named_resource);
		MAKE_STRING_ID(system_msg, manf_no_power);
		MAKE_STRING_ID(system_msg, manf_no_unknown_resource);
		MAKE_STRING_ID(system_msg, no_ingredients);
		MAKE_STRING_ID(system_msg, no_power);
		MAKE_STRING_ID(system_msg, manf_schematic_error);
		MAKE_STRING_ID(system_msg, manf_schematic_generic_component);
		MAKE_STRING_ID(system_msg, manf_output_hopper_full);
	}

	bool isOutputHopperFull(ManufactureInstallationObject const & mio)
	{
		ServerObject const * const outputHopper = mio.getOutputHopper();
		if (outputHopper == nullptr)
			return false;

		VolumeContainer const * const hopperContainer = ContainerInterface::getVolumeContainer(*outputHopper);
		if (hopperContainer == nullptr)
			return false;

		return (hopperContainer->getCurrentVolume() >= hopperContainer->getTotalVolume());
	}
}

using namespace ManufactureInstallationObjectNamespace;

//----------------------------------------------------------------------

class ManufactureInstallationObject::TaskManufactureObject : public NonCriticalTaskQueue::TaskRequest
{
public:
	TaskManufactureObject          (const CachedNetworkId & manfInstallation);
	virtual ~TaskManufactureObject ();
	virtual bool run               ();
	
private:
	
	CachedNetworkId m_manfInstallation;
	
private:
	TaskManufactureObject            (const TaskManufactureObject&);
	TaskManufactureObject & operator=(const TaskManufactureObject&);
};

//----------------------------------------------------------------------

ManufactureInstallationObject::ManufactureInstallationObject(const ServerManufactureInstallationObjectTemplate* newTemplate) :
	InstallationObject(newTemplate),
	m_currentCrate(CachedNetworkId::cms_cachedInvalid)
{
//	addMembersToPackages();
}

//-----------------------------------------------------------------------

ManufactureInstallationObject::~ManufactureInstallationObject()
{
}

//-----------------------------------------------------------------------

/**
 * Initializes a newly created object.
 */
void ManufactureInstallationObject::initializeFirstTimeObject()
{
	InstallationObject::initializeFirstTimeObject();

	// initialize the manufacturer's skill bonus
	setObjVarItem(OBJVAR_OWNER_SKILL, 0);
}	// ManufactureInstallationObject::initializeFirstTimeObject

//-----------------------------------------------------------------------

/**
 * Initializes a newly created object.
 */
void ManufactureInstallationObject::endBaselines()
{
	InstallationObject::endBaselines();

	// if we are active, make sure we have the right data
	if (isAuthoritative() && isActive())
	{
		if (getSchematic() == nullptr)
		{
			WARNING(true, ("Manufacture installation %s is set to active but has not schematic available!", 
				getNetworkId().getValueString().c_str()));
			setTickCount(0);
			deactivate();
		}
	}
}	// ManufactureInstallationObject::endBaselines

//-----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
Controller* ManufactureInstallationObject::createDefaultController()
{
	Controller* const _controller = new ManufactureInstallationController(this);

	setController(_controller);
	return _controller;
}

// ----------------------------------------------------------------------

void ManufactureInstallationObject::onContainerChildLostItem(ServerObject * destination, ServerObject& item, ServerObject* source, ServerObject* transferer)
{
	InstallationObject::onContainerChildLostItem(destination, item, source, transferer);
	if (source == getInputHopper())
	{
		LOG("CustomerService", ("Crafting:removed object %s from manf station %s input hopper by %s", item.getNetworkId().getValueString().c_str(), getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(transferer).c_str()));
	}
	else if (source == getOutputHopper())
	{
		LOG("CustomerService", ("Crafting:removed object %s from manf station %s output hopper by %s", item.getNetworkId().getValueString().c_str(), getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(transferer).c_str()));
	}
}

// ----------------------------------------------------------------------

void ManufactureInstallationObject::onContainerChildGainItem(ServerObject& item, ServerObject* source, ServerObject* transferer)
{
	InstallationObject::onContainerChildGainItem(item, source, transferer);
	const ServerObject * destination = safe_cast<const ServerObject *>(ContainerInterface::getContainedByObject(item));
	if (destination == getInputHopper())
	{
		LOG("CustomerService", ("Crafting:added object %s to manf station %s input hopper by %s", item.getNetworkId().getValueString().c_str(), getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(transferer).c_str()));
	}
	else if (destination == getOutputHopper())
	{
		LOG("CustomerService", ("Crafting:added object %s to manf station %s output hopper by %s", item.getNetworkId().getValueString().c_str(), getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(transferer).c_str()));
	}
}

// ----------------------------------------------------------------------

/**
 * Returns a bitmask giving the types of objects the station can manufacture.
 *
 * @return the types mask
 */
uint32 ManufactureInstallationObject::getValidSchematicTypes() const
{
	int temp;
	if (getObjVars().getItem(OBJVAR_CRAFTING_TYPE,temp))
		return static_cast<uint32>(temp);
	else
	{
		WARNING_STRICT_FATAL(true, ("ManufactureInstallationObject::getValidSchematicTypes called on object %s that does not have the crafting type objvar <%s>", 
			getNetworkId().getValueString().c_str(), OBJVAR_CRAFTING_TYPE.c_str()));
		return 0;
	}
}	// ManufactureInstallationObject::getValidSchematicTypes

// ----------------------------------------------------------------------

/**
 * Returns the number of items created since the installation was last activated.
 *
 * @return the item count
 */
int ManufactureInstallationObject::getNumNewItems() const
{
	int numItems = 0;
	getObjVars().getItem(OBJVAR_NUM_ITEMS, numItems);
	return numItems;
}	// ManufactureInstallationObject::getNumNewItems

// ----------------------------------------------------------------------

/**
 * Returns the name of the owner of the installation.
 *
 * @return the owner name
 */
const std::string & ManufactureInstallationObject::getOwnerName() const
{
	static std::string name;

	getObjVars().getItem(OBJVAR_OWNER_NAME, name);
	return name;
}	// ManufactureInstallationObject::getOwnerName

// ----------------------------------------------------------------------

/**
 * Returns the station id of the owner of this installation.
 *
 * @return the owner station id
 */
StationId ManufactureInstallationObject::getOwnerStationId() const
{
	int id = 0;
	getObjVars().getItem(OBJVAR_OWNER_STATION_ID, id);
	return static_cast<StationId>(id);
}	// ManufactureInstallationObject::getOwnerStationId

// ----------------------------------------------------------------------

/**
 * Sets the id of the owner of this installation. Also sets the owner name,
 * since the owner may be offline when we need to send a message to him.
 *
 * @param id		the owner id
 */
void ManufactureInstallationObject::setOwnerId(const NetworkId &id)
{
	InstallationObject::setOwnerId(id);

	const Object * const object = NetworkIdManager::getObjectById(id);
	if (object != nullptr)
	{
		const ServerObject * const owner = safe_cast<const ServerObject *>(object);
		setObjVarItem(OBJVAR_OWNER_NAME, Unicode::wideToNarrow(owner->getObjectName()));

		// we need to store the owner's Station id for logging purposes
		const CreatureObject * const creatureOwner = owner->asCreatureObject();
		if (creatureOwner != nullptr)
		{
			const PlayerObject * const player = PlayerCreatureController::getPlayerObject(creatureOwner);
			if (player != nullptr)
			{
				setObjVarItem(OBJVAR_OWNER_STATION_ID, static_cast<int>(player->getStationId()));
			}
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Returns the input hopper object for this installation.
 *
 * @return the input hopper
 */
ServerObject * ManufactureInstallationObject::getInputHopper() const
{
	SlottedContainer const * const container = ContainerInterface::getSlottedContainer(*this);

	if (container == nullptr)
	{
		DEBUG_WARNING(true, ("Manufacture installation %s does not have a container!", getDebugInformation().c_str()));
		return nullptr;
	}

	Container::ContainerErrorCode tmp = Container::CEC_Success;
	const Container::ContainedItem hopperId = container->getObjectInSlot(SlotIdManager::findSlotId(INPUT_HOPPER_SLOT_NAME), tmp);
	if (hopperId == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("Manufacture installation %s does not have an input hopper!", getDebugInformation().c_str()));
		return nullptr;
	}

	return safe_cast<ServerObject *>(hopperId.getObject());
}	// ManufactureInstallationObject::getInputHopper

// ----------------------------------------------------------------------

/**
 * Returns the output hopper object for this installation.
 *
 * @return the output hopper
 */
ServerObject * ManufactureInstallationObject::getOutputHopper() const
{
	SlottedContainer const * const container = ContainerInterface::getSlottedContainer(*this);

	if (container == nullptr)
	{
		DEBUG_WARNING(true, ("Manufacture installation %s does not have a container!", getDebugInformation().c_str()));
		return nullptr;
	}

	Container::ContainerErrorCode tmp = Container::CEC_Success;
	const Container::ContainedItem hopperId = container->getObjectInSlot(SlotIdManager::findSlotId(OUTPUT_HOPPER_SLOT_NAME), tmp);
	if (hopperId == NetworkId::cms_invalid)
	{
		DEBUG_WARNING(true, ("Manufacture installation %s does not have an output hopper!", getDebugInformation().c_str()));
		return nullptr;
	}

	return safe_cast<ServerObject *>(hopperId.getObject());
}	// ManufactureInstallationObject::getOutputHopper

// ----------------------------------------------------------------------

/**
 * Adds a manufacturing schematic to this installation.
 *
 * @param schematic		the schematic to add
 * @param transferrer	who is adding the schematic
 *
 * @return true on success, false on fail
 */
bool ManufactureInstallationObject::addSchematic(ManufactureSchematicObject & schematic, ServerObject * transferer)
{
	if(isAuthoritative())
	{
		if (isActive())
			return false;

		// make sure the schematic is of a type that can be used in this station
		if ((schematic.getCategory() & getValidSchematicTypes()) == 0)
			return false;

		SlottedContainer const * const container = ContainerInterface::getSlottedContainer(*this);

		if (container == nullptr)
		{
			DEBUG_WARNING(true, ("Manufacture installation %s does not have a container!", getDebugInformation().c_str()));
			return false;
		}

		// see if there is a schematic already in the slot
		const SlotId slotId = SlotIdManager::findSlotId(MANF_SCHEMATIC_SLOT_NAME);
		Container::ContainerErrorCode tmp = Container::CEC_Success;
		const Container::ContainedItem schematicId = container->getObjectInSlot(slotId, tmp);
		if (schematicId != Container::ContainedItem::cms_invalid)
			return false;

		bool result = ContainerInterface::transferItemToSlottedContainer(*this, schematic, slotId, transferer, tmp);
		if (result)
			LOG("CustomerService", ("Crafting:added schematic %s to manf station %s by %s", schematic.getNetworkId().getValueString().c_str(), getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(transferer).c_str()));
		return result;
	}

	WARNING_STRICT_FATAL(! isAuthoritative(), ("ManufactureInstallationObject::addSchematic() returns a value and was inoked on a non-authoritative object (%s : %s)", getObjectTemplateName(), getNetworkId().getValueString().c_str()));
	return false; //@todo : is this correct?
}

// ----------------------------------------------------------------------

/**
 * Shuts down station if we are losing our schematic.
 */
bool ManufactureInstallationObject::onContainerAboutToLoseItem(ServerObject * destination, ServerObject& item, ServerObject* transferer)
{
	if (isAuthoritative())
	{
		bool isSchematic = false;
		const ManufactureSchematicObject * const schematic = getSchematic();
		if (schematic != nullptr && schematic->getNetworkId() == item.getNetworkId())
		{
			isSchematic = true;
			if (isActive())
			{
				WARNING(true, ("Manufacture installation %s is having its schematic %s removed while it is active, shutting down station",
					getNetworkId().getValueString().c_str(),
					item.getNetworkId().getValueString().c_str()));
				deactivate();
			}
		}
		bool result = InstallationObject::onContainerAboutToLoseItem(destination, item, transferer);
		if (result && isSchematic)
			LOG("CustomerService", ("Crafting:removed schematic %s from manf station %s by %s", schematic->getNetworkId().getValueString().c_str(), getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(transferer).c_str()));
		return result;
	}
	else
		return InstallationObject::onContainerAboutToLoseItem(destination, item, transferer);
}	// ManufactureInstallationObject::onContainerAboutToLoseItem

// ----------------------------------------------------------------------

/**
 * Returns the manufacturing schematic for this installation.
 *
 * @return the schematic
 */
ManufactureSchematicObject * ManufactureInstallationObject::getSchematic() const
{
	// get our manufacturing schematic
	SlottedContainer const * const container = ContainerInterface::getSlottedContainer(*this);

	if (container == nullptr)
	{
		DEBUG_WARNING(true, ("Manufacture installation %s does not have a container!", getDebugInformation().c_str()));
		return nullptr;
	}

	Container::ContainerErrorCode tmp = Container::CEC_Success;
	return dynamic_cast<ManufactureSchematicObject *>(container->getObjectInSlot(
														  SlotIdManager::findSlotId(MANF_SCHEMATIC_SLOT_NAME), tmp ).getObject());
}	// ManufactureInstallationObject::getSchematic

// ----------------------------------------------------------------------

/**
 * Returns the time needed to manufacture an object.
 *
 * @return the manufacture time, in secs
 */
float ManufactureInstallationObject::getTimePerObject() const
{
	ManufactureSchematicObject * const schematic = getSchematic();
	if (schematic == nullptr)
		return 0;

	if (ConfigServerGame::getManufactureTimeOverride() >= 1.0f)
		return ConfigServerGame::getManufactureTimeOverride();

	float baseTime = schematic->getManufactureTime() * schematic->getComplexity();
	if (baseTime < 1.0f)
	{
		WARNING(true, ("ManufactureInstallationObject::getTimePerObject base time = %.2f sec for manf schematic %s, using 1 sec.",
			baseTime, schematic->getNetworkId().getValueString().c_str()));
		baseTime = 1.0f;
	}

	float time = baseTime;

	// adjust the base time for the manufacturer's skills
	const NetworkId & ownerId = getOwnerId();
	if (ownerId != NetworkId::cms_invalid)
	{
		// if the owner is online, we'll use his skill, else we'll use the
		// skill value we saved as an objvar
		const CreatureObject * const owner = dynamic_cast<const CreatureObject *>(NetworkIdManager::getObjectById(ownerId));

		int ownerSkill = 0;
		getObjVars().getItem(OBJVAR_OWNER_SKILL,ownerSkill);

		int skill = 0;
		if (owner != nullptr)
		{
			skill = owner->getEnhancedModValueUncapped(FACTORY_SKILL_MOD);

			// if the owner's skill doesn't match our saved value, change the
			// saved value
			if (ownerSkill != skill)
			{
				const_cast<ManufactureInstallationObject *>(this)->setObjVarItem(
					OBJVAR_OWNER_SKILL, skill);
			}
		}
		else
		{
			skill = ownerSkill;
		}
		time -= (skill / 100.0f) * time;
	}

	if (time < 1.0f)
		time = 1.0f;

	return time;
}	// ManufactureInstallationObject::getTimePerObject

// ----------------------------------------------------------------------

/**
 * Activate the installation. Assumes the installation has already been
 * placed in its intended location.
 */
void ManufactureInstallationObject::activate(const NetworkId &actorId)
{
	if (isAuthoritative() && !isActive())
	{
		if (getSchematic() == nullptr)
		{
			WARNING(true, ("Manufacture installation %s tried to activate with "
				"no schematic available!", getNetworkId().getValueString().c_str()));
			return;
		}
	}
	InstallationObject::activate(actorId);

	if (isActive())
	{
		setTickCount(static_cast<float>(ServerClock::getInstance().getGameTimeSeconds()));
		setObjVarItem(OBJVAR_NUM_ITEMS, 0);

#if 0
		// TODO: createObject doesn't properly restore ingredients when it fails,
		// until it is fixed, this will eat resources.

		//-- see if we have the right ingredients
		if (!createObject (true))
		{
			setTickCount(0);	// to avoid recursion
			deactivate();
		}
#endif


	}
	else if (getPower() <= 0)
	{
		Unicode::String oob;
		{
			ProsePackage pp;
			ProsePackageManagerServer::createSimpleProsePackage (*this, StringIds::manf_no_power, pp);
			OutOfBandPackager::pack(pp, -1, oob);
		}
		const Object * const owner = NetworkIdManager::getObjectById(getOwnerId());
		if (owner != nullptr)
		{
			Chat::sendSystemMessage(getOwnerName(), Unicode::emptyString, oob);
		}

		Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::no_power, Unicode::emptyString, oob);
	}
}

// ----------------------------------------------------------------------

/**
 * Deactivate the installation.
 */
void ManufactureInstallationObject::deactivate()
{
	if (isAuthoritative() && isActive())
	{
		// tell the owner we have stopped making items
		Unicode::String oob;
		{
			ProsePackage pp;
			ProsePackageManagerServer::createSimpleProsePackage (*this, StringIds::manf_done, pp);
			pp.digitInteger = getNumNewItems();
			OutOfBandPackager::pack(pp, -1, oob);
		}
		const Object * const owner = NetworkIdManager::getObjectById(getOwnerId());
		if (owner != nullptr)
		{
			Chat::sendSystemMessage(getOwnerName(), Unicode::emptyString, oob);
		}

		Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::manf_done_sub, Unicode::emptyString, oob);

		InstallationObject::deactivate();
	}
}

// ----------------------------------------------------------------------

/**
 * Based on the time passed and ingredients available, creates objects and puts
 * them in the output hopper.
 */
void ManufactureInstallationObject::harvest()
{
	if (isAuthoritative() && isActive())
	{
		if (getTickCount() > 0)
		{
			ManufactureSchematicObject * schematic = getSchematic();
			int maxItemCount = -1 ;
			if (schematic)
			{
				maxItemCount = schematic->getCount();
			}

			if (schematic == nullptr || maxItemCount <= 0)
			{
				setTickCount(0);	// to avoid recursion
				deactivate();
				if (schematic == nullptr)
				{
					WARNING(true, ("ManufactureInstallationObject::harvest called on object %s with a nullptr schematic", 
						getNetworkId().getValueString().c_str()));

					Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::manf_error, StringIds::manf_error_1, Unicode::emptyString);
				}
				else
				{
					WARNING(true, ("ManufactureInstallationObject::harvest called on object %s with a max item count of %d", 
						getNetworkId().getValueString().c_str(), maxItemCount));

					Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::manf_error, StringIds::manf_error_2, Unicode::emptyString);
				}
				return;
			}

			float currentTime = static_cast<float>(ServerClock::getInstance().getGameTimeSeconds());
			float deltaTime = currentTime - getTickCount();
			float timePerObject = getTimePerObject();
			if (timePerObject < 1.0f)
			{
				WARNING(true, ("ManufactureInstallationObject::harvest time per object = %.2f sec for manf schematic %s, using 1 sec.",
					timePerObject, schematic->getNetworkId().getValueString().c_str()));
				timePerObject = 1.0f;
			}
			// make sure we had enough power to make the items
			float outOfPowerTime = getOutOfPowerTime();
			if (outOfPowerTime >= 0 && outOfPowerTime < currentTime)
			{
				deltaTime = outOfPowerTime - getTickCount();
				if (deltaTime < timePerObject)
				{
					// turn off the station
					setTickCount(0);	// to avoid recursion
					deactivate();
					LOG("CustomerService", ("Power:shut down installation %s owned by %s due to 0 power", getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(getOwnerId()).c_str()));
					// tell the owner we are out of power
					Unicode::String oob;
					{
						ProsePackage pp;
						ProsePackageManagerServer::createSimpleProsePackage (*this, StringIds::manf_no_power, pp);
						OutOfBandPackager::pack(pp, -1, oob);
					}
					const Object * owner = NetworkIdManager::getObjectById(getOwnerId());
					if (owner != nullptr)
					{
						Chat::sendSystemMessage(getOwnerName(), Unicode::emptyString, oob);
					}

					Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::no_power, Unicode::emptyString, oob);

					return;
				}
			}
			else if (deltaTime < timePerObject)
				return;
			while (deltaTime >= timePerObject && maxItemCount > 0)
			{
				NonCriticalTaskQueue::getInstance().addTask(new TaskManufactureObject(CachedNetworkId(*this)));
				deltaTime -= timePerObject;
				--maxItemCount;
			}
			// if we've queued the max number of items, don't try to queue any more,
			// but don't stop the station (this should give us enough time to make
			// the items in the queue)
			if (maxItemCount == 0)
				deltaTime = 0;
			setTickCount(currentTime - deltaTime);
			return;
		}
	}
	InstallationObject::harvest();
}	// ManufactureInstallationObject::harvest

// ----------------------------------------------------------------------

/**
 * Creates an object from the ingredients in the input hopper and put it in the
 * output hopper.
 *
 * @return true on success, false if the object couldn't be created
 */
bool ManufactureInstallationObject::createObject(bool testOnly)
{
	static const SlotId newObjectSlotId(SlotIdManager::findSlotId(NEW_OBJECT_SLOT_NAME));

	if (!isAuthoritative())
	{
		if (testOnly)
			return false;

		sendControllerMessageToAuthServer(CM_manufactureInstallationCreateObject, 0);
		return true;
	}
	
	int i;
	
	ManufactureSchematicObject * schematic = getSchematic();
	if (schematic == nullptr)
	{
		WARNING(true, ("ManufactureInstallationObject::createObject can't find schematic for station %s", 
			getNetworkId().getValueString().c_str()));
		
		Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::manf_error, StringIds::manf_error_3, Unicode::emptyString);
		
		return false;
	}
	
	if (schematic->getCount() <= 0)
	{
		WARNING(true, ("ManufactureInstallationObject::createObject station %s has schematic %s with count %d", 
			getNetworkId().getValueString().c_str(), 
			schematic->getNetworkId().getValueString().c_str(),
			schematic->getCount()));
		
		Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::manf_error, StringIds::manf_error_4, Unicode::emptyString);
		
		return false;
	}
	
	ServerObject * inputHopper = getInputHopper();
	if (inputHopper == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("ManufactureInstallationObject::createObject can't find input hopper for station %s", 
			getNetworkId().getValueString().c_str()));
		
		Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::manf_error, StringIds::manf_error_5, Unicode::emptyString);
		
		return false;
	}
	
	ServerObject * outputHopper = getOutputHopper();
	if (outputHopper == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("ManufactureInstallationObject::createObject can't find output hopper for station %s", 
			getNetworkId().getValueString().c_str()));
		
		Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::manf_error, StringIds::manf_error_6, Unicode::emptyString);
		
		return false;
	}

	// before attempting to create object, make sure there is room in the output hopper
	bool outputHopperFull = false;

	if (schematic->getItemsPerContainer() == 1)
	{
		outputHopperFull = isOutputHopperFull(*this);
	}
	else
	{
		// is there a current crate in the output hopper put we can stuff objects into
		FactoryObject * crate = getCurrentCrate(*schematic);
		if (crate == nullptr)
		{
			// is there room in the output hopper for a new crate
			outputHopperFull = isOutputHopperFull(*this);
		}
	}

	if (outputHopperFull)
	{
		Unicode::String oob;
		{
			ProsePackage pp;
			ProsePackageManagerServer::createSimpleProsePackage (*this, StringIds::manf_output_hopper_full, pp);
			OutOfBandPackager::pack(pp, -1, oob);
		}
		const Object * const owner = NetworkIdManager::getObjectById(getOwnerId());
		if (owner != nullptr)
		{
			Chat::sendSystemMessage(getOwnerName(), Unicode::emptyString, oob);
		}

		Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::manf_error, Unicode::emptyString, oob);

		return false;
	}
	
	// remove ingredients from the input hopper and add them to the manufacturing
	// schematic
	IngredientVector ingredients;
	Crafting::IngredientSlot slot;
	Crafting::Ingredients::const_iterator ingredientIter;
	int slotCount = schematic->getSlotsCount();
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	bool invalidSchematicComponent = false;
	for (i = 0; i < slotCount; ++i)
	{
		if (!schematic->getSlot(i, slot, false))
		{
			WARNING(true, ("ManufactureInstallationObject::createObject station %s can't find slot %d for schematic %s",
				getNetworkId().getValueString().c_str(), i, 
				schematic->getNetworkId().getValueString().c_str()));
			break;
		}
		if (slot.ingredients.empty())
		{
			WARNING(true, ("ManufactureInstallationObject::createObject station %s, schematic %s: slot %d has no ingredient info",
				getNetworkId().getValueString().c_str(),
				schematic->getNetworkId().getValueString().c_str(), i));
			break;
		}
		
		for (ingredientIter = slot.ingredients.begin();
			ingredientIter != slot.ingredients.end(); ++ingredientIter)
		{
			const NetworkId ingredientId((*ingredientIter)->ingredient);
			int ingredientCount = (*ingredientIter)->count;
			if (ingredientCount <= 0)
			{
				WARNING_STRICT_FATAL(true, ("ManufactureInstallationObject::createObject, Manufacture schematic %s has invalid ingredient %s count %d", 
					schematic->getNetworkId().getValueString().c_str(), ingredientId.getValueString().c_str(), ingredientCount));
				break;
			}
			if (slot.ingredientType == Crafting::IT_item ||
				slot.ingredientType == Crafting::IT_template ||
				slot.ingredientType == Crafting::IT_templateGeneric ||
				slot.ingredientType == Crafting::IT_schematic ||
				slot.ingredientType == Crafting::IT_schematicGeneric)
			{
				const Crafting::ComponentIngredient * componentInfo =
					safe_cast<const Crafting::ComponentIngredient *>((*ingredientIter).get());
				int j = 0;
				if (slot.ingredientType != Crafting::IT_templateGeneric)
				{
					if (componentInfo->ingredient != NetworkId::cms_invalid)
					{
						// the ingredient needs to be made with the same schematic
						if (transferCraftedIngredientToSchematic(*inputHopper, *schematic, componentInfo->ingredient, ingredientCount, ingredients))
						{
							j = ingredientCount;
						}
					}
					else
					{
						invalidSchematicComponent = true;
					}
				}
				else
				{
					// the ingredient needs to be made with the same template
					const ObjectTemplate * componentTemplate = ObjectTemplateList::fetch(
						componentInfo->templateName);
					if (componentTemplate != nullptr)
					{
						for (j = 0; j < ingredientCount; ++j)
						{
							if (transferTemplateIngredientToSchematic(*inputHopper, *schematic, *componentTemplate, ingredients) == NetworkId::cms_invalid)
							{
								break;
							}
						}
						componentTemplate->releaseReference();
					}
				}
				if (j != ingredientCount)
					break;
			}
			else if (slot.ingredientType == Crafting::IT_resourceType ||
				slot.ingredientType == Crafting::IT_resourceClass)
			{
				if (transferResourceTypeToSchematic(*inputHopper, *schematic,
					ingredientId, ingredientCount) == NetworkId::cms_invalid)
				{
					break;
				}
			}
			else
				break;
		}
		if (ingredientIter != slot.ingredients.end())
		{
			// tell the player what ingredient is missing
			Unicode::String oob;
			if (slot.ingredientType == Crafting::IT_item ||
				slot.ingredientType == Crafting::IT_template ||
				slot.ingredientType == Crafting::IT_templateGeneric ||
				slot.ingredientType == Crafting::IT_schematic ||
				slot.ingredientType == Crafting::IT_schematicGeneric)
			{
				const Crafting::ComponentIngredient * const componentInfo = safe_cast<const Crafting::ComponentIngredient *>((*ingredientIter).get());
				
				ProsePackage pp;
				ProsePackageManagerServer::createSimpleProsePackage (*this, StringIds::manf_no_component, pp);
				pp.other.str = componentInfo->componentName;
				OutOfBandPackager::pack(pp, -1, oob);
			}
			else
			{
				ResourceTypeObject const * const resource = ServerUniverse::getInstance().getResourceTypeById(ingredientIter->get()->ingredient);
				
				ProsePackage pp;
				ProsePackageManagerServer::createSimpleProsePackageParticipant (*this, pp.target);
				
				if (resource != nullptr)
				{
					pp.stringId = StringIds::manf_no_named_resource;
					pp.other.str = Unicode::narrowToWide(resource->getResourceName());
				}
				else
					pp.stringId = StringIds::manf_no_unknown_resource;
				
				OutOfBandPackager::pack(pp, -1, oob);
			}

			const Object * const owner = NetworkIdManager::getObjectById(getOwnerId());
			if (owner != nullptr)
				Chat::sendSystemMessage(getOwnerName(), Unicode::emptyString, oob);
			
			Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::no_ingredients, Unicode::emptyString, oob);
			
			break;
		}
	}

	if (invalidSchematicComponent)
	{
		LOG("CustomerService", ("Crafting:schematic %s owned "
			"by %s is invalid due to being made with a non-"
			"serialized component", 
			getNetworkId().getValueString().c_str(), 
			PlayerObject::getAccountDescription(getOwnerId()).c_str()));
		
		Unicode::String oob;
		ProsePackage pp;
		ProsePackageManagerServer::createSimpleProsePackage (*this, StringIds::manf_schematic_generic_component, pp);
		OutOfBandPackager::pack(pp, -1, oob);
		Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::manf_schematic_error, 
			Unicode::emptyString, oob);
//		Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::manf_error, StringIds::manf_error_9, Unicode::emptyString);
	}
	if (i != slotCount)
	{
		restoreIngredients(ingredients);
		schematic->clearSlotSources();
		return false;
	}
	else
	{
		if (testOnly)
			return true;

		destroyIngredients(ingredients);
	}
	
	if (schematic->getItemsPerContainer() == 1)
	{
		TangibleObject * object = safe_cast<TangibleObject *>(schematic->
			manufactureObject(getOwnerId(), *this, newObjectSlotId, false));
		if (object == nullptr)
		{
			WARNING_STRICT_FATAL(true, ("ManufactureInstallationObject::createObject, Unable to create object for station %s", 
				getNetworkId().getValueString().c_str()));
			
			Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::manf_error, StringIds::manf_error_8, Unicode::emptyString);
			
			return false;
		}
		if (!ContainerInterface::transferItemToVolumeContainer(*outputHopper, *object, nullptr, tmp))
		{
			object->permanentlyDestroy(DeleteReasons::BadContainerTransfer);
			WARNING_STRICT_FATAL(true, ("ManufactureInstallationObject::createObject, Unable to transfer new object for station %s, error code = %d",
				getNetworkId().getValueString().c_str(), tmp));
			
			Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::manf_error, StringIds::manf_error_9, Unicode::emptyString);
		
			return false;
		}
	}
	else
	{
		// put the object in a box of objects
		FactoryObject * crate = getCurrentCrate(*schematic);
		if (crate == nullptr)
		{
			// make a new crate to put the object in
			crate = makeNewCrate(*schematic);
		}
		if (crate == nullptr)
		{
			WARNING_STRICT_FATAL(true, ("ManufactureInstallationObject::createObject, Unable to create factory crate for station %s", 
				getNetworkId().getValueString().c_str()));
			
			Chat::sendPersistentMessage(*this, getOwnerName(), StringIds::manf_error, StringIds::manf_error_7, Unicode::emptyString);
			
			return false;
		}
		if (!crate->addObject())
		{
			// something went very wrong, shut down
			setTickCount(0);	// to avoid recursion
			deactivate();
			return false;
		}
		schematic->incrementCount(-1);
		if (schematic->getCount() == 0)
		{
			// delete the schematic
			schematic->permanentlyDestroy(DeleteReasons::Consumed);
			LOG("CustomerService", ("Crafting:destroyed schematic %s in manf station %s owned by %s", schematic->getNetworkId().getValueString().c_str(), getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(getOwnerId()).c_str()));
		}
	}
	setObjVarItem(OBJVAR_NUM_ITEMS, getNumNewItems() + 1);
	
	if (schematic->isBeingDestroyed())
	{
		// turn off the station
		setTickCount(0);	// to avoid recursion
		deactivate();
	}
	else
		schematic->clearSlotSources();

	return true;
}	// ManufactureInstallationObject::createObject

// ----------------------------------------------------------------------

/**
 * Restores the ingredients in the ingredient list to the input hopper.
 */
void ManufactureInstallationObject::restoreIngredients(IngredientVector const &ingredients)
{
	// Ingredients with a 0 count are objects which are still in the input hopper, so don't need to be moved
	for (IngredientVector::const_iterator i = ingredients.begin(); i != ingredients.end(); ++i)
	{
		const CachedNetworkId & itemId = (*i).first;
		int count = (*i).second;
		if (count != 0)
		{
			FactoryObject * crate = dynamic_cast<FactoryObject *>(itemId.getObject());
			if (crate != nullptr)
				crate->incrementCount(count);
		}
	}
}	// ManufactureInstallationObject::restoreIngredients

// ----------------------------------------------------------------------

/**
 * Destroys the ingredients in the ingredient list.
 */
void ManufactureInstallationObject::destroyIngredients(IngredientVector const &ingredients)
{
	for (IngredientVector::const_iterator i = ingredients.begin(); i != ingredients.end(); ++i)
	{
		const CachedNetworkId & itemId = (*i).first;
		int count = (*i).second;
		if (count == 0)
		{
			// this is a single component, destroy it
			itemId.getObject()->asServerObject()->permanentlyDestroy(
				DeleteReasons::Consumed);
			LOG("CustomerService", ("Crafting:destroying ingredient %s in manf station %s owned by %s", itemId.getValueString().c_str(), getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(getOwnerId()).c_str()));
		}
		else
		{
			FactoryObject * crate = dynamic_cast<FactoryObject *>(itemId.getObject());
			if (crate != nullptr)
			{
				LOG("CustomerService", ("Crafting:destroying %d ingredients from crate %s in manf station %s owned by %s", count, itemId.getValueString().c_str(), getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(getOwnerId()).c_str()));
				if (crate->getCount() == 0)
					crate->permanentlyDestroy(DeleteReasons::Consumed);
			}
		}
	}
}	// ManufactureInstallationObject::destroyIngredients

// ----------------------------------------------------------------------

/**
 * Transfers an object from the input hopper to a manufacturing schematic, if
 * the object was crafted with a given manufacturing schematic.
 *
 * @param inputHopper		the station's input hopper
 * @param schematic			manf schematic to transfer to
 * @param craftedId			id of the schematic the object must be crafted with
 * @param ingredientCount	number of ingredients needed
 *
 * @return true if there were enough ingredients, false if not
 */
bool ManufactureInstallationObject::transferCraftedIngredientToSchematic(
	ServerObject & inputHopper, ManufactureSchematicObject & schematic,
	const NetworkId & craftedId, int ingredientCount, IngredientVector &ingredients)
{
	WARNING_STRICT_FATAL( ! ( inputHopper.isAuthoritative() && 
		schematic.isAuthoritative() && isAuthoritative() ), (
		"transferCrafterIngredientToSchematic on ManufactureInstallationObject (%s:%s) without all of the participants (inputHopper %s:%s and schematic %s:%s) authoritative on this server!", 
		getObjectTemplateName(), 
		getNetworkId().getValueString().c_str(), 
		inputHopper.getObjectTemplateName(), 
		inputHopper.getNetworkId().getValueString().c_str(), 
		schematic.getObjectTemplateName(), 
		schematic.getNetworkId().getValueString().c_str()));
	// go through all the ingredients until we find one that has the same
	// crafted id as the component wanted

	VolumeContainer * container = ContainerInterface::getVolumeContainer(inputHopper);
	if (container == nullptr)
		return false;

	// stuff to keep track of crates that don't have enough ingredients
	int partialCrateObjectCount = 0;
	std::vector<FactoryObject *> partialCrates;

	for (ContainerIterator iter = container->begin(); iter != container->end() && ingredientCount > 0; ++iter)
	{
		const Container::ContainedItem & itemId = *iter;
		TangibleObject * object = safe_cast<TangibleObject *>(itemId.getObject());
		if (object != nullptr && object->getCraftedId() == craftedId)
		{
			// see if the object is a factory or not
			FactoryObject * crate = dynamic_cast<FactoryObject *>(object);
			if (crate != nullptr)
			{
				// see if the crate has enough objects or not
				int crateCount = crate->getCount();
				if (crateCount >= ingredientCount)
				{
					ingredients.push_back(std::make_pair(CachedNetworkId(*crate), ingredientCount));
					crate->incrementCount(-ingredientCount);
					ingredientCount = 0;
				}
				else
				{
					// save off this crate in case we have other crates that
					// will get us our ingredients
					partialCrateObjectCount += crateCount;
					partialCrates.push_back(crate);
					if (partialCrateObjectCount >= ingredientCount)
						break;
				}
			}
			else
			{
				// Make sure the ingredient we are considering has not already been used, since they are still in the container.
				std::pair<CachedNetworkId, int> const potentialIngredient(std::make_pair(CachedNetworkId(*object), 0));
				if (std::find(ingredients.begin(), ingredients.end(), potentialIngredient) == ingredients.end())
				{
					ingredients.push_back(potentialIngredient);
					--ingredientCount;
				}
			}
		}
	}

	// if we aren't done, see if we have enough small crates to give us what we
	// need
	if (ingredientCount > 0 && partialCrateObjectCount >= ingredientCount)
	{
		for (std::vector<FactoryObject *>::iterator iter = partialCrates.begin();
			iter != partialCrates.end() && ingredientCount > 0; ++iter)
		{
			FactoryObject * crate = *iter;
			int crateCount = crate->getCount();
			if (crateCount <= ingredientCount)
			{
				ingredients.push_back(std::make_pair(CachedNetworkId(*crate), crateCount));
				crate->incrementCount(-crateCount);
				ingredientCount -= crateCount;
			}
			else
			{
				ingredients.push_back(std::make_pair(CachedNetworkId(*crate), ingredientCount));
				crate->incrementCount(-ingredientCount);
				ingredientCount = 0;
			}
		}
	}

	if (ingredientCount == 0)
		return true;
	return false;
}	// ManufactureInstallationObject::transferCraftedIngredientToSchematic

// ----------------------------------------------------------------------

/**
 * Transfers an object from the input hopper to a manufacturing schematic, if
 * the object was made with a given template.
 *
 * @param inputHopper			the station's input hopper
 * @param schematic				manf schematic to transfer to
 * @param componentTemplate		template the object must match
 *
 * @return the id of the object transferred, or NetworkId::cms_invalid on failure
 */
const NetworkId & ManufactureInstallationObject::transferTemplateIngredientToSchematic(
	ServerObject & inputHopper, ManufactureSchematicObject & schematic,
	const ObjectTemplate & componentTemplate, IngredientVector &ingredients)
{
	// go through all the ingredients until we find one that has the same
	// template as the component wanted

	VolumeContainer * container = ContainerInterface::getVolumeContainer(
		inputHopper);
	if (container == nullptr)
		return NetworkId::cms_invalid;

	for (ContainerIterator iter = container->begin(); iter != container->end(); ++iter)
	{
		const Container::ContainedItem & itemId = *iter;
		TangibleObject * object = safe_cast<TangibleObject *>(itemId.getObject());
		if (object != nullptr)
		{
			// see if the object is a factory or not
			FactoryObject * crate = dynamic_cast<FactoryObject *>(object);
			if (crate != nullptr && crate->getCount() > 0)
			{
				if (crate->getContainedObjectTemplate() == &componentTemplate)
				{
					ingredients.push_back(std::make_pair(CachedNetworkId(*crate), 1));
					crate->incrementCount(-1);
					return crate->getNetworkId();
				}
			}
			else if (object->getObjectTemplate() == &componentTemplate)
			{
				// Make sure the ingredient we are considering has not already been used, since they are still in the container.
				std::pair<CachedNetworkId, int> const potentialIngredient(std::make_pair(CachedNetworkId(*object), 0));
				if (std::find(ingredients.begin(), ingredients.end(), potentialIngredient) == ingredients.end())
				{
					ingredients.push_back(std::make_pair(CachedNetworkId(*object), 0));
					return object->getNetworkId();
				}
			}
		}
	}

	return NetworkId::cms_invalid;
}	// ManufactureInstallationObject::transferTemplateIngredientToSchematic

// ----------------------------------------------------------------------

/**
 * Transfers a resource type from the input hopper to a manufacturing schematic.
 *
 * @param inputHopper		the station's input hopper
 * @param schematic			manf schematic to transfer to
 * @param resourceTypeId	id of the resource type to transfer
 * @param resourceCount		number of resource units to transfer
 *
 * @return the id of the resource transferred, or NetworkId::cms_invalid on failure
 */
const NetworkId & ManufactureInstallationObject::transferResourceTypeToSchematic(
	ServerObject & inputHopper, ManufactureSchematicObject & schematic,
	const NetworkId & resourceTypeId, int resourceCount)
{
	VolumeContainer * const container = ContainerInterface::getVolumeContainer(inputHopper);
	if (container == nullptr)
		return NetworkId::cms_invalid;

	// keep a map of the sources that are providing the resources
	std::vector<std::pair<NetworkId, int> > sources;
	std::vector<ResourceContainerObject *>  smallCrates;
	int smallCrateResourceCount = 0;

	ContainerIterator iter = container->begin();
	// if no resource container has enough resources available, keep a list of
	// valid containers around to see if we can combine them
	bool foundIt = false;
	while (!foundIt && iter != container->end())
	{
		const Container::ContainedItem & itemId = *iter;
		ResourceContainerObject * object = dynamic_cast<ResourceContainerObject *>(
			itemId.getObject());
		if (object != nullptr && object->getResourceTypeId() == resourceTypeId)
		{
			// see if this crate fills our requirements
			if (object->getQuantity() >= resourceCount)
			{
				if (!object->removeResource(resourceTypeId, resourceCount, &sources))
					return NetworkId::cms_invalid;
				foundIt = true;
			}
			else
			{
				// keep track of small crates - we may be able to add their resources
				// together to get what we need
				smallCrates.push_back(object);
				smallCrateResourceCount += object->getQuantity();
				if (smallCrateResourceCount >= resourceCount)
				{
					int count = resourceCount;
					std::vector<ResourceContainerObject *>::iterator crateIter = 
						smallCrates.begin(); 
					while (count > 0 && crateIter != smallCrates.end())
					{
						ResourceContainerObject * smallCrate = *crateIter;
						if (smallCrate->getQuantity() <= count)
						{
							count -= smallCrate->getQuantity();
							if (!smallCrate->removeResource(resourceTypeId, smallCrate->getQuantity(), &sources))
								return NetworkId::cms_invalid;
							// small crate has been deleted by the resource system
							*crateIter = nullptr;
							++crateIter;
						}
						else
						{
							if (!smallCrate->removeResource(resourceTypeId, count, &sources))
								return NetworkId::cms_invalid;
							count = 0;
						}
					}
					if (count > 0)
					{
						// WTF?!
						WARNING_STRICT_FATAL(true, ("ManufactureInstallationObject::transferResourceTypeToSchematic was pulling resources off small crates and ran out! This should be impossible!"));
						return NetworkId::cms_invalid;
					}
					foundIt = true;
				}
				else
					++iter;
			}
		}
		else
			++iter;
	}
	if (!foundIt)
		return NetworkId::cms_invalid;

	// xfer the resource data to the schematic
	for (std::vector<std::pair<NetworkId, int> >::const_iterator sourceIter =
		sources.begin(); sourceIter != sources.end(); ++sourceIter)
	{
		schematic.addIngredient(resourceTypeId, (*sourceIter).second, (*sourceIter).first);
	}
	return resourceTypeId;
}	// ManufactureInstallationObject::transferResourceTypeToSchematic

// ----------------------------------------------------------------------

/**
 * Returns a non-full crate currently in the output hopper we can stuff objects into
 *
 * @param contents		an object we want to put in the crate
 *
 * @return the crate, or nullptr if we have no crate
 */
FactoryObject * ManufactureInstallationObject::getCurrentCrate(const ManufactureSchematicObject & source)
{
	FactoryObject * crate = nullptr;

	ServerObject * outputHopper = getOutputHopper();
	if (outputHopper == nullptr)
		return nullptr;

	if (m_currentCrate.getObject() != nullptr)
	{
		crate = safe_cast<FactoryObject *>(m_currentCrate.getObject());

		// make sure the crate is still in our hopper and is not full
		if (crate != nullptr && ContainerInterface::getContainedByObject(*crate) == outputHopper &&
			crate->getCraftedId() == source.getOriginalId())
		{
			if (crate->getCount() < source.getItemsPerContainer())
				return crate;
			else
				return nullptr;
		}
	}

	// see if there is another non-full crate in the output hopper we could use
	const VolumeContainer * hopperContainer = ContainerInterface::getVolumeContainer(
		*outputHopper);
	if (hopperContainer != nullptr)
	{
		for (ContainerConstIterator iter = hopperContainer->begin();
			iter != hopperContainer->end(); ++iter)
		{
			crate = dynamic_cast<FactoryObject *>((*iter).getObject());
			if (crate != nullptr && crate->getCraftedId() == source.getOriginalId() && crate->getCount() < source.getItemsPerContainer())
			{
				// change our current crate to the one we found
				m_currentCrate = CachedNetworkId(*crate);
				return crate;
			}
		}
	}

	return nullptr;
}	// ManufactureInstallationObject::getCurrentCrate

// ----------------------------------------------------------------------

/**
 * Creates a new FactoryObject crate that we will stuff manufactured objects
 * into.
 *
 * @param contents		an object we want to put in the crate
 *
 * @return the new crate
 */
FactoryObject * ManufactureInstallationObject::makeNewCrate(const ManufactureSchematicObject & source)
{
	ServerObject * outputHopper = getOutputHopper();
	if (outputHopper == nullptr)
		return nullptr;

	// get the correct factory object template from the draft schematic
	const ManufactureSchematicObject * manfSchematic = getSchematic();
	if (manfSchematic == nullptr)
		return nullptr;
	const DraftSchematicObject * draftSchematic = DraftSchematicObject::getSchematic(
		manfSchematic->getDraftSchematic());
	if (draftSchematic == nullptr)
		return nullptr;

	ServerObject * object = nullptr;
	const ServerFactoryObjectTemplate * crateTemplate = draftSchematic->getCrateObjectTemplate();
	if (crateTemplate != nullptr)
	{
		object = ServerWorld::createNewObject(*crateTemplate, *outputHopper, false);
	}
	else
	{
		object = ServerWorld::createNewObject(DEFAULT_FACTORY_OBJECT_TEMPLATE,
			*outputHopper, false);
	}
	if (object == nullptr)
		return nullptr;

	FactoryObject * crate = dynamic_cast<FactoryObject *>(object);
	if (crate == nullptr)
		object->permanentlyDestroy(DeleteReasons::SetupFailed);
	else
	{
		crate->initialize(source);
		crate->setCraftedId(source.getOriginalId());
		crate->setOwnerId(getOwnerId());
		crate->setCreatorId(source.getCreatorId());
		crate->persist();
	}

	m_currentCrate = CachedNetworkId(*crate);
	LOG("CustomerService", ("Crafting:manf station %s owned by %s created crate %s", getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(getOwnerId()).c_str(), crate->getNetworkId().getValueString().c_str()));
	return crate;
}	// ManufactureInstallationObject::makeNewCrate


// ======================================================================
// ManufactureInstallationObject::TaskManufactureObject methods

/**
 * Class constructor.
 */
ManufactureInstallationObject::TaskManufactureObject::TaskManufactureObject(
	const CachedNetworkId & manfInstallation) :
	NonCriticalTaskQueue::TaskRequest(),
	m_manfInstallation(manfInstallation)
{
}	// TaskManufactureObject::TaskManufactureObject

//----------------------------------------------------------------------

/**
 * Class destructor.
 */
ManufactureInstallationObject::TaskManufactureObject::~TaskManufactureObject()
{
}	// TaskManufactureObject::~TaskManufactureObject

//----------------------------------------------------------------------

/**
 * Creates an object. Called when this task reaches the head of the queue.
 *
 * @return true on done, false if we need more time
 */
bool ManufactureInstallationObject::TaskManufactureObject::run()
{
	ManufactureInstallationObject * manfInst = safe_cast<ManufactureInstallationObject *>
		(m_manfInstallation.getObject());
	if (manfInst == nullptr || !manfInst->isActive())
		return true;

	if (!manfInst->createObject())
	{
		manfInst->setTickCount(0);	// to avoid recursion
		manfInst->deactivate();
	}

	return true;
}	// TaskManufactureObject::run

// ----------------------------------------------------------------------

void ManufactureInstallationObject::getAttributes(AttributeVector & data) const
{
	InstallationObject::getAttributes(data);

	ManufactureSchematicObject * schematic = getSchematic();
	if (schematic != nullptr)
	{
		char valueBuffer[32];
  		const size_t valueBuffer_size = sizeof (valueBuffer);

		if (!schematic->getAssignedObjectName().empty())
		{
			data.push_back(std::make_pair(SharedObjectAttributes::manufacture_object, 
				schematic->getAssignedObjectName()));
		}
		else
		{
			data.push_back(std::make_pair(SharedObjectAttributes::manufacture_object, 
				DraftSchematicObject::getSchematic(schematic->getDraftSchematic())->
				getObjectName()));
		}

		snprintf(valueBuffer, valueBuffer_size, "%i", static_cast<int>(getTimePerObject() + 0.5f));
		data.push_back(std::make_pair(SharedObjectAttributes::manufacture_time, Unicode::narrowToWide(valueBuffer)));

		snprintf(valueBuffer, valueBuffer_size, "%i", getNumNewItems());
		data.push_back(std::make_pair(SharedObjectAttributes::manufacture_count, Unicode::narrowToWide(valueBuffer)));
	}
}

// ----------------------------------------------------------------------

float ManufactureInstallationObject::alter(float time)
{
	{
		PROFILER_AUTO_BLOCK_DEFINE("ManufactureInstallationObject::alter");
		if (isAuthoritative() && isActive())
			harvest ();
	}

	const float result = InstallationObject::alter (time);
	if (result != AlterResult::cms_kill && isAuthoritative() && isActive())  //lint !e777 // != on floats
	{
		// we don't need to alter until the next object is ready to be made
		// (plus a little bit to make sure we are definitely >= the time)
		return getTimePerObject() + 0.1f;
	}
	return result;
}

// ======================================================================
