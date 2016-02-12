// ======================================================================
//
// Container.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/Container.h"

#include "sharedObject/AlterResult.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedObject/ConfigSharedObject.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Object.h"

#include <algorithm>
#include <cstdio>

// ======================================================================
//Lint suppressions.

//lint -esym(613,  ContainerIterator::m_iterator) // (Warning -- Possible use of nullptr pointer 'ContainerIterator::m_iterator' ) // This is an opaque data type to us.  We can only check against a container::end().
//lint -esym(1540, ContainerIterator::m_iterator) // (Warning -- Pointer member 'ContainerIterator::m_iterator' neither freed nor zero'ed by destructor -- Effective C++ #6) // It's okay, the iterator type is opaque to us, we have no idea it's a pointer.
//lint -esym(1554, ContainerIterator::m_iterator) // (Warning -- Direct copy of pointer 'ContainerIterator::m_iterator'. // It's okay, the iterator type is opaque to us, we have no idea it's a pointer.
//lint -esym(1555, ContainerIterator::m_iterator) // (Warning -- Direct pointer copy of member 'ContainerIterator::m_iterator' within copy assignment operator: 'ContainerIterator::operator=(const ContainerIterator &) // Opaque type, we don't know its a pointer.
//lint -esym(1555, ContainerIterator::m_owner)    // (Warning -- Direct pointer copy of member 'ContainerIterator::m_owner' within copy assignment operator: 'ContainerIterator::operator=(const ContainerIterator &)    // We do not own this memory, so it's okay to overwrite the pointer.  We can't leak it.

//lint -esym(613,  ContainerConstIterator::m_iterator) // (Warning -- Possible use of nullptr pointer 'ContainerConstIterator::m_iterator' ) // This is an opaque data type to us.  We can only check against a container::end().
//lint -esym(1540, ContainerConstIterator::m_iterator) // (Warning -- Pointer member 'ContainerConstIterator::m_iterator' neither freed nor zero'ed by destructor -- Effective C++ #6) // It's okay, the iterator type is opaque to us, we have no idea it's a pointer.
//lint -esym(1554, ContainerConstIterator::m_iterator) // (Warning -- Direct copy of pointer 'ContainerConstIterator::m_iterator'. // It's okay, the iterator type is opaque to us, we have no idea it's a pointer.
//lint -esym(1555, ContainerConstIterator::m_iterator) // (Warning -- Direct pointer copy of member 'ContainerConstIterator::m_iterator' within copy assignment operator: 'ContainerConstIterator::operator=(const ContainerConstIterator &) // Opaque type, we don't know its a pointer.
//lint -esym(1555, ContainerConstIterator::m_owner)    // (Warning -- Direct pointer copy of member 'ContainerConstIterator::m_owner' within copy assignment operator: 'ContainerConstIterator::operator=(const ContainerConstIterator &)    // We do not own this memory, so it's okay to overwrite the pointer.  We can't leak it.

//-----------------------------------------------------------------------

namespace ContainerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int checkDepth(const Container& container)
	{
		int count = 0;
		const ContainedByProperty* parentCpb = container.getOwner().getContainedByProperty();
		if (!parentCpb)
			return count;
		
		const Object* parentObject = parentCpb->getContainedBy();
		if (!parentObject)
			return count;

		while(parentObject)
		{
			++count;
			parentCpb = parentObject->getContainedByProperty();
			if (!parentCpb)
				return count;
			parentObject = parentCpb->getContainedBy();
		}
		return count;
	}

}

using namespace ContainerNamespace;

//-----------------------------------------------------------------------

ContainerIterator::ContainerIterator() :
m_iterator(),
m_owner()
{
}
//-----------------------------------------------------------------------

ContainerIterator::ContainerIterator(Container & owner, const std::vector<CachedNetworkId>::iterator & iter) :
m_iterator(iter),
m_owner(&owner)
{
}
//-----------------------------------------------------------------------

ContainerIterator::~ContainerIterator()
{
	// We don't own the owner pointer.
	m_owner = 0;
}
//-----------------------------------------------------------------------

ContainerIterator::ContainerIterator(const ContainerIterator & rhs) :
m_iterator(rhs.m_iterator),
m_owner(rhs.m_owner) //lint !e1554 // Direct copy of pointer member. // @todo revisit.  There doesn't appear to be any mechanism in place to guarantee the owner container sticks around for the iterator.  This is dangerous.
{
}
//-----------------------------------------------------------------------

ContainerIterator & ContainerIterator::operator= (const ContainerIterator & rhs)
{
	if ((&rhs == this) || (rhs == *this))
		return *this;
	m_iterator = rhs.m_iterator;
	m_owner = rhs.m_owner;
	return *this;
}
//-----------------------------------------------------------------------

CachedNetworkId & ContainerIterator::operator*()
{
	// @todo fix this interface -- it is broken.  It is possible to
	// construct a ContainerIterator with a nullptr m_iterator, but there is
	// no way to gracefully exit from this function since it requires a non-const
	// reference.  Otherwise I would return CachedNetworkId::cms_cachedIvalid.
	NOT_NULL(m_owner);
	DEBUG_FATAL(m_iterator == m_owner->m_contents.end(), ("Tried to dereference invalid iterator.\n"));

	return *m_iterator;
}

//-----------------------------------------------------------------------

bool ContainerIterator::operator==(const ContainerIterator & rhs) const
{
	return m_iterator == rhs.m_iterator;
}
//-----------------------------------------------------------------------

bool ContainerIterator::operator!=(const ContainerIterator & rhs) const
{
	return !(rhs == *this);
}


ContainerIterator ContainerIterator::operator++(int tmp)
{
	UNREF(tmp);
	ContainerIterator *retval = this; 
	++(*this);
	return *retval;
}
//-----------------------------------------------------------------------

ContainerIterator& ContainerIterator::operator++()
{
	NOT_NULL(m_owner);
	if (m_iterator == m_owner->m_contents.end())
		return *this;

	++m_iterator;
	while (m_iterator != m_owner->m_contents.end() && *m_iterator == NetworkId::cms_invalid)
	{
		++m_iterator;
	}
	return *this;
}

//-----------------------------------------------------------------------

ContainerConstIterator::ContainerConstIterator() :
m_iterator(),
m_owner()
{
}
//-----------------------------------------------------------------------

ContainerConstIterator::ContainerConstIterator(const Container & owner, const std::vector<CachedNetworkId>::const_iterator & iter) :
m_iterator(iter),
m_owner(&owner)
{
}
//-----------------------------------------------------------------------

ContainerConstIterator::~ContainerConstIterator()
{
	// We don't own the m_owner pointer.
	m_owner = 0;
}
//-----------------------------------------------------------------------

ContainerConstIterator::ContainerConstIterator(const ContainerConstIterator & rhs) :
m_iterator(rhs.m_iterator),
m_owner(rhs.m_owner) //lint !e1554 // Direct copy of pointer member. // @todo revisit.  There doesn't appear to be any mechanism in place to guarantee the owner container sticks around for the iterator.  This is dangerous.
{
}
//-----------------------------------------------------------------------

ContainerConstIterator & ContainerConstIterator::operator= (const ContainerConstIterator & rhs)
{
	if ((&rhs == this) || (rhs == *this))
		return *this;
	m_iterator = rhs.m_iterator;
	m_owner = rhs.m_owner;
	return *this;
}
//-----------------------------------------------------------------------

const CachedNetworkId & ContainerConstIterator::operator*() const
{
	// @todo fix this interface -- it is broken.  It is possible to
	// construct a ContainerIterator with a nullptr m_iterator, but there is
	// no way to gracefully exit from this function since it requires a non-const
	// reference.  Otherwise I would return CachedNetworkId::cms_cachedIvalid.
	NOT_NULL(m_owner);
	if (m_iterator == m_owner->m_contents.end())
	{
		DEBUG_WARNING(true, ("Tried to dereference invalid iterator.\n"));
		return CachedNetworkId::cms_cachedInvalid;
	}

	return *m_iterator;
}

//-----------------------------------------------------------------------

bool ContainerConstIterator::operator==(const ContainerConstIterator & rhs) const
{
	return m_iterator == rhs.m_iterator;
}
//-----------------------------------------------------------------------

bool ContainerConstIterator::operator!=(const ContainerConstIterator & rhs) const
{
	return !(rhs == *this);
}

//-----------------------------------------------------------------------

ContainerConstIterator ContainerConstIterator::operator++(int tmp)
{
	UNREF(tmp);
	ContainerConstIterator *retval = this; 
	++(*this);
	return *retval;
}

//-----------------------------------------------------------------------

ContainerConstIterator& ContainerConstIterator::operator++()
{
	NOT_NULL(m_owner);

	if (m_iterator == m_owner->m_contents.end())
		return *this;

	++m_iterator;
	while (m_iterator != m_owner->m_contents.end() && *m_iterator == NetworkId::cms_invalid)
	{
		++m_iterator;
	}


	return *this;
}

//-----------------------------------------------------------------------

Container::Container(PropertyId propertyId, Object& owner) :
Property(propertyId, owner),
m_changed(false),
m_contents(),
m_numContents(0)
{
}

//-----------------------------------------------------------------------

Container::~Container()
{
	Contents::const_iterator i = m_contents.begin();
	for(;i != m_contents.end(); ++i)
	{
		if (*i != NetworkId::cms_invalid) //was getValue
		{
			Object *const object = i->getObject();
			if (object)
			{
				IGNORE_RETURN(AlterScheduler::removeObject(*object));
				delete object;
			}
		}
	}
}

//-----------------------------------------------------------------------

ContainerIterator Container::begin()
{
	//Getting ready to put in fix for iterator code.  
	ContainerIterator ret(*this, m_contents.begin());
	while (ret.m_iterator != m_contents.end() && *ret == NetworkId::cms_invalid)
	{
		++ret.m_iterator;
	}
	return ret;
}

//-----------------------------------------------------------------------

ContainerIterator Container::end()
{
	return ContainerIterator(*this, m_contents.end());
}

//-----------------------------------------------------------------------

ContainerConstIterator Container::begin() const
{
	//Getting ready to put in fix for iterator code.  
	ContainerConstIterator ret(*this, m_contents.begin());
	while (ret.m_iterator != m_contents.end() && *ret == NetworkId::cms_invalid)
	{
		++ret.m_iterator;
	}
	return ret;
}

//-----------------------------------------------------------------------

ContainerConstIterator Container::end() const
{
	return ContainerConstIterator(*this, m_contents.end());
}

//-----------------------------------------------------------------------

int Container::addToContents(Object& item, ContainerErrorCode& error)
{
	error = CEC_Success;

	if (!mayAdd(item, error))
		return -1;

	ContainedByProperty * containedItem = item.getContainedByProperty();
	FATAL(!containedItem, ("Cannot add an item with no containedByProperty to a container."));
	if (containedItem->getContainedBy() && containedItem->getContainedBy() != &getOwner())
	{
		WARNING_STRICT_FATAL(true, ("Cannot add an item [%s] to a container [%s] when it is already contained!", item.getNetworkId().getValueString().c_str(), getOwner().getNetworkId().getValueString().c_str()));
		error = CEC_AlreadyIn;
		return -1;
	}

	if (ConfigSharedObject::getContainerLoopChecking())
	{
		//Check that we don't introduce an infinite loop
		//First check for container property. Only containers can introduce a loop
		Container* contCheck = item.getContainerProperty();
		if (contCheck)
		{
			ContainedByProperty * cbIter = getOwner().getContainedByProperty();
			if (cbIter)
			{
				NetworkId iterId = cbIter->getContainedByNetworkId();
				if (iterId != NetworkId::cms_invalid)
				{
					std::vector<NetworkId> checkVector;
					checkVector.reserve(10);
					checkVector.push_back(getOwner().getNetworkId());
					for( int count = 0; iterId != NetworkId::cms_invalid; ++count)
					{
						if (count > ConfigSharedObject::getContainerMaxDepth())
						{
							DEBUG_FATAL(true, ("Too deep a container heirarchy"));
							WARNING(true, ("Too deep a container heirarchy")); //lint !e527 // unreachable // reachable in release.
							error = CEC_TooDeep;
							return -1;
						}
						checkVector.push_back(iterId);
						Object* obj = cbIter->getContainedBy();
						iterId = NetworkId::cms_invalid;
						if (obj)
						{
							cbIter = obj->getContainedByProperty();
							if (cbIter)
							{
								iterId = cbIter->getContainedByNetworkId();
							}
						}
						
						
					}
					if (std::find(checkVector.begin(), checkVector.end(), item.getNetworkId()) != checkVector.end())
					{
						//detected a loop.
						WARNING_STRICT_FATAL(true, ("Adding item %s to %s would have introduced a container loop", item.getNetworkId().getValueString().c_str(), getOwner().getNetworkId().getValueString().c_str()));
						error = CEC_AlreadyIn;
						return -1;
						
					}
				}
			}
			
		}
	}

	if (checkDepth(*this) > ConfigSharedObject::getContainerMaxDepth())
	{
		error = CEC_TooDeep;
		return -1;
	}
	
	if (isItemContained(item.getNetworkId(), error))
	{
		WARNING_STRICT_FATAL(true, ("Cannot add an item [%s] to a container [%s] when it is already in it!  This shouldn't happen because the item's contained by property says it is not in this container, but it is in the container's internal list.", item.getNetworkId().getValueString().c_str(), getOwner().getNetworkId().getValueString().c_str()));
		error = CEC_AlreadyIn;
		return -1;
	}

	const CachedNetworkId id(item);

	containedItem->setContainedBy(getOwner().getNetworkId());
	int retval =  insertNewItem(id);
	if (retval == -1)
		error = CEC_Unknown;
	return retval;
}

//-----------------------------------------------------------------------

PropertyId Container::getClassPropertyId()
{
	return PROPERTY_HASH(Container, 0x55DC5726);
}

//-----------------------------------------------------------------------

int Container::insertNewItem(const Container::ContainedItem& id)
{
	ContainerErrorCode tmp = CEC_Success;;
	if (isItemContained(id, tmp))
	{
		DEBUG_WARNING(true, ("Cannot add item [%s] to container [%s] when it is already in it!", id.getValueString().c_str(), getOwner().getNetworkId().getValueString().c_str()));
		return -1;
	}
	
	if (id == getOwner().getNetworkId())
	{
		WARNING_STRICT_FATAL(true, ("Tried to add an item %s to itself!\n", id.getValueString().c_str()));
		return -1;
	}


	Contents::iterator i = m_contents.begin();
	int count = 0;
	++m_numContents;
	for(;i != m_contents.end(); ++i)
	{
		if (*i == NetworkId::cms_invalid)
		{
			(*i) = id;
			return count;
		}
		++count;
	}
	m_contents.push_back(id);
	return count;
}

//-----------------------------------------------------------------------


int Container::find(const NetworkId& item, ContainerErrorCode& error) const
{
	error = CEC_Success;
	int count = 0;
	Contents::const_iterator i = m_contents.begin();
	for(;i != m_contents.end(); ++i)
	{
		if ((*i) == item)
		{
			return count;
		}
		++count;
	}
	error = CEC_NotFound;
	return -1;
}

//-----------------------------------------------------------------------

int Container::getNumberOfItems (void) const
{
	return m_numContents;
}

//-----------------------------------------------------------------------

void Container::addToWorld()
{
	for (Contents::const_iterator i = m_contents.begin(); i != m_contents.end(); ++i)
	{
		Object *obj = (*i).getObject();
		if (obj && isContentItemExposedWith(*obj))
			obj->addToWorld();
	}
}

//-----------------------------------------------------------------------

void Container::removeFromWorld()
{
	for (Contents::const_iterator i = m_contents.begin(); i != m_contents.end(); ++i)
	{
		Object *obj = (*i).getObject();
		if (obj && isContentItemExposedWith(*obj))
			obj->removeFromWorld();
	}
}

// ----------------------------------------------------------------------
/**
 * Returns true if the item is or would be observed with its container.
 *
 * This function can be called on an item in the container or an item
 * that the caller already knows it will be putting in the container.
 *
 * If this function returns true, it indicates that the contained item
 * should be added to the world and should be observed whenever the same
 * happens to the container. In other words, the item within the container
 * has the same visibility as the container itself.  Mounts make use of this
 * to expose the rider whenever the mount is exposed.
 *
 * Derived container class note: derived containers should chain up to
 * this function first.  If it returns true, you're done because that
 * implies the container blanket-exposes all contents.  If the function
 * returns false, then do any container-specific checks that might
 * allow this particular object to be exposed.  Slotted containers work
 * this way to allow slot-by-slot specification of observation 
 * characteristics.
 *
 * @param item  the item that is contained or will soon be contained by
 *              the container.
 *
 * @return  true if the item should be exposed with the same observation
 *          and world characterstics as the container.
 */

bool Container::isContentItemObservedWith(Object const &item) const
{
	UNREF(item);
	return false;
}

//-----------------------------------------------------------------------
/**
 * Returns true if the item should be in the world if its container is
 * in the world.
 *
 * This function can be called on an item in the container or an item
 * that the caller already knows it will be putting in the container.
 *
 * If this function returns true, it indicates that the contained item
 * should be added to the world whenever the container is added to the world.
 *
 * Derived container class note: derived containers should chain up to
 * this function first.  If it returns true, you're done because that
 * implies the container blanket-exposes all contents.  If the function
 * returns false, then do any container-specific checks that might
 * allow this particular object to be exposed.  Slotted containers work
 * this way to allow slot-by-slot specification of exposing
 * characteristics.
 *
 * @param item  the item that is contained or will soon be contained by
 *              the container.
 *
 * @return  true if the item should be exposed with the same observation
 *          and world characterstics as the container.
 */

bool Container::isContentItemExposedWith(Object const &item) const
{
	UNREF(item);
	return false;
}

//-----------------------------------------------------------------------

bool Container::canContentsBeObservedWith() const
{
	return false;
}

//-----------------------------------------------------------------------

void Container::alter(real time)
{
	FATAL(!Object::shouldObjectsAlterChildrenAndContents(), ("Container::alter called when containers should not be altering!"));

	std::vector<Object *> objectsToDelete;
	for (Contents::const_iterator i = m_contents.begin(); i != m_contents.end(); ++i)
	{
		Object *obj = (*i).getObject();
		if (obj)
		{
			if (obj->isInitialized() && !isContentItemExposedWith(*obj) && !obj->isChildObject())
			{
				AlterScheduler::setMostRecentAlterTime(*obj);
				if (obj->alter(time) == AlterResult::cms_kill) //lint !e777 // testing floats for equality // it's okay, set/compare against constants.
				{
					objectsToDelete.push_back(obj);
				}
			}
		}
	}
	for (std::vector<Object *>::iterator j = objectsToDelete.begin(); j != objectsToDelete.end(); ++j)
	{
		Object *const objectToKill = *j;
		ContainerErrorCode error = CEC_Success;
		IGNORE_RETURN(remove (*objectToKill, error));
		delete objectToKill;
	}
	objectsToDelete.clear();
}

//-----------------------------------------------------------------------

void Container::conclude()
{
}

//-----------------------------------------------------------------------


bool Container::isItemContained  (const NetworkId& item, ContainerErrorCode& error) const
{
	return (Container::find(item, error) != -1);
}

//-----------------------------------------------------------------------

bool Container::remove (Object &item, ContainerErrorCode& error)
{
	error = CEC_Unknown;
	bool retval = false;
	ContainedByProperty* property = item.getContainedByProperty();
	WARNING_STRICT_FATAL(!property, ("Cannot remove an item that has no containedby property"));
	if (property)
	{
		if (property->getContainedBy() != &getOwner())
		{
			WARNING_STRICT_FATAL(true, ("Cannot remove an item [%s] from container [%s] whose containedBy says it isn't in this container.", item.getNetworkId().getValueString().c_str(), getOwner().getNetworkId().getValueString().c_str()));
			error = CEC_NotFound;
			return false;
		}
		retval = internalRemoveItem(item);
		if (retval)
		{
			error = CEC_Success;
			property->setContainedBy(NetworkId::cms_invalid);
		}
	}
	return retval;
}

//-----------------------------------------------------------------------

const Container::ContainedItem& Container::getContents (int position) const
{
	DEBUG_FATAL(position > static_cast<int>(m_contents.size()), ("Tried to index a container larger than the number of contents."));
	return m_contents[static_cast<Contents::size_type>(position)];
}

//-----------------------------------------------------------------------

void Container::debugPrint(std::string &buffer) const
{
	char tempBuffer[1024];

	buffer += "====[BEGIN: container]====\n";

		//-- List supported slots and container indices.
		sprintf(tempBuffer, "container id [%s]: contains [%d] objects.\n", getOwner().getNetworkId().getValueString().c_str(), static_cast<int>(m_contents.size()));
		buffer += tempBuffer;

		int index = 0;
		Contents::const_iterator const endIt = m_contents.end();
		for (Contents::const_iterator it = m_contents.begin(); it != endIt; ++it, ++index)
		{
			Object const *const object = it->getObject();

			sprintf(tempBuffer, "\tindex [%d]: object id [%s], template [%s]\n", index, it->getValueString().c_str(), object ? object->getObjectTemplateName() : "<getObject() reported nullptr object>");
			buffer += tempBuffer;
		}

	buffer += "====[END:   container]====\n";
}

// ----------------------------------------------------------------------

void Container::debugLog() const
{
#ifdef _DEBUG
	DEBUG_REPORT_LOG(true, ("====[BEGIN: container]====\n"));

		//-- List supported slots and container indices.
		DEBUG_REPORT_LOG(true, ("container id [%s]: contains [%d] objects.\n", getOwner().getNetworkId().getValueString().c_str(), static_cast<int>(m_contents.size())));

		int index = 0;
		Contents::const_iterator const endIt = m_contents.end();
		for (Contents::const_iterator it = m_contents.begin(); it != endIt; ++it, ++index)
		{
			Object const *const object = it->getObject();
			DEBUG_REPORT_LOG(true, ("\tindex [%d]: object id [%s], template [%s]\n", index, it->getValueString().c_str(), object ? object->getObjectTemplateName() : "<getObject() reported nullptr object>"));
		}

	DEBUG_REPORT_LOG(true, ("====[END:   container]====\n"));
#endif
}

//-----------------------------------------------------------------------
bool Container::internalRemoveItem(const Object & item)
{

	Contents::iterator i = m_contents.begin();
	for(; i != m_contents.end(); ++i)
	{
		if ((*i) == item.getNetworkId())
		{
			(*i) = NetworkId::cms_invalid;
			--m_numContents;
			return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------

/**
 * Remove an item, identified by an iterator.
 * Note that this does not invalidate iterators into the container.
 */
bool Container::remove(ContainerIterator &pos, ContainerErrorCode& error)
{
	error = CEC_Success;
	if (*(pos.m_iterator) != NetworkId::cms_invalid)
	{
		Object *obj = (*(pos.m_iterator)).getObject();
		ContainedByProperty* property = 0;
		if (obj)
			property = obj->getContainedByProperty();
		WARNING_STRICT_FATAL(!property, ("Cannot remove an item that has no containedby property"));
		if (property)
		{
			if (property->getContainedBy() != &getOwner())
			{
				WARNING_STRICT_FATAL(true, ("Cannot remove an item [%s] from container [%s] whose containedBy says it isn't in this container.", (*pos).getValueString().c_str(), getOwner().getNetworkId().getValueString().c_str()));
				error = CEC_NotFound;
				return false;
			}
			property->setContainedBy(NetworkId::cms_invalid);
		}
		
		(*pos.m_iterator) = NetworkId::cms_invalid;
		--m_numContents;
		return true;	
	}
	else
	{
		error = CEC_Unknown;
		return false;
	}
}

//------------------------------------------------------------------------------------------

bool Container::mayAdd (const Object& item, ContainerErrorCode& error) const
{
	error = CEC_Success;
	if (item.getNetworkId() == getOwner().getNetworkId())
	{
		error = CEC_AddSelf;
		return false;
	}

	if (ConfigSharedObject::getContainerLoopChecking())
	{
		//Check that we don't introduce an infinite loop
		//First check for container property. Only containers can introduce a loop
		const Container* contCheck = item.getContainerProperty();
		if (contCheck)
		{
			const ContainedByProperty * cbIter = getOwner().getContainedByProperty();
			if (cbIter)
			{
				NetworkId iterId = cbIter->getContainedByNetworkId();
				if (iterId != NetworkId::cms_invalid)
				{
					std::vector<NetworkId> checkVector;
					checkVector.reserve(10);
					checkVector.push_back(getOwner().getNetworkId());
					for( int count = 0; iterId != NetworkId::cms_invalid; ++count)
					{
						if (count > ConfigSharedObject::getContainerMaxDepth())
						{
							WARNING(true, ("Too deep a container heirarchy"));
							error = CEC_TooDeep;
							return false;
						}
						checkVector.push_back(iterId);
						const Object* obj = cbIter->getContainedBy();
						iterId = NetworkId::cms_invalid;
						if (obj)
						{
							cbIter = obj->getContainedByProperty();
							if (cbIter)
							{
								iterId = cbIter->getContainedByNetworkId();
							}
						}
						
						
					}
					if (std::find(checkVector.begin(), checkVector.end(), item.getNetworkId()) != checkVector.end())
					{
						//detected a loop.
						error = CEC_AlreadyIn;
						return false;
						
					}
				}
			}
			
		}
	}

	if (checkDepth(*this) > ConfigSharedObject::getContainerMaxDepth())
	{
		error = CEC_TooDeep;
		return false;
	}

	if (item.getNetworkId() == getOwner().getNetworkId())
	{
		error = CEC_AddSelf;
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------
//DO NOT CALL DIRECTLY
int Container::depersistContents(const Object& item)
{
	CachedNetworkId id(item.getNetworkId());
	return insertNewItem(id);
}


//-----------------------------------------------------------------------
//DO NOT CALL DIRECTLY
bool Container::internalItemRemoved(const Object& item)
{
	return internalRemoveItem(item);
}

//-----------------------------------------------------------------------
