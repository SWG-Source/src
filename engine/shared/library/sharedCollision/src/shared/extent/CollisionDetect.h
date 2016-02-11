// ======================================================================
//
// CollisionDetect.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CollisionDetect_H
#define INCLUDED_CollisionDetect_H

#include "sharedMath/Range.h"

class BaseExtent;
class Object;
class Vector;
class Sphere;
class Capsule;

// ======================================================================

class DetectResult
{
public:

	DetectResult()
	: collided(false),
	  collisionTime(Range::empty),
	  extentA(nullptr),
	  extentB(nullptr)
	{
	}

	DetectResult ( bool result )
	: collided(result),
	  collisionTime( result ? Range::inf : Range::empty ),
	  extentA(nullptr),
	  extentB(nullptr)
	{
	}
	
	DetectResult ( bool result, BaseExtent const * a, BaseExtent const * b )
	: collided(result),
	  collisionTime( result ? Range::inf : Range::empty ),
	  extentA(a),
	  extentB(b)
	{
	}
	
	DetectResult ( bool result, Range time, BaseExtent const * a, BaseExtent const * b )
	: collided(result),
	  collisionTime(time),
	  extentA(a),
	  extentB(b)
	{
	}
	
	// ----------
	
	bool                collided;
	Range               collisionTime;
	BaseExtent const *  extentA;
	BaseExtent const *  extentB;
};

// ----------

class CollisionDetect
{
public:

	// Tests that use velocity _must_ return a collision time range clamped to (0,1)

	static DetectResult     testObjects             ( Object const * objA, Object const * objB );
	static DetectResult     testObjects             ( Object const * objA, Vector const & velocity, Object const * objB );
	static DetectResult     testObjectsAsCylinders  ( Object const * objA, Object const * objB );
	static DetectResult     testObjectExtent        ( Object const * objA, BaseExtent const * extentB );

	static DetectResult     testExtents             ( BaseExtent const * extentA, BaseExtent const * extentB );
	static DetectResult     testExtentsAsCylinders  ( BaseExtent const * extentA, BaseExtent const * extentB );
	static DetectResult     testExtents             ( BaseExtent const * extentA, Vector const & velocity, BaseExtent const * extentB );

	static DetectResult     testSphereExtent        ( Sphere const & sphereA, BaseExtent const * extentB );
	static DetectResult     testSphereExtent        ( Sphere const & sphereA, Vector const & velocity, BaseExtent const * extentB );
	static DetectResult     testSphereObject        ( Sphere const & sphereA_w, Object const * objB );
	static DetectResult     testSphereObject        ( Sphere const & sphereA_w, Vector const & velocity, Object const * objB );

	static DetectResult     testCapsuleExtent       ( Capsule const & capsuleA, BaseExtent const * extentB );
	static DetectResult     testCapsuleObject       ( Capsule const & capsuleA_w, Object const * objB );

	static DetectResult     testCapsuleObjectAgainstAllTypes(Capsule const & capsule_w, Object const * objB);

	static void             resetTestCounter        ( void );
	static int              getTestCounter          ( void );

protected:

	static DetectResult     testSimpleSimple        ( BaseExtent const * extentA, BaseExtent const * extentB );
	static DetectResult     testComponentAny        ( BaseExtent const * extentA, BaseExtent const * extentB );
	static DetectResult     testAnyComponent        ( BaseExtent const * extentA, BaseExtent const * extentB );
	static DetectResult     testDetailAny           ( BaseExtent const * extentA, BaseExtent const * extentB );
	static DetectResult     testAnyDetail           ( BaseExtent const * extentA, BaseExtent const * extentB );

	static DetectResult     testSimpleSimple        ( BaseExtent const * extentA, Vector const & velocity, BaseExtent const * extentB );
	static DetectResult     testComponentAny        ( BaseExtent const * extentA, Vector const & velocity, BaseExtent const * extentB );
	static DetectResult     testAnyComponent        ( BaseExtent const * extentA, Vector const & velocity, BaseExtent const * extentB );
	static DetectResult     testDetailAny           ( BaseExtent const * extentA, Vector const & velocity, BaseExtent const * extentB );
	static DetectResult     testAnyDetail           ( BaseExtent const * extentA, Vector const & velocity, BaseExtent const * extentB );
};

// ======================================================================

#endif

