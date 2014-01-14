//=======================================================================
//
// ShipTargeting.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//=======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipTargeting.h"

#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedGame/SharedObjectTemplateInterface.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/SharedShipObjectTemplate.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/ShipComponentAttachmentManager.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Object.h"
#include "sharedObject/World.h"

#include <algorithm>
#include <vector>

//=======================================================================

namespace ShipTargetingNamespace
{
	float const cs_maximumSweepTargetingDistance = 2000.0f;
	float const cs_maximumTargetingWidth = 1000.0f;

	void sweepForCloseShips(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject, ShipTargeting::ObjectVector & closeObjects);
	void allShipsInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject, ShipTargeting::ObjectVector & resultObjects);

	void getAllAttachmentHardpoints(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * object, ShipComponentAttachmentManager::HardpointVector & hardpoints);
	float smallestDistanceToObject(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * object, Vector const & targetingObject_w);
	float smallestDotToObject(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * object, Vector const & targetingObject_w, Vector const & targetingObjectFramek_w);
}

// ----------------------------------------------------------------------

void ShipTargetingNamespace::sweepForCloseShips(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject, ShipTargeting::ObjectVector & closeObjects)
{
	NOT_NULL(targetingObject);
	if (targetingObject == 0)  //lint !e774 // previous debug test interupts flow
		return;

	Vector const center_w(targetingObject->getPosition_w());
	Vector const targetingObjectFramek_w(targetingObject->getObjectFrameK_w());
	Capsule const capsule_w(Sphere(center_w, cs_maximumTargetingWidth), targetingObjectFramek_w * cs_maximumSweepTargetingDistance);

	ColliderList colliderList;
	int const queryMask = static_cast<int>(SpatialDatabase::Q_Physicals);
	CollisionWorld::getDatabase()->queryFor(queryMask, CellProperty::getWorldCellProperty(), true, capsule_w, colliderList);

	closeObjects.reserve(colliderList.size());
	ColliderList::const_iterator ii = colliderList.begin();
	ColliderList::const_iterator iiEnd = colliderList.end();
	for (; ii != iiEnd; ++ii)
	{
		CollisionProperty * const closeObjectCollisionProperty = *ii;

		if (closeObjectCollisionProperty != 0)
		{
			Object * const object = &closeObjectCollisionProperty->getOwner();
			if (sharedObjectTemplateInterface.isShipObject(object))
			{
				if (object != targetingObject->getRootParent())
				{
					closeObjects.push_back(object);
				}
			}
		}
	}

	std::sort(closeObjects.begin(), closeObjects.end());
}

// ----------------------------------------------------------------------

void ShipTargetingNamespace::allShipsInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject, ShipTargeting::ObjectVector & resultObjects)
{
	int const tangibleListIndex = static_cast<int>(WOL_Tangible);
	int const numberOfTangibles = World::getNumberOfObjects(tangibleListIndex);
	resultObjects.reserve(static_cast<unsigned int>(numberOfTangibles));

	for (int i = 0; i < numberOfTangibles; ++i)
	{
		Object * const object = World::getObject(tangibleListIndex, i);
		if (object != 0)
		{
			if (sharedObjectTemplateInterface.isShipObject(object))
			{
				if (object != targetingObject->getRootParent())
				{
					resultObjects.push_back(object);
				}
			}
		}
	}

	std::sort(resultObjects.begin(), resultObjects.end());
}

// ----------------------------------------------------------------------

void ShipTargetingNamespace::getAllAttachmentHardpoints(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const object, ShipComponentAttachmentManager::HardpointVector & hardpoints)
{
	if (sharedObjectTemplateInterface.isShipObject(object))
	{
		for (int i = static_cast<int>(ShipChassisSlotType::SCST_first); i != static_cast<int>(ShipChassisSlotType::SCST_num_types); ++i)
		{
			ShipChassisSlotType::Type chassisSlot = static_cast<ShipChassisSlotType::Type>(i);

			uint32 const chassisType = sharedObjectTemplateInterface.getShipChassisType(object);
			uint32 const componentCrc = sharedObjectTemplateInterface.getShipComponentCrc(object, chassisSlot);

			ShipComponentAttachmentManager::TemplateHardpointPairVector const & hardPointPairs =
				ShipComponentAttachmentManager::getAttachmentsForShip (chassisType, componentCrc, chassisSlot);

			ShipComponentAttachmentManager::HardpointVector const & extraHardpoints = ShipComponentAttachmentManager::getExtraHardpointsForComponent(chassisType, componentCrc, chassisSlot);

			int const numberOfHardpointsToAdd = static_cast<int>(hardpoints.size() + hardPointPairs.size() + extraHardpoints.size());
			hardpoints.reserve(static_cast<unsigned int>(numberOfHardpointsToAdd));

			ShipComponentAttachmentManager::TemplateHardpointPairVector::const_iterator ii = hardPointPairs.begin();
			ShipComponentAttachmentManager::TemplateHardpointPairVector::const_iterator iiEnd = hardPointPairs.end();

			for (; ii != iiEnd; ++ii)
			{
				hardpoints.push_back(ii->second);
			}

			hardpoints.insert(hardpoints.end(), extraHardpoints.begin(), extraHardpoints.end());
		}
	}
}

// ----------------------------------------------------------------------

float ShipTargetingNamespace::smallestDistanceToObject(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const object, Vector const & targetingObject_w)
{
	NOT_NULL(object);

	float bestDistanceSqr = FLT_MAX;

	{
		// check against the root first
		Vector const shipToTest_w(object->getPosition_w());
		bestDistanceSqr = (shipToTest_w - targetingObject_w).magnitudeSquared();
	}

	if (ShipTargeting::isCapShipOrSpaceStation(sharedObjectTemplateInterface, object))
	{
		// check against the attachments
		Appearance const * const appearance = object->getAppearance();

		if (appearance != 0)
		{
			ShipComponentAttachmentManager::HardpointVector hardpoints;

			ShipTargetingNamespace::getAllAttachmentHardpoints(sharedObjectTemplateInterface, object, hardpoints);

			if (!hardpoints.empty())
			{
				ShipComponentAttachmentManager::HardpointVector::const_iterator ii = hardpoints.begin();
				ShipComponentAttachmentManager::HardpointVector::const_iterator iiEnd = hardpoints.end();

				for (; ii != iiEnd; ++ii)
				{
					PersistentCrcString const & hardpointName = *ii;

					Transform transform;

					if (appearance->findHardpoint(hardpointName, transform))
					{
						Vector const attachment_w(object->rotateTranslate_o2w(transform.getPosition_p()));
						float const testDistanceSqr = (targetingObject_w - attachment_w).magnitudeSquared();

						if (testDistanceSqr < bestDistanceSqr)
						{
							bestDistanceSqr = testDistanceSqr;
						}				
					}
				}
			}
		}
	}
	return bestDistanceSqr;
}

// ----------------------------------------------------------------------

float ShipTargetingNamespace::smallestDotToObject(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const object, Vector const & targetingObject_w, Vector const & targetingObjectFramek_w)
{
	NOT_NULL(object);

	float bestDot = -1.0f;

	{
		// check against the root first
		Vector const shipToTest_w(object->getPosition_w());
		Vector shipToTestDirection(shipToTest_w - targetingObject_w);
		if (shipToTestDirection.normalize())
		{
			bestDot = targetingObjectFramek_w.dot(shipToTestDirection);
		}
	}

	if (ShipTargeting::isCapShipOrSpaceStation(sharedObjectTemplateInterface, object))
	{
		// check against the attachments
		Appearance const * const appearance = object->getAppearance();

		if (appearance != 0)
		{
			ShipComponentAttachmentManager::HardpointVector hardpoints;

			ShipTargetingNamespace::getAllAttachmentHardpoints(sharedObjectTemplateInterface, object, hardpoints);

			if (!hardpoints.empty())
			{
				ShipComponentAttachmentManager::HardpointVector::const_iterator ii = hardpoints.begin();
				ShipComponentAttachmentManager::HardpointVector::const_iterator iiEnd = hardpoints.end();

				for (; ii != iiEnd; ++ii)
				{
					PersistentCrcString const & hardpointName = *ii;

					Transform transform;

					if (appearance->findHardpoint(hardpointName, transform))
					{
						Vector const attachment_w(object->rotateTranslate_o2w(transform.getPosition_p()));
						Vector shipToTestDirection(attachment_w - targetingObject_w);
						if (shipToTestDirection.normalize())
						{
							float const testDot = targetingObjectFramek_w.dot(shipToTestDirection);

							if (testDot > bestDot)
							{
								bestDot = testDot;
							}
						}
					}
				}
			}
		}
	}

	return bestDot;
}

//=======================================================================

using namespace ShipTargetingNamespace;

//=======================================================================

bool ShipTargeting::isPlayerShip(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const object)
{
	NOT_NULL(object);

	SharedObjectTemplate const * const sharedObjectTemplate = sharedObjectTemplateInterface.getSharedTemplate(object);

	NOT_NULL(sharedObjectTemplate);

	SharedShipObjectTemplate const * const sharedShipObjectTemplate = safe_cast<SharedShipObjectTemplate const *>(sharedObjectTemplate);
	if (sharedShipObjectTemplate != 0)
	{
		return (sharedShipObjectTemplate->getPlayerControlled());
	}

	return false;
}

// ----------------------------------------------------------------------

bool ShipTargeting::isNPCShip(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const object)
{
	return !isPlayerShip(sharedObjectTemplateInterface, object);
}

// ----------------------------------------------------------------------

bool ShipTargeting::isCapShipOrSpaceStation(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const object)
{
	NOT_NULL(object);

	SharedObjectTemplate const * const sharedObjectTemplate = sharedObjectTemplateInterface.getSharedTemplate(object);

	NOT_NULL(sharedObjectTemplate);

	SharedObjectTemplate::GameObjectType const GOT = sharedObjectTemplate->getGameObjectType();

	return (GOT == SharedObjectTemplate::GOT_ship_capital) || (GOT == SharedObjectTemplate::GOT_ship_station);
}

// ----------------------------------------------------------------------

Object * ShipTargeting::closestObjectFromListMeetingCriteria(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject, ShipTargeting::ObjectVector & toTestAgainst, ShipTargeting::TestObjectCriteriaFunction testFunction)
{
	NOT_NULL(targetingObject);
	if (targetingObject == 0)  //lint !e774 // previous debug test interupts flow
		return 0;

	Vector const targetingObject_w(targetingObject->getPosition_w());
	float bestDistanceSqr = FLT_MAX;
	Object * bestTarget = 0;

	ObjectVector::const_iterator ii = toTestAgainst.begin();
	ObjectVector::const_iterator iiEnd = toTestAgainst.end();
	for (; ii != iiEnd; ++ii)
	{
		Object * const objectToTest = *ii;

		if (objectToTest != 0)
		{
			if ((testFunction != 0) && (!testFunction(sharedObjectTemplateInterface, objectToTest)))
			{
				continue;
			}

			float const testDistanceSqr = smallestDistanceToObject(sharedObjectTemplateInterface,
																	objectToTest,
																	targetingObject_w);

			if (testDistanceSqr < bestDistanceSqr)
			{
				bestDistanceSqr = testDistanceSqr;
				bestTarget = objectToTest;
			}
		}
	}

	return bestTarget;
}

// ----------------------------------------------------------------------

Object * ShipTargeting::sweepForClosestShip(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject)
{
	NOT_NULL(targetingObject);
	if (targetingObject == 0)  //lint !e774 // previous debug test interupts flow
		return 0;

	ObjectVector closeObjects;
	sweepForCloseShips(sharedObjectTemplateInterface, targetingObject, closeObjects);
	return closestObjectFromListMeetingCriteria(sharedObjectTemplateInterface, targetingObject, closeObjects);
}

// ----------------------------------------------------------------------

Object * ShipTargeting::sweepForClosestCapShip(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject)
{
	NOT_NULL(targetingObject);
	if (targetingObject == 0)  //lint !e774 // previous debug test interupts flow
		return 0;

	ObjectVector closeObjects;
	sweepForCloseShips(sharedObjectTemplateInterface, targetingObject, closeObjects);
	return closestObjectFromListMeetingCriteria(sharedObjectTemplateInterface, targetingObject, closeObjects, isCapShipOrSpaceStation);
}

// ----------------------------------------------------------------------

Object * ShipTargeting::sweepForClosestUnderReticle(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject)
{
	NOT_NULL(targetingObject);
	if (targetingObject == 0)  //lint !e774 // previous debug test interupts flow
		return 0;

	ObjectVector closeObjects;
	sweepForCloseShips(sharedObjectTemplateInterface, targetingObject, closeObjects);

	Vector const targetingObject_w(targetingObject->getPosition_w());
	Vector const targetingObjectFramek_w(targetingObject->getObjectFrameK_w());
	float bestDot = -1.0f;
	Object * bestTarget = 0;

	ObjectVector::const_iterator ii = closeObjects.begin();
	ObjectVector::const_iterator iiEnd = closeObjects.end();
	for (; ii != iiEnd; ++ii)
	{
		Object * const objectToTest = *ii;

		if ((objectToTest != 0) && (objectToTest != targetingObject->getRootParent()))
		{
			float const testDot = smallestDotToObject(sharedObjectTemplateInterface,
													objectToTest,
													targetingObject_w,
													targetingObjectFramek_w);
			if (testDot > bestDot)
			{
				bestTarget = objectToTest;
				bestDot = testDot;
			}
		}
	}

	return bestTarget;
}

// ----------------------------------------------------------------------

Object * ShipTargeting::sweepForClosestPlayer(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject)
{
	ObjectVector closeObjects;
	sweepForCloseShips(sharedObjectTemplateInterface, targetingObject, closeObjects);
	return closestObjectFromListMeetingCriteria(sharedObjectTemplateInterface, targetingObject, closeObjects, isPlayerShip);
}

// ----------------------------------------------------------------------

Object * ShipTargeting::sweepForClosestNPC(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject)
{
	ObjectVector closeObjects;
	sweepForCloseShips(sharedObjectTemplateInterface, targetingObject, closeObjects);
	return closestObjectFromListMeetingCriteria(sharedObjectTemplateInterface, targetingObject, closeObjects, isNPCShip);
}

// ----------------------------------------------------------------------

Object * ShipTargeting::sweepForClosestShipWithRelationShip(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject, ActorAndTargetHaveReletionship function)
{
	ObjectVector resultObjects;
	sweepForShipsWithRelationship(sharedObjectTemplateInterface, targetingObject, function, resultObjects);
	return closestObjectFromListMeetingCriteria(sharedObjectTemplateInterface, targetingObject, resultObjects);
}

// ----------------------------------------------------------------------

Object * ShipTargeting::closestShipInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject)
{
	NOT_NULL(targetingObject);
	if (targetingObject == 0)  //lint !e774 // previous debug test interupts flow
		return 0;

	ObjectVector closeObjects;
	allShipsInWorld(sharedObjectTemplateInterface, targetingObject, closeObjects);
	return closestObjectFromListMeetingCriteria(sharedObjectTemplateInterface, targetingObject, closeObjects);
}

// ----------------------------------------------------------------------

Object * ShipTargeting::closestCapShipInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject)
{
	NOT_NULL(targetingObject);
	if (targetingObject == 0)  //lint !e774 // previous debug test interupts flow
		return 0;

	ObjectVector closeObjects;
	allShipsInWorld(sharedObjectTemplateInterface, targetingObject, closeObjects);
	return closestObjectFromListMeetingCriteria(sharedObjectTemplateInterface, targetingObject, closeObjects, isCapShipOrSpaceStation);
}

// ----------------------------------------------------------------------

Object * ShipTargeting::closestUnderReticleInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject)
{
	NOT_NULL(targetingObject);
	if (targetingObject == 0)  //lint !e774 // previous debug test interupts flow
		return 0;

	ObjectVector closeObjects;
	allShipsInWorld(sharedObjectTemplateInterface, targetingObject, closeObjects);

	Vector const targetingObject_w(targetingObject->getPosition_w());
	Vector const targetingObjectFramek_w(targetingObject->getObjectFrameK_w());
	float bestDot = -1.0f;
	Object * bestTarget = 0;

	ObjectVector::const_iterator ii = closeObjects.begin();
	ObjectVector::const_iterator iiEnd = closeObjects.end();
	for (; ii != iiEnd; ++ii)
	{
		Object * const objectToTest = *ii;

		if ((objectToTest != 0) && (objectToTest != targetingObject->getRootParent()))
		{
			float const testDot = smallestDotToObject(sharedObjectTemplateInterface,
														objectToTest,
														targetingObject_w,
														targetingObjectFramek_w);
			if (testDot > bestDot)
			{
				bestTarget = objectToTest;
				bestDot = testDot;
			}
		}
	}

	return bestTarget;
}

// ----------------------------------------------------------------------

Object * ShipTargeting::closestPlayerInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject)
{
	ObjectVector closeObjects;
	allShipsInWorld(sharedObjectTemplateInterface, targetingObject, closeObjects);
	return closestObjectFromListMeetingCriteria(sharedObjectTemplateInterface, targetingObject, closeObjects, isPlayerShip);
}

// ----------------------------------------------------------------------

Object * ShipTargeting::closestNPCInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject)
{
	ObjectVector closeObjects;
	allShipsInWorld(sharedObjectTemplateInterface, targetingObject, closeObjects);
	return closestObjectFromListMeetingCriteria(sharedObjectTemplateInterface, targetingObject, closeObjects, isNPCShip);
}

// ----------------------------------------------------------------------

Object * ShipTargeting::closestShipInWorldWithRelationShip(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * targetingObject, ActorAndTargetHaveReletionship function)
{
	ObjectVector resultObjects;
	allShipsWithRelationshipInWorld(sharedObjectTemplateInterface, targetingObject, function, resultObjects);
	return closestObjectFromListMeetingCriteria(sharedObjectTemplateInterface, targetingObject, resultObjects);
}

// ----------------------------------------------------------------------

void ShipTargeting::sweepForShipsWithRelationship(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject, ActorAndTargetHaveReletionship function, ObjectVector & resultObjects)
{
	NOT_NULL(targetingObject);
	NOT_NULL(function);

	ObjectVector closeObjects;
	sweepForCloseShips(sharedObjectTemplateInterface, targetingObject, closeObjects);

	resultObjects.reserve(closeObjects.size());

	ObjectVector::const_iterator ii = closeObjects.begin();
	ObjectVector::const_iterator iiEnd = closeObjects.end();
	for (; ii != iiEnd; ++ii)
	{
		Object * const objectToTest = *ii;
		if (objectToTest != 0)
		{
			if (function(targetingObject, objectToTest))
			{
				resultObjects.push_back(objectToTest);
			}
		}
	}
	std::sort(resultObjects.begin(), resultObjects.end());
}

// ----------------------------------------------------------------------

void ShipTargeting::sweepForShipsUnderDefinedReticle(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject, float targetingFov, ObjectVector & resultObjects)
{
	ObjectVector closeObjects;
	sweepForCloseShips(sharedObjectTemplateInterface, targetingObject, closeObjects);

	Vector const targetingObject_w(targetingObject->getPosition_w());
	Vector const targetingObjectFramek_w(targetingObject->getObjectFrameK_w());
	float const halfOnScreenTargetFov = targetingFov * 0.5f;

	ObjectVector::const_iterator ii = closeObjects.begin();
	ObjectVector::const_iterator iiEnd = closeObjects.end();
	for (; ii != iiEnd; ++ii)
	{
		Object * const objectToTest = *ii;
		if ((objectToTest != 0) && (objectToTest != targetingObject->getRootParent()))
		{
			float const testDot = smallestDotToObject(sharedObjectTemplateInterface,
														objectToTest,
														targetingObject_w,
														targetingObjectFramek_w);
			float const angle = acos(testDot);
			if (angle < halfOnScreenTargetFov)
			{
				resultObjects.push_back(objectToTest);
			}
		}
	}
	std::sort(resultObjects.begin(), resultObjects.end());
}

// ----------------------------------------------------------------------

void ShipTargeting::allShipsWithRelationshipInWorld(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject, ActorAndTargetHaveReletionship function, ObjectVector & resultObjects)
{
	NOT_NULL(targetingObject);
	NOT_NULL(function);

	int const tangibleListIndex = static_cast<int>(WOL_Tangible);
	int const numberOfTangibles = World::getNumberOfObjects(tangibleListIndex);
	resultObjects.reserve(static_cast<unsigned int>(numberOfTangibles));

	for (int i = 0; i < numberOfTangibles; ++i)
	{
		Object * const objectToTest = World::getObject(tangibleListIndex, i);
		if (objectToTest != 0)
		{
			if (sharedObjectTemplateInterface.isShipObject(objectToTest))
			{
				if (objectToTest != targetingObject->getRootParent())
				{
					if (function(targetingObject, objectToTest))
					{
						resultObjects.push_back(objectToTest);
					}
				}
			}
		}
	}

	std::sort(resultObjects.begin(), resultObjects.end());
}


// ----------------------------------------------------------------------

void ShipTargeting::allShipsInWorldUnderDefinedReticle(SharedObjectTemplateInterface const & sharedObjectTemplateInterface, Object const * const targetingObject, float targetingFov, ObjectVector & resultObjects)
{
	Vector const targetingObject_w(targetingObject->getPosition_w());
	Vector const targetingObjectFramek_w(targetingObject->getObjectFrameK_w());
	float const halfOnScreenTargetFov = targetingFov * 0.5f;

	int const tangibleListIndex = static_cast<int>(WOL_Tangible);
	int const numberOfTangibles = World::getNumberOfObjects(tangibleListIndex);
	resultObjects.reserve(static_cast<unsigned int>(numberOfTangibles));

	for (int i = 0; i < numberOfTangibles; ++i)
	{
		Object * const objectToTest = World::getObject(tangibleListIndex, i);
		if ((objectToTest != 0) && (objectToTest != targetingObject->getRootParent()))
		{
			if (sharedObjectTemplateInterface.isShipObject(objectToTest))
			{
				float const testDot = smallestDotToObject(sharedObjectTemplateInterface,
															objectToTest,
															targetingObject_w,
															targetingObjectFramek_w);
				float const angle = acos(testDot);
				if (angle < halfOnScreenTargetFov)
				{
					resultObjects.push_back(objectToTest);
				}
			}
		}
	}

	std::sort(resultObjects.begin(), resultObjects.end());
}

// ----------------------------------------------------------------------

Object * ShipTargeting::getNextTarget(ObjectVector const & objectList, Object const * const currentTarget)
{
	if (!objectList.empty())
	{
		WARNING_DEBUG_FATAL(!std::is_sorted(objectList.begin(), objectList.end()), ("ShipTargeting::getNextTarget: objectList must be sorted"));
		ObjectVector::const_iterator ii = std::lower_bound(objectList.begin(), objectList.end(), currentTarget);

		if (ii == objectList.end())
		{
			// we didn't find the current target in the list.
			// just return the first object in the list
			return *(objectList.begin());
		}
		else
		{
			// advance the iterator by one and wrap if necessary
			++ii;
			if (ii == objectList.end())
			{
				ii = objectList.begin();
			}
			return *ii;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------

Object * ShipTargeting::getPreviousTarget(ObjectVector const & objectList, Object const * const currentTarget)
{
	if (!objectList.empty())
	{
		WARNING_DEBUG_FATAL(!std::is_sorted(objectList.begin(), objectList.end()), ("ShipTargeting::getNextTarget: objectList must be sorted"));
		ObjectVector::const_iterator ii = std::lower_bound(objectList.begin(), objectList.end(), currentTarget);

		if (ii == objectList.end())
		{
			// we didn't find the current target in the list.
			// just return the first object in the list
			return *(objectList.begin());
		}
		else
		{
			// advance the iterator by one and wrap if necessary
			if (ii == objectList.begin())
			{
				ii = objectList.end();
			}
			--ii;
			return *ii;
		}
	}
	return 0;
}


//=======================================================================
