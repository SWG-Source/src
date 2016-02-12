// ======================================================================
//
// SlottedContainer.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================


#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/SlottedContainer.h"

#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainmentProperty.h"
#include "sharedObject/Object.h"

#include <cstdio>
#include <map>

// ----------------------------------------------------------------------

namespace SlottedContainerNamespace
{
	std::vector<int>  s_tempIntContainer;
}

using namespace SlottedContainerNamespace;

// ------------------------------------------------------------------------

SlottedContainer::SlottedContainer(Object& owner, const std::vector<SlotId>& validSlots) :
Container(SlottedContainer::getClassPropertyId(), owner),
m_slotMap(new std::map<SlotId, int>)
{
	//Inialize the slot list with empty values.
	std::vector<SlotId>::const_iterator i = validSlots.begin();
	for (; i != validSlots.end(); ++i)
	{
		IGNORE_RETURN(m_slotMap->insert(SlotMap::value_type(*i ,-1)));
	}
}

// ------------------------------------------------------------------------

SlottedContainer::~SlottedContainer()
{
	if (m_slotMap)
	{
		delete m_slotMap;
		m_slotMap = nullptr;
	}
}

// ------------------------------------------------------------------------

bool SlottedContainer::isContentItemObservedWith(Object const &item) const
{
	//-- Rule 1: if base container claims that the item is visible with the container,
	//   we stick with that.  This prevents us from changing any existing behavior at
	//   the time this code is written.
	bool const observedWithBaseContainer = Container::isContentItemObservedWith(item);
	if (observedWithBaseContainer)
		return true;

	//-- Rule 2: if the item is in this container, check if any of the current arrangement's
	//   slots have the observeWithParent attribute set.  If so, return true, if not, return false.
	ContainedByProperty const *const containedByProperty = item.getContainedByProperty();
	if (!containedByProperty)
		return false;

	SlottedContainmentProperty const *const slottedContainmentProperty = safe_cast<SlottedContainmentProperty const*>(item.getProperty(SlottedContainmentProperty::getClassPropertyId()));
	if (!slottedContainmentProperty)
		return false;
	int const arrangementIndex = slottedContainmentProperty->getCurrentArrangement();

	// Note: when checking if item is in container, we must also check
	// that contained item's arrangement is set to a valid arrangement.
	// This function can be called during container transfers prior to
	// the arrangementIndex being set.  When this occurs, handle this
	// case as if the item is not in the container because it's not really
	// there in its entirety yet.
	
	Object const *const containedByObject = containedByProperty->getContainedBy();
	bool const isInThisContainer = (containedByObject == &getOwner()) && (arrangementIndex >= 0);
	
	if (isInThisContainer)
	{
		SlottedContainmentProperty::SlotArrangement const &slots = slottedContainmentProperty->getSlotArrangement(arrangementIndex);
		SlottedContainmentProperty::SlotArrangement::const_iterator const endIt = slots.end();
		for (SlottedContainmentProperty::SlotArrangement::const_iterator it = slots.begin(); it != endIt; ++it)
		{
			bool const observeWithParent = SlotIdManager::getSlotObserveWithParent(*it);
			if (observeWithParent)
				return true;
		}

		return false;
	}

	//-- Rule 3: if the item is not in this container, determine which arrangement it would
	//   use if it went in this slot.  If no arrangement is valid, return false.  If an arrangement
	//   is valid, check each slot in the arrangement.  If any slot has observeWithParent set true,
	//   return true; otherwise, return false.
	{
		ContainerErrorCode  errorCode;
		int arrangementIndex = -1;
		bool const result = getFirstUnoccupiedArrangement(item, arrangementIndex, errorCode);

		if (!result || (errorCode != CEC_Success))
			return false;

		SlottedContainmentProperty::SlotArrangement const &slots = slottedContainmentProperty->getSlotArrangement(arrangementIndex);
		SlottedContainmentProperty::SlotArrangement::const_iterator const endIt = slots.end();
		for (SlottedContainmentProperty::SlotArrangement::const_iterator it = slots.begin(); it != endIt; ++it)
		{
			bool const observeWithParent = SlotIdManager::getSlotObserveWithParent(*it);
			if (observeWithParent)
				return true;
		}

		return false;
	}
}

// ------------------------------------------------------------------------

bool SlottedContainer::isContentItemExposedWith(Object const &item) const
{
	//-- Rule 1: if base container claims that the item is visible with the container,
	//   we stick with that.  This prevents us from changing any existing behavior at
	//   the time this code is written.
	bool const exposedWithBaseContainer = Container::isContentItemExposedWith(item);
	if (exposedWithBaseContainer)
		return true;

	//-- Rule 2: if the item is in this container, check if any of the current arrangement's
	//   slots have the exposeWithParent attribute set.  If so, return true, if not, return false.
	ContainedByProperty const *const containedByProperty = item.getContainedByProperty();
	if (!containedByProperty)
		return false;

	SlottedContainmentProperty const *const slottedContainmentProperty = safe_cast<SlottedContainmentProperty const*>(item.getProperty(SlottedContainmentProperty::getClassPropertyId()));
	if (!slottedContainmentProperty)
		return false;
	int const arrangementIndex = slottedContainmentProperty->getCurrentArrangement();

	// Note: when checking if item is in container, we must also check
	// that contained item's arrangement is set to a valid arrangement.
	// This function can be called during container transfers prior to
	// the arrangementIndex being set.  When this occurs, handle this
	// case as if the item is not in the container because it's not really
	// there in its entirety yet.
	
	Object const *const containedByObject = containedByProperty->getContainedBy();
	bool const isInThisContainer = (containedByObject == &getOwner()) && (arrangementIndex >= 0);
	
	if (isInThisContainer)
	{
		SlottedContainmentProperty::SlotArrangement const &slots = slottedContainmentProperty->getSlotArrangement(arrangementIndex);
		SlottedContainmentProperty::SlotArrangement::const_iterator const endIt = slots.end();
		for (SlottedContainmentProperty::SlotArrangement::const_iterator it = slots.begin(); it != endIt; ++it)
		{
			bool const exposeWithParent = SlotIdManager::getSlotExposeWithParent(*it);
			if (exposeWithParent)
				return true;
		}

		return false;
	}

	//-- Rule 3: if the item is not in this container, determine which arrangement it would
	//   use if it went in this slot.  If no arrangement is valid, return false.  If an arrangement
	//   is valid, check each slot in the arrangement.  If any slot has exposeWithParent set true,
	//   return true; otherwise, return false.
	{
		ContainerErrorCode  errorCode;
		int arrangementIndex = -1;
		bool const result = getFirstUnoccupiedArrangement(item, arrangementIndex, errorCode);

		if (!result || (errorCode != CEC_Success))
			return false;

		SlottedContainmentProperty::SlotArrangement const &slots = slottedContainmentProperty->getSlotArrangement(arrangementIndex);
		SlottedContainmentProperty::SlotArrangement::const_iterator const endIt = slots.end();
		for (SlottedContainmentProperty::SlotArrangement::const_iterator it = slots.begin(); it != endIt; ++it)
		{
			bool const exposeWithParent = SlotIdManager::getSlotExposeWithParent(*it);
			if (exposeWithParent)
				return true;
		}

		return false;
	}
}

// ------------------------------------------------------------------------

bool SlottedContainer::canContentsBeObservedWith() const
{
	// Content items can be observed with this container if any of our slots are marked for observe with parent.
	for (SlotMap::const_iterator i = m_slotMap->begin(); i != m_slotMap->end(); ++i)
		if (SlotIdManager::getSlotObserveWithParent((*i).first))
			return true;
	return false;
}

// ------------------------------------------------------------------------

bool SlottedContainer::add (Object& item, int arrangementIndex, ContainerErrorCode & error)
{
	error = CEC_Success;
	if (arrangementIndex < 0)
	{
		error = CEC_InvalidArrangement;
		return false;
	}

	SlottedContainmentProperty * slottedProperty = safe_cast<SlottedContainmentProperty *>(item.getProperty(SlottedContainmentProperty::getClassPropertyId()));
	if (!slottedProperty)
	{
		WARNING_STRICT_FATAL(true, ("Tried to add an item %s to a slot container with no slotted property. Make sure its shared object template has a valid arrangement", item.getNetworkId().getValueString().c_str()));
		error = CEC_Unknown;
		return false;
	}

	if (!internalCheckSlottedAdd (item, arrangementIndex, error))
		return false;

	int position = addToContents(item, error);
	if (position < 0)
		return false;

	if (!internalDoSlottedAdd(item, position, arrangementIndex))
	{
		WARNING_STRICT_FATAL(true, ("internal Check add worked, but trying to do it failed."));
		error = CEC_Unknown;
		return false;
	}
	slottedProperty->setCurrentArrangement(arrangementIndex);
	return true;

}
// ------------------------------------------------------------------------

bool SlottedContainer::add (Object& item, const SlotId & slotId, ContainerErrorCode& error)
{
	error = CEC_Success;
	SlottedContainmentProperty * slottedProperty = safe_cast<SlottedContainmentProperty *>(item.getProperty(SlottedContainmentProperty::getClassPropertyId()));
	if (!slottedProperty)
	{
		WARNING_STRICT_FATAL(true, ("Tried to add an item %s to a slot container with no slotted property Make sure its shared object template has a valid arrangement ", item.getNetworkId().getValueString().c_str()));
		error = CEC_Unknown;
		return false;
	}

	return add(item, slottedProperty->getBestArrangementForSlot(slotId), error);
}

// ------------------------------------------------------------------------

bool SlottedContainer::internalCheckSlottedAdd( Object& item, int arrangementIndex, ContainerErrorCode & error ) const
{
	error = CEC_Success;
	if (arrangementIndex < 0)
	{
		error = CEC_InvalidArrangement;
		return false;
	}

	SlottedContainmentProperty * slottedProperty = safe_cast<SlottedContainmentProperty *>(item.getProperty(SlottedContainmentProperty::getClassPropertyId()));
	if (!slottedProperty)
	{
		WARNING_STRICT_FATAL(true, ("Tried to add an item %s to a slot container with no slotted property Make sure its shared object template has a valid arrangement", item.getNetworkId().getValueString().c_str()));
		error = CEC_Unknown;
		return false;
	}

	// realize that this is a 2*O(1) search, once for the mayAdd, the once for the fill.
	if (!mayAdd(item, arrangementIndex, error))
		return false;

	return true;
}

// ------------------------------------------------------------------------

bool SlottedContainer::internalDoSlottedAdd( Object& item, int position, int arrangementIndex )
{
	if (arrangementIndex < 0)
		return false;

	SlottedContainmentProperty * slottedProperty = safe_cast<SlottedContainmentProperty *>(item.getProperty(SlottedContainmentProperty::getClassPropertyId()));
	// get the arrangement from the item with arrangementIndex
	const int numSlots = slottedProperty->getNumberOfSlots(arrangementIndex);
	// fill each slot in the list with our own slot map
	for (int i = 0; i < numSlots; ++i)
	{
		SlotMap::iterator j = m_slotMap->find(slottedProperty->getSlotId(arrangementIndex, i));
		if (j != m_slotMap->end() && j->second == -1)
		{
			j->second = position;
		}
		else
			WARNING_STRICT_FATAL(true, ("We're trying to add an item to a slotted container that does not have a slot that this item %s with this arrangment %d can go into or has something in it already.", item.getNetworkId().getValueString().c_str(), arrangementIndex));
	}
	return true;
}

// ------------------------------------------------------------------------

int SlottedContainer::find(const SlotId &slot) const
{
	SlotMap::const_iterator i = m_slotMap->find(slot);
	if (i == m_slotMap->end())					//lint !e1702 STL operator== both a member and not
	{
		DEBUG_WARNING(true, ("Tried to find from an invalid slot on this container [%s].  Check the container's slot descriptor file to make sure it has slot %s.", getOwner().getNetworkId().getValueString().c_str(), SlotIdManager::getSlotName(slot).getString() ));
		return -1;
	}

    return i->second;						//lint !e1702 STL operator== both a member and not
}

// ------------------------------------------------------------------------

Container::ContainedItem SlottedContainer::getObjectInSlot (const SlotId &slot, ContainerErrorCode & error)
{
	error = CEC_Success;
	if (!hasSlot(slot))
	{
		error = CEC_NoSlot;
		return CachedNetworkId::cms_cachedInvalid;
	}

	int position = find(slot);
	if (position < 0)
	{
		error = CEC_NotFound;
		return CachedNetworkId::cms_cachedInvalid;
	}

	return getContents(position);
} //lint !e1762 // function could be made const // yes, but we don't want you to modify contents of a const container

// ------------------------------------------------------------------------

const Container::ContainedItem SlottedContainer::getObjectInSlot (const SlotId &slot, ContainerErrorCode & error) const
{
	error = CEC_Success;
	//@todo I must not be doing this right...I should be sharing code with the above routine.
	if (!hasSlot(slot))
	{
		error = CEC_NoSlot;
		return CachedNetworkId::cms_cachedInvalid;
	}

	int position = find(slot);
	if (position < 0)
	{
		error = CEC_NotFound;
		return CachedNetworkId::cms_cachedInvalid;
	}

	return getContents(position);
}

// ------------------------------------------------------------------------

/**
 * Retrieves all the objects in slots associated with a combat bone.
 *
 * @param bone		the combat bone id
 * @param objects	list that will be filled with the objects in the slots
 */
void SlottedContainer::getObjectsForCombatBone(uint32 bone,
	std::vector<Container::ContainedItem> & objects) const
{
	std::vector<SlotId> slots;
	SlotIdManager::findSlotIdsForCombatBone(bone, slots);
	std::vector<SlotId>::const_iterator iter;
	ContainerErrorCode tmp = CEC_Success;
	for (iter = slots.begin(); iter != slots.end(); ++iter)
	{
		Container::ContainedItem item = getObjectInSlot(*iter, tmp);
		if (item != NetworkId::cms_invalid)
			objects.push_back(item);
	}
}

// ------------------------------------------------------------------------

PropertyId SlottedContainer::getClassPropertyId()
{
	return PROPERTY_HASH(SlottedContainer, 0x7ED71F2E);
}

// ------------------------------------------------------------------------

bool SlottedContainer::getFirstUnoccupiedArrangement(const Object &item, int& arrangementIndex, ContainerErrorCode& error) const
{
	error = CEC_Success;
	arrangementIndex = -1;

	//-- get list of valid arrangements
	std::vector<int> tmp;
	getValidArrangements(item, tmp, error, true, true);

	bool result = false;

	if (tmp.empty())
	{
		result = false;
	}
	else
	{
		arrangementIndex = tmp.front();
		result = true;
	}

	return result;
}

// ------------------------------------------------------------------------
/**
 * Return a list of valid arrangement indices for the item in this container.
 *
 * @todo  Modify getValidArrangements to take an argument that checks for unoccupied arrangements.
 */

void SlottedContainer::getValidArrangements(const Object& item, std::vector<int>& returnList, ContainerErrorCode& error, bool returnOnFirst, bool unoccupiedArrangementsOnly) const
{
	error = CEC_Success;
	returnList.clear();

	const SlottedContainmentProperty* slottedContainment = safe_cast<const SlottedContainmentProperty *>(item.getProperty(SlottedContainmentProperty::getClassPropertyId()));
	if (!slottedContainment)
	{
		error = CEC_WrongType;
		return;
	}

	//@todo revisit slotted properties to use iterators
	int numArrangements = slottedContainment->getNumberOfArrangements();

	//For each arrangement, check ourself to see if each slot in that arrangement exists.
	//As soon as we find an arrangement that has all of our slots, we can return that arrangement index
	for (int i = 0; i < numArrangements; ++i)
	{
		const int numSlots       = slottedContainment->getNumberOfSlots(i);
		int       availableSlots = 0;
		bool         slotPresent    = true;

		for (int j = 0; j < numSlots; ++j)
		{
			SlotId slot = slottedContainment->getSlotId(i,j);

			// check if slot is present in this container
			if (!hasSlot(slot))
			{
				//A slot is not present, flag for next loop and break to the next loop
				slotPresent = false;
				break;
			}

			// check if slot is available
			if (unoccupiedArrangementsOnly && isSlotEmpty(slot, error))
				++availableSlots;
		}
		//If we haven't been flagged above then we found a valid arrangement
		if (slotPresent)
		{
			// check if we're ignoring unoccupied slots or if all slots were unoccupied
			if (!unoccupiedArrangementsOnly || (availableSlots == numSlots))
			{
				returnList.push_back(i);
				if (returnOnFirst)
					return;
			}
		}
	}

	//We found no arrangements on the item that only contains slot ids that we have.
	error = CEC_SlotOccupied;
	return;
}

// ------------------------------------------------------------------------
/**
 * Append the contents of the slotted container to the specified buffer.
 *
 * @param buffer  the string that will have debug information appended to it on return.
 */

void SlottedContainer::debugPrint(std::string &buffer) const
{
	char tempBuffer[1024];

	buffer += "====[BEGIN: slotted container]====\n";

		//-- List supported slots and container indices.
		sprintf(tempBuffer, "container id [%s]: contains [%d] slots.\n", getOwner().getNetworkId().getValueString().c_str(), static_cast<int>(m_slotMap->size()));
		buffer += tempBuffer;

		SlotMap::const_iterator const endIt = m_slotMap->end();
		for (SlotMap::const_iterator it = m_slotMap->begin(); it != endIt; ++it)
		{
			sprintf(tempBuffer, "\tslot [%3d, %20s]: index [%3d]\n", it->first.getSlotId(), SlotIdManager::getSlotName(it->first).getString(), it->second);
			buffer += tempBuffer;
		}

		buffer += "embedding container contents now:\n";
		Container::debugPrint(buffer);

	buffer += "====[END:   slotted container]====\n";
}

// ----------------------------------------------------------------------

void SlottedContainer::debugLog() const
{
#ifdef _DEBUG
	DEBUG_REPORT_LOG(true, ("====[BEGIN: slotted container]====\n"));

		//-- List supported slots and container indices.
		DEBUG_REPORT_LOG(true, ("container id [%s]: contains [%d] slots.\n", getOwner().getNetworkId().getValueString().c_str(), static_cast<int>(m_slotMap->size())));

		SlotMap::const_iterator const endIt = m_slotMap->end();
		for (SlotMap::const_iterator it = m_slotMap->begin(); it != endIt; ++it)
		{
			DEBUG_REPORT_LOG(true, ("\tslot [%20s]: index [%3d]\n", SlotIdManager::getSlotName(it->first).getString(), it->second));
		}

		DEBUG_REPORT_LOG(true, ("Embedding container contents now:\n"));
		Container::debugLog();

	DEBUG_REPORT_LOG(true, ("====[END:   slotted container]====\n"));
#endif
}

// --------------------------------------------------------------------------------

bool SlottedContainer::hasSlot (const SlotId &slot) const
{
	SlotMap::const_iterator i = m_slotMap->find(slot);
	if (i == m_slotMap->end())					//lint !e1702 STL operator== both a member and not
		return false;
	return true;
}

// --------------------------------------------------------------------------------

/**
 * Returns the slot ids for this container.
 *
 * @param slots		list that will be filled in with the slot ids
 */
void SlottedContainer::getSlotIdList(std::vector<SlotId> & slots) const
{
	SlotMap::const_iterator iter;
	for (iter = m_slotMap->begin(); iter != m_slotMap->end(); ++iter)
	{
		if ((*iter).first != SlotId::invalid)
			slots.push_back(iter->first);
	}
}

// ----------------------------------------------------------------------

bool SlottedContainer::isSlotEmpty (const SlotId &slot, ContainerErrorCode& error) const
{
	error = CEC_Success;
	// find slot data
	if (!hasSlot(slot))
	{
		error = CEC_NoSlot;
		return false;
	}

	bool retval = (find(slot) == -1);
	if (!retval)
		error = CEC_SlotOccupied;
	return retval;
}

// ------------------------------------------------------------------------

bool SlottedContainer::mayAdd(const Object& item, ContainerErrorCode& error) const
{
	error = CEC_Success;
	//@todo This is an inefficient function.  getValidArrangements and mayAdd(object, int) both do a lot of searching that is redundant.

	s_tempIntContainer.clear();
	getValidArrangements(item, s_tempIntContainer, error);

	std::vector<int>::iterator const endIt = s_tempIntContainer.end();
	for(std::vector<int>::iterator i = s_tempIntContainer.begin(); i != endIt; ++i)
	{
		if (mayAdd(item, (*i), error))
			return true;
	}
	return false;
}

// ------------------------------------------------------------------------

bool SlottedContainer::mayAdd(const Object& item, int arrangementIndex, ContainerErrorCode& error) const
{
	error = CEC_Success;
	if (arrangementIndex < 0)
	{
		error = CEC_InvalidArrangement;
		return false;
	}

	if (item.getNetworkId() == getOwner().getNetworkId())
	{
		error = CEC_AddSelf;
		return false;
	}

	const SlottedContainmentProperty* slottedContainment = safe_cast<const SlottedContainmentProperty *>(item.getProperty(SlottedContainmentProperty::getClassPropertyId()));
	if (!slottedContainment)
	{
		WARNING_STRICT_FATAL(true, ("Tried to check slots with an item with no slotted containement property."));
		error = CEC_Unknown;
		return false;
	}

	int numSlots = slottedContainment->getNumberOfSlots(arrangementIndex);
	for (int j = 0; j < numSlots; ++j)
	{
		SlotId slot = slottedContainment->getSlotId(arrangementIndex,j);
		if (!isSlotEmpty(slot, error))
		{
			//A slot does not exist or is occupied in the arrangement already.
			return false;
		}
	}
	return Container::mayAdd(item, error);
}

// ------------------------------------------------------------------------

bool SlottedContainer::mayAdd(const Object& item, const SlotId& slotId, ContainerErrorCode& error) const
{
	error = CEC_Success;
	if (!isSlotEmpty(slotId, error))
	{
		return false;
	}

	const SlottedContainmentProperty* slottedContainment = safe_cast<const SlottedContainmentProperty *>(item.getProperty(SlottedContainmentProperty::getClassPropertyId()));
	if (!slottedContainment)
	{
		WARNING_STRICT_FATAL(true, ("Tried to check slots with an item with no slotted containement property."));
		error = CEC_Unknown;
		return false;
	}
	return mayAdd(item, slottedContainment->getBestArrangementForSlot(slotId), error);

}


// ------------------------------------------------------------------------

bool SlottedContainer::internalRemove(const Object& item, int overrideArrangement)
{
	ContainerErrorCode tmp = CEC_Success;
	UNREF(overrideArrangement);
		//Check for the item's position in contents
	int position = Container::find(item.getNetworkId(), tmp);
	if (position == -1)
	{
		WARNING_STRICT_FATAL(true, ("SlottedContainer::internalRemove() called with an invalid item: container owner id=[%s], template=[%s]; item id=[%s], template=[%s]",
				getOwner().getNetworkId().getValueString().c_str(), getOwner().getObjectTemplateName(),
				item.getNetworkId().getValueString().c_str(), item.getObjectTemplateName()));
		return false;
	}

	//Clean up the slot map
	SlotMap::iterator i = m_slotMap->begin();
	for(;i != m_slotMap->end(); ++i)						//lint !e1702 STL operator== both a member and not
	{
		if(i->second == position)						//lint !e1702 STL operator== both a member and not
		{
			i->second = -1;
		}
	}
	return true;
}

// ------------------------------------------------------------------------

bool SlottedContainer::remove(Object& item, ContainerErrorCode& error)
{
	error = CEC_Success;
	SlottedContainmentProperty * slottedProperty = safe_cast<SlottedContainmentProperty *>(item.getProperty(SlottedContainmentProperty::getClassPropertyId()));
	if (!slottedProperty)
	{
		WARNING_STRICT_FATAL(true, ("Tried to remove an item %s to a slot container with no slotted property", item.getNetworkId().getValueString().c_str()));
		error = CEC_Unknown;
		return false;
	}

	//Check for the item's position in contents
	int position = Container::find(item.getNetworkId(), error);
	if (position == -1)
	{
		DEBUG_WARNING(true, ("Called SlottedContainer::remove() with item [%s] from container [%s], but the item was not found in the base container's contents.  This means the item was not in the container.", item.getNetworkId().getValueString().c_str(), getOwner().getNetworkId().getValueString().c_str()));
		error = CEC_NotFound;
		return false;
	}

	//Try to remove it from contents
	if (!internalRemove(item))
	{
		error = CEC_Unknown;
		return false;
	}

	if (!Container::remove(item, error))
	{
		return false;
	}

	slottedProperty->setCurrentArrangement(-1);
	return true;
}

// ----------------------------------------------------------------------

bool SlottedContainer::remove(ContainerIterator &pos, ContainerErrorCode& error)
{
	error = CEC_Success;
	Object *obj=(*pos).getObject();
	if (obj)
		return remove(*obj, error);
	else
	{
		error = CEC_Unknown;
		return false;
	}
}

// ------------------------------------------------------------------------

void SlottedContainer::removeItemFromSlotOnly(const Object &item)
{
	IGNORE_RETURN(internalRemove(item));
}

// ------------------------------------------------------------------------

int SlottedContainer::depersistContents(Object&)
{
	WARNING_STRICT_FATAL(true, ("Do not use this interface.  Use interface with arrangement instead."));
	return -1;
}

// ------------------------------------------------------------------------

void SlottedContainer::depersistSlotContents(Object& item, int arrangementIndex)
{
	ContainerErrorCode tmp = CEC_Success;
	SlottedContainmentProperty * slottedProperty = safe_cast<SlottedContainmentProperty *>(item.getProperty(SlottedContainmentProperty::getClassPropertyId()));
	if (!slottedProperty)
	{
		WARNING_STRICT_FATAL(true, ("Tried to add an item %s to a slot container with no slotted property\n", item.getNetworkId().getValueString().c_str()));
		return;
	}

//Special case...when depersisting an object its to have arrangmentIndex = -1 for baselines
	if (arrangementIndex < 0)
	{
		int tmp2 = Container::depersistContents(item);
		UNREF(tmp2);
		return;
	}


	//Make sure we can depersist into the arrangement we had last saved.
	if (!internalCheckSlottedAdd(item, arrangementIndex, tmp))
	{
		//if not try to find a valid one to depersist to.
		int arrangement = -1;
		DEBUG_WARNING(true, ("Could not depersist item [%s] into container [%s] with arrangement %d, ContainerErrorCode=%d.  This is typically bad, and may result in the item being in an invalid state or even moved to 0 0 0 in the world.  We will now attempt to find a different arrangement to depersist this item with.", item.getNetworkId().getValueString().c_str(), getOwner().getNetworkId().getValueString().c_str(), arrangementIndex, static_cast<int>(tmp)));
		if (!getFirstUnoccupiedArrangement(item, arrangement, tmp))
		{
			//warning/fatal if we can't find a valid arrangement
			WARNING_STRICT_FATAL(true, ("Could not depersist item [%s] into container [%s] because there was no good arrangement, ContainerErrorCode=%d.  See previous warning.  We then attempted to find a different arrangement to put the item into, because the item really belongs in this container, but could not because it was either already occupied or the arrangement of the item was changed so that it cannot be placed in this container anymore.   This is typically bad, and may result in the item being in an invalid state or even moved to 0 0 0 in the world.  This might happen because the arrangement file was changed for this object and existing objects could no longer be persisted into their containers.", item.getNetworkId().getValueString().c_str(), getOwner().getNetworkId().getValueString().c_str(), static_cast<int>(tmp)));
		}
		else
		{
			int position = Container::depersistContents(item);
			if (position >= 0 && internalDoSlottedAdd(item, position, arrangement))
			{
				//change the current arrangement to the new one we found
				slottedProperty->setCurrentArrangement(arrangement);
			}
			else
			{
				WARNING_STRICT_FATAL(true, ("Could not depersist item [%s] into container [%s] because add item failed.  We found a good different arrangement to add the item to, but the base container prevented the transfer.", item.getNetworkId().getValueString().c_str(), getOwner().getNetworkId().getValueString().c_str()));
			}
		}
	}

	//It's ok to depersist into the arrangement we were saved with.
	else
	{
		int position = Container::depersistContents(item);


		if (arrangementIndex >= 0 && !internalDoSlottedAdd(item, position, arrangementIndex))
		{
			WARNING_STRICT_FATAL(true, ("Could not depersist item [%s] into container [%s] because add item failed.  For some reason the base container prevented the transfer.", item.getNetworkId().getValueString().c_str(), getOwner().getNetworkId().getValueString().c_str()));
		}
	}
}

// ------------------------------------------------------------------------

bool SlottedContainer::internalItemRemoved(const Object& item)
{
	if (internalRemove(item))
	{
		return Container::internalItemRemoved(item);
	}

	return false;
}

// ------------------------------------------------------------------------

void SlottedContainer::updateArrangement(Object& item, int oldArrangement, int newArrangement)
{
	ContainerErrorCode tmp = CEC_Success;
	if (!internalRemove(item, oldArrangement))
	{
		DEBUG_WARNING(true, ("Remove part of update failed in slotted container"));
	}

	int position = Container::find(item.getNetworkId(), tmp);
	if (position < 0)
	{
		DEBUG_WARNING(true, ("could not find object in update slotted container"));
	}
	else if (newArrangement >= 0 && !internalDoSlottedAdd(item, position, newArrangement))
	{
		DEBUG_WARNING(true, ("updating slotted container failed"));
	}

}

//----------------------------------------------------------------------

const SlotId & SlottedContainer::findFirstSlotIdForObject (const NetworkId & id) const
{
	Container::ContainerErrorCode tmp = Container::CEC_Success;
	const int position = Container::find (id, tmp);
	if (position >= 0)
	{
		for (SlottedContainer::SlotMap::const_iterator sit = m_slotMap->begin(); sit != m_slotMap->end(); ++sit)
		{
			const int objPosition    = (*sit).second;

			if (objPosition == position)
				return (*sit).first;
		}
	}

	return SlotId::invalid;
}

// ------------------------------------------------------------------------
