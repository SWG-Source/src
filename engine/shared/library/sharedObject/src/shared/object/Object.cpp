// ======================================================================
//
// Object.cpp
// copyright 1998 Bootprint Entertainment
// Copyright 2000-2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/Object.h"

#include "sharedCollision/CollisionProperty.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/DebugInfoManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Production.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/SpatialSubdivision.h"
#include "sharedMath/Sphere.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ConfigSharedObject.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/Controller.h"
#include "sharedObject/Dynamics.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectNotification.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/ScheduleData.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedObject/World.h"

#include <algorithm>
#include <limits>
#include <map>
#include <typeinfo>
#include <vector>

#define ENABLE_OBJECTTOWORLDDIRTY 1
#define PROFILE_GETTRANSFORM_O2W 0

// ======================================================================

class Object::NotificationList
{
public:

	NotificationList();
	NotificationList(const NotificationList &otherList, const ObjectNotification &otherEntry, bool addOtherEntry);

	bool equalsAdd(const NotificationList &otherList, const ObjectNotification &otherEntry) const;
	bool equalsRemove(const NotificationList &otherList, const ObjectNotification &otherEntry) const;
	bool contains(const ObjectNotification &otherEntry) const;

	void addToWorld(Object &object) const;
	void removeFromWorld(Object &object) const;

	void positionChanged(Object &object, bool dueToParentChange, const Vector &oldPosition) const;
	void rotationChanged(Object &object, bool dueToParentChange) const;
	void positionAndRotationChanged(Object &object, bool dueToParentChange, const Vector &oldPosition) const;

	void cellChanged(Object &object, bool dueToParentChange) const;
	void extentChanged(Object &object) const;
	void pobFixupComplete(Object &object) const;
	
protected:

	typedef std::vector<const ObjectNotification *> Notifications;
	Notifications m_notificationList;
};

// ======================================================================

Object::NotificationList::NotificationList()
: m_notificationList()
{
}

// ----------------------------------------------------------------------

Object::NotificationList::NotificationList(const NotificationList &otherList, const ObjectNotification &otherEntry, bool addOtherEntry)
: m_notificationList()
{
	if (addOtherEntry)
	{
		// handle adding the other entry

		m_notificationList.reserve(otherList.m_notificationList.size() + 1);

		bool addedOtherEntry = false;

		const int otherPriority = otherEntry.getPriority();

		// copy the other notification list, inserting the other entry in sorted position
		const Notifications::const_iterator end = otherList.m_notificationList.end();
		for (Notifications::const_iterator i = otherList.m_notificationList.begin(); i != end; ++i)
		{
			const ObjectNotification *add = *i;
			const int addPriority = add->getPriority();

			if (!addedOtherEntry && (otherPriority < addPriority || (otherPriority == addPriority && &otherEntry < add)))
			{
				m_notificationList.push_back(&otherEntry);
				addedOtherEntry = true;
			}

			m_notificationList.push_back(add);
		}

		// if the other entry didn't get copied already, it's the last element
		if (!addedOtherEntry)
			m_notificationList.push_back(&otherEntry);

		DEBUG_FATAL(m_notificationList.size() != otherList.m_notificationList.size() + 1, ("failed to build new notification list correctly"));
	}
	else
	{
		// handle creating a copy but deleting the other entry
		m_notificationList.reserve(otherList.m_notificationList.size() - 1);

		bool deletedOtherEntry = false;

		// copy the other notification list, deleting the other entry if found
		const Notifications::const_iterator end = otherList.m_notificationList.end();
		for (Notifications::const_iterator i = otherList.m_notificationList.begin(); i != end; ++i)
		{
			const ObjectNotification *add = *i;
			if (add == &otherEntry)
			{
				// skip it --- we are removing this notification
				deletedOtherEntry = true;
				continue;
			}

			// copy the source notification
			m_notificationList.push_back(add);
		}

		DEBUG_FATAL(!deletedOtherEntry, ("didn't find notification [0x%08x] in source list for deletion", &otherEntry));
		DEBUG_FATAL(m_notificationList.size() != otherList.m_notificationList.size() - 1, ("failed to build new notification list correctly"));

	}
}

// ----------------------------------------------------------------------

bool Object::NotificationList::equalsAdd(const NotificationList &otherList, const ObjectNotification &otherEntry) const
{
	return otherList.equalsRemove(*this, otherEntry);
}

// ----------------------------------------------------------------------

bool Object::NotificationList::equalsRemove(const NotificationList &otherList, const ObjectNotification &otherEntry) const
{
	const uint otherSize = otherList.m_notificationList.size();
	if (m_notificationList.size() + 1 != otherSize)
		return false;

	// compare against the other list and the other entry
	bool once = false;
	uint i = 0;
	uint j = 0;
	for ( ; i < otherSize; ++i)
	{
		if (otherList.m_notificationList[i] == &otherEntry)
		{
			// go on to next local element
			DEBUG_FATAL(once, ("found other entry twice"));
			once = true;
		}
		else
			if (j < otherSize-1 && otherList.m_notificationList[i] == m_notificationList[j])
				++j;
			else
				return false;
	}

	// make sure the item to remove was found
	if (!once)
		return false;

	return true;
}

// ----------------------------------------------------------------------

bool Object::NotificationList::contains(const ObjectNotification &otherEntry) const
{
	return std::find(m_notificationList.begin(), m_notificationList.end(), &otherEntry) != m_notificationList.end();
}

// ----------------------------------------------------------------------
/**
 * Return whether the object is initialized or not.
 *
 * Uninitialized objects should not be processed by the AlterScheduler
 * nor should their alter calls be made.
 */

bool Object::isInitialized() const
{
	//-- By default, all objects are initialized.  Derived classes can override
	//   this behavior and should not chain down to Object::isInitialized().
	return true;
}

// ----------------------------------------------------------------------

void Object::NotificationList::addToWorld(Object &object) const
{
	Notifications::const_iterator end = m_notificationList.end();
	for (Notifications::const_iterator i = m_notificationList.begin(); i != end; ++i)
		(*i)->addToWorld(object);
}

// ----------------------------------------------------------------------

void Object::NotificationList::removeFromWorld(Object &object) const
{
	Notifications::const_reverse_iterator end = m_notificationList.rend();
	for (Notifications::const_reverse_iterator i = m_notificationList.rbegin(); i != end; ++i)
		(*i)->removeFromWorld(object);
}

// ----------------------------------------------------------------------

void Object::NotificationList::positionChanged(Object &object, bool dueToParentChange, const Vector &oldPosition) const
{
	Notifications::const_iterator end = m_notificationList.end();
	for (Notifications::const_iterator i = m_notificationList.begin(); i != end; ++i)
		if (!(*i)->positionChanged(object, dueToParentChange, oldPosition))
			return;
}

// ----------------------------------------------------------------------

void Object::NotificationList::rotationChanged(Object &object, bool dueToParentChange) const
{
	Notifications::const_iterator end = m_notificationList.end();
	for (Notifications::const_iterator i = m_notificationList.begin(); i != end; ++i)
		(*i)->rotationChanged(object, dueToParentChange);
}

// ----------------------------------------------------------------------

void Object::NotificationList::positionAndRotationChanged(Object &object, bool dueToParentChange, const Vector &oldPosition) const
{
	Notifications::const_iterator end = m_notificationList.end();
	for (Notifications::const_iterator i = m_notificationList.begin(); i != end; ++i)
		if (!(*i)->positionAndRotationChanged(object, dueToParentChange, oldPosition))
			return;
}

// ----------------------------------------------------------------------

void Object::NotificationList::cellChanged(Object &object, bool dueToParentChange) const
{
	Notifications::const_iterator end = m_notificationList.end();
	for (Notifications::const_iterator i = m_notificationList.begin(); i != end; ++i)
		(*i)->cellChanged(object, dueToParentChange);
}

// ----------------------------------------------------------------------

void Object::NotificationList::extentChanged(Object &object) const
{
	Notifications::const_iterator end = m_notificationList.end();
	for (Notifications::const_iterator i = m_notificationList.begin(); i != end; ++i)
		(*i)->extentChanged(object);
}

// ----------------------------------------------------------------------

void Object::NotificationList::pobFixupComplete(Object &object) const
{
	Notifications::const_iterator end = m_notificationList.end();
	for (Notifications::const_iterator i = m_notificationList.begin(); i != end; ++i)
		(*i)->pobFixupComplete(object);
}

// ======================================================================

class Object::NotificationListManager
{
public:

	static void install();

	static const NotificationList *getEmptyNotificationList();
	static const NotificationList *addNotification(const NotificationList *notificationList, const ObjectNotification &notification);
	static const NotificationList *removeNotification(const NotificationList *notificationList, const ObjectNotification &notification);

public:

	typedef std::vector<const Object::NotificationList*> List;

private:

	static void remove();

private:

	static List                      ms_list;
	static Object::NotificationList  ms_empty;
};

// ======================================================================

Object::NotificationListManager::List  Object::NotificationListManager::ms_list;
Object::NotificationList               Object::NotificationListManager::ms_empty;

// ======================================================================

void Object::NotificationListManager::install()
{
	ms_list.push_back(&ms_empty);
	ExitChain::add(remove, "Object::NotificationListManager::remove");
}

// ----------------------------------------------------------------------

void Object::NotificationListManager::remove()
{
	// don't delete the first element, it wasn't allocated from the heap
	const uint count = ms_list.size();
	for (uint i = 1; i < count; ++i)
	{
		const NotificationList *notificationList = ms_list.back();
		ms_list.pop_back();
		delete notificationList; //lint !e605 // Increase in pointer capability
	}

	// remove the first element from the list
	ms_list.pop_back();
}

// ======================================================================

const Object::NotificationList *Object::NotificationListManager::getEmptyNotificationList()
{
	return &ms_empty;
}

// ----------------------------------------------------------------------

const Object::NotificationList *Object::NotificationListManager::addNotification(const NotificationList *source, const ObjectNotification &notification)
{
	const List::const_iterator end = ms_list.end();

	// search for an existing copy of the requested notification list
	for (List::const_iterator i = ms_list.begin(); i != end; ++i)
		if ((*i)->equalsAdd(*source, notification))
			return *i;

	// wasn't found, create one
	NotificationList *notificationList = new NotificationList(*source, notification, true);
	ms_list.push_back(notificationList);

	return notificationList;
}

// ----------------------------------------------------------------------

const Object::NotificationList *Object::NotificationListManager::removeNotification(const NotificationList *source, const ObjectNotification &notification)
{
	const List::const_iterator end = ms_list.end();

	// search for an existing copy of the requested notification list
	for (List::const_iterator i = ms_list.begin(); i != end; ++i)
		if ((*i)->equalsRemove(*source, notification))
			return *i;

	// wasn't found, create one
	NotificationList *notificationList = new NotificationList(*source, notification, false);
	ms_list.push_back(notificationList);

	return notificationList;
}

// ======================================================================

namespace ObjectNamespace
{
	typedef std::vector<Object::DpvsObjects *>       DpvsObjectsList;
	typedef std::map<PropertyId,int>                 PropertySearchStatistics;
	typedef std::vector<Property *>                  PropertyList;
	typedef std::vector<Object::PropertyList *>      PropertyListList;
	typedef std::vector<Object *>                 AttachedObjects;
	typedef std::vector<AttachedObjects *>        AttachedObjectsList;

	MemoryBlockManager                              *ms_transformMemoryBlockManager;
	DpvsObjectsList                                  ms_systemAllocatedDpvsObjectsList;
	int                                              ms_allocatedDpvsObjects;
	DpvsObjectsList                                  ms_freeDpvsObjectsList;
	PropertyListList                                 ms_propertyListList;
	AttachedObjectsList                              ms_attachedObjectsList;
	bool                                             ms_disallowObjectDelete;
	bool                                             ms_logObjectDelete;
	bool                                             ms_validateObjectPosition;
	bool                                             ms_objectsAlterChildrenAndContents;

	PropertySearchStatistics                         ms_propertySearchStatistics;
	int                                              ms_propertySearchesPerFrame;
	bool                                             ms_reportPropertySearchStatistics;

	Object::InWorldAddDpvsObjectHookFunction         ms_inWorldAddDpvsObjectHookFunction;
	Object::InWorldRemoveDpvsObjectHookFunction      ms_inWorldRemoveDpvsObjectHookFunction;
	Object::LeakedDpvsObjectHookFunction             ms_leakedDpvsObjectHookFunction;

	char                                             ms_crashReportInfo[MAX_PATH * 2];

	void                    remove();
	Transform              *newLocalTransform();
	void                    deleteLocalTransform(Transform *t);
	Object::DpvsObjects    *newDpvsObjects();
	void                    deleteDpvsObjects(Object::DpvsObjects *t);
	PropertyList *          newPropertyList();
	void                    deletePropertyList(PropertyList * propertyList);
	AttachedObjects *       newAttachedObjects();
	void                    deleteAttachedObjects(AttachedObjects * attachedObjects);
	void                    reportPropertySearchStatistics();
	void                    validatePosition(Object const & object, Vector const & position);
	AxialBox const          getTangibleExtentInternal(Object const &obj);

#if PROFILE_GETTRANSFORM_O2W == 1
	bool ms_debugReport;
	int ms_totalCalls;
	int ms_totalAttachedCalls;
	int ms_totalAttachedDirtyCalls;

	void debugReport();
#endif

	std::string const & ms_debugInfoSectionName = "Object";
}
using namespace ObjectNamespace;

// ======================================================================

void Object::install(bool objectsAlterChildrenAndContents)
{
	NotificationListManager::install();
	ms_transformMemoryBlockManager = new MemoryBlockManager("Object::ms_transformMemoryBlockManager", true, sizeof(Transform), 0, 0, 0);
	ms_objectsAlterChildrenAndContents = objectsAlterChildrenAndContents;
	ExitChain::add(&ObjectNamespace::remove, "Object::remove");
	DebugFlags::registerFlag(ms_objectsAlterChildrenAndContents, "SharedObject", "objectsAlterChildrenAndContents");
	DebugFlags::registerFlag(ms_reportPropertySearchStatistics, "SharedObject", "reportPropertySearchStatistics", reportPropertySearchStatistics);
	DebugFlags::registerFlag(ms_logObjectDelete, "SharedObject", "logObjectDelete");
	DebugFlags::registerFlag(ms_validateObjectPosition, "SharedObject", "validateObjectPosition");
#if PROFILE_GETTRANSFORM_O2W == 1
	DebugFlags::registerFlag(ms_debugReport, "SharedObject/Object", "debugReport", ObjectNamespace::debugReport);
#endif

	ms_crashReportInfo[0] = '\0';
	CrashReportInformation::addDynamicText(ms_crashReportInfo);
}

// ----------------------------------------------------------------------

bool Object::shouldObjectsAlterChildrenAndContents()
{
	return ms_objectsAlterChildrenAndContents;
}

// ----------------------------------------------------------------------

void ObjectNamespace::reportPropertySearchStatistics()
{
	DEBUG_REPORT_PRINT(true, ("Object property searches (%d):\n", ms_propertySearchesPerFrame));
	ms_propertySearchesPerFrame = 0;
	PropertySearchStatistics ::const_iterator const iEnd = ms_propertySearchStatistics.end();
	for (PropertySearchStatistics::const_iterator i = ms_propertySearchStatistics.begin(); i != iEnd; ++i)
		DEBUG_REPORT_PRINT(true, ("  %08lx %d\n", i->first, i->second));
}

// ----------------------------------------------------------------------

#if PROFILE_GETTRANSFORM_O2W == 1

void ObjectNamespace::debugReport()
{
	DEBUG_REPORT_PRINT(true, ("totalCalls=%d\n", ms_totalCalls));
	DEBUG_REPORT_PRINT(true, ("totalAttachedCalls=%d\n", ms_totalAttachedCalls));
	DEBUG_REPORT_PRINT(true, ("totalAttachedDirtyCalls=%d\n", ms_totalAttachedDirtyCalls));
}

#endif

// ----------------------------------------------------------------------

void ObjectNamespace::remove()
{
	CrashReportInformation::removeDynamicText(ms_crashReportInfo);

	DebugFlags::unregisterFlag(ms_objectsAlterChildrenAndContents);
	DebugFlags::unregisterFlag(ms_reportPropertySearchStatistics);
	DebugFlags::unregisterFlag(ms_logObjectDelete);
	DebugFlags::unregisterFlag(ms_validateObjectPosition);
#if PROFILE_GETTRANSFORM_O2W == 1
	DebugFlags::unregisterFlag(ms_debugReport);
#endif

	delete ms_transformMemoryBlockManager;
	ms_transformMemoryBlockManager = nullptr;

	DEBUG_WARNING(static_cast<int>(ms_freeDpvsObjectsList.size()) != ms_allocatedDpvsObjects, ("Leaked %d DpvsObjects lists", ms_allocatedDpvsObjects - static_cast<int>(ms_freeDpvsObjectsList.size())));
	while (!ms_systemAllocatedDpvsObjectsList.empty())
	{
		delete [] ms_systemAllocatedDpvsObjectsList.back();
		ms_systemAllocatedDpvsObjectsList.pop_back();
	}

	while (!ms_propertyListList.empty())
	{
		delete ms_propertyListList.back();
		ms_propertyListList.pop_back();
	}

	while (!ms_attachedObjectsList.empty())
	{
		delete ms_attachedObjectsList.back();
		ms_attachedObjectsList.pop_back();
	}
}

// ----------------------------------------------------------------------

Transform *ObjectNamespace::newLocalTransform()
{
	void *memory = ms_transformMemoryBlockManager->allocate();
	return new(memory) Transform;
}

// ----------------------------------------------------------------------

void ObjectNamespace::deleteLocalTransform(Transform *transform)
{
	void *const transformMemory = transform;
	transform->~Transform();

	ms_transformMemoryBlockManager->free(transformMemory);
}

// ----------------------------------------------------------------------

void ObjectNamespace::validatePosition(Object const & object, Vector const & position)
{
	if (std::abs(position.x) > 16000.0f || std::abs(position.y) > 16000.0f || std::abs(position.z) > 16000.0f)
	{
		Object const * parent = object.getAttachedTo();

		WARNING_STRICT_FATAL(ms_validateObjectPosition, ("Position (%f,%f,%f) is out-of-world and is invalid for object id=[%s], template=[%s], pointer=[%p], appearance=[%s], parent id=[%s], template=[%s], pointer=[%p], appearance=[%s]\n", 
			position.x, 
			position.y, 
			position.z,
			object.getNetworkId().getValueString().c_str(), 
			object.getObjectTemplateName(), 
			&object, 
			object.getAppearance() ? object.getAppearance()->getAppearanceTemplateName() : nullptr, 
			parent ? parent->getNetworkId().getValueString().c_str() : nullptr, 
			parent ? parent->getObjectTemplateName() : nullptr, 
			parent ? parent : nullptr, 
			parent && parent->getAppearance() ? parent->getAppearance()->getAppearanceTemplateName() : nullptr));
	}
}

// ----------------------------------------------------------------------

Object::DpvsObjects *ObjectNamespace::newDpvsObjects()
{
	if (ms_freeDpvsObjectsList.empty())
	{
		// allocate them in bulk from the global memory manager
		const int count = 256;
		Object::DpvsObjects *dpvsObjectListArray = new Object::DpvsObjects[count];
		ms_systemAllocatedDpvsObjectsList.push_back(dpvsObjectListArray);
		ms_allocatedDpvsObjects += count;
		for (int i = 0; i < count; ++i)
			ms_freeDpvsObjectsList.push_back(dpvsObjectListArray+i);
	}

	Object::DpvsObjects *result = ms_freeDpvsObjectsList.back();
	ms_freeDpvsObjectsList.pop_back();
	return result;
}

// ----------------------------------------------------------------------

void ObjectNamespace::deleteDpvsObjects(Object::DpvsObjects *dpvsObjectList)
{
	if (!dpvsObjectList->empty())
	{
		DEBUG_FATAL(true, ("leaked [%d] dpvsObjects", static_cast<int>(dpvsObjectList->size())));
		dpvsObjectList->clear(); //lint !e527 // unreachable // Reachable in release.
	}
	ms_freeDpvsObjectsList.push_back(dpvsObjectList);
}

// ----------------------------------------------------------------------

PropertyList * ObjectNamespace::newPropertyList()
{
	if (ms_propertyListList.empty())
	{
		for (int i = 0; i < 256; ++i)
			ms_propertyListList.push_back(new PropertyList);
	}

	PropertyList * const result = ms_propertyListList.back();
	ms_propertyListList.pop_back();

	return result;
}

// ----------------------------------------------------------------------

void ObjectNamespace::deletePropertyList(PropertyList * const propertyList)
{
	if (ms_propertyListList.size() < 256)
	{
		DEBUG_FATAL(!propertyList->empty(), ("propertyList is not empty"));
		propertyList->clear();
		ms_propertyListList.push_back(propertyList);
	}
	else
		delete propertyList;
}

// ----------------------------------------------------------------------

AttachedObjects * ObjectNamespace::newAttachedObjects()
{
	if (ms_attachedObjectsList.empty())
	{
		for (int i = 0; i < 256; ++i)
			ms_attachedObjectsList.push_back(new AttachedObjects);
	}

	AttachedObjects * const result = ms_attachedObjectsList.back();
	ms_attachedObjectsList.pop_back();

	return result;
}

// ----------------------------------------------------------------------

void ObjectNamespace::deleteAttachedObjects(AttachedObjects * const attachedObjects)
{
	if (ms_attachedObjectsList.size() < 256)
	{
		DEBUG_FATAL(!attachedObjects->empty(), ("attachedObjects is not empty"));
		attachedObjects->clear();
		ms_attachedObjectsList.push_back(attachedObjects);
	}
	else
		delete attachedObjects;
}

// ----------------------------------------------------------------------

void Object::setDpvsObjectHookFunctions(InWorldAddDpvsObjectHookFunction inWorldAddDpvsObjectHookFunction, InWorldRemoveDpvsObjectHookFunction inWorldRemoveDpvsObjectHookFunction, LeakedDpvsObjectHookFunction leakedDpvsObjectHookFunction)
{
	ms_inWorldAddDpvsObjectHookFunction    = inWorldAddDpvsObjectHookFunction;
	ms_inWorldRemoveDpvsObjectHookFunction = inWorldRemoveDpvsObjectHookFunction;
	ms_leakedDpvsObjectHookFunction        = leakedDpvsObjectHookFunction;
}

// ----------------------------------------------------------------------

void Object::setDisallowObjectDelete(bool const disallowObjectDelete)
{
	ms_disallowObjectDelete = disallowObjectDelete;
}

// ======================================================================
// Construct an object

Object::Object():
	m_propertyList(newPropertyList()),
	m_inWorld(false),
	m_active(true),
	m_kill(false),
	m_authoritative(false),
	m_childObject(false),
	m_objectToWorldDirty(true),
#if OBJECT_SUPPORTS_DESTROYED_FLAG
	m_isDestroyed(false),
#endif
#if OBJECT_SUPPORTS_IS_ALTERING_FLAG
	m_isAltering(false),
#endif
	m_objectTemplate(nullptr),
	m_notificationList(NotificationListManager::getEmptyNotificationList()),
	m_debugName(nullptr),
	m_networkId(NetworkId::cms_invalid),
	m_appearance(nullptr),
	m_controller(nullptr),
	m_dynamics(nullptr),
	m_attachedToObject(nullptr),
	m_attachedObjects(nullptr),
	m_dpvsObjects(nullptr),
	m_rotations(0),
	m_scale(Vector::xyz111),
	m_objectToParent(),
	m_objectToWorld(nullptr),
	m_watchedByList(),
	m_containerProperty(nullptr),
	m_collisionProperty(nullptr),
	m_spatialSubdivisionHandle (0),
	m_useAlterScheduler(true),
	m_scheduleData(nullptr),
	m_shouldBakeIntoMesh(true),
	m_defaultAppearance(nullptr),
	m_alternateAppearance(nullptr),
	m_containedBy(nullptr)
{
	m_defaultAppearance = m_appearance;
}

// ----------------------------------------------------------------------

Object::Object(const ObjectTemplate *objectTemplate, const NetworkId &networkId):
	m_propertyList(newPropertyList()),
	m_inWorld(false),
	m_active(true),
	m_kill(false),
	m_authoritative(false),
	m_childObject(false),
	m_objectToWorldDirty(true),
#if OBJECT_SUPPORTS_DESTROYED_FLAG
	m_isDestroyed(false),
#endif
#if OBJECT_SUPPORTS_IS_ALTERING_FLAG
	m_isAltering(false),
#endif
	m_objectTemplate(objectTemplate),
	m_notificationList(NotificationListManager::getEmptyNotificationList()),
	m_debugName(0),
	m_networkId(networkId),
	m_appearance(0),
	m_controller(nullptr),
	m_dynamics(nullptr),
	m_attachedToObject(nullptr),
	m_attachedObjects(nullptr),
	m_dpvsObjects(nullptr),
	m_rotations(0),
	m_scale(Vector::xyz111),
	m_objectToParent(),
	m_objectToWorld(nullptr),
	m_watchedByList(),
	m_containerProperty(nullptr),
	m_collisionProperty(nullptr),
	m_spatialSubdivisionHandle (0),
	m_useAlterScheduler(true),
	m_scheduleData(nullptr),
	m_shouldBakeIntoMesh(true),
	m_defaultAppearance(nullptr),
	m_alternateAppearance(nullptr),
	m_containedBy(nullptr)
{
	objectTemplate->addReference();
	NetworkIdManager::addObject(*this);

	m_defaultAppearance = m_appearance;
}

// ----------------------------------------------------------------------

Object::Object(const ObjectTemplate *objectTemplate, InitializeFlag):
	m_propertyList(newPropertyList()),
	m_inWorld(false),
	m_active(true),
	m_kill(false),
	m_authoritative(false),
	m_childObject(false),
	m_objectToWorldDirty(true),
#if OBJECT_SUPPORTS_DESTROYED_FLAG
	m_isDestroyed(false),
#endif
#if OBJECT_SUPPORTS_IS_ALTERING_FLAG
	m_isAltering(false),
#endif
	m_objectTemplate(objectTemplate),
	m_notificationList(NotificationListManager::getEmptyNotificationList()),
	m_debugName(0),
	m_networkId(NetworkId::cms_invalid),
	m_appearance(0),
	m_controller(nullptr),
	m_dynamics(nullptr),
	m_attachedToObject(nullptr),
	m_attachedObjects(nullptr),
	m_dpvsObjects(nullptr),
	m_rotations(0),
	m_scale(Vector::xyz111),
	m_objectToParent(),
	m_objectToWorld(nullptr),
	m_watchedByList(),
	m_containerProperty(nullptr),
	m_collisionProperty(nullptr),
	m_spatialSubdivisionHandle (0),
	m_useAlterScheduler(true),
	m_scheduleData(nullptr),
	m_shouldBakeIntoMesh(true),
	m_defaultAppearance(nullptr),
	m_alternateAppearance(nullptr),
	m_containedBy(nullptr)
{
	objectTemplate->addReference();
	NetworkIdManager::addObject(*this);

	m_defaultAppearance = m_appearance;
}

// ----------------------------------------------------------------------
/**
 * Destroy an object.
 */

Object::~Object(void)
{
	//-- Store the name of hte most recently deleted object so we can track down live crashes if it fails somewhere in here.
	IGNORE_RETURN(snprintf(ms_crashReportInfo, sizeof(ms_crashReportInfo) - 1, "~Object: name=[%s] template=[%s]\n", getDebugName(), getObjectTemplateName()));
	ms_crashReportInfo[sizeof(ms_crashReportInfo) - 1] = '\0';

	DEBUG_REPORT_LOG(ms_logObjectDelete, ("Deleting object id=[%s], template=[%s], pointer=[%p], appearance=[%s], parent id=[%s], template=[%s], pointer=[%p], appearance=[%s]\n", getNetworkId().getValueString().c_str(), getObjectTemplateName(), this, getAppearance() ? getAppearance()->getAppearanceTemplateName() : nullptr, m_attachedToObject ? m_attachedToObject->getNetworkId().getValueString().c_str() : nullptr, m_attachedToObject ? m_attachedToObject->getObjectTemplateName() : nullptr, m_attachedToObject ? m_attachedToObject : nullptr, m_attachedToObject && m_attachedToObject->getAppearance() ? m_attachedToObject->getAppearance()->getAppearanceTemplateName() : nullptr));
	FATAL(ConfigSharedObject::getAllowDisallowObjectDelete() && ms_disallowObjectDelete, ("Object id=[%s], template=[%s], pointer=[%p] is deleting itself when delete is not allowed", getNetworkId().getValueString().c_str(), getObjectTemplateName(), this));

#if OBJECT_SUPPORTS_IS_ALTERING_FLAG
	FATAL(m_isAltering, ("Object id=[%s], template=[%s], pointer=[%p] is deleting itself while it is getting altered. Unsupported, fix object.", getNetworkId().getValueString().c_str(), getObjectTemplateName(), this));
#endif

	if (m_inWorld)
		Object::removeFromWorld();

	if (m_scheduleData)
	{
		bool const wasInAlterScheduler = AlterScheduler::removeObject(*this);
		UNREF(wasInAlterScheduler);

		delete m_scheduleData;
		m_scheduleData = 0;
	}

#if OBJECT_SUPPORTS_DESTROYED_FLAG
	// make sure we can tell this object is getting destroyed.
	m_isDestroyed = true;
#endif

	// remove object from parent (make sure it's in the world cell)
	if (m_attachedToObject)
	{
		detachFromObject(DF_world);
		m_attachedToObject = 0;
	}

	// destroy all the properties
	while(!m_propertyList->empty())
	{
		Property * p = m_propertyList->back();
		m_propertyList->pop_back();
		delete p;
	}
	m_containerProperty = 0;
	m_collisionProperty = 0;
	m_containedBy = 0;

	deletePropertyList(m_propertyList);

	delete m_defaultAppearance;
	delete m_alternateAppearance;

	m_appearance = 0;
	m_defaultAppearance = 0;
	m_alternateAppearance = 0;

	// destroy children
	if (m_attachedObjects)
	{
		while (!m_attachedObjects->empty())
		{
			Object *child = m_attachedObjects->back();
			if (!child->isChildObject())
			{
				WARNING_STRICT_FATAL(true, ("deleting object with non-child attached objects, parent id=[%s], objectTemplate=[%s], appearanceTemplate=[%s], child id=[%s], ptr=[0x%08x], objectTemplate=[%s], appearanceTemplate=[%s], name=[%s])", getNetworkId().getValueString().c_str(), getObjectTemplateName(), getAppearance() && getAppearance()->getAppearanceTemplate() ? getAppearance()->getAppearanceTemplate()->getName() : 0, child->getNetworkId().getValueString().c_str(), child, child->getObjectTemplateName(), child->getAppearance() && child->getAppearance()->getAppearanceTemplate() ? child->getAppearance()->getAppearanceTemplate()->getName() : 0, child->getDebugName()));
				child->detachFromObject(DF_parent);
			}
			else
			{
				delete child;
			}
		}

		deleteAttachedObjects(m_attachedObjects);
		m_attachedObjects = nullptr;
	}

	if (m_objectToWorld)
	{
		deleteLocalTransform(m_objectToWorld);
		m_objectToWorld = 0;
	}

	if (m_objectTemplate != nullptr)
		m_objectTemplate->releaseReference();
	m_objectTemplate = nullptr;

	m_notificationList = 0;

	if (m_dpvsObjects)
	{
		if (!m_dpvsObjects->empty())
		{
			WARNING(true, ("Object still has %d DPVS objects attached", static_cast<int>(m_dpvsObjects->size())));

			if (ms_leakedDpvsObjectHookFunction)
			{
				DpvsObjects::iterator const iEnd = m_dpvsObjects->end();
				for (DpvsObjects::iterator i = m_dpvsObjects->begin(); i != iEnd; ++i)
				{
					ms_leakedDpvsObjectHookFunction(*i);
				}
			}

			m_dpvsObjects->clear();

		}

		deleteDpvsObjects(m_dpvsObjects);
		m_dpvsObjects = 0;
	}

	delete m_controller;
	m_controller = 0;

	delete m_dynamics;
	m_dynamics = 0;

	delete [] m_debugName;
	m_debugName = 0;

	NetworkIdManager::removeObject(*this);

	m_spatialSubdivisionHandle = 0;
}

// ----------------------------------------------------------------------

#if OBJECT_SUPPORTS_DESTROYED_FLAG

bool Object::isDestroyed() const
{
	return m_isDestroyed;
}

#endif

// ----------------------------------------------------------------------

#if OBJECT_SUPPORTS_IS_ALTERING_FLAG

bool Object::isAltering() const
{
	return m_isAltering;
}

#endif

// ----------------------------------------------------------------------
/**
 * Set a new name for the object.
 *
 * If the object had a name, that name will be deleted.  The new name will
 * be copied into a dynamically allocated buffer using DuplicateString().
 *
 * @param newName  New name for the object
 */

void Object::setDebugName(const char *newName)
{
	delete [] m_debugName;
	m_debugName = 0;

	if (newName)
		m_debugName = DuplicateString(newName);
}

// ----------------------------------------------------------------------

std::string const Object::getDebugInformation(bool const includeParent) const
{
	char buffer[512];

	if (includeParent && m_attachedToObject)
	{
		IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "[ptr=%p, id=%s, ot=%s, at=%s, child=%i, pptr=%p, pid=%s, pot=%s, pat=%s]", 
			this,
			getNetworkId().getValueString().c_str(),
			getObjectTemplateName(), 
			getAppearanceTemplateName(), 
			isChildObject() ? 1 : 0,
			m_attachedToObject,
			m_attachedToObject->getNetworkId().getValueString().c_str(),
			m_attachedToObject->getObjectTemplateName(), 
			m_attachedToObject->getAppearanceTemplateName()));
	}
	else
	{
		IGNORE_RETURN(snprintf(buffer, sizeof(buffer) - 1, "[ptr=%p, id=%s, ot=%s, at=%s]", 
			this,
			getNetworkId().getValueString().c_str(),
			getObjectTemplateName(), 
			getAppearanceTemplateName()));
	}

	buffer[sizeof(buffer) - 1] = '\0';

	return buffer;
}

// ----------------------------------------------------------------------

bool Object::hasNotification(const ObjectNotification &notification) const
{
	return m_notificationList->contains(notification);
}

// ----------------------------------------------------------------------

void Object::addNotification(const ObjectNotification &notification, bool allowWhenInWorld)
{
	UNREF(allowWhenInWorld);
	DEBUG_FATAL(m_inWorld && !allowWhenInWorld, ("Cannot change notification while part of the world"));

	if (m_notificationList->contains(notification))
	{
		DEBUG_WARNING(true, ("Adding notification that is already present"));
		return;
	}
	m_notificationList = NotificationListManager::addNotification(m_notificationList, notification);
}

// ----------------------------------------------------------------------

void Object::removeNotification(const ObjectNotification &notification, bool allowWhenInWorld)
{
	UNREF(allowWhenInWorld);
	DEBUG_FATAL(m_inWorld && !allowWhenInWorld, ("Cannot change notification while part of the world"));
	DEBUG_FATAL(!m_notificationList->contains(notification), ("Removing notification that does not exist"));
	m_notificationList = NotificationListManager::removeNotification(m_notificationList, notification);
}

// ----------------------------------------------------------------------

void Object::addToWorld()
{
	if (m_inWorld)
	{
		DEBUG_WARNING(true, ("Object (%s : %s) is already in the world.", getObjectTemplateName(), getNetworkId().getValueString().c_str()));
		return;
	}

	setObjectToWorldDirty(true);

	m_inWorld = true;

	if (m_appearance)
		m_appearance->addToWorld();

	{
		PropertyList::iterator iEnd = m_propertyList->end();
		for (PropertyList::iterator i = m_propertyList->begin(); i != iEnd; ++i)
			(*i)->addToWorld();
	}

	m_notificationList->addToWorld(*this);

	if (m_attachedObjects && !m_attachedObjects->empty())
	{
		const AttachedObjects::iterator iEnd = m_attachedObjects->end();
		for (AttachedObjects::iterator i = m_attachedObjects->begin(); i != iEnd; ++i)
			// Do not add attached child objects that are already in the world.
			//   This happens when an object has a PortalProperty in its PropertyList.
			if ((*i)->isChildObject() && !(*i)->isInWorld())
				(*i)->addToWorld();
	}
}

// ----------------------------------------------------------------------
/**
 * Remove an object from the world.
 *
 * The notifications will be called in the reverse order.
 */

void Object::removeFromWorld()
{
	if (!m_inWorld)
	{
		WARNING_STRICT_FATAL(true, ("Object %s not in the world", getDebugInformation(true).c_str()));
		return;
	}

	setObjectToWorldDirty(true);

	{
		for (PropertyList::iterator i = m_propertyList->begin(); i != m_propertyList->end(); ++i)
			(*i)->removeFromWorld();
	}

	m_notificationList->removeFromWorld(*this);

	if (m_appearance)
		m_appearance->removeFromWorld();

	if (m_attachedObjects && !m_attachedObjects->empty())
	{
		const AttachedObjects temp(*m_attachedObjects);
		const AttachedObjects::const_iterator end = temp.end();
		for (AttachedObjects::const_iterator i = temp.begin(); i != end; ++i)
		{
			Object *attached= *i;
			if (attached->isChildObject())
				attached->removeFromWorld();
			else
			{
				if (attached->isInWorld())
				{
					DEBUG_WARNING(true, ("Removing an object [id=%s template=%s] from the world with non-child attached objects [id=%s ptr=0x%08x template=%s name=%s]", getNetworkId ().getValueString ().c_str (), getObjectTemplateName () ? getObjectTemplateName () : "nullptr", attached->getNetworkId ().getValueString ().c_str (), attached, attached->getObjectTemplateName () ? attached->getObjectTemplateName () : "nullptr", attached->getDebugName()));
					attached->detachFromObject(DF_parent);
				}
			}
		}
	}

	m_inWorld = false;
}

// ----------------------------------------------------------------------

void Object::addDpvsObject(DPVS::Object *dpvsObject)
{
	NOT_NULL(dpvsObject);
	if (!m_dpvsObjects)
		m_dpvsObjects = newDpvsObjects();

	DpvsObjects::iterator i = std::find(m_dpvsObjects->begin(), m_dpvsObjects->end(), dpvsObject);

	if (i != m_dpvsObjects->end())
	{
		DEBUG_FATAL(true, ("Object::addDpvsObject() Adding duplicate DPVS object [0x%08x] to Object %s", dpvsObject, getDebugInformation().c_str()));
		return;
	}

	m_dpvsObjects->push_back(dpvsObject);

	if (ms_inWorldAddDpvsObjectHookFunction && isInWorld())
		ms_inWorldAddDpvsObjectHookFunction(this, dpvsObject);
}

// ----------------------------------------------------------------------

void Object::removeDpvsObject(DPVS::Object *dpvsObject)
{
	NOT_NULL(dpvsObject);
	NOT_NULL(m_dpvsObjects);

	DpvsObjects::iterator i = std::find(m_dpvsObjects->begin(), m_dpvsObjects->end(), dpvsObject);

	if (i == m_dpvsObjects->end())
	{
		DEBUG_FATAL(true, ("Object::removeDpvsObject() Removing unknown DPVS object [0x%08x] from Object %s", dpvsObject, getDebugInformation().c_str()));
		return;
	}

	*i = m_dpvsObjects->back();
	m_dpvsObjects->pop_back();

	if (ms_inWorldRemoveDpvsObjectHookFunction && isInWorld())
		ms_inWorldRemoveDpvsObjectHookFunction(dpvsObject);
}

// ----------------------------------------------------------------------

void Object::positionChanged(bool dueToParentChange, const Vector &oldPosition)
{
#ifdef _DEBUG
	IGNORE_RETURN(getTransform_o2p().validate());
	IGNORE_RETURN(getTransform_o2w().validate());
#endif

	setObjectToWorldDirty(true);

	if (m_inWorld)
	{
		validatePosition(*this, getPosition_p());

		m_notificationList->positionChanged(*this, dueToParentChange, oldPosition);

		if (m_attachedObjects && !m_attachedObjects->empty())
		{
			const AttachedObjects::iterator end = m_attachedObjects->end();
			for (AttachedObjects::iterator i = m_attachedObjects->begin(); i != end; ++i)
				(*i)->positionChanged(true, (*i)->getPosition_p());
		}
	}
}

// ----------------------------------------------------------------------

void Object::rotationChanged(bool dueToParentChange)
{
#ifdef _DEBUG
	IGNORE_RETURN(getTransform_o2p().validate());
	IGNORE_RETURN(getTransform_o2w().validate());
#endif

	setObjectToWorldDirty(true);

	if (m_inWorld)
	{
		m_notificationList->rotationChanged(*this, dueToParentChange);

		if (m_attachedObjects && !m_attachedObjects->empty())
		{
			const AttachedObjects::iterator end = m_attachedObjects->end();
			for (AttachedObjects::iterator i = m_attachedObjects->begin(); i != end; ++i)
				(*i)->rotationChanged(true);
		}
	}
}

// ----------------------------------------------------------------------

void  Object::positionAndRotationChanged(bool dueToParentChange, const Vector &oldPosition)
{
#ifdef _DEBUG
	IGNORE_RETURN(getTransform_o2p().validate());
	IGNORE_RETURN(getTransform_o2w().validate());
#endif

	setObjectToWorldDirty(true);

	if (m_inWorld)
	{
		validatePosition(*this, getPosition_p());

		m_notificationList->positionAndRotationChanged(*this, dueToParentChange, oldPosition);

		if (m_attachedObjects && !m_attachedObjects->empty())
		{
			const AttachedObjects::iterator end = m_attachedObjects->end();
			for (AttachedObjects::iterator i = m_attachedObjects->begin(); i != end; ++i)
				(*i)->positionAndRotationChanged(true, (*i)->getPosition_p());
		}
	}
}

// ----------------------------------------------------------------------

void Object::cellChanged(bool dueToParentChange)
{
	setObjectToWorldDirty(true);

	if (m_inWorld)
	{
		m_notificationList->cellChanged(*this, dueToParentChange);

		if (m_attachedObjects && !m_attachedObjects->empty())
		{
			const AttachedObjects::iterator end = m_attachedObjects->end();
			for (AttachedObjects::iterator i = m_attachedObjects->begin(); i != end; ++i)
				(*i)->cellChanged(true);
		}
	}
}

// ----------------------------------------------------------------------

void Object::extentChanged()
{
	if (m_inWorld)
		m_notificationList->extentChanged(*this);
}

// ----------------------------------------------------------------------

void Object::pobFixupComplete()
{
	if (m_inWorld)
		m_notificationList->pobFixupComplete(*this);
}

// ----------------------------------------------------------------------
/**
 * Set an active state for the object.
 *
 * Inactive objects will not render.
 *
 * @param newActive  New active state for the object
 */

void Object::setActive(bool newActive)
{
	m_active = newActive;
}

// ----------------------------------------------------------------------
/**
 * Set a new dynamics for the object.
 *
 * The old dynamics pointer is simply overwritten.
 *
 * @param newDynamics  New dynamics for the object
 */

void Object::setDynamics(Dynamics *newDynamics)
{
	if (m_dynamics == newDynamics)
		return;

	delete m_dynamics;
	m_dynamics = newDynamics;
}

// ----------------------------------------------------------------------
/**
 * Set a new controller for the object.
 *
 * The old controller pointer is simply overwritten.
 *
 * @param newController  New controller for the object
 */

void Object::setController(Controller *newController)
{
	if (m_controller == newController)
		return;

	delete m_controller;
	m_controller = newController;
}
// ----------------------------------------------------------------------

/**
 * Determine if the object currently resides in the World cell
 * @return true if in the world cell, else false
 */

bool Object::isInWorldCell() const
{
	return getParentCell() == CellProperty::getWorldCellProperty();
}

// ----------------------------------------------------------------------

CellProperty *Object::getParentCell() const
{
	Property *cell = nullptr;

	for (Object *o = const_cast<Object *>(getAttachedTo()); o && !cell; o = o->getAttachedTo())
		cell = o->getCellProperty();

	if (!cell)
		return CellProperty::getWorldCellProperty();

	return safe_cast<CellProperty*>(cell);
}

// ----------------------------------------------------------------------

void Object::setParentCell(CellProperty *cellProperty)
{
	NOT_NULL(cellProperty);

	// if we are already in that cell, don't do anything.
	if (getParentCell() == cellProperty)
		return;

	// if we were in another cell, detach us.  This will leave out object in world space.
	if (!isInWorldCell())
		detachFromObject(DF_world);

	// we are now relative to the new cell
	if (cellProperty != CellProperty::getWorldCellProperty())
		attachToObject_w(&cellProperty->getOwner(), false);

	// issue a cell notification change
	cellChanged(false);
}

// ----------------------------------------------------------------------
/**
 * Get the object-to-world transformation for this object.
 *
 * @return Const reference to the object-to-world transform for this object
 */

const Transform &Object::getTransform_o2w() const
{
#if PROFILE_GETTRANSFORM_O2W == 1
	++ms_totalCalls;
#endif

	if (m_attachedToObject)
	{
#if PROFILE_GETTRANSFORM_O2W == 1
		++ms_totalAttachedCalls;
#endif

#if ENABLE_OBJECTTOWORLDDIRTY == 1
		if (m_objectToWorldDirty)
#endif
		{
#if PROFILE_GETTRANSFORM_O2W == 1
			++ms_totalAttachedDirtyCalls;
#endif

			m_objectToWorld->multiply(m_attachedToObject->getTransform_o2w(), m_objectToParent);
			setObjectToWorldDirty(false);
		}

		return *m_objectToWorld;
	}

	return m_objectToParent;
}

// ----------------------------------------------------------------------

void Object::setTransform_o2w(const Transform &objectToWorld)
{
	CellProperty const * cell = getParentCell();

	if(cell == CellProperty::getWorldCellProperty())
	{
		setTransform_o2p(objectToWorld);
		return;
	}

	Transform const & cellToWorld = cell->getOwner().getTransform_o2w();

	Transform worldToCell;

	worldToCell.invert(cellToWorld);

	Transform objectToCell;

	objectToCell.multiply(worldToCell,objectToWorld);

	setTransform_o2p(objectToCell);
}

// ----------------------------------------------------------------------
/**
 * Get the appearance-to-parent transformation for this object.
 *
 * @return Const reference to the object-to-parent transform for this object
 */

void Object::setTransform_a2w(const Transform &appearanceToWorld) const
{
	if (!m_appearance)
	{
		Appearance *dummyAppearance = new Appearance(0);
		const_cast<Object *>(this)->setAppearance(dummyAppearance);
	}

	m_appearance->setTransform_w(appearanceToWorld);
}

// ----------------------------------------------------------------------
/**
 * Get the appearance-to-parent transformation for this object.
 *
 * @return Const reference to the object-to-parent transform for this object
 */

const Transform &Object::getTransform_a2w() const
{
	if (!m_appearance)
	{
		Appearance *dummyAppearance = new Appearance(0);
		const_cast<Object *>(this)->setAppearance(dummyAppearance);
	}

	return m_appearance->getTransform_w();
}

// ----------------------------------------------------------------------

const Transform Object::getTransform_o2c() const
{
	Transform result = getTransform_o2p();

	for (const Object *o = getAttachedTo(); o && !o->getCellProperty(); o = o->getAttachedTo())
		result.multiply(o->getTransform_o2p(), result);

	return result;
}

// ----------------------------------------------------------------------
/**
 * Set a new Appearance for the object.
 *
 * This routine will delete the old appearance and assign the specified
 * appearance.
 *
 * @param newAppearance  New appearance for the object
 */

void Object::setAppearance(Appearance *newAppearance)
{
	if (m_appearance == newAppearance)
		return;

	
	if (m_appearance) // delete the old appearance
	{
		// DBE - Copy the transform from the old appearance to the new one.
		// This was added to mimic the behavior of when the appearance-to-world 
		// transform was stored in Object instead of Appearance.
		if (newAppearance)
		{
			newAppearance->setTransform_w(m_appearance->getTransform_w());
		}

		if (isInWorld())
		{
			m_appearance->removeFromWorld();
		}
		delete m_appearance;
	}
	else
	{
		// DBE - Initialize the transform of the new appearance to our object-to-world
		// transform.  This was added to mimic the behavior of when the appearance-to-world 
		// transform was stored in Object instead of Appearance.
		newAppearance->setTransform_w(getTransform_o2w());
	}

	// start using the new appearance
	m_appearance = newAppearance;

	if (m_appearance)
	{
		m_appearance->setOwner(this);
		m_appearance->setScale(m_scale);

		if (isInWorld())
		{
			m_appearance->addToWorld();
		}
	}

	m_defaultAppearance = m_appearance;


}

// ----------------------------------------------------------------------
/**
 * Steal the Appearance from this object.
 *
 * This routine will return the current appearance of this object, and
 * then reset its appearance to nullptr.
 *
 * @return The current appearance of this object
 */

Appearance *Object::stealAppearance(void)
{
	Appearance *oldAppearance = m_appearance;

	if(m_appearance == m_defaultAppearance)
		m_defaultAppearance = nullptr;
	else if (m_appearance == m_alternateAppearance)
		m_alternateAppearance = nullptr;
	
	m_appearance = nullptr;

	if (oldAppearance)
	{
		if (isInWorld())
			oldAppearance->removeFromWorld();

		oldAppearance->setOwner(nullptr);
	}

	return oldAppearance;
}

// ----------------------------------------------------------------------
/**
 * Get the appearance's sphere's center.
 *
 * If the appearance doesn't exist, it returns the position of the object.
 */

const Vector &Object::getAppearanceSphereCenter(void) const
{
	if (m_appearance)
		return m_appearance->getSphere().getCenter();
	else
		return Vector::zero;
}

// ----------------------------------------------------------------------
/**
 * Get the appearance's sphere's radius.
 */

real Object::getAppearanceSphereRadius(void) const
{
	if(!m_appearance)
		return 0;
	return m_appearance->getSphere().getRadius();
}

// ----------------------------------------------------------------------
/**
 * Get the appearance's sphere's center in world space.
 *
 * If the object does not have an appearance, this routine will return
 * the same value as getPosition_w(), the object's world position.
 */

const Vector Object::getAppearanceSphereCenter_w(void) const
{
	if (m_appearance)
		return rotateTranslate_o2w(getAppearanceSphereCenter());
	else
		return getPosition_w();
}

// ----------------------------------------------------------------------
/**
 * Alter the object specifying the amount of time that has passed.
 *
 * If the object is active, and has a Controller, the Controller's alter() routine will
 * be called.
 *
 * If the object is active, and has an Appearance, the Appearances's alter() routine will
 * be called.
 *
 * If this object is active, it will call alter() for all its child objects,
 *
 * @param time  Number of elapsed seconds
 */

float Object::alter(float time)
{
	PROFILER_AUTO_BLOCK_DEFINE("Object::alter");

	DEBUG_FATAL(!isInitialized(), ("Alter called on uninitialized object:pointer=[%p],id=[%s],template=[%s].", this, getNetworkId().getValueString().c_str(), getObjectTemplateName()));

	if (m_kill)
		return AlterResult::cms_kill;

	// Allow Object components to dictate whether they need an alter.
	float alterResult = AlterResult::cms_keepNoAlter;
	float appearanceAlterResult = AlterResult::cms_keepNoAlter;
	float controllerAlterResult = AlterResult::cms_keepNoAlter;
	float dynamicsAlterResult = AlterResult::cms_keepNoAlter;
	float childrenAlterResult = AlterResult::cms_keepNoAlter;

	if (m_active)
	{
		// Note: characters assume appearances get an alter() prior to the controller getting an alter()
		if (m_appearance)
		{
			appearanceAlterResult = m_appearance->alter(time);
			AlterResult::incorporateAlterResult(alterResult, appearanceAlterResult);
		}

		if (m_controller)
		{
			controllerAlterResult = m_controller->alter(time);
			AlterResult::incorporateAlterResult(alterResult, controllerAlterResult);
		}

		if (m_dynamics)
		{
			dynamicsAlterResult = m_dynamics->alter(time);
			AlterResult::incorporateAlterResult(alterResult, dynamicsAlterResult);
		}

		if (ms_objectsAlterChildrenAndContents)
		{
			// alter all the child objects
			if (m_attachedObjects)
			{
				//-- note: children may not remove other children
				int size = static_cast<int>(m_attachedObjects->size ());

				for (int i = 0; i < size;)
				{
					Object * const thisChild = (*m_attachedObjects)[static_cast<uint>(i)];
					NOT_NULL (thisChild);

					// handle child objects wanting to die
					if (thisChild->isChildObject())
					{
						if (thisChild->isInitialized())
						{
							AlterScheduler::setMostRecentAlterTime(*thisChild);
							float const childAlterResult = thisChild->alter(time);
							if (childAlterResult == AlterResult::cms_kill) //lint !e777 // testing floats for equality // This is okay, we use constants for both set and test.
							{
								delete thisChild;
								--size;
							}
							else
							{
								++i;
								AlterResult::incorporateAlterResult(childrenAlterResult, childAlterResult);
							}
						}
						else
						{
							DEBUG_WARNING(true, ("Alter was about to be called on uninitialized child object [id=%s template=%s] with parent object [id=%s template=%s], skipping child alter", thisChild->getNetworkId().getValueString().c_str(), thisChild->getObjectTemplateName(), getNetworkId().getValueString().c_str(), getObjectTemplateName()));
							++i;
						}
					}
					else
						++i;
				}

				AlterResult::incorporateAlterResult(alterResult, childrenAlterResult);
			}

			// alter all the contained objects
			if (m_containerProperty)
				m_containerProperty->alter(time);
		}
	}

#ifdef _DEBUG
	AlterScheduler::countObjectAlter();
	if (AlterScheduler::reportObjectAlters())
	{
		char const * appearanceType = m_appearance ? typeid(*m_appearance).name() : "none";
		char const * controllerType = m_controller ? typeid(*m_controller).name() :  "none";
		char const * dynamicsType   = m_dynamics   ? typeid(*m_dynamics).name() : "none";
		AlterScheduler::reportObjectAlter(this, appearanceType, appearanceAlterResult, controllerType, controllerAlterResult, dynamicsType, dynamicsAlterResult, childrenAlterResult, alterResult);
	}
#endif

	// @todo fix this up to return what's appropriate for the state of this instance.
	return alterResult;
}

// ----------------------------------------------------------------------
/**
 * Conclude the object specifying the amount of time that has passed.
 *
 * If the object is active, and has a Controller, the Controller's conclude() routine will
 * be called.
 *
 * If this object is active, it will call conclude() for all its child objects,
 *
 * @param time  Number of elapsed seconds
 */

void Object::conclude()
{
	if (isActive())
	{
		if (m_controller)
			m_controller->conclude();

		// conclude all the child objects
		if (m_attachedObjects)
		{
			const AttachedObjects::iterator end = m_attachedObjects->end();
			for (AttachedObjects::iterator i = m_attachedObjects->begin(); i != end; ++i)
				if ((*i)->isChildObject())
					(*i)->conclude ();
		}
	}
}

// ----------------------------------------------------------------------

int Object::getNumberOfAttachedObjects() const
{
	if (!m_attachedObjects)
		return 0;

	return static_cast<int>(m_attachedObjects->size());
}

// ----------------------------------------------------------------------

Object * Object::getAttachedObject(int const attachedObjectIndex)
{
	NOT_NULL(m_attachedObjects);
	DEBUG_FATAL(attachedObjectIndex < 0 || attachedObjectIndex >= getNumberOfAttachedObjects(), ("attached object index out of range %d/%d", attachedObjectIndex, getNumberOfAttachedObjects()));

	Object * const attached = (*m_attachedObjects)[static_cast<uint>(attachedObjectIndex)];
	NOT_NULL (attached);
	return attached;
}

// ----------------------------------------------------------------------

Object const * Object::getAttachedObject(int const attachedObjectIndex) const
{
	NOT_NULL(m_attachedObjects);
	DEBUG_FATAL(attachedObjectIndex < 0 || attachedObjectIndex >= getNumberOfAttachedObjects(), ("attached object index out of range %d/%d", attachedObjectIndex, getNumberOfAttachedObjects()));

	Object const * const attached = (*m_attachedObjects)[static_cast<uint>(attachedObjectIndex)];
	NOT_NULL (attached);
	return attached;
}

// ----------------------------------------------------------------------
/**
 * Get the number of child objects of this object.
 *
 * This routine walks a linked list of the objects to count the number
 * of them.  If you only need to know if the Object has child objects,
 * call the more efficient hasChildObjects() routine instead.
 *
 * @return The number of child objects of this object
 * @see Object::hasChildObjects()
 */

int Object::getNumberOfChildObjects(void) const
{
	// todo @portal: make this reflect only child objects, not attached objects
	if (!m_attachedObjects)
		return 0;

	return static_cast<int> (m_attachedObjects->size());
}

// ----------------------------------------------------------------------
/**
 * Get the specified child object.
 *
 * If the childObjectIndex is out of range, this routine will call Fatal
 * in debug compiles.
 *
 * @param childObjectIndex  Index of the child object to retrieve
 * @return Pointer to the child object
 */

Object *Object::getChildObject(int childObjectIndex)
{
	// todo @portal: make this reflect only child objects, not attached objects

	NOT_NULL(m_attachedObjects);
	DEBUG_FATAL(childObjectIndex < 0 || childObjectIndex >= getNumberOfChildObjects(), ("child object index out of range %d/%d", childObjectIndex, getNumberOfChildObjects()));

	Object *child = (*m_attachedObjects) [static_cast<uint> (childObjectIndex)];
	NOT_NULL (child);
	return child;
}

// ----------------------------------------------------------------------
/**
 * Get the specified child object.
 *
 * If the childObjecIndex is out of range, this routine will call Fatal
 * in debug compiles.
 *
 * @param childObjectIndex  Index of the child object to retrieve
 * @return Pointer to the child object
 */

const Object *Object::getChildObject(int childObjectIndex) const
{
	// todo @portal: make this reflect only child objects, not attached objects

	NOT_NULL(m_attachedObjects);
	DEBUG_FATAL(childObjectIndex < 0 || childObjectIndex >= getNumberOfChildObjects(), ("child object index out of range %d/%d", childObjectIndex, getNumberOfChildObjects()));

	const Object *child = (*m_attachedObjects) [static_cast<uint> (childObjectIndex)];
	NOT_NULL (child);
	return child;
}

// ----------------------------------------------------------------------

void Object::attachToObject_p(Object * const object, bool const asChildObject)
{
	NOT_NULL(object);
	DEBUG_FATAL(m_childObject, ("Object::attachToObject_p: object %s already is a child object", getDebugInformation(true).c_str()));
	DEBUG_FATAL(m_attachedToObject, ("Object::attachToObject_p: object %s already is relative to another object", getDebugInformation(true).c_str()));
	DEBUG_FATAL(this == object, ("attempting to attach to self"));

	// If the child object is already attached to an object, detach it.
	if (m_attachedToObject) 
		detachFromObject(DF_none);

	// put this on the list of attached objects
	if (!object->m_attachedObjects)
		object->m_attachedObjects = newAttachedObjects();
	object->m_attachedObjects->push_back(this);

	// remove the object from the world if its being added as a child
	// and is currently in the world.  This must happen before the m_childObject
	// flag is set so that the worlds don't erroneously optimize out necessary
	// operations by looking at the isChildObject() method.  This issue happened
	// with removing objects from the ClientWorld sphere trees.
	if (asChildObject && isInWorld())
		removeFromWorld();

	// every child should know its parent
	m_attachedToObject = object;
	m_childObject = asChildObject;

	// create a new objectToWorld transform for the child
	m_objectToWorld = newLocalTransform();
	setObjectToWorldDirty(true);

	if (asChildObject)
	{
		// add the child to the world if the parent is in the world.
		if (object->isInWorld())
			addToWorld();

		// if parent objects alter their children, ensure the new child
		// object is not in the alter scheduler.
		if (ms_objectsAlterChildrenAndContents)
			IGNORE_RETURN(AlterScheduler::removeObject(*this));
	}

	//-- always schedule an object for alter next frame if it has received a new child object
	if (World::isInstalled() && World::existsInWorld(getRootParent()))
		scheduleForAlter();
}

// ----------------------------------------------------------------------

void Object::attachToObject_w(Object *object, bool asChildObject)
{
#ifdef _DEBUG
	IGNORE_RETURN(m_objectToParent.validate());
#endif
	// calculate frame and position in the master's space
	const Vector i = object->rotate_w2o(getObjectFrameI_w());
	const Vector j = object->rotate_w2o(getObjectFrameJ_w());
	const Vector k = object->rotate_w2o(getObjectFrameK_w());
	const Vector p = object->rotateTranslate_w2o(getPosition_w());

	// set the transform of the child object
	m_objectToParent.setLocalFrameIJK_p(i, j, k);
	m_objectToParent.setPosition_p(p);
	reorthonormalize();

	// now do the attach in master space
	attachToObject_p(object, asChildObject);
#ifdef _DEBUG
	IGNORE_RETURN(m_objectToParent.validate());
#endif

}

// ----------------------------------------------------------------------
/**
 * Detach an object from its master.
 *
 * Removal of an attached object causes the last attached object to move
 * into its place in the vector.  Therefore, attached object removal causes
 * the order of attached objects to change.
 */

void Object::detachFromObject(DetachFlags const detachFlags)
{
	NOT_NULL(m_attachedToObject);
#ifdef _DEBUG
	IGNORE_RETURN(m_objectToParent.validate());
#endif

	// leave the child object where it was in the same cell
	bool const toParentCell = (detachFlags == DF_parent);
	bool const noCell = (detachFlags == DF_none);

	CellProperty * const cellProperty = noCell ? 0 : (toParentCell ? getParentCell() : CellProperty::getWorldCellProperty());
	bool const shouldAttach = (!toParentCell || noCell) ? false : m_attachedToObject != &cellProperty->getOwner();
	m_objectToParent = shouldAttach ? getTransform_o2c() : getTransform_o2w();
	deleteLocalTransform(m_objectToWorld);
	m_objectToWorld = nullptr;
	setObjectToWorldDirty(true);

	// remove from the attached objects list
	AttachedObjects *attachedObjects = m_attachedToObject->m_attachedObjects;
	const AttachedObjects::iterator i = std::find(attachedObjects->begin(), attachedObjects->end(), this);
	DEBUG_FATAL(i == attachedObjects->end(), ("attached object was not found in parent"));
	*i = attachedObjects->back();
	attachedObjects->pop_back();

	// set as unattached
	m_attachedToObject = nullptr;

	bool const wasChildObject = isChildObject();
	bool const wasInWorld = isInWorld();
	if (wasChildObject && wasInWorld)
		removeFromWorld();

	m_childObject = false;

 	if (wasChildObject && wasInWorld)
		addToWorld();

#ifdef _DEBUG
	IGNORE_RETURN(m_objectToParent.validate());
#endif

	if (shouldAttach)
		setParentCell(cellProperty);
}

// ----------------------------------------------------------------------
/**
 * Add a new child object to the end of the child object list.
 *
 * This routine will append the new child object to the list of child
 * objects.
 *
 * @param newChildObject  New child object
 */

void Object::addChildObject_o(Object * const newChildObject)
{
	DEBUG_FATAL(!ms_objectsAlterChildrenAndContents, ("Only the client should add child objects."));
	newChildObject->attachToObject_p(this, true);
}

// ----------------------------------------------------------------------
/**
 * Add a new child object to the end of the child object list.
 *
 * This routine will append the new child object to the list of child
 * objects.
 *
 * @param newChildObject  New child object
 */

void Object::addChildObject_p(Object * const newChildObject)
{
	// calculate frame and position in this object's space
	Vector p = rotateTranslate_p2o(newChildObject->getPosition_p());
	Vector i = rotate_p2o(newChildObject->getObjectFrameI_p());
	Vector j = rotate_p2o(newChildObject->getObjectFrameJ_p());
	Vector k = rotate_p2o(newChildObject->getObjectFrameK_p());

	// set the transform of the child object
	newChildObject->setTransformIJK_o2p(i, j, k);
	newChildObject->setPosition_p(p);
	newChildObject->reorthonormalize();

	// add this object as a child object
	addChildObject_o(newChildObject);
}

// ----------------------------------------------------------------------
/**
 * Remove a child object from this object.
 *
 * If the specified child object is not a child object of this object,
 * this routine will call Fatal() in Debug compiles.
 *
 * The child object will be removed, and greater child objects will shift
 * down one index position.
 *
 * @param childObjectToRemove  The child object to remove
 */

void Object::removeChildObject(Object * childObjectToRemove, DetachFlags detachFlags) const
{
	DEBUG_FATAL(!childObjectToRemove || (childObjectToRemove->getAttachedTo() != this), ("Removing a child that is not attached to this object."));
	childObjectToRemove->detachFromObject(detachFlags);
}

// ----------------------------------------------------------------------
/**
 * Get the object at the top level of the object hierarchy
 *
 * @return the object at the top of the hierarchy or the current object if the object is at the top of the hierarchy
 * @todo   don't make this routine recursive
 */

Object *Object::getRootParent(void)
{
	DEBUG_FATAL(m_childObject && m_attachedToObject == nullptr, ("am a child but attached to is nullptr"));
	return m_childObject ? NON_NULL(m_attachedToObject)->getRootParent() : this;
}

// ----------------------------------------------------------------------
/**
 * Get the object at the top level of the object hierarchy
 *
 * @return the object at the top of the hierarchy or the current object if the object is at the top of the hierarchy
 * @todo   don't make this routine recursive
 */

const Object *Object::getRootParent(void) const
{
	DEBUG_FATAL(m_childObject && m_attachedToObject == nullptr, ("am a child but attached to is nullptr"));
	return m_childObject ? NON_NULL(m_attachedToObject)->getRootParent() : this;
}

// ----------------------------------------------------------------------
/**
 * Move the object in it's parent space.
 *
 * This routine moves the object in it's parent space, or the world space if
 * the object has no parent. Therefore, moving along the Z axis will move the
 * object forward along the Z-axis of it's parent space, not forward in the
 * direction in which it is pointed.
 *
 * @param vector  Offset to move in parent space
 * @see Object::move_o()
 */

void Object::move_p(const Vector &vector)
{
	if (vector != Vector::zero)
	{
		const Vector oldPosition = getPosition_p();
		m_objectToParent.move_p(vector);
		positionChanged(false, oldPosition);
	}
}

// ----------------------------------------------------------------------

void Object::setRegionOfInfluenceEnabled(bool enabled) const
{
	UNREF(enabled);
}

// ----------------------------------------------------------------------
/**
 * Called after the specified object has its containedByProperty changed
 *
 * This default implementation does nothing.
 *
 * Each object is assumed to have no more than one container associated
 * with it; therefore, it is unambiguous which container is under
 * consideration for this object.
 *
 * @param oldValue  The container that used to hold this object
 * @param newValue  The container that now holds this object.
 */


void Object::containedByModified(const NetworkId & oldValue, const NetworkId & newValue,bool isLocal)
{
	UNREF (oldValue);
	UNREF (newValue);
	UNREF (isLocal);
}

// ----------------------------------------------------------------------
/**
 * Called after the specified object has its slotted arrangement changed
 *
 * This default implementation does nothing.
 *
 * Each object is assumed to have no more than one container associated
 * with it; therefore, it is unambiguous which container is under
 * consideration for this object.
 *
 * @param oldValue  The old arrangement
 * @param newValue  The new arrangement
 */

void Object::arrangementModified(int oldValue, int newValue, bool isLocal)
{
	UNREF (oldValue);
	UNREF (newValue);
	UNREF (isLocal);
}

// ----------------------------------------------------------------------
/**
 * Set the scale for this object.
 *
 * @param scale  New scale for this object
 */

void Object::setScale(const Vector &scale)
{
	m_scale = scale;
	if (m_appearance)
		m_appearance->setScale(scale);

	if (isInWorld())
		extentChanged();
}

// ----------------------------------------------------------------------

void Object::setRecursiveScale(Vector const & scale)
{
	setScale(scale);
	int const numberOfChildObjects = getNumberOfChildObjects();
	for (int i = 0; i < numberOfChildObjects; ++i)
	{
		Object * const child = getChildObject(i);
		if (child != 0)
		{
			child->setRecursiveScale(scale);
		}
	}
}

//-----------------------------------------------------------------------

Controller* Object::stealController(void)
{
	Controller* returnValue = m_controller;
	m_controller = nullptr;
	return returnValue;
}

// ----------------------------------------------------------------------
/**
 * Transform sphere from the world frame to the object's frame.
 *
 * @param sphere  Vector to rotate and translate from world space into object space
 * @return The source world-space sphere transformed into object space
 */

const Sphere Object::rotateTranslate_o2w(const Sphere &sphere) const
{
	return Sphere(getTransform_o2w().rotateTranslate_l2p(sphere.getCenter()), sphere.getRadius());
}

// ----------------------------------------------------------------------
/**
 * Transform sphere from the world frame to the object's frame.
 *
 * @param sphere  Vector to rotate and translate from world space into object space
 * @return The source world-space sphere transformed into object space
 */

const Sphere Object::rotateTranslate_w2o(const Sphere &sphere) const
{
	return Sphere(getTransform_o2w().rotateTranslate_p2l(sphere.getCenter()), sphere.getRadius());
}

// ----------------------------------------------------------------------
/**
 * Determine if this object has child objects.
 *
 * @return True if the object has one or more child objects, otherwise false.
 */

bool Object::hasChildObjects() const
{
	return (m_attachedObjects && !m_attachedObjects->empty());
}

// ======================================================================

const char *Object::getObjectTemplateName() const
{
	return m_objectTemplate ? m_objectTemplate->getName() : 0;
}

// ----------------------------------------------------------------------

Tag Object::getObjectType() const
{
	return m_objectTemplate ? m_objectTemplate->getId() : 0;
}

// ----------------------------------------------------------------------

void Object::setNetworkId(const NetworkId &networkId)
{
	NetworkIdManager::removeObject(*this);
	m_networkId = networkId;
	NetworkIdManager::addObject(*this);
}

// ----------------------------------------------------------------------

void Object::kill()
{
	m_kill = true;
}

// ----------------------------------------------------------------------

void Object::addProperty(Property &property)
{
	// Default is to add property only while not in world.
	bool const allowWhileInWorld = false;
	addProperty(property, allowWhileInWorld);
}

// ----------------------------------------------------------------------

void Object::addProperty(Property &property, bool allowWhileInWorld)
{
	UNREF(allowWhileInWorld);

	PropertyId propertyId = property.getPropertyId();

	bool const inWorldNow = isInWorld();
	DEBUG_FATAL(getProperty(propertyId),("Cannot add already existing property."));
	DEBUG_FATAL(inWorldNow && !allowWhileInWorld, ("Cannot add property to object already in the world."));
	m_propertyList->push_back(&property);

	if (propertyId == CellProperty::getClassPropertyId() || propertyId == PortalProperty::getClassPropertyId() || propertyId == SlottedContainer::getClassPropertyId() || propertyId == VolumeContainer::getClassPropertyId())
	{
		DEBUG_FATAL(m_containerProperty, ("Object already has a container property"));
		m_containerProperty = safe_cast<Container*>(&property);
	}
	else
	{
		if (propertyId == ContainedByProperty::getClassPropertyId())
		{
			DEBUG_FATAL(m_containedBy, ("Object already has a collision property"));
			m_containedBy = safe_cast<ContainedByProperty*>(&property);
		}
		else
			if (propertyId == CollisionProperty::getClassPropertyId())
			{
				DEBUG_FATAL(m_collisionProperty, ("Object already has a collision property"));
				m_collisionProperty = safe_cast<CollisionProperty*>(&property);
			}
	}

	//-- If this object is already in the world, let the property know about it.
	if (inWorldNow)
		property.addToWorld();

} //lint !e1764 // (Info -- Reference parameter 'property' (line 1717) could be declared const ref) // No, we need a pointer to a non-const Property.

// ----------------------------------------------------------------------

Property const *Object::getProperty(PropertyId const &id) const
{
	++ms_propertySearchStatistics[id];
	++ms_propertySearchesPerFrame;

	for (PropertyList::const_iterator i = m_propertyList->begin(); i != m_propertyList->end(); ++i)
		if ((*i)->getPropertyId() == id)
			return *i;

	return nullptr;
}

// ----------------------------------------------------------------------

Property *Object::getProperty(PropertyId const &id)
{
	++ms_propertySearchStatistics[id];
	++ms_propertySearchesPerFrame;

	for (PropertyList::const_iterator i = m_propertyList->begin(); i != m_propertyList->end(); ++i)
		if ((*i)->getPropertyId() == id)
			return *i;

	return nullptr;
}

// ----------------------------------------------------------------------

void Object::removeProperty(PropertyId const &id)
{
	PropertyList::iterator i = m_propertyList->begin();
	for (; i != m_propertyList->end(); ++i)
		if ((*i)->getPropertyId() == id)
			break;

	if (i != m_propertyList->end())
	{
		IGNORE_RETURN(m_propertyList->erase(i));
	}
	else
	{
		DEBUG_FATAL(true, ("property not found"));
		return;
	}

	if (id == CellProperty::getClassPropertyId() || id == PortalProperty::getClassPropertyId() || id == SlottedContainer::getClassPropertyId() || id == VolumeContainer::getClassPropertyId())
		m_containerProperty = nullptr;
	else
		if (id == ContainedByProperty::getClassPropertyId())
			m_containedBy = nullptr;
		else
			if (id == CollisionProperty::getClassPropertyId())
				m_collisionProperty = nullptr;
}

// ----------------------------------------------------------------------

void Object::lookAt_p (const Vector &position_p)
{
	lookAt_o(rotateTranslate_p2o (position_p));
}

// ----------------------------------------------------------------------

void Object::lookAt_p (const Vector &position_p, const Vector &j_p)
{
	Vector k = position_p - getPosition_p(); // fwd vector in parent space
	if (!k.normalize())
		return;
	Vector i = j_p.cross(k);
	if (!i.normalize())
		return;
	k = i.cross(j_p);
	if (!k.normalize())
		return;
	setTransformIJK_o2p(i, j_p, k);
}

// ----------------------------------------------------------------------

void Object::lookAt_o (const Vector &position_o)
{
	yaw_o (position_o.theta());
	pitch_o (position_o.phi());
}

// ----------------------------------------------------------------------

void Object::lookAt_o (const Vector &position_o, const Vector &j_o)
{
	lookAt_p(rotateTranslate_o2p(position_o), rotate_o2p(j_o));
}

// ----------------------------------------------------------------------
/**
 * Sets the new appearance from the specified appearance path.
 */

void Object::setAppearanceByName(char const *path)
{
	if (path != nullptr)
	{
		if (TreeFile::exists(path))
		{
			// Create the object's new appearance

			Appearance *appearance = AppearanceTemplateList::createAppearance(path);
			
			if (appearance != nullptr) 
			{
				setAppearance(appearance);
			} else {
				DEBUG_WARNING(true, ("Object::setAppearanceByName() - Unable to find the object's appearance because the file does not exist: %s", path));
			}
		}
		else
		{
			DEBUG_WARNING(true, ("Object::setAppearanceByName() - Unable to change the object's appearance because the file does not exist: %s", path));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("Object::setAppearanceByName() - nullptr appearance path specified for object: %s", (getObjectTemplateName() == nullptr) ? "" : getObjectTemplateName()));
	}
}

// ----------------------------------------------------------------------

char const * Object::getAppearanceTemplateName() const
{
	return m_appearance ? m_appearance->getAppearanceTemplateName() : 0;
}

//----------------------------------------------------------------------
/**
* Find the actual world position of any object, regardless of containment.
*
* Note we're trying to get a real world coordinate, so we'll stop looking
* at the parent the second we have an object that's in the world.
*/

const Vector Object::findPosition_w () const
{
	const Object * obj = this;

	const ContainedByProperty * cbp = getContainedByProperty ();

	for ( ; !obj->isInWorld() && cbp; )
	{
		const Object * const parent = cbp->getContainedBy ();

		if (parent)
		{
			obj = parent;
			cbp = obj->getContainedByProperty ();
		}
		else
			break;
	}

	return obj->getPosition_w ();
}

// ----------------------------------------------------------------------

void Object::scheduleForAlter()
{
	if (ms_objectsAlterChildrenAndContents)
	{
		scheduleForAlter_scheduleTopmostWorldParent();
	}
	else
	{
		DEBUG_FATAL(!isInitialized(), ("scheduleForAlter() called on uninitialized object %s", getDebugInformation().c_str()));

		AlterScheduler::submitForAlter(*this);
	}
}

// ----------------------------------------------------------------------

SlottedContainer * Object::getSlottedContainerProperty()
{
	Container *container = getContainerProperty();
	if (container && container->getPropertyId() == SlottedContainer::getClassPropertyId())
		return safe_cast<SlottedContainer *>(container);
	return nullptr;
}

// ----------------------------------------------------------------------

SlottedContainer const * Object::getSlottedContainerProperty() const
{
	Container const *container = getContainerProperty();
	if (container && container->getPropertyId() == SlottedContainer::getClassPropertyId())
		return safe_cast<SlottedContainer const *>(container);
	return nullptr;
}

// ----------------------------------------------------------------------

VolumeContainer * Object::getVolumeContainerProperty()
{
	Container *container = getContainerProperty();
	if (container && container->getPropertyId() == VolumeContainer::getClassPropertyId())
		return safe_cast<VolumeContainer *>(container);
	return nullptr;
}

// ----------------------------------------------------------------------

VolumeContainer const * Object::getVolumeContainerProperty() const
{
	Container const *container = getContainerProperty();
	if (container && container->getPropertyId() == VolumeContainer::getClassPropertyId())
		return safe_cast<VolumeContainer const *>(container);
	return nullptr;
}

// ----------------------------------------------------------------------

CellProperty * Object::getCellProperty()
{
	Container *container = getContainerProperty();
	if (container && container->getPropertyId() == CellProperty::getClassPropertyId())
		return safe_cast<CellProperty *>(container);
	return nullptr;
}

// ----------------------------------------------------------------------

CellProperty const * Object::getCellProperty() const
{
	Container const *container = getContainerProperty();
	if (container && container->getPropertyId() == CellProperty::getClassPropertyId())
		return safe_cast<CellProperty const *>(container);
	return nullptr;
}

// ----------------------------------------------------------------------

PortalProperty * Object::getPortalProperty()
{
	Container *container = getContainerProperty();
	if (container && container->getPropertyId() == PortalProperty::getClassPropertyId())
		return safe_cast<PortalProperty *>(container);
	return nullptr;
}

// ----------------------------------------------------------------------

PortalProperty const * Object::getPortalProperty() const
{
	Container const *container = getContainerProperty();
	if (container && container->getPropertyId() == PortalProperty::getClassPropertyId())
		return safe_cast<PortalProperty const *>(container);
	return nullptr;
}

// ----------------------------------------------------------------------

SpatialSubdivisionHandle* Object::getSpatialSubdivisionHandle ()
{
	return m_spatialSubdivisionHandle;
}

// ----------------------------------------------------------------------

void Object::setSpatialSubdivisionHandle (SpatialSubdivisionHandle* const spatialSubdivisionHandle)
{
	m_spatialSubdivisionHandle = spatialSubdivisionHandle;
}

// ----------------------------------------------------------------------

ClientObject * Object::asClientObject()
{
	return 0;
}

// ----------------------------------------------------------------------

ClientObject const * Object::asClientObject() const
{
	return 0;
}

// ----------------------------------------------------------------------

ServerObject * Object::asServerObject()
{
	return 0;
}

// ----------------------------------------------------------------------

ServerObject const * Object::asServerObject() const
{
	return 0;
}

// ----------------------------------------------------------------------

bool Object::hasScheduleData() const
{
	return (m_scheduleData != nullptr);
}

// ----------------------------------------------------------------------

void Object::createScheduleDataIfNecessary(ScheduleTime initialMostRecentAlterTime)
{
	if (!m_scheduleData)
	{
#if OBJECT_SUPPORTS_DESTROYED_FLAG
		// Do not let a destroyed object get handled by the alter scheduler.
		FATAL(m_isDestroyed, ("createScheduleDataIfNecessary(): trying to add a destroyed object to the alter scheduler."));
#endif

		m_scheduleData = new ScheduleData(initialMostRecentAlterTime);
		AlterScheduler::initializeScheduleTimeMapIterator(*this);
	}
}

// ----------------------------------------------------------------------

Object::ScheduleTime Object::getMostRecentAlterTime() const
{
	DEBUG_FATAL(!m_scheduleData, ("Calling getMostRecentAlterTime() on an object that has no schedule data."));
	return m_scheduleData->getMostRecentAlterTime();
}

// ----------------------------------------------------------------------

void Object::setMostRecentAlterTime(ScheduleTime  mostRecentAlterTime)
{
	DEBUG_FATAL(!m_scheduleData, ("Calling setMostRecentAlterTime() on an object that has no schedule data."));
	m_scheduleData->setMostRecentAlterTime(mostRecentAlterTime);
}

// ----------------------------------------------------------------------

bool Object::isInAlterNextFrameList() const
{
	return (m_scheduleData == nullptr) ? false : (m_scheduleData->getAlterNextFrameNext() != nullptr) || (m_scheduleData->getAlterNextFramePrevious() != nullptr);
}

// ----------------------------------------------------------------------

void Object::insertIntoAlterNextFrameList(Object *afterThisObject)
{
	DEBUG_FATAL(!m_scheduleData, ("Calling insertIntoAlterNextFrameList() on an object that has no schedule data."));
	DEBUG_FATAL(isInAlterNextFrameList(), ("insertIntoAlterNextFrameList(): object id=[%s],template=[%s] already in AlterNextFrame list.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
	NOT_NULL(afterThisObject);
	NOT_NULL(afterThisObject->m_scheduleData);
	DEBUG_FATAL(afterThisObject->m_scheduleData->getAlterNextFrameNext() && !afterThisObject->m_scheduleData->getAlterNextFrameNext()->m_scheduleData, ("Non-nullptr forward link does not have schedule data."));
	DEBUG_FATAL(afterThisObject->m_scheduleData->getAlterNextFrameNext() && (afterThisObject->m_scheduleData->getAlterNextFrameNext()->m_scheduleData->getAlterNextFramePrevious() != afterThisObject), ("List corruption: alter next frame."));

	//-- Get new next and previous object for the list.
	Object *newPrevious = afterThisObject;
	Object *newNext     = afterThisObject->m_scheduleData->getAlterNextFrameNext();

	//-- Handle new previous link.
	m_scheduleData->setAlterNextFramePrevious(newPrevious);
	newPrevious->m_scheduleData->setAlterNextFrameNext(this);

	//-- Handle new next link.
	m_scheduleData->setAlterNextFrameNext(newNext);
	if (newNext)
		newNext->m_scheduleData->setAlterNextFramePrevious(this);

#if AS_USE_HARDCORE_CONTAINER_VALIDATION
	//-- Post assertion.
	DEBUG_FATAL(!AlterScheduler::findObjectInAlterNextFrameList(this), ("insertIntoAlterNextFrameList(): object should be in list but isn't: pointer=[%p],id=[%s],template=[%s].", this, getNetworkId().getValueString().c_str(), getObjectTemplateName()));
#endif
}

// ----------------------------------------------------------------------

void Object::removeFromAlterNextFrameList()
{
	DEBUG_FATAL(!isInAlterNextFrameList(), ("Called Object::removeFromAlterNextFrameList() when not in the list."));
	NOT_NULL(m_scheduleData);

	//-- Get the next and previous for the list.
	Object *newPrevious = m_scheduleData->getAlterNextFramePrevious();
	Object *newNext     = m_scheduleData->getAlterNextFrameNext();

	DEBUG_FATAL(newPrevious && !newPrevious->m_scheduleData, ("Previous node missing schedule data."));
	DEBUG_FATAL(newNext     && !newNext->m_scheduleData,     ("Next node missing schedule data."));

	//-- Handle previous.
	m_scheduleData->setAlterNextFramePrevious(nullptr);
	if (newPrevious)
		newPrevious->m_scheduleData->setAlterNextFrameNext(newNext);


	//-- Handle next.
	m_scheduleData->setAlterNextFrameNext(nullptr);
	if (newNext)
		newNext->m_scheduleData->setAlterNextFramePrevious(newPrevious);


#if AS_USE_HARDCORE_CONTAINER_VALIDATION
	//-- Post assertion.
	DEBUG_FATAL(AlterScheduler::findObjectInAlterNextFrameList(this), ("removeFromAlterNextFrameList(): object shouldn't be in list but is: pointer=[%p],id=[%s],template=[%s].", this, getNetworkId().getValueString().c_str(), getObjectTemplateName()));
#endif
}

// ----------------------------------------------------------------------

int Object::getAlterSchedulePhase() const
{
	DEBUG_FATAL(m_scheduleData == nullptr, ("calling Object::getAlterSchedulePhase() when Object has no schedule data: id=[%s], template=[%s]", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
	return m_scheduleData->getSchedulePhase();
}

// ----------------------------------------------------------------------

void Object::setAlterSchedulePhase(int schedulePhaseIndex)
{
	DEBUG_FATAL(m_scheduleData == nullptr, ("calling Object::setAlterSchedulePhase() when Object has no schedule data: id=[%s], template=[%s]", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
	m_scheduleData->setSchedulePhase(schedulePhaseIndex);
}

// ----------------------------------------------------------------------

bool Object::isInAlterNowList() const
{
	return (m_scheduleData == nullptr) ? false : (m_scheduleData->getAlterNowNext() != nullptr) || (m_scheduleData->getAlterNowPrevious() != nullptr);
}

// ----------------------------------------------------------------------

void Object::insertIntoAlterNowList(Object *afterThisObject)
{
	DEBUG_FATAL(!m_scheduleData, ("Calling insertIntoAlterNowList() on an object that has no schedule data."));
	DEBUG_FATAL(isInAlterNowList(), ("insertIntoAlterNowList(): object id=[%s],template=[%s] already in AlterNow list.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
	NOT_NULL(afterThisObject);
	NOT_NULL(afterThisObject->m_scheduleData);
	DEBUG_FATAL(afterThisObject->m_scheduleData->getAlterNowNext() && !afterThisObject->m_scheduleData->getAlterNowNext()->m_scheduleData, ("Non-nullptr forward link does not have schedule data."));
	DEBUG_FATAL(afterThisObject->m_scheduleData->getAlterNowNext() && (afterThisObject->m_scheduleData->getAlterNowNext()->m_scheduleData->getAlterNowPrevious() != afterThisObject), ("List corruption: alter now."));

	//-- Get new next and previous object for the list.
	Object *newPrevious = afterThisObject;
	Object *newNext     = afterThisObject->m_scheduleData->getAlterNowNext();

	//-- Handle new previous link.
	m_scheduleData->setAlterNowPrevious(newPrevious);
	newPrevious->m_scheduleData->setAlterNowNext(this);

	//-- Handle new next link.
	m_scheduleData->setAlterNowNext(newNext);
	if (newNext)
		newNext->m_scheduleData->setAlterNowPrevious(this);

#if AS_USE_HARDCORE_CONTAINER_VALIDATION
	//-- Post assertion.
	DEBUG_FATAL(!AlterScheduler::findObjectInAlterNowList(this), ("insertIntoAlterNowList(): object should be in list but isn't: pointer=[%p],id=[%s],template=[%s].", this, getNetworkId().getValueString().c_str(), getObjectTemplateName()));
#endif
}

// ----------------------------------------------------------------------

void Object::removeFromAlterNowList()
{
	DEBUG_FATAL(!isInAlterNowList(), ("Called Object::removeFromAlterNowList() when not in the list."));
	NOT_NULL(m_scheduleData);

	//-- Get the next and previous for the list.
	Object *newPrevious = m_scheduleData->getAlterNowPrevious();
	Object *newNext     = m_scheduleData->getAlterNowNext();

	DEBUG_FATAL(newPrevious && !newPrevious->m_scheduleData, ("Previous node missing schedule data."));
	DEBUG_FATAL(newNext     && !newNext->m_scheduleData,     ("Next node missing schedule data."));

	//-- Handle previous.
	m_scheduleData->setAlterNowPrevious(nullptr);
	if (newPrevious)
		newPrevious->m_scheduleData->setAlterNowNext(newNext);

	//-- Handle next.
	m_scheduleData->setAlterNowNext(nullptr);
	if (newNext)
		newNext->m_scheduleData->setAlterNowPrevious(newPrevious);

#if AS_USE_HARDCORE_CONTAINER_VALIDATION
	//-- Post assertion.
	DEBUG_FATAL(AlterScheduler::findObjectInAlterNowList(this), ("removeFromAlterNowList(): object shouldn't be in list but is: pointer=[%p],id=[%s],template=[%s].", this, getNetworkId().getValueString().c_str(), getObjectTemplateName()));
#endif
}

// ----------------------------------------------------------------------

bool Object::isInConcludeList() const
{
	return (m_scheduleData == nullptr) ? false : (m_scheduleData->getConcludeNext() != nullptr) || (m_scheduleData->getConcludePrevious() != nullptr);
}

// ----------------------------------------------------------------------

void Object::insertIntoConcludeList(Object *afterThisObject)
{
	DEBUG_FATAL(!m_scheduleData, ("Calling insertIntoConcludeList() on an object that has no schedule data."));
	DEBUG_FATAL(isInConcludeList(), ("insertIntoConcludeList(): object id=[%s],template=[%s] already in Conclude list.", getNetworkId().getValueString().c_str(), getObjectTemplateName()));
	NOT_NULL(afterThisObject);
	NOT_NULL(afterThisObject->m_scheduleData);
	DEBUG_FATAL(afterThisObject->m_scheduleData->getConcludeNext() && !afterThisObject->m_scheduleData->getConcludeNext()->m_scheduleData, ("Non-nullptr forward link does not have schedule data."));
	DEBUG_FATAL(afterThisObject->m_scheduleData->getConcludeNext() && (afterThisObject->m_scheduleData->getConcludeNext()->m_scheduleData->getConcludePrevious() != afterThisObject), ("List corruption: alter now."));

	//-- Get new next and previous object for the list.
	Object *newPrevious = afterThisObject;
	Object *newNext     = afterThisObject->m_scheduleData->getConcludeNext();

	//-- Handle new previous link.
	m_scheduleData->setConcludePrevious(newPrevious);
	newPrevious->m_scheduleData->setConcludeNext(this);

	//-- Handle new next link.
	m_scheduleData->setConcludeNext(newNext);
	if (newNext)
		newNext->m_scheduleData->setConcludePrevious(this);

#if AS_USE_HARDCORE_CONTAINER_VALIDATION
	//-- Post assertion.
	DEBUG_FATAL(!AlterScheduler::findObjectInConcludeList(this), ("insertIntoConcludeList(): object should be in list but isn't: pointer=[%p],id=[%s],template=[%s].", this, getNetworkId().getValueString().c_str(), getObjectTemplateName()));
#endif
}

// ----------------------------------------------------------------------

void Object::removeFromConcludeList()
{
	DEBUG_FATAL(!isInConcludeList(), ("Called Object::removeFromConcludeList() when not in the list."));
	NOT_NULL(m_scheduleData);

	//-- Get the next and previous for the list.
	Object *newPrevious = m_scheduleData->getConcludePrevious();
	Object *newNext     = m_scheduleData->getConcludeNext();

	DEBUG_FATAL(newPrevious && !newPrevious->m_scheduleData, ("Previous node missing schedule data."));
	DEBUG_FATAL(newNext     && !newNext->m_scheduleData,     ("Next node missing schedule data."));

	//-- Handle previous.
	m_scheduleData->setConcludePrevious(nullptr);
	if (newPrevious)
		newPrevious->m_scheduleData->setConcludeNext(newNext);

	//-- Handle next.
	m_scheduleData->setConcludeNext(nullptr);
	if (newNext)
		newNext->m_scheduleData->setConcludePrevious(newPrevious);

#if AS_USE_HARDCORE_CONTAINER_VALIDATION
	//-- Post assertion.
	DEBUG_FATAL(AlterScheduler::findObjectInConcludeList(this), ("removeFromConcludeList(): object shouldn't be in list but is: pointer=[%p],id=[%s],template=[%s].", this, getNetworkId().getValueString().c_str(), getObjectTemplateName()));
#endif
}

// ----------------------------------------------------------------------

void *Object::getScheduleTimeMapIterator()
{
	DEBUG_FATAL(!m_scheduleData, ("getScheduleTimeMapIterator() called but this object doesn't have schedule data."));
	return &m_scheduleData->getScheduleTimeMapIterator();
}

// ----------------------------------------------------------------------

Object *Object::getNextFromAlterNowList()
{
	DEBUG_FATAL(!m_scheduleData, ("getNextFromAlterNowList() called but this object doesn't have schedule data."));
	return m_scheduleData->getAlterNowNext();
}

// ----------------------------------------------------------------------

Object *Object::getPreviousFromAlterNowList()
{
	DEBUG_FATAL(!m_scheduleData, ("getPreviousFromAlterNowList() called but this object doesn't have schedule data."));
	return m_scheduleData->getAlterNowPrevious();
}

// ----------------------------------------------------------------------

Object *Object::getNextFromAlterNextFrameList()
{
	DEBUG_FATAL(!m_scheduleData, ("getNextFromAlterNextFrameList() called but this object doesn't have schedule data."));
	return m_scheduleData->getAlterNextFrameNext();
}

// ----------------------------------------------------------------------

Object *Object::getPreviousFromAlterNextFrameList()
{
	DEBUG_FATAL(!m_scheduleData, ("getPreviousFromAlterNextFrameList() called but this object doesn't have schedule data."));
	return m_scheduleData->getAlterNextFramePrevious();
}

// ----------------------------------------------------------------------

Object *Object::getNextFromConcludeList()
{
	DEBUG_FATAL(!m_scheduleData, ("getNextFromConcludeList() called but this object doesn't have schedule data."));
	return m_scheduleData->getConcludeNext();
}

// ----------------------------------------------------------------------

Object *Object::getPreviousFromConcludeList()
{
	DEBUG_FATAL(!m_scheduleData, ("getPreviousFromConcludeList() called but this object doesn't have schedule data."));
	return m_scheduleData->getConcludePrevious();
}

// ----------------------------------------------------------------------

#if OBJECT_SUPPORTS_IS_ALTERING_FLAG

void Object::setIsAltering(bool newIsAltering)
{
	DEBUG_FATAL((newIsAltering && m_isAltering) || (!newIsAltering && !m_isAltering), ("Object::setIsAltering(): setting same alter state, likely bad logic flow for object. old/new state is [%s].", m_isAltering ? "true" : "false"));
	m_isAltering = newIsAltering;
}

#endif

// ----------------------------------------------------------------------
/**
 * Alter scheduler scheduleForAlter strategy that goes up the parent
 * chain to the top-most object in the world for alter scheduling.
 *
 * Client-side objects adopt this strategy, including
 * ClientObject and HardpointObject.  Not all client-side objects
 * derive from ClientObject, so we stick this strategy in the base
 * Object class, override the virtual scheduleForAlter(), and forward
 * that function to this one.
 *
 * More details:
 *
 * Ensure this Object instance gets an alter() call via the
 * AlterScheduler the next time alter processing occurs.
 *
 * This function will ensure only the top-most object in the world is
 * added to the AlterScheduler.  We do this because those are the only
 * objects that should receive an alter, as they propagate alter calls
 * to their children and contained items.
 *
 * To find the top-most instance to alter given this instance, first
 * we traverse the parent-child loop looking for the top-most parent.
 * Then we traverse the container-contained hierarchy looking for the
 * first Object that is in the world.  The traversed object that meets
 * the following conditions is put on the AlterScheduler: (1)  does not have
 * a parent object and (2) is not contained or is contained but is in the world.
 */
void Object::scheduleForAlter_scheduleTopmostWorldParent()
{
	Object *alterObject = this;

	do
	{
		//-- Traverse parent links until there is no more parent.
		for (Object *parentObject = alterObject->getParent(); parentObject != nullptr; parentObject = alterObject->getParent())
			alterObject = parentObject;

		//-- Traverse container until we're at a container object that is in the world.
		if (!alterObject->isInWorld())
		{
			//-- Use the first container parent that is in the world.
			ContainedByProperty *containedByProperty = alterObject->getContainedByProperty();
			while (containedByProperty)
			{
				// Get the container object.
				Object *const containerObject = containedByProperty->getContainedBy();
				if (!containerObject)
					break;
				else
				{
					alterObject = containerObject;
					if (alterObject->isInWorld())
					{
						// We're done searching.
						containedByProperty = nullptr;
					}
					else
					{
						// The container object is not in the world, so search for it's container.
						containedByProperty = alterObject->getContainedByProperty();
					}
				}
			}

			//-- If we get here and the alterObject is not in the world, something seems wrong.
			//   This object needs to be altered but doesn't have a root-most object in the world.
			// DEBUG_FATAL(!alterObject->isInWorld(), ("trying to schedule an alter for an object [%s] whose root-most container object [%s] isn't in the world!", getNetworkId().getValueString().c_str(), alterObject->getNetworkId().getValueString().c_str()));
		}

		// New container might be a child object, make sure we test for a parent again.
		// If a parent exists, we need to run through the loop again to find the parent object.
	} while (alterObject->getParent() != nullptr);

	NOT_NULL(alterObject);
	if (alterObject->isInitialized())
		AlterScheduler::submitForAlter(*alterObject);
}

// ----------------------------------------------------------------------

void Object::nullWatchers()
{
	m_watchedByList.nullWatchers();
}

// ----------------------------------------------------------------------

AxialBox const ObjectNamespace::getTangibleExtentInternal(Object const &obj)
{
	AxialBox result;

	//-- If this has an applicable appearance, the result begins with this
	Appearance const * const appearance = obj.getAppearance();
	if (appearance)
		result = appearance->getTangibleExtent();

	//-- Iterate through all of the children and grow the result axial box
	int const numberOfChildObjects = obj.getNumberOfChildObjects();
	for (int i = 0; i < numberOfChildObjects; ++i)
	{
		Object const * const childObject = obj.getChildObject(i);
		if (!childObject || !childObject->isChildObject())
			continue;

		AxialBox const axialBox = getTangibleExtentInternal(*childObject);

		//-- Transform box from child space into parent space and grow result
		if (!axialBox.isEmpty())
			for (int j = 0; j < 8; ++j)
				result.add(childObject->getTransform_o2p().rotateTranslate_l2p(axialBox.getCorner(j)));
	}

	return result;
}

// ----------------------------------------------------------------------

AxialBox const Object::getTangibleExtent() const
{
	AxialBox result = getTangibleExtentInternal(*this);

	// Return a zero sized box at the appearance center if there is no other
	// tangible extent.  If there is no appearance this centers at the local
	// origin.
	if (result.isEmpty())
		result.add(getAppearanceSphereCenter());

	return result;
}

// ----------------------------------------------------------------------

bool Object::getUseAlterScheduler() const
{
	return m_useAlterScheduler;
}

// ----------------------------------------------------------------------

void Object::setUseAlterScheduler(bool const useAlterScheduler) 
{
	m_useAlterScheduler = useAlterScheduler;
}

// ----------------------------------------------------------------------

void Object::setObjectToWorldDirty(bool const objectToWorldDirty) const
{
#if ENABLE_OBJECTTOWORLDDIRTY == 1
	for (int i = 0; i < getNumberOfAttachedObjects(); ++i)
		getAttachedObject(i)->setObjectToWorldDirty(true);
#endif

	m_objectToWorldDirty = objectToWorldDirty;
}

// ----------------------------------------------------------------------

Sphere const Object::getCollisionSphereExtent_o() const
{
	Sphere sphere_o(Sphere::unit);

	CollisionProperty const * const collisionProperty = getCollisionProperty();

	if (collisionProperty != 0)
	{
		sphere_o = collisionProperty->getBoundingSphere_l();
	}
	else
	{
		sphere_o.setRadius(1.0f);
	}

	return sphere_o;
}

// ----------------------------------------------------------------------

Sphere const Object::getCollisionSphereExtent_w() const
{
	Sphere const sphere_o(getCollisionSphereExtent_o());
	Sphere sphere_w;

	sphere_w.setCenter(rotateTranslate_o2w(sphere_o.getCenter()));
	sphere_w.setRadius(sphere_o.getRadius());

	return sphere_w;
}

// ----------------------------------------------------------------------

float Object::getDistanceBetweenCollisionSpheres_w(Object const & object) const
{
	float distance = 0.0f;

	if (&object != this)
	{
		Vector const & a_w = getPosition_w();
		Vector const & b_w = object.getPosition_w();

		distance = a_w.magnitudeBetween(b_w);
		distance -= getCollisionSphereExtent_o().getRadius();
		distance -= object.getCollisionSphereExtent_o().getRadius();
		distance = std::max(0.0f, distance);
	}

	return distance;
}

// ----------------------------------------------------------------------

void Object::getObjectInfo(std::map<std::string, std::map<std::string, Unicode::String> > & propertyMap) const
{
/**
  When adding a variable to this class, please add it here.  Variable that aren't easily displayable are still listed, for tracking purposes.
*/

/**
	Don't compile in production build because this maps human-readable values to data members and makes hacking easier
*/
#if PRODUCTION == 0

	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "InWorld", m_inWorld);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Active", m_active);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Kill", m_kill);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Authoritative", m_authoritative);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ChildObject", m_childObject);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ObjectToWorldDirty", m_objectToWorldDirty);
#if OBJECT_SUPPORTS_DESTROYED_FLAG
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "IsDestroyed", m_isDestroyed);
#endif
#if OBJECT_SUPPORTS_IS_ALTERING_FLAG
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "IsAltering", m_isAltering);
#endif
	if(getObjectTemplateName())
	{
		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ObjectTemplateName", getObjectTemplateName());
	}
	else
	{
		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ObjectTemplateName", "Unknown");
	}
//const NotificationList   *m_notificationList;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "DebugName", m_debugName);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "NetworkId", m_networkId.getValueString());
	if(m_objectTemplate)
	{
		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AppearanceTemplateName", getAppearanceTemplateName());
	}
	else
	{
		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AppearanceTemplateName", "Unknown");
	}
//Controller               *m_controller;
//Dynamics                 *m_dynamics;

	if(m_attachedToObject)
		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AttachedToObject", m_attachedToObject->getNetworkId().getValueString());
	else
		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "AttachedToObject", "");

//AttachedObjects          *m_attachedObjects;
//DpvsObjects              *m_dpvsObjects;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Rotations", m_rotations);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "Scale", m_scale);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ObjectToParent", m_objectToParent);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ObjectToWorld", m_objectToWorld);
//mutable WatchedByList     m_watchedByList;
//Container                *m_containerProperty;
//CollisionProperty        *m_collisionProperty;
//SpatialSubdivisionHandle *m_spatialSubdivisionHandle;
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "UseAlterScheduler", m_useAlterScheduler);
//ScheduleData             *m_scheduleData;

	if(m_containedBy)
		DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "ContainedBy", m_containedBy->getContainedByNetworkId().getValueString());
#else
	UNREF(propertyMap);
#endif
}

// ----------------------------------------------------------------------

void Object::setAlternateAppearance(const char * path)
{

	if(!path)
		return;
	
	Appearance *alternateAppearance = nullptr;
	
	if (TreeFile::exists(path))
	{	
		alternateAppearance = AppearanceTemplateList::createAppearance(path);

		if (alternateAppearance == nullptr) {
			DEBUG_WARNING(true, ("Object::setAlternateAppearance() - Unable to change the object's appearance because the file does not exist: %s", path));
			return;
		}
	}
	else
	{
		return;
	}

	bool const instantlyApplyAppearance = isUsingAlternateAppearance();

	if(instantlyApplyAppearance)
	{
		if(isInWorld() && m_appearance)
			m_appearance->removeFromWorld();
	}

	if (m_alternateAppearance == alternateAppearance)
	{
		delete alternateAppearance;
		return;
	}


	if (m_alternateAppearance) // delete the old appearance
	{
		// DBE - Copy the transform from the old appearance to the new one.
		// This was added to mimic the behavior of when the appearance-to-world 
		// transform was stored in Object instead of Appearance.
		if (alternateAppearance)
		{
			alternateAppearance->setTransform_w(m_alternateAppearance->getTransform_w());
		}

		delete m_alternateAppearance;

		m_alternateAppearance = nullptr;
	}
	else
	{
		// DBE - Initialize the transform of the new appearance to our object-to-world
		// transform.  This was added to mimic the behavior of when the appearance-to-world 
		// transform was stored in Object instead of Appearance.
		alternateAppearance->setTransform_w(getTransform_o2w());
	}

	// start using the new appearance
	m_alternateAppearance = alternateAppearance;

	if(instantlyApplyAppearance)
	{
		m_alternateAppearance->setOwner(this);
		m_alternateAppearance->setScale(m_scale);

		if (isInWorld())
		{
			m_alternateAppearance->addToWorld();
		}

		m_appearance = m_alternateAppearance;
	}
	else
	{
		useAlternateAppearance();
	}

	DEBUG_REPORT_LOG(false, ("Successfully set the alternate appearance for [%s] to [%s]\n", getNetworkId().getValueString().c_str(), path));
}

// ----------------------------------------------------------------------

void Object::useAlternateAppearance()
{
	if(!m_alternateAppearance || m_alternateAppearance == m_appearance)
		return;

	if(isInWorld())
		m_appearance->removeFromWorld();
	
	m_alternateAppearance->setOwner(this);
	m_alternateAppearance->setScale(m_scale);


	if (isInWorld())
	{
		m_alternateAppearance->addToWorld();
	}

	m_appearance = m_alternateAppearance;

	DEBUG_REPORT_LOG(false, ("Object [%s] changed to their alternate appearance.\n", getNetworkId().getValueString().c_str()));

}

// ----------------------------------------------------------------------

void Object::useDefaultAppearance()
{
	if(!m_defaultAppearance ||  m_defaultAppearance == m_appearance)
		return;

	if(m_appearance)
		m_appearance->removeFromWorld();

	m_defaultAppearance->setOwner(this);
	m_defaultAppearance->setScale(m_scale);

	if (isInWorld())
	{
		m_defaultAppearance->addToWorld();
	}

	m_appearance = m_defaultAppearance;

	DEBUG_REPORT_LOG(false, ("Object [%s] changed to their default appearance.\n", getNetworkId().getValueString().c_str()));
}
// ======================================================================
