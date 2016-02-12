// ======================================================================
//
// ObjectList.cpp
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/ObjectList.h"

#include "sharedObject/AlterResult.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/Object.h"

#include <vector>

// ======================================================================

namespace ObjectListNamespace
{
	void setRegionOfInfluenceEnabled(const Object *const object, const bool enabled)
	{
		object->setRegionOfInfluenceEnabled(enabled);

		for (int i = 0; i < object->getNumberOfChildObjects(); ++i)
			setRegionOfInfluenceEnabled(object->getChildObject(i), enabled);
	}
}

// ======================================================================
// Construct an ObjectList

ObjectList::ObjectList(
	int   initialVectorSize  // The initial entries set aside for the object list
	)
:
	m_altering(false),
	m_objectVector(new ObjectVector()),
	m_alterSafeObjectVector(new ObjectVector())
{
	DEBUG_FATAL(initialVectorSize < 0, ("negative vector size"));

	// Reserve the requested amount of entries to reduce further allocation

	m_objectVector->reserve(static_cast<unsigned int> (initialVectorSize));
	m_alterSafeObjectVector->reserve(static_cast<unsigned int> (initialVectorSize));
}

// ----------------------------------------------------------------------
/**
 * Destroy an ObjectList.
 */

ObjectList::~ObjectList()
{
	delete m_objectVector;
	delete m_alterSafeObjectVector;
}

// ----------------------------------------------------------------------
/**
 * Add an Object to the ObjectList.
 * 
 * This routine will call Fatal in debug compiles if it is passed a nullptr
 * object.
 * 
 * This routine will call Fatal in debug compiles if the object list
 * overflows.
 * 
 * @param objectToAdd  Object to add to the ObjectList
 */

void ObjectList::addObject(Object *objectToAdd)
{
	NOT_NULL(objectToAdd);

	// Add the new object

	m_objectVector->push_back(objectToAdd);
}

// ----------------------------------------------------------------------
/**
 * Remove an Object from the ObjectList.
 * 
 * This routine will call Fatal in debug compiles if it is passed a nullptr
 * object.
 * 
 * @param objectToRemove  Pointer to the object to remove
 */

void ObjectList::removeObject(const Object *objectToRemove)
{
	int index = 0;
	if (find(objectToRemove, &index))
		removeObjectByIndex(objectToRemove, index);
}

// ----------------------------------------------------------------------

void ObjectList::removeObjectByIndex(const Object* object, int index)
{
	NOT_NULL(object);
	DEBUG_FATAL(index >= static_cast<int>(m_objectVector->size()), ("Index out of range"));
	DEBUG_FATAL((*m_objectVector)[static_cast<ObjectVector::size_type>(index)] != object, ("Object is not the one indexed."));

	// Replace the found entry with the entry from the back of the vector
	// so there is no shifting of the items in the vector.
	(*m_objectVector)[static_cast<ObjectVector::size_type>(index)] = m_objectVector->back();

	// Remove the last item in the vector.
	m_objectVector->pop_back();

	// nullptr the object from the alter-safe object list.
	uint size = m_alterSafeObjectVector->size();
	for (uint i = 0; i < size; ++i)
	{
		if ((*m_alterSafeObjectVector)[i] == object)
		{
			(*m_alterSafeObjectVector)[i] = nullptr;
			return;
		}
	}
}


// ----------------------------------------------------------------------
/**
 * Remove all Objects from the ObjectList.
 * 
 * Removes all Objects from the ObjectList
 * 
 * This routine may not be called between prepareToAlter() and alter().
 * 
 * If deleteObjects is true, this routine will call delete for all objects in the list
 * 
 * @param deleteObjects  Flag to delete all objects when they are removed
 */

void ObjectList::removeAll (bool deleteObjects)
{
	DEBUG_FATAL(m_altering, ("m_altering"));

	if (deleteObjects)
	{
		ObjectVector::iterator end = m_objectVector->end();
		for (ObjectVector::iterator i = m_objectVector->begin(); i != end; ++i)
			delete (*i);
	}

	m_objectVector->clear();
}

// ----------------------------------------------------------------------

void ObjectList::addToWorld()
{
	ObjectVector::iterator iEnd = m_objectVector->end();
	for (ObjectVector::iterator i = m_objectVector->begin(); i != iEnd; ++i)
		(*i)->addToWorld();
}

// ----------------------------------------------------------------------

void ObjectList::removeFromWorld()
{
	ObjectVector::iterator iEnd = m_objectVector->end();
	for (ObjectVector::iterator i = m_objectVector->begin(); i != iEnd; ++i)
		(*i)->removeFromWorld();
}

// ----------------------------------------------------------------------
/**
 * Prepare to call alter() on this object list.
 * 
 * This routine must be called before calling alter().  It will copy setup
 * the ObjectList so that objects may be added and removed from the ObjectList
 * during the alter() without affecting that frame's list of objects that are
 * are being altered.
 */

void ObjectList::prepareToAlter()
{
	DEBUG_FATAL(m_altering, ("m_altering"));

	m_altering = true;

	// Duplicate the object vector

	(*m_alterSafeObjectVector) = (*m_objectVector);
}

// ----------------------------------------------------------------------
/**
 * Call alter() for all objects in the ObjectList.
 * 
 * prepareToAlter() must be called before this routine can be called.
 * 
 * This routine will simply iterate over all the objects in the ObjectList
 * and call their alter routines with the specified time.
 * 
 * @param time  Time to pass to each Object's alter routine
 * @see ObjectList::prepareToAlter()
 */

float ObjectList::alter(real time)
{
	DEBUG_FATAL(!m_altering, ("prepareToAlter was not called"));

	float finalAlterResult = AlterResult::cms_keepNoAlter;

	for (uint32 i = 0; i < m_alterSafeObjectVector->size(); ++i)
	{
		// Cache the object pointer because the iter value may be nulled if the object is removed from the list withing its alter()
		Object *obj = (*m_alterSafeObjectVector)[i];
		if (!obj)
			continue;

		if (obj->isInitialized())
		{
			float const alterResult = obj->alter(time);
			if (alterResult == AlterResult::cms_kill) //lint !e777 // Testing floats for equality // It's okay, we're using constants.
			{
				// Make sure the object is still in the object list before removing it
				if ((*m_alterSafeObjectVector)[i] != nullptr)
					removeObject(obj);
				delete obj;
			}
			else
				AlterResult::incorporateAlterResult(finalAlterResult, alterResult);
		}
	}
	
	m_alterSafeObjectVector->clear();
	m_altering = false;

	return finalAlterResult;
}

// ----------------------------------------------------------------------
/**
 * Call conclude() for all objects in the ObjectList.
 * 
 * This routine may not be called between prepareToAlter() and alter().
 * 
 * This routine will simply iterate over all the objects in the ObjectList
 * and call their conclude routines with the specified time.
 */

void ObjectList::conclude()
{
	DEBUG_FATAL(m_altering, ("m_altering"));

	ObjectVector::iterator end = m_objectVector->end();

	for (ObjectVector::iterator i = m_objectVector->begin(); i != end; ++i)
	{
		(*i)->conclude();
	}
}

// ----------------------------------------------------------------------
/**
 * Call addToScene() for all objects in the ObjectList.
 * 
 * This routine may not be called between prepareToAlter() and alter().
 * 
 * This routine will simply iterate over all the objects in the ObjectList
 * and call their addToScene() routine with the specified camera.
 * 
 * @param camera  Camera rendering the scene.
 * @param excludedObject Object to exclude from the scene.
 */

void ObjectList::setRegionOfInfluenceEnabled(bool enabled) const
{
	ObjectVector::iterator end = m_objectVector->end();

	for (ObjectVector::iterator i = m_objectVector->begin(); i != end; ++i)
		ObjectListNamespace::setRegionOfInfluenceEnabled(*i, enabled);
}

// ----------------------------------------------------------------------
/**
 * Call addToScene() for all objects in the ObjectList.
 * 
 * This routine may not be called between prepareToAlter() and alter().
 * 
 * This routine will simply iterate over all the objects in the ObjectList
 * and call their addToScene() routine with the specified camera.
 * 
 * @param camera  Camera rendering the scene.
 * @param excludedObject Object to exclude from the scene.
 */

void ObjectList::render(const Object *excludedObject) const
{
	DEBUG_FATAL(m_altering, ("m_altering"));

	ObjectVector::iterator end = m_objectVector->end();

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
 * This routine will simply iterate over all the objects in the ObjectList
 * searching for the objectToCheck, and returns true with the index if
 * found, and false otherwise.
 * 
 * @param objectToCheck  Object to find
 * @param index  Object's index in the object list
 */

bool ObjectList::find(const Object *objectToCheck, int *index) const
{
	ObjectVector::iterator end = m_objectVector->end();

	for (ObjectVector::iterator i = m_objectVector->begin(); i != end; ++i)
	{
		Object *obj = (*i);
	
		if (obj == objectToCheck)
		{
			if (index)
				*index = std::distance(m_objectVector->begin(), i);
			return true;
		}
	
	}
	
	return false;
}

// ----------------------------------------------------------------------

bool ObjectList::isEmpty() const
{
	return m_objectVector->empty();
}

// ----------------------------------------------------------------------
/**
 * Return the number of objects contained in the object list.
 * 
 * @return The number of objects contained in the object list
 */

int ObjectList::getNumberOfObjects() const
{
	return static_cast<int> (m_objectVector->size());
}

// ----------------------------------------------------------------------
/**
 * Get an object in the object list.
 * 
 * @param index  Index of the object in the object list
 */

Object *ObjectList::operator [](int index)
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

const Object *ObjectList::operator [](int index) const
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

Object *ObjectList::getObject(int index)
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

const Object *ObjectList::getObject(int index) const
{
	DEBUG_FATAL(index < 0 || index >= getNumberOfObjects(), ("index out of range %d/%d", index, getNumberOfObjects()));
	return (*m_objectVector)[static_cast<unsigned int> (index)];
}

// ======================================================================
