//======================================================================
//
// ShipTargeting.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipTargeting_H
#define INCLUDED_ShipTargeting_H

//======================================================================

class Object;
class SharedObjectTemplateInterface;

//----------------------------------------------------------------------

class ShipTargeting
{
public:

	typedef std::vector<Object *> ObjectVector;

	typedef bool (*ActorAndTargetHaveReletionship)(Object const * actor, Object const * target);

	static bool isPlayerShip(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * object);
	static bool isNPCShip(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * object);
	static bool isCapShipOrSpaceStation(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * object);
protected:

	// TestObjectCriteriaFunction must return true if the object passes the test
	typedef bool (*TestObjectCriteriaFunction)(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * object);
	static 	Object * closestObjectFromListMeetingCriteria(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject, ShipTargeting::ObjectVector & toTestAgainst, TestObjectCriteriaFunction = 0);

	static Object * sweepForClosestShip(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject);
	static Object * sweepForClosestCapShip(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject);
	static Object * sweepForClosestUnderReticle(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject);
	static Object * sweepForClosestPlayer(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject);
	static Object * sweepForClosestNPC(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject);
	static Object * sweepForClosestShipWithRelationShip(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject, ActorAndTargetHaveReletionship function);

	static Object * closestShipInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject);
	static Object * closestCapShipInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject);
	static Object * closestUnderReticleInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject);
	static Object * closestPlayerInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject);
	static Object * closestNPCInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject);
	static Object * closestShipInWorldWithRelationShip(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject, ActorAndTargetHaveReletionship function);

	static void sweepForShipsWithRelationship(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject, ActorAndTargetHaveReletionship function, ObjectVector & resultObjects);
	static void sweepForShipsUnderDefinedReticle(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject, float targetingFov, ObjectVector & resultObjects);

	static void allShipsWithRelationshipInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject, ActorAndTargetHaveReletionship function, ObjectVector & resultObjects);
	static void allShipsInWorldUnderDefinedReticle(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject, float targetingFov, ObjectVector & resultObjects);

	static Object * getNextTarget(ObjectVector const & objectList, Object const * currentTarget);
	static Object * getPreviousTarget(ObjectVector const & objectList, Object const * currentTarget);

private:

	ShipTargeting();
	ShipTargeting(ShipTargeting const & copy);
	ShipTargeting & operator=(ShipTargeting const & copy);

};

//lint -esym(1512, ShipTargeting) // No virtual destructor (because there are no data members at all)

//======================================================================

#endif
