// ======================================================================
//
// AlterScheduler.cpp
// Copyright 2002-2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/AlterScheduler.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/ConfigSharedObject.h"
#include "sharedObject/Object.h"
#include "sharedSynchronization/Mutex.h"

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <string>
#include <typeinfo>
#include <vector>

#if _MSC_VER < 1300
using std::type_info;
#endif

// ======================================================================

#ifdef _DEBUG
	 // Set this flag to 1 to do additional (slow) error checking.
#  define VALIDATE_OBJECTS             1
#  define   USE_TYPEINFO_CATCH_BLOCK   1
#  define DO_RECURSION_CHECK           1
#  define DO_ON_DEBUG(op)              op
#else
#  define VALIDATE_OBJECTS             1
#  define   USE_TYPEINFO_CATCH_BLOCK   1
#  define DO_RECURSION_CHECK           0
#  define DO_ON_DEBUG(op)              NOP
#endif

#define OBJECT_SCHEDULE_TIME_MAP_ITERATOR(objectReference) \
	*(static_cast<AlterScheduler::ScheduleTimeMap::iterator*>((objectReference).getScheduleTimeMapIterator()))

#if AS_USE_HARDCORE_CONTAINER_VALIDATION
#  define DO_ON_HARDCORE_VALIDATION(op) op
#else
#  define DO_ON_HARDCORE_VALIDATION(op) NOP
#endif

#if VALIDATE_OBJECTS
#  define DO_ON_VALIDATE_OBJECTS(op) op
#else
#  define DO_ON_VALIDATE_OBJECTS(op) NOP
#endif

#if OBJECT_SUPPORTS_IS_ALTERING_FLAG
#  define DO_ON_OBJECT_ALTER_FLAG_SUPPORTED(op)  op
#else
#  define DO_ON_OBJECT_ALTER_FLAG_SUPPORTED(op)  NOP
#endif

// ======================================================================

namespace AlterSchedulerNamespace
{
	typedef AlterScheduler::ScheduleTime     ScheduleTime;
	typedef AlterScheduler::ScheduleTimeMap  ScheduleTimeMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class ObjectInfo
	{
		MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

	public:

		explicit ObjectInfo(Object &object);
		~ObjectInfo();

		Object const *getObject() const;
		char const   *getNetworkIdString() const;
		char const   *getObjectTemplateName() const;

	private:
	
		// Disabled.
		ObjectInfo();

	private:

		Object *m_object;
		char   *m_networkIdString;
		char   *m_objectTemplateName;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::map<Object*, ObjectInfo*>  ObjectInfoMap;
	typedef std::set<Object*>               ObjectSet;
	typedef std::list<Object*>              ObjectList;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	float const  cs_schedulerTicksPerSecond = 1000.0f;  // # schedule ticks per second.  Frame rates will not be able to exceed this.
	float const  cs_secondsPerSchedulerTick = 1.0f / cs_schedulerTicksPerSecond;

	uint32 const cs_freeFillPattern         = 0xEFEFEFEF; // this should match MemoryManager's free fill pattern.
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// Sentinel/guard nodes for first entry in these two lists.
	Object                             *s_alterNextFrameListFirst[AS_MAX_SCHEDULE_PHASE_COUNT];
	Object                             *s_alterNowListFirst;
	Object                             *s_concludeListFirst;
	bool                                s_areListsValid;

	ScheduleTime                        s_currentTime;
	float                               s_schedulerElapsedTime;
	int                                 s_objectsAltered;
	int                                 s_objectsNotAltered;
	bool                                s_report;
	bool                                s_suspendExecution;
	bool                                s_alwaysAlter;

	bool                                s_trackObjectInfo;
	ObjectInfoMap                       s_objectInfoMap;

#if DO_RECURSION_CHECK
	Mutex                              *s_criticalSection;
	int                                 s_recursionDepth;
#endif

#ifdef _DEBUG
	bool                                s_logAddRemove;
	bool                                s_reportPerObject;
	bool                                s_profileAlterByType;
	Object const *                      s_currentlyAlteringObject;
	int                                 s_totalObjectAlterCalls;
	int                                 s_reportedObjectAlterCalls;
	char const *                        s_reportedAppearanceType;
	float                               s_reportedAppearanceResult;
	char const *                        s_reportedControllerType;
	float                               s_reportedControllerResult;
	char const *                        s_reportedDynamicsType;
	float                               s_reportedDynamicsResult;
	float                               s_reportedChildrenResult;
	float                               s_reportedObjectResult;
#endif

	char                                s_crashReportInfo[MAX_PATH * 2];


	ScheduleTimeMap                     s_scheduleMap;  // Sorted by next alter time for efficient scheduling.

	AlterScheduler::PostAlterHookFunction s_postAlterHookFunction;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  remove();
	void  reportPrint();
	void  validateObject(Object const* object);
	void  addObjectInfoAsNecessary(Object &object);

#ifdef _DEBUG
	void  doPerObjectAlterReportCollection(Object const* object, float alterResult);
#endif

#if DO_RECURSION_CHECK
	void  incrementRecursionDepth();
	void  decrementRecursionDepth();
	int   getRecursionDepth();

	void  doPreAlterRecursionCheck();
	void  doPostAlterRecursionCheck();
#else
	#define doPreAlterRecursionCheck()  NOP
	#define doPostAlterRecursionCheck() NOP
#endif

	void  incrementSchedulerTimerByElapsedTime(float schedulerElapsedTime);
}

using namespace AlterSchedulerNamespace;

// ======================================================================
// class AlterSchedulerNamespace::ObjectInfo
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(AlterSchedulerNamespace::ObjectInfo, true, 0, 0, 0);

// ======================================================================

AlterSchedulerNamespace::ObjectInfo::ObjectInfo(Object &object):
	m_object(&object),
	m_networkIdString(DuplicateString(object.getNetworkId().getValueString().c_str())),
	m_objectTemplateName(DuplicateString(object.getObjectTemplateName()))
{
}

// ----------------------------------------------------------------------

AlterSchedulerNamespace::ObjectInfo::~ObjectInfo()
{
	delete [] m_objectTemplateName;
	delete [] m_networkIdString;
	m_object = 0;
}

// ----------------------------------------------------------------------

Object const *AlterSchedulerNamespace::ObjectInfo::getObject() const
{
	return m_object;
}

// ----------------------------------------------------------------------

char const *AlterSchedulerNamespace::ObjectInfo::getNetworkIdString() const
{
	return m_networkIdString;
}

// ----------------------------------------------------------------------

char const *AlterSchedulerNamespace::ObjectInfo::getObjectTemplateName() const
{
	return m_objectTemplateName;
}

// ======================================================================
// Namespace AlterSchedulerNamespace functions
// ======================================================================

void AlterSchedulerNamespace::remove()
{
	CrashReportInformation::removeDynamicText(s_crashReportInfo);

	s_areListsValid = false;

	for (int i = 0; i < AS_MAX_SCHEDULE_PHASE_COUNT; ++i)
		delete s_alterNextFrameListFirst[i];

	delete s_alterNowListFirst;
	delete s_concludeListFirst;

#if DO_RECURSION_CHECK
	delete s_criticalSection;
	s_criticalSection = 0;
#endif

	ObjectInfoMap::iterator const endIt = s_objectInfoMap.end();
	for (ObjectInfoMap::iterator it = s_objectInfoMap.begin(); it != endIt; ++it)
		delete it->second;

	s_objectInfoMap.clear();
}

// ----------------------------------------------------------------------

void AlterSchedulerNamespace::reportPrint()
{
	DEBUG_REPORT_PRINT(true, ("AlterScheduler: elapsed time:                  [%.3f]\n", s_schedulerElapsedTime));
	DEBUG_REPORT_PRINT(true, ("AlterScheduler: internal time:                 [%d].\n", static_cast<int>(s_currentTime)));
	DEBUG_REPORT_PRINT(true, ("AlterScheduler: scheduler map entry count:     [%d].\n", static_cast<int>(s_scheduleMap.size())));
	DEBUG_REPORT_PRINT(true, ("AlterScheduler: most recent frame alter count: [%d].\n", s_objectsAltered));
}

// ----------------------------------------------------------------------
/**
 * nullptr objects are not considered valid by this function.  Do not call this on a nullptr
 * object if that happens to be valid in the context in which this function is used.
 */

void AlterSchedulerNamespace::validateObject(Object const *object)
{
	FATAL(!object, ("validateObject(): alter scheduler found nullptr object."));
	
	DO_ON_VALIDATE_OBJECTS(bool isInvalid = false);

#if VALIDATE_OBJECTS

#ifdef _DEBUG
	//-- Check if object memory is deleted.
	if (*reinterpret_cast<uint32 const*>(object) == cs_freeFillPattern) //lint !e740 // unusual pointer cast // Yes.
	{
		DEBUG_WARNING(true, ("validateObject(): alter scheduler found deleted object (object memory starts with 4 bytes of free-fill pattern).")); //lint !e740 // unusual pointer cast from incompatible indirect types // okay, I'm doing something unusual.
		isInvalid = true;
	}
#endif // _DEBUG

#if USE_TYPEINFO_CATCH_BLOCK
	if (!isInvalid)
	{
		try
		{
#endif

			// If you crash here, object is an invalid pointer and should not be handled by the alter scheduler.
			type_info const &typeInfo = typeid(*object);
			UNREF(typeInfo);
			
#if USE_TYPEINFO_CATCH_BLOCK
		}
		catch (...)
		{ //lint !e1775 // catch block does not declare any exception // that's right: I want to catch it all.
			WARNING(true, ("validateObject(): Object-derived class with address [%x] failed to return valid C++ type info; likely a deleted Object.", reinterpret_cast<unsigned int const>(object)));
			isInvalid = true;
		}
	}
#endif

#if OBJECT_SUPPORTS_DESTROYED_FLAG	
	//-- Check if specified object is already destroyed.
	if (!isInvalid && object->isDestroyed())
	{
		WARNING(true, ("validateObject(): alter scheduler found destroyed object, pointer=[%p].", object));
		isInvalid = true;
	}
#endif // OBJECT_SUPPORTS_DESTROYED_FLAG

	if (isInvalid)
	{
		//-- Print out object info for the invalid object if we have it.
		ObjectInfo *objectInfo = nullptr;
		if (s_trackObjectInfo)
		{
			ObjectInfoMap::iterator findIt = s_objectInfoMap.find(const_cast<Object*>(object));
			if (findIt != s_objectInfoMap.end())
				objectInfo = findIt->second;
		}

		if (objectInfo)
			FATAL(true, ("We tracked object info for pointer=[%p]: object id=[%s], object template=[%s].", object, objectInfo->getNetworkIdString(), objectInfo->getObjectTemplateName()));
		else
			FATAL(true, ("We either don't track object info or we had no object info for object pointer=[%p].", object));
	}
	
#endif // VALIDATE_OBJECTS

	UNREF(object);
}

// ----------------------------------------------------------------------
/**
 * Return true if two strings are the same (or are both nullptr); otherwise, return false.
 */

static bool SafeStringEqual(char const *string1, char const *string2)
{
	if (string1 && string2)
		return (strcmp(string1, string2) == 0);
	else if (!string1 && !string2)
		return true;
	else
		return false;
}

// ----------------------------------------------------------------------

void AlterSchedulerNamespace::addObjectInfoAsNecessary(Object &object)
{
	DEBUG_FATAL(!s_trackObjectInfo, ("addObjectInfoAsNecessary(): Shouldn't be calling this function, object info tracking is disabled."));

	//-- Check if we have an entry for this object.
	ObjectInfoMap::iterator findIt = s_objectInfoMap.find(&object);
	if (findIt == s_objectInfoMap.end())
	{
		//-- Doesn't exist.  Add now.
		IGNORE_RETURN(s_objectInfoMap.insert(ObjectInfoMap::value_type(&object, new ObjectInfo(object))));
	}
	else
	{
		//-- Verify that they're the same.  If not, object pointer has been reused, create new object info.
		ObjectInfo *const existingInfo = findIt->second;
		NOT_NULL(existingInfo);

		bool replace = false;
		if (!SafeStringEqual(existingInfo->getNetworkIdString(), object.getNetworkId().getValueString().c_str()))
		{
			DEBUG_REPORT_LOG(true, ("AlterSchedulerNamespace::addObjectInfoAsNecessary(): Object pointer [%p] has new network id, old=[%s],new=[%s], Object pointer probably reused.\n", &object, existingInfo->getNetworkIdString(), object.getNetworkId().getValueString().c_str()));
			replace = true;
		}

		if (!SafeStringEqual(existingInfo->getObjectTemplateName(), object.getObjectTemplateName()))
		{
			DEBUG_REPORT_LOG(true, ("AlterSchedulerNamespace::addObjectInfoAsNecessary(): Object pointer [%p] has new object template name, old=[%s],new=[%s], Object pointer probably reused.\n", &object, existingInfo->getObjectTemplateName(), object.getObjectTemplateName()));
			replace = true;
		}

		if (replace)
		{
			delete existingInfo;
			findIt->second = new ObjectInfo(object);
		}
	}
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void AlterSchedulerNamespace::doPerObjectAlterReportCollection(Object const* object, float alterResult)
{
	if (s_reportPerObject)
	{
		char appearanceAlterResultText[16];
		char controllerAlterResultText[16];
		char dynamicsAlterResultText[16];
		char childrenAlterResultText[16];
		char objectAlterResultText[16];
		char finalAlterResultText[16];

		char const * debugName = "deleted";
		char const * objectTemplateName = "";

		debugName = object->getDebugName();
		if (!debugName)
			debugName = "unknown";
		objectTemplateName = object->getObjectTemplateName();
		if (!objectTemplateName)
			objectTemplateName = "unknown";

		AlterResult::formatTime(appearanceAlterResultText, sizeof(appearanceAlterResultText), s_reportedAppearanceResult);
		AlterResult::formatTime(controllerAlterResultText, sizeof(controllerAlterResultText), s_reportedControllerResult);
		AlterResult::formatTime(dynamicsAlterResultText, sizeof(dynamicsAlterResultText), s_reportedDynamicsResult);
		AlterResult::formatTime(childrenAlterResultText, sizeof(childrenAlterResultText), s_reportedChildrenResult);
		AlterResult::formatTime(objectAlterResultText, sizeof(objectAlterResultText), s_reportedObjectResult);
		AlterResult::formatTime(finalAlterResultText, sizeof(finalAlterResultText), alterResult);
		DEBUG_REPORT_PRINT(true, ("object %s %s\n  %d calls to Object::alter\n  %s %s appearance\n  %s %s controller\n  %s %s dynamics\n  %s children\n  %s object\n  %s final\n", debugName, objectTemplateName, s_totalObjectAlterCalls, appearanceAlterResultText, s_reportedAppearanceType, controllerAlterResultText, s_reportedControllerType, dynamicsAlterResultText, s_reportedDynamicsType, childrenAlterResultText, objectAlterResultText, finalAlterResultText));
	}
}

#endif

// ----------------------------------------------------------------------

#if DO_RECURSION_CHECK

void AlterSchedulerNamespace::incrementRecursionDepth()
{
	NOT_NULL(s_criticalSection);

	s_criticalSection->enter();
	{
		++s_recursionDepth;
	}
	s_criticalSection->leave();
}

#endif

// ----------------------------------------------------------------------

#if DO_RECURSION_CHECK

void AlterSchedulerNamespace::decrementRecursionDepth()
{
	NOT_NULL(s_criticalSection);

	s_criticalSection->enter();
	{
		--s_recursionDepth;
		DEBUG_FATAL(s_recursionDepth < 0, ("invalid recursion depth < 0 [%d].", s_recursionDepth));
	}
	s_criticalSection->leave();
}

#endif

// ----------------------------------------------------------------------

#if DO_RECURSION_CHECK

int AlterSchedulerNamespace::getRecursionDepth()
{
	NOT_NULL(s_criticalSection);

	int value;
	s_criticalSection->enter();
	{
		value = s_recursionDepth;
	}
	s_criticalSection->leave();

	return value;
}

#endif

// ----------------------------------------------------------------------

#if DO_RECURSION_CHECK

void AlterSchedulerNamespace::doPreAlterRecursionCheck()
{
	//-- Check for recursive calling on the front end.
	incrementRecursionDepth();
	FATAL(getRecursionDepth() > 1, ("Recursively calling AlterSchedulerNamespace::doAlterConclude(), this must be fixed."));
}

#endif

// ----------------------------------------------------------------------

#if DO_RECURSION_CHECK

void AlterSchedulerNamespace::doPostAlterRecursionCheck()
{
	//-- Check for recursive calling on the back end.
	decrementRecursionDepth();
	FATAL(getRecursionDepth() > 0, ("AlterSchedulerNamespace::doAlterConclude() called recursively, must fix."));
}

#endif

// ----------------------------------------------------------------------

void AlterSchedulerNamespace::incrementSchedulerTimerByElapsedTime(float schedulerElapsedTime)
{
	// Compute new absolute scheduler time.
	s_schedulerElapsedTime = schedulerElapsedTime;

	ScheduleTime const previousSchedulerTime = s_currentTime;
	s_currentTime += static_cast<ScheduleTime>(schedulerElapsedTime * cs_schedulerTicksPerSecond);

	if (previousSchedulerTime == s_currentTime)
	{
		//-- If this warning/fatal is hit, check the following:
		// * Did something call AlterScheduler::alter() with zero time?  That should not happen.
		// * Is the ticks per second float-to-integral conversion factor too small?  If so, fast framerates can cause this variable to stay zero.
		//   Fixes for this are to increase the # ticks per second (may require changing ScheduleTime from 32 bits to 64 bits).
		WARNING_STRICT_FATAL(true, ("doAlterConclude() called with elapsed time [%g] resulting in no forward time movement, bumping up elapsed time to [%g].", schedulerElapsedTime, cs_secondsPerSchedulerTick));
		++s_currentTime;
	}
}

// ======================================================================
// class AlterScheduler: PUBLIC STATIC
// ======================================================================

void AlterScheduler::install()
{
	DebugFlags::registerFlag(s_report, "SharedObject/AlterScheduler", "report", reportPrint);
	DebugFlags::registerFlag(s_suspendExecution, "SharedObject/AlterScheduler", "suspendExecution");
	DebugFlags::registerFlag(s_alwaysAlter, "SharedObject/AlterScheduler", "alwaysAlter");
	DebugFlags::registerFlag(s_trackObjectInfo, "SharedObject/AlterScheduler", "trackObjectInfo");
		
	ObjectInfo::install();

	for (int i = 0; i < AS_MAX_SCHEDULE_PHASE_COUNT; ++i)
	{
		s_alterNextFrameListFirst[i] = new Object;
		s_alterNextFrameListFirst[i]->createScheduleDataIfNecessary(s_currentTime);
	}

	s_alterNowListFirst = new Object;
	s_alterNowListFirst->createScheduleDataIfNecessary(s_currentTime);

	s_concludeListFirst = new Object;
	s_concludeListFirst->createScheduleDataIfNecessary(s_currentTime);

	s_areListsValid = true;

#if DO_RECURSION_CHECK
	s_criticalSection = new Mutex();
#endif

	s_crashReportInfo[0] = '\0';
	CrashReportInformation::addDynamicText(s_crashReportInfo);

	DO_ON_DEBUG( DebugFlags::registerFlag(s_reportPerObject, "SharedObject/AlterScheduler", "reportPerObject") );
	DO_ON_DEBUG( DebugFlags::registerFlag(s_profileAlterByType, "SharedObject/AlterScheduler", "profileAlterByType") );
	DO_ON_DEBUG( DebugFlags::registerFlag(s_logAddRemove, "SharedObject/AlterScheduler", "logAddRemove") );

	ExitChain::add(remove, "AlterScheduler");
}

// ----------------------------------------------------------------------

void AlterScheduler::submitForAlter(Object &object)
{
	if (!object.getUseAlterScheduler())
		return;

	DEBUG_REPORT_LOG(s_logAddRemove, ("AlterScheduler::submitForAlter: object pointer=[%p], id=[%s], template=[%s]\n", &object, object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName()));
	DO_ON_VALIDATE_OBJECTS(validateObject(&object));

	if (s_trackObjectInfo)
		addObjectInfoAsNecessary(object);

	DEBUG_FATAL(!object.isInitialized(), ("AlterScheduler::submitForAlter(): tried to add an uninitialized object to alter scheduler, call site must fix. Pointer=[%p],id=[%s],template=[%s].", &object, object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName()));

	//-- If this is the first time this Object is getting scheduled, create the
	//   per-Object ScheduleData and initialize the most recent alter time to the specified value.
	object.createScheduleDataIfNecessary(s_currentTime);
	addToAlterNextFrameList(object);
}

// ----------------------------------------------------------------------
/**
 * @return true if the object existed in the alter scheduler, false otherwise.
 */

bool AlterScheduler::removeObject(Object &object)
{
	DO_ON_OBJECT_ALTER_FLAG_SUPPORTED( DEBUG_FATAL(object.isAltering(), ("AlterScheduler::removeObject(): unsupported: tried to remove object from alter scheduler during alter. Pointer=[%p],id=[%s],template=[%s].", &object, object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName())) );

	DEBUG_REPORT_LOG(s_logAddRemove, ("AlterScheduler::removeObject: object pointer=[%p], id=[%s], template=[%s]\n", &object, object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName()));

	if (!object.hasScheduleData())
		return false;

	bool result = false;

	PROFILER_AUTO_BLOCK_DEFINE("AlterScheduler::removeObject");
	
	//-- Remove object from each of the alter scheduler lists.
	if (object.isInAlterNowList())
	{
		object.removeFromAlterNowList();
		result = true;
	}

	if (object.isInAlterNextFrameList())
	{
		object.removeFromAlterNextFrameList();
		result = true;
	}

	if (object.isInConcludeList())
	{
		object.removeFromConcludeList();
		result = true;
	}

	ScheduleTimeMap::iterator &mapIt = OBJECT_SCHEDULE_TIME_MAP_ITERATOR(object);
	if (mapIt != s_scheduleMap.end())
	{
		s_scheduleMap.erase(mapIt);
		mapIt = s_scheduleMap.end();
		result = true;
	}

	DO_ON_HARDCORE_VALIDATION( DEBUG_FATAL(AlterScheduler::findObjectInScheduleTimeMap(&object), ("removeObject(): object shouldn't be in map but is: pointer=[%p],id=[%s],template=[%s].", &object, object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName())) );

#if AS_USE_HARDCORE_CONTAINER_VALIDATION
	if (s_areListsValid)
	{
		uint32 const findResult = findObject(&object);
		FATAL(findResult  > 0, ("object pointer=[%p], id=[%s], template=[%s] should not be accessible but is [%d], alter scheduler corruption, TRF bug.", 
			&object, object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName(), findResult));
	}
#endif

	return result;
}

// ----------------------------------------------------------------------

void AlterScheduler::setObjectSchedulePhase(Object &object, int schedulePhaseIndex)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, schedulePhaseIndex, AS_MAX_SCHEDULE_PHASE_COUNT);

	object.createScheduleDataIfNecessary(s_currentTime);
	object.setAlterSchedulePhase(schedulePhaseIndex);
}

// ----------------------------------------------------------------------

void AlterScheduler::alter(float elapsedTime)
{
	DO_ON_DEBUG(if (s_suspendExecution) return);

	PROFILER_AUTO_BLOCK_DEFINE("AlterScheduler::alter");
	doAlterAndConcludeForAllObjects(elapsedTime, CS_none, nullptr);
}

// ----------------------------------------------------------------------

void AlterScheduler::alterAllAndConcludeOne(float elapsedTime, Object * objectToConclude)
{
	DO_ON_DEBUG(if (s_suspendExecution) return);

	PROFILER_AUTO_BLOCK_DEFINE("AlterScheduler::alterAndConclude");
	doAlterAndConcludeForAllObjects(elapsedTime, CS_one, objectToConclude);
}

// ----------------------------------------------------------------------

void AlterScheduler::alterAndConclude(float elapsedTime)
{
	DO_ON_DEBUG(if (s_suspendExecution) return);

	PROFILER_AUTO_BLOCK_DEFINE("AlterScheduler::alterAndConclude");
	doAlterAndConcludeForAllObjects(elapsedTime, CS_all, nullptr);
}

// ----------------------------------------------------------------------

void AlterScheduler::setPostAlterHookFunction (PostAlterHookFunction postAlterHookFunction)
{
	s_postAlterHookFunction = postAlterHookFunction;
}

// ----------------------------------------------------------------------

void AlterScheduler::initializeScheduleTimeMapIterator(Object &object)
{
	// Set iterator value contained in pointer to the end of the schedule time map.
	OBJECT_SCHEDULE_TIME_MAP_ITERATOR(object) = s_scheduleMap.end();
}

// ----------------------------------------------------------------------

bool AlterScheduler::isIteratorInScheduleTimeMap(void const *iterator)
{
	DEBUG_FATAL(!iterator, ("AlterScheduler::isIteratorInScheduleTimeMap(): iterator pointer is nullptr."));
	return (*static_cast<ScheduleTimeMap::iterator const*>(iterator) != s_scheduleMap.end());
}

// ----------------------------------------------------------------------

bool AlterScheduler::findObjectInAlterNowList(Object const *object)
{
	DO_ON_HARDCORE_VALIDATION(validateAlterNowList());

	for (Object *searchObject = s_alterNowListFirst->getNextFromAlterNowList(); searchObject != nullptr; searchObject = searchObject->getNextFromAlterNowList())
	{
		if (searchObject == object)
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool AlterScheduler::findObjectInAlterNextFrameLists(Object const *object)
{
	DO_ON_HARDCORE_VALIDATION(validateAlterNextFrameList());

	for (int phaseIndex = 0; phaseIndex < AS_MAX_SCHEDULE_PHASE_COUNT; ++phaseIndex)
	{
		for (Object *searchObject = s_alterNextFrameListFirst[phaseIndex]->getNextFromAlterNextFrameList(); searchObject != nullptr; searchObject = searchObject->getNextFromAlterNextFrameList())
		{
			if (searchObject == object)
				return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool AlterScheduler::findObjectInConcludeList(Object const *object)
{
	DO_ON_HARDCORE_VALIDATION(validateConcludeList());

	for (Object *searchObject = s_concludeListFirst->getNextFromConcludeList(); searchObject != nullptr; searchObject = searchObject->getNextFromConcludeList())
	{
		if (searchObject == object)
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool AlterScheduler::findObjectInScheduleTimeMap(Object const *object)
{
	DO_ON_HARDCORE_VALIDATION(validateScheduleTimeMap());

	ScheduleTimeMap::iterator const endIt = s_scheduleMap.end();
	for (ScheduleTimeMap::iterator it = s_scheduleMap.begin(); it != endIt; ++it)
		if (it->second == object)
			return true;

	return false;
}

// ----------------------------------------------------------------------

uint32 AlterScheduler::findObject(Object const *object)
{
	uint32  result = 0;

	if (findObjectInAlterNowList(object))
		result |= 0x01;

	if (findObjectInAlterNextFrameLists(object))
		result |= 0x02;

	if (findObjectInScheduleTimeMap(object))
		result |= 0x04;

	return result;
}

// ----------------------------------------------------------------------

void AlterScheduler::validateAlterNowList()
{
	ObjectSet   objectSet;
	ObjectList  objectList;

	//-- Add each object to the set and list.
	{
		for (Object *object = s_alterNowListFirst->getNextFromAlterNowList(); object != nullptr; object = object->getNextFromAlterNowList())
		{
			DO_ON_VALIDATE_OBJECTS(validateObject(object));

			// Insert into set, checking for multiple entries of same value.
			std::pair<ObjectSet::iterator, bool> result = objectSet.insert(object);
			UNREF(result);
			DEBUG_FATAL(!result.second, ("validateAlterNowList(): failed, object appears multiple times: pointer=[%p], object id=[%s], object template=[%s].", object, object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));

			// Append to list.
			objectList.push_back(object);
		}
	}

	//-- Walk list forward and make sure they match.
	Object *lastObject = s_alterNowListFirst;
	{
		ObjectList::iterator const endIt = objectList.end();
		Object *object = s_alterNowListFirst->getNextFromAlterNowList();
		for (ObjectList::iterator it = objectList.begin(); (it != endIt) && (object != nullptr); object = object->getNextFromAlterNowList(), ++it)
		{
			lastObject = object;
			DEBUG_WARNING(object != *it, ("validateAlterNowList(): forward linkage check failed, stl list: pointer=[%p], object id=[%s], object template=[%s].", *it, (*it)->getNetworkId().getValueString().c_str(), (*it)->getObjectTemplateName()));
			DEBUG_FATAL(object != *it, ("validateAlterNowList(): forward linkage check failed, alter scheduler list: pointer=[%p], object id=[%s], object template=[%s].", object, object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));
		}
	}

	//-- Walk list backward and make sure they match.
	{
		ObjectList::reverse_iterator const endIt = objectList.rend();
		Object *object = lastObject;
		for (ObjectList::reverse_iterator it = objectList.rbegin(); (it != endIt) && (object != nullptr); object = object->getPreviousFromAlterNowList(), ++it)
		{
			DEBUG_WARNING(object != *it, ("validateAlterNowList(): reverse linkage check failed, stl list: pointer=[%p], object id=[%s], object template=[%s].", *it, (*it)->getNetworkId().getValueString().c_str(), (*it)->getObjectTemplateName()));
			DEBUG_FATAL(object != *it, ("validateAlterNowList(): reverse linkage check failed, alter scheduler list: pointer=[%p], object id=[%s], object template=[%s].", object, object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));
		}
	}
}

// ----------------------------------------------------------------------

void AlterScheduler::validateAlterNextFrameLists()
{
	ObjectSet   objectSet;
	ObjectList  objectList;

	for (int phaseIndex = 0; phaseIndex < AS_MAX_SCHEDULE_PHASE_COUNT; ++phaseIndex)
	{
		//-- Add each object to the set and list.
		{
			for (Object *object = s_alterNextFrameListFirst[phaseIndex]->getNextFromAlterNextFrameList(); object != nullptr; object = object->getNextFromAlterNextFrameList())
			{
				DO_ON_VALIDATE_OBJECTS(validateObject(object));

				// Insert into set, checking for multiple entries of same value.
				std::pair<ObjectSet::iterator, bool> result = objectSet.insert(object);
				UNREF(result);
				DEBUG_FATAL(!result.second, ("validateAlterNextFrameList(): failed, object appears multiple times: pointer=[%p], object id=[%s], object template=[%s].", object, object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));

				// Append to list.
				objectList.push_back(object);
			}
		}

		//-- Walk list forward and make sure they match.
		Object *lastObject = s_alterNextFrameListFirst[phaseIndex];
		{
			ObjectList::iterator const endIt = objectList.end();
			Object *object = s_alterNextFrameListFirst[phaseIndex]->getNextFromAlterNextFrameList();
			for (ObjectList::iterator it = objectList.begin(); (it != endIt) && (object != nullptr); object = object->getNextFromAlterNextFrameList(), ++it)
			{
				lastObject = object;
				DEBUG_WARNING(object != *it, ("validateAlterNextFrameList(): forward linkage check failed, stl list: pointer=[%p], object id=[%s], object template=[%s].", *it, (*it)->getNetworkId().getValueString().c_str(), (*it)->getObjectTemplateName()));
				DEBUG_FATAL(object != *it, ("validateAlterNextFrameList(): forward linkage check failed, alter scheduler list: pointer=[%p], object id=[%s], object template=[%s].", object, object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));
			}
		}

		//-- Walk list backward and make sure they match.
		{
			ObjectList::reverse_iterator const endIt = objectList.rend();
			Object *object = lastObject;
			for (ObjectList::reverse_iterator it = objectList.rbegin(); (it != endIt) && (object != nullptr); object = object->getPreviousFromAlterNextFrameList(), ++it)
			{
				DEBUG_WARNING(object != *it, ("validateAlterNextFrameList(): reverse linkage check failed, stl list: pointer=[%p], object id=[%s], object template=[%s].", *it, (*it)->getNetworkId().getValueString().c_str(), (*it)->getObjectTemplateName()));
				DEBUG_FATAL(object != *it, ("validateAlterNextFrameList(): reverse linkage check failed, alter scheduler list: pointer=[%p], object id=[%s], object template=[%s].", object, object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));
			}
		}

		objectSet.clear();
		objectList.clear();
	}
}

// ----------------------------------------------------------------------

void AlterScheduler::validateConcludeList()
{
	ObjectSet   objectSet;
	ObjectList  objectList;

	//-- Add each object to the set and list.
	{
		for (Object *object = s_concludeListFirst->getNextFromConcludeList(); object != nullptr; object = object->getNextFromConcludeList())
		{
			DO_ON_VALIDATE_OBJECTS(validateObject(object));

			// Insert into set, checking for multiple entries of same value.
			std::pair<ObjectSet::iterator, bool> result = objectSet.insert(object);
			UNREF(result);
			DEBUG_FATAL(!result.second, ("validateConcludeList(): failed, object appears multiple times: pointer=[%p], object id=[%s], object template=[%s].", object, object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));

			// Append to list.
			objectList.push_back(object);
		}
	}

	//-- Walk list forward and make sure they match.
	Object *lastObject = s_concludeListFirst;
	{
		ObjectList::iterator const endIt = objectList.end();
		Object *object = s_concludeListFirst->getNextFromConcludeList();
		for (ObjectList::iterator it = objectList.begin(); (it != endIt) && (object != nullptr); object = object->getNextFromConcludeList(), ++it)
		{
			lastObject = object;
			DEBUG_WARNING(object != *it, ("validateConcludeList(): forward linkage check failed, stl list: pointer=[%p], object id=[%s], object template=[%s].", *it, (*it)->getNetworkId().getValueString().c_str(), (*it)->getObjectTemplateName()));
			DEBUG_FATAL(object != *it, ("validateConcludeList(): forward linkage check failed, alter scheduler list: pointer=[%p], object id=[%s], object template=[%s].", object, object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));
		}
	}

	//-- Walk list backward and make sure they match.
	{
		ObjectList::reverse_iterator const endIt = objectList.rend();
		Object *object = lastObject;
		for (ObjectList::reverse_iterator it = objectList.rbegin(); (it != endIt) && (object != nullptr); object = object->getPreviousFromConcludeList(), ++it)
		{
			DEBUG_WARNING(object != *it, ("validateConcludeList(): reverse linkage check failed, stl list: pointer=[%p], object id=[%s], object template=[%s].", *it, (*it)->getNetworkId().getValueString().c_str(), (*it)->getObjectTemplateName()));
			DEBUG_FATAL(object != *it, ("validateConcludeList(): reverse linkage check failed, alter scheduler list: pointer=[%p], object id=[%s], object template=[%s].", object, object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));
		}
	}
}

// ----------------------------------------------------------------------

void AlterScheduler::validateScheduleTimeMap()
{
	//-- Make sure each object is in this only once.
	ObjectSet   objectSet;

	int         duplicateCount = 0;

	ScheduleTimeMap::iterator const endIt = s_scheduleMap.end();
	for (ScheduleTimeMap::iterator it = s_scheduleMap.begin(); it != endIt; ++it)
	{
		DO_ON_VALIDATE_OBJECTS(validateObject(it->second));

		// Insert into set, checking for multiple entries of same value.
		std::pair<ObjectSet::iterator, bool> result = objectSet.insert(it->second);
		if (!result.second)
		{
			DEBUG_WARNING(true, ("validateAlterScheduleTimeMap(): failed, object appears multiple times: pointer=[%p], object id=[%s], object template=[%s].", it->second, it->second->getNetworkId().getValueString().c_str(), it->second->getObjectTemplateName()));
			++duplicateCount;
		}
	}

	DEBUG_FATAL(duplicateCount > 0, ("validateScheduleTimeMap(): duplicates found, see warnings in output."));
}

// ----------------------------------------------------------------------

void AlterScheduler::validateAllContainers()
{
	validateAlterNowList();
	validateAlterNextFrameLists();
	validateConcludeList();
	validateScheduleTimeMap();
}

// ----------------------------------------------------------------------

float AlterScheduler::getTimeSinceLastFrame()
{
	return s_schedulerElapsedTime;
}

// ----------------------------------------------------------------------

void AlterScheduler::setMostRecentAlterTime(Object &object)
{
	if (object.hasScheduleData())
		object.setMostRecentAlterTime(s_currentTime);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

bool AlterScheduler::reportObjectAlters()
{
	return s_reportPerObject;
}

#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG

void AlterScheduler::countObjectAlter()
{
	++s_totalObjectAlterCalls;
	++s_reportedObjectAlterCalls;
}

#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG

void AlterScheduler::reportObjectAlter(Object const * object, char const * appearanceType, float appearanceResult, char const * controllerType, float controllerResult, char const * dynamicsType, float dynamicsResult, float childrenResult, float objectResult)
{
	if (object == s_currentlyAlteringObject)
	{
		s_reportedAppearanceType    = appearanceType;
		s_reportedAppearanceResult  = appearanceResult;
		s_reportedControllerType    = controllerType;
		s_reportedControllerResult  = controllerResult;
		s_reportedDynamicsType      = dynamicsType;
		s_reportedDynamicsResult    = dynamicsResult;
		s_reportedChildrenResult    = childrenResult;
		s_reportedObjectResult      = objectResult;
	}
}

#endif

// ======================================================================
// class AlterScheduler: PRIVATE STATIC
// ======================================================================

void AlterScheduler::addToAlterNextFrameList(Object &object)
{
	//-- Ensure it's not in the future schedule list.
	//   Note it's okay if it's in the alter now list since the object may
	//   get a submitForAlter() from a related object during alter processing.
	ScheduleTimeMap::iterator &mapIt = OBJECT_SCHEDULE_TIME_MAP_ITERATOR(object);
	if (mapIt != s_scheduleMap.end())
	{
		s_scheduleMap.erase(mapIt);
		mapIt = s_scheduleMap.end();
	}

	DO_ON_HARDCORE_VALIDATION( DEBUG_FATAL(AlterScheduler::findObjectInScheduleTimeMap(&object), ("addToAlterNextFrameList(): object shouldn't be in map but is: pointer=[%p],id=[%s],template=[%s].", &object, object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName())) );

	//-- Add to next frame list as necessary.
	if (!object.isInAlterNextFrameList())
	{
		int const schedulePhase = object.getAlterSchedulePhase();
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, schedulePhase, AS_MAX_SCHEDULE_PHASE_COUNT);

		object.insertIntoAlterNextFrameList(s_alterNextFrameListFirst[schedulePhase]);
	}
}

// ----------------------------------------------------------------------

void AlterScheduler::addToAlterNowList(Object &object)
{
	//-- Remove from any other container.
	if (object.isInAlterNextFrameList())
		object.removeFromAlterNextFrameList();

	ScheduleTimeMap::iterator &mapIt = OBJECT_SCHEDULE_TIME_MAP_ITERATOR(object);
	if (mapIt != s_scheduleMap.end())
	{
		s_scheduleMap.erase(mapIt);
		mapIt = s_scheduleMap.end();
	}

	DO_ON_HARDCORE_VALIDATION( DEBUG_FATAL(AlterScheduler::findObjectInScheduleTimeMap(&object), ("addToAlterNowList(): object shouldn't be in map but is: pointer=[%p],id=[%s],template=[%s].", &object, object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName())) );

	//-- Add to alter now list as necessary.
	if (!object.isInAlterNowList())
		object.insertIntoAlterNowList(s_alterNowListFirst);
}

// ----------------------------------------------------------------------

void AlterScheduler::addToScheduleMap(Object &object, ScheduleTime nextAlterTime)
{
	//-- Remove from alter next frame list.
	if (object.isInAlterNextFrameList())
		object.removeFromAlterNextFrameList();

	ScheduleTimeMap::iterator &mapIt = OBJECT_SCHEDULE_TIME_MAP_ITERATOR(object);
	if (mapIt != s_scheduleMap.end())
		s_scheduleMap.erase(mapIt);

	//-- Add object to schedule map.
	mapIt = s_scheduleMap.insert(ScheduleTimeMap::value_type(nextAlterTime, &object));

	DO_ON_HARDCORE_VALIDATION( DEBUG_FATAL(!AlterScheduler::findObjectInScheduleTimeMap(&object), ("addToScheduleMap(): object should be in map but isn't: pointer=[%p],id=[%s],template=[%s].", &object, object.getNetworkId().getValueString().c_str(), object.getObjectTemplateName())) );
}

// ----------------------------------------------------------------------

void AlterScheduler::dumpScheduleMap()
{
	REPORT_LOG(true, ("Dumping schedule map: %d entries.\n", static_cast<int>(s_scheduleMap.size())));

	ScheduleTimeMap::iterator const endIt = s_scheduleMap.end();
	int i = 0;

	for (ScheduleTimeMap::iterator it = s_scheduleMap.begin(); it != endIt; ++it, ++i)
	{
		Object *const object = it->second;
		if (object)
		{
			ScheduleTime nextAlterTime = it->first;
			REPORT_LOG(true, ("%d: object id [%s], ptr=[%p], last alter [%d], next alter [%d].\n", i+1, object->getNetworkId().getValueString().c_str(), object, static_cast<int>(object->getMostRecentAlterTime()), static_cast<int>(nextAlterTime)));
		}
	}
}

// ----------------------------------------------------------------------

void AlterScheduler::moveReadyObjectsFromSchedulerToNextFrameList()
{
	//-- Copy all expired alter scheduler entries into the "alter next frame" list.
	{
		PROFILER_AUTO_BLOCK_DEFINE("update expired");

		while (!s_scheduleMap.empty())
		{
			ScheduleTimeMap::iterator it = s_scheduleMap.begin();
			if (!s_alwaysAlter && (s_currentTime < it->first))
			{
				//-- We're not always altering and we've hit an entry whose expiration time has not yet been hit.
				//   We're done searching for new entries to alter this frame.
				break;
			}
			else
			{
				//-- Get object, validate it.
				Object *const object = it->second;
				DO_ON_VALIDATE_OBJECTS(validateObject(object));

				//-- This function will remove the object from the schedule map.
				addToAlterNextFrameList(*object);

				DO_ON_HARDCORE_VALIDATION( DEBUG_FATAL(findObjectInAlterNowList(object), ("found object in alter now list, unexpected.")) );
				DO_ON_HARDCORE_VALIDATION( DEBUG_FATAL(findObjectInScheduleTimeMap(object), ("found object in time schedule map, unexpected.")) );
				DO_ON_HARDCORE_VALIDATION( DEBUG_FATAL(!findObjectInAlterNextFrameList(object), ("didn't find object in alter next frame list, unexpected.")) );
			}
		}
	}
}

// ----------------------------------------------------------------------

void AlterScheduler::moveObjectsFromAlterNextFrameListToAlterNowList(int schedulePhaseIndex)
{
	//-- Copy all alter next frame entries into the alter now list.
	s_objectsAltered = 0;
	{
		PROFILER_AUTO_BLOCK_DEFINE("copy next frame");

		//if (s_alterNextFrameListFirst != nullptr) {
			for (Object *object = s_alterNextFrameListFirst[schedulePhaseIndex]->getNextFromAlterNextFrameList(); object != nullptr; )
			{
				//-- Add object to alter now list.  This removes the object from the alter next frame list.
				DO_ON_VALIDATE_OBJECTS(validateObject(object));

				//-- Save next object in list.
				Object *const nextObject = object->getNextFromAlterNextFrameList();

				addToAlterNowList(*object);
				++s_objectsAltered;

				DO_ON_HARDCORE_VALIDATION( DEBUG_FATAL(findObjectInScheduleTimeMap(object), ("found object in time schedule map, unexpected.")) );
				DO_ON_HARDCORE_VALIDATION( DEBUG_FATAL(findObjectInAlterNextFrameList(object), ("found object in alter next frame list, unexpected.")) );
				DO_ON_HARDCORE_VALIDATION( DEBUG_FATAL(!findObjectInAlterNowList(object), ("didn't find object in alter now list, unexpected.")) );

				//-- Increment loop.
				object = nextObject;
			}
		//}
	}
}

// ----------------------------------------------------------------------

void AlterScheduler::alterSingleObject(Object *object, ConcludeStyle concludeStyle, Object *&nextObject)
{
	DO_ON_HARDCORE_VALIDATION(validateAllContainers());
	DO_ON_HARDCORE_VALIDATION(DEBUG_FATAL(!findObjectInAlterNowList(object), ("didn't find object in alter now list, unexpected.")));
	DO_ON_VALIDATE_OBJECTS(validateObject(object));

	//-- Figure out how much time to apply to alter().
	float const elapsedTime = cs_secondsPerSchedulerTick * (s_currentTime - object->getMostRecentAlterTime());

	DO_ON_DEBUG(s_reportedObjectAlterCalls = 0);
	DO_ON_DEBUG(s_currentlyAlteringObject = object);

#if defined(_WIN32) && defined(_DEBUG)
	char const * const typeName = s_profileAlterByType ? typeid(*object).name() : nullptr;
	PROFILER_BLOCK_DEFINE(profilerBlock, typeName);
	if (typeName)
		PROFILER_BLOCK_ENTER(profilerBlock);
#endif

	float alterResult = AlterResult::cms_alterNextFrame;
	bool  alreadyProcessed = (object->getMostRecentAlterTime() == s_currentTime);
	if (!alreadyProcessed)
	{
		//-- Update the most recent alter time for this object.
		object->setMostRecentAlterTime(s_currentTime);

		//-- Perform the alter.
		DO_ON_OBJECT_ALTER_FLAG_SUPPORTED(object->setIsAltering(true));
		alterResult = object->alter(elapsedTime);
		DO_ON_OBJECT_ALTER_FLAG_SUPPORTED(object->setIsAltering(false));

#if defined(_WIN32) && defined(_DEBUG)
		if (typeName)
			PROFILER_BLOCK_LEAVE(profilerBlock);
#endif

		DO_ON_DEBUG(s_currentlyAlteringObject = nullptr);
		DO_ON_VALIDATE_OBJECTS(validateObject(object));
	}

	//-- Get the next object.  Can't do it before this because this object may remove the next object from the alter now list.
	DO_ON_HARDCORE_VALIDATION(DEBUG_FATAL(!findObjectInAlterNowList(object), ("didn't find object in alter now list, unexpected.")));
	nextObject = object->getNextFromAlterNowList();

#if VALIDATE_OBJECTS
	if (nextObject != nullptr)
	{
		DO_ON_HARDCORE_VALIDATION(DEBUG_FATAL(!findObjectInAlterNowList(nextObject), ("didn't find object in alter now list, unexpected.")));
		validateObject(nextObject);
	}
#endif

	//-- If we've already processed this entry in a previous iteration prior to a delete and restart,
	//   don't process this alter again.
	if (alreadyProcessed)
		return;

	//-- Validate post-alter assertions.
	// Ensure the object hasn't crept into the schedule map.  Only applicable if
	// we haven't already processed this object prior to a loop restart (due to deleted object).
	DEBUG_FATAL(OBJECT_SCHEDULE_TIME_MAP_ITERATOR(*object) != s_scheduleMap.end(), ("AlterScheduler: object pointer=[%p],id=[%s],template=[%s] was in schedule map immediately after alter, shouldn't happen.", object, object->getNetworkId().getValueString().c_str(), object->getObjectTemplateName()));
	DO_ON_DEBUG(doPerObjectAlterReportCollection(object, alterResult));

	//-- We will move items into the conclude list IF the object is going to do a conclude all.

	//-- Determine what happens next to the object based on alter()'s return value.
	if (alterResult == AlterResult::cms_kill) //lint !e777 // Testing floats for equality. // This is okay.
	{
		IGNORE_RETURN(snprintf(s_crashReportInfo, sizeof(s_crashReportInfo) - 1, "Alter Killing: %s\n", object->getObjectTemplateName()));
		s_crashReportInfo[sizeof(s_crashReportInfo) - 1] = '\0';

		PROFILER_AUTO_BLOCK_DEFINE("AlterScheduler::delete"); //lint !e578 // redeclarations // can't help.

		// Remove object from the world.
		if (object->isInWorld())
			object->removeFromWorld();

		// Remove object from alter scheduler.
		IGNORE_RETURN(removeObject(*object));

		delete object;
		DO_ON_HARDCORE_VALIDATION(validateAllContainers());

		//-- Since we deleted object, we may have deleted our next link to follow.  Restart alter list processing.
		//   Yes, this could be very inefficient since execution could restart potentially as many times as there are
		//   objects in the list.  In practice, this shouldn't be an issue because we typically delete several objects
		//   at the same time via this one delete call, meaning many deletes are covered by a single list traversal
		//   restart.
		nextObject = s_alterNowListFirst->getNextFromAlterNowList();
	}
	else
	{
		PROFILER_AUTO_BLOCK_DEFINE("AlterScheduler::scheduleNext"); //lint !e578 // redeclarations // can't help.

		//-- Unlink from alter now list.
		object->removeFromAlterNowList();
		DO_ON_HARDCORE_VALIDATION(validateAllContainers());

		if (concludeStyle == CS_all)
			object->insertIntoConcludeList(s_concludeListFirst);

		// If we're always altering, set the effect alter result to "alter next frame", regardless
		// of the real return value.  Otherwise we do the normal thing and use the returned alter
		// result.  Note we've already checked for the alter-requested kill by this point, so we're
		// not overwriting that.

		float effectiveAlterResult;

		//-- Check if this object was scheduled for an alter during processing of this loop.  If so,
		//   then the object is already in the alter next frame map and there's nothing more to do;
		//   otherwise, figure out when to schedule it and add to schedule map.
		if (!object->isInAlterNextFrameList())
		{
			if (alterResult == AlterResult::cms_keepNoAlter) //lint !e777 // Testing floats for equality. // This is okay, we're using constants.
			{
				// Check if we're testing by scheduling no-alter values as some time in the future.
				// If the no-alter schedule delay is non-zero, we really want to schedule alters for objects
				// that request no alter.  Typically this delay will be a large value (e.g. weeks into the future).
				// The delay is in seconds.
				int const futureAlterTime = ConfigSharedObject::getAlterSchedulerNoAlterScheduleDelay ();
				effectiveAlterResult = (futureAlterTime > 0) ? static_cast<float>(futureAlterTime) : alterResult;
			}
			else
				effectiveAlterResult = alterResult;
			
			if (effectiveAlterResult != AlterResult::cms_keepNoAlter) //lint !e777 // Testing floats for equality. // This is okay, we're using constants.
			{
				// This object would like to be scheduled for an alter some time in the future.
				
				// Calculate absolute time for next scheduling.
				ScheduleTime const dt = static_cast<ScheduleTime>(effectiveAlterResult * cs_schedulerTicksPerSecond);
				ScheduleTime const absoluteScheduleTime = s_currentTime + dt;

				// Add it to the schedule list for the specified scheduler time.
				// If this object is nullptr, it means it returned an AlterResult that indicated it should still be alive but it somehow got killed.
//DEBUG_REPORT_LOG(object->getNetworkId() != NetworkId::cms_invalid, ("[aitest] scheduling %s to alter at %lu (%lu + %lu)\n", 
//	object->getNetworkId().getValueString().c_str(), 
//	static_cast<unsigned long>(absoluteScheduleTime),
//	static_cast<unsigned long>(s_currentTime),
//	static_cast<unsigned long>(dt)));
				addToScheduleMap(*object, absoluteScheduleTime);
				DO_ON_HARDCORE_VALIDATION(validateAllContainers());
			}
		}
	}
}

// ----------------------------------------------------------------------

void AlterScheduler::concludeAndRemoveAllConcludeEntries()
{
	PROFILER_AUTO_BLOCK_DEFINE("conclude");

	Object *nextObject;
	for (Object *object = s_concludeListFirst->getNextFromConcludeList(); object != nullptr; object = nextObject)
	{
		//-- Conclude the object.
		DO_ON_VALIDATE_OBJECTS(validateObject(object));
		object->conclude();

		nextObject = object->getNextFromConcludeList();
		object->removeFromConcludeList();

		DO_ON_HARDCORE_VALIDATION(validateAllContainers());
	}
}

// ----------------------------------------------------------------------

void AlterScheduler::doAlterAndConcludeForAllObjects(float schedulerElapsedTime, ConcludeStyle concludeStyle, Object *objectToConclude)
{
	doPreAlterRecursionCheck();
	incrementSchedulerTimerByElapsedTime(schedulerElapsedTime);
	moveReadyObjectsFromSchedulerToNextFrameList();
	
//DEBUG_REPORT_LOG(true, ("[aitest] Altering objects at %lu\n", static_cast<unsigned long>(s_currentTime)));

	for (int schedulePhaseIndex = 0; schedulePhaseIndex < AS_MAX_SCHEDULE_PHASE_COUNT; ++schedulePhaseIndex)
	{
		moveObjectsFromAlterNextFrameListToAlterNowList(schedulePhaseIndex);

		DO_ON_DEBUG(s_totalObjectAlterCalls = 0);
		
		//-- Do an alter for the "alter now" list.
		// MAIN ALTER PROCESSING LOOP.
		{
			PROFILER_AUTO_BLOCK_DEFINE("alter");

			DO_ON_HARDCORE_VALIDATION(validateAllContainers());

			Object *nextObject;
			for (Object *object = s_alterNowListFirst->getNextFromAlterNowList(); object != nullptr; object = nextObject)
				alterSingleObject(object, concludeStyle, nextObject);
		}

		DO_ON_HARDCORE_VALIDATION(validateAllContainers());
	}
	
	//-- Only call this on the last run.
	if (s_postAlterHookFunction)
		s_postAlterHookFunction(schedulerElapsedTime);

	if (concludeStyle == CS_all)
	{
		//-- Do a conclude on the conclude list.  We must do all frame alters before we do a conclude.
		//   This is because an alter for an object later in the list may cause changes to an object earlier in the
		//   list, which will require an adjustment to conclude handling for the earlier object.
		concludeAndRemoveAllConcludeEntries();
	}
	else
	{
		if ((concludeStyle == CS_one) && objectToConclude)
		{
			PROFILER_AUTO_BLOCK_DEFINE("conclude");

			DO_ON_VALIDATE_OBJECTS(validateObject(objectToConclude));
			objectToConclude->conclude();
		}
	}
	DO_ON_HARDCORE_VALIDATION(validateAllContainers());

	doPostAlterRecursionCheck();
}

// ======================================================================
