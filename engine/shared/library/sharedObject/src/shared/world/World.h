// ======================================================================
//
// World.h
// asommers 2-26-99
//
// copyright 1999, bootprint entertainment
// copyright 2001, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_World_H
#define INCLUDED_World_H

// ======================================================================

enum WorldObjectLists
{
	WOL_MarkerAllObjectListsStart,
	WOL_MarkerRenderedStart       = WOL_MarkerAllObjectListsStart,

	WOL_MarkerObjectsStart        = WOL_MarkerRenderedStart,

		WOL_Tangible                = WOL_MarkerObjectsStart,
		WOL_TangibleNotTargetable,
		WOL_Intangible,
		WOL_TangibleFlora,

	WOL_MarkerObjectsEnd,
	WOL_MarkerRenderedEnd         = WOL_MarkerObjectsEnd,

		WOL_Inactive                = WOL_MarkerObjectsEnd,

	WOL_Count,
	WOL_MarkerAllObjectListsEnd   = WOL_Count
};

// ======================================================================

class LotManager;
class Object;
class ObjectList;
class NetworkId;

namespace MessageDispatch
{
	class Emitter;
}

// ======================================================================

class World
{
public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Messages
	{
	public:

		static const char * const OBJECT_ADDED;
		static const char * const OBJECT_REMOVING;
		static const char * const OBJECT_REMOVED;
		static const char * const OBJECT_ADDED_PTR;

		// message type contains a std::pair<NetworkId, int>
		// second parameter is the index which was operated upon
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct Emitter;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	static int                  getFrameNumber ();

	static const LotManager*    getConstLotManager ();
	static LotManager*          getLotManager ();

	static void                 snapAllObjectsToTerrain ();
	static void                 snapObjectsToTerrain (int listIndex);
	static void                 snapObjectToTerrain (Object* object, bool alignToTerrain=false);

	static void                 validate ();
	static bool                 isValid ();

	static bool                 existsInWorld (const Object* object);
	static bool                 isInstalled();

	static int                  getNumberOfObjects (int listIndex);
	static Object*              getObject (int listIndex, int index);
	static const Object*        getConstObject (int listIndex, int index);

protected:

	static void                 addObject    (Object* object, int listIndex);
	static bool                 removeObject (const Object* object, int listIndex);
	static void                 queueObject  (Object* object);

	static Object*              findClosestObjectTo (const Object* object, int listIndex);
	static const Object*        findClosestConstObjectTo (const Object* object, int listIndex);

	static const Object*        findNextObject (const Object* object, int listIndex);

	static int findObjectList(Object const * object);

	static const MessageDispatch::Emitter& getEmitter ();

	static int                  getNumberOfQueuedObjects ();
	static Object*              getQueuedObject (int index);
	static const Object*        getConstQueuedObject (int index);
	static bool                 existsInList (const Object* object, int listIndex);

	static void                 install ();
	static void                 remove ();

	static void                 beginFrame ();
	static void                 endFrame ();

	static void                 debugReport ();

private:

	World ();
	virtual ~World ();
	World (const World&);
	World& operator= (const World&);
};

// ======================================================================

#endif
