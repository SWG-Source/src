// ======================================================================
//
// AiMovementSwarm.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementSwarm.h"

#include "serverGame/AiCreatureController.h"
#include "serverGame/AiMovementArchive.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverUtility/ServerClock.h"
#include "sharedDebug/Profiler.h"
#include "sharedLog/Log.h"
#include "sharedObject/World.h"

#include <unordered_map>


using namespace Scripting;

namespace AiMovementSwarmNamespace
{
	typedef std::unordered_map<CachedNetworkId, std::vector<AiMovementSwarm::CreatureWatcher>, CachedNetworkId> targetMap;
	typedef std::unordered_map<CachedNetworkId, Vector, CachedNetworkId> offsetMap;

	// map of swarm targets to the creatures swarming them
	targetMap s_swarmMap;
	// map of the creatures moving to where they want to move to (generated each frame)
	offsetMap s_offsetMap;
	// frame we last updated the goals in
	unsigned long s_lastFrame = 0;
	// how close we'll allow a creature to get to their goal before stopping them
	const float s_goalBuffer = 0.5f;
}
using namespace AiMovementSwarmNamespace;


// ======================================================================

AiMovementSwarm::AiMovementSwarm( AICreatureController * controller ) :
	AiMovementFollow(controller),
	m_offset(0)
{
}

AiMovementSwarm::AiMovementSwarm( AICreatureController * controller, ServerObject const * target ) :
	AiMovementFollow(controller, target, 0, 0),
	m_offset(0)
{
	init();
}

// ----------

AiMovementSwarm::AiMovementSwarm( AICreatureController * controller, ServerObject const * target, float offset) :
	AiMovementFollow(controller, target, 0, 0),
	m_offset(offset)
{
	init();
}

// ----------

AiMovementSwarm::AiMovementSwarm( AICreatureController * controller, Archive::ReadIterator & source ) :
	AiMovementFollow(controller, source),
	m_offset(0)
{
	Archive::get(source, m_offset);
	init();
}

// ----------

AiMovementSwarm::~AiMovementSwarm()
{
	cleanup();
}

// ----------------------------------------------------------------------

void AiMovementSwarm::pack( Archive::ByteStream & target ) const
{
	AiMovementFollow::pack(target);
	Archive::put(target, m_offset);
}

// ----------------------------------------------------------------------

void AiMovementSwarm::alter ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementSwarm::alter");

	// if this is a new frame, update the goals of all swarming creatures
	unsigned long currentFrame = ServerClock::getInstance().getServerFrame();
	if (currentFrame != s_lastFrame)
	{
		computeGoals();
		s_lastFrame = currentFrame;
	}

	// update the offset from our target we want to go to
	if (m_target.getObject() != nullptr)
	{
		const CreatureObject * owner = m_controller->getCreature();
		if (owner != nullptr)
		{
			offsetMap::const_iterator found = s_offsetMap.find(CachedNetworkId(*owner));
			if (found != s_offsetMap.end())
			{
				Vector offset((*found).second);
				offset += owner->getPosition_w();
				offset -= m_target.getObject()->getPosition_w();

				// The offset is in world space so we need to convert it to parent space
				Vector offset_p = m_target.getObject()->rotate_w2p(offset);

				m_target.setOffset_p(offset_p, false);
			}
		}
	}

	AiMovementFollow::alter(time);
}

// ----------------------------------------------------------------------

void AiMovementSwarm::refresh()
{
	init();
	AiMovementFollow::refresh();
}

// ----------------------------------------------------------------------

void AiMovementSwarm::clear()
{
	cleanup();
	AiMovementFollow::clear();
}

// ----------------------------------------------------------------------

void AiMovementSwarm::getDebugInfo ( std::string & outString ) const
{
	AiMovementFollow::getDebugInfo(outString);

	outString += "\n";
	outString += "AiMovementSwarm:\n";

	const int BUFSIZE = 256;
	char buffer[BUFSIZE];

	snprintf(buffer, BUFSIZE, "%.2f", m_offset);
	outString += buffer;
}

// ----------------------------------------------------------------------

AiMovementType AiMovementSwarm::getType() const
{
	return AMT_swarm;
}

// ----------------------------------------------------------------------

AiMovementSwarm * AiMovementSwarm::asAiMovementSwarm()
{
	return this;
}

// ----------------------------------------------------------------------

AiStateResult AiMovementSwarm::triggerWaiting()
{
	// note: don't use the m_target position function, because it includes the offset position
	const Object * target = m_target.getObject();
	if (target != nullptr)
		m_controller->turnToward(target->getParentCell(), target->getPosition_p());
	return AiMovementFollow::triggerWaiting();
}

// ----------------------------------------------------------------------

/**
 * Add our owner and target to the swarm map.
 */
void AiMovementSwarm::init()
{
	if (m_offset < 0)
		m_offset = 0;

	const CreatureObject * creatureOwner = m_controller->getCreature();
	const CreatureObject * creatureTarget = CreatureObject::asCreatureObject(m_target.getObject());
	if (creatureOwner != nullptr && creatureTarget != nullptr)
	{
		CreatureWatcher watchedCreature(creatureOwner);
		targetMap::iterator found = s_swarmMap.find(CachedNetworkId(*creatureTarget));
		if (found != s_swarmMap.end())
		{
			std::vector<AiMovementSwarm::CreatureWatcher> & movers = (*found).second;
			if (std::find(movers.begin(), movers.end(), watchedCreature) == movers.end())
				movers.push_back(watchedCreature);
		}
		else
		{
			std::vector<CreatureWatcher> owners;
			owners.push_back(watchedCreature);
			s_swarmMap.insert(std::make_pair(CachedNetworkId(*creatureTarget), owners));
		}
	}
	m_target.setRadius(s_goalBuffer);
}

// ----------------------------------------------------------------------

/**
 * Remove ourself from the swarm map.
 */
void AiMovementSwarm::cleanup()
{
	// note: using static_cast instead of safe_cast because the owner may be in the process of being destructed
	const CreatureObject * owner = static_cast<const CreatureObject *>(m_controller->getOwner());
	const CreatureObject * target = static_cast<const CreatureObject *>(m_target.getObject());
	if (owner != nullptr && target != nullptr)
	{
		targetMap::iterator found = s_swarmMap.find(CachedNetworkId(*target));
		if (found != s_swarmMap.end())
		{
			std::vector<CreatureWatcher> & swarmers = (*found).second;
			std::vector<CreatureWatcher>::iterator swarmer = std::find(swarmers.begin(), swarmers.end(), CreatureWatcher(owner));
			if (swarmer != swarmers.end())
			{
				swarmers.erase(swarmer);
				if (swarmers.empty())
					s_swarmMap.erase(found);
			}
		}
	}	
}

// ----------------------------------------------------------------------

/**
 * Compute the goal position for every creature swarming this frame.
 * NOTE: we may have to throttle this if it's taking up too much time/frame.
 */
void AiMovementSwarm::computeGoals()
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementSwarm::computeGoals");

	s_offsetMap.clear();

	for (targetMap::iterator i = s_swarmMap.begin(); i != s_swarmMap.end();)
	{
		const CreatureObject * target = CreatureObject::asCreatureObject((*i).first.getObject());
		if (target != nullptr && !target->isDead())
		{
			computeGoals(*target, (*i).second);
			if ((*i).second.empty())
				s_swarmMap.erase(i++);
			else
				++i;
		}
		else
		{
			s_swarmMap.erase(i++);
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Compute the goal position for every creature swarming a given target.
 *
 * @param target	the creature things are swarming to
 * @param movers	the creatures that are swarming the target
 */
void AiMovementSwarm::computeGoals(const CreatureObject & target, std::vector<CreatureWatcher> & movers)
{
	int i;
	int count = movers.size();;
	float distanceBuffer = s_goalBuffer * 2.0f;
	const float targetRadius = target.getRadius();
	Vector targetPos(target.getPosition_w());
	std::vector<Vector> movement(count);

	for (i = 0; i < count; ++i)
	{
		// determine movement to target
		const CreatureObject * mover = movers[i];
		const AICreatureController * controller = nullptr;
		const AiMovementSwarm * swarmMovement = nullptr;
		if (mover != nullptr)
		{
			controller = AICreatureController::asAiCreatureController(mover->getController());
			if (controller != nullptr)
				swarmMovement = dynamic_cast<const AiMovementSwarm *>(controller->getCurrentMovement());
		}

		if (mover == nullptr || mover->isDead())
		{
			// dump the mover from our list
			--count;
			movers[i] = movers[count];
			movers.pop_back();
			movement[i] = movement[count];
			movement.pop_back();
			--i;
			continue;
		}
		if (swarmMovement == nullptr)
		{
			// there are legitimate cases where a creature is in a swarm list but doesn't
			// have its movement as AiMovementSwarm, such as when the movement is 
			// pending or suspended
			continue;
		}

		Vector moverPos(mover->getPosition_w());
		const float moverRadius = mover->getRadius();
		const float moverOffset = swarmMovement->getOffset();
		float desiredDistance = targetRadius + moverRadius + distanceBuffer + moverOffset;
		Vector v = targetPos - moverPos;
		float distanceToTarget = v.magnitude();
		if (distanceToTarget != 0)
		{
			movement[i] += v * ((distanceToTarget - desiredDistance)/distanceToTarget);
		}

		// determine movement away from any creatures we're intersecting
		for (int j = i + 1; j < count; ++j)
		{
			const CreatureObject * blocker = movers[j];
			if (blocker == nullptr || blocker->isDead())
			{
				continue;
			}

			Vector blockerPos(blocker->getPosition_w());
			const float blockerRadius = blocker->getRadius();
			v = blockerPos - moverPos;
			distanceToTarget = v.magnitude();
			float separationDistance = moverRadius + blockerRadius + distanceBuffer;
			if (distanceToTarget > 0 && distanceToTarget < separationDistance)
			{
				// move the creatures away from each other proportional to their size
				float pushDistance = separationDistance - distanceToTarget;
				float p = blockerRadius / (moverRadius + blockerRadius);
				movement[i] -= v * ((pushDistance * p) / distanceToTarget);
				movement[j] += v * ((pushDistance * (1.0f - p)) / distanceToTarget);
			}
		}
	}

	// update the desired destinations of all the movers for when they are altered
	for (i = 0; i < count; ++i)
	{
		if (movers[i] != nullptr)
			s_offsetMap[CachedNetworkId(*movers[i])] = movement[i];
	}
}

// ======================================================================
