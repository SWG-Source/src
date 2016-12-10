//========================================================================
//
// DraftSchematicObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/DraftSchematicObject.h"

#include "UnicodeUtils.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/ServerController.h"
#include "serverGame/ServerManufactureSchematicObjectTemplate.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedNetworkMessages/MessageQueueResourceWeights.h"
#include "sharedObject/NetworkController.h"
#include "sharedObject/ObjectTemplateList.h"

// ======================================================================
// local constants


static const int MAX_ATTRIBUTES = 32;
static const std::string REQUEST_RESOURCE_WEIGHTS_SCRIPT_METHOD("OnRequestResourceWeights");


// ======================================================================
// static members

DraftSchematicObject::SchematicMap * DraftSchematicObject::m_schematics = nullptr;
const SharedObjectTemplate * DraftSchematicObject::m_defaultSharedTemplate = nullptr;


// ======================================================================

/**
 * Class constructor.
 *
 * @param newTemplate		template to create the schematic from.
 */
DraftSchematicObject::DraftSchematicObject(
	const ServerDraftSchematicObjectTemplate* newTemplate) :
	IntangibleObject(newTemplate),
	m_slots(),
	m_attributes(),
	m_experimentalAttributes(),
	m_combinedCrc(newTemplate->getCrcName().getCrc(), getSharedTemplate()->getCrcName().getCrc())
{
int i;
ServerDraftSchematicObjectTemplate::IngredientSlot slot;

	// put the slots in the name lookup map
	int count = newTemplate->getSlotsCount();
	for (i = 0; i < count; ++i)
	{
		newTemplate->getSlots(slot, i);
		m_slots.insert(std::make_pair(slot.name, i));
	}

	// get the experimental attributes from the attributes, and create a map 
	// between them
	int minValue = 0, maxValue = 0;
	count = getAttribsCount();
	for (i = 0; i < count; ++i)
	{
		const SharedDraftSchematicObjectTemplate::SchematicAttribute & attrib = 
			getAttrib(i);

		m_attributes[attrib.name] = i;
		if (!attrib.experiment.getTable().empty() && !attrib.experiment.getText().empty())
		{
			getAttribMinMax(i, minValue, maxValue);
			if (minValue != maxValue)
				m_experimentalAttributes[attrib.experiment].push_back(attrib.name);
		}
	}
}	// DraftSchematicObject::DraftSchematicObject	

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
DraftSchematicObject::~DraftSchematicObject()
{
}	// DraftSchematicObject::~DraftSchematicObject

//-----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * DraftSchematicObject::getDefaultSharedTemplate(void) const
{
static const ConstCharCrcLowerString templateName("object/draft_schematic/base/shared_draft_schematic_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(
			ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create "
			"default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "DraftSchematicObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// DraftSchematicObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void DraftSchematicObject::removeDefaultTemplate(void)
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// DraftSchematicObject::removeDefaultTemplate

//-----------------------------------------------------------------------

/**
 * Requests info about resource weights for a given draft schematic.
 *
 * @param requester				the player making the request
 * @param draftSchematicName	the schematic the player wants to know about
 */
void DraftSchematicObject::requestResourceWeights(ServerObject & requester, uint32 draftSchematicCrc)
{
	if (requester.getClient() == nullptr)
	{
		WARNING (true, ("DraftSchematicObject::requestResourceWeights invalid client for [%s]", requester.getNetworkId ().getValueString ().c_str ()));
		return;
	}

	const DraftSchematicObject * const schematic = getSchematic(draftSchematicCrc);
	if (schematic == nullptr)
	{
		WARNING (true, ("DraftSchematicObject::requestResourceWeights invalid schematic [%lu] for [%s]", draftSchematicCrc, requester.getNetworkId ().getValueString ().c_str ()));
		return;
	}

	const ServerDraftSchematicObjectTemplate * const schematicTemplate = safe_cast<const ServerDraftSchematicObjectTemplate *>(schematic->getObjectTemplate());
	if (schematicTemplate == nullptr)
	{
		WARNING (true, ("DraftSchematicObject::requestResourceWeights invalid schematic template [%lu] for [%s]", draftSchematicCrc, requester.getNetworkId ().getValueString ().c_str ()));
		return;
	}

	std::vector<const char *> desiredAttribs;
	{
		for (NameToIndexMap::const_iterator i = schematic->m_attributes.begin(); i != schematic->m_attributes.end(); ++i)
			desiredAttribs.push_back((*i).first.getText().c_str());
	}

	std::vector<const char *> attributes(MAX_ATTRIBUTES * 2, static_cast<const char *>(nullptr));
	std::vector<int> slots(MAX_ATTRIBUTES * 2, 0);
	std::vector<int> counts(MAX_ATTRIBUTES * 2, 0);
	std::vector<int> weights(MAX_ATTRIBUTES * 2 * Crafting::RA_numResourceAttributes * 2, 0);
	ScriptParams params;
	params.addParam(requester.getNetworkId());
	params.addParam(desiredAttribs);
	params.addParam(attributes);
	params.addParam(slots);
	params.addParam(counts);
	params.addParam(weights);

	// call script function
	GameScriptObject::runOneScript(schematicTemplate->getManufactureScripts(0), REQUEST_RESOURCE_WEIGHTS_SCRIPT_METHOD, "Os[s*[i*[i*[i*[", params);

	const std::vector<const char *> & newAttributes = params.getStringArrayParam(2);
	const std::vector<int> & newSlots = params.getIntArrayParam(3);
	const std::vector<int> & newCounts = params.getIntArrayParam(4);
	const std::vector<int> & newWeights = params.getIntArrayParam(5);
	std::vector<int>::const_iterator slotIter(newSlots.begin());
	std::vector<int>::const_iterator countIter(newCounts.begin());
	std::vector<int>::const_iterator weightIter(newWeights.begin());

	MessageQueueResourceWeights * const message = new MessageQueueResourceWeights(schematic->getCombinedCrc());

	// find out how many attributes were returned
	int attribCount = 0;
	{
		for (std::vector<const char *>::const_iterator i(newAttributes.begin());
			i != newAttributes.end(); ++i, ++attribCount)
		{
			if (*i == nullptr)
				break;
		}
	}
	attribCount /= 2;

	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < attribCount; ++j)
		{
			const StringId sid("crafting", newAttributes[j + i * attribCount]);;
			const int attrib = schematic->getAttribIndex(sid);
			if (attrib == -1)
			{
				// skip over the data for that attrib
				slotIter++;
				const int weightCount = *countIter++;
				weightIter += weightCount * 2;
				continue;
			}
			const int slot = *slotIter++;
			const int weightCount = *countIter++;
			for (int k = 0; k < weightCount; ++k)
			{
				const int resource = *weightIter++;
				const int weight = *weightIter++;
				if (i == 0)
				{
					message->addAssemblyWeight(attrib, slot, resource, weight);
				}
				else
				{
					message->addResourceMaxWeight(attrib, slot, resource, weight);
				}
			}
		}
	}

	(safe_cast<ServerController *>(requester.getController()))->appendMessage(
		static_cast<int>(CM_resourceWeights), 0.0f, message, 
		GameControllerMessageFlags::SEND | 
		GameControllerMessageFlags::RELIABLE | 
		GameControllerMessageFlags::DEST_AUTH_CLIENT);
}	// DraftSchematicObject::requestResourceWeights

//-----------------------------------------------------------------------

/**
 * Creates a manufacture schematic based on this schematic.
 *
 * @param creator		the player creating the schematic
 *
 * @return the manufacture schematic
 */
ManufactureSchematicObject * DraftSchematicObject::createManufactureSchematic(
	const CachedNetworkId & creator) const
{
	Object * object = ServerManufactureSchematicObjectTemplate::createObject(
		"object/manufacture_schematic/generic_schematic.iff", *this);
	if (object == nullptr)
		return nullptr;
	ManufactureSchematicObject * schematic = safe_cast<ManufactureSchematicObject *>(object);
	schematic->init(*this, creator);
	schematic->setNetworkId(ObjectIdManager::getInstance().getNewObjectId());

	return schematic;
}	// DraftSchematicObject::createManufactureSchematic

//-----------------------------------------------------------------------

/**
 * Gets a draft scrematic slot by name.
 *
 * @param slot		slot info to be filled
 * @param name		name of the slot to get
 *
 * @return true on success, false if the slot wasn't found
 */
bool DraftSchematicObject::getSlot(
	ServerDraftSchematicObjectTemplate::IngredientSlot & slot, int index) const
{
	if (index < 0 || index >= static_cast<int>((safe_cast<
		const ServerDraftSchematicObjectTemplate*>(
		getObjectTemplate()))->getSlotsCount()))
	{
		return false;
	}

	slot.options.clear();
	(safe_cast<const ServerDraftSchematicObjectTemplate*>(getObjectTemplate()))->getSlots(slot, index);
	return true;
}

//-----------------------------------------------------------------------

/**
 * Gets a draft scrematic slot by name.
 *
 * @param slot		slot info to be filled
 * @param name		name of the slot to get
 *
 * @return true on success, false if the slot wasn't found
 */
bool DraftSchematicObject::getSlot(
	ServerDraftSchematicObjectTemplate::IngredientSlot & slot, const StringId & name) const
{
	NameToIndexMap::const_iterator iter = m_slots.find(name);
	if (iter != m_slots.end())
		return getSlot(slot, (*iter).second);
	return false;
}	// DraftSchematicObject::getSlot(const StringId &)

//-----------------------------------------------------------------------

/**
 * Gets the min and max values for an attribute of the schematic.
 *
 * @param index			index of the object attribute
 * @param minValue		value that will be filled in with the min attribute value
 * @param maxValue		value that will be filled in with the max attribute value
 *
 * @return true on success, false if the name is not a valid attribute
 */
bool DraftSchematicObject::getAttribMinMax(int index, int & minValue, 
	int & maxValue) const
{
	const SharedDraftSchematicObjectTemplate::SchematicAttribute & minAttrib = 
		getAttribMin(index);
	const SharedDraftSchematicObjectTemplate::SchematicAttribute & maxAttrib = 
		getAttribMax(index);
		
	minValue = minAttrib.value;
	maxValue = maxAttrib.value;
	return true;
}	// DraftSchematicObject::getAttribMinMax(int)

//-----------------------------------------------------------------------

/**
 * Gets the min and max values for an attribute of the schematic.
 *
 * @param name			the attribute name
 * @param minValue		value that will be filled in with the min attribute value
 * @param maxValue		value that will be filled in with the max attribute value
 *
 * @return true on success, false if the name is not a valid attribute
 */
bool DraftSchematicObject::getAttribMinMax(const StringId & name, int & minValue, 
	int & maxValue) const
{
	NameToIndexMap::const_iterator iter = m_attributes.find(name);
	if (iter == m_slots.end())
		return false;

	int index = (*iter).second;
	return getAttribMinMax(index, minValue, maxValue);
}	// DraftSchematicObject::getAttribMinMax(const StringId &)

//-----------------------------------------------------------------------

/**
 * Returns the name of a experimental attribute associated with an object attribute.
 * 
 * @param index		index of the object attribute
 *
 * @return the experimental attribute name
 */
const StringId DraftSchematicObject::getExperimentalAttributeFromAttribute(
	int index) const
{
static const StringId NO_EXPERIMENTATION;

	const SharedDraftSchematicObjectTemplate::SchematicAttribute & attrib = 
		getAttrib(index);
	if (attrib.experiment == NO_EXPERIMENTATION)
		return NO_EXPERIMENTATION;

	// make sure we can really experiment with the attribute
	int minValue, maxValue;
	if (!getAttribMinMax(index, minValue, maxValue))
		return NO_EXPERIMENTATION;

	if (minValue == maxValue)
	{
		WARNING(true, ("Draft schematic %s has attribute %s with experimental "
			"attribute %s even though the attribute min/max values are the "
			"same! Not allowing experimentation.", getObjectTemplateName(),
			attrib.name.getCanonicalRepresentation().c_str(),
			attrib.experiment.getCanonicalRepresentation().c_str()));
		return NO_EXPERIMENTATION;
	}

	return attrib.experiment;
}	// DraftSchematicObject::getExperimentalAttributeFromAttribute

//-----------------------------------------------------------------------

/**
 * Returns the name of a experimental attribute associated with an object attribute.
 * 
 * @param attribName	the object attribute name
 *
 * @return the experimental attribute name
 */
const StringId DraftSchematicObject::getExperimentalAttributeFromAttribute(
	const StringId & attribName) const
{
static const StringId NO_EXPERIMENTATION;

	const SharedDraftSchematicObjectTemplate::SchematicAttribute & attrib = 
		getAttrib(attribName);

	if (attrib.experiment == NO_EXPERIMENTATION)
		return NO_EXPERIMENTATION;

	// make sure we can really experiment with the attribute
	int minValue, maxValue;
	if (!getAttribMinMax(attribName, minValue, maxValue))
		return NO_EXPERIMENTATION;

	if (minValue == maxValue)
	{
		WARNING(true, ("Draft schematic %s has attribute %s with experimental "
			"attribute %s even though the attribute min/max values are the "
			"same! Not allowing experimentation.", getObjectTemplateName(),
			attrib.name.getCanonicalRepresentation().c_str(),
			attrib.experiment.getCanonicalRepresentation().c_str()));
		return NO_EXPERIMENTATION;
	}

	return attrib.experiment;
}	// DraftSchematicObject::getExperimentalAttributeFromAttribute

//-----------------------------------------------------------------------

/**
 * Returns the names of object attributes associated with an experimental attribute.
 * 
 * @param attribName	the experimental attribute name
 *
 * @return the object attribute name list
 */
const std::vector<StringId> & DraftSchematicObject::getFromAttributeFromExperimentalAttribute(
	const StringId & experimentAttribName) const
{
	return (*m_experimentalAttributes.find(experimentAttribName)).second;
}	// DraftSchematicObject::getFromAttributeFromExperimentalAttribute


// ======================================================================
// factory methods

/**
 * Returns a draft schematic with a given name. If the schematic isn't loaded, 
 * loads it.
 *
 * @param name		name of the schematic to load
 *
 * @return the draft schematic
 */
const DraftSchematicObject * DraftSchematicObject::getSchematic(
	const std::string & name)
{
	NOT_NULL(m_schematics);
	return getSchematic(Crc::calculate(name.c_str()));
}	// DraftSchematicObject::getSchematic(const std::string &)

//-----------------------------------------------------------------------

/**
 * Returns a draft schematic with a given name. If the schematic isn't loaded, 
 * loads it.
 *
 * @param crc		crc of name of the schematic to load
 *
 * @return the draft schematic
 */
const DraftSchematicObject * DraftSchematicObject::getSchematic(uint32 crc)
{
	NOT_NULL(m_schematics);
	if (m_schematics == nullptr || crc == 0)
		return nullptr;

	// see if the schematic is already loaded
	SchematicMap::iterator result = m_schematics->find(crc);
	if (result != m_schematics->end())
		return (*result).second;

	// get the template name from the crc
	const ConstCharCrcString & name = ObjectTemplateList::lookUp(crc);
	if (name.isEmpty())
	{
		WARNING(true, ("Unable to find template name for crc %u", crc));
		return nullptr;
	}

	// create a new schematic
	if (!TreeFile::exists(name.getString()))
	{
		WARNING(true, ("Draft schematic template %s file not found", name.getString()));
		return nullptr;
	}

	const ObjectTemplate * objTemplate = ObjectTemplateList::fetch(name);
	if (objTemplate == nullptr)
	{
		WARNING(true, ("Can't create object template %s", name.getString()));
		return nullptr;
	}

	const ServerDraftSchematicObjectTemplate * schematicTemplate = 
		dynamic_cast<const ServerDraftSchematicObjectTemplate *>(objTemplate);
	if (schematicTemplate == nullptr)
	{
		WARNING(true, ("Template %s is not a draft schematic", name.getString()));
		objTemplate->releaseReference();
		return nullptr;
	}

	const DraftSchematicObject * schematic = new DraftSchematicObject(schematicTemplate);
	objTemplate->releaseReference();

	m_schematics->insert(std::make_pair(crc, schematic));
	return schematic;
}	// DraftSchematicObject::getSchematic(uint32)

//-----------------------------------------------------------------------

/**
 * Sets up the draft schematic factory.
 */
void DraftSchematicObject::install()
{
	if (m_schematics == nullptr)
		m_schematics = new SchematicMap();
}	// DraftSchematicObject::install

//-----------------------------------------------------------------------

/**
 * Cleans up the draft schematic factory
 */
void DraftSchematicObject::remove()
{
	if (m_schematics != nullptr)
	{
		SchematicMap::iterator iter;
		for (iter = m_schematics->begin(); iter != m_schematics->end(); ++iter)
		{
			delete (*iter).second;
			(*iter).second = nullptr;
		}
		delete m_schematics;
		m_schematics = nullptr;
	}
}	// DraftSchematicObject::remove

// ----------------------------------------------------------------------

void DraftSchematicObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	IntangibleObject::getAttributes(data);
}

// ======================================================================
