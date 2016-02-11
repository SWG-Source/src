// ======================================================================
//
// World.cpp
// asommers 2-26-99
//
// copyright 1999, bootprint entertainment
// copyright 2001, sony online entertainment
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/World.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMessageDispatch/Emitter.h"
#include "sharedMessageDispatch/Message.h"
#include "sharedObject/AlterScheduler.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ConfigSharedObject.h"
#include "sharedObject/LotManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectList.h"
#include "sharedTerrain/TerrainObject.h"

#include <string>
#include <vector>

// ======================================================================
// World::Emitter
// ======================================================================

struct World::Emitter : public MessageDispatch::Emitter
{
public:
	
	typedef MessageDispatch::Message<std::pair<NetworkId, int> > ObjectMessage; //lint -esym(754, Emitter::ObjectMessage) // (Info -- local structure member 'Emitter::ObjectMessage' not referenced) // It is referenced, Lint is incorrect.
	~Emitter ();
};

// ----------------------------------------------------------------------

World::Emitter::~Emitter ()
{
}

// ======================================================================
// WorldNamespace
// ======================================================================

namespace WorldNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool              ms_installed;
	int               ms_frameNumber;

#ifdef _DEBUG
	bool              ms_debugReport;
	const char* const cms_objectListNames [WOL_Count] = 
	{
		"         WOL_Tangible",
		"WOL_TangNotTargetable",
		"       WOL_Intangible",
		"    WOL_TangibleFlora",
		"         WOL_Inactive"
	};

	bool              ms_logUninitializedObjects = false;

#endif

	ObjectList*       ms_objectList [WOL_Count];
	ObjectList*       ms_queuedObjectList;
	LotManager*       ms_lotManager;

	World::Emitter    ms_emitter;

	bool ms_valid = false;

	typedef std::set<Object const *> ObjectSet;
	ObjectSet ms_objectSet;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace WorldNamespace;

// ======================================================================
// PUBLIC STATIC World
// ======================================================================

const char* const World::Messages::OBJECT_ADDED     = "World::ObjectAdded";
const char* const World::Messages::OBJECT_REMOVED   = "World::ObjectRemoved";
const char* const World::Messages::OBJECT_REMOVING  = "World::ObjectRemoving";
const char* const World::Messages::OBJECT_ADDED_PTR = "World::ObjectAddedPtr";

// ======================================================================

int World::getFrameNumber ()
{
	return ms_frameNumber;
}

// ----------------------------------------------------------------------

const LotManager* World::getConstLotManager ()
{
	return ms_lotManager;
}

// ----------------------------------------------------------------------

LotManager* World::getLotManager ()
{
	return ms_lotManager;
}

// ----------------------------------------------------------------------

void World::snapAllObjectsToTerrain ()
{
	int i;
	for (i = static_cast<int>(WOL_MarkerObjectsStart); i < static_cast<int>(WOL_MarkerObjectsEnd); ++i)
		snapObjectsToTerrain (i);
}

// ----------------------------------------------------------------------

void World::snapObjectsToTerrain (const int listIndex)
{
	int i;
	for (i = 0; i < getNumberOfObjects (listIndex); i++)
		snapObjectToTerrain (getObject (listIndex, i));
}

// ----------------------------------------------------------------------

void World::snapObjectToTerrain (Object* object, bool alignToTerrain)
{
	if (!TerrainObject::getInstance ())
		return;

	NOT_NULL (object);

	if (!object->getObjectTemplate ())
		IGNORE_RETURN(TerrainObject::getInstance ()->placeObject (*object, alignToTerrain));
	else
	{
		SharedObjectTemplate const * const sharedObjectTemplate = object->getObjectTemplate() ? object->getObjectTemplate()->asSharedObjectTemplate() : 0;
		if (sharedObjectTemplate && sharedObjectTemplate->getSnapToTerrain ())
			IGNORE_RETURN(TerrainObject::getInstance ()->placeObject (*object, alignToTerrain, true));
	}
}

// ======================================================================
// PROTECTED STATIC World
// ======================================================================

void World::install ()
{
	DEBUG_FATAL (ms_installed, ("World::install - already installed"));
	ms_installed = true;

	//-- create the object lists
	int i;
	for (i = 0; i < static_cast<int>(WOL_Count); i++)
		ms_objectList [i] = new ObjectList (100);

	ms_queuedObjectList = new ObjectList (100);

#ifdef _DEBUG
	DebugFlags::registerFlag (ms_debugReport, "SharedObject", "reportWorld", debugReport);
	DebugFlags::registerFlag (ms_logUninitializedObjects, "SharedObject", "logWorldUninitializedObjects");
#endif

	ms_lotManager = new LotManager (16384.f, 8.f);

	ms_frameNumber = 0;

	//-- tell the appearance template list we allow timed templates
	AppearanceTemplateList::setAllowTimedTemplates (true);

	ms_valid = true;
}

// ----------------------------------------------------------------------

bool World::isValid ()
{
	return ms_valid;
}

// ----------------------------------------------------------------------

bool World::isInstalled()
{
	return ms_installed;
}

// ----------------------------------------------------------------------

void World::remove ()
{
	DEBUG_FATAL (!ms_installed, ("World::install - not installed"));

	ms_valid = false;

	//-- tell the appearance template list we no longer allow timed templates (this will delete all exiting timed templates)
	AppearanceTemplateList::setAllowTimedTemplates (false);

	//-- delete all objectLists
	{
		for (int i = 0; i < static_cast<int>(WOL_Count); ++i)
		{
			ObjectList* const objectList = ms_objectList [i];

			while (!objectList->isEmpty())
			{
				int const lastObjectIndex = objectList->getNumberOfObjects() - 1;
				Object *const object = objectList->getObject(lastObjectIndex);
				objectList->removeObjectByIndex(object, lastObjectIndex);

				// do not delete contained objects, since their container will delete them
				if (!object->getAttachedTo())
					delete object;
			}

			delete ms_objectList [i];
			ms_objectList [i] = 0;
		}
	}

	//-- delete queuedObject list
	{
		while (!ms_queuedObjectList->isEmpty ())
		{
			int const lastObjectIndex = ms_queuedObjectList->getNumberOfObjects() - 1;
			Object *const object = ms_queuedObjectList->getObject(lastObjectIndex);
			ms_queuedObjectList->removeObjectByIndex(object, lastObjectIndex);

			delete object;
		}

		delete ms_queuedObjectList;
		ms_queuedObjectList = 0;
	}

	delete ms_lotManager;
	ms_lotManager = 0;

#ifdef _DEBUG
	if (!ms_objectSet.empty())
	{
		DEBUG_REPORT_LOG(true, ("World::remove: The object set should be empty.  The following objects have leaked:\n"));
		ObjectSet::const_iterator end = ms_objectSet.end();
		for (ObjectSet::const_iterator iter = ms_objectSet.begin(); iter != end; ++iter)
		{
			Object const * const object = *iter;
			DEBUG_REPORT_LOG(true, ("- %s\n", object->getDebugInformation(true).c_str()));
		}
	}

	DebugFlags::unregisterFlag(ms_debugReport);
	DebugFlags::unregisterFlag(ms_logUninitializedObjects);
#endif

	ms_installed = false;
}

// ----------------------------------------------------------------------

void World::addObject (Object* object, int listIndex)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (object);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, listIndex, static_cast<int>(WOL_Count));
	DEBUG_FATAL (existsInList (object, listIndex), ("object already in list"));
	DEBUG_FATAL (existsInWorld (object), ("object already in world"));

	ms_objectSet.insert(object);
	ms_objectList [listIndex]->addObject (object);

	//-- notification of addition is guaranteed to occur AFTER the object is actually added
	{
		Emitter::ObjectMessage const msg(Messages::OBJECT_ADDED, std::make_pair(object->getNetworkId(), listIndex));
		ms_emitter.emitMessage(msg);
	}
	{
		MessageDispatch::Message<Object *> const msg(Messages::OBJECT_ADDED_PTR, object);
		ms_emitter.emitMessage(msg);
	}

	//-- Add the object to the AlterScheduler.  This is the way that the system
	//   gives an object its first alter.
	if (object->isInitialized())
		AlterScheduler::submitForAlter(*object);
	else
		DEBUG_REPORT_LOG(ms_logUninitializedObjects, ("World::addObject(): skipping AlterScheduler::submitForAlter() on uninitialized object:pointer=[%p],id=[%s],template=[%s].\n", object, object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));
}

// ----------------------------------------------------------------------

bool World::removeObject (const Object* object, int listIndex)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	NOT_NULL (object);
	DEBUG_FATAL (!object, ("World::removeObject - object is nullptr"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, listIndex, static_cast<int>(WOL_Count));

	ms_objectSet.erase(object);

	bool result;
	int  index = 0;
	if (ms_objectList [listIndex]->find (object, &index))
	{
		const NetworkId id = object->getNetworkId();

		{
			Emitter::ObjectMessage const msg(Messages::OBJECT_REMOVING, std::make_pair(id, listIndex));
			ms_emitter.emitMessage(msg);
		}

		ms_objectList [listIndex]->removeObjectByIndex (object, index);

		{
			Emitter::ObjectMessage const msg(Messages::OBJECT_REMOVED, std::make_pair(id, listIndex));
			ms_emitter.emitMessage(msg);
		}

		result = true;
	}
	else
		result = false;

	//-- Let the alter scheduler know this object should no longer be altered as if in the world.  
	//   WARNING: do not remove this line! When objects are transferred from the world into a container,
	//            this line prevents a double alter.  The double alter would have come from those objects 
	//            both (1) remaining in the alter scheduler from previously being added to the world and 
	//            (2)then altered by the container.
	if (Object::shouldObjectsAlterChildrenAndContents())
		IGNORE_RETURN(AlterScheduler::removeObject(*const_cast<Object*>(object)));

	return result;
}

// ----------------------------------------------------------------------

void World::queueObject  (Object* object)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	DEBUG_FATAL (!object, ("World::queueObject - object is nullptr"));

	ms_queuedObjectList->addObject (object);
}

// ----------------------------------------------------------------------

int World::getNumberOfObjects (int listIndex)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, listIndex, static_cast<int>(WOL_Count));
	
	return ms_objectList [listIndex]->getNumberOfObjects ();
}

// ----------------------------------------------------------------------

const Object* World::getConstObject (int listIndex, int index)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, listIndex, static_cast<int>(WOL_Count));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, ms_objectList [listIndex]->getNumberOfObjects ());

	return ms_objectList [listIndex]->getObject (index);
}

// ----------------------------------------------------------------------

Object* World::getObject (int listIndex, int index)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, listIndex, static_cast<int>(WOL_Count));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, ms_objectList [listIndex]->getNumberOfObjects ());

	return ms_objectList [listIndex]->getObject (index);
}

// ----------------------------------------------------------------------

int World::getNumberOfQueuedObjects ()
{
	DEBUG_FATAL (!ms_installed, ("not installed"));

	return ms_queuedObjectList->getNumberOfObjects ();
}

// ----------------------------------------------------------------------

Object* World::getQueuedObject (const int index)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));

	return ms_queuedObjectList->getObject (index);
}

// ----------------------------------------------------------------------

const Object* World::getConstQueuedObject (const int index)
{
	return getQueuedObject (index);
}

// ----------------------------------------------------------------------

const Object* World::findNextObject (const Object* object, int listIndex)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, listIndex, static_cast<int>(WOL_Count));

	//-- if there are no objects, return 0
	if (!ms_objectList [listIndex]->getNumberOfObjects ())
		return 0;

	//-- if object is 0, return the first object (we are guaranteed at this point that the list has objects)
	if (!object)
		return ms_objectList [listIndex]->getObject (0);

	//-- search for object in list
	int i;
	for (i = 0; i < ms_objectList [listIndex]->getNumberOfObjects (); i++)
		if (ms_objectList [listIndex]->getObject (i) == object)
			break;

	//-- if object was not found in the list, return the first object
	if (i == ms_objectList [listIndex]->getNumberOfObjects ())
		return ms_objectList [listIndex]->getObject (0);

	//-- return the next object
	i = (i + 1) % ms_objectList [listIndex]->getNumberOfObjects ();
	return ms_objectList [listIndex]->getObject (i);
}

// ----------------------------------------------------------------------

int World::findObjectList(Object const * const object)
{
	int listIndex = WOL_MarkerAllObjectListsStart;
	while (listIndex < WOL_MarkerAllObjectListsEnd)
	{
		if (existsInList(object, listIndex))
			break;

		++listIndex;
	}

	return listIndex;
}

// ----------------------------------------------------------------------

Object* World::findClosestObjectTo (const Object* object, int listIndex)
{
	DEBUG_FATAL (!ms_installed, ("not installed"));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, listIndex, static_cast<int>(WOL_Count));

	NOT_NULL (object);

	Object* closestObject    = 0;
	float   shortestDistance = REAL_MAX;

	int i;
	for (i = 0; i < ms_objectList [listIndex]->getNumberOfObjects (); i++)
	{
		if (object != ms_objectList [listIndex]->getObject (i))
		{
			Object* const objectToCheck = ms_objectList [listIndex]->getObject (i);
			const Vector  position1 = object->getAppearance () ? object->getAppearanceSphereCenter_w () : object->getPosition_w ();
			const Vector  position2 = objectToCheck->getAppearance () ? objectToCheck->getAppearanceSphereCenter_w () : objectToCheck->getPosition_w ();
			const float   magnitudeBetweenSquared = position1.magnitudeBetweenSquared (position2);

			if (magnitudeBetweenSquared < shortestDistance)
			{
				shortestDistance = magnitudeBetweenSquared;
				closestObject    = objectToCheck;
			}
		}
	}

	return closestObject;
}

// ----------------------------------------------------------------------

const Object* World::findClosestConstObjectTo (const Object* object, int listIndex)
{
	return findClosestObjectTo (object, listIndex);
}

// ----------------------------------------------------------------------

void World::validate ()
{
	for (int i = 0; i < static_cast<int> (WOL_Count); ++i)
	{
		for (int j = 0; j < getNumberOfObjects (i); ++j)
		{
			const Object* const object = getObject (i, j);

			if (!object->isInWorld ())
				DEBUG_FATAL (true, ("World::validate - object [%s] in world object list %i wasn't in the world", object->getNetworkId ().getValueString ().c_str (), i));
		}
	}
}

// ----------------------------------------------------------------------

void World::beginFrame ()
{
	DEBUG_FATAL (!ms_installed, ("not installed"));

	if (ConfigSharedObject::getValidateWorld ())
		validate();

	//-- add any queued objects to the world
	{
		for (int i = 0; i < ms_queuedObjectList->getNumberOfObjects (); ++i)
			ms_queuedObjectList->getObject (i)->addToWorld();

		ms_queuedObjectList->removeAll ();
	}
}

// ----------------------------------------------------------------------

void World::endFrame ()
{
	++ms_frameNumber;
}

// ----------------------------------------------------------------------

const MessageDispatch::Emitter& World::getEmitter ()
{
	DEBUG_FATAL (!ms_installed, ("not installed\n"));
	return ms_emitter;
}

// ----------------------------------------------------------------------

bool World::existsInList (const Object* object, int listIndex)
{
	int index;
	return ms_objectList [listIndex]->find (object, &index);
}

// ----------------------------------------------------------------------

bool World::existsInWorld (const Object* object)
{
	return ms_objectSet.find(object) != ms_objectSet.end();
}

// ======================================================================
// PRIVATE STATIC World
// ======================================================================

void World::debugReport ()
{
	if (!ms_installed)
		return;

	DEBUG_REPORT_PRINT (true, ("-- World\n"));
	for (int i = 0; i < static_cast<int> (WOL_Count); i++)
		DEBUG_REPORT_PRINT (true, ("[%s] %4i objects\n", cms_objectListNames [i], World::getNumberOfObjects (i)));
}

// ======================================================================
