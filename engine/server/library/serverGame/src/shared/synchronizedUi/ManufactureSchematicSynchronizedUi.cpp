// ======================================================================
//
// ManufactureSchematicSynchronizedUi.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ManufactureSchematicSynchronizedUi.h"

#include "serverGame/ManufactureSchematicObject.h"
#include "sharedGame/CraftingData.h"
//#include "serverGame/ServerWorld.h"


//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param owner		manf schematic creating this class
 */
ManufactureSchematicSynchronizedUi::ManufactureSchematicSynchronizedUi(ManufactureSchematicObject& owner) :
	ServerSynchronizedUi(owner),
	m_slotName(),
	m_slotType(),
	m_slotIngredient(),
	m_slotIngredientCount(),
	m_slotComplexity(),
	m_slotDraftOption(),
	m_slotDraftIndex(),
	m_slotIngredientsChanged(0),
	m_attributeName(),
	m_attributeValue(),
	m_minAttribute(),
	m_maxAttribute(),
	m_resourceMaxAttribute(),
	m_customName(),
	m_customIndex(),
	m_customMinIndex(),
	m_customMaxIndex(),
	m_customChanged(0),
	m_experimentMod(0),
	m_appearance(),
	m_ready(false)
{
	addToUiPackage(m_slotName);
	addToUiPackage(m_slotType);
	addToUiPackage(m_slotIngredient);
	addToUiPackage(m_slotIngredientCount);
	addToUiPackage(m_slotComplexity);
	addToUiPackage(m_slotDraftOption);
	addToUiPackage(m_slotDraftIndex);
	addToUiPackage(m_slotIngredientsChanged);
	addToUiPackage(m_attributeName);
	addToUiPackage(m_attributeValue);
	addToUiPackage(m_minAttribute);
	addToUiPackage(m_maxAttribute);
	addToUiPackage(m_resourceMaxAttribute);
	addToUiPackage(m_customName);
	addToUiPackage(m_customIndex);
	addToUiPackage(m_customMinIndex);
	addToUiPackage(m_customMaxIndex);
	addToUiPackage(m_customChanged);
	addToUiPackage(m_experimentMod);
	addToUiPackage(m_appearance);
	addToUiPackage(m_ready);
}	// ManufactureSchematicSynchronizedUi::ManufactureSchematicSynchronizedUi

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
ManufactureSchematicSynchronizedUi::~ManufactureSchematicSynchronizedUi()
{
}	// ManufactureSchematicSynchronizedUi::~ManufactureSchematicSynchronizedUi

//-----------------------------------------------------------------------

/**
 * Creates an empty slot with a given name. We assume there isn't already a slot 
 * with the name.
 *
 * @param name			the slot name
 * @param complexity	the slot's initial complexity
 */
void ManufactureSchematicSynchronizedUi::createNewSlot(const StringId & name, 
	float complexity)
{
	m_slotName.push_back(name);
	m_slotType.push_back(Crafting::IT_none);
	m_slotIngredient.push_back(NetworkIdList());
	m_slotIngredientCount.push_back(IntList());
	m_slotComplexity.push_back(complexity);
	m_slotDraftOption.push_back(-1);
	m_slotDraftIndex.push_back(-1);
	signalIngredientsChanged ();
}	// ManufactureSchematicSynchronizedUi::createNewSlot(const StringId & name)

//-----------------------------------------------------------------------

/**
 * Sets the slot type of a manf slot.
 *
 * @param name		the name of the slot
 * @param type		the slot type
 */
void ManufactureSchematicSynchronizedUi::setSlotType(const StringId & name, int type)
{
	int iter = m_slotName.find(name);
	if (iter >= 0)
	{
		m_slotType.set(iter, type);
		signalIngredientsChanged ();
	}

}	// ManufactureSchematicSynchronizedUi::setSlotType

//-----------------------------------------------------------------------

/**
 * Sets the slot option index of a manf slot.
 *
 * @param name		the name of the slot
 * @param option	the option index
 */
void ManufactureSchematicSynchronizedUi::setSlotOption(const StringId & name, int option)
{
	int iter = m_slotName.find(name);
	if (iter >= 0)
	{
		m_slotDraftOption.set(iter, option);
		signalIngredientsChanged ();
	}

}	// ManufactureSchematicSynchronizedUi::setSlotOption

//-----------------------------------------------------------------------

/**
 * Sets the draft schematic slot index of a manf slot.
 *
 * @param name		the name of the slot
 * @param index		the draft schematic index
 */
void ManufactureSchematicSynchronizedUi::setSlotIndex(const StringId & name, int index)
{
	int iter = m_slotName.find(name);
	if (iter >= 0)
	{
		m_slotDraftIndex.set(iter, index);
		signalIngredientsChanged ();
	}

}	// ManufactureSchematicSynchronizedUi::setSlotIndex

//-----------------------------------------------------------------------

/**
 * Modifies a slot's complexity by a value.
 *
 * @param name			the name of the slot
 * @param complexity	the slot complexity
 */ 
void ManufactureSchematicSynchronizedUi::setSlotComplexity(const StringId & name, 
	float complexity)
{
	int iter = m_slotName.find(name);
	if (iter >= 0)
	{
		m_slotComplexity.set(iter, complexity);
		signalIngredientsChanged ();
	}

}	// ManufactureSchematicSynchronizedUi::setSlotComplexity

//-----------------------------------------------------------------------

/**
 * Adds an ingredient to a schematic slot.
 *
 * @param name			the name of the slot
 * @param ingredientId	the ingredient to add
 * @param amount		amount of ingredient to add
 */
void ManufactureSchematicSynchronizedUi::addSlotIngredient(const StringId & name, 
	const NetworkId & ingredientId, int amount)
{
	int iter = m_slotName.find(name);
	if (iter >= 0)
	{
		// auto delta vector doesn't support sub-list updates, so we have to 
		// re-write the entire list of ingredients
		NetworkIdList ingredients(m_slotIngredient.get(iter));
		IntList counts(m_slotIngredientCount.get(iter));

		ingredients.push_back(ingredientId);
		counts.push_back(amount);

		m_slotIngredient.set(iter, ingredients);
		m_slotIngredientCount.set(iter, counts);

		signalIngredientsChanged ();
	}
}	// ManufactureSchematicSynchronizedUi::addSlotIngredient

//-----------------------------------------------------------------------

/**
 * Sets the amount of an ingredient needed for a schematic slot.
 *
 * @param name				the name of the slot
 * @param ingredientId		the ingredient to set the count of
 * @param amount			the amount of the ingredient needed
 */
void ManufactureSchematicSynchronizedUi::setSlotIngredientCount(const StringId & name, 
	const NetworkId & ingredientId, int amount)
{
	int iter = m_slotName.find(name);
	if (iter >= 0)
	{
		// find the index of the matching ingredient
		const NetworkIdList & ingredients = m_slotIngredient.get(iter);
		NetworkIdList::const_iterator iter2 = std::find(ingredients.begin(), 
			ingredients.end(), ingredientId);
		if (iter2 != ingredients.end())
		{
			// auto delta vector doesn't support sub-list updates, so we have to 
			// re-write the entire list of ingredients
			IntList counts(m_slotIngredientCount.get(iter));
			counts[iter2 - ingredients.begin()] = amount;
			m_slotIngredientCount.set(iter, counts);

			signalIngredientsChanged ();
		}
	}
}	// ManufactureSchematicSynchronizedUi::setSlotIngredientCount

//-----------------------------------------------------------------------

/**
 * Clears the data for a given slot in the schematic.
 *
 * @param name			the name of the slot to clear
 * @param complexity	the slot's initial complexity
 */
void ManufactureSchematicSynchronizedUi::clearSlot(const StringId & name, 
	float complexity)
{
	int iter = m_slotName.find(name);
	if (iter >= 0)
	{
		m_slotType.set(iter, Crafting::IT_none);
		m_slotIngredient.set(iter, NetworkIdList());
		m_slotIngredientCount.set(iter, IntList());
		m_slotComplexity.set(iter, complexity);
		m_slotDraftOption.set(iter, -1);
		signalIngredientsChanged ();
	}
}	// ManufactureSchematicSynchronizedUi::clearSlot

//-----------------------------------------------------------------------

/**
 * Removes an optional slot from the schematic.
 *
 * @param name		the name of the slot to clear
 */
void ManufactureSchematicSynchronizedUi::removeSlot(const StringId & name)
{
	int iter = m_slotName.find(name);
	if (iter >= 0)
	{
		m_slotName.erase(iter);
		m_slotType.erase(iter);
		m_slotIngredient.erase(iter);
		m_slotIngredientCount.erase(iter);
		m_slotComplexity.erase(iter);
		m_slotDraftOption.erase(iter);
		m_slotDraftIndex.erase(iter);
		signalIngredientsChanged ();
	}
}	// ManufactureSchematicSynchronizedUi::removeSlot

//-----------------------------------------------------------------------

/**
 * Returns the value of an attribute.
 *
 * @param name		name of the attribute
 *
 * @return value of the attribute
 */
float ManufactureSchematicSynchronizedUi::getAttribute(const StringId & name) const
{
	float result = 0;
	int index = m_attributeName.find(name);
	if (index != -1)
		result = m_attributeValue[index];
	return result;
}	// ManufactureSchematicSynchronizedUi::getAttribute

//-----------------------------------------------------------------------

/**
 * Sets the value of an attribute.
 *
 * @param name		name of the attribute
 * @param value		value of the attribute
 */
void ManufactureSchematicSynchronizedUi::setAttribute(const StringId & name, 
	float value)
{
	int index = m_attributeName.find(name);
	if (index == -1)
	{
		addAttribute(name);
		index = m_attributeName.size() - 1;
	}
	m_attributeValue.set(index, value);
}	// ManufactureSchematicSynchronizedUi::setAttribute

//-----------------------------------------------------------------------

/**
 * Returns the min value of an attribute.
 *
 * @param name		name of the attribute
 *
 * @return min value of the attribute
 */
float ManufactureSchematicSynchronizedUi::getMinAttribute(const StringId & name) const
{
	float result = 0;
	int index = m_attributeName.find(name);
	if (index != -1)
		result = m_minAttribute[index];
	return result;
}	// ManufactureSchematicSynchronizedUi::getMinAttribute

//-----------------------------------------------------------------------

/**
 * Sets the min value of an attribute.
 *
 * @param name		name of the attribute
 * @param value		min value of the attribute
 */
void ManufactureSchematicSynchronizedUi::setMinAttribute(const StringId & name, float value)
{
	int index = m_attributeName.find(name);
	if (index == -1)
	{
		addAttribute(name);
		index = m_attributeName.size() - 1;
	}
	m_minAttribute.set(index, value);
}	// ManufactureSchematicSynchronizedUi::setMinAttribute

//-----------------------------------------------------------------------

/**
 * Returns the max value of an attribute.
 *
 * @param name		name of the attribute
 *
 * @return max value of the attribute
 */
float ManufactureSchematicSynchronizedUi::getMaxAttribute(const StringId & name) const
{
	float result = 0;
	int index = m_attributeName.find(name);
	if (index != -1)
		result = m_maxAttribute[index];
	return result;
}	// ManufactureSchematicSynchronizedUi::getMaxAttribute

//-----------------------------------------------------------------------

/**
 * Sets the max value of an attribute.
 *
 * @param name		name of the attribute
 * @param value		value of the attribute
 */
void ManufactureSchematicSynchronizedUi::setMaxAttribute(const StringId & name, float value)
{
	int index = m_attributeName.find(name);
	if (index == -1)
	{
		addAttribute(name);
		index = m_attributeName.size() - 1;
	}
	m_maxAttribute.set(index, value);
}	// ManufactureSchematicSynchronizedUi::setMaxAttribute

//-----------------------------------------------------------------------

/**
 * Returns the resource max value of an attribute.
 *
 * @param name		name of the attribute
 *
 * @return resource max value of the attribute
 */
float ManufactureSchematicSynchronizedUi::getResourceMaxAttribute(const StringId & name) const
{
	float result = 0;
	int index = m_attributeName.find(name);
	if (index != -1)
		result = m_resourceMaxAttribute[index];
	return result;
}	// ManufactureSchematicSynchronizedUi::getResourceMaxAttribute

//-----------------------------------------------------------------------

/**
 * Sets the resource max value of an attribute.
 *
 * @param name		name of the attribute
 * @param value		resource max value of the attribute
 */
void ManufactureSchematicSynchronizedUi::setResourceMaxAttribute(const StringId & name, 
	float value)
{
	int index = m_attributeName.find(name);
	if (index == -1)
	{
		addAttribute(name);
		index = m_attributeName.size() - 1;
	}
	m_resourceMaxAttribute.set(index, value);
}	// ManufactureSchematicSynchronizedUi::setResourceMaxAttribute

//-----------------------------------------------------------------------

/**
 * Returns all the values associated with an attribute.
 *
 * @param name					name of the attribute
 * @param value                 filled in with the attribute value
 * @param minValue				filled in with the min attribute value
 * @param maxValue				filled in with the max attribute value
 * @param resourceMaxValue		filled in with the resource max attribute value
 */
void ManufactureSchematicSynchronizedUi::getAllAttributeValues(const StringId & name, 
	float & value, float & minValue, float & maxValue, float & resourceMaxValue) const
{
	int index = m_attributeName.find(name);
	if (index != -1)
	{
		value = m_attributeValue[index];
		minValue = m_minAttribute[index];
		maxValue = m_maxAttribute[index];
		resourceMaxValue = m_resourceMaxAttribute[index];
	}
	else
		value = minValue = maxValue = resourceMaxValue = 0;
}	// ManufactureSchematicSynchronizedUi::getAllAttributeValues

//-----------------------------------------------------------------------

/**
 * Sets the limits of an attribute.
 *
 * @param name					name of the attribute
 * @param minValue				min value of the attribute
 * @param maxValue				max value of the attribute
 * @param resourceMaxValue		resource max value of the attribute
 */
void ManufactureSchematicSynchronizedUi::setAttributeLimits(const StringId & name, 
	float minValue, float maxValue, float resourceMaxValue)
{
	int index = m_attributeName.find(name);
	if (index == -1)
	{
		addAttribute(name);
		index = m_attributeName.size() - 1;
	}
	m_minAttribute.set(index, minValue);
	m_maxAttribute.set(index, maxValue);
	m_resourceMaxAttribute.set(index, resourceMaxValue);
}	// ManufactureSchematicSynchronizedUi::setAttribute

//-----------------------------------------------------------------------

/**
 * Adds a new attribute to the attribute lists.
 *
 * @param name		name of the attribute
 */
void ManufactureSchematicSynchronizedUi::addAttribute(const StringId & name)
{
	m_attributeName.push_back(name);
	m_attributeValue.push_back(0);
	m_minAttribute.push_back(0);
	m_maxAttribute.push_back(0);
	m_resourceMaxAttribute.push_back(0);
}	// ManufactureSchematicSynchronizedUi::addAttribute

//-----------------------------------------------------------------------

/**
 * Sets or adds customization data to the schematic.
 *
 * @param name			the customization data name
 * @param value			value of the customization data
 * @param minValue		minimum possible value of the customization data
 * @param maxValue		maximum possible value of the customization data
 */
void ManufactureSchematicSynchronizedUi::setCustomization(const std::string & name, 
	int value, int minValue, int maxValue)
{
	int iter = m_customName.find(name);
	if (iter >= 0)
	{
		m_customIndex.set(iter, value);
		m_customMinIndex.set(iter, minValue);
		m_customMaxIndex.set(iter, maxValue);
		signalCustomizationChanged ();
	}
	else
	{
		m_customName.push_back(name);
		m_customIndex.push_back(value);
		m_customMinIndex.push_back(minValue);
		m_customMaxIndex.push_back(maxValue);
		signalCustomizationChanged ();
	}
}	// ManufactureSchematicSynchronizedUi::setCustomization(const std::string &, int, int, int)

//-----------------------------------------------------------------------

/**
 * Sets the value of customization data of the schematic.
 *
 * @param name			the customization data name
 * @param value			value of the customization data
 */
void ManufactureSchematicSynchronizedUi::setCustomization(const std::string & name, 
	int value)
{
	int iter = m_customName.find(name);
	if (iter >= 0)
	{
		m_customIndex.set(iter, value);
		signalCustomizationChanged ();
	}
}	// ManufactureSchematicSynchronizedUi::setCustomization(const std::string &, int)

//-----------------------------------------------------------------------

/**
 * Returns the name of a customization parameter.
 *
 * @param index		the index of the param
 *
 * @return the customization name
 */
const std::string & ManufactureSchematicSynchronizedUi::getCustomizationName(int index)
{
static const std::string emptyString;

	if (index < 0 || index >= static_cast<int>(m_customName.size()))
		return emptyString;
	return m_customName[index];
}	// ManufactureSchematicSynchronizedUi::getCustomizationName

//-----------------------------------------------------------------------

/**
 * Returns the min value for a customization parameter.
 *
 * @param name			the customization data name
 *
 * @return the min value
 */
int ManufactureSchematicSynchronizedUi::getMinCustomization(const std::string & name)
{
	int minValue = 0;

	int index = m_customName.find(name);
	if (index >= 0)
	{
		minValue = m_customMinIndex[index];
	}
	return minValue;
}	// ManufactureSchematicSynchronizedUi::getMinCustomization

//-----------------------------------------------------------------------

/**
 * Returns the max value for a customization parameter.
 *
 * @param name			the customization data name
 *
 * @return the max value
 */
int ManufactureSchematicSynchronizedUi::getMaxCustomization(const std::string & name)
{
	int maxValue = 0;

	int index = m_customName.find(name);
	if (index >= 0)
	{
		maxValue = m_customMaxIndex[index];
	}
	return maxValue;
}	// ManufactureSchematicSynchronizedUi::getMaxCustomization

//-----------------------------------------------------------------------

/**
 * Returns the current value for a customization parameter.
 *
 * @param name			the customization data name
 *
 * @return the current value
 */
int ManufactureSchematicSynchronizedUi::getCurrentCustomization(const std::string & name)
{
	int value = 0;

	int index = m_customName.find(name);
	if (index >= 0)
	{
		value = m_customIndex[index];
	}
	return value;
}	// ManufactureSchematicSynchronizedUi::getCurrentCustomization

//-----------------------------------------------------------------------

void ManufactureSchematicSynchronizedUi::signalIngredientsChanged ()
{
	m_slotIngredientsChanged.set (static_cast<uint8>(m_slotIngredientsChanged.get () + 1));
}

//----------------------------------------------------------------------

void ManufactureSchematicSynchronizedUi::signalCustomizationChanged ()
{
	m_customChanged.set (static_cast<uint8>(m_customChanged.get () + 1));
}

// ======================================================================
