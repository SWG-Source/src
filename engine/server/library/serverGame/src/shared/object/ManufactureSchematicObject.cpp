//========================================================================
//
// ManufactureSchematicObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ManufactureSchematicObject.h"

#include "UnicodeUtils.h"
#include "serverGame/Chat.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/DraftSchematicObject.h"
#include "serverGame/FactoryObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ManufactureSchematicSynchronizedUi.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ProsePackageManagerServer.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerController.h"
#include "serverGame/ServerManufactureSchematicObjectTemplate.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/CraftingData.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/MessageQueueDraftSlots.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsQueryResponse.h"
#include "sharedObject/ArrangementDescriptor.h"
#include "sharedObject/ArrangementDescriptorList.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/RangedIntCustomizationVariable.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"


namespace ManufactureSchematicObjectNamespace
{
	// objvar names
	const std::string OBJVAR_CRAFTING_IS_PROTOTYPE("crafting.isPrototype");
	const std::string OBJVAR_SLOTS("ingr");
	const std::string OBJVAR_SLOT_NAME("name");
	const std::string OBJVAR_SLOT_INDEX("index");
	const std::string OBJVAR_SLOT_TYPE("type");
	const std::string OBJVAR_SLOT_OPTION("option");
	const std::string OBJVAR_SLOT_COMPLEXITY("complex");
	const std::string OBJVAR_INGREDIENTS("ingr");
	const std::string OBJVAR_RESOURCE_ID("id");
	const std::string OBJVAR_INGR_RESOURCE_ID(OBJVAR_INGREDIENTS + '.' + OBJVAR_RESOURCE_ID);
	const std::string OBJVAR_RESOURCE_IMPORTED_DATA("resourceData");
	const std::string OBJVAR_INGR_RESOURCE_IMPORTED_DATA(OBJVAR_INGREDIENTS + '.' + OBJVAR_RESOURCE_IMPORTED_DATA);
	const std::string OBJVAR_RESOURCE_COUNT("cnt");
	const std::string OBJVAR_COMPONENT_NAME("name");
	const std::string OBJVAR_COMPONENT_ID("realId");
	const std::string OBJVAR_COMPONENT_CRAFTED_ID("id");
	const std::string OBJVAR_COMPONENT_CRAFTED_XP_TYPE("xpType");
	const std::string OBJVAR_COMPONENT_TEMPLATE_NAME("tpl");
	const std::string OBJVAR_SLOT_SOURCES("src");
	const std::string OBJVAR_ATTRIBUTES("crafting_attributes");
	const std::string OBJVAR_COMPONENTS("crafting_components");
	const std::string OBJVAR_STORED_RESOURCES("crafting_resource");
	const std::string OBJVAR_ATTRIBUTE_BONUSES("attrib_bonuses");
	const std::string OBJVAR_ATTRIBUTE_BONUS("attribute.bonus");
	const std::string OBJVAR_INVALID_SCHEMATIC("ImAnInvalidSchematic");
	const std::string OBJVAR_ORIGINAL_OBJECT_ID("original_object_id");

	const StringId ATTRIBUTES_COMPLEXITY("crafting","complexity");

	// this is a special schematic that we use to show an image for schematics
	// that have been removed from the game
	const std::string MISSING_SCHEMATIC_SUBSTITUTE("object/draft_schematic/base/missing_schematic_substitute.iff");
}

using namespace ManufactureSchematicObjectNamespace;

//----------------------------------------------------------------------

const SharedObjectTemplate * ManufactureSchematicObject::m_defaultSharedTemplate = nullptr;


//========================================================================

/**
 * Class constructor. This will only get called when making a schematic created 
 * by a designer.
 *
 * @param newTemplate		template to create the schematic from.
 */
ManufactureSchematicObject::ManufactureSchematicObject(
	const ServerManufactureSchematicObjectTemplate* newTemplate) :
	IntangibleObject(newTemplate),
	m_draftSchematicSharedTemplate(),
	m_draftSchematic(),
	m_creatorId(NetworkId::cms_invalid),
	m_creatorName(Unicode::narrowToWide(newTemplate->getCreator())),
	m_itemsPerContainer(1),
	m_attributes(),
	m_manufactureTime(1),
	m_isCrafting(false),
	m_customAppearance(),
	m_appearanceData(),
	m_resourceMaxAttributes(),
	m_schematicChangedSignal(0),
	m_factories(),
	m_isMakingObject(false)
{
	addMembersToPackages();
}	// ManufactureSchematicObject::ManufactureSchematicObject(designer made)

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
ManufactureSchematicObject::~ManufactureSchematicObject()
{
	// 	m_synchronizedUi deleted by superclass
}	// ManufactureSchematicObject::~ManufactureSchematicObject

//-----------------------------------------------------------------------

ManufactureSchematicObject * ManufactureSchematicObject::asManufactureSchematicObject()
{
	return this;
}

//-----------------------------------------------------------------------

ManufactureSchematicObject const * ManufactureSchematicObject::asManufactureSchematicObject() const
{
	return this;
}

//-----------------------------------------------------------------------

void ManufactureSchematicObject::kill()
{
	IntangibleObject::kill();
}

//-----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * ManufactureSchematicObject::getDefaultSharedTemplate() const
{
	static const ConstCharCrcLowerString templateName("object/manufacture_schematic/base/shared_manufacture_schematic_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "ManufactureSchematicObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// ManufactureSchematicObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void ManufactureSchematicObject::removeDefaultTemplate()
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// ManufactureSchematicObject::removeDefaultTemplate

//-----------------------------------------------------------------------

/**
 * Initializes the schematic from a draft schematic being used at a crafting 
 * station.
 *
 * @param schematic		the draft schematic used to make this schematic
 * @param creator		the player crafting
 */
void ManufactureSchematicObject::init(const DraftSchematicObject & schematic, const CachedNetworkId & creator)
{
	m_draftSchematic               = schematic.getObjectTemplate()->getCrcName().getCrc();
	m_draftSchematicSharedTemplate = schematic.getSharedTemplate()->getCrcName().getCrc();

	m_creatorId = creator;
	if (creator.getObject() != nullptr)
	{
		m_creatorName = safe_cast<const ServerObject *>(creator.getObject())->
			getObjectName();
	}

	m_itemsPerContainer = schematic.getItemsPerContainer();
	m_manufactureTime   = schematic.getManufactureTime();

	m_attributes.clear();
	m_resourceMaxAttributes.clear();

	if (isAuthoritative())
		removeObjVarItem(".");

	m_isCrafting = true;
}	// ManufactureSchematicObject::init

//-----------------------------------------------------------------------

/**
 * Initializes data for a newly created object.
 */
void ManufactureSchematicObject::initializeFirstTimeObject()
{
	IntangibleObject::initializeFirstTimeObject();

	const DraftSchematicObject * const sourceSchematic = DraftSchematicObject::getSchematic(getDraftSchematic());
	NOT_NULL(sourceSchematic);

	const ServerDraftSchematicObjectTemplate * const sourceSchematicTemplate = 
		safe_cast<const ServerDraftSchematicObjectTemplate *>(sourceSchematic->getObjectTemplate());
	NOT_NULL(sourceSchematicTemplate);

	setObjectName         (sourceSchematic->getAssignedObjectName ());
	setObjectNameStringId (sourceSchematic->getObjectNameStringId ());

	setCount(10);

	// the slots need to be set up externally, filtered by player skill
	eraseObjVarListEntries(OBJVAR_SLOTS);

	computeComplexity();

	// copy the draft schematic objvars onto us
	{
	DynamicVariableList objvars;
	sourceSchematicTemplate->getObjvars(objvars);
	for (DynamicVariableList::MapType::const_iterator i = objvars.begin(); 
		i != objvars.end(); ++i)
	{
		setObjVarItem((*i).first, (*i).second);
	}
	}

	{
	const int count = sourceSchematicTemplate->getManufactureScriptsCount();
	GameScriptObject * const scriptObject = getScriptObject();
	NOT_NULL(scriptObject);
	for (int i = 0; i < count; ++i)
	{
		scriptObject->attachScript(sourceSchematicTemplate->getManufactureScripts(i), true);
	}
	}
}	// ManufactureSchematicObject::initializeFirstTimeObject

//-----------------------------------------------------------------------

/**
 * Initializes an object just proxied.
 */
void ManufactureSchematicObject::endBaselines()
{
	IntangibleObject::endBaselines();

	recalculateData();
}	// ManufactureSchematicObject::endBaselines

//-----------------------------------------------------------------------

/**
 * Creates a synchronized ui class in order to get data to the client during a 
 * crafting session.
 *
 * @return the newly created class
 */
ServerSynchronizedUi * ManufactureSchematicObject::createSynchronizedUi ()
{
	ManufactureSchematicSynchronizedUi * sync = new ManufactureSchematicSynchronizedUi(*this);

	// set up the sync ui for our slots
	int i;
	int count;
	const DraftSchematicObject * draft = DraftSchematicObject::getSchematic(
		m_draftSchematic.get());
	NOT_NULL(draft);
	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);
	{
		Crafting::IngredientSlot slotData;
		count = slots.getCount();
		for (i = 0; i < count; ++i)
		{
			if (getSlot(i, slotData, false))
			{
				sync->createNewSlot(slotData.name, slotData.complexity);
				sync->setSlotType(slotData.name, slotData.ingredientType);
				sync->setSlotOption(slotData.name, slotData.draftSlotOption);
				sync->setSlotIndex(slotData.name, slotData.draftSlotIndex);
			}
		}
	}

	count = draft->getExperimentalAttributesCount();
	for (i = 0; i < count; ++i)
	{
		sync->setAttribute(draft->getExperimentalAttribute(i), 0);
	}

	sync->setReady();

	return sync;
}

//------------------------------------------------------------------------------

/**
 * Re-initializes the schematic.
 */
void ManufactureSchematicObject::reset()
{
	const DraftSchematicObject * const schematic = DraftSchematicObject::getSchematic(m_draftSchematic.get());
	NOT_NULL(schematic);

	const CreatureObject * owner = NetworkIdManager::getObjectById(m_creatorId.get())->asServerObject()->asCreatureObject();
	NOT_NULL(owner);

	init(*schematic, CachedNetworkId(*owner));

	computeComplexity();
	setObjVarItem(OBJVAR_CRAFTING_IS_PROTOTYPE, true);
	
	Crafting::IngredientSlot manfSlot;
	ServerDraftSchematicObjectTemplate::IngredientSlot slot;
	const int slotCount = schematic->getSlotsCount();
	for (int i = 0; i < slotCount; ++i)
	{
		if (!schematic->getSlot(slot, i))
		{
			DEBUG_WARNING(true, ("draft schematic slot %d for schematic %s not "
				"found!",i, schematic->getObjectTemplateName()));
			continue;
		}
		if (!slot.optional || slot.optionalSkillCommand.empty() ||
			owner->hasCommand(slot.optionalSkillCommand))
		{
			// create the slot
			IGNORE_RETURN(getSlot(slot.name, manfSlot));
			// set the manufacturing slot->draft slot map
			setSlotIndex(slot.name, i);
		}
	}
}	// ManufactureSchematicObject::reset

//------------------------------------------------------------------------------

/**
 * Returns the draft schematic name crc for the schematic used to define this 
 * schematic.
 *
 * @return the schematic crc
 */
uint32 ManufactureSchematicObject::getDraftSchematic() const
{
	return m_draftSchematic.get();
}

//------------------------------------------------------------------------------

/**
 * Returns if we are creating a finished item or a prototype.
 *
 * @return true if we are making a finished item, false if a prototype
 */
bool ManufactureSchematicObject::isMakingObject() const
{
	return m_isMakingObject;
}

//------------------------------------------------------------------------------

/**
 * Returns our network id. Needed so that the ManufactureObjectInterface can 
 * report its id.
 *
 * @return our network id
 */
const NetworkId & ManufactureSchematicObject::getDerivedNetworkId() const
{
	return getNetworkId();
}	// ManufactureSchematicObject::getDerivedNetworkId

//------------------------------------------------------------------------------

/**
 * Sets the value for an attribute of the schematic
 *
 * @param attribute		the attribute data
 */
void ManufactureSchematicObject::setAttribute(const ServerIntangibleObjectTemplate::SchematicAttribute & attribute)
{
	setAttribute(attribute.name, static_cast<float>(attribute.value));
}

//------------------------------------------------------------------------------

/**
 * Sets the value for an attribute of the schematic
 *
 * @param name		the attribute name
 * @param value		the attribute value
 */
void ManufactureSchematicObject::setAttribute(const StringId & name, float value)
{
	if (isAuthoritative())
	{
		m_attributes.set(name, value);
	}
	else
	{
		WARNING_STRICT_FATAL(true, ("ManufactureSchematicObject::setAttribute "
			"called on non-auth object %s", getNetworkId().getValueString().c_str()));
	}
}

//-----------------------------------------------------------------------

const std::map<StringId, float> & ManufactureSchematicObject::getResourceMaxAttributes() const
{
	return m_resourceMaxAttributes.getMap();
}

//-----------------------------------------------------------------------

void ManufactureSchematicObject::setResourceMaxAttribute(const StringId & name, float value)
{
	m_resourceMaxAttributes.set(name, value);
}

//-----------------------------------------------------------------------

/**
 * Returns the crafting type of this schematic.
 *
 * @return the crafting type
 */
ServerIntangibleObjectTemplate::CraftingType ManufactureSchematicObject::getCategory() const
{
	const DraftSchematicObject * draft = DraftSchematicObject::getSchematic(
		getDraftSchematic());

	// There's a possibility that the draft schematic this schematic is based on
	// has been changed enough to make this schematic useless, or has been removed
	// from the game. If this happens, we log it and tell the player to delete
	// this schematic
	static const int BUFSIZE = 1024;
	char errBuffer[BUFSIZE];
	bool schematicOk = true;
	
	if (getObjVars().hasItem(OBJVAR_INVALID_SCHEMATIC))
	{
		schematicOk = false;
	}
	else if (draft != nullptr)
	{
		// test the ingredients
		Crafting::IngredientSlot sourceSlot;
		ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
		int numSlots = getSlotsCount();
		for (int i = 0; i < numSlots; ++i)
		{
			// see if we can get the slot from ourself
			if (!getSlot(i, sourceSlot, true))
			{
				sprintf(errBuffer, "slot # %d not existing", i);
				schematicOk = false;
				break;
			}

			// see if the draft schematic knows about the slot
			if (!draft->getSlot(draftSlot, sourceSlot.name))
			{
				snprintf(errBuffer, BUFSIZE, "missing slot %s in draft schematic %s", 
					sourceSlot.name.getCanonicalRepresentation().c_str(), 
					draft->getObjectTemplateName());
				errBuffer[BUFSIZE-1] = '\0';
				schematicOk = false;
				break;
			}
		}
	}
	else
	{
		sprintf(errBuffer, "its source draft schematic (crc=%lu) not existing", 
			getDraftSchematic());
		schematicOk = false;
	}

	if (schematicOk)
		return draft->getCategory();

	// this schematic can't be used, inform the player
	const CreatureObject * owner = nullptr;
	const Object * object = ContainerInterface::getContainedByObject(*this);
	while (owner == nullptr && object != nullptr && object->asServerObject() != nullptr)
	{
		if (object->asServerObject()->asCreatureObject() != nullptr)
			owner = object->asServerObject()->asCreatureObject();
		else
			object = ContainerInterface::getContainedByObject(*object);
	}

	// log that the schematic can't be used
	LOG("CustomerService", ("Crafting:Manufacturing schematic %s "
		"owned by %s is unuseable due to %s", getNetworkId().getValueString().c_str(), 
		PlayerObject::getAccountDescription(owner).c_str(), errBuffer));
	if (owner != nullptr)
	{
		// send mail to the owner telling them their schematic can't be used
		const static StringId message("system_msg", "manf_schematic_unuseable");
		Unicode::String oob;
		ProsePackage pp;
		ProsePackageManagerServer::createSimpleProsePackage (*this, message, pp);
		OutOfBandPackager::pack(pp, -1, oob);
		Unicode::String messageString = Unicode::narrowToWide("@" + message.getCanonicalRepresentation());
		Chat::sendPersistentMessage("", Unicode::wideToNarrow(owner->getObjectName()), 
			messageString, Unicode::emptyString, oob);
	}

	return static_cast<ServerIntangibleObjectTemplate::CraftingType>(0);
}	// ManufactureSchematicObject::getCategory

//-----------------------------------------------------------------------

/**
 * Forwards the draft schematic flag.
 *
 * @return true if a crafting session destroys it's ingredients
 */
bool ManufactureSchematicObject::mustDestroyIngredients() const
{
	const DraftSchematicObject * draft = DraftSchematicObject::getSchematic(
		m_draftSchematic.get());
	NOT_NULL(draft);
	if (draft != nullptr)
		return draft->mustDestroyIngredients();
	return false;
}	// ManufactureSchematicObject::mustDestroyIngredients

//-----------------------------------------------------------------------

/**
 * Returns an ingredient slot needed to manufacture an item with this schematic.
 *
 * @param index				slot to get
 * @param data				structure to be filled in with the slot data
 * @param creatingObject	flag that we are getting the slot in order to create 
 *							a crafted object; if the slot is a component slot, 
 *							this will cause the data.ingredient value to be the 
 *                          component id, instead of the component's manf schematic id
 *
 * @return true on success, false if the index was out of range
 */
bool ManufactureSchematicObject::getSlot(int index, Crafting::IngredientSlot & data, 
	bool creatingObject) const
{
	const DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);

	if (index < 0 || index >= static_cast<int>(slots.getCount()))
		return false;

	const DynamicVariableList::NestedList slot(slots.getItemByPosition(index).getNestedList());

	int intval=0;
	if (slot.getItem(OBJVAR_SLOT_TYPE,intval))
		data.ingredientType = static_cast<enum Crafting::IngredientType>(intval);
	slot.getItem(OBJVAR_SLOT_NAME,data.name);
	slot.getItem(OBJVAR_SLOT_OPTION,data.draftSlotOption);
	slot.getItem(OBJVAR_SLOT_INDEX,data.draftSlotIndex);
	slot.getItem(OBJVAR_SLOT_COMPLEXITY,data.complexity);
	data.ingredients.clear();

	// get the draft schematic and slot info for the slot to determine the 
	// ingredient type
	const DraftSchematicObject * const draft = DraftSchematicObject::getSchematic(getDraftSchematic());
	if (!draft)
	{
		WARNING (true, ("ManufactureSchematicObject [%d] has invalid draft "
			"schematic [%u]", getNetworkId().getValueString().c_str(), 
			getDraftSchematic()));
		return false;
	}

	ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
	if (!draft->getSlot(draftSlot, data.name))
		return false;

	ServerIntangibleObjectTemplate::IngredientType ingredientType;
	if (data.draftSlotOption < 0 || data.draftSlotOption >= static_cast<int>(draftSlot.options.size()))
	{
		ingredientType = draftSlot.options[0].ingredientType;
	}
	else
		ingredientType = draftSlot.options[data.draftSlotOption].ingredientType;

	DynamicVariableList::NestedList slotIngredients(slot,OBJVAR_INGREDIENTS);

	if (ingredientType == ServerIntangibleObjectTemplate::IT_item ||
		ingredientType == ServerIntangibleObjectTemplate::IT_template ||
		ingredientType == ServerIntangibleObjectTemplate::IT_templateGeneric ||
		ingredientType == ServerIntangibleObjectTemplate::IT_schematic ||
		ingredientType == ServerIntangibleObjectTemplate::IT_schematicGeneric)
	{
		// ingredient is a component

		// in theory there can be more components than sources (although not the
		// reverse), so we need to keep track of the sources separately
		DynamicVariableList::NestedList sources(slot,OBJVAR_SLOT_SOURCES);
		int sourceIndex = 0;

		char buffer[32];
		const int count = slotIngredients.getCount();
		for (int i = 0; i < count; ++i)
		{
			_itoa(i, buffer, 10);
			const DynamicVariableList::NestedList ingredient(slotIngredients,buffer);

			Crafting::ComponentIngredient * const component = new Crafting::ComponentIngredient();

			data.ingredients.push_back(Crafting::SimpleIngredientPtr(component));
			component->count = 1;
			if (!ingredient.hasItem(OBJVAR_COMPONENT_NAME))
				return false;
			ingredient.getItem(OBJVAR_COMPONENT_NAME,component->componentName);

			const bool isComponentCrafted = ingredient.hasItem(OBJVAR_COMPONENT_CRAFTED_XP_TYPE);
			if (isComponentCrafted)
			{
				// set the xp type
				ingredient.getItem(OBJVAR_COMPONENT_CRAFTED_XP_TYPE,component->xpType);
				
				// set the xp source and amount
				if (!sources.empty())
				{
					DynamicVariableList::NestedList::const_iterator data(sources.getItemByPosition(sourceIndex));
					if (data != sources.end())
					{
						component->source = NetworkId(data.getName());
						data.getValue(component->sourceXp);
						++sourceIndex;
					}
				}
			}
			if (creatingObject)
			{
				ingredient.getItem(OBJVAR_COMPONENT_ID, component->ingredient);
			}
			else
			{
				if (ingredientType != ServerIntangibleObjectTemplate::IT_templateGeneric && isComponentCrafted)
				{
					ingredient.getItem(OBJVAR_COMPONENT_CRAFTED_ID, component->ingredient);
				}
				else
				{
					if (!ingredient.getItem(OBJVAR_COMPONENT_TEMPLATE_NAME,component->templateName))
						return false;
				}
			}
		}
	}
	else
	{
		// ingredient is a resource
		NetworkId ingredientId;
		if (slotIngredients.getItem(OBJVAR_RESOURCE_ID,ingredientId))
		{
			int resourceCount = 0;
			slotIngredients.getItem(OBJVAR_RESOURCE_COUNT, resourceCount);

			// get the xp type based on the resource container
			int xpType = 0;
			const ResourceTypeObject * const resourceType = ServerUniverse::getInstance().getResourceTypeById(ingredientId);
			if (resourceType != nullptr)
			{
				std::string crateTemplateName;
				resourceType->getCrateTemplate(crateTemplateName);
				const ServerObjectTemplate * crateTemplate = safe_cast<const ServerObjectTemplate *>(ObjectTemplateList::fetch(crateTemplateName));
				if (crateTemplate != nullptr && crateTemplate->getXpPointsCount() > 0)
				{
					ServerObjectTemplate::Xp xpData;
					crateTemplate->getXpPoints(xpData, 0);
					xpType = xpData.type;
				}
			}

			const DynamicVariableList::NestedList sources(slot,OBJVAR_SLOT_SOURCES);
			if (!sources.empty())
			{
				for (DynamicVariableList::NestedList::const_iterator source=sources.begin(); source!=sources.end(); ++source)
				{
					const NetworkId sourceId(source.getName());
					int sourceResourceCount = 0;
					source.getValue(sourceResourceCount);
					data.ingredients.push_back(Crafting::SimpleIngredientPtr(
												   new Crafting::SimpleIngredient(ingredientId, sourceResourceCount, 
																				  sourceId, xpType)));
				}
			}
			else
			{
				data.ingredients.push_back(Crafting::SimpleIngredientPtr(
					new Crafting::SimpleIngredient(ingredientId, resourceCount, 
					NetworkId::cms_invalid, xpType)));
			}
		}
	}

	return true;
}	// ManufactureSchematicObject::getSlot(int)

//-----------------------------------------------------------------------

/**
 * Gets the ingredient slot with a given name. If the slot does not exist (which 
 * may be the case for optional slots) the slot will be created.
 *
 * @param name		the slot name
 * @param data		structure to be filled in with the slot data
 *
 * @return true on success, false on error
 */
bool ManufactureSchematicObject::getSlot(const StringId & name, Crafting::IngredientSlot & data)
{
	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);
	DynamicVariableList::NestedList::const_iterator i(slots.begin());

	int index=0;
	
	// see if the slot already exists
	for (; i!=slots.end(); ++i)
	{
		DynamicVariableList::NestedList slot=i.getNestedList();
		StringId nameValue;
		if (slot.getItem(OBJVAR_SLOT_NAME,nameValue) && name == nameValue)
		{
			break;
		}
		++index;
	}
	if (i == slots.end())
	{
		// check the draft schematic for the slot
		const DraftSchematicObject * draft = DraftSchematicObject::getSchematic(
			m_draftSchematic.get());
		NOT_NULL(draft);

		ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
		if (!draft->getSlot(draftSlot, name))
			return false;

		// add the obvars for the new slot
		// @todo: can we make the list name the slot name? Is the StringId text unique?
		char buffer[256];
		char secondBuffer[32];
		if(slots.getCount() < 10)
			sprintf(secondBuffer, "0%d", slots.getCount());
		else
			sprintf(secondBuffer, "%d", slots.getCount());
		sprintf(buffer, "%s.%s.", OBJVAR_SLOTS.c_str(), secondBuffer);
		setObjVarItem(buffer + OBJVAR_SLOT_TYPE, Crafting::IT_none);
		setObjVarItem(buffer + OBJVAR_SLOT_NAME, draftSlot.name);
		setObjVarItem(buffer + OBJVAR_SLOT_OPTION, -1);
		setObjVarItem(buffer + OBJVAR_SLOT_INDEX, -1);
		setObjVarItem(buffer + OBJVAR_SLOT_COMPLEXITY, draftSlot.complexity);

		ManufactureSchematicSynchronizedUi * const sync = dynamic_cast<
			ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
		if (sync)
			sync->createNewSlot(draftSlot.name, draftSlot.complexity);
	}

	return getSlot(index, data, false);
}	// ManufactureSchematicObject::getSlot(const StringId &)

//-----------------------------------------------------------------------

/**
 * Returns the number of ingredient slots needed to manufacture an item with 
 * this schematic.
 *
 * @return the ingredient count
 */
int ManufactureSchematicObject::getSlotsCount() const
{
	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);
	return slots.getCount();
}	// ManufactureSchematicObject::getSlotsCount

//-----------------------------------------------------------------------

/**
 * Sets the slot type of a manf slot.
 *
 * @param name		the name of the slot
 * @param type		the slot type
 */
void ManufactureSchematicObject::setSlotType(const StringId & name, int type)
{
	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);

	for (DynamicVariableList::NestedList::const_iterator i(slots.begin()); i!=slots.end(); ++i)
	{
		DynamicVariableList::NestedList slot=i.getNestedList();
		StringId nameValue;
		if (slot.getItem(OBJVAR_SLOT_NAME,nameValue) && name == nameValue)
		{
			setObjVarItem(slot.getNameWithPath()+'.'+OBJVAR_SLOT_TYPE, type);

			ManufactureSchematicSynchronizedUi * sync = safe_cast<
				ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
			if (sync)
				sync->setSlotType(name, type);
			break;
		}
	}
}	// ManufactureSchematicObject::setSlotType

//-----------------------------------------------------------------------

/**
 * Sets the slot option index of a manf slot.
 *
 * @param name		the name of the slot
 * @param option	the option index
 */
void ManufactureSchematicObject::setSlotOption(const StringId & name, int option)
{
	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);

	for (DynamicVariableList::NestedList::const_iterator i(slots.begin()); i!=slots.end(); ++i)
	{
		DynamicVariableList::NestedList slot = i.getNestedList();
		StringId nameValue;
		if (slot.getItem(OBJVAR_SLOT_NAME,nameValue) && (name == nameValue))
		{
			setObjVarItem(slot.getNameWithPath()+'.'+OBJVAR_SLOT_OPTION, option);

			ManufactureSchematicSynchronizedUi * sync = safe_cast<
				ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
			if (sync)
				sync->setSlotOption(name, option);
			break;
		}
	}
}	// ManufactureSchematicObject::setSlotOption

//-----------------------------------------------------------------------

/**
 * Sets the draft schematic slot index of a manf slot.
 *
 * @param name		the name of the slot
 * @param index		the draft schematic index
 */
void ManufactureSchematicObject::setSlotIndex(const StringId & name, int index)
{
	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);

	for (DynamicVariableList::NestedList::const_iterator i(slots.begin()); i!=slots.end(); ++i)
	{
		DynamicVariableList::NestedList slot=i.getNestedList();
		StringId nameValue;
		if (slot.getItem(OBJVAR_SLOT_NAME,nameValue) && (name==nameValue))
		{
			setObjVarItem(slot.getNameWithPath()+'.'+OBJVAR_SLOT_INDEX, index);

			ManufactureSchematicSynchronizedUi * sync = safe_cast<
				ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
			if (sync)
				sync->setSlotIndex(name, index);
			break;
		}
	}
}	// ManufactureSchematicObject::setSlotIndex

//-----------------------------------------------------------------------

/**
 * Modifies a slot's complexity by a value.
 *
 * @param name				the name of the slot
 * @param complexityDelta	value to add to the current slot complexity
 */ 
void ManufactureSchematicObject::modifySlotComplexity(const StringId & name, 
	float complexityDelta)
{
	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);

	for (DynamicVariableList::NestedList::const_iterator i(slots.begin()); i!=slots.end(); ++i)
	{
		DynamicVariableList::NestedList slot=i.getNestedList();
		StringId nameValue;
		if (slot.getItem(OBJVAR_SLOT_NAME,nameValue) && (name==nameValue))
		{
			float complexity =0;
			slot.getItem(OBJVAR_SLOT_COMPLEXITY,complexity);
			setObjVarItem(slot.getNameWithPath()+'.'+OBJVAR_SLOT_COMPLEXITY, complexity + complexityDelta);

			ManufactureSchematicSynchronizedUi * sync = safe_cast<
				ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
			if (sync)
				sync->setSlotComplexity(name, complexity + complexityDelta);
			break;
		}
	}
}	// ManufactureSchematicObject::modifySlotComplexity

//-----------------------------------------------------------------------

/**
 * Adds a resource type to a schematic slot.
 *
 * @param name			the name of the slot
 * @param resourceId	id of the resource to add
 * @param count			number of resources to add
 */
void ManufactureSchematicObject::addSlotResource(const StringId & name, 
	const NetworkId & resourceId, int count, const NetworkId & sourceId)
{
	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);

	for (DynamicVariableList::NestedList::const_iterator i(slots.begin()); i!=slots.end(); ++i)
	{
		DynamicVariableList::NestedList slot=i.getNestedList();
		StringId nameValue;
		if (slot.getItem(OBJVAR_SLOT_NAME,nameValue) && (name==nameValue))
		{
			DynamicVariableList::NestedList slotIngredients(slot,OBJVAR_INGREDIENTS);

			setObjVarItem(slotIngredients.getNameWithPath()+'.'+OBJVAR_RESOURCE_ID, resourceId);

			// if the resource is an imported resource, save the resource data as well
			removeObjVarItem(slotIngredients.getNameWithPath()+'.'+OBJVAR_RESOURCE_IMPORTED_DATA);
			if (resourceId.getValue() > NetworkId::cms_maxNetworkIdWithoutClusterId)
			{
				ResourceTypeObject const * const rto = ServerUniverse::getInstance().getImportedResourceTypeById(resourceId);
				if (rto)
				{
					std::string const resourceData = rto->getResourceTypeDataForExport();

					if (!resourceData.empty())
						setObjVarItem(slotIngredients.getNameWithPath()+'.'+OBJVAR_RESOURCE_IMPORTED_DATA, resourceData);
				}
			}

			int ingredientCount=0;
			if (slotIngredients.getItem(OBJVAR_RESOURCE_COUNT,ingredientCount))
			{
				setObjVarItem(slotIngredients.getNameWithPath()+'.'+OBJVAR_RESOURCE_COUNT, 
					ingredientCount + count);
			}
			else
			{
				setObjVarItem(slotIngredients.getNameWithPath() + '.' + 
					OBJVAR_RESOURCE_COUNT, count);
			}

			// create the name of the objvar for the resource xp source
			char buffer[1024];
			sprintf(buffer, "%s.%s.%s", slot.getNameWithPath().c_str(),
				OBJVAR_SLOT_SOURCES.c_str(), sourceId.getValueString().c_str());
			
			// see if the xp source already contributed to this slot
			int oldCount = 0;
			IGNORE_RETURN(getObjVars().getItem(buffer, oldCount));
			
			// set the xp source objvar
			setObjVarItem(buffer, count + oldCount);

			// tell the client
			ManufactureSchematicSynchronizedUi * sync = safe_cast<
				ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
			if (sync)
				sync->addSlotIngredient(name, resourceId, count);
			break;
		}
	}
}	// ManufactureSchematicObject::addSlotResource

//-----------------------------------------------------------------------

/**
 * Adds a component to a schematic slot.
 *
 * @param name				the name of the slot
 * @param component			the component to add
 * @param ingredientType	the ingredient type that the slot requires
 */
void ManufactureSchematicObject::addSlotComponent(const StringId & name, 
	const TangibleObject & component, ServerIntangibleObjectTemplate::IngredientType 
	ingredientType)
{
	// Check if the object being added is a factory
	const TangibleObject * componentPtr = &component;
	const FactoryObject * factory = dynamic_cast<const FactoryObject *>(&component);
	if (factory != nullptr)
	{
		componentPtr = factory->getContainedObject();
		if (componentPtr == nullptr)
		{
			WARNING(true, ("ManufactureSchematicObject::addSlotComponent passed "
				"FactoryObject %s with no contained object", 
				factory->getNetworkId().getValueString().c_str()));
			return;
		}
	}

	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);

	for (DynamicVariableList::NestedList::const_iterator i(slots.begin()); i!=slots.end(); ++i)
	{
		DynamicVariableList::NestedList slot=i.getNestedList();
		StringId nameValue;
		if (slot.getItem(OBJVAR_SLOT_NAME,nameValue) && (name==nameValue))
		{
			DynamicVariableList::NestedList slotIngredients(slot,OBJVAR_INGREDIENTS);
			char buffer[1024];
			sprintf(buffer, "%s.%s.%s.%d.", OBJVAR_SLOTS.c_str(), slot.getName().c_str(), 
				OBJVAR_INGREDIENTS.c_str(), slotIngredients.getCount());
			setObjVarItem(buffer + OBJVAR_COMPONENT_NAME, componentPtr->getEncodedObjectName());
			setObjVarItem(buffer + OBJVAR_COMPONENT_ID, componentPtr->getNetworkId());
			
			if (ingredientType != ServerIntangibleObjectTemplate::IT_templateGeneric &&
				componentPtr->isCrafted())
			{
				// set the ingredient id
				setObjVarItem(buffer + OBJVAR_COMPONENT_CRAFTED_ID, componentPtr->getCraftedId());
			}
			else
			{
				// set the ingredient template
				setObjVarItem(buffer + OBJVAR_COMPONENT_TEMPLATE_NAME, 
					Unicode::narrowToWide(componentPtr->getObjectTemplateName()));
			}
			if (componentPtr->isCrafted())
			{
				// set the xp type
				setObjVarItem(buffer + OBJVAR_COMPONENT_CRAFTED_XP_TYPE, componentPtr->getCreatorXpType());
				
				// add crafter xp data
				sprintf(buffer, "%s.%s.%s", slot.getNameWithPath().c_str(), 
					OBJVAR_SLOT_SOURCES.c_str(), componentPtr->getCreatorId().getValueString().c_str());

				setObjVarItem(buffer, componentPtr->getCreatorXp());
			}

			// transfer component objvars on the component to us
			StringId slotName;
			slot.getItem(OBJVAR_SLOT_NAME,slotName);
			std::string componentCopyName = OBJVAR_COMPONENTS + "." + slotName.getText();
			if (!getObjVars().hasItem(componentCopyName))
				copyObjVars(componentCopyName, *componentPtr, OBJVAR_COMPONENTS);

			// transfer attrib bonuses on the component to us
			std::vector<int> attribBonuses;
			if (component.getAttribBonuses(attribBonuses))
			{
				std::vector<float> componentBonuses(attribBonuses.size());
				std::copy(attribBonuses.begin(), attribBonuses.end(), componentBonuses.begin());
				setObjVarItem(componentCopyName + "." + OBJVAR_ATTRIBUTE_BONUSES, componentBonuses);
			}

			ManufactureSchematicSynchronizedUi * sync = safe_cast<ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
			if (sync)
				sync->addSlotIngredient(name, componentPtr->getNetworkId(), 1);
			break;
		}
	}
}	// ManufactureSchematicObject::addSlotComponent

//-----------------------------------------------------------------------

/**
 * Finds a component in our volume container that matches given component info.
 *
 * @param info		info about the component we want
 *
 * @return the component
 */
TangibleObject * ManufactureSchematicObject::getComponent(
	const Crafting::ComponentIngredient & info) const
{
	const VolumeContainer * volumeContainer = ContainerInterface::getVolumeContainer(*this);
	if (volumeContainer == nullptr)
	{
		DEBUG_WARNING(true, ("Manf schematic %s does not have a volume container!",
			getNetworkId().getValueString().c_str()));
		return nullptr;
	}

	// check if the component is in a FactoryObject; if it is, return the factory,
	// whether it's in our volume container or not
	if (info.ingredient != NetworkId::cms_invalid)
	{
		ServerObject * object = ServerWorld::findObjectByNetworkId(info.ingredient);
		if (object != nullptr)
		{
			Object * container = ContainerInterface::getContainedByObject(*object);
			if (container != nullptr && dynamic_cast<FactoryObject*>(container) != nullptr)
				return safe_cast<TangibleObject *>(container);
		}
	}

	for (ContainerConstIterator iter = volumeContainer->begin(); 
		iter != volumeContainer->end(); ++iter)
	{
		const Container::ContainedItem & item = *iter;
		TangibleObject * object = safe_cast<TangibleObject *>(item.getObject());
		if (object != nullptr)
		{
			if (info.ingredient != NetworkId::cms_invalid)
			{
				// need matching crafted id
//				if (info.ingredient == object->getCraftedId())
				if (info.ingredient == object->getNetworkId())
					return object;
			}
			else
			{
				// need matching template name
				if (info.templateName == object->getObjectTemplateName())
					return object;
			}
		}
	}
	return nullptr;
}	// ManufactureSchematicObject::getComponent

//-----------------------------------------------------------------------

/**
 * Sets the ids of the players who are supplying resources for a schematic slot.
 *
 * @param slotIndex		the slot index
 * @param sources		the ids of the players and the amount of resources they 
 *						supplied
 */
void ManufactureSchematicObject::setSlotSources(int slotIndex, 
	const std::vector<std::pair<NetworkId, int> > & sources)
{
	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);
	DynamicVariableList::NestedList::const_iterator slotIterator=slots.getItemByPosition(slotIndex);
	DynamicVariableList::NestedList slot(slotIterator.getNestedList());
	DynamicVariableList::NestedList slotSources(slot,OBJVAR_SLOT_SOURCES);

	eraseObjVarListEntries(slotSources.getNameWithPath());

	int count = sources.size();
	for (int i = 0; i < count; ++i)
	{
		const std::pair<NetworkId, int> & source = sources[i];
		setObjVarItem(slotSources.getNameWithPath()+'.'+source.first.getValueString(),
					  source.second);
	}
}	// ManufactureSchematicObject::setSlotSources

//-----------------------------------------------------------------------

/**
 * Clears the data for a given slot in the schematic.
 *
 * @param name		the name of the slot to clear
 */
void ManufactureSchematicObject::clearSlot(const StringId & name)
{
	const DraftSchematicObject * draft = DraftSchematicObject::getSchematic(
		m_draftSchematic.get());
	NOT_NULL(draft);

	ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
	if (!draft->getSlot(draftSlot, name))
		return;

	const DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);

	for (DynamicVariableList::NestedList::const_iterator i(slots.begin()); i!=slots.end(); ++i)
	{
		const DynamicVariableList::NestedList slot(i.getNestedList());
		StringId nameValue;
		if (slot.getItem(OBJVAR_SLOT_NAME,nameValue) && (name == nameValue))
		{
			setObjVarItem(slot.getNameWithPath()+'.'+OBJVAR_SLOT_TYPE, Crafting::IT_none);
			setObjVarItem(slot.getNameWithPath()+'.'+OBJVAR_SLOT_OPTION, -1);
			setObjVarItem(slot.getNameWithPath()+'.'+OBJVAR_SLOT_COMPLEXITY, draftSlot.complexity);

			eraseObjVarListEntries(slot.getNameWithPath()+'.'+OBJVAR_INGREDIENTS);
			eraseObjVarListEntries(slot.getNameWithPath()+'.'+OBJVAR_SLOT_SOURCES);

			std::string componentData = OBJVAR_COMPONENTS + "." + name.getText();
			if (getObjVars().hasItem(componentData))
				removeObjVarItem(componentData);

			ManufactureSchematicSynchronizedUi * sync = safe_cast<ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
			if (sync)
				sync->clearSlot(name, draftSlot.complexity);
			break;
		}
	}
}	// ManufactureSchematicObject::clearSlot(const StringId &)

//-----------------------------------------------------------------------

/**
 * Clears the data for a given slot in the schematic.
 *
 * @param index		index of the slot to clear
 */
void ManufactureSchematicObject::clearSlot(int index)
{
	const DraftSchematicObject * draft = DraftSchematicObject::getSchematic(
		m_draftSchematic.get());
	NOT_NULL(draft);

	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);
	DynamicVariableList::NestedList::const_iterator slotIterator(slots.getItemByPosition(index));
	DynamicVariableList::NestedList slot(slotIterator.getNestedList());

	StringId name;
	slot.getItem(OBJVAR_SLOT_NAME, name);

	ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
	if (!draft->getSlot(draftSlot, name))
		return;

	setObjVarItem(slot.getNameWithPath()+'.'+OBJVAR_SLOT_TYPE, Crafting::IT_none);
	setObjVarItem(slot.getNameWithPath()+'.'+OBJVAR_SLOT_OPTION, -1);
	setObjVarItem(slot.getNameWithPath()+'.'+OBJVAR_SLOT_COMPLEXITY, draftSlot.complexity);

	eraseObjVarListEntries(slot.getNameWithPath()+'.'+OBJVAR_INGREDIENTS);
	eraseObjVarListEntries(slot.getNameWithPath()+'.'+OBJVAR_SLOT_SOURCES);

	std::string componentData = OBJVAR_COMPONENTS + "." + name.getText();
	if (getObjVars().hasItem(componentData))
		removeObjVarItem(componentData);

	ManufactureSchematicSynchronizedUi * sync = safe_cast<
		ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		sync->clearSlot(name, draftSlot.complexity);
}	// ManufactureSchematicObject::clearSlot(int)

//-----------------------------------------------------------------------

/**
 * Clears the sources for all the schematic's slots.
 */
void ManufactureSchematicObject::clearSlotSources()
{
	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);

	for (DynamicVariableList::NestedList::const_iterator i(slots.begin()); 
		i != slots.end(); ++i)
	{
		DynamicVariableList::NestedList slot = i.getNestedList();

		DynamicVariableList::NestedList slotSources(slot,OBJVAR_SLOT_SOURCES);
		eraseObjVarListEntries(slotSources.getNameWithPath());
	}

	// clear any factories being used
	if (!m_factories.empty())
	{
		for (Archive::AutoDeltaVector<CachedNetworkId>::const_iterator iter =
			m_factories.begin(); iter != m_factories.end(); ++iter)
		{
			FactoryObject * factory = safe_cast<FactoryObject *>((*iter).getObject());
			if (factory != nullptr)
			{
				factory->endCraftingSession();
			}
		}
		m_factories.clear();
	}

	// get rid of all our held ingredients
	VolumeContainer * volumeContainer = ContainerInterface::getVolumeContainer(*this);
	if (volumeContainer != nullptr && volumeContainer->getNumberOfItems() > 0)
	{
		// for a stacked ingredient, if there are still more than one items in the stack,
		// "consume" 1 item from the stack, and return it to the player's inventory
		std::set<ServerObject*> itemsToReturnToInventory;
		ServerObject* inventory = nullptr;

		for (ContainerIterator iter = volumeContainer->begin(); 
			iter != volumeContainer->end(); ++iter)
		{
			if ((*iter).getObject() != nullptr)
			{
				bool destroyItem = true;
				TangibleObject* to = safe_cast<ServerObject*>((*iter).getObject())->asTangibleObject();
				if (to && (to->getCount() > 1) && (dynamic_cast<FactoryObject *>(to) == nullptr))
				{
					if (inventory == nullptr)
					{
						ServerObject * o = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*to));
						while (o)
						{
							if (o->asCreatureObject() && PlayerCreatureController::getPlayerObject(o->asCreatureObject()))
							{
								inventory = o->asCreatureObject()->getInventory();
								break;
							}

							o = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*o));
						}
					}

					if (inventory)
					{
						to->setCount(to->getCount() - 1);
						itemsToReturnToInventory.insert(to);
						destroyItem = false;
					}
				}

				if (destroyItem)
				{
					safe_cast<ServerObject*>((*iter).getObject())->permanentlyDestroy(
						DeleteReasons::Replaced);
				}
			}
		}

		if (inventory && !itemsToReturnToInventory.empty())
		{
			Container::ContainerErrorCode errCode;
			for (std::set<ServerObject*>::const_iterator iter = itemsToReturnToInventory.begin(); iter != itemsToReturnToInventory.end(); ++iter)
			{
				if (!ContainerInterface::transferItemToVolumeContainer(*inventory, **iter, nullptr, errCode))
				{
					(*iter)->permanentlyDestroy(DeleteReasons::Replaced);
				}
			}
		}
	}

	eraseObjVarListEntries(OBJVAR_STORED_RESOURCES);
}	// ManufactureSchematicObject::clearSlotSources()

//-----------------------------------------------------------------------

/**
 * Clears the sources for a given schematic slot.
 *
 * @param index		the slot index
 */
void ManufactureSchematicObject::clearSlotSources(int index)
{
	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);
	DynamicVariableList::NestedList slot(slots,index);

	DynamicVariableList::NestedList slotSources(slot,OBJVAR_SLOT_SOURCES);
	eraseObjVarListEntries(slotSources.getNameWithPath());
}	// ManufactureSchematicObject::clearSlotSources(int)

//-----------------------------------------------------------------------

/**
 * Removes an optional slot from the schematic.
 *
 * @param name		the name of the slot to remove
 */
void ManufactureSchematicObject::removeSlot(const StringId & name)
{
	const DraftSchematicObject * draft = DraftSchematicObject::getSchematic(
		m_draftSchematic.get());
	NOT_NULL(draft);

	ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
	if (!draft->getSlot(draftSlot, name))
		return;

	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);

	for (DynamicVariableList::NestedList::const_iterator i(slots.begin()); i!=slots.end(); ++i)
	{
		DynamicVariableList::NestedList slot(i.getNestedList());

		StringId varName;
		if (slot.getItem(OBJVAR_SLOT_NAME,varName) && (name == varName))
		{
			std::string slotName(OBJVAR_SLOTS + "." + slot.getName());

			computeComplexity();
			removeObjVarItem(slotName);

			ManufactureSchematicSynchronizedUi * sync = safe_cast<
				ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
			if (sync)
				sync->removeSlot(name);
			break;
		}
	}
}	// ManufactureSchematicObject::removeSlot

//-----------------------------------------------------------------------

/**
 * Sets an approximate experiment mod for an assembled schematic so the player 
 * will have some idea of the risk experimenting with an item.
 *
 * @param mod		approximent mod that will be applied during experimentation
 */
void ManufactureSchematicObject::setExperimentMod(float mod)
{
	ManufactureSchematicSynchronizedUi * sync = safe_cast<
		ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		sync->setExperimentMod(mod);
}	// ManufactureSchematicObject::setExperimentMod

//-----------------------------------------------------------------------

/**
 * Returns a list of the experimental attribute names.
 *
 * @return the name list
 */
const std::vector<StringId> & ManufactureSchematicObject::getExperimentAttributeNames() const
{
static const std::vector<StringId> emptyList;

	const ManufactureSchematicSynchronizedUi * sync = safe_cast<
		const ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		return sync->getAttributeNames();
	else
		return emptyList;
}	// ManufactureSchematicObject::getExperimentAttributeNames

//-----------------------------------------------------------------------

/**
 * Returns the value of an experimental attribute.
 *
 * @param name		name of the attribute
 *
 * @return value of the attribute
 */
float ManufactureSchematicObject::getExperimentAttribute(const StringId & name) const 
{
	float result = 0;
	const ManufactureSchematicSynchronizedUi * sync = safe_cast<
		const ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		result = sync->getAttribute(name);
	return result;
}	// ManufactureSchematicObject::getExperimentAttribute

//-----------------------------------------------------------------------

/**
 * Sets the value of an experimental attribute.
 *
 * @param name		name of the attribute
 * @param value		value of the attribute
 */
void ManufactureSchematicObject::setExperimentAttribute(const StringId & name, 
	float value)
{
	ManufactureSchematicSynchronizedUi * sync = safe_cast<
		ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		sync->setAttribute(name, value);
}	// ManufactureSchematicObject::setExperimentAttribute

//-----------------------------------------------------------------------

/**
 * Returns the min value of an experimental attribute.
 *
 * @param name		name of the attribute
 *
 * @return min value of the attribute
 */
float ManufactureSchematicObject::getMinExperimentAttribute(const StringId & name) const
{
	float result = 0;
	const ManufactureSchematicSynchronizedUi * sync = safe_cast<
		const ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		result = sync->getMinAttribute(name);
	return result;
}	// ManufactureSchematicObject::getMinExperimentAttribute

//-----------------------------------------------------------------------

/**
 * Sets the min value of an experimental attribute.
 *
 * @param name		name of the attribute
 * @param value		min value of the attribute
 */
void ManufactureSchematicObject::setMinExperimentAttribute(const StringId & name, 
	float value)
{
	ManufactureSchematicSynchronizedUi * sync = safe_cast<
		ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		sync->setMinAttribute(name, value);
}	// ManufactureSchematicObject::setMinExperimentAttribute

//-----------------------------------------------------------------------

/**
 * Returns the max value of an experimental attribute.
 *
 * @param name		name of the attribute
 *
 * @return max value of the attribute
 */
float ManufactureSchematicObject::getMaxExperimentAttribute(const StringId & name) const
{
	float result = 0;
	const ManufactureSchematicSynchronizedUi * sync = safe_cast<
		const ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		result = sync->getMaxAttribute(name);
	return result;
}	// ManufactureSchematicObject::getMaxExperimentAttribute

//-----------------------------------------------------------------------

/**
 * Sets the max value of an experimental attribute.
 *
 * @param name		name of the attribute
 * @param value		max value of the attribute
 */
void ManufactureSchematicObject::setMaxExperimentAttribute(const StringId & name, 
	float value)
{
	ManufactureSchematicSynchronizedUi * sync = safe_cast<
		ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		sync->setMaxAttribute(name, value);
}	// ManufactureSchematicObject::setMaxExperimentAttribute

//-----------------------------------------------------------------------

/**
 * Returns the resource max value of an experimental attribute.
 *
 * @param name		name of the attribute
 *
 * @return resource max value of the attribute
 */
float ManufactureSchematicObject::getResourceMaxExperimentAttribute(const StringId & name) const
{
	float result = 0;
	const ManufactureSchematicSynchronizedUi * sync = safe_cast<
		const ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		result = sync->getResourceMaxAttribute(name);
	return result;
}	// ManufactureSchematicObject::getResourceMaxExperimentAttribute

//-----------------------------------------------------------------------

/**
 * Sets the resource max value of an experimental attribute.
 *
 * @param name		name of the attribute
 * @param value		resource max value of the attribute
 */
void ManufactureSchematicObject::setResourceMaxExperimentAttribute(const StringId & name, 
	float value)
{
	ManufactureSchematicSynchronizedUi * sync = safe_cast<
		ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		sync->setResourceMaxAttribute(name, value);
}	// ManufactureSchematicObject::setResourceMaxExperimentAttribute

//-----------------------------------------------------------------------

/**
 * Returns all the values associated with an experimental attribute.
 *
 * @param name					name of the attribute
 * @param value                 filled in with the attribute value
 * @param minValue				filled in with the min attribute value
 * @param maxValue				filled in with the max attribute value
 * @param resourceMaxValue		filled in with the resource max attribute value
 */
void ManufactureSchematicObject::getAllExperimentAttributeValues(const StringId & name, 
	float & value, float & minValue, float & maxValue, float & resourceMaxValue) const
{
	const ManufactureSchematicSynchronizedUi * sync = safe_cast<
		const ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		sync->getAllAttributeValues(name, value, minValue, maxValue, resourceMaxValue);
	else
		value = minValue = maxValue = resourceMaxValue = 0;
}	// ManufactureSchematicObject::getAllExperimentAttributeValues

//-----------------------------------------------------------------------

/**
 * Sets the limits of an experimental attribute.
 *
 * @param name					name of the attribute
 * @param minValue				min attribute value
 * @param maxValue				max attribute value
 * @param resourceMaxValue		resource max attribute value
 */
void ManufactureSchematicObject::setExperimentAttributeLimits(const StringId & name, 
	float minValue, float maxValue, float resourceMaxValue)
{
	ManufactureSchematicSynchronizedUi * sync = safe_cast<
		ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		sync->setAttributeLimits(name, minValue, maxValue, resourceMaxValue);
}	// ManufactureSchematicObject::setExperimentAttributeLimits

//-----------------------------------------------------------------------

/**
 * Returns the list of available appearances a crafter may choose from.
 *
 * @return the appearance list
 */
const std::vector<std::string> & ManufactureSchematicObject::getAppearances() const
{
static const std::vector<std::string> emptyList;

	const ManufactureSchematicSynchronizedUi * sync = safe_cast<
		const ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		return sync->getAppearances();
	return emptyList;
}	// ManufactureSchematicObject::getAppearances

//-----------------------------------------------------------------------

/**
 * Adds an appearance to the list of available appearances.
 *
 * @param appearance		the appearance filename
 */
void ManufactureSchematicObject::addAppearance(const std::string & appearance)
{
	ManufactureSchematicSynchronizedUi * sync = safe_cast<
		ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		sync->addAppearance(appearance);
}	// ManufactureSchematicObject::addAppearance

//-----------------------------------------------------------------------

/**
 * Returns the custom appearance objects made by this schematic will use.
 *
 * @return the appearance filename
 */
const std::string ManufactureSchematicObject::getCustomAppearance() const
{
	static const std::string emptyString;

	std::string result;
	if (!getObjVars().getItem("customAppearance",result))
		return emptyString;

	return result;
}	// ManufactureSchematicObject::getCustomAppearance

//-----------------------------------------------------------------------

/**
 * Sets the custom appearance objects made by this schematic will use.
 *
 * @param appearance		the appearance filename
 */
void ManufactureSchematicObject::setCustomAppearance(const std::string & appearance)
{
	m_customAppearance = appearance;
	setObjVarItem("customAppearance", appearance);
}	// ManufactureSchematicObject::setCustomAppearance(const std::string &)

//-----------------------------------------------------------------------

/**
 * Sets the custom appearance objects made by this schematic will use, based on 
 * the appearance list available to a crafter.
 *
 * @param index		index into the appearance list to use
 */
void ManufactureSchematicObject::setCustomAppearance(int index)
{
	const std::vector<std::string> & appearances = getAppearances();
	if (index < 0 || index >= static_cast<int>(appearances.size()))
		return;
	
	setCustomAppearance(appearances[index]);
}	// ManufactureSchematicObject::setCustomAppearance(int)

//-----------------------------------------------------------------------

/**
 * Returns customization data for the schematic.
 *
 * @param name		the name of the customization data
 * @param data		structure to be filled in with the customization data
 *
 * @return true on success, false if the customization name wasn't found
 *
bool ManufactureSchematicObject::getCustomization(const std::string & name, 
	Crafting::CustomInfo & data) const
{
	const DynamicVariableList * objvars = getObjVars();
	NOT_NULL(objvars);
	const DynamicVariableList * customs = safe_cast<const DynamicVariableList *>(objvars->getItemByName("cust"));
	if (customs == nullptr)	
		return false;

	const DynamicVariableList * custom = safe_cast<const DynamicVariableList *>(customs->getItemByName(name));
	if (custom == nullptr)
		return false;

	data.name = name;
	data.index = safe_cast<const DynamicVariableInt *>(custom->getItemByName("index"))->getValue();
	data.minIndex = safe_cast<const DynamicVariableInt *>(custom->getItemByName("mindex"))->getValue();
	data.maxIndex = safe_cast<const DynamicVariableInt *>(custom->getItemByName("maxdex"))->getValue();
	return true;
}	// ManufactureSchematicObject::getCustomization(const std::string &)
*/
//-----------------------------------------------------------------------

/**
 * Returns customization data for the schematic.
 *
 * @param index		index of the customization data
 * @param data		structure to be filled in with the customization data
 *
 * @return true on success, false if the index was out of range
 *
bool ManufactureSchematicObject::getCustomization(int index, 
	Crafting::CustomInfo & data) const
{
	const DynamicVariableList * objvars = getObjVars();
	NOT_NULL(objvars);
	const DynamicVariableList * customs = safe_cast<const DynamicVariableList *>(objvars->getItemByName("cust"));
	if (customs == nullptr)	
		return false;

	int count = customs->getCount();
	if (index < 0 || index >= count)
		return false;

	const DynamicVariableList * custom = safe_cast<const DynamicVariableList *>(customs->getItemByPosition(index));
	NOT_NULL(custom);
	data.name = custom->getName();
	data.index = safe_cast<const DynamicVariableInt *>(custom->getItemByName("index"))->getValue();
	data.minIndex = safe_cast<const DynamicVariableInt *>(custom->getItemByName("mindex"))->getValue();
	data.maxIndex = safe_cast<const DynamicVariableInt *>(custom->getItemByName("maxdex"))->getValue();
	return true;
}	// ManufactureSchematicObject::getCustomization(int)
*/
//-----------------------------------------------------------------------

/**
 * Returns the number of customization options available for the schematic.
 *
 * @return the customization count
 *
int ManufactureSchematicObject::getCustomizationsCount() const
{
	const DynamicVariableList * objvars = getObjVars();
	NOT_NULL(objvars);
	const DynamicVariableList * customs = safe_cast<const DynamicVariableList *>(objvars->getItemByName("cust"));
	if (customs == nullptr)	
		return 0;

	return customs->getCount();
}	// ManufactureSchematicObject::getCustomizationsCount
*/
//-----------------------------------------------------------------------

/**
 * Sets or adds customization data to the schematic.
 *
 * @param name			the customization data name
 * @param value			value of the customization data
 * @param minValue		minimum possible value of the customization data
 * @param maxValue		maximum possible value of the customization data
 */
void ManufactureSchematicObject::setCustomization(const std::string & name, 
	int value, int minValue, int maxValue)
{
	ManufactureSchematicSynchronizedUi * sync = safe_cast<ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync)
		sync->setCustomization(name, value, minValue, maxValue);
}	// ManufactureSchematicObject::setCustomization(const std::string &, int, int, int)

//-----------------------------------------------------------------------

/**
 * Sets the value of customization data of the schematic.
 *
 * @param index			index of the customization data
 * @param value			value of the customization data
 * @param prototype		prototype object to change the customization of
 *
 * @return true if the value was set, false if not
 */
bool ManufactureSchematicObject::setCustomization(int index, int value, 
	ServerObject & prototype)
{
	ManufactureSchematicSynchronizedUi * sync = safe_cast<ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync == nullptr)
		return false;

	const std::string & customName = sync->getCustomizationName(index);
	if (customName.empty())
		return false;

	return setCustomization(customName, value, prototype);
}	// ManufactureSchematicObject::setCustomization(int, int, ServerObject *)

//-----------------------------------------------------------------------

/**
 * Sets the value of customization data of the schematic.
 *
 * @param name			the customization data name
 * @param value			value of the customization data
 * @param prototype		prototype object to change the customization of
 *
 * @return true if the value was set, false if not
 */
bool ManufactureSchematicObject::setCustomization(const std::string & name, 
	int value, ServerObject & prototype)
{
	ManufactureSchematicSynchronizedUi * sync = safe_cast<ManufactureSchematicSynchronizedUi *>(getSynchronizedUi ());
	if (sync == nullptr)
		return false;

	// make sure the value is within range
	if (value < sync->getMinCustomization(name))
		return false;
	if (value > sync->getMaxCustomization(name))
		return false;
	sync->setCustomization(name, value);

	// update the prototype
	prototype.customize(name, value);

	// save the customization string
	CustomizationDataProperty * const cdProperty = safe_cast<CustomizationDataProperty*>(prototype.getProperty(CustomizationDataProperty::getClassPropertyId()));
	if (cdProperty != nullptr)
	{
		CustomizationData *const customizationData = cdProperty->fetchCustomizationData();
		if (customizationData != nullptr)
		{
			m_appearanceData = customizationData->writeLocalDataToString();
			setObjVarItem("customization_data", m_appearanceData.get());
		}
	}

	return true;
}	// ManufactureSchematicObject::setCustomization(const std::string &, int, ServerObject *)

//-----------------------------------------------------------------------

/**
 * Computes the complexity of the schematic based on the ingredients being used.
 */
void ManufactureSchematicObject::computeComplexity()
{
static const StringId COMPLEXITY_ID("crafting", "complexity");

	const DraftSchematicObject * draft = DraftSchematicObject::getSchematic(
		m_draftSchematic.get());
	NOT_NULL(draft);

	float complexity = draft->getComplexity();

	DynamicVariableList::NestedList slots(getObjVars(),"ingr");
	{
		for (DynamicVariableList::NestedList::const_iterator i(slots.begin()); i!=slots.end(); ++i)
		{
			DynamicVariableList::NestedList slot(i.getNestedList());
			int varType=0;
			float varComplex=0;
			if (slot.getItem("type",varType) && slot.getItem("complex",varComplex) && static_cast<enum Crafting::IngredientType>(varType) != Crafting::IT_none)
				complexity += varComplex;
		}
	}

	// set the current value of the "complexity" attribute
	setAttribute(COMPLEXITY_ID, complexity); 

	setComplexity(complexity);
}	// ManufactureSchematicObject::computeComplexity

//-----------------------------------------------------------------------

/**
 * Tells us that a FactoryObject has references that are being used as components
 * during crafting. This will allow us to remove/clean up the components.
 *
 * @param factory		the factory object
 */
void ManufactureSchematicObject::addCraftingFactory(const FactoryObject & factory)
{
	if (isAuthoritative())
	{
		CachedNetworkId id(factory);

		if (m_factories.find(id) < 0)
			m_factories.push_back(id);
		else
		{
			WARNING(true, ("ManufactureSchematicObject::addCraftingFactory %s tried "
				"to add factory %s that was already added", 
				getNetworkId().getValueString().c_str(),
				factory.getNetworkId().getValueString().c_str()));
		}
	}
	else
	{
		WARNING(true, ("ManufactureSchematicObject::addCraftingFactory called on "
			"non-authoritative object %s", getNetworkId().getValueString().c_str()));
	}
}	// ManufactureSchematicObject::addCraftingFactory

//-----------------------------------------------------------------------

/**
 * Tells us that a FactoryObject is being removed from a crafting session.
 * In general this should only be called if the factory is being removed by
 * some unexpected event, not as part of the crafting process.
 *
 * @param factory		the factory object
 */
void ManufactureSchematicObject::removeCraftingFactory(const FactoryObject & factory)
{
	if (isAuthoritative())
	{
		CachedNetworkId id(factory);

		int index = m_factories.find(id);
		if (index >= 0)
			m_factories.erase(index);
	}
	else
	{
		WARNING(true, ("ManufactureSchematicObject::removeCraftingFactory called on "
			"non-authoritative object %s", getNetworkId().getValueString().c_str()));
	}
}	// ManufactureSchematicObject::removeCraftingFactory

//-----------------------------------------------------------------------

/**
 * Add a component ingredient to the schematic's volume container.
 *
 * @param component		the ingredient to add
 *
 * @return true on success, false on fail
 */
bool ManufactureSchematicObject::addIngredient(ServerObject & component)
{
	Container::ContainerErrorCode tmp = Container::CEC_Success;		
	return ContainerInterface::transferItemToVolumeContainer(*this, component, nullptr, tmp);
}	// ManufactureSchematicObject::addIngredient

//-----------------------------------------------------------------------

/**
 * Add a resource ingredient to the schematic's objvars.
 *
 * @param resourceId	the resource type id
 * @param count			the amount of resource
 * @param sourceId		who mined the resource
 *
 * @return true on success, false on fail
 */
bool ManufactureSchematicObject::addIngredient(const NetworkId & resourceId, int count, const NetworkId & sourceId)
{
	std::string objvarName(OBJVAR_STORED_RESOURCES + "." + resourceId.getValueString() + "." + sourceId.getValueString());
	
	// get how much the source has already contributed the resource
	int currentCount = 0;
	getObjVars().getItem(objvarName, currentCount);

	// update the source's count
	currentCount += count;
	setObjVarItem(objvarName, currentCount);

	return true;
}	// ManufactureSchematicObject::addIngredient

//-----------------------------------------------------------------------

/**
 * Remove a component ingredient from the schematic's volume container.
 *
 * @param component		the component to remove
 * @param destination	the container to put the component into
 *
 * @return true on success, false on fail
 */
bool ManufactureSchematicObject::removeIngredient(ServerObject & component, 
	ServerObject & destination)
{
	// if the component is a factory, treat is differently
	FactoryObject * factory = dynamic_cast<FactoryObject *>(&component);
	if (factory != nullptr)
	{
		// if the factory is contained by us, move it to the destination,
		// making sure that its count is 1; if it is not contained by us,
		// just increase its count
		if (ContainerInterface::getContainedByObject(*factory) == this)
		{
			// since components are removed one at a time from us, the factory
			// should only contain one item when it is removed
			if (factory->getCount() > 1)
				factory->removeCraftingReferences(factory->getCount() - 1);
			Container::ContainerErrorCode errCode;
			return ContainerInterface::transferItemToVolumeContainer(destination, 
				*factory, nullptr, errCode);
		}
		else
		{
			// for the moment, just increment the factory count, even if the factory
			// isn't in the destination container
			factory->addCraftingReferences(1);
			return true;
		}
	}
	else
	{
		// make sure the component is in our container
		Object * container = ContainerInterface::getContainedByObject(component);
		if (container == nullptr || container->getNetworkId() != getNetworkId())
		{
			FactoryObject * factory = dynamic_cast<FactoryObject *>(container);
			if (factory != nullptr)
				return removeIngredient(*factory, destination);
			return false;
		}

		Container::ContainerErrorCode errCode;
		return ContainerInterface::transferItemToVolumeContainer(destination, component,
			nullptr, errCode);
	}
}	// ManufactureSchematicObject::removeIngredient

//-----------------------------------------------------------------------

/**
 * Remove a resource ingredient from the schematic's objvars.
 *
 * @param resourceId	the resource type id
 * @param count			the amount of resource
 * @param destination	the resource container to put the resource in
 *
 * @return true on success, false on fail
 */
bool ManufactureSchematicObject::removeIngredient(const NetworkId & resourceId, int count, ResourceContainerObject & destination)
{
	// make sure the destination holds the resource type
	if (destination.getResourceTypeId() != resourceId && 
		destination.getResourceTypeId() != NetworkId::cms_invalid)
	{
		return false;
	}

	// make sure the destination has enough room
	if (destination.getMaxQuantity() - destination.getQuantity() < count)
		return false;

	// find the resource from our objvars
	std::vector<std::string> removedObjvars;
	const std::string objvarName(OBJVAR_STORED_RESOURCES + "." + resourceId.getValueString());
	{
		DynamicVariableList::NestedList sources(getObjVars(), objvarName);
		for (DynamicVariableList::NestedList::const_iterator i = sources.begin(); count > 0 && i != sources.end(); ++i)
		{
			int sourceCount = 0;
			i.getValue(sourceCount);
			if (sourceCount > count)
			{
				IGNORE_RETURN(destination.addResource(resourceId, count, NetworkId(i.getName())));
				setObjVarItem(i.getNameWithPath(), sourceCount - count);
				count = 0;
			}
			else
			{
				IGNORE_RETURN(destination.addResource(resourceId, sourceCount, NetworkId(i.getName())));
				removedObjvars.push_back(i.getNameWithPath());
				count -= sourceCount;
			}
		}
	}
	{
		// remove sources that have all their contribution removed
		for (std::vector<std::string>::const_iterator i = removedObjvars.begin();
			i != removedObjvars.end(); ++i)
		{
			removeObjVarItem(*i);
		}
	}

	if (count > 0)
		return false;

	return true;
}	// ManufactureSchematicObject::removeIngredient

//----------------------------------------------------------------------

bool ManufactureSchematicObject::hasIngredient (const NetworkId & componentId) const
{
	const ServerObject * const obj = safe_cast<const ServerObject *>(NetworkIdManager::getObjectById (componentId));


	if (obj && ContainerInterface::getContainedByObject (*obj) == this)
		return true;

	return false;
}

//-----------------------------------------------------------------------

/**
 * Destroys all the ingredients in the volume container and resets our slots.
 */
void ManufactureSchematicObject::destroyAllIngredients()
{
	if (isAuthoritative())
	{
		// clear any factories being used
		if (!m_factories.empty())
		{
			for (Archive::AutoDeltaVector<CachedNetworkId>::const_iterator iter =
				m_factories.begin(); iter != m_factories.end(); ++iter)
			{
				FactoryObject * factory = safe_cast<FactoryObject *>((*iter).getObject());
				if (factory != nullptr)
				{
					factory->endCraftingSession();
				}
			}
			m_factories.clear();
		}

		// empty our volume container
		VolumeContainer * const container = ContainerInterface::getVolumeContainer(*this);
		if (container != nullptr)
		{
			// for a stacked ingredient, if there are still more than one items in the stack,
			// "consume" 1 item from the stack, and return it to the player's inventory
			std::set<ServerObject*> itemsToReturnToInventory;
			ServerObject* inventory = nullptr;

			ContainerIterator iter;
			for (iter = container->begin(); iter != container->end(); ++iter)
			{
				CachedNetworkId & itemId = *iter;
				if (itemId.getObject() != nullptr)
				{
					bool destroyItem = true;
					TangibleObject* to = safe_cast<ServerObject*>(itemId.getObject())->asTangibleObject();
					if (to && (to->getCount() > 1) && (dynamic_cast<FactoryObject *>(to) == nullptr))
					{
						if (inventory == nullptr)
						{
							ServerObject * o = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*to));
							while (o)
							{
								if (o->asCreatureObject() && PlayerCreatureController::getPlayerObject(o->asCreatureObject()))
								{
									inventory = o->asCreatureObject()->getInventory();
									break;
								}

								o = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*o));
							}
						}

						if (inventory)
						{
							to->setCount(to->getCount() - 1);
							itemsToReturnToInventory.insert(to);
							destroyItem = false;
						}
					}

					if (destroyItem)
					{
						safe_cast<ServerObject*>(itemId.getObject())->permanentlyDestroy(DeleteReasons::Consumed);
					}
				}
				else
				{
					WARNING_STRICT_FATAL(true, ("Manf schematic %s has ingredient id %s with no object", getNetworkId().getValueString().c_str(),itemId.getValueString().c_str()));
				}
			}

			if (inventory && !itemsToReturnToInventory.empty())
			{
				Container::ContainerErrorCode errCode;
				for (std::set<ServerObject*>::const_iterator iter = itemsToReturnToInventory.begin(); iter != itemsToReturnToInventory.end(); ++iter)
				{
					if (!ContainerInterface::transferItemToVolumeContainer(*inventory, **iter, nullptr, errCode))
					{
						(*iter)->permanentlyDestroy(DeleteReasons::Consumed);
					}
				}
			}
		}

		// dump the resource objvars
		eraseObjVarListEntries(OBJVAR_STORED_RESOURCES);

		// reset our slots
		const DynamicVariableList::NestedList slots(getObjVars(),"ingr");

		int count = slots.getCount();
		for (int i = 0; i < count; ++i)
		{
			clearSlot(i);
		}
	}
	else
	{
		WARNING(true, ("ManufactureSchematicObject::destroyAllIngredients called on "
			"non-authoritative object %s", getNetworkId().getValueString().c_str()));
	}
}	// ManufactureSchematicObject::destroyAllIngredients

//-----------------------------------------------------------------------

/**
 * Creates an item with this schematic. It is assumed that the ingredients 
 * needed to make it are available. Note that this function is duplicated in
 * FactoryObject::manufactureObject; if this function changes, that one should 
 * too.
 *
 * @param creatorId			the id of the player making the object
 * @param container			the container to create the object in
 * @param containerSlotId	the slot in the container to put the object in
 * @param prototype			flag that we are creating a prototype, and shouldn't 
 *							check for the correct ingredients
 *
 * @return the manufactured object
 */
ServerObject * ManufactureSchematicObject::manufactureObject(const NetworkId & creatorId, ServerObject & container, const SlotId & containerSlotId, bool prototype)
{
	if (getCount() <= 0)
		return nullptr;

	const DraftSchematicObject * draftSchematic = DraftSchematicObject::getSchematic(m_draftSchematic.get());

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
	TangibleObject * const object = 
		dynamic_cast<TangibleObject *>(ServerWorld::createNewObject(*draftSchematic->getCraftedObjectTemplate(), container, containerSlotId, false));
	if (object == nullptr)
	{
		DEBUG_WARNING(true, ("Player %s failed to create object for template "
			"%s.\n", creatorId.getValueString().c_str(), 
			draftSchematic->getCraftedObjectTemplate()->getName()));
		return nullptr;
	}

	if (!getAssignedObjectName().empty())
		object->setObjectName(getAssignedObjectName());

	// set the object appearance
	const std::string & appearance = getCustomAppearance();
	if (!appearance.empty())
		object->setCustomAppearance(appearance);

	// set the customization data for the object
	if (!m_appearanceData.get().empty())
		object->setAppearanceData(m_appearanceData.get());

	// mark the item as having been created by us
	object->setCraftedId(getNetworkId());
	object->setCraftedType(draftSchematic->getCategory());
	object->setCreatorId(creatorId);
	object->setOwnerId(creatorId);
	
	// set the creator xp type from the schematic template
	const ServerObjectTemplate * schematicTemplate = safe_cast<const 
		ServerObjectTemplate *>(draftSchematic->getObjectTemplate());
	if (schematicTemplate != nullptr && schematicTemplate->getXpPointsCount() > 0)
	{
		ServerObjectTemplate::Xp xpData;
		schematicTemplate->getXpPoints(xpData, 0);
		object->setCreatorXpType(xpData.type);
	}

	if (!prototype)
	{
		if (!setObjectComponents(object, true))
		{
			object->permanentlyDestroy(DeleteReasons::BadContainerTransfer);
			return nullptr;
		}

		// allow the schematic scripts to modify the object
		// prototype objects are modified at the end of the crafting session
		m_isMakingObject = true;
		ScriptParams params;
		params.addParam(creatorId);
		params.addParam(object->getNetworkId());
		params.addParam(*this);
		params.addParam(false);
		params.addParam(true);
		int result = getScriptObject()->trigAllScripts(Scripting::TRIG_MANUFACTURE_OBJECT, params);
		m_isMakingObject = false;
		if (result == SCRIPT_OVERRIDE)
		{
			object->permanentlyDestroy(DeleteReasons::Script);
			return nullptr;
		}

		incrementCount(-1);
		clearSlotSources();

		if (getCount() == 0)
		{
			// delete myself
			permanentlyDestroy(DeleteReasons::Consumed);
		}		

	}
	else
	{
		// flag the object as a prototype
		setObjVarItem(OBJVAR_CRAFTING_IS_PROTOTYPE, true);
	}
	
	return object;
}	// ManufactureSchematicObject::manufactureObject(const NetworkId &, ServerObject &, const SlotId &, bool)

//-----------------------------------------------------------------------

/**
 * Creates an item with this schematic. This is for use in creating a finished 
 * object without ingredients.
 *
 * @position	where to create the object
 *
 * @return the manufactured object
 */
ServerObject * ManufactureSchematicObject::manufactureObject(const Vector & position)
{
	const DraftSchematicObject * draftSchematic = DraftSchematicObject::getSchematic(
		m_draftSchematic.get());
	if (draftSchematic == nullptr)
	{
		return nullptr;
	}
	if (draftSchematic->getCraftedObjectTemplate() == nullptr)
	{
		DEBUG_WARNING(true, ("Draft schematic %s does not have a craftable "
			"object template!\n", draftSchematic->getTemplateName()));
		return nullptr;
	}

	Transform tr;
	tr.setPosition_p(position);
	// create the item
	TangibleObject * object = dynamic_cast<TangibleObject *>(ServerWorld::createNewObject(
		*draftSchematic->getCraftedObjectTemplate(), tr, 0,
		false));
	if (object == nullptr)
	{
		DEBUG_WARNING(true, ("Failed to create object for template "
			"%s.\n", draftSchematic->getCraftedObjectTemplate()->getName()));
		return nullptr;
	}

	if (!getAssignedObjectName().empty())
		object->setObjectName(getAssignedObjectName());

	// set the object appearance
	const std::string & appearance = getCustomAppearance();
	if (!appearance.empty())
		object->setCustomAppearance(appearance);

	// set the customization data for the object
	if (!m_appearanceData.get().empty())
		object->setAppearanceData(m_appearanceData.get());

	// mark the item as having been created by us
	object->setCraftedId(getNetworkId());
	object->setCraftedType(draftSchematic->getCategory());
	object->setCreatorId(NetworkId::cms_invalid);
	
	// flag the object as a prototype
	setObjVarItem(OBJVAR_CRAFTING_IS_PROTOTYPE, true);
	
	return object;
}	// ManufactureSchematicObject::manufactureObject(const Vector &)

//-----------------------------------------------------------------------

/**
 * Transfers the components in the schematic's container to an object it has 
 * created.
 *
 * @param object			the object that was created
 * @param checkResources	flag to verify there are enough resources to craft 
 *							the object
 *
 * @return true on success, false if there was an error
 */
bool ManufactureSchematicObject::setObjectComponents(TangibleObject * object, bool checkResources)
{
	static const uint32 INVALID_CRC = Crc::normalizeAndCalculate("invalid");
	int i;

	const DraftSchematicObject * draftSchematic = DraftSchematicObject::getSchematic(m_draftSchematic.get());
	NOT_NULL(draftSchematic);

	// get the visible components string; if missing, flag this as the 1st 
	// item made
	bool firstItem = false;
	std::vector<int> visibleComponents;
	if (!getObjVars().getItem("visComp",visibleComponents))
		firstItem = true;

	// check the objects in the schematic volume container to make sure they match 
	VolumeContainer * volumeContainer = ContainerInterface::getVolumeContainer(*this);
	if (volumeContainer == nullptr)
	{
		DEBUG_WARNING(true, ("Manf schematic %s does not have a volume container!",
			getNetworkId().getValueString().c_str()));
		return false;
	}

	// for a stacked ingredient, if there are still more than one items in the stack,
	// "consume" 1 item from the stack, and return it to the player's inventory
	std::set<ServerObject*> itemsToReturnToInventory;
	ServerObject* inventory = nullptr;

	DynamicVariableList::NestedList slots(getObjVars(),OBJVAR_SLOTS);

	// check the objects in the schematic volume container to make sure they match 
	// the ingredients needed
	Crafting::IngredientSlot slotData;
	ContainerIterator iter;
	Crafting::Ingredients::const_iterator ingredientIter;
	std::vector<std::pair<NetworkId, int> > sources;
	int slotCount = getSlotsCount();
	for (i = 0; i < slotCount; ++i)
	{
		if (!getSlot(i, slotData, false))
			break;
		if (slotData.ingredients.empty())
			break;

		DynamicVariableList::NestedList slot(slots,i);
		DynamicVariableList::NestedList slotIngredients(slot,OBJVAR_INGREDIENTS);

		sources.clear();

		for (ingredientIter = slotData.ingredients.begin(); 
			ingredientIter != slotData.ingredients.end(); ++ingredientIter)
		{
			const Crafting::SimpleIngredient * ingredient = (*ingredientIter).get();
			NOT_NULL(ingredient);
			int ingredientCount = ingredient->count;
			if (ingredientCount <= 0)
			{
				WARNING_STRICT_FATAL(true, ("Manufacture schematic %s has "
					"invalid ingredient %s count %d", 
					getNetworkId().getValueString().c_str(),
					ingredient->ingredient.getValueString().c_str(), ingredientCount));
				break;
			}
			if (slotData.ingredientType == Crafting::IT_item || 
				slotData.ingredientType == Crafting::IT_template ||
				slotData.ingredientType == Crafting::IT_templateGeneric ||
				slotData.ingredientType == Crafting::IT_schematic ||
				slotData.ingredientType == Crafting::IT_schematicGeneric)
			{
				const Crafting::ComponentIngredient * component = dynamic_cast<
					const Crafting::ComponentIngredient *>(ingredient);
				NOT_NULL(component);
				
				// find an ingredient in our volume container that matches the 
				// component
				for (iter = volumeContainer->begin(); 
					iter != volumeContainer->end() && ingredientCount > 0; 
					++iter)
				{
					Container::ContainedItem & item = *iter;
					TangibleObject * const testComponent = dynamic_cast<TangibleObject *>(item.getObject());
					if (testComponent == nullptr)
						continue;
					if (itemsToReturnToInventory.count(testComponent) > 0)
						continue;
					FactoryObject * crate = nullptr;
					bool found = false;
					bool foundCrate = false;
					if (component->ingredient != NetworkId::cms_invalid)
					{
						if (component->ingredient == testComponent->getCraftedId())
						{
							found = true;
							
							// see if the test component is a crate
							crate = dynamic_cast<FactoryObject *>(testComponent);
							if (crate != nullptr && crate->getCount() == ingredientCount)
							{
								foundCrate = true;
							}
						}
					}
					else
					{
						if (testComponent->getObjectTemplateName() == component->templateName)
							found = true;
					}
					if (found)
					{
						if (foundCrate)
							ingredientCount = 0;
						else
							--ingredientCount;

						if (testComponent->isCrafted())
						{
							// set the xp type for the component
							int ingredientIndex = ingredientIter - 
								slotData.ingredients.begin();
							DynamicVariableList::NestedList ingredientVars(slotIngredients,ingredientIndex);
							{
								setObjVarItem(ingredientVars.getContextName()+
									OBJVAR_COMPONENT_CRAFTED_XP_TYPE, 
									testComponent->getCreatorXpType());
							}

							// add xp info to the sources list
							sources.push_back(std::make_pair(testComponent->getCreatorId(),
								testComponent->getCreatorXp()));
						}

						// if the component is visible when attached to the object,
						// get it's id from the component table and add it to the
						// visible components list
						if (firstItem)
						{
							ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
							if (draftSchematic->getSlot(draftSlot, slotData.draftSlotIndex))
							{
								if (!draftSlot.appearance.empty())
								{
									// check with the table for this type of component
									const std::string & componentTableName = ConfigServerGame::getCraftingComponentTableFile(draftSlot.appearance);
									if (!componentTableName.empty())
									{
										DataTable * componentTable = DataTableManager::getTable(componentTableName, true);
										if (componentTable != nullptr)
										{
											uint32 value = INVALID_CRC;
											if (draftSlot.appearance == "component")
											{
												// use the component name directly
												int row;
												if (foundCrate)
													row = componentTable->searchColumnString(0, crate->getContainedSharedTemplateName());
												else
													row = componentTable->searchColumnString(0, testComponent->getSharedTemplateName());
												if (row >= 0)
												{
													value = componentTable->getIntValue(0, row);
												}
											}
											else
											{
												// check if the component is compatable with the crafted object, and use a default component if not
												int row = componentTable->searchColumnString(0, object->getObjectTemplateName());
												int column;
												if (foundCrate)
													column = componentTable->findColumnNumber(crate->getItemCustomAppearance());
												else
													column = componentTable->findColumnNumber(testComponent->getCustomAppearance());
												if (row >= 0 && column >= 0)
												{
													value = componentTable->getIntValue(column, row);
												}
												if (row >= 0 && value == INVALID_CRC)
												{
													column = componentTable->findColumnNumber("default");
													if (column >= 0)
														value = componentTable->getIntValue(column, row);
												}
											}
											if (value != INVALID_CRC)
											{
												visibleComponents.push_back(value);
											}
										}
									}
								}
							}
						}

						bool destroyItem = true;
						if ((testComponent->getCount() > 1) && (dynamic_cast<FactoryObject *>(testComponent) == nullptr))
						{
							if (inventory == nullptr)
							{
								ServerObject * o = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*testComponent));
								while (o)
								{
									if (o->asCreatureObject() && PlayerCreatureController::getPlayerObject(o->asCreatureObject()))
									{
										inventory = o->asCreatureObject()->getInventory();
										break;
									}

									o = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*o));
								}
							}

							if (inventory)
							{
								testComponent->setCount(testComponent->getCount() - 1);
								itemsToReturnToInventory.insert(testComponent);
								destroyItem = false;
							}
						}

						if (destroyItem)
						{
							testComponent->permanentlyDestroy(DeleteReasons::Consumed);
						}
					}
				}
			}
			else if (checkResources && (
				slotData.ingredientType == Crafting::IT_resourceType || 
				slotData.ingredientType == Crafting::IT_resourceClass))
			{
				//@todo: check resources against the OBJVAR_STORED_RESOURCES data
			}
			else if (checkResources)
				break;
		}
		if (ingredientIter != slotData.ingredients.end())
			break;

		if (!sources.empty())
		{
			// set up the sources so they'll get experience
			setSlotSources(i, sources);
		}
	}

	if (inventory && !itemsToReturnToInventory.empty())
	{
		Container::ContainerErrorCode errCode;
		for (std::set<ServerObject*>::const_iterator iter = itemsToReturnToInventory.begin(); iter != itemsToReturnToInventory.end(); ++iter)
		{
			if (!ContainerInterface::transferItemToVolumeContainer(*inventory, **iter, nullptr, errCode))
			{
				(*iter)->permanentlyDestroy(DeleteReasons::Consumed);
			}
		}

		itemsToReturnToInventory.clear();
	}

	if (i != slotCount)
		return false;

	// see if we have anything left in our volume container, and delete them
	if (volumeContainer->getNumberOfItems() > 0)
	{
		WARNING(true, ("ManufactureSchematicObject::setObjectComponents schematic "
			"%s(%s) has extra ingredients! Deleting them.", 
			getNetworkId().getValueString().c_str(), 
			draftSchematic->getCraftedObjectTemplate()->getName()));

		for (iter = volumeContainer->begin(); iter != volumeContainer->end(); ++iter)
		{
			if ((*iter).getObject() != nullptr)
			{
				ServerObject * temp = safe_cast<ServerObject*>((*iter).getObject());

				bool destroyItem = true;
				TangibleObject* to = temp->asTangibleObject();
				if (to && (to->getCount() > 1) && (dynamic_cast<FactoryObject *>(to) == nullptr))
				{
					if (inventory == nullptr)
					{
						ServerObject * o = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*to));
						while (o)
						{
							if (o->asCreatureObject() && PlayerCreatureController::getPlayerObject(o->asCreatureObject()))
							{
								inventory = o->asCreatureObject()->getInventory();
								break;
							}

							o = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*o));
						}
					}

					if (inventory)
					{
						to->setCount(to->getCount() - 1);
						itemsToReturnToInventory.insert(to);
						destroyItem = false;
					}
				}

				if (destroyItem)
				{
					WARNING(true, ("ManufactureSchematicObject::setObjectComponents "
						"deleting extra ingredient %s(%s).", 
						(*iter).getValueString().c_str(),
						temp->getObjectTemplateName()));
					temp->permanentlyDestroy(DeleteReasons::Replaced);
				}
			}
			else
			{
				WARNING(true, ("ManufactureSchematicObject::setObjectComponents "
					"extra ingredient %s has no object.", 
					(*iter).getValueString().c_str()));
			}
		}
	}

	eraseObjVarListEntries(OBJVAR_STORED_RESOURCES);

	// tell the object what components are visible
	object->setVisibleComponents(visibleComponents);
	if (firstItem)
	{
		// add the visible component list to our objvars
		setObjVarItem("visComp", visibleComponents);
	}

	if (inventory && !itemsToReturnToInventory.empty())
	{
		Container::ContainerErrorCode errCode;
		for (std::set<ServerObject*>::const_iterator iter = itemsToReturnToInventory.begin(); iter != itemsToReturnToInventory.end(); ++iter)
		{
			if (!ContainerInterface::transferItemToVolumeContainer(*inventory, **iter, nullptr, errCode))
			{
				(*iter)->permanentlyDestroy(DeleteReasons::Replaced);
			}
		}
	}

	return true;
}	// ManufactureSchematicObject::setObjectComponents

// ----------------------------------------------------------------------

/**
 * Sends info about the ingredients needed by the schematic to a player.
 *
 * @param player		the player to send the data to
 */
void ManufactureSchematicObject::requestSlots(ServerObject & player) const
{
	Crafting::IngredientSlot slot;
	MessageQueueDraftSlots::Slot slotInfo;
	MessageQueueDraftSlots::Option optionInfo;
	Crafting::Ingredients::const_iterator ingredientIter;

	MessageQueueDraftSlotsQueryResponse * const message = 
		new MessageQueueDraftSlotsQueryResponse(std::make_pair(m_draftSchematic.get(), 
		m_draftSchematicSharedTemplate.get()));

	int slotCount = getSlotsCount();
	for (int i = 0; i < slotCount; ++i)
	{
		if (!getSlot(i, slot, false))
			continue;

		if (slot.ingredientType != Crafting::IT_invalid && 
			slot.ingredientType != Crafting::IT_none)
		{
			slotInfo.name = slot.name;
			slotInfo.optional = false;
			optionInfo.type = slot.ingredientType;

			// get the number of ingredients needed
			optionInfo.amountNeeded = 0;
			for (ingredientIter = slot.ingredients.begin(); 
				ingredientIter != slot.ingredients.end(); ++ingredientIter)
			{
				optionInfo.amountNeeded += (*ingredientIter)->count;
			}

			// get the ingredient name
			const NetworkId & ingredientId = slot.ingredients.front()->ingredient;
			switch (slot.ingredientType)
			{
				case Crafting::IT_item:
				case Crafting::IT_template:
				case Crafting::IT_templateGeneric:
				case Crafting::IT_schematic:
				case Crafting::IT_schematicGeneric:
					{
						const TangibleObject * const ingredient = dynamic_cast<const TangibleObject *>(NetworkIdManager::getObjectById(ingredientId));
						if (ingredient != nullptr)
						{
							optionInfo.ingredient = ingredient->getEncodedObjectName();
							// since names can be duplicated, concatinate the id of
							// the manf schematic to the name
							if (ingredient->isCrafted())
							{
								optionInfo.ingredient += Unicode::narrowToWide(TangibleObject::generateSerialNumber(ingredient->getCraftedId()));
							}
							else
							{
								optionInfo.ingredient += Unicode::narrowToWide(TangibleObject::generateSerialNumber(ingredient->getNetworkId()));
							}
						}
						else
							optionInfo.ingredient = Unicode::narrowToWide("unknown");
					}
					break;

				case Crafting::IT_resourceType:
				case Crafting::IT_resourceClass:
					{
						ResourceTypeObject const * const ingredient = ServerUniverse::getInstance().getResourceTypeById(ingredientId);
						if (ingredient != nullptr)
							optionInfo.ingredient = Unicode::narrowToWide(ingredient->getResourceName());
						else
							optionInfo.ingredient = Unicode::narrowToWide("unknown");
					}
					break;
			
				default:
					break;
			}
			slotInfo.options.push_back(optionInfo);
			message->addSlot(slotInfo);
			slotInfo.options.clear();
		}
	}
}	// ManufactureSchematicObject::requestSlots

//----------------------------------------------------------------------

void ManufactureSchematicObject::getIngredientInfo (IngredientInfoVector & iiv) const
{
	Crafting::IngredientSlot slot;
	Unicode::String ingredientName;
	
	typedef std::map<Unicode::String, std::pair<bool, int> > IngredientMap;
	IngredientMap imap;
	
	// compute the total ingredients needed for the schematic
	int slotCount = getSlotsCount();
	for (int i = 0; i < slotCount; ++i)
	{
		if (!getSlot(i, slot, false))
			continue;

		if (slot.ingredientType == Crafting::IT_invalid ||
			slot.ingredientType == Crafting::IT_none)
			continue;

		// get the number of ingredients needed
		int amountNeeded = 0;
		for (Crafting::Ingredients::const_iterator ingredientIter = slot.ingredients.begin(); 
		ingredientIter != slot.ingredients.end(); ++ingredientIter)
		{
			amountNeeded += (*ingredientIter)->count;
		}
		
		ingredientName.clear ();
		
		if (amountNeeded <= 0)
			continue;
		
		bool isComponent = false;
		
		// get the ingredient name
		const Crafting::SimpleIngredient * const info = slot.ingredients.front().get();
		NOT_NULL(info);

		const NetworkId & ingredientId = info->ingredient;
		switch (slot.ingredientType)
		{
		case Crafting::IT_item:
		case Crafting::IT_template:
		case Crafting::IT_schematic:
			{
				const Crafting::ComponentIngredient * const component = safe_cast<const Crafting::ComponentIngredient *>(info);
				NOT_NULL (component);

				ingredientName = component->componentName;
				// since names can be duplicated, concatenate the id of
				// the manf schematic to the name
				if (component->ingredient != NetworkId::cms_invalid)
				{
					//-- nullptr-separate the string if needed
					if (!ingredientName.empty () && ingredientName [0] == '@')
						ingredientName.push_back ('\0');

					ingredientName.push_back (' ');
					ingredientName += Unicode::narrowToWide (TangibleObject::generateSerialNumber (component->ingredient));
				}
				
				isComponent = true;
			}
			break;
			
		case Crafting::IT_templateGeneric:
		case Crafting::IT_schematicGeneric:
			{
				const Crafting::ComponentIngredient * const component = safe_cast<const Crafting::ComponentIngredient *>(info);
				NOT_NULL (component);
				
				const ServerObjectTemplate * const serverTemplate = dynamic_cast<const ServerObjectTemplate *>(ObjectTemplateList::fetch (component->templateName));
				
				if (serverTemplate)
				{
					const std::string & sharedTemplateName = serverTemplate->getSharedTemplate ();
					const SharedObjectTemplate * const sharedTemplate = dynamic_cast<const SharedObjectTemplate *>(ObjectTemplateList::fetch (sharedTemplateName));
					
					if (sharedTemplate)
					{
						ingredientName.push_back ('@');
						ingredientName += Unicode::narrowToWide (sharedTemplate->getObjectName ().getCanonicalRepresentation ());
						sharedTemplate->releaseReference ();
					}
					else
						WARNING (true, ("ManufactureSchematicObject templateGeneric shared template [%s] not found for server template [%s]", sharedTemplateName.c_str (), component->templateName.c_str ()));
					
					serverTemplate->releaseReference ();
				}
				else
					WARNING (true, ("ManufactureSchematicObject templateGeneric server template [%s] not found", component->templateName.c_str ()));
				
				isComponent = true;
				
			}
			break;
			
		case Crafting::IT_resourceType:
		case Crafting::IT_resourceClass:
			{
				ResourceTypeObject const * const ingredient = ServerUniverse::getInstance().getResourceTypeById(ingredientId);
				if (ingredient != nullptr)
					ingredientName = Unicode::narrowToWide(ingredient->getResourceName ());
				}
			break;
			
		default:
			break;
		}
		
		if (!ingredientName.empty ())
		{
			const IngredientMap::iterator result = imap.find (ingredientName);
			if (result == imap.end())
				imap.insert (std::make_pair(ingredientName, std::make_pair (isComponent, amountNeeded)));
			else
			{
				int & totalAmountNeeded = (*result).second.second;
				totalAmountNeeded += amountNeeded;
			}
		}
	}
	
	iiv.reserve (iiv.size () + imap.size ());
	{
		for (IngredientMap::const_iterator iter = imap.begin(); iter != imap.end(); ++iter)
		{
			const Unicode::String & name = (*iter).first;
			const bool isComponent       = (*iter).second.first;
			const int amount             = (*iter).second.second;

			iiv.push_back (IngredientInfo (name, std::make_pair (isComponent, amount)));
		}
	}
}

//----------------------------------------------------------------------

/**
 * Returns the attribute bonuses this object grants when equipped.
 *
 * @param attribBonuses		list to be filled in with the bonuses
 */
void ManufactureSchematicObject::getAttribBonuses(std::vector<std::pair<int, int> > & attribBonuses) const
{
	attribBonuses.clear();

	DynamicVariableList::NestedList mods(getObjVars(), OBJVAR_ATTRIBUTE_BONUS);

	for (DynamicVariableList::NestedList::const_iterator i = mods.begin();
		i != mods.end(); ++i)
	{
		int bonus = 0;
		i.getValue(bonus);
		int attribute = atoi(i.getName().c_str());
		if (isdigit(i.getName()[0]) && attribute >= 0 && 
			attribute < Attributes::NumberOfAttributes)
		{
			if (bonus > ConfigServerGame::getMaxItemAttribBonus())
				bonus = ConfigServerGame::getMaxItemAttribBonus();
			else if (bonus < -ConfigServerGame::getMaxItemAttribBonus())
				bonus = -ConfigServerGame::getMaxItemAttribBonus();
			attribBonuses.push_back(std::make_pair(attribute, bonus));
		}
		else
		{
			WARNING(true, ("Object %s has invalid attribute bonus objvar name of %s",
				getNetworkId().getValueString().c_str(), i.getName().c_str()));
		}
	}
}	// ManufactureSchematicObject::getAttribBonuses

//----------------------------------------------------------------------

/**
 * Returns the attribute bonus of this object for a given attribute.
 *
 * @param attribute		the attribute
 *
 * @return the bonus for the attribute
 */
int ManufactureSchematicObject::getAttribBonus(int attribute) const
{
	if (attribute < 0 || attribute >= Attributes::NumberOfAttributes)
		return 0;

	DynamicVariableList::NestedList mods(getObjVars(), OBJVAR_ATTRIBUTE_BONUS);
	char buffer[8];
	snprintf(buffer, sizeof(buffer), "%d", attribute);
	int bonus = 0;
	IGNORE_RETURN(mods.getItem(buffer, bonus));

	if (bonus > ConfigServerGame::getMaxItemAttribBonus())
		bonus = ConfigServerGame::getMaxItemAttribBonus();
	else if (bonus < -ConfigServerGame::getMaxItemAttribBonus())
		bonus = -ConfigServerGame::getMaxItemAttribBonus();
	return bonus;
}	// ManufactureSchematicObject::getAttribBonus

//----------------------------------------------------------------------

/**
 * Sets the attribute bonus this object applies when equipped.
 *
 * @param attribute		the attribute
 * @param bonus			the bonus
 */
void ManufactureSchematicObject::setAttribBonus(int attribute, int bonus)
{
	if (attribute < 0 || attribute >= Attributes::NumberOfAttributes)
		return;

	char buffer[8];
	snprintf(buffer, sizeof(buffer), "%d", attribute);
	if (bonus != 0)
		setObjVarItem(OBJVAR_ATTRIBUTE_BONUS + '.' + buffer, bonus);
	else
		removeObjVarItem(OBJVAR_ATTRIBUTE_BONUS + '.' + buffer);
}	// ManufactureSchematicObject::setAttribBonus

// ----------------------------------------------------------------------

const std::map<StringId, float> & ManufactureSchematicObject::getAttributes() const
{
	return m_attributes.getMap();
}

// ----------------------------------------------------------------------

/**
 * Returns the volume the schematic takes up in a player's datapad. The volume
 * is defined by the draft schematic template.
 */
int ManufactureSchematicObject::getVolume() const
{
	const DraftSchematicObject * const schematic = DraftSchematicObject::getSchematic(getDraftSchematic());
	if (schematic != nullptr)
		return schematic->getObjectTemplate()->asServerObjectTemplate()->getVolume();
	return IntangibleObject::getVolume();
}	// ManufactureSchematicObject::getVolume

// ----------------------------------------------------------------------

void ManufactureSchematicObject::getAttributes (AttributeVector &data) const
{
	char valueBuffer[32];
	const size_t valueBuffer_size = sizeof (valueBuffer);

	IntangibleObject::getAttributes(data);

	static IngredientInfoVector iiv;
	iiv.clear ();

	getIngredientInfo (iiv);

	data.reserve (data.size () + iiv.size () + 3);

	snprintf (valueBuffer, valueBuffer_size, "%d", getVolume ());
	data.push_back (std::make_pair (SharedObjectAttributes::data_volume, Unicode::narrowToWide (valueBuffer)));

	for (IngredientInfoVector::const_iterator it = iiv.begin (); it != iiv.end (); ++it)
	{
		const Unicode::String & name = (*it).first;
		const int amount            = (*it).second.second;

		if (name.empty ())
			continue;

		snprintf(valueBuffer, valueBuffer_size, "%i", amount);

		if (name [0] == '@')
			data.push_back (std::make_pair (SharedObjectAttributes::cat_manf_schem_ing_resource + "." + Unicode::wideToNarrow (name), Unicode::narrowToWide (valueBuffer)));
		else
			data.push_back (std::make_pair (SharedObjectAttributes::cat_manf_schem_ing_resource + ".\"" + Unicode::wideToNarrow (name), Unicode::narrowToWide (valueBuffer)));
	}

	snprintf(valueBuffer, valueBuffer_size, "%i", getCount());
	data.push_back(std::make_pair(SharedObjectAttributes::manf_limit, Unicode::narrowToWide(valueBuffer)));

	//-- show manufactured object attribs
	data.push_back(std::make_pair(SharedObjectAttributes::manf_attribs, Unicode::narrowToWide("\\#pcontrast2 ----------\\#.")));

	retrieveStoredObjectAttributes (data);
}

// ----------------------------------------------------------------------

void ManufactureSchematicObject::recalculateData()
{
	if (isAuthoritative())
	{
		// copy the OBJVAR_ATTRIBUTES objvars to the m_attributes member
		if (getObjVars().hasItem(OBJVAR_ATTRIBUTES))
		{
			const DynamicVariableList::NestedList attributes(getObjVars(), OBJVAR_ATTRIBUTES);
			for (DynamicVariableList::NestedList::const_iterator i = attributes.begin();
				i != attributes.end(); ++i)
			{
				float value;
				i.getValue(value);
				m_attributes.set(StringId(i.getName()), value);
			}
			removeObjVarItem(OBJVAR_ATTRIBUTES);
		}

		// bug fix - set the schematic's complexity from the "crafting:complexity" attrib
		Archive::AutoDeltaMap<StringId, float>::const_iterator found = m_attributes.find(ATTRIBUTES_COMPLEXITY);
		if (found != m_attributes.end())
		{
			float complexity = (*found).second;
			if (complexity != getComplexity())
				setComplexity(complexity);
		}

		const DraftSchematicObject * const schematic = DraftSchematicObject::getSchematic(m_draftSchematic.get());

		if (schematic)
		{
			m_draftSchematicSharedTemplate = schematic->getSharedTemplate()->getCrcName().getCrc();
		}
		else
		{
			const DraftSchematicObject * const missingSchematic = DraftSchematicObject::getSchematic(MISSING_SCHEMATIC_SUBSTITUTE);
			if (missingSchematic != nullptr)
				m_draftSchematicSharedTemplate = missingSchematic->getSharedTemplate()->getCrcName().getCrc();
			else
				WARNING_STRICT_FATAL(true, ("Cannot find draft schematic %s! This must always exist!", MISSING_SCHEMATIC_SUBSTITUTE.c_str()));
		}

		m_customAppearance = getCustomAppearance();

		std::string appearanceString;
		if (getObjVars().getItem("customization_data",appearanceString))
			m_appearanceData = appearanceString;
	}

	// resource types for imported resources used in the manufacturing schematic
	// needs to be registered on this game server; this must be done on each game
	// server as it receives the manufacturing schematic
	DynamicVariableList::NestedList const slots(getObjVars(),OBJVAR_SLOTS);
	for (DynamicVariableList::NestedList::const_iterator i = slots.begin(); i != slots.end(); ++i)
	{
		DynamicVariableList::NestedList const slot(i.getNestedList());
		if (slot.hasItem(OBJVAR_INGR_RESOURCE_IMPORTED_DATA) && slot.hasItem(OBJVAR_INGR_RESOURCE_ID))
		{
			NetworkId id;
			if (slot.getItem(OBJVAR_INGR_RESOURCE_ID, id) && !ServerUniverse::getInstance().getImportedResourceTypeById(id))
			{
				std::string resourceData;
				if (slot.getItem(OBJVAR_INGR_RESOURCE_IMPORTED_DATA, resourceData))
				{
					NetworkId const resourceTypeId = ResourceTypeObject::addImportedResourceType(resourceData);
					if (isAuthoritative() && resourceTypeId.isValid())
					{
						if (resourceTypeId.getValue() <= NetworkId::cms_maxNetworkIdWithoutClusterId)
							removeObjVarItem(slot.getNameWithPath() + '.' + OBJVAR_INGR_RESOURCE_IMPORTED_DATA);

						if (resourceTypeId != id)
							IGNORE_RETURN(setObjVarItem(slot.getNameWithPath() + '.' + OBJVAR_INGR_RESOURCE_ID, resourceTypeId));
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void ManufactureSchematicObject::getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const
{
	if (name == "msoCtsPackUnpack")
	{
		// creator Id/Name
		CreatureObject const * containingPlayer = nullptr;

		ServerObject const * parent = safe_cast<ServerObject const*>(ContainerInterface::getContainedByObject(*this));
		while (parent)
		{
			containingPlayer = parent->asCreatureObject();
			if (containingPlayer && PlayerCreatureController::getPlayerObject(containingPlayer))
				break;

			containingPlayer = nullptr;
			parent = safe_cast<ServerObject const*>(ContainerInterface::getContainedByObject(*parent));
		}

		bool const creatorIsOwner = (containingPlayer && (containingPlayer->getNetworkId() == m_creatorId.get()));
		Archive::AutoDeltaVariable<bool>(creatorIsOwner).pack(target);

		// items per container
		m_itemsPerContainer.pack(target);

		// manufacture time
		m_manufactureTime.pack(target);

		// attributes
		m_attributes.pack(target);

		// resource ingredients
		Archive::AutoDeltaMap<std::string, std::pair<NetworkId, std::string> > resourceIngredients;

		DynamicVariableList::NestedList const slots(getObjVars(),OBJVAR_SLOTS);
		for (DynamicVariableList::NestedList::const_iterator i = slots.begin(); i != slots.end(); ++i)
		{
			DynamicVariableList::NestedList const slot(i.getNestedList());
			if (slot.hasItem(OBJVAR_INGR_RESOURCE_ID))
			{
				NetworkId id;
				if (slot.getItem(OBJVAR_INGR_RESOURCE_ID, id))
				{
					ResourceTypeObject const * const rto = ServerUniverse::getInstance().getResourceTypeById(id);
					if (rto)
						resourceIngredients.set(slot.getNameWithPath(), std::make_pair(((id.getValue() <= NetworkId::cms_maxNetworkIdWithoutClusterId) ? NetworkId(id.getValueWithClusterId(GameServer::getInstance().getClusterId(), static_cast<NetworkId::NetworkIdType>(1), NetworkId::cms_maxNetworkIdWithoutClusterId)) : id), rto->getResourceTypeDataForExport()));
				}
			}
		}

		resourceIngredients.pack(target);

		// the manufacturing schematic "original" NetworkId; see the comment for getOriginalId() for more information
		NetworkId const originalId = getOriginalId();
		Archive::AutoDeltaVariable<NetworkId>(originalId).pack(target);
	}
	else
	{
		IntangibleObject::getByteStreamFromAutoVariable(name, target);
	}
}

// ----------------------------------------------------------------------

void ManufactureSchematicObject::setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source)
{
	Archive::ReadIterator ri(source);
	if (name == "msoCtsPackUnpack")
	{
		// creator Id/Name
		Archive::AutoDeltaVariable<bool> creatorIsOwner;
		creatorIsOwner.unpackDelta(ri);

		CreatureObject const * containingPlayer = nullptr;

		if (creatorIsOwner.get())
		{
			ServerObject const * parent = safe_cast<ServerObject const*>(ContainerInterface::getContainedByObject(*this));
			while (parent)
			{
				containingPlayer = parent->asCreatureObject();
				if (containingPlayer && PlayerCreatureController::getPlayerObject(containingPlayer))
					break;

				containingPlayer = nullptr;
				parent = safe_cast<ServerObject const*>(ContainerInterface::getContainedByObject(*parent));
			}
		}

		if (creatorIsOwner.get() && containingPlayer)
		{
			m_creatorId = containingPlayer->getNetworkId();
			m_creatorName = containingPlayer->getObjectName();
			replaceStoredObjectAttribute(Unicode::narrowToWide(SharedObjectAttributes::crafter), Unicode::toLower(containingPlayer->getAssignedObjectFirstName()));
		}
		else
		{
			// if the manufacturing schematic's creator is transferring the manufacturing schematic,
			// then on the destination galaxy, preserve the manufacturing schematic's creator
			// information by setting the destination manufacturing schematic's creator to the
			// destination character; otherwise, set the destination manufacturing schematic's creator
			// to some non-zero oid that cannot possibly be a player on the target galaxy (like 1), to
			// make it look like the manufacturing schematic's creator is no longer on the galaxy,
			// like the manufacturing schematic's creator has been deleted and removed from the DB
			m_creatorId = NetworkId(static_cast<NetworkId::NetworkIdType>(10000000)); // 10,000,000 is max gold oid
			m_creatorName = Unicode::String();
			replaceStoredObjectAttribute(Unicode::narrowToWide(SharedObjectAttributes::crafter), Unicode::String());
		}

		// items per container
		m_itemsPerContainer.unpackDelta(ri);

		// manufacture time
		m_manufactureTime.unpackDelta(ri);

		// attributes
		typedef Archive::AutoDeltaMap<StringId, float>::Command AttributeCommands;
		std::vector<AttributeCommands> attributes;
		Archive::AutoDeltaMap<StringId, float>::unpack(ri, attributes);

		m_attributes.clear();
		for (std::vector<AttributeCommands>::const_iterator attributeIter = attributes.begin(); attributeIter != attributes.end(); ++attributeIter)
			m_attributes.set(attributeIter->key, attributeIter->value);

		// resource ingredients
		typedef Archive::AutoDeltaMap<std::string, std::pair<NetworkId, std::string> >::Command ResourceIngredientCommands;
		std::vector<ResourceIngredientCommands> resourceIngredients;
		Archive::AutoDeltaMap<std::string, std::pair<NetworkId, std::string> >::unpack(ri, resourceIngredients);

		for (std::vector<ResourceIngredientCommands>::const_iterator resourceIngredientIter = resourceIngredients.begin(); resourceIngredientIter != resourceIngredients.end(); ++resourceIngredientIter)
		{
			IGNORE_RETURN(setObjVarItem(resourceIngredientIter->key + '.' + OBJVAR_INGR_RESOURCE_ID, resourceIngredientIter->value.first));
			IGNORE_RETURN(setObjVarItem(resourceIngredientIter->key + '.' + OBJVAR_INGR_RESOURCE_IMPORTED_DATA, resourceIngredientIter->value.second));
		}

		// the manufacturing schematic "original" NetworkId; see the comment for getOriginalId() for more information
		Archive::AutoDeltaVariable<NetworkId> originalId;
		originalId.unpackDelta(ri);

		removeObjVarItem(OBJVAR_ORIGINAL_OBJECT_ID);
		IGNORE_RETURN(setObjVarItem(OBJVAR_ORIGINAL_OBJECT_ID, originalId.get()));

		recalculateData();

		// need to tell all proxy game servers, if any, to create the imported resource
		// types for any ingredient, if it hasn't already been created on that game server
		ProxyList const &proxyList = getExposedProxyList();
		if (!proxyList.empty())
		{
			bool calledServerMessageForwardingBegin = false;
			DynamicVariableList::NestedList const slots(getObjVars(),OBJVAR_SLOTS);
			for (DynamicVariableList::NestedList::const_iterator i = slots.begin(); i != slots.end(); ++i)
			{
				DynamicVariableList::NestedList const slot(i.getNestedList());
				if (slot.hasItem(OBJVAR_INGR_RESOURCE_IMPORTED_DATA) && slot.hasItem(OBJVAR_INGR_RESOURCE_ID))
				{
					NetworkId id;
					std::string resourceData;
					if (slot.getItem(OBJVAR_INGR_RESOURCE_ID, id) && (id.getValue() > NetworkId::cms_maxNetworkIdWithoutClusterId) && slot.getItem(OBJVAR_INGR_RESOURCE_IMPORTED_DATA, resourceData))
					{
						if (!calledServerMessageForwardingBegin)
						{
							ServerMessageForwarding::begin(std::vector<uint32>(proxyList.begin(), proxyList.end()));
							calledServerMessageForwardingBegin = true;
						}

						GenericValueTypeMessage<std::pair<NetworkId, std::string> > const addImportedResourceTypeMsg("AddImportedResourceType", std::make_pair(id, resourceData));
						ServerMessageForwarding::send(addImportedResourceTypeMsg);
					}
				}
			}

			if (calledServerMessageForwardingBegin)
				ServerMessageForwarding::end();
		}
	}
	else
	{
		IntangibleObject::setAutoVariableFromByteStream(name, source);
	}

	addObjectToConcludeList();
}

// ----------------------------------------------------------------------
// crates that get generated from a manufacturing schematic use the
// NetworkId of the manufacturing schematic to generate the crate's
// (and the items pulled from the crate) serial number; however, when
// a manufacturing schematic is CTS(ed), it gets a different NetworkId
// on the destination cluster, thus causing it to create crate with a
// different serial number; in order to preserve the serial number
// of crates generated from a manufacturing schematic regardless of
// how many times the manufacturing schematic gets CTS(ed), for
// manufacturing schematic that is created because of CTS, we need
// to preserve the NetworkId of the original schematic and use that
// NetworkId when generating the crate from the schematic
NetworkId ManufactureSchematicObject::getOriginalId() const
{
	DynamicVariableList const & objvar = getObjVars();

	if (objvar.hasItem(OBJVAR_ORIGINAL_OBJECT_ID) && (objvar.getType(OBJVAR_ORIGINAL_OBJECT_ID) == DynamicVariable::NETWORK_ID))
	{
		NetworkId originalId;
		IGNORE_RETURN(objvar.getItem(OBJVAR_ORIGINAL_OBJECT_ID, originalId));
		return originalId;
	}

	return getNetworkId();
}

// ======================================================================
