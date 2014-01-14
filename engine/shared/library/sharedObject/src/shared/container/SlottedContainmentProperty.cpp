// ======================================================================
//
// SlottedContainmentProperty.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/SlottedContainmentProperty.h"

#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"
#include <string>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(SlottedContainmentProperty, true, 0, 0, 0);

// ======================================================================

SlottedContainmentProperty::SlottedContainmentProperty(Object& owner) : 
Property(SlottedContainmentProperty::getClassPropertyId(), owner),
m_arrangementList(),
m_currentArrangement(-1)
{
	//Add slots that can hold anything
	SlotIdManager::AnythingSlotVector anythingSlots = SlotIdManager::getSlotsThatHoldAnything();
	for (SlotIdManager::AnythingSlotVector::iterator i = anythingSlots.begin(); i != anythingSlots.end(); ++i)
	{
		SlotArrangement tmp;
		CrcLowerString tmp2(i->c_str());
		SlotId slotId = SlotIdManager::findSlotId(tmp2);
		tmp.push_back(slotId);
		addArrangement(tmp);
	}
}

// ---------------------------------------------------------------------


SlottedContainmentProperty::~SlottedContainmentProperty()
{

}

// ---------------------------------------------------------------------

void SlottedContainmentProperty::addArrangement(const SlotArrangement &arrangement)
{
	if (arrangement.size() >= SCP_MaxArrangementSize)
	{
		WARNING_STRICT_FATAL(true, ("Cannot add this arrangement.  It is too big."));
		return;
	}
	m_arrangementList.push_back(arrangement);
}

// ---------------------------------------------------------------------

bool SlottedContainmentProperty::canManipulateArrangement(int arrangementIndex) const
{
	if (arrangementIndex < 0)
	{
		WARNING_STRICT_FATAL(true, ("Passed -1 to canManipulateArrangement"));
		return true;
	}
	
	int numArrangements = getNumberOfArrangements();
	if (numArrangements == 0)
		return false;
	if (arrangementIndex >= numArrangements)
	{
		const Object& owner = getOwner();
		WARNING_STRICT_FATAL(true, ("Tried to pass an invalid arrangement index %d for object %s", arrangementIndex, owner.getNetworkId().getValueString().c_str()));
		return false;
	}

	std::vector<SlotId>::const_iterator i = m_arrangementList[static_cast<ArrangementList::size_type>(arrangementIndex)].begin();
	for (;i != m_arrangementList[static_cast<ArrangementList::size_type>(arrangementIndex)].end(); ++i)
	{
		if (!SlotIdManager::isSlotPlayerModifiable(*i))
			return false;
	}
	return true;
}


// ---------------------------------------------------------------------

/**
*  This function returns the "best" arrangement that has a given slotId.
*  The best arrangement is the one that has the given slotId nearest to the front of 
*  the slot list.
*/
int SlottedContainmentProperty::getBestArrangementForSlot(const SlotId & slotId) const
{
	//@todo Make sure arrangements are added in data in the correct order to ensure "bestness"
	int bestValue = SCP_MaxArrangementSize;  
	int bestArrangement = -1;

	int arrangementIndex = 0;
	for (ArrangementList::const_iterator i = m_arrangementList.begin(); i != m_arrangementList.end(); ++i, ++arrangementIndex)
	{
		int slotIndex = 0;
		for (SlotArrangement::const_iterator j = i->begin(); j != i->end(); ++j, ++slotIndex)
		{
			if (slotId == *j && slotIndex < bestValue)
			{
				bestValue = slotIndex;
				bestArrangement = arrangementIndex;
				break;
			}
		}

		// quick test to see if we can stop iterating.
		if (bestValue == 0)
			break;
	}

	return bestArrangement;
}

// ---------------------------------------------------------------------

int SlottedContainmentProperty::getNumberOfArrangements () const
{
	return static_cast<int>(m_arrangementList.size());
}

// ---------------------------------------------------------------------

int SlottedContainmentProperty::getNumberOfSlots (int arrangementIndex) const
{
	int numArrangements = getNumberOfArrangements();
	if (numArrangements == 0)
		return 0;
	if (arrangementIndex < 0 || arrangementIndex >= numArrangements)
	{
		WARNING_STRICT_FATAL(true, ("Tried to pass an invalid arrangement index to this slotted container"));
		return 0;
	}
	return static_cast<int>(m_arrangementList[static_cast<ArrangementList::size_type>(arrangementIndex)].size());
}

// ---------------------------------------------------------------------

PropertyId SlottedContainmentProperty::getClassPropertyId()
{
	return PROPERTY_HASH(SlottedContainment, 0xED3067A9);
}

// -----------------------------------------------------------

const SlottedContainmentProperty::SlotArrangement & SlottedContainmentProperty::getSlotArrangement(int arrangementIndex) const
{
	if (arrangementIndex < 0 || arrangementIndex >= getNumberOfArrangements())
	{
		WARNING_STRICT_FATAL(true, ("Tried to pass an invalid arrangement index to getSlotArrangement"));
		return m_arrangementList[0];
	}
	return m_arrangementList[static_cast<ArrangementList::size_type>(arrangementIndex)];
}


// -----------------------------------------------------------

SlotId SlottedContainmentProperty::getSlotId (int arrangementIndex, int slotIndex) const
{
	if (arrangementIndex < 0)
	{
		WARNING_STRICT_FATAL(true, ("Tried to pass a negative arrangement index [%d] to this slotted container", arrangementIndex));
		return SlotId::invalid;
	}

	if (slotIndex < 0)
	{
		WARNING_STRICT_FATAL(true, ("Tried to pass a negative slotIndex [%d] to this slotted container", slotIndex));
		return SlotId::invalid;
	}

	int num = getNumberOfArrangements();
	if (num == 0)
		return SlotId::invalid;
	if (num < arrangementIndex)
	{
		WARNING_STRICT_FATAL(true, ("Tried to pass an invalid arrangement index to this slotted container"));
		return SlotId::invalid;
	}
	
	num = getNumberOfSlots(arrangementIndex);
	if (num == 0)
		return SlotId::invalid;
	if (num < slotIndex)
	{
		WARNING_STRICT_FATAL(true, ("Tried to pass an invalid slot index to this slotted container"));
		return SlotId::invalid;
	}

	return m_arrangementList[static_cast<ArrangementList::size_type>(arrangementIndex)][static_cast<SlotArrangement::size_type>(slotIndex)];
}

// ---------------------------------------------------------------------

bool SlottedContainmentProperty::isInAppearanceSlot() const
{
	int arrangement = getCurrentArrangement();
	int numSlots = getNumberOfSlots(arrangement);
	for (int i = 0; i < numSlots; ++i)
	{
		SlotId id = getSlotId(arrangement, i);
		if (SlotIdManager::isSlotAppearanceRelated(id))
			return true;
	}
	return false;
}

// ---------------------------------------------------------------------

int SlottedContainmentProperty::getCurrentArrangement() const
{
	return m_currentArrangement;
}

// ------------------------------------------------------------------------

void SlottedContainmentProperty::setCurrentArrangement(int arrangement, bool local)
{
	if (m_currentArrangement != arrangement)
	{
		int oldValue = m_currentArrangement;
		m_currentArrangement = arrangement;
		getOwner().arrangementModified(oldValue, arrangement, local);
	}
}

// ======================================================================

