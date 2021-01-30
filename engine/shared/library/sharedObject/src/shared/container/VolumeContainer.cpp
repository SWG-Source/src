// ======================================================================
//
// VolumeContainer.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================


#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/VolumeContainer.h"

#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/VolumeContainmentProperty.h"
#include "sharedObject/ObjectTemplate.h"

#include "sharedFoundation/CrcLowerString.h"

#include <algorithm>
#include <cstdio>

//-----------------------------------------------------------------------

namespace VolumeContainerNamespace
{
	VolumeContainer* getVolumeContainerParent(VolumeContainer& self)
	{
		Object & owner = self.getOwner ();
		ContainedByProperty * containedByProperty = owner.getContainedByProperty ();
		if (containedByProperty)
		{
			Object * parent = containedByProperty->getContainedBy ();
			if (parent)
			{
				return parent->getVolumeContainerProperty();
			}
		}
		return nullptr;
	}

	const VolumeContainer* getVolumeContainerParent(const VolumeContainer& self)
	{
		const Object & owner = self.getOwner ();
		const ContainedByProperty * const containedByProperty = owner.getContainedByProperty ();
		if (containedByProperty)
		{
			const Object * const parent = containedByProperty->getContainedBy ();
			if (parent)
			{
				return parent->getVolumeContainerProperty();
			}
		}
		return nullptr;
	}

	const unsigned int serverHolocronCrc = CrcLowerString::calculateCrc("object/player_quest/pgc_quest_holocron.iff");
}

using namespace VolumeContainerNamespace;


//-----------------------------------------------------------------------

VolumeContainer::VolumeContainer(Object& owner, int totalVolume) :
Container(VolumeContainer::getClassPropertyId(), owner),
m_currentVolume(0),
m_totalVolume(totalVolume)
{
}

//-----------------------------------------------------------------------

VolumeContainer::~VolumeContainer()
{
}

//-----------------------------------------------------------------------

void VolumeContainer::childVolumeChanged(int volume, bool updateParent)
{
	if (m_totalVolume != VL_NoVolumeLimit)
		m_currentVolume += volume;

	if (updateParent)
	{
		VolumeContainer *parent = getVolumeContainerParent(*this);
		if (parent)
			parent->childVolumeChanged(volume, true);
	}
}

//-----------------------------------------------------------------------


void VolumeContainer::insertNewItem(const Object & item, const VolumeContainmentProperty * itemProp)
{

	const VolumeContainmentProperty * prop = itemProp ? itemProp : safe_cast<const VolumeContainmentProperty *>(item.getProperty(VolumeContainmentProperty::getClassPropertyId()));
	if (!prop)
	{
		WARNING_STRICT_FATAL(true, ("Item %s has no volume property", item.getNetworkId().getValueString().c_str()));
		return;
	}
	if (m_totalVolume != VL_NoVolumeLimit)
		m_currentVolume += prop->getVolume();
}

//-----------------------------------------------------------------------

bool VolumeContainer::add (Object& item, ContainerErrorCode& error, bool allowOverloaded)
{
	// save off current volume
	const int oldVolume = m_currentVolume;

	error = CEC_Success;
	VolumeContainmentProperty * prop = safe_cast<VolumeContainmentProperty *>(item.getProperty(VolumeContainmentProperty::getClassPropertyId()));
	if (!prop)
	{
		WARNING_STRICT_FATAL(true, ("Cannot add an item to a volume container without a containment property."));
		error = CEC_Unknown;
		return false;
	}

	if (!allowOverloaded && !checkVolume(*prop))
	{
		error = CEC_Full;
		return false;
	}

	if (addToContents(item, error) == -1)
	{
		return false;
	}

	insertNewItem(item, prop);

	// if volume has changed, update parent volume count
	if (m_currentVolume != oldVolume)
	{
		VolumeContainer *parent = getVolumeContainerParent(*this);
		if (parent)
			parent->childVolumeChanged((m_currentVolume - oldVolume), true);
	}

	return true;
}

//-----------------------------------------------------------------------

bool VolumeContainer::checkVolume (const VolumeContainmentProperty &item) const
{
	return checkVolume(item.getVolume());
}

//-----------------------------------------------------------------------

bool VolumeContainer::checkVolume (int addedVolume) const
{
	if (m_totalVolume == VL_NoVolumeLimit)
		return true;

	bool retval = false;
	
	retval =  (m_currentVolume + addedVolume <= m_totalVolume);

//-- check parent to make sure it isn't full
	if (retval)
	{
		const VolumeContainer * const volumeContainer = getVolumeContainerParent(*this);
		if (volumeContainer)
		{
			retval = volumeContainer->checkVolume(addedVolume);
		}
	}
	return retval;
}
//-----------------------------------------------------------------------

int VolumeContainer::debugDoNotUseSetCapacity(int newValue)
{
	int oldValue = m_totalVolume;
	m_totalVolume = newValue;
	return oldValue;
}

//-----------------------------------------------------------------------


void VolumeContainer::debugPrint(std::string &buffer) const
{
	char tempBuffer[1024];

	buffer += "====[BEGIN: volume container]====\n";

		sprintf(tempBuffer, "container id [%s]: current volume [%d] total volume [%d].\n", getOwner().getNetworkId().getValueString().c_str(),
			m_currentVolume, m_totalVolume);
		buffer += tempBuffer;

		buffer += "embedding container contents now:\n";
		Container::debugPrint(buffer);

	buffer += "====[END:   volume container]====\n";
}

//-----------------------------------------------------------------------

PropertyId VolumeContainer::getClassPropertyId()
{
	return PROPERTY_HASH(VolumeContainer, 0xA5193F23);
}

//-----------------------------------------------------------------------


bool VolumeContainer::mayAdd(const Object& item, ContainerErrorCode& error) const
{
	error = CEC_Success;
	if (item.getNetworkId() == getOwner().getNetworkId())
	{
		error = CEC_AddSelf;
		return false;
	}

	const VolumeContainmentProperty * prop = safe_cast<const VolumeContainmentProperty *>(item.getProperty(VolumeContainmentProperty::getClassPropertyId()));
	if (!prop)
	{
		error = CEC_Unknown;
		return false;
	}

	const VolumeContainer * vol = safe_cast<const VolumeContainer *>(item.getProperty(VolumeContainer::getClassPropertyId()));
	if (vol)
	{
		if (m_totalVolume != VL_NoVolumeLimit && vol->getTotalVolume() >= getTotalVolume())
		{
			unsigned int ownerCrc = CrcLowerString::calculateCrc(getOwner().getObjectTemplateName());
			unsigned int itemCrc = CrcLowerString::calculateCrc(item.getObjectTemplateName());
			
			if(ownerCrc == serverHolocronCrc && itemCrc == serverHolocronCrc)
			{
				// We're moving a holocron into another holocron, let it continue.
			}
			else
			{
				error = CEC_TooLarge;
				return false;
			}

		}
	}

	if (!checkVolume(*prop))
	{
		error = CEC_Full;
		return false;
	}

	return Container::mayAdd(item, error);
}

//-----------------------------------------------------------------------

int VolumeContainer::recalculateVolume()
{
	int volume = 0;
	for (ContainerIterator iter = begin(); iter != end(); ++iter)
	{
		Object* const obj = (*iter).getObject();
		if (!obj)
		{
			WARNING_STRICT_FATAL(true, ("Container with non-existant object %s", (*iter).getValueString().c_str()));
		}
		else
		{
			VolumeContainmentProperty * const prop = safe_cast<VolumeContainmentProperty *>(obj->getProperty(VolumeContainmentProperty::getClassPropertyId()));
			if (!prop)
			{
				WARNING_STRICT_FATAL(true, ("We have an item in a volume container with no property %s", (*iter).getValueString().c_str()));
			}
			else if (m_totalVolume != VL_NoVolumeLimit)
			{
				volume += prop->getVolume();
			}

		}
	}
	m_currentVolume = volume;
	
	//-- if our volume is recalculated, we must inform our parent if contained by a volume container
	VolumeContainer * const volumeContainer = getVolumeContainerParent(*this);
	if (volumeContainer)
		IGNORE_RETURN (volumeContainer->recalculateVolume ());

	DEBUG_WARNING(m_totalVolume > 0 && m_currentVolume > m_totalVolume, ("Recalculate Volume ended up being greater than our capacity"));
	return m_currentVolume;
}

//-----------------------------------------------------------------------

bool VolumeContainer::internalRemove(const Object & item, const VolumeContainmentProperty * itemProp)
{
	const VolumeContainmentProperty * prop = itemProp ? itemProp : safe_cast<const VolumeContainmentProperty *>(item.getProperty(VolumeContainmentProperty::getClassPropertyId()));
	if (!prop)
	{
		WARNING_STRICT_FATAL(true, ("Item %s has no volume property", item.getNetworkId().getValueString().c_str()));
		return false;
	}

	if (m_totalVolume != VL_NoVolumeLimit)
		m_currentVolume = m_currentVolume - prop->getVolume();

	return true;
}

//-----------------------------------------------------------------------

bool VolumeContainer::remove (Object &item, ContainerErrorCode& error)
{
	// save off current volume
	const int oldVolume = m_currentVolume;

	error = CEC_Success;
	VolumeContainmentProperty * prop = safe_cast<VolumeContainmentProperty *>(item.getProperty(VolumeContainmentProperty::getClassPropertyId()));
	if (!prop)
	{
		WARNING_STRICT_FATAL(true, ("Cannot remove an item from a volume container without a containment property."));
		error = CEC_Unknown;
		return false;
	}

	bool retval = Container::remove(item, error);
	if (!retval)
	{
		return false;
	}
	if (!internalRemove(item))
	{
		WARNING_STRICT_FATAL(true, ("tried to remove item %s from volume container in internal routine but failed", item.getNetworkId().getValueString().c_str()));
		error = CEC_Unknown;
		return false;
	}

	// if volume has changed, update parent volume count
	if (m_currentVolume != oldVolume)
	{
		VolumeContainer *parent = getVolumeContainerParent(*this);
		if (parent)
			parent->childVolumeChanged((m_currentVolume - oldVolume), true);
	}

	return true;
}

// ----------------------------------------------------------------------

bool VolumeContainer::remove(ContainerIterator &pos, ContainerErrorCode  & error)
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

//-----------------------------------------------------------------------

int VolumeContainer::depersistContents(const Object& item)
{
	// save off current volume
	const int oldVolume = m_currentVolume;

	insertNewItem(item);

	// if volume has changed, update parent volume count
	if (m_currentVolume != oldVolume)
	{
		// if we are contained in another volume container, we
		// need up update the count in the parent container;
		// however, if we are here because of a baseline, we don't
		// need to update the parent container because when the
		// parent adds us, it will update the count at that time;
		// if we are here because of a delta, then we need to
		// update the parent; if we are here because of a baseline,
		// then we are not yet in the parent container, even though
		// we have a pointer to the parent container
		VolumeContainer *parent = getVolumeContainerParent(*this);
		if (parent)
		{
			bool containedInParent = false;

			for (ContainerIterator i = parent->begin(); i != parent->end(); ++i)
			{
				if ((*i) == getOwner().getNetworkId())
				{
					containedInParent = true;
					break;
				}
			}

			if (containedInParent)
				parent->childVolumeChanged((m_currentVolume - oldVolume), true);
		}
	}

	return Container::depersistContents(item);
}


//-----------------------------------------------------------------------

bool VolumeContainer::internalItemRemoved(const Object& item)
{
	// save off current volume
	const int oldVolume = m_currentVolume;

	if (internalRemove(item))
	{
		// if volume has changed, update parent volume count
		if (m_currentVolume != oldVolume)
		{
			const VolumeContainmentProperty * prop = safe_cast<const VolumeContainmentProperty *>(item.getProperty(VolumeContainmentProperty::getClassPropertyId()));
			if (prop)
			{
				VolumeContainer *parent = getVolumeContainerParent(*this);
				if (parent)
					parent->childVolumeChanged((m_currentVolume - oldVolume), true);
			}
		}

		return Container::internalItemRemoved(item);
	}
	return false;
}

//----------------------------------------------------------------------

int VolumeContainer::getTotalVolumeLimitedByParents   () const
{
	int totalVolume     = m_totalVolume;

	//-- container is unlimited
	if (totalVolume <= 0)
		return totalVolume;

	int remainingVolume = m_totalVolume - m_currentVolume;
	
	const Object * parent = &getOwner ();
	
	while (parent)
	{
		const ContainedByProperty * const containedByProperty = parent->getContainedByProperty ();

		parent = containedByProperty->getContainedBy ();
		if (parent)
		{
			const VolumeContainer * const volumeContainer = parent->getVolumeContainerProperty();
			if (!volumeContainer)
				break;
			
			const int parentTotalVolume   = volumeContainer->getTotalVolume   ();
			const int parentCurrentVolume = volumeContainer->getCurrentVolume ();
			
			//-- we've reached an unlimited container
			if (parentTotalVolume <= 0)
				break;
			
			const int remainingInParent   = std::max (0, parentTotalVolume - parentCurrentVolume);
			remainingVolume               = std::min (remainingVolume, remainingInParent);
		}
	}
	
	return m_currentVolume + remainingVolume;
}

//-----------------------------------------------------------------------

