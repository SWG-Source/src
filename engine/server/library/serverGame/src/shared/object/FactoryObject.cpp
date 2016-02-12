//========================================================================
//
// FactoryObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/FactoryObject.h"

#include "serverGame/Chat.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/DraftSchematicObject.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ProsePackageManagerServer.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerFactoryObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/TangibleController.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedObject/VolumeContainmentProperty.h"
#include "UnicodeUtils.h"


//----------------------------------------------------------------------

//const static ConstCharCrcLowerString OBJECT_SLOT_NAME("anythingnomod1");
//const static ConstCharCrcLowerString COMPONENT_SLOT_NAME("anythingnomod2");

const static std::string OBJVAR_OBJECT_LIST("factory.objects");
const static std::string OBJVAR_PENDING_LIST("factory.pending");
const static std::string OBJVAR_SHARED_TEMPLATE("factory.sharedTemplate");
const static std::string OBJVAR_ATTRIBUTES("crafting_attributes");
const static std::string OBJVAR_DRAFT_SCHEMATIC("draftSchematic");
const static std::string OBJVAR_APPEARANCE_DATA("appearanceData");
const static std::string OBJVAR_CUSTOM_APPEARANCE("customAppearance");

static const StringId STRING_ID_CANT_DELETE("system_msg", "cant_delete_crafting_factory");
static const StringId STRING_ID_CANT_SPLIT("system_msg", "cant_split_crate");


//----------------------------------------------------------------------

const SharedObjectTemplate * FactoryObject::m_defaultSharedTemplate = nullptr;


//-----------------------------------------------------------------------

FactoryObject::FactoryObject(const ServerFactoryObjectTemplate* newTemplate) :
	TangibleObject(newTemplate),
	m_badFactoryLogged(false),
	m_attributes(),
	m_craftingSchematic(),
	m_craftingCount(0)
{
	addMembersToPackages();
}

//-----------------------------------------------------------------------

FactoryObject::~FactoryObject()
{
}

//-----------------------------------------------------------------------

/**
 * Initializes the factory for the type of object it will be storing.
 *
 * @param source		the schematic the objects in this factory are created from
 */
void FactoryObject::initialize(const ManufactureSchematicObject & source)
{
static const std::string _noCopyList[] = {
	"crafting",
	"crafting_attributes",
	"ingr",
	"item_attrib_keys",
	"item_attrib_values",
	"crafting_resource"
	// @todo: it would be nice if we could get rid of crafting_components,
	// but some scripts are incorrectly relying on that objvar list existing
	};
static const int _noCopyList_size = sizeof(_noCopyList) / sizeof(_noCopyList[0]);
static const std::set<std::string> noCopyList(&_noCopyList[0], &_noCopyList[_noCopyList_size]);

	int temp = source.getDraftSchematic();
	setObjVarItem(OBJVAR_DRAFT_SCHEMATIC, temp);
	if (!source.getAppearanceData().empty())
		setObjVarItem(OBJVAR_APPEARANCE_DATA, source.getAppearanceData());

	if (!source.getCustomAppearance().empty())
	{
		setObjVarItem(OBJVAR_CUSTOM_APPEARANCE, source.getCustomAppearance());
	}

	if (!source.getAssignedObjectName().empty())
		setObjectName(source.getAssignedObjectName());

	// copy the schematic attribute data
	const AttribMap & attributes = source.getAttributes();
	for (AttribMap::const_iterator iter = attributes.begin(); iter !=
		attributes.end(); ++iter)
	{
		setAttribute((*iter).first, (*iter).second);
	}

	// copy objvars on the schematic, except for ones we know we don't need
	std::string name;
	const DynamicVariableList & list = source.getObjVars();
	for (DynamicVariableList::MapType::const_iterator iter2 = list.begin();
		iter2 != list.end(); ++iter2)
	{
		const std::string & fullName = (*iter2).first;
		const std::string::size_type dot = fullName.find('.');
		name = fullName.substr(0, dot);
		if (noCopyList.find(name) == noCopyList.end())
			copyObjVars(name, source, name);
	}

	// copy the manufacture scripts
	const ScriptList & sourceScripts = source.getScriptObject()->getScripts();
	for (ScriptList::const_iterator it = sourceScripts.begin(); it != sourceScripts.end(); ++it)
		getScriptObject()->attachScript((*it).getScriptName(), false);
}	// FactoryObject::initialize

//-----------------------------------------------------------------------

/**
 * Initializes a newly created object.
 */
void FactoryObject::initializeFirstTimeObject()
{
	TangibleObject::initializeFirstTimeObject();
}	// FactoryObject::initializeFirstTimeObject

//-----------------------------------------------------------------------

/**
 * Initializes an object just proxied.
 */
void FactoryObject::endBaselines()
{
	TangibleObject::endBaselines();

	if (isAuthoritative())
		calculateAttributes();
}	// FactoryObject::endBaselines

//-----------------------------------------------------------------------

/**
 * Signal that we have just been loaded from the database.
 */
void FactoryObject::onLoadedFromDatabase()
{
	TangibleObject::onLoadedFromDatabase();

	if (getLoadContents() && getCount() > 0)
	{
		// verify that we have a contained object
		if (getContainedObject() == nullptr)
		{
			WARNING_STRICT_FATAL(true, ("Factory object %s has a count of %d, "
				"but no contained object! We are setting the count to 0.", 
				getNetworkId().getValueString().c_str(), getCount()));
			setCount(0);
		}
	}
}	// FactoryObject::onLoadedFromDatabase

//-----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * FactoryObject::getDefaultSharedTemplate() const
{
	static const ConstCharCrcLowerString templateName("object/factory/base/shared_factory_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "FactoryObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// FactoryObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void FactoryObject::removeDefaultTemplate()
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// FactoryObject::removeDefaultTemplate

//-----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
Controller* FactoryObject::createDefaultController()
{
	Controller* controller = new TangibleController(this);

	setController(controller);
	return controller;
}	// FactoryObject::createDefaultController

// ----------------------------------------------------------------------

/**
 * Sets the item attributes from the factory's current objvars.
 */
void FactoryObject::calculateAttributes()
{
	if (isAuthoritative())
	{
		m_attributes.clear();
		
		const DynamicVariableList::NestedList attributes(getObjVars(), OBJVAR_ATTRIBUTES);
		for (DynamicVariableList::NestedList::const_iterator i = attributes.begin();
			i != attributes.end(); ++i)
		{
			float value;
			i.getValue(value);
			m_attributes.set(StringId(i.getName()), value);
		}
	}
	else
	{
		WARNING_STRICT_FATAL(true, ("FactoryObject::calculateAttributes called on "
			"non-auth object %s", getNetworkId().getValueString().c_str()));
	}
}	// FactoryObject::calculateAttributes

// ----------------------------------------------------------------------

/**
 * Tests if this factory can be used in the game. If its source schematic has
 * been removed we can no longer use this item to make new objects or in 
 * crafting/manufactuing.
 *
 * @return true if the factory is useable, false if not
 */
bool FactoryObject::isFactoryOk() const
{
	// if we haven't been initialized, assume we are ok
	if (!getObjVars().hasItem(OBJVAR_DRAFT_SCHEMATIC))
		return true;

	if (m_badFactoryLogged)
		return false;

	bool factoryOk = true;
	if (getLoadContents())
	{
		static const int BUFSIZE = 1024;
		char errBuffer[BUFSIZE];
		const DraftSchematicObject * draft = DraftSchematicObject::getSchematic(
			getDraftSchematic());
		if (draft != nullptr)
		{
			// make sure we have an object instance defined
			if (getCount() > 0 && getContainedObject() == nullptr)
			{
				sprintf(errBuffer, "not having a contained object instance");
				factoryOk = false;
			}
		}
		else
		{
			sprintf(errBuffer, "its source draft schematic (crc=%lu) not existing", 
				getDraftSchematic());
			factoryOk = false;
		}
		if (!factoryOk && !m_badFactoryLogged)
		{
			// send out logs/emails
			m_badFactoryLogged = true;
			const ServerObject * owner = nullptr;
			const Object * object = NetworkIdManager::getObjectById(getOwnerId());
			if (object != nullptr)
				owner = object->asServerObject();

			// log that the schematic can't be used
			LOG("CustomerService", ("Crafting: factory crate object %s "
				"owned by %s is unuseable due to %s", getNetworkId().getValueString().c_str(), 
				PlayerObject::getAccountDescription(owner).c_str(), errBuffer));
			if (owner != nullptr)
			{
				// send mail to the owner telling them their schematic can't be used
				const static StringId message("system_msg", "crate_unuseable");
				Unicode::String oob;
				ProsePackage pp;
				ProsePackageManagerServer::createSimpleProsePackage (*this, message, pp);
				OutOfBandPackager::pack(pp, -1, oob);
				Unicode::String messageString = Unicode::narrowToWide("@" + message.getCanonicalRepresentation());
				Chat::sendPersistentMessage("", Unicode::wideToNarrow(owner->getObjectName()), 
					messageString, Unicode::emptyString, oob);
			}
		}
	}
	return factoryOk;
}	// FactoryObject::isFactoryOk

// ----------------------------------------------------------------------

/**
 * Called if we are being deleted. If we are being used in a crafting session,
 * prevent the deletion.
 */
bool FactoryObject::canDestroy() const
{
	if (inCraftingSession() && m_craftingCount.get() != 0)
	{
		Unicode::String oob;
		{
			ProsePackage pp;
			ProsePackageManagerServer::createSimpleProsePackage (*this, 
				STRING_ID_CANT_DELETE, pp);
			OutOfBandPackager::pack(pp, -1, oob);
		}
		const Object * const owner = NetworkIdManager::getObjectById(getOwnerId());
		if (owner != nullptr)
		{
			Chat::sendSystemMessage(*(owner->asServerObject()), Unicode::emptyString, oob);
		}
		return false;
	}
	return TangibleObject::canDestroy();
}	// FactoryObject::canDestroy

/**
 * Called when the factory is transferred to another container. If it is being used
 * in a crafting session, we create another factory for component items to reference.
 */
void FactoryObject::onContainerTransfer(ServerObject * destination,
	ServerObject* transferer)
{
	if (inCraftingSession() && destination != nullptr &&
		destination->getNetworkId() != m_craftingSchematic.get())
	{
		if (!isFactoryOk())
			return;

		Object * schematic = m_craftingSchematic.get().getObject();
		if (schematic == nullptr)
		{
			WARNING(true, ("FactoryObject::onContainerTransfer: could not get the schematic for factory %s", getNetworkId().getValueString().c_str()));
			return;
		}
		if (ContainerInterface::getTopmostContainer(*this) != 
			ContainerInterface::getTopmostContainer(*schematic))
		{
			// if we're being used in crafting, and not being transferred to/from 
			// the schematic, we need to make a new factory in the schematic for 
			// our components to reference
			if (m_craftingCount.get() != 0)
			{
				FactoryObject * newFactory = nullptr;
				if (getCount() > 1)
				{
					// make a new factory in the manf schematic
					newFactory = makeCopy(*(m_craftingSchematic.get().getObject()->
						asServerObject()), 1);
					if (newFactory != nullptr)
					{
						// NOTE: potential dupe here, but the player shouldn't be able to
						// remove the new factory without decreasing the component count
						// in the manf schematic
						setCount(getCount() + 1);

						// swap the contained item instances so that any current references
						// will point to a local object
						TangibleObject * myObject = const_cast<TangibleObject *>(getContainedObject());
						if (myObject == nullptr)
						{
							WARNING(true, ("FactoryObject::onContainerTransfer: getContainedObject for %s returned nullptr", getNetworkId().getValueString().c_str()));
							return;
						}
						TangibleObject * newObject = const_cast<TangibleObject *>(newFactory->getContainedObject());
						if (newObject == nullptr)
						{
							WARNING(true, ("FactoryObject::onContainerTransfer: getContainedObject for %s returned nullptr", newFactory->getNetworkId().getValueString().c_str()));
							newFactory->permanentlyDestroy(DeleteReasons::SetupFailed);
							return;
						}
						Container::ContainerErrorCode error;
						if (!ContainerInterface::transferItemToVolumeContainer (*newFactory, *myObject, nullptr, error))
						{
							WARNING(true, ("FactoryObject::onContainerTransfer: could not transfer factory %s to container %s", newFactory->getNetworkId().getValueString().c_str(), myObject->getNetworkId().getValueString().c_str()));
							newFactory->permanentlyDestroy(DeleteReasons::SetupFailed);
							return;
						}
						if (!ContainerInterface::transferItemToVolumeContainer (*this, *newObject, nullptr, error))
						{
							WARNING(true, ("FactoryObject::onContainerTransfer: could not transfer factory %s to container %s", getNetworkId().getValueString().c_str(), newObject->getNetworkId().getValueString().c_str()));
							return;
						}
					}
				}
				else
				{
					// make a new factory with one item, but don't destroy ourself
					newFactory = makeCopy(*(m_craftingSchematic.get().getObject()->
						asServerObject()), 1, false);
					if (newFactory == nullptr)
					{
						WARNING(true, ("FactoryObject::onContainerTransfer: could not make a copy of factory %s", getNetworkId().getValueString().c_str()));
						return;
					}
					// make a new object for ourself
					// NOTE: potential dupe here, but the player shouldn't be able to
					// remove the new factory without decreasing the component count
					// in the manf schematic
					addObject();
				}
				// update the crafting status of ourself and the new factory
				ManufactureSchematicObject * schematic = safe_cast<ManufactureSchematicObject *>(
					m_craftingSchematic.get().getObject());
				if (schematic != nullptr)
				{
					if (newFactory != nullptr)
					{
						// set up the new factory to have the same crafting info
						// that we do
						newFactory->m_craftingSchematic = m_craftingSchematic;
						newFactory->m_craftingCount = m_craftingCount.get();
						schematic->addCraftingFactory(*newFactory);
					}
					schematic->removeCraftingFactory(*this);
				}
			}
			endCraftingSession();
		}
	}
	TangibleObject::onContainerTransfer(destination, transferer);
}	// FactoryObject::onContainerTransfer

// ----------------------------------------------------------------------

/**
 * Tells the factory that we are using it in a crafting session.
 */
bool FactoryObject::startCraftingSession(ManufactureSchematicObject & schematic)
{
	if (inCraftingSession() || getCount() <= 1 || !isFactoryOk())
		return false;

	m_craftingSchematic = CachedNetworkId(schematic);
	m_craftingCount = 0;
	schematic.addCraftingFactory(*this);
	return true;
}	// FactoryObject::startCraftingSession

// ----------------------------------------------------------------------

/**
 * Tells the factory that we have stopped using it in a crafting session.
 */
bool FactoryObject::endCraftingSession()
{
	if (!inCraftingSession())
		return false;

	m_craftingSchematic = CachedNetworkId::cms_cachedInvalid;
	m_craftingCount = 0;
	return true;
}	// FactoryObject::endCraftingSession

// ----------------------------------------------------------------------

/**
 * Tells the factory to reset its object count for items used during this
 * crafting session.
 */
bool FactoryObject::resetCraftingSession()
{
	if (!inCraftingSession())
		return false;

	incrementCount(m_craftingCount.get());
	m_craftingCount = 0;
	return true;
}	// FactoryObject::resetCraftingSession

// ----------------------------------------------------------------------

/**
 * Returns if the factory thinks it is in a crafting session or not.
 */
bool FactoryObject::inCraftingSession() const
{
	if (m_craftingSchematic.get() == CachedNetworkId::cms_cachedInvalid)
		return false;

	// make sure the crafting schematic id is a valid object
	if (m_craftingSchematic.get().getObject() == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("FactoryObject %s has a schematic id of %s "
			"that isn't a valid object!!!", getNetworkId().getValueString().c_str(),
			m_craftingSchematic.get().getValueString().c_str()));
		const_cast<FactoryObject*>(this)->m_craftingSchematic = CachedNetworkId::cms_cachedInvalid;
		const_cast<FactoryObject*>(this)->m_craftingCount = 0;
		return false;
	}

	return true;
}	// FactoryObject::inCraftingSession

// ----------------------------------------------------------------------

/**
 * Tells the factory to act as if items were removed from it to be used in a
 * crafting session. We can remove all but one of our items. Removing the last
 * item should be done by moving this factory to the manf schematic.
 *
 * @param count		the number of items to remove
 *
 * @return true on success, false if the items weren't removed
 */
bool FactoryObject::removeCraftingReferences(int count)
{
	if (!inCraftingSession() || count <= 0 || count >= getCount())
		return false;

	incrementCount(-count);
	m_craftingCount.set(m_craftingCount.get() + count);
	return true;
}	// FactoryObject::removeCraftingReferences

// ----------------------------------------------------------------------

/**
 * Tells the factory to add back crafting references that had been removed from
 * it.
 *
 * @param count		the number of items to add
 *
 * @return true on success, false if the items weren't added
 */
bool FactoryObject::addCraftingReferences(int count)
{
	if (!inCraftingSession() || count <= 0 || count > m_craftingCount.get())
		return false;

	incrementCount(count);
	m_craftingCount.set(m_craftingCount.get() - count);
	return true;
}	// FactoryObject::addCraftingReferences

// ----------------------------------------------------------------------

/**
 * If the factory is empty, makes a new object and stores it in the factory.
 * Otherwise just increases the count for the number of objects the factory
 * contains.
 *
 * @return true if the object was added, false if not
 */
bool FactoryObject::addObject()
{
	if (!isFactoryOk())
		return false;

	if (isAuthoritative())
	{
		if (getCount() == 0)
		{
			TangibleObject * object = manufactureObject();
			if (object != nullptr)
			{
				setObjectName(object->getObjectName());
				setComplexity(object->getComplexity());

				incrementCount(1);
			}
			else
				return false;
		}
		else
		{
			// just increment the count of the number of items we are storing
			incrementCount(1);
		}
		
		const ServerObject * parent = safe_cast<const ServerObject *>(ContainerInterface::getFirstParentInWorld(*this));
		if (parent != nullptr)
			LOG("CustomerService", ("Crafting:incrementing count of crate %s (owner %s) to %d", getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(parent->getOwnerId()).c_str(), getCount()));
		else
			LOG("CustomerService", ("Crafting:incrementing count of crate %s to %d", getNetworkId().getValueString().c_str(), getCount()));
	}
	else
	{
		WARNING_STRICT_FATAL(true, ("Factory object %s addObject called for "
			"non-authoritative factory", getNetworkId().getValueString().c_str()));
	}
	return true;
}	// FactoryObject::addObject


// ----------------------------------------------------------------------

/**
 * Removes an object from this factory, in order for it to be placed in the game.
 *
 * @param destination		where to transfer the object once it has been loaded
 *
 * @return true if the request to get the item was sent to the database, false on error
 *
 * @todo: do we need to add a count of items to remove?
 */
bool FactoryObject::removeObject(ServerObject & destination)
{
	if (!isFactoryOk())
		return false;

	if (getCount() > 0 && ContainerInterface::getVolumeContainer(destination) != nullptr)
	{
		if (!getLoadContents())
		{
			// old style factory
			// request the object from the database
			NetworkId objectId(removeIdFromObjectList());
			if (objectId != NetworkId::cms_invalid)
			{
				loadContainedObjectFromDB(objectId);
				incrementCount(-1);

				// add the object to a pending list
				addIdToPendingList(objectId, destination);
				return true;
			}
		}
		else
		{
			// new style factory
			TangibleObject * object = nullptr;
			if (getCount() > 1)
			{
				object = manufactureObject();
			}
			else
			{
				// use the 1st object that was created for the factory
				// (we should actually be a slotted container, but we have to
				// keep backward compatable)
				const VolumeContainer * container = ContainerInterface::getVolumeContainer(*this);
				if (container->getNumberOfItems() > 0)
				{
					if (container->getNumberOfItems() > 1)
					{
						WARNING_STRICT_FATAL(true, ("FactoryObject::removeObject "
							"factory %s contains more than one item",
							getNetworkId().getValueString().c_str()));
					}
					const CachedNetworkId & id = *(container->begin());
					Object * obj = id.getObject();
					if (obj != nullptr && obj->asServerObject()->asTangibleObject() != nullptr)
					{
						object = obj->asServerObject()->asTangibleObject();
					}
					else
					{
						WARNING_STRICT_FATAL(true, ("FactoryObject::removeObject "
							"factory %s contains and invalid object",
							getNetworkId().getValueString().c_str()));
						object = manufactureObject();
					}
				}
				else
				{
					WARNING_STRICT_FATAL(true, ("FactoryObject::removeObject "
						"factory %s contains no objects, but the count is %d",
						getNetworkId().getValueString().c_str(), getCount()));
					object = manufactureObject();
				}
			}
			if (object != nullptr)
			{
				Container::ContainerErrorCode error;
				if (ContainerInterface::transferItemToVolumeContainer(destination, *object, nullptr, error))
				{
					incrementCount(-1);

					const ServerObject * parent = safe_cast<const ServerObject *>(ContainerInterface::getFirstParentInWorld(destination));
					if (parent != nullptr)
						LOG("CustomerService", ("Crafting:removed object %s from crate %s to container %s of player %s. Crate count = %d", object->getNetworkId().getValueString().c_str(), getNetworkId().getValueString().c_str(), destination.getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(parent->getNetworkId()).c_str(), getCount()));
					else
						LOG("CustomerService", ("Crafting:removed object %s from crate %s to container %s. Crate count = %d", object->getNetworkId().getValueString().c_str(), getNetworkId().getValueString().c_str(), destination.getNetworkId().getValueString().c_str(), getCount()));
					if (getCount() == 0)
					{
						LOG("CustomerService", ("Crafting:destroying empty crate %s owned by %s", getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(getOwnerId()).c_str()));
						permanentlyDestroy(DeleteReasons::Consumed);
					}
					return true;
				}
				else
				{
					object->permanentlyDestroy(DeleteReasons::BadContainerTransfer);
				}
			}
		}
	}
	return false;
}	// FactoryObject::removeObject

// ----------------------------------------------------------------------

/**
 * Destroys some of the contents of the factory.
 *
 * @param count		the number of items to destroy
 *
 * @return the number of items actually destroyed
 */
int FactoryObject::deleteContents(int count)
{
	if (count > getCount())
		count = getCount();

	incrementCount(-count);
	if (getCount() == 0)
		permanentlyDestroy(DeleteReasons::Consumed);

	return count;
}	// FactoryObject::deleteContents

// ----------------------------------------------------------------------

/**
 * Tansfers objects in this crate to another crate.
 *
 * @param toCrate		the crate to transfer to
 * @param objectCount	the number of objects to transfer
 *
 * @return true on success, false on error
 */
bool FactoryObject::transferObjects(FactoryObject & toCrate, int objectCount)
{
	if (!isFactoryOk())
		return false;

	// verify the object count
	if (objectCount < 0 || objectCount > getCount())
		return false;

	if (objectCount == 0)
		return true;

	// verify the to crate
	if (toCrate.getCraftedId() != getCraftedId())
		return false;

	for (int i = 0; i < objectCount; ++i)
	{
		NetworkId objectId(removeIdFromObjectList());
		if (objectId != NetworkId::cms_invalid)
		{
			incrementCount(-1);
			// @todo: can we tell the db to change the owner of the object
			toCrate.addIdToObjectList(objectId);
			toCrate.incrementCount(1);
		}
		else
		{
			WARNING_STRICT_FATAL(true, ("FactoryObject::transferObjects factory "
				"%s failed to get an object from its list even though the object "
				"count said we should be able to!", getNetworkId().getValueString().c_str()));
			// transfer all the objects back to us
			for (int j = 0; j < i; ++j)
			{
				NetworkId objectId(toCrate.removeIdFromObjectList());
				FATAL(objectId == NetworkId::cms_invalid, ("FactoryObject::transferObjects "
					"trying to recover from prevoius error, couldn't get our "
					"objects back!!!!"));
				addIdToObjectList(objectId);
				toCrate.incrementCount(-1);
				incrementCount(-1);
			}
			return false;
		}
	}
	LOG("CustomerService", ("Crafting:transferred %d objects from crate %s owned by %s to crate %s owned by %s. Counts = %d, %d", objectCount, getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(getOwnerId()).c_str(), toCrate.getNetworkId().getValueString().c_str(), PlayerObject::getAccountDescription(toCrate.getOwnerId()).c_str(), getCount(), toCrate.getCount()));
	return true;
}	// FactoryObject::transferObjects

// ----------------------------------------------------------------------

/**
 * Called when an object has been added to our container from the database. If
 * the object is one that was requested by removeObject(), transfer the object
 * to the destination.
 */
void FactoryObject::onContainedObjectLoaded(const NetworkId &oid)
{
	TangibleObject::onContainedObjectLoaded(oid);

	// check if the object is on our pending list
	ServerObject * destination = removeIdFromPendingList(oid);
	if (destination != nullptr)
	{
		TangibleObject * item = safe_cast<TangibleObject *>(NetworkIdManager::getObjectById(
			oid));
		if (item != nullptr)
		{
			Container::ContainerErrorCode error = Container::CEC_Success;
			if (ContainerInterface::transferItemToVolumeContainer(*destination, *item, nullptr, error))
			{
				// if that was the last item of ours, delete ourself
				if (getCount() == 0 && getPendingListCount() == 0)
					permanentlyDestroy(DeleteReasons::Consumed);
			}
			else
			{
				// re-depersist the object
				addIdToObjectList(oid);
				incrementCount(1);
				item->unload();
				// tell the owner something went wrong
				Object * owner = NetworkIdManager::getObjectById(getOwnerId());
				if (owner != nullptr)
				{
					ContainerInterface::sendContainerMessageToClient(*safe_cast<ServerObject*>(owner), error);
				}
			}
		}
		else
		{
			WARNING(true, ("FactoryObject::onContainedObjectLoaded factory %s received object %s which is not on the server",
				getNetworkId().getValueString().c_str(), oid.getValueString().c_str()));
			// add the object back for the moment
			addIdToObjectList(oid);
			incrementCount(1);
		}
	}
}	// FactoryObject::onContainedObjectLoaded

// ----------------------------------------------------------------------

/**
 * Creates a new object from the factory. Note this essentially duplicates the
 * functionality of ManufactureSchematicObject::manufactureObject; if that
 * function changes, this one should too.
 *
 * @return the new object, or nullptr on error
 */
TangibleObject * FactoryObject::manufactureObject()
{
	if (!isFactoryOk())
		return nullptr;

	const DraftSchematicObject * draftSchematic = DraftSchematicObject::getSchematic(getDraftSchematic());

	if (draftSchematic == nullptr)
	{
		return nullptr;
	}
	if (draftSchematic->getCraftedObjectTemplate() == nullptr)
	{
		DEBUG_WARNING(true, ("Draft schematic %s does not have a craftable object template!\n", draftSchematic->getTemplateName()));
		return nullptr;
	}

	// create the item
	TangibleObject * const object = dynamic_cast<TangibleObject *>(
		ServerWorld::createNewObject(*draftSchematic->getCraftedObjectTemplate(),
		*this, false));
	if (object == nullptr)
	{
		DEBUG_WARNING(true, ("Player %s failed to create object for template "
			"%s.\n", getOwnerId().getValueString().c_str(),
			draftSchematic->getCraftedObjectTemplate()->getName()));
		return nullptr;
	}

	if (!getAssignedObjectName().empty())
		object->setObjectName(getAssignedObjectName());

	// set the object appearance
	const std::string & appearance = getItemCustomAppearance();
	if (!appearance.empty())
		object->setCustomAppearance(appearance);

	// set the customization data for the object
	if (!getAppearanceData().empty())
		object->setAppearanceData(getAppearanceData());

	// mark the item as having been created by us
	object->setCraftedId(getCraftedId());
	object->setCraftedType(draftSchematic->getCategory());
	object->setOwnerId(getOwnerId());
	if (getCreatorId() != NetworkId::cms_invalid)
		object->setCreatorId(getCreatorId());
	else
		object->setCreatorId(getOwnerId());

	// set the creator xp type from the schematic template
	const ServerObjectTemplate * schematicTemplate = safe_cast<const
		ServerObjectTemplate *>(draftSchematic->getObjectTemplate());
	if (schematicTemplate != nullptr && schematicTemplate->getXpPointsCount() > 0)
	{
		ServerObjectTemplate::Xp xpData;
		schematicTemplate->getXpPoints(xpData, 0);
		object->setCreatorXpType(xpData.type);
	}

	// allow the schematic scripts to modify the object
	// prototype objects are modified at the end of the crafting session
	ScriptParams params;
	params.addParam(getOwnerId());
	params.addParam(object->getNetworkId());
	params.addParam(*this);
	params.addParam(false);
	params.addParam(true);
	int result = getScriptObject()->trigAllScripts(Scripting::TRIG_MANUFACTURE_OBJECT,
		params);
	if (result == SCRIPT_OVERRIDE)
	{
		object->permanentlyDestroy(DeleteReasons::Script);
		return nullptr;
	}

	return object;
}	// FactoryObject::manufactureObject

// ----------------------------------------------------------------------

/**
 * Returns the of our content's template.
 *
 * @return the template name
 */
const char * FactoryObject::getContainedTemplateName() const
{
	if (getLoadContents() && getCount() > 0)
	{
		const VolumeContainer * container = ContainerInterface::getVolumeContainer(*this);
		const CachedNetworkId & id = *(container->begin());
		const Object * obj = id.getObject();
		if (obj != nullptr && obj->asServerObject() != nullptr)
			return obj->asServerObject()->getTemplateName();
	}
	return nullptr;
}	// FactoryObject::getContainedTemplateName

// ----------------------------------------------------------------------

/**
 * Returns the name of our content's shared template.
 *
 * @return the shared template name
 */
const char * FactoryObject::getContainedSharedTemplateName() const
{
static std::string sharedTemplateName;

	if (!getLoadContents())
	{
		// old style factory
		if (getObjVars().getItem(OBJVAR_SHARED_TEMPLATE, sharedTemplateName))
			return sharedTemplateName.c_str();
	}
	else if (getCount() > 0)
	{
		// new style factory
		const VolumeContainer * container = ContainerInterface::getVolumeContainer(*this);
		const CachedNetworkId & id = *(container->begin());
		const Object * obj = id.getObject();
		if (obj != nullptr && obj->asServerObject() != nullptr)
			return obj->asServerObject()->getSharedTemplateName();
	}
	return nullptr;
}	// FactoryObject::getContainedSharedTemplateName

// ----------------------------------------------------------------------

/**
 * Returns the template for our contents.
 *
 * @return the content template
 */
const ObjectTemplate * FactoryObject::getContainedObjectTemplate() const
{
	if (getLoadContents() && getCount() > 0)
	{
		const VolumeContainer * container = ContainerInterface::getVolumeContainer(*this);
		const CachedNetworkId & id = *(container->begin());
		const Object * obj = id.getObject();
		if (obj != nullptr && obj->asServerObject() != nullptr)
			return obj->asServerObject()->getObjectTemplate();
	}
	return nullptr;
}	// FactoryObject::getContainedObjectTemplate

// ----------------------------------------------------------------------

/**
 * Returns our contained object.
 *
 * @return the contained object
 */
const TangibleObject * FactoryObject::getContainedObject() const
{
	if (getLoadContents() && getCount() > 0)
	{
		const VolumeContainer * container = ContainerInterface::getVolumeContainer(*this);
		const CachedNetworkId & id = *(container->begin());
		const Object * obj = id.getObject();
		if (obj != nullptr && obj->asServerObject() != nullptr)
		{
			return obj->asServerObject()->asTangibleObject();
		}
	}
	return nullptr;
}	// FactoryObject::getContainedObject

// ----------------------------------------------------------------------

/**
 * Adds an object id to our id list.
 *
 * @param id		the id to add
 */
void FactoryObject::addIdToObjectList(const NetworkId & id)
{
	setObjVarItem(OBJVAR_OBJECT_LIST + "." + id.getValueString(), id);
}	// FactoryObject::addIdToObjectList

// ----------------------------------------------------------------------

/**
 * Removes an id from our id list.
 *
 * @return the id that was removed
 */
const NetworkId FactoryObject::removeIdFromObjectList()
{
	DynamicVariableList::NestedList objectsList(getObjVars(),OBJVAR_OBJECT_LIST);

	if (objectsList.empty())
		return NetworkId::cms_invalid;

	DynamicVariableList::NestedList::const_iterator item = objectsList.begin();
	NetworkId itemId;
	item.getValue(itemId);
	removeObjVarItem(item.getNameWithPath());

	return itemId;
}	// FactoryObject::removeIdFromObjectList

// ----------------------------------------------------------------------

/**
 * Adds an object id to our pending list.
 *
 * @param id			the id to add
 * @param destination	where to xfer the object once it is loaded
 */
void FactoryObject::addIdToPendingList(const NetworkId & id, const ServerObject & destination)
{
	setObjVarItem(OBJVAR_PENDING_LIST + "." + id.getValueString(), destination.getNetworkId());
}	// FactoryObject::addIdToPendingList

// ----------------------------------------------------------------------

/**
 * Removes an id from our pending list.
 *
 * @param id		the id to remove
 *
 * @return the container object associated with the id
 */
ServerObject * FactoryObject::removeIdFromPendingList(const NetworkId & id)
{
	const DynamicVariableList::NestedList pendingList(getObjVars(),OBJVAR_PENDING_LIST);

	const std::string & idString = id.getValueString();

	NetworkId objectId;
	if (!pendingList.getItem(idString,objectId))
		return nullptr;

	ServerObject * const object = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(objectId));
	removeObjVarItem(pendingList.getContextName() + idString);
	return object;
}	// FactoryObject::removeIdFromPendingList

// ----------------------------------------------------------------------

/**
 * Gets the number of objects in our object list.
 *
 * @return the list count
 */
int FactoryObject::getObjectListCount() const
{
	return DynamicVariableList::NestedList(getObjVars(),OBJVAR_OBJECT_LIST).getCount();
}	// FactoryObject::getObjectListCount

// ----------------------------------------------------------------------

/**
 * Gets the number of objects in our pending list.
 *
 * @return the list count
 */
int FactoryObject::getPendingListCount() const
{
	return DynamicVariableList::NestedList(getObjVars(),OBJVAR_PENDING_LIST).getCount();
}	// FactoryObject::getPendingListCount

// ----------------------------------------------------------------------

const std::string & FactoryObject::getAppearanceData() const
{
	static const std::string emptyString;
	static std::string result;

	if (!getObjVars().getItem(OBJVAR_APPEARANCE_DATA,result))
		return emptyString;

	return result;
}

// ----------------------------------------------------------------------

/**
 * Returns the custom appearance objects made by this schematic will use.
 *
 * @return the appearance filename
 */
const std::string & FactoryObject::getItemCustomAppearance() const
{
	static const std::string emptyString;
	static std::string result;

	if (!getObjVars().getItem(OBJVAR_CUSTOM_APPEARANCE,result))
		return emptyString;

	return result;
}	// FactoryObject::getItemCustomAppearance

//-----------------------------------------------------------------------

/**
 * Returns the draft schematic name crc for the schematic used to define this
 * schematic.
 *
 * @return the schematic crc
 */
uint32 FactoryObject::getDraftSchematic() const
{
	int result = 0;
	getObjVars().getItem(OBJVAR_DRAFT_SCHEMATIC, result);
	return result;
}	// FactoryObject::getDraftSchematic

//-----------------------------------------------------------------------

/**
 * Returns an ingredient slot needed to manufacture an item with this schematic.
 *
 * @param index				slot to get
 * @param data				structure to be filled in with the slot data
 * @param creatingObject	flag that we are getting the slot in order to create
 *							a crafted object; should always be true
 *
 * @return true on success, false if the index was out of range
 */
bool FactoryObject::getSlot(int index, Crafting::IngredientSlot & data, bool creatingObject) const
{
	UNREF(index);
	UNREF(data);
	UNREF(creatingObject);

	WARNING_STRICT_FATAL(true, ("FactoryObject::getSlot called for object %s. This should never happen!",
		getNetworkId().getValueString().c_str()));
	return false;
}	// FactoryObject::getSlot

//-----------------------------------------------------------------------

/**
 * Returns the number of ingredient slots needed to manufacture an item with
 * this schematic.
 *
 * @return the ingredient count (always 0, since we don't need slot data)
 */
int FactoryObject::getSlotsCount() const
{
	return 0;
}	// FactoryObject::getSlotsCount

//-----------------------------------------------------------------------

/**
 * Returns the list of attributes for the object this factory makes.
 */
const std::map<StringId, float> & FactoryObject::getAttributes() const
{
	return m_attributes.getMap();
}	// FactoryObject::getAttributes

//-----------------------------------------------------------------------

/**
 * Sets the value for an attribute of the schematic
 *
 * @param attribute		the attribute data
 */
void FactoryObject::setAttribute(const ServerIntangibleObjectTemplate::SchematicAttribute & attribute)
{
	setAttribute(attribute.name, static_cast<float>(attribute.value));
}	// FactoryObject::setAttribute

//-----------------------------------------------------------------------

/**
 * Sets the value for an attribute of the schematic
 *
 * @param name		the attribute name
 * @param value		the attribute value
 */
void FactoryObject::setAttribute(const StringId & name, float value)
{
static const StringId XP_ATTRIB("crafting", "xp");

	if (isAuthoritative())
	{
		if (name == XP_ATTRIB)
			value = 0;
		m_attributes.set(name, value);
		setObjVarItem(OBJVAR_ATTRIBUTES + "." + name.getCanonicalRepresentation(), value);
	}
	else
	{
		WARNING_STRICT_FATAL(true, ("FactoryObject::setAttribute called on "
			"non-auth object %s", getNetworkId().getValueString().c_str()));
	}
}	// FactoryObject::setAttribute

//-----------------------------------------------------------------------

/**
 * This is a dummy function needed for compatability with the
 * ManufactureSchematicObject. Note that unlike the other dummy functions,
 * this will get legitimately called while making an object, but the data
 * won't be used.
 */
const std::map<StringId, float> & FactoryObject::getResourceMaxAttributes() const
{
static const AttribMap dummy;

	return dummy;
}	// FactoryObject::getResourceMaxAttributes

//-----------------------------------------------------------------------

/**
 * This is a dummy function needed for compatability with the
 * ManufactureSchematicObject.
 */
void FactoryObject::setResourceMaxAttribute(const StringId & name, float value)
{
	UNREF(name);
	UNREF(value);

	WARNING_STRICT_FATAL(true, ("FactoryObject::setResourceMaxAttribute "
		"called for object %s. This should never happen!",
		getNetworkId().getValueString().c_str()));
}	// FactoryObject::setResourceMaxAttribute

//-----------------------------------------------------------------------

/**
 * This is a dummy function needed for compatability with the
 * ManufactureSchematicObject.
 */
const std::vector<StringId> & FactoryObject::getExperimentAttributeNames() const
{
static const StringIdList dummy;

	WARNING_STRICT_FATAL(true, ("FactoryObject::getExperimentAttributeNames "
		"called for object %s. This should never happen!",
		getNetworkId().getValueString().c_str()));
	return dummy;
}	// FactoryObject::getExperimentAttributeNames

//-----------------------------------------------------------------------

/**
 * This is a dummy function needed for compatability with the
 * ManufactureSchematicObject.
 */
void FactoryObject::setExperimentAttribute(const StringId & name, float value)
{
	UNREF(name);
	UNREF(value);

	WARNING_STRICT_FATAL(true, ("FactoryObject::setExperimentAttribute "
		"called for object %s. This should never happen!",
		getNetworkId().getValueString().c_str()));
}	// FactoryObject::setExperimentAttribute

//-----------------------------------------------------------------------

/**
 * This is a dummy function needed for compatability with the
 * ManufactureSchematicObject.
 */
void FactoryObject::getAllExperimentAttributeValues(const StringId & name,
	float & value, float & minValue, float & maxValue, float & resourceMaxValue) const
{
	UNREF(name);
	UNREF(value);
	UNREF(minValue);
	UNREF(maxValue);
	UNREF(resourceMaxValue);

	WARNING_STRICT_FATAL(true, ("FactoryObject::getAllExperimentAttributeValues "
		"called for object %s. This should never happen!",
		getNetworkId().getValueString().c_str()));
}	// FactoryObject::getAllExperimentAttributeValues

//-----------------------------------------------------------------------

/**
 * This is a dummy function needed for compatability with the
 * ManufactureSchematicObject.
 */
void FactoryObject::setExperimentAttributeLimits(const StringId & name,
	float minValue, float maxValue, float resourceMaxValue)
{
	UNREF(name);
	UNREF(minValue);
	UNREF(maxValue);
	UNREF(resourceMaxValue);

	WARNING_STRICT_FATAL(true, ("FactoryObject::setExperimentAttributeLimits "
		"called for object %s. This should never happen!",
		getNetworkId().getValueString().c_str()));
}	// FactoryObject::setExperimentAttributeLimits

//-----------------------------------------------------------------------

/**
 * Returns if we are creating a finished item or a prototype.
 *
 * @return true (we always make finished items)
 */
bool FactoryObject::isMakingObject(void) const
{
	return true;
}	// FactoryObject::isMakingObject

//-----------------------------------------------------------------------

/**
 * Returns our network id. Needed so that the ManufactureObjectInterface can 
 * report its id.
 *
 * @return our network id
 */
const NetworkId & FactoryObject::getDerivedNetworkId() const
{
	return getNetworkId();
}	// FactoryObject::getDerivedNetworkId

//-----------------------------------------------------------------------

/**
 * Makes a copy of this factory.
 *
 * @param destination		where to create the copy (should be a volume container)
 * @param count				how many items the new factory will be set to have;
 *							items added to the new factory will be removed from
 *							this one
 * @param destroySource		flag that we should destroy ourself if all our items
 *							are being moved to the copy
 *
 * @return the new factory
 */
FactoryObject * FactoryObject::makeCopy(ServerObject & destination, int count,
	bool destroySource)
{
	if (!isFactoryOk())
		return nullptr;

	// don't allow making a copy if we are an old crate
	if (!getLoadContents())
	{
		Unicode::String oob;
		{
			ProsePackage pp;
			ProsePackageManagerServer::createSimpleProsePackage (*this, 
				STRING_ID_CANT_SPLIT, pp);
			OutOfBandPackager::pack(pp, -1, oob);
		}
		const Object * const owner = NetworkIdManager::getObjectById(getOwnerId());
		if (owner != nullptr)
		{
			Chat::sendSystemMessage(*(owner->asServerObject()), Unicode::emptyString, oob);
		}
		return nullptr;
	}

	if (count <= 0)
	{
		WARNING_STRICT_FATAL(true, ("FactoryObject::makeCopy %s passed invalid "
			"count %d", getNetworkId().getValueString().c_str(), count));
		return nullptr;
	}
	if (count > getCount())
	{
		WARNING_STRICT_FATAL(true, ("FactoryObject::makeCopy %s given count %d "
			"that is > than our count %d", getNetworkId().getValueString().c_str(),
			count, getCount()));
		return nullptr;
	}

	VolumeContainer *destVolumeContainer = ContainerInterface::getVolumeContainer(destination);
	
	if (destVolumeContainer == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("FactoryObject::makeCopy %s passed "
			"destination %s that is not a volume container",
			getNetworkId().getValueString().c_str(),
			destination.getNetworkId().getValueString().c_str()));
		return nullptr;
	}

	// create the new factory
	if (safe_cast<const ServerObjectTemplate *>(getObjectTemplate()) == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("FactoryObject::makeCopy: %s has no object template!", getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	FactoryObject * newFactory = safe_cast<FactoryObject *>(ServerWorld::createNewObject(
		*safe_cast<const ServerObjectTemplate *>(getObjectTemplate()), destination,
		false));
	if (newFactory == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("FactoryObject::makeCopy %s could not create "
			"the new factory", getNetworkId().getValueString().c_str()));
		return nullptr;
	}

	// copy our objvars and scripts to the new factory
	{
	const DynamicVariableList & list = getObjVars();
	for (DynamicVariableList::MapType::const_iterator i = list.begin();
		i != list.end(); ++i)
	{
		const std::string & name = (*i).first;
		newFactory->copyObjVars(name, *this, name);
	}
	}

	{
	const ScriptList & sourceScripts = getScriptObject()->getScripts();
	for (ScriptList::const_iterator i = sourceScripts.begin(); i != sourceScripts.end(); ++i)
		newFactory->getScriptObject()->attachScript((*i).getScriptName(), false);
	}

	// copy other data
	newFactory->setCraftedId(getCraftedId());
	newFactory->setOwnerId(getOwnerId());

	if (getCreatorId() != NetworkId::cms_invalid)
		newFactory->setCreatorId(getCreatorId());
	else
		newFactory->setCreatorId(getOwnerId());

	if (!getAssignedObjectName().empty())
		newFactory->setObjectName(getAssignedObjectName());

	// update the new factory's attributes
	{
	const DynamicVariableList::NestedList attributes(getObjVars(), OBJVAR_ATTRIBUTES);
	for (DynamicVariableList::NestedList::const_iterator i = attributes.begin();
		i != attributes.end(); ++i)
	{
		float value;
		i.getValue(value);
		newFactory->setAttribute(StringId(i.getName()), value);
	}
	}

	// move items from us to the new factory
	if (count == getCount())
	{
		// move our contained object to the new factory, and delete ourself if
		// we aren't already being deleted
		const TangibleObject * object = FactoryObject::getContainedObject();
		if (object != nullptr)
		{
			Container::ContainerErrorCode error;
			ContainerInterface::transferItemToVolumeContainer(*newFactory,
				*const_cast<TangibleObject *>(object), nullptr, error);
			newFactory->setCount(count);
			setCount(0);
			if (destroySource && !isBeingDestroyed())
				permanentlyDestroy(DeleteReasons::Consumed);
		}
	}
	else
	{
		if (newFactory->addObject())
		{
			newFactory->setCount(count);
			setCount(getCount() - count);
			IGNORE_RETURN(destVolumeContainer->recalculateVolume());
		}
		else
		{
			newFactory->permanentlyDestroy(DeleteReasons::SetupFailed);
			newFactory = nullptr;
		}
	}
	return newFactory;
}	// FactoryObject::makeCopy

//-----------------------------------------------------------------------

void FactoryObject::getAttributes(AttributeVector & data) const
{
	TangibleObject::getAttributes(data);

	char valueBuffer[32];
  	const size_t valueBuffer_size = sizeof (valueBuffer);

	snprintf(valueBuffer, valueBuffer_size, "%i", getCount());
	data.push_back(std::make_pair(SharedObjectAttributes::factory_count, Unicode::narrowToWide(valueBuffer)));

	//-- show manufactured object attribs
	data.push_back(std::make_pair(SharedObjectAttributes::factory_attribs, Unicode::narrowToWide("\\#pcontrast2 ----------\\#.")));

	if (!getLoadContents())
	{
		// old-style crate
		retrieveStoredObjectAttributes (data);
	}
	else
	{
		// new-style crate
		const ServerObject * const storedObject = getContainedObject();
		if (storedObject != nullptr)
		{
			data.reserve (data.size () + 2);

			data.push_back(std::make_pair(SharedObjectAttributes::object_type, 
				Unicode::narrowToWide ("@") + Unicode::narrowToWide (GameObjectTypes::getStringId (storedObject->getGameObjectType ()).getCanonicalRepresentation ())));
		
			const Unicode::String & originalName = Unicode::narrowToWide ("@") + Unicode::narrowToWide (storedObject->getObjectNameStringId ().getCanonicalRepresentation ());

			if (storedObject->asCreatureObject ())
				data.push_back (std::make_pair (SharedObjectAttributes::original_name_creature, originalName));
			else
				data.push_back (std::make_pair (SharedObjectAttributes::original_name, originalName));

			storedObject->getAttributes(getOwnerId(), data);
		}
	}
}

//-----------------------------------------------------------------------

