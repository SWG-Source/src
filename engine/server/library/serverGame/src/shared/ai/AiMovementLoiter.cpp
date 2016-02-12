// ======================================================================
//
// AiMovementLoiter.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementLoiter.h"

#include "serverGame/AiLogManager.h"
#include "serverGame/AiMovementArchive.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/ScriptFunctionTable.h"
#include "sharedCollision/BaseExtent.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorLocator.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedCollision/Footprint.h"
#include "sharedCollision/MultiList.h"
#include "sharedCollision/SpatialDatabase.h"
#include "sharedGame/AiDebugString.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedLog/Log.h"
#include "sharedMath/Capsule.h"
#include "sharedObject/CellProperty.h"
#include "sharedTerrain/TerrainObject.h"

// ======================================================================
//
// AiMovementLoiterNamespace
//
// ======================================================================

namespace AiMovementLoiterNamespace
{
	Vector getRandomLoiterPosition_p(Vector const anchorPosition_p, float const minDistance, float const maxDistance);
	bool isPositionOnFloor(Floor const * const floor, Vector const & position_p, float const objectRadius, Vector & floorPosition);

	unsigned int const s_maxCachedAiLocations = 5;
	float const s_failedSelectWaypointSleepTime = 2.0f;
}

using namespace AiMovementLoiterNamespace;

// ----------------------------------------------------------------------
Vector AiMovementLoiterNamespace::getRandomLoiterPosition_p(Vector const anchorPosition_p, float const minDistance, float const maxDistance)
{
	Vector result = anchorPosition_p;
	float const angle = Random::randomReal(PI_TIMES_2);
	float const distance = Random::randomReal(minDistance, maxDistance);

	result.x += distance * cos(angle);
	result.z += distance * sin(angle);

	return result;
}

// ----------------------------------------------------------------------
bool AiMovementLoiterNamespace::isPositionOnFloor(Floor const * const floor, Vector const & position_p, float const radius, Vector & floorPosition_p)
{
	bool result = false;
	FloorMesh const * const floorMesh = (floor != nullptr) ? floor->getFloorMesh() : nullptr;

	if (floorMesh != nullptr)
	{
		// See if the circle fits entirely on the floor
		{
			{
				bool const allowJump = true;
				FloorLocator outLoc;

				if (   floorMesh->findFloorTri(FloorLocator(Vector(position_p.x - radius, position_p.y, position_p.z + radius), 0.0f), allowJump, outLoc)
					&& floorMesh->findFloorTri(FloorLocator(Vector(position_p.x + radius, position_p.y, position_p.z + radius), 0.0f), allowJump, outLoc)
					&& floorMesh->findFloorTri(FloorLocator(Vector(position_p.x + radius, position_p.y, position_p.z - radius), 0.0f), allowJump, outLoc)
					&& floorMesh->findFloorTri(FloorLocator(Vector(position_p.x - radius, position_p.y, position_p.z - radius), 0.0f), allowJump, outLoc)
					&& floorMesh->findFloorTri(FloorLocator(Vector(position_p.x, position_p.y, position_p.z), 0.0f), allowJump, outLoc))
				{
					result = true;
					floorPosition_p = outLoc.getPosition_p();
				}
			}
		}
	}

	return result;
}

// ======================================================================
//
// AiMovementLoiter
//
// ======================================================================

// ----------------------------------------------------------------------
AiMovementLoiter::AiMovementLoiter( AICreatureController * controller)
 : AiMovementWaypoint(controller)
 , m_anchor()
 , m_target()
 , m_sleepTime(1.0f)
 , m_minDistance(0.0f)
 , m_maxDistance(1.0f)
 , m_minDelay(1.0f)
 , m_maxDelay(1.0f)
 , m_cachedAiLocations(new CachedAiLocations)
 , m_goalLocationIndex(-1)
 , m_bubbleCheckResult(BCR_invalid)
{
	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("AiMovementLoiter::AiMovementLoiter1() owner(%s:%s) minDistance(%.2f) maxDistance(%.2f) minDelay(%.2f) maxDelay(%.2f)", controller->getOwner()->getNetworkId().getValueString().c_str(), FileNameUtils::get(controller->getOwner()->getDebugName(), FileNameUtils::fileName).c_str(), m_minDistance, m_maxDistance, m_minDelay, m_maxDelay));

	m_cachedAiLocations->reserve(s_maxCachedAiLocations);
	updateWaypoint();

	CHANGE_STATE( AiMovementLoiter::stateWaiting);
}

// ----------------------------------------------------------------------
AiMovementLoiter::AiMovementLoiter( AICreatureController * controller, CellProperty const * anchorCell, Vector const & anchor, float minDistance, float maxDistance, float minDelay, float maxDelay)
 : AiMovementWaypoint(controller)
 , m_anchor(anchorCell, anchor)
 , m_target()
 , m_sleepTime(minDelay)
 , m_minDistance(minDistance)
 , m_maxDistance(maxDistance)
 , m_minDelay(minDelay)
 , m_maxDelay(maxDelay)
 , m_cachedAiLocations(new CachedAiLocations)
 , m_goalLocationIndex(-1)
 , m_bubbleCheckResult(BCR_invalid)
{
	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("AiMovementLoiter::AiMovementLoiter2() owner(%s:%s) minDistance(%.2f) maxDistance(%.2f) minDelay(%.2f) maxDelay(%.2f)", controller->getOwner()->getNetworkId().getValueString().c_str(), FileNameUtils::get(controller->getOwner()->getDebugName(), FileNameUtils::fileName).c_str(), m_minDistance, m_maxDistance, m_minDelay, m_maxDelay));

	m_cachedAiLocations->reserve(s_maxCachedAiLocations);
	updateWaypoint();

	CHANGE_STATE( AiMovementLoiter::stateWaiting);
}

// ----------------------------------------------------------------------
AiMovementLoiter::AiMovementLoiter( AICreatureController * controller, ServerObject const * anchor, float minDistance, float maxDistance, float minDelay, float maxDelay)
 : AiMovementWaypoint(controller)
 , m_anchor(anchor)
 , m_target()
 , m_sleepTime(minDelay)
 , m_minDistance(minDistance)
 , m_maxDistance(maxDistance)
 , m_minDelay(minDelay)
 , m_maxDelay(maxDelay)
 , m_cachedAiLocations(new CachedAiLocations)
 , m_goalLocationIndex(-1)
 , m_bubbleCheckResult(BCR_invalid)
{
	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("AiMovementLoiter::AiMovementLoiter3() owner(%s:%s) minDistance(%.2f) maxDistance(%.2f) minDelay(%.2f) maxDelay(%.2f)", controller->getOwner()->getNetworkId().getValueString().c_str(), FileNameUtils::get(controller->getOwner()->getDebugName(), FileNameUtils::fileName).c_str(), m_minDistance, m_maxDistance, m_minDelay, m_maxDelay));
	DEBUG_WARNING((m_maxDistance < 1.0f), ("AiMovementLoiter::AiMovementLoiter() maxDistance(%f) < 1.0f", maxDistance));

	m_cachedAiLocations->reserve(s_maxCachedAiLocations);
	updateWaypoint();

	CHANGE_STATE( AiMovementLoiter::stateWaiting);
}

// ----------------------------------------------------------------------
AiMovementLoiter::AiMovementLoiter( AICreatureController * controller, Archive::ReadIterator & source)
 : AiMovementWaypoint(controller, source)
 , m_anchor(source)
 , m_target(source)
 , m_sleepTime(0.0f)
 , m_minDistance(0.0f)
 , m_maxDistance(0.0f)
 , m_minDelay(0.0f)
 , m_maxDelay(0.0f)
 , m_cachedAiLocations(new CachedAiLocations)
 , m_goalLocationIndex(-1)
 , m_bubbleCheckResult(BCR_invalid)
{
	Archive::get(source, m_minDistance);
	Archive::get(source, m_maxDistance);
	Archive::get(source, m_minDelay);
	Archive::get(source, m_maxDelay);

	m_sleepTime = m_minDelay;
	m_cachedAiLocations->reserve(s_maxCachedAiLocations);

	LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("AiMovementLoiter::AiMovementLoiter4() owner(%s:%s) minDistance(%.2f) maxDistance(%.2f) minDelay(%.2f) maxDelay(%.2f)", controller->getOwner()->getNetworkId().getValueString().c_str(), FileNameUtils::get(controller->getOwner()->getDebugName(), FileNameUtils::fileName).c_str(), m_minDistance, m_maxDistance, m_minDelay, m_maxDelay));

	SETUP_SYNCRONIZED_STATE( AiMovementLoiter::stateWaiting);
	SETUP_SYNCRONIZED_STATE( AiMovementLoiter::stateThinking);
	SETUP_SYNCRONIZED_STATE( AiMovementLoiter::stateMoving);
}

// ----------------------------------------------------------------------
AiMovementLoiter::~AiMovementLoiter()
{
	delete m_cachedAiLocations;
	m_cachedAiLocations = nullptr;
}

// ----------------------------------------------------------------------
void AiMovementLoiter::pack( Archive::ByteStream & target) const
{
	AiMovementWaypoint::pack(target);
	m_anchor.pack(target);
	m_target.pack(target);
	Archive::put(target, m_minDistance);
	Archive::put(target, m_maxDistance);
	Archive::put(target, m_minDelay);
	Archive::put(target, m_maxDelay);
}

// ----------------------------------------------------------------------
void AiMovementLoiter::alter(float time)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementLoiter::alter");

	AiMovementWaypoint::alter(time);
}

// ----------------------------------------------------------------------

bool AiMovementLoiter::getHibernateOk ( void ) const
{
	return true;
}

// ----------------------------------------------------------------------
void AiMovementLoiter::getDebugInfo ( std::string & outString) const
{
	AiMovementWaypoint::getDebugInfo(outString);

	FormattedString<256> fs;
	outString += "\n";
	outString += "AiMovementLoiter:\n";

	Vector const anchor = m_anchor.getPosition_w();

	outString += fs.sprintf("Anchor: (%f, %f, %f)\n", anchor.x, anchor.y, anchor.z);
	outString += fs.sprintf("Loiter dist: [%f...%f]\n", m_minDistance, m_maxDistance);
	outString += fs.sprintf("Loiter delay: [%f...%f]\n", m_minDelay, m_maxDelay);
}

// ----------------------------------------------------------------------
AiStateResult AiMovementLoiter::stateWaiting(float /*time*/)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementLoiter::stateWaiting");

	if (m_sleepTime > 0.0f)
	{
		m_controller->sleep(m_sleepTime);
		m_sleepTime = 0.0f;

		return ASR_Done;
	}
	else
	{
		return triggerThinking();
	}
}

// ----------------------------------------------------------------------
AiStateResult AiMovementLoiter::stateThinking(float /*time*/)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementLoiter::stateThinking");

	if (!isAnchorValid())
	{
		// If our anchor somehow becomes invalid, put the AI back on track
		// with a known good anchor position  being the home location
		// of the AI

		Location const & homeLocation = m_controller->getHomeLocation();

		m_anchor = AiLocation(homeLocation.getCell(), homeLocation.getCoordinates());
	}

	if (generateWaypoint())
	{
		// Successfully picked a new loiter location

		m_sleepTime = Random::randomReal(m_minDelay, m_maxDelay);
		return triggerMoving();
	}

	// Failed to pick a new loiter location

	m_sleepTime = s_failedSelectWaypointSleepTime;
	return triggerWaiting();
}

// ----------------------------------------------------------------------
AiStateResult AiMovementLoiter::stateMoving(float time)
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementLoiter::stateMoving");

	if (!isAnchorValid())
	{
		return triggerTargetLost();
	}

	if (m_controller->getStuckCounter() > 5)
	{
		if (m_goalLocationIndex >= 0)
		{
			LOGC(AiLogManager::isLogging(m_controller->getOwner()->getNetworkId()), "debug_ai", ("AiMovementLoiter::stateMoving() owner(%s:%s) waypointCount(%u/%u) I was stuck...removing bad index(%d)...warping to anchor position", m_controller->getOwner()->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_controller->getOwner()->getDebugName(), FileNameUtils::fileName).c_str(), m_cachedAiLocations->size(), s_maxCachedAiLocations, m_goalLocationIndex));

			m_controller->warpTo(m_anchor.getCell(), m_anchor.getPosition_p());
			(*m_cachedAiLocations).erase((*m_cachedAiLocations).begin() + m_goalLocationIndex);
		}
		else
		{
			LOGC(AiLogManager::isLogging(m_controller->getOwner()->getNetworkId()), "debug_ai", ("AiMovementLoiter::stateMoving() owner(%s:%s) I was stuck...and I don't even have any loiter positions!", m_controller->getOwner()->getNetworkId().getValueString().c_str(), FileNameUtils::get(m_controller->getOwner()->getDebugName(), FileNameUtils::fileName).c_str()));
		}

		return triggerThinking();
	}

	return AiMovementWaypoint::stateMoving(time);
}

// ----------------------------------------------------------------------
AiStateResult AiMovementLoiter::triggerWaiting()
{
	m_controller->triggerScriptsWithParam(Scripting::TRIG_LOITER_WAITING, m_sleepTime);

	return AiMovementWaypoint::triggerWaiting();
}

// ----------------------------------------------------------------------
AiStateResult AiMovementLoiter::triggerThinking()
{
	CHANGE_STATE( AiMovementLoiter::stateThinking);

	return ASR_Continue;
}

// ----------------------------------------------------------------------
AiStateResult AiMovementLoiter::triggerMoving()
{
	m_controller->triggerScriptsSimple(Scripting::TRIG_LOITER_MOVING);

	return AiMovementWaypoint::triggerMoving();
}

// ----------------------------------------------------------------------
AiStateResult AiMovementLoiter::triggerWaypoint()
{
	m_controller->triggerScriptsSimple(Scripting::TRIG_LOITER_WAYPOINT);

	return AiMovementWaypoint::triggerWaypoint();
}

// ----------------------------------------------------------------------
AiStateResult AiMovementLoiter::triggerTargetLost()
{
	return AiMovementWaypoint::triggerTargetLost();
}

// ----------------------------------------------------------------------
bool AiMovementLoiter::hasWaypoint() const
{
	return m_target.isValid();
}

// ----------------------------------------------------------------------
AiLocation const & AiMovementLoiter::getWaypoint() const
{
	return m_target;
}

// ----------------------------------------------------------------------
bool AiMovementLoiter::updateWaypoint()
{
	m_target.update();
	m_anchor.update();

	return true;
}

// ----------------------------------------------------------------------
void AiMovementLoiter::clearWaypoint()
{
	m_target = AiLocation();
}

// ----------------------------------------------------------------------
bool AiMovementLoiter::generateWaypoint()
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementLoiter::generateWaypoint");

	bool result = false;
	CreatureObject * const creatureOwner = m_controller->getCreature();

	// If somehow the position of the waypoint changes, then start
	// recalculating the waypoints
	{
		if (m_anchor.hasChanged())
		{
			LOGC(AiLogManager::isLogging(creatureOwner->getNetworkId()), "debug_ai", ("AiMovementLoiter::generateWaypoint(anchorChanged) owner(%s:%s) waypointCount(%u/%u)", creatureOwner->getNetworkId().getValueString().c_str(), FileNameUtils::get(creatureOwner->getDebugName(), FileNameUtils::fileName).c_str(), m_cachedAiLocations->size(), s_maxCachedAiLocations));

			m_anchor.setChanged(false);
			m_cachedAiLocations->clear();
			m_goalLocationIndex = -1;
			m_bubbleCheckResult = BCR_invalid;
		}
	}

	// If we are not at full capacity on waypoints, lets keep creating more
	{
		if (m_cachedAiLocations->size() < m_cachedAiLocations->capacity())
		{
			Vector randomPosition_p = getRandomLoiterPosition_p(m_anchor.getPosition_p(), m_minDistance, m_maxDistance);

			// First check to see if we are standing on a floor

			Floor const * const ownerFloor = CollisionWorld::getFloorStandingOn(*creatureOwner);

			if (ownerFloor != nullptr)
			{
				// The owner is standing on a floor

				Vector floorPosition_p;

				if (isPositionOnFloor(ownerFloor, randomPosition_p, creatureOwner->getRadius(), floorPosition_p))
				{
					m_cachedAiLocations->push_back(AiLocation(m_controller->getCreatureCell(), floorPosition_p));

					LOGC(AiLogManager::isLogging(creatureOwner->getNetworkId()), "debug_ai", ("AiMovementLoiter::generateWaypoint(floor) owner(%s:%s) waypointCount(%u/%u)", creatureOwner->getNetworkId().getValueString().c_str(), FileNameUtils::get(creatureOwner->getDebugName(), FileNameUtils::fileName).c_str(), m_cachedAiLocations->size(), s_maxCachedAiLocations));
				}
			}
			else
			{
				// The owner is standing on terrain

				// Pre-check to see if there is nothing to collide with. This
				// check will use a sphere of the max loiter radius centered
				// around the loiter anchor position.
				{
					if (m_bubbleCheckResult == BCR_invalid)
					{
						bool const restrictToSameCell = true;
						float const radius = m_maxDistance;
						Capsule const capsule(m_anchor.getPosition_p(), m_anchor.getPosition_p(), radius);
						ColliderList collidedWith;

						CollisionWorld::getDatabase()->queryFor(SpatialDatabase::Q_Static, m_controller->getCreatureCell(), restrictToSameCell, capsule, collidedWith);

						// Check out the collision results
						{
							m_bubbleCheckResult = BCR_success;
							ColliderList::const_iterator iterCollider = collidedWith.begin();

							for (; iterCollider != collidedWith.end(); ++iterCollider)
							{
								CollisionProperty * const collisionProperty = *iterCollider;
								Object const & object = collisionProperty->getOwner();

								if (object.getNetworkId() != creatureOwner->getNetworkId())
								{
									// We have collided with something

									m_bubbleCheckResult = BCR_failure;
									break;
								}
							}
						}
					}
				}

				// Make sure the loiter position is not on impassible terrain

				TerrainObject const * const terrainObject = TerrainObject::getConstInstance();

				if (   (terrainObject != nullptr)
				    && terrainObject->isPassable(randomPosition_p))
				{
					// Snap the random position to the terrain
					{
						float terrainHeight;

						if (terrainObject->getHeightForceChunkCreation(randomPosition_p, terrainHeight))
						{
							randomPosition_p = Vector(randomPosition_p.x, terrainHeight, randomPosition_p.z);
						}
					}

					bool collision = false;

					// If we failed the bubble check, see if we can straight line to the random position
					{
						if (m_bubbleCheckResult != BCR_success)
						{
							// We know there is some objects around us so try to find some good
							// positions by casting a ray from the anchor location to the random position

							bool const restrictToSameCell = true;
							float const radius = creatureOwner->getCollisionSphereExtent_o().getRadius();
							Capsule const capsule(m_anchor.getPosition_p(), randomPosition_p, radius);
							ColliderList collidedWith;

							CollisionWorld::getDatabase()->queryFor(SpatialDatabase::Q_Static, m_controller->getCreatureCell(), restrictToSameCell, capsule, collidedWith);

							// Check out the collision results
							{
								ColliderList::const_iterator iterCollider = collidedWith.begin();

								for (; iterCollider != collidedWith.end(); ++iterCollider)
								{
									CollisionProperty * const collisionProperty = *iterCollider;
									Object const & object = collisionProperty->getOwner();

									if (object.getNetworkId() != creatureOwner->getNetworkId())
									{
										BaseExtent const * const extent = collisionProperty->getExtent_l();

										if (extent != nullptr)
										{
											Vector const begin_o(object.rotateTranslate_w2o(m_anchor.getPosition_p()));
											Vector const end_o(object.rotateTranslate_w2o(randomPosition_p));

											if (extent->intersect(begin_o, end_o))
											{
												// We have collided with something

												collision = true;
												break;
											}
										}
									}
								}
							}
						}
					}

					// If there was no collisions to this point then save it
					{
						if (!collision)
						{
							m_cachedAiLocations->push_back(AiLocation(m_controller->getCreatureCell(), randomPosition_p));
							LOGC(AiLogManager::isLogging(creatureOwner->getNetworkId()), "debug_ai", ("AiMovementLoiter::generateWaypoint(terrain) owner(%s:%s) waypointCount(%u/%u)", creatureOwner->getNetworkId().getValueString().c_str(), FileNameUtils::get(creatureOwner->getDebugName(), FileNameUtils::fileName).c_str(), m_cachedAiLocations->size(), s_maxCachedAiLocations));
						}
					}
				}
			}
		}
	}

	// If we have some cached locations, then lets pick one
	{
		if (!m_cachedAiLocations->empty())
		{
			int const cachedAiLocationCount = static_cast<int>(m_cachedAiLocations->size());
			int const index = rand() % cachedAiLocationCount;

			if (index != m_goalLocationIndex)
			{
				result = true;
				m_goalLocationIndex = index;
				m_target = (*m_cachedAiLocations)[static_cast<unsigned int>(index)];
			}
			else
			{
				m_target.clear();
			}
		}
	}

	return result;
}

#ifdef _DEBUG
// ----------------------------------------------------------------------
void AiMovementLoiter::addDebug(AiDebugString & aiDebugString)
{
	AiMovementWaypoint::addDebug(aiDebugString);

	FormattedString<512> fs;
	aiDebugString.addText(fs.sprintf("LOITER %s\n%s\ndist[%.0f...%.0f]\ndelay[%.1f...%.1f]\n", (m_anchor.getObject() != nullptr) ? "TARGET" : "POSITION", m_stateName.c_str(), m_minDistance, m_maxDistance, m_minDelay, m_maxDelay), PackedRgb::solidCyan);

	//if (m_bubbleCheckResult == BCR_invalid)
	//{
	//	aiDebugString.addText("BubbleCheckResult: invalid\n", PackedRgb::solidCyan);
	//}
	//else if (m_bubbleCheckResult == BCR_success)
	//{
	//	aiDebugString.addText("BubbleCheckResult: success\n", PackedRgb::solidCyan);
	//}
	//else if (m_bubbleCheckResult == BCR_failure)
	//{
	//	aiDebugString.addText("BubbleCheckResult: failure\n", PackedRgb::solidCyan);
	//}

	// Draw the current anchor position
	{
		Object const * const anchorObject = m_anchor.getObject();

		if (anchorObject != nullptr)
		{
			// We are anchored to a moving target

			aiDebugString.addLineToPosition(anchorObject->getPosition_w(), PackedRgb::solidYellow);
		}
		else
		{
			// We are anchored to a stationary target

			Location const & homeLocation = m_controller->getHomeLocation();

			if (   (homeLocation.getCoordinates() != m_anchor.getPosition_p())
			    && (homeLocation.getCell() != m_anchor.getCellId()))
			{
				aiDebugString.addLineToPosition(m_anchor.getPosition_w(), PackedRgb::solidYellow);
			}
		}
	}

	// Draw all the current cached locations
	{
		CachedAiLocations::const_iterator iterCachedAiLocations = m_cachedAiLocations->begin();

		for (; iterCachedAiLocations != m_cachedAiLocations->end(); ++iterCachedAiLocations)
		{
			AiLocation const & location = *iterCachedAiLocations;
			Vector const & center_w = location.getPosition_w();
			float const radius = 0.25f;
			aiDebugString.addCircle(center_w, radius, PackedRgb::solidCyan);
		}
	}
}
#endif // _DEBUG

// ----------------------------------------------------------------------
AiMovementType AiMovementLoiter::getType() const
{
	return AMT_loiter;
}

// ----------------------------------------------------------------------
AiMovementLoiter * AiMovementLoiter::asAiMovementLoiter()
{
	return this;
}

// ----------------------------------------------------------------------
bool AiMovementLoiter::isAnchorValid() const
{
	bool result = true;

	if (!m_anchor.isValid())
	{
		result = false;
	}
	else
	{
		TangibleObject const * const tangibleObject = TangibleObject::asTangibleObject(m_anchor.getObject());

		if (tangibleObject != nullptr)
		{
			if (tangibleObject->isInCombat())
			{
				result = false;
			}
		}
	}

	return result;
}

// ======================================================================
