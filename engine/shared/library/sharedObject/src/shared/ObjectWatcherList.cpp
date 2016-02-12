// ======================================================================
//
// ObjectWatcherList.cpp
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/ObjectWatcherList.h"

#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/Object.h"
#include "sharedFoundation/Watcher.h"

#include <vector>

// ======================================================================

namespace ObjectWatcherListNamespace
{
	void setRegionOfInfluenceEnabled(Object const * const object, bool const enabled, bool skipCell)
	{
		if (skipCell && nullptr != object->getCellProperty())
			return;

		object->setRegionOfInfluenceEnabled(enabled);

		for (int i = 0; i < object->getNumberOfChildObjects(); ++i)
			setRegionOfInfluenceEnabled(object->getChildObject(i), enabled, skipCell);
	}
}

// ======================================================================
// Construct an ObjectWatcherList

ObjectWatcherList::ObjectWatcherList (int initialVectorSize) :
m_altering              (false),
m_objectVector          (new ObjectVector),
m_alterSafeObjectVector (new ObjectVector),
m_skipCellRegionOfInfluence(false)
{
	DEBUG_FATAL(initialVectorSize < 0, ("negative vector size"));

	// Reserve the requested amount of entries to reduce further allocation

	m_objectVector->reserve          (static_cast<size_t> (initialVectorSize));
	m_alterSafeObjectVector->reserve (static_cast<size_t> (initialVectorSize));
}

// ----------------------------------------------------------------------
/**
 * Destroy an ObjectWatcherList.
 */

ObjectWatcherList::~ObjectWatcherList(void)
{
	delete m_objectVector;
	m_objectVector = 0;
	delete m_alterSafeObjectVector;
	m_alterSafeObjectVector = 0;
}

// ----------------------------------------------------------------------
/**
 * Add an Object to the ObjectWatcherList.
 * 
 * This routine will call Fatal in debug compiles if it is passed a nullptr
 * object.
 * 
 * This routine will call Fatal in debug compiles if the object list
 * overflows.
 * 
 * @param objectToAdd  Object to add to the ObjectWatcherList
 */

void ObjectWatcherList::addObject(Object & objectToAdd)
{
	m_objectVector->push_back(Watcher<Object>(&objectToAdd));
}

// ----------------------------------------------------------------------
/**
 * Remove an Object from the ObjectWatcherList.
 * 
 * This routine will call Fatal in debug compiles if it is passed a nullptr
 * object.
 * 
 * @param objectToRemove  Pointer to the object to remove
 */

void ObjectWatcherList::removeObject(const Object & objectToRemove)
{
	int index = 0;
	if (find(objectToRemove, &index))
		removeObjectByIndex(objectToRemove, index);
}

// ----------------------------------------------------------------------

void ObjectWatcherList::removeObjectByIndex (const Object & object, int index)
{
	DEBUG_FATAL(index >= static_cast<int>(m_objectVector->size()), ("Index out of range"));
	DEBUG_FATAL((*m_objectVector)[static_cast<ObjectVector::size_type>(index)] != &object, ("Object is not the one indexed."));

	// Replace the found entry with the entry from the back of the vector
	// so there is no shifting of the items in the vector.
	(*m_objectVector)[static_cast<ObjectVector::size_type>(index)] = m_objectVector->back();

	// Remove the last item in the vector.
	m_objectVector->pop_back();

	// nullptr the object from the alter-safe object list.
	uint size = m_alterSafeObjectVector->size();
	for (uint i = 0; i < size; ++i)
	{
		if ((*m_alterSafeObjectVector)[i] == &object)
		{
			(*m_alterSafeObjectVector)[i] = nullptr;
			return;
		}
	}
}


// ----------------------------------------------------------------------
/**
 * Remove all Objects from the ObjectWatcherList.
 * 
 * Removes all Objects from the ObjectWatcherList
 * 
 * This routine may not be called between prepareToAlter() and alter().
 * 
 * If deleteObjects is true, this routine will call delete for all objects in the list
 * 
 * @param deleteObjects  Flag to delete all objects when they are removed
 */

void ObjectWatcherList::removeAll (bool deleteObjects)
{
	DEBUG_FATAL(m_altering, ("m_altering"));

	if (deleteObjects)
	{
		ObjectVector::iterator end = m_objectVector->end();
		for (ObjectVector::iterator i = m_objectVector->begin(); i != end; ++i)
			delete i->getPointer();
	}

	m_objectVector->clear();
}

// ----------------------------------------------------------------------

void ObjectWatcherList::addToWorld()
{
	ObjectVector::iterator iEnd = m_objectVector->end();
	for (ObjectVector::iterator i = m_objectVector->begin(); i != iEnd; ++i)
	{
		Object * const object = (*i);
		if (object)
			object->addToWorld();
	}
}

// ----------------------------------------------------------------------

void ObjectWatcherList::removeFromWorld()
{
	ObjectVector::iterator iEnd = m_objectVector->end();
	for (ObjectVector::iterator i = m_objectVector->begin(); i != iEnd; ++i)
	{
		Object * const object = (*i);
		if (object)
			object->removeFromWorld();
	}
}

// ----------------------------------------------------------------------
/**
 * Prepare to call alter() on this object list.
 * 
 * This routine must be called before calling alter().  It will copy setup
 * the ObjectWatcherList so that objects may be added and removed from the ObjectWatcherList
 * during the alter() without affecting that frame's list of objects that are
 * are being altered.
 */

void ObjectWatcherList::prepareToAlter(void)
{
	DEBUG_FATAL(m_altering, ("m_altering"));

	m_altering = true;

	// Duplicate the object vector

	(*m_alterSafeObjectVector) = (*m_objectVector);
}

// ----------------------------------------------------------------------
/**
 * Call alter() for all objects in the ObjectWatcherList.
 * 
 * prepareToAlter() must be called before this routine can be called.
 * 
 * This routine will simply iterate over all the objects in the ObjectWatcherList
 * and call their alter routines with the specified time.
 * 
 * @param time  Time to pass to each Object's alter routine
 * @see ObjectWatcherList::prepareToAlter()
 */

void ObjectWatcherList::alter(real time)
{
	DEBUG_FATAL(!m_altering, ("prepareToAlter was not called"));

	for (size_t i = 0; i < m_alterSafeObjectVector->size(); ++i)
	{
		// Cache the object pointer because the iter value may be nulled if the object,
		// is removed from the list withing its alter()
	
		Object * const obj = (*m_alterSafeObjectVector)[i];
		if (!obj || !obj->isInitialized())
			continue;

		float const alterResult = obj->alter(time);
		if (alterResult == AlterResult::cms_kill)
		{
			// alter() return false, so remove and delete the object
	
			// Make sure the object is still in the object list before removing it
			
			if ((*m_alterSafeObjectVector)[i] != nullptr)
			{
				removeObject(*obj);
			}
	
			delete obj;
		}
	}
	
	m_alterSafeObjectVector->clear();
	m_altering = false;
}

// ----------------------------------------------------------------------
/**
 * Call conclude() for all objects in the ObjectWatcherList.
 * 
 * This routine may not be called between prepareToAlter() and alter().
 * 
 * This routine will simply iterate over all the objects in the ObjectWatcherList
 * and call their conclude routines with the specified time.
 */

void ObjectWatcherList::conclude()
{
	DEBUG_FATAL(m_altering, ("m_altering"));

	ObjectVector::iterator end = m_objectVector->end();

	for (ObjectVector::iterator i = m_objectVector->begin(); i != end; ++i)
	{
		Object * const object = (*i);
		if (object && object->isInitialized())
			object->conclude();
	}
}

// ----------------------------------------------------------------------
/**
 * Call addToScene() for all objects in the ObjectWatcherList.
 * 
 * This routine may not be called between prepareToAlter() and alter().
 * 
 * This routine will simply iterate over all the objects in the ObjectWatcherList
 * and call their addToScene() routine with the specified camera.
 * 
 * @param camera  Camera rendering the scene.
 * @param excludedObject Object to exclude from the scene.
 */

void ObjectWatcherList::setRegionOfInfluenceEnabled(bool enabled) const
{
	ObjectVector::iterator end = m_objectVector->end();
	for (ObjectVector::iterator i = m_objectVector->begin(); i != end; ++i)
	{
		Object const * const object = (*i);
		if (object)
			ObjectWatcherListNamespace::setRegionOfInfluenceEnabled(object, enabled, m_skipCellRegionOfInfluence);
	}
}

// ----------------------------------------------------------------------
/**
 * Call addToScene() for all objects in the ObjectWatcherList.
 * 
 * This routine may not be called between prepareToAlter() and alter().
 * 
 * This routine will simply iterate over all the objects in the ObjectWatcherList
 * and call their addToScene() routine with the specified camera.
 * 
 * @param camera  Camera rendering the scene.
 * @param excludedObject Object to exclude from the scene.
 */

void ObjectWatcherList::render(const Object *excludedObject) const
{
	DEBUG_FATAL(m_altering, ("m_altering"));
	
	const ObjectVector::iterator end = m_objectVector->end();
		
	for (ObjectVector::iterator i = m_objectVector->begin(); i != end; ++i)
	{
		Object *const object = *i;
		if (object && (object != excludedObject))
		{
			const Appearance *const appearance = object->getAppearance();
			if (appearance)
			{
				appearance->setTransform_w(object->getTransform_o2w());
				appearance->objectListCameraRender();
			}
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Find the index of an object in an object list.
 * 
 * Accepts an optional index to be used to return the object's index
 * in the object list.
 * 
 * This routine will simply iterate over all the objects in the ObjectWatcherList
 * searching for the objectToCheck, and returns true with the index if
 * found, and false otherwise.
 * 
 * @param objectToCheck  Object to find
 * @param index  Object's index in the object list
 */

bool ObjectWatcherList::find(const Object & objectToCheck, int *index) const
{
	ObjectVector::iterator end = m_objectVector->end();

	for (ObjectVector::iterator i = m_objectVector->begin(); i != end; ++i)
	{
		const Object * const obj = (*i);
	
		if (obj == &objectToCheck)
		{
			if (index)
				*index = std::distance(m_objectVector->begin(), i);
			return true;
		}
	
	}
	
	return false;
}

//----------------------------------------------------------------------

int ObjectWatcherList::removeNulls ()
{
	int count = 0;
	for (ObjectVector::iterator i = m_objectVector->begin(); i != m_objectVector->end();)
	{
		const Object * const obj = (*i);
	
		if (obj)
			++i;
		else
		{
			++count;
			i = m_objectVector->erase (i);
		}	
	}
	
	return count;
}

// ----------------------------------------------------------------------
/**
 * Return the number of objects contained in the object list.
 * 
 * @return The number of objects contained in the object list
 */

int ObjectWatcherList::getNumberOfObjects(void) const
{
	return static_cast<int> (m_objectVector->size());
}

// ----------------------------------------------------------------------
/**
 * Get an object in the object list.
 * 
 * @param index  Index of the object in the object list
 */

Object *ObjectWatcherList::operator [](int index)
{
	DEBUG_FATAL(index < 0 || index >= getNumberOfObjects(), ("index out of range %d/%d", index, getNumberOfObjects()));
	return (*m_objectVector)[static_cast<unsigned int> (index)];
}

// ----------------------------------------------------------------------
/**
 * Get a const object from the object list.
 * 
 * @param index  Index of the object in the object list
 */

const Object *ObjectWatcherList::operator [](int index) const
{
	DEBUG_FATAL(index < 0 || index >= getNumberOfObjects(), ("index out of range %d/%d", index, getNumberOfObjects()));
	return (*m_objectVector)[static_cast<unsigned int> (index)];
}

// ----------------------------------------------------------------------
/**
 * Get an object in the object list.
 * 
 * @param index  Index of the object in the object list
 */

Object *ObjectWatcherList::getObject(int index)
{
	DEBUG_FATAL(index < 0 || index >= getNumberOfObjects(), ("index out of range %d/%d", index, getNumberOfObjects()));
	return (*m_objectVector)[static_cast<unsigned int> (index)];
}

// ----------------------------------------------------------------------
/**
 * Get a const object from the object list.
 * 
 * @param index  Index of the object in the object list
 */

const Object *ObjectWatcherList::getObject(int index) const
{
	DEBUG_FATAL(index < 0 || index >= getNumberOfObjects(), ("index out of range %d/%d", index, getNumberOfObjects()));
	return (*m_objectVector)[static_cast<unsigned int> (index)];
}

//----------------------------------------------------------------------

void ObjectWatcherList::setSkipCellRegionOfInfluence(bool skip)
{
	m_skipCellRegionOfInfluence = skip;
}

// ======================================================================
