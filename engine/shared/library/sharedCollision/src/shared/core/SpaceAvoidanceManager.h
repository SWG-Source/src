// ======================================================================
// 
// SpaceAvoidanceManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#ifndef INCLUDED_SpaceAvoidanceManager_H
#define INCLUDED_SpaceAvoidanceManager_H

class Object;
class Transform;
class Vector;

// ----------------------------------------------------------------------
class SpaceAvoidanceManager
{
public:

	static void install();
	static bool getAvoidancePosition(Object const & object, Vector const & sweepVector, Vector const & targetPosition, Vector & avoidancePosition_w, bool collideWithDynamicObjects = false);
	static bool getAvoidancePosition(Transform const & transform_o2w, float const collisionRadius, Vector const & sweepVector, Vector const & targetPosition, Vector & avoidancePosition_w, const Object * const excludeObject, bool useDynamics = false);

private:

	static void remove();

	// Disable

	SpaceAvoidanceManager();
	~SpaceAvoidanceManager();
	SpaceAvoidanceManager(SpaceAvoidanceManager const &);
	SpaceAvoidanceManager & operator =(SpaceAvoidanceManager const &);
};

// ======================================================================

#endif // INCLUDED_SpaceAvoidanceManager_H
