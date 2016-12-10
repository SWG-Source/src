// ======================================================================
//
// AlterScheduler.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AlterScheduler_H
#define INCLUDED_AlterScheduler_H

// ======================================================================

// Enable AS_USE_HARDCORE_CONTAINER_VALIDATION by setting value to 1 if you
// believe the alter scheduler is not processing Objects properly.  This will
// greatly slow down AlterScheduler performance but will highlight container
// corruption if and when it happens.

#ifdef _DEBUG
#  define AS_USE_HARDCORE_CONTAINER_VALIDATION 0
#else
#  define AS_USE_HARDCORE_CONTAINER_VALIDATION 0
#endif

#define AS_MAX_SCHEDULE_PHASE_COUNT  2

// ======================================================================

class Object;

// ======================================================================

class AlterScheduler
{
public:

	class ScheduleTimeComparator;

	typedef uint64                                   ScheduleTime;
	typedef std::multimap<ScheduleTime, Object*>  ScheduleTimeMap;

public:

	static void install();

	static void submitForAlter(Object &object);
	static bool removeObject(Object &object);

	static void setObjectSchedulePhase(Object &object, int schedulePhaseIndex);

	static void alter(float schedulerElapsedTime);
	static void alterAndConclude(float elapsedTime);
	static void alterAllAndConcludeOne(float elapsedTime, Object * objectToConclude);

	typedef void (*PostAlterHookFunction) (float elapsedTime);
	static void setPostAlterHookFunction (PostAlterHookFunction postAlterHookFunction);

	static void initializeScheduleTimeMapIterator(Object &object);
	static bool isIteratorInScheduleTimeMap(void const *iterator);

	static bool   findObjectInAlterNowList(Object const *object);
	static bool   findObjectInAlterNextFrameLists(Object const *object);
	static bool   findObjectInConcludeList(Object const *object);
	static bool   findObjectInScheduleTimeMap(Object const *object);
	static uint32 findObject(Object const *object);

	static void   validateAlterNowList();
	static void   validateAlterNextFrameLists();
	static void   validateConcludeList();
	static void   validateScheduleTimeMap();
	static void   validateAllContainers();

	static float  getTimeSinceLastFrame();

	static void   setMostRecentAlterTime(Object &object);

#ifdef _DEBUG

	static bool reportObjectAlters();
	static void countObjectAlter();
	static void reportObjectAlter(Object const *object, char const * appearanceType, float appearanceResult, char const * controllerType, float controllerResult, char const * dynamicsType, float dynamicsResult, float childrenResult, float objectResult);

#endif

private:

	enum ConcludeStyle
	{
		CS_none,
		CS_one,
		CS_all
	};

private:

	static void addToAlterNextFrameList(Object &object);
	static void addToAlterNowList(Object &object);
	static void addToScheduleMap(Object &object, ScheduleTime nextAlterTime);

	static void dumpScheduleMap();
	static void moveReadyObjectsFromSchedulerToNextFrameList();
	static void moveObjectsFromAlterNextFrameListToAlterNowList(int schedulePhaseIndex);
	static void prepareListsForAlter();
	static void alterSingleObject(Object *object, ConcludeStyle concludeStyle, Object *&nextObject);
	static void concludeAndRemoveAllConcludeEntries();
	static void doAlterAndConcludeForAllObjects(float schedulerElapsedTime, ConcludeStyle concludeStyle, Object *objectToConclude);

};

// ======================================================================

#endif
