// ======================================================================
//
// CollisionCallbackManager.cpp
// tford
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/CollisionCallbackManager.h"

#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedLog/Log.h"
#include "sharedMath/Capsule.h"
#include "sharedObject/Object.h"
#include "sharedTerrain/TerrainObject.h"
#include <map>

// ======================================================================

namespace CollisionCallbackManagerNamespace
{
	typedef std::map<int, CollisionCallbackManager::NoHitFunction> NoHitFunctionArray;
	typedef std::map<int, CollisionCallbackManager::OnHitByObjectFunction> OnHitRow;
	typedef std::map<int, OnHitRow> OnHitByObjectFunctionMatrix;

	NoHitFunctionArray ms_NoHitFunctionArray;
	OnHitByObjectFunctionMatrix ms_functionMatrix;

	CollisionCallbackManager::ConvertObjectToIndexFunction ms_convertObjectToIndex = 0;
	CollisionCallbackManager::DoCollisionWithTerrainFunction ms_doCollisionWithTerrain = 0;

	bool ms_debugReport;

	void remove();
	void noCollisionDetectionThisFrame(Object * const object);
	bool collisionDetectionOnHit(Object * const object, Object * const wasHitByThisObject);
	bool doCollisionWithTerrain(Object * const object);

	typedef std::map<NetworkId, std::map<NetworkId, int/*referenceCount*/> > IgnoreIntersectList;
	IgnoreIntersectList s_ignoreIntersectList;
}

using namespace CollisionCallbackManagerNamespace;

// ======================================================================

CollisionCallbackManager::Result::Result()
: m_pointOfCollision_p()
, m_deltaToMoveBack_p()
, m_newReflection_p()
, m_normalOfSurface_p()
{
}

// ======================================================================

void CollisionCallbackManager::install()
{
	InstallTimer const installTimer("CollisionCallbackManager::install");

	CollisionWorld::registerNoCollisionDetectionThisFrame(CollisionCallbackManagerNamespace::noCollisionDetectionThisFrame);
	CollisionWorld::registerDoCollisionDetectionOnHit(CollisionCallbackManagerNamespace::collisionDetectionOnHit);
	CollisionWorld::registerDoCollisionWithTerrain(CollisionCallbackManagerNamespace::doCollisionWithTerrain);

	ms_debugReport = ConfigSharedCollision::getReportEvents();
	DebugFlags::registerFlag(ms_debugReport, "SharedGame/CollisionCallbackManager", "debugReport");

	ExitChain::add(remove, "CollisionCallbackManagerNamespace::remove");
}

// ----------------------------------------------------------------------

void CollisionCallbackManagerNamespace::remove()
{
	DebugFlags::unregisterFlag(ms_debugReport);

	if (!s_ignoreIntersectList.empty())
	{
		//-- If you see this warning, it means that addIgnoreIntersect was called without a corresponding removeIgnoreIntersect.
		WARNING(true, ("CollisionCallbackManagerNamespace::remove() The s_ignoreIntersectList is not empty(%u), this is a sign of a reference counting problem.", s_ignoreIntersectList.size()));
	}
}

// ----------------------------------------------------------------------

void CollisionCallbackManager::registerCanTestCollisionDetectionOnObjectThisFrame(CanTestCollisionDetectionOnObjectThisFrameFunction function)
{
	CollisionWorld::registerCanTestCollisionDetectionOnObjectThisFrame(function);
}

// ----------------------------------------------------------------------

void CollisionCallbackManager::registerNoHitFunction(NoHitFunction function, int theObjectGOT)
{
	ms_NoHitFunctionArray[theObjectGOT] = function;
}

// ----------------------------------------------------------------------

void CollisionCallbackManager::registerOnHitFunction(OnHitByObjectFunction function, int theObjectGOT, int ObjectThatWasHitByGOT)
{
	ms_functionMatrix[theObjectGOT][ObjectThatWasHitByGOT] = function;
}

// ----------------------------------------------------------------------

void CollisionCallbackManager::registerConvertObjectToIndexFunction(ConvertObjectToIndexFunction function)
{
	ms_convertObjectToIndex = function;
}

// ----------------------------------------------------------------------

void CollisionCallbackManager::registerDoCollisionWithTerrainFunction(DoCollisionWithTerrainFunction function)
{
	ms_doCollisionWithTerrain = function;
}

// ----------------------------------------------------------------------

bool CollisionCallbackManager::intersectAndReflect(Object * const object, Object * const wasHitByThisObject, Result & result)
{
	// See if we need to ignore this collision

	if (!s_ignoreIntersectList.empty())
	{
		NetworkId first(object->getNetworkId());
		NetworkId second(wasHitByThisObject->getNetworkId());

		if (second < first)
		{
			first = wasHitByThisObject->getNetworkId();
			second = object->getNetworkId();
		}

		IgnoreIntersectList::const_iterator iterIgnoreIntersectList = s_ignoreIntersectList.find(first);

		if (iterIgnoreIntersectList != s_ignoreIntersectList.end())
		{
			std::map<NetworkId, int>::const_iterator iter = iterIgnoreIntersectList->second.find(second);

			if (iter != iterIgnoreIntersectList->second.end())
			{
				// Ignore this intersection

				return false;
			}
		}
	}

	// The pair is not in the ignore list, so allow the intersection check

	CollisionProperty const * collision = object->getCollisionProperty();
	NOT_NULL(collision);

	CollisionProperty const * wasHitByThisCollision = wasHitByThisObject->getCollisionProperty();
	NOT_NULL(wasHitByThisCollision);

	Capsule queryCapsule_w(collision->getQueryCapsule_w());

	float const radius = queryCapsule_w.getRadius();

	Vector const deltaTraveled_w(queryCapsule_w.getDelta());
	Vector direction_w(deltaTraveled_w);
	if (direction_w.normalize())
	{
		Vector const begin_w(queryCapsule_w.getPointA());
		Vector const end_w(queryCapsule_w.getPointB() + direction_w * radius);

		DEBUG_REPORT_LOG(ms_debugReport, ("begin     = %f %f %f\n", begin_w.x, begin_w.y, begin_w.z));
		DEBUG_REPORT_LOG(ms_debugReport, ("end       = %f %f %f\n", end_w.x, end_w.y, end_w.z));
		DEBUG_REPORT_LOG(ms_debugReport, ("direction = %f %f %f\n", direction_w.x, direction_w.y, direction_w.z));
		DEBUG_REPORT_LOG(ms_debugReport, ("length    = %f\n", deltaTraveled_w.magnitude()));

		Vector normal_l;
		float time = 0.0f;

		Vector const begin_l(wasHitByThisObject->rotateTranslate_w2o(begin_w));
		Vector const end_l(wasHitByThisObject->rotateTranslate_w2o(end_w));

		BaseExtent const * const wasHitByThisBaseExtent_l = wasHitByThisCollision->getExtent_l();
		NOT_NULL(wasHitByThisBaseExtent_l);

		if (wasHitByThisBaseExtent_l->intersect(begin_l, end_l, &normal_l, &time))
		{
			Vector const pointOfCollision_w(Vector::linearInterpolate (begin_w, end_w, time));
			Vector const normal_w(wasHitByThisObject->rotate_o2w(normal_l));

			DEBUG_REPORT_LOG(ms_debugReport, ("\t\tHIT!\n"));
			DEBUG_REPORT_LOG(ms_debugReport, ("\t\ttime  = %f\n", time));
			DEBUG_REPORT_LOG(ms_debugReport, ("\t\tpoint = %f %f %f\n", pointOfCollision_w.x, pointOfCollision_w.y, pointOfCollision_w.z));

			result.m_pointOfCollision_p = pointOfCollision_w;
			result.m_normalOfSurface_p = normal_w;
			result.m_deltaToMoveBack_p = pointOfCollision_w - end_w;
			result.m_newReflection_p = normal_w.reflectIncoming(direction_w);

			DEBUG_REPORT_LOG(ms_debugReport, ("\t\t = %f %f %f\n", result.m_deltaToMoveBack_p.x, result.m_deltaToMoveBack_p.y, result.m_deltaToMoveBack_p.z));

			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

bool CollisionCallbackManager::intersectAndReflectWithTerrain(Object * const object, Result & result)
{
	CollisionProperty const * collision = object->getCollisionProperty();
	NOT_NULL(collision);

	Capsule queryCapsule_w(collision->getQueryCapsule_w());

	float const radius = queryCapsule_w.getRadius();

	Vector const deltaTraveled_w(queryCapsule_w.getDelta());
	Vector direction_w(deltaTraveled_w);
	if (direction_w.normalize())
	{
		Vector const begin_w(queryCapsule_w.getPointA());
		Vector const end_w(queryCapsule_w.getPointB() + direction_w * radius);

		DEBUG_REPORT_LOG(ms_debugReport, ("terrain begin     = %f %f %f\n", begin_w.x, begin_w.y, begin_w.z));
		DEBUG_REPORT_LOG(ms_debugReport, ("terrain end       = %f %f %f\n", end_w.x, end_w.y, end_w.z));
		DEBUG_REPORT_LOG(ms_debugReport, ("terrain direction = %f %f %f\n", direction_w.x, direction_w.y, direction_w.z));
		DEBUG_REPORT_LOG(ms_debugReport, ("terrain length    = %f\n", deltaTraveled_w.magnitude()));

		TerrainObject const * const terrainObject = TerrainObject::getConstInstance();
		if (terrainObject != 0)
		{
			CollisionInfo info;
			if (terrainObject->collide (begin_w, end_w, info))
			{
				#ifdef _DEBUG
				// calculate the parametric time for logging
				float const actualDistance = begin_w.magnitudeBetween(info.getPoint());
				float const attemptedDistance = begin_w.magnitudeBetween(end_w);
				float const parametricTime = (attemptedDistance != 0.0f) ? (actualDistance / attemptedDistance) : 0.0f;
				#endif

				Vector const & pointOfCollision_w = info.getPoint();

				#ifdef _DEBUG
				DEBUG_REPORT_LOG(ms_debugReport, ("\t\tterrain HIT!\n"));
				DEBUG_REPORT_LOG(ms_debugReport, ("\t\tterrain time  = %f\n", parametricTime));
				DEBUG_REPORT_LOG(ms_debugReport, ("\t\tterrain point = %f %f %f\n", pointOfCollision_w.x, pointOfCollision_w.y, pointOfCollision_w.z));
				#endif

				result.m_pointOfCollision_p = pointOfCollision_w;
				result.m_normalOfSurface_p = info.getNormal();
				result.m_deltaToMoveBack_p = pointOfCollision_w - end_w;
				result.m_newReflection_p = result.m_normalOfSurface_p.reflectIncoming(direction_w);

				#ifdef _DEBUG
				DEBUG_REPORT_LOG(ms_debugReport, ("\t\tterrain = %f %f %f\n", result.m_deltaToMoveBack_p.x, result.m_deltaToMoveBack_p.y, result.m_deltaToMoveBack_p.z));
				#endif

				return true;
			}
		}
	}

	return false;
}

// ======================================================================

void CollisionCallbackManagerNamespace::noCollisionDetectionThisFrame(Object * const object)
{
	FATAL(!ms_convertObjectToIndex, ("CollisionCallbackManagerNamespace::noCollisionDetectionThisFrame: ms_convertObjectToIndex == nullptr."));
	FATAL(!object, ("CollisionCallbackManagerNamespace::noCollisionDetectionThisFrame: object == nullptr."));

	int const index = ms_convertObjectToIndex(object);

	NoHitFunctionArray::const_iterator ii = ms_NoHitFunctionArray.find(index);

	CollisionCallbackManager::NoHitFunction const function = (ii != ms_NoHitFunctionArray.end()) ? ii->second : 0;

	if (function)
	{
		(*function)(object);
	}
}

// ----------------------------------------------------------------------

bool CollisionCallbackManagerNamespace::collisionDetectionOnHit(Object * const object, Object * const wasHitByThisObject)
{
	FATAL(!ms_convertObjectToIndex, ("CollisionCallbackManagerNamespace::collisionDetectionOnHit: ms_convertObjectToIndex == nullptr."));
	FATAL(!object, ("CollisionCallbackManagerNamespace::collisionDetectionOnHit: object == nullptr."));
	FATAL(!wasHitByThisObject, ("CollisionCallbackManagerNamespace::collisionDetectionOnHit: wasHitByThisObject == nullptr."));

	CollisionCallbackManager::OnHitByObjectFunction function = 0;

	int const objectColumn = ms_convertObjectToIndex(object);
	int const wasHitByThisObjectRow = ms_convertObjectToIndex(wasHitByThisObject);

	OnHitByObjectFunctionMatrix::const_iterator ii = ms_functionMatrix.find(objectColumn);

	if (ii != ms_functionMatrix.end())
	{
		OnHitRow const & row = ii->second;

		OnHitRow::const_iterator jj = row.find(wasHitByThisObjectRow);

		function = (jj != row.end()) ? jj->second : 0;
	}

	if (function)
	{
		return (*function)(object, wasHitByThisObject);
	}

	return false;
}

// ----------------------------------------------------------------------

bool CollisionCallbackManagerNamespace::doCollisionWithTerrain(Object * const object)
{
	if (ms_doCollisionWithTerrain)
	{
		return (*ms_doCollisionWithTerrain)(object);
	}
	return false;
}

// ----------------------------------------------------------------------

void CollisionCallbackManager::addIgnoreIntersect(NetworkId const & networkId1, NetworkId const & networkId2)
{
	NetworkId first(networkId1);
	NetworkId second(networkId2);

	if (second < first)
	{
		first = networkId2;
		second = networkId1;
	}

	// See if this pair is already in the list

	IgnoreIntersectList::iterator iterIgnoreIntersectList = s_ignoreIntersectList.find(first);

	if (iterIgnoreIntersectList != s_ignoreIntersectList.end())
	{
		std::map<NetworkId, int>::iterator iter = iterIgnoreIntersectList->second.find(second);

		if (iter != iterIgnoreIntersectList->second.end())
		{
			// Already in the list, don't add it again, just ref count it

			++iter->second;

			//LOG("space_debug_ai", ("CollisionCallbackManager::addIgnoreIntersect() networkId1(%s) networkId1(%s) REF COUNTING IGNORE", networkId1.getValueString().c_str(), networkId2.getValueString().c_str()));

			return;
		}
	}

	// The pair is not in the list so add it

	std::map<NetworkId, int> newEntry;
	newEntry.insert(std::make_pair(second, 1));
	s_ignoreIntersectList.insert(std::make_pair(first, newEntry));

	//LOG("space_debug_ai", ("CollisionCallbackManager::addIgnoreIntersect() networkId1(%s) networkId1(%s) ADDING NEW IGNORE", networkId1.getValueString().c_str(), networkId2.getValueString().c_str()));
}

// ----------------------------------------------------------------------

void CollisionCallbackManager::removeIgnoreIntersect(NetworkId const & networkId1, NetworkId const & networkId2)
{
	//LOG("space_debug_ai", ("CollisionCallbackManager::removeIgnoreIntersect() networkId1(%s) networkId1(%s)", networkId1.getValueString().c_str(), networkId2.getValueString().c_str()));

	NetworkId first(networkId1);
	NetworkId second(networkId2);

	if (second < first)
	{
		first = networkId2;
		second = networkId1;
	}

	// See if this pair is already in the list

	IgnoreIntersectList::iterator iterIgnoreIntersectList = s_ignoreIntersectList.find(first);

	if (iterIgnoreIntersectList != s_ignoreIntersectList.end())
	{
		std::map<NetworkId, int>::iterator iter = iterIgnoreIntersectList->second.find(second);

		if (iter != iterIgnoreIntersectList->second.end())
		{
			//LOG("space_debug_ai", ("CollisionCallbackManager::removeIgnoreIntersect() networkId1(%s) networkId1(%s) MATCH FOUND", networkId1.getValueString().c_str(), networkId2.getValueString().c_str()));

			if (--iter->second <= 0)
			{
				iterIgnoreIntersectList->second.erase(iter);

				if (iterIgnoreIntersectList->second.empty())
				{
					s_ignoreIntersectList.erase(iterIgnoreIntersectList);
				}

				//LOG("space_debug_ai", ("CollisionCallbackManager::removeIgnoreIntersect() networkId1(%s) networkId1(%s) REMOVING IGNORE", networkId1.getValueString().c_str(), networkId2.getValueString().c_str()));
			}
		}
	}
}

// ----------------------------------------------------------------------

int CollisionCallbackManager::getIgnoredIntersectionCount()
{
	return static_cast<int>(s_ignoreIntersectList.size());
}

// ======================================================================
