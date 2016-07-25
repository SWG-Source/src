// ======================================================================
//
// LineOfSightCache.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/LineOfSightCache.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Clock.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/PortalProperty.h"
#include "sharedUtility/Location.h"

#include <queue>

// ======================================================================

namespace LineOfSightCacheNamespace
{
	typedef std::pair<Object const *, Object const *> ObjectPair;
	typedef std::pair<Object const *, Location> ObjectLocationPair;
	typedef std::map<ObjectPair, bool> LineOfSightCacheMap;
	typedef std::map<ObjectLocationPair, bool> LineOfSightLocationCacheMap;
	typedef std::pair<ObjectPair, unsigned long> LineOfSightCacheExpireQueueEntry;
	typedef std::pair<ObjectLocationPair, unsigned long> LineOfSightLocationCacheExpireQueueEntry;
	typedef std::queue<LineOfSightCacheExpireQueueEntry> LineOfSightCacheExpireQueue;
	typedef std::queue<LineOfSightLocationCacheExpireQueueEntry> LineOfSightLocationCacheExpireQueue;

	LineOfSightCacheMap s_lineOfSightCacheMap;
	LineOfSightCacheExpireQueue s_lineOfSightCacheExpireQueue;
	LineOfSightLocationCacheMap s_lineOfSightLocationCacheMap;
	LineOfSightLocationCacheExpireQueue s_lineOfSightLocationCacheExpireQueue;
}

using namespace LineOfSightCacheNamespace;

// ======================================================================

void LineOfSightCache::update()
{
	unsigned long const frameStartTime = Clock::getFrameStartTimeMs();

	while (!s_lineOfSightCacheExpireQueue.empty())
	{
		LineOfSightCacheExpireQueueEntry const &entry = s_lineOfSightCacheExpireQueue.front();
		if (static_cast<int>(entry.second - frameStartTime) > 0)
			break;
		IGNORE_RETURN(s_lineOfSightCacheMap.erase(entry.first));
		s_lineOfSightCacheExpireQueue.pop();
	}
	while (!s_lineOfSightLocationCacheExpireQueue.empty())
	{
		LineOfSightLocationCacheExpireQueueEntry const &entry = s_lineOfSightLocationCacheExpireQueue.front();
		if (static_cast<int>(entry.second - frameStartTime) > 0)
			break;
		IGNORE_RETURN(s_lineOfSightLocationCacheMap.erase(entry.first));
		s_lineOfSightLocationCacheExpireQueue.pop();
	}
}

// ----------------------------------------------------------------------

bool LineOfSightCache::checkLOS(Object const &a, Object const &b)
{
	PROFILER_AUTO_BLOCK_DEFINE("LineOfSightCache:checkLOS");

	Object const *source = ContainerInterface::getFirstParentInWorld(a);
	Object const *target = ContainerInterface::getFirstParentInWorld(b);

	if (!source || !target)
		return false;

	if (source == target)
		return true;

	if (source > target)
		std::swap(source, target);

	std::pair<LineOfSightCacheMap::iterator, bool> result = s_lineOfSightCacheMap.insert(std::make_pair(ObjectPair(source, target), false));

	if (result.second)
	{
		PROFILER_AUTO_BLOCK_DEFINE("LineOfSightCache:checkLOS (uncached)");

		s_lineOfSightCacheExpireQueue.push(
			LineOfSightCacheExpireQueueEntry(
				ObjectPair(source, target),
				Clock::getFrameStartTimeMs() + ConfigServerGame::getLineOfSightCacheDurationMs()));

		CellProperty const * const sourceCell = source->getParentCell();
		CellProperty const * const targetCell = target->getParentCell();

		// if source and target are in the same cell in a player structure, skip LOS check;
		// this is similar to the skip collision check if the object is in a player structure
		// (see CollisionProperty::canCollideWith)
		if (sourceCell && (sourceCell == targetCell) && (sourceCell != CellProperty::getWorldCellProperty()))
		{
			PortalProperty const * portalProperty = sourceCell->getPortalProperty();

			if (portalProperty)
			{
				IsPlayerHouseHook hook = ConfigSharedCollision::getIsPlayerHouseHook();

				if (hook && hook(&portalProperty->getOwner()))
				{
					(*result.first).second = true; // line of sight is clear
					return true;
				}
			}
		}

		Vector const sourcePos(source->getPosition_c());
		Vector const targetPos(target->getPosition_c());

		float const sourceHeight = sourcePos.y + ConfigServerGame::getLineOfSightCacheMinHeight();
		float const targetHeight = targetPos.y + ConfigServerGame::getLineOfSightCacheMinHeight();

		CollisionProperty const * const sourceCollision = NON_NULL(source->getCollisionProperty());
		CollisionProperty const * const targetCollision = NON_NULL(target->getCollisionProperty());

		if (!sourceCollision->isInCollisionWorld())
			sourceCollision->updateExtents();
		if (!targetCollision->isInCollisionWorld())
			targetCollision->updateExtents();

		BaseExtent const * const sourceExtent = sourceCollision ? sourceCollision->getExtent_p() : 0;
		BaseExtent const * const targetExtent = targetCollision ? targetCollision->getExtent_p() : 0;

		ServerObject const *sourceServerObject = source->asServerObject();
		ServerObject const *targetServerObject = target->asServerObject();

		CreatureObject const *sourceCreatureObject = (sourceServerObject) ? sourceServerObject->asCreatureObject() : 0;
		CreatureObject const *targetCreatureObject = (targetServerObject) ? targetServerObject->asCreatureObject() : 0;

		Postures::Enumerator sourcePosture = (sourceCreatureObject) ? sourceCreatureObject->getPosture() : Postures::Invalid;
		Postures::Enumerator targetPosture = (targetCreatureObject) ? targetCreatureObject->getPosture() : Postures::Invalid;

		Vector sourceTop = sourcePos;
		Vector targetTop = targetPos;

		// first process special handling for creature objects
		if (sourceCreatureObject)
		{
			if (sourceExtent)
			{
				sourceTop = sourceExtent->getCenter();
				if (sourceCreatureObject->isPlayerControlled())
				{
					if (sourcePosture == Postures::Upright)
						sourceTop.y += sourceExtent->getRadius() * ConfigSharedCollision::getLosUprightScale();
					else if (sourcePosture == Postures::Prone || sourcePosture == Postures::LyingDown || sourcePosture == Postures::KnockedDown)
						sourceTop.y -= sourceExtent->getRadius() * ConfigSharedCollision::getLosProneScale();
					else if (sourcePosture == Postures::Crouched || sourcePosture == Postures::Sitting)
						sourceTop.y += sourceExtent->getRadius() * ConfigSharedCollision::getLosUprightScale() * 0.5f;
				}
				else
				{
					// we currently don't have a good algorithm for a LOS point on non-player creatures - use extent center
				}
			}
		}
		else
		{
			if (sourceExtent)
			{
				// if the extent exists, use the top of it
				sourceTop = sourceExtent->getCenter();
				sourceTop.y += sourceExtent->getRadius();
			}
			else
			{
				const Sphere sourceSphere = sourceCollision ? sourceCollision->getBoundingSphere_w() : Sphere(sourceTop, 0.0f);
				if (sourceSphere.getRadius() != 0.0f)
				{
					// we move the point up to the top of the collision sphere
					sourceTop = source->getTransform_p2w().rotateTranslate_p2l(sourceSphere.getCenter());
					sourceTop.y += sourceSphere.getRadius();
				}
			}
		}

		if (targetCreatureObject)
		{
			if (targetExtent)
			{
				targetTop = targetExtent->getCenter();
				if (targetCreatureObject->isPlayerControlled())
				{
					if (targetPosture == Postures::Upright)
						targetTop.y += targetExtent->getRadius() * ConfigSharedCollision::getLosUprightScale();
					else if (targetPosture == Postures::Prone || targetPosture == Postures::LyingDown || targetPosture == Postures::KnockedDown)
						targetTop.y -= targetExtent->getRadius() * ConfigSharedCollision::getLosProneScale();
					else if (targetPosture == Postures::Crouched || targetPosture == Postures::Sitting)
						targetTop.y += targetExtent->getRadius() * ConfigSharedCollision::getLosUprightScale() * 0.5f;
				}
				else
				{
					// we currently don't have a good algorithm for a LOS point on non-player creatures - use extent center
				}
			}
		}
		else
		{
			if (targetExtent)
			{
				// if the extent exists, use the top of it
				targetTop = targetExtent->getCenter();
				targetTop.y += targetExtent->getRadius();
			}
			else
			{
				const Sphere targetSphere = targetCollision ? targetCollision->getBoundingSphere_w() : Sphere(targetTop, 0.0f);
				if (targetSphere.getRadius() != 0.0f)
				{
					// we move the point up to the top of the collision sphere
					targetTop = target->getTransform_p2w().rotateTranslate_p2l(targetSphere.getCenter());
					targetTop.y += targetSphere.getRadius();
				}
			}
		}

		if (sourceTop.y < sourceHeight)
			sourceTop.y = sourceHeight;
		if (targetTop.y < targetHeight)
			targetTop.y = targetHeight;

		// Check both directions for obstructions
		QueryInteractionResult qirResult = QIR_None;
		float outHitTime = 0.f;
		Object const *outHitObject = 0;

		qirResult = CollisionWorld::queryInteraction(
			sourceCell, sourceTop,
			targetCell, targetTop,
			source,
			!ConfigSharedCollision::getIgnoreTerrainLos(),
			ConfigSharedCollision::getGenerateTerrainLos(),
			ConfigSharedCollision::getTerrainLOSMinDistance(),
			ConfigSharedCollision::getTerrainLOSMaxDistance(),
			outHitTime,
			outHitObject);

		if (qirResult == QIR_None || outHitObject == target)
		{
			// We don't need to do terrain LOS in the opposite direction if the
			// initial terrain LOS check was done using all possible terrain
			// between the 2 points (generating any missing terrain in the process).
			qirResult = CollisionWorld::queryInteraction(
				targetCell, targetTop,
				sourceCell, sourceTop,
				target,
				(!ConfigSharedCollision::getIgnoreTerrainLos() && !ConfigSharedCollision::getGenerateTerrainLos()),
				false,
				ConfigSharedCollision::getTerrainLOSMinDistance(),
				ConfigSharedCollision::getTerrainLOSMaxDistance(),
				outHitTime,
				outHitObject);

			if (qirResult == QIR_None || outHitObject == source)
				(*result.first).second = true; // line of sight is clear
		}
	}

	// return the (now) cached result
	return (*result.first).second;
}

// ----------------------------------------------------------------------

bool LineOfSightCache::checkLOS(Object const &a, Location const &b)
{
	PROFILER_AUTO_BLOCK_DEFINE("LineOfSightLocationCache:checkLOS");

	Object const *source = ContainerInterface::getFirstParentInWorld(a);
	if (!source)
		return false;

	// round the target location
	float roundValue = ConfigServerGame::getLineOfSightLocationRoundValue();
	Vector targetPos(b.getCoordinates());
	targetPos.x -= static_cast<float>(fmod(static_cast<float>(targetPos.x), roundValue));
	targetPos.y -= static_cast<float>(fmod(static_cast<float>(targetPos.y), roundValue));
	targetPos.z -= static_cast<float>(fmod(static_cast<float>(targetPos.z), roundValue));
	Location target(targetPos, b.getCell(), b.getSceneIdCrc());

	std::pair<LineOfSightLocationCacheMap::iterator, bool> result = s_lineOfSightLocationCacheMap.insert(std::make_pair(ObjectLocationPair(source, target), false));

	if (result.second)
	{
		PROFILER_AUTO_BLOCK_DEFINE("LineOfSightLocationCache:checkLOS (uncached)");

		s_lineOfSightLocationCacheExpireQueue.push(
			LineOfSightLocationCacheExpireQueueEntry(
				ObjectLocationPair(source, target),
				Clock::getFrameStartTimeMs() + ConfigServerGame::getLineOfSightCacheDurationMs()));

		CellProperty const * const sourceCell = source->getParentCell();
		CellProperty const * targetCell = nullptr;
		if (b.getCell() != NetworkId::cms_invalid)
		{
			Object const * cellObject = NetworkIdManager::getObjectById(b.getCell());
			if (cellObject != nullptr)
				targetCell = ContainerInterface::getCell(*cellObject);
		}
		if (targetCell == nullptr)
			targetCell = CellProperty::getWorldCellProperty();

		// if source and target are in the same cell in a player structure, skip LOS check;
		// this is similar to the skip collision check if the object is in a player structure
		// (see CollisionProperty::canCollideWith)
		if (sourceCell && (sourceCell == targetCell) && (sourceCell != CellProperty::getWorldCellProperty()))
		{
			PortalProperty const * portalProperty = sourceCell->getPortalProperty();

			if (portalProperty)
			{
				IsPlayerHouseHook hook = ConfigSharedCollision::getIsPlayerHouseHook();

				if (hook && hook(&portalProperty->getOwner()))
				{
					(*result.first).second = true; // line of sight is clear
					return true;
				}
			}
		}

		Vector const sourcePos(source->getPosition_c());
		float const sourceHeight = sourcePos.y + ConfigServerGame::getLineOfSightCacheMinHeight();

		CollisionProperty const * const sourceCollision = NON_NULL(source->getCollisionProperty());
		if (!sourceCollision->isInCollisionWorld())
			sourceCollision->updateExtents();

		BaseExtent const * const sourceExtent = sourceCollision ? sourceCollision->getExtent_p() : 0;
		ServerObject const *sourceServerObject = source->asServerObject();
		CreatureObject const *sourceCreatureObject = (sourceServerObject) ? sourceServerObject->asCreatureObject() : 0;
		Postures::Enumerator sourcePosture = (sourceCreatureObject) ? sourceCreatureObject->getPosture() : Postures::Invalid;
		Vector sourceTop(sourcePos);

		// first process special handling for creature objects
		if (sourceCreatureObject)
		{
			if (sourceExtent)
			{
				sourceTop = sourceExtent->getCenter();
				if (sourceCreatureObject->isPlayerControlled())
				{
					if (sourcePosture == Postures::Upright)
						sourceTop.y += sourceExtent->getRadius() * ConfigSharedCollision::getLosUprightScale();
					else if (sourcePosture == Postures::Prone || sourcePosture == Postures::LyingDown || sourcePosture == Postures::KnockedDown)
						sourceTop.y -= sourceExtent->getRadius() * ConfigSharedCollision::getLosProneScale();
					else if (sourcePosture == Postures::Crouched || sourcePosture == Postures::Sitting)
						sourceTop.y += sourceExtent->getRadius() * ConfigSharedCollision::getLosUprightScale() * 0.5f;
				}
				else
				{
					// we currently don't have a good algorithm for a LOS point on non-player creatures - use extent center
				}
			}
		}
		else
		{
			if (sourceExtent)
			{
				// if the extent exists, use the top of it
				sourceTop = sourceExtent->getCenter();
				sourceTop.y += sourceExtent->getRadius();
			}
			else
			{
				const Sphere sourceSphere = sourceCollision ? sourceCollision->getBoundingSphere_w() : Sphere(sourceTop, 0.0f);
				if (sourceSphere.getRadius() != 0.0f)
				{
					// we move the point up to the top of the collision sphere
					sourceTop = source->getTransform_p2w().rotateTranslate_p2l(sourceSphere.getCenter());
					sourceTop.y += sourceSphere.getRadius();
				}
			}
		}

		if (sourceTop.y < sourceHeight)
			sourceTop.y = sourceHeight;

		// Check both directions for obstructions
		QueryInteractionResult qirResult = QIR_None;
		float outHitTime = 0.f;
		Object const *outHitObject = 0;

		qirResult = CollisionWorld::queryInteraction(
			sourceCell, sourceTop,
			targetCell, b.getCoordinates(),
			source,
			!ConfigSharedCollision::getIgnoreTerrainLos(),
			ConfigSharedCollision::getGenerateTerrainLos(),
			ConfigSharedCollision::getTerrainLOSMinDistance(),
			ConfigSharedCollision::getTerrainLOSMaxDistance(),
			outHitTime,
			outHitObject);

		if (qirResult == QIR_None)
		{
			// We don't need to do terrain LOS in the opposite direction if the
			// initial terrain LOS check was done using all possible terrain
			// between the 2 points (generating any missing terrain in the process).
			qirResult = CollisionWorld::queryInteraction(
				targetCell, b.getCoordinates(),
				sourceCell, sourceTop,
				nullptr,
				(!ConfigSharedCollision::getIgnoreTerrainLos() && !ConfigSharedCollision::getGenerateTerrainLos()),
				false,
				ConfigSharedCollision::getTerrainLOSMinDistance(),
				ConfigSharedCollision::getTerrainLOSMaxDistance(),
				outHitTime,
				outHitObject);

			if (qirResult == QIR_None || outHitObject == source)
				(*result.first).second = true; // line of sight is clear
		}
	}

	// return the (now) cached result
	return (*result.first).second;
}

// ======================================================================