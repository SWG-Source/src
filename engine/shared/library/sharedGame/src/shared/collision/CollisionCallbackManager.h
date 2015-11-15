// ======================================================================
//
// CollisionCallbackManager.h
// Copyright 2004, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CollisionCallbackManager_H
#define INCLUDED_CollisionCallbackManager_H

#include "sharedMath/Vector.h"

// ======================================================================

class Object;
class NetworkId;

// CollisionCallbackManager sits on top of CollisionWorld and offers a bridge between
// the game and CollisionWorld.  It also acts as a dispatch mediator for implementing
// collisions between objects with different GOT types.

class CollisionCallbackManager
{
public:
	typedef bool (*CanTestCollisionDetectionOnObjectThisFrameFunction)(Object * const object);
	typedef void (*NoHitFunction)(Object * const object);
	typedef bool (*OnHitByObjectFunction)(Object * const object, Object * const wasHitByThisObject);
	typedef bool (*DoCollisionWithTerrainFunction)(Object * const object);
	typedef int (*ConvertObjectToIndexFunction)(Object * const object);

	static void install();

	static void registerCanTestCollisionDetectionOnObjectThisFrame(CanTestCollisionDetectionOnObjectThisFrameFunction);
	static void registerNoHitFunction(NoHitFunction function, int theObjectGOT);
	static void registerOnHitFunction(OnHitByObjectFunction function, int theObjectGOT, int ObjectThatWasHitByGOT);
	static void registerDoCollisionWithTerrainFunction(DoCollisionWithTerrainFunction function);
	static void registerConvertObjectToIndexFunction(ConvertObjectToIndexFunction function);

	struct Result
	{
		Vector m_pointOfCollision_p;
		Vector m_deltaToMoveBack_p;
		Vector m_newReflection_p;
		Vector m_normalOfSurface_p;

		Result();
	};

	static bool intersectAndReflect(Object * const object, Object * const wasHitByThisObject, Result & result);
	static bool intersectAndReflectWithTerrain(Object * const object, Result & result);
	static void addIgnoreIntersect(NetworkId const & networkId1, NetworkId const & networkId2);
	static void removeIgnoreIntersect(NetworkId const & networkId1, NetworkId const & networkId2);
	static int getIgnoredIntersectionCount();

private:
	CollisionCallbackManager();
	CollisionCallbackManager(CollisionCallbackManager const & copy);
	CollisionCallbackManager & operator = (CollisionCallbackManager const & copy);
};

// ======================================================================

#endif

