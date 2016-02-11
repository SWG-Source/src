// ======================================================================
//
// AiMovementPathFollow.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementPathFollow.h"

#include "serverGame/AiCreatureController.h"
#include "serverGame/AiMovementArchive.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverPathfinding/ServerPathBuilder.h"
#include "serverPathfinding/ServerPathfindingMessaging.h"
#include "serverScript/ScriptFunctionTable.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedPathfinding/ConfigSharedPathfinding.h"

using namespace Scripting;

// ======================================================================
//
// AiMovementPathFollowNamespace
//
// ======================================================================

namespace AiMovementPathFollowNamespace
{
	float const s_maxCutDistance = 64.0f;
}

using namespace AiMovementPathFollowNamespace;

// ----------------------------------------------------------------------

#ifdef _DEBUG

void sendWaypointInfo ( AiLocation const & loc )
{
	UNREF(loc);

	if(ConfigServerGame::getSendBreadcrumbs())
	{
		ServerPathfindingMessaging::getInstance().sendWaypointInfo(loc);
	}
}

// ----------

void sendEraseWaypoint ( AiLocation const & loc )
{
	UNREF(loc);

	if(ConfigServerGame::getSendBreadcrumbs())
	{
		ServerPathfindingMessaging::getInstance().sendEraseWaypoint(loc);
	}
}

#endif

// ======================================================================

AiMovementPathFollow::AiMovementPathFollow( AICreatureController * controller )
: AiMovementWaypoint( controller ),
  m_path( new AiPath() )
{
}

AiMovementPathFollow::AiMovementPathFollow( AICreatureController * controller, Archive::ReadIterator & source )
: AiMovementWaypoint( controller, source ),
  m_path( new AiPath() )
{
	if (m_path != nullptr)
		Archive::get(source, *m_path);
}

// ----------

AiMovementPathFollow::~AiMovementPathFollow()
{
	clearPath();

	delete m_path;
	m_path = nullptr;
}

// ----------------------------------------------------------------------

void AiMovementPathFollow::pack( Archive::ByteStream & target ) const
{
	AiMovementWaypoint::pack(target);
	if (m_path != nullptr)
		Archive::put(target, *m_path);
	else
		Archive::put(target, static_cast<int>(0));
}

// ----------------------------------------------------------------------

void AiMovementPathFollow::alter ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementPathFollow::alter");

	cutPath();

	AiMovementWaypoint::alter(time);
}

// ----------------------------------------------------------------------

void AiMovementPathFollow::getDebugInfo ( std::string & outString ) const
{
	AiMovementWaypoint::getDebugInfo(outString);

	outString += "\n";

	outString += "AiMovementPathFollow:\n";

	char buffer[256];

	sprintf(buffer,"Path length : %d\n",m_path->size());
	outString += buffer;
}

// ----------------------------------------------------------------------

void AiMovementPathFollow::clear ( void )
{
	AiMovementWaypoint::clear();
	clearPath();
}

// ----------------------------------------------------------------------

void AiMovementPathFollow::refresh( void )
{
	AiMovementWaypoint::refresh();
	clear();
}

// ----------------------------------------------------------------------

AiStateResult AiMovementPathFollow::triggerWaypoint ( void )
{
	clearWaypoint();

	if(m_path->empty())
	{
		return triggerPathComplete();
	}
	else
	{
		return ASR_Done;
	}
}

// ----------------------------------------------------------------------

AiStateResult AiMovementPathFollow::triggerPathComplete ( void )
{
	return triggerWaiting();
}

// ----------------------------------------------------------------------

bool AiMovementPathFollow::hasWaypoint ( void ) const
{
	return !m_path->empty();
}

// ----------------------------------------------------------------------

AiLocation const & AiMovementPathFollow::getWaypoint ( void ) const
{
	if(m_path->empty())
	{
		DEBUG_FATAL(true,("AiMovementPathFollow::getWaypoint - no waypoint, path is empty.\n"));
	}

	return m_path->front();
}

// ----------------------------------------------------------------------

bool AiMovementPathFollow::updateWaypoint ( void )
{
	return true;
}

// ----------------------------------------------------------------------

void AiMovementPathFollow::clearWaypoint ( void )
{
	popNodeFront();
}

// ----------------------------------------------------------------------

bool AiMovementPathFollow::getDecelerate ( void ) const
{
	if(getPathLength() > 1)
		return false;
	else
		return AiMovementWaypoint::getDecelerate();
}

// ----------------------------------------------------------------------

void AiMovementPathFollow::addPathNode ( AiLocation const & newNode )
{
	if(!newNode.isValid()) return;

	m_path->push_back(newNode);

	m_path->back().detach();

#ifdef _DEBUG
	sendWaypointInfo(m_path->back());
#endif

	// sanity check

	if(m_path->size() > 200)
	{
		endBehavior();

		triggerTargetLost();

		return;
	}
}

// ----------------------------------------------------------------------

void AiMovementPathFollow::popNodeFront ( void )
{
	if(m_path->empty()) return;

#ifdef _DEBUG
	sendEraseWaypoint(m_path->front());
#endif

	m_path->erase(m_path->begin());
}

// ----------------------------------------------------------------------

void AiMovementPathFollow::popNodeBack ( void )
{
	if(m_path->empty()) return;

#ifdef _DEBUG
	sendEraseWaypoint(m_path->back());
#endif

	AiPath::iterator it = m_path->end();

	it--;

	m_path->erase(it);
}

// ----------------------------------------------------------------------

bool AiMovementPathFollow::hasPath ( void ) const
{
	return m_path && !m_path->empty();
}

// ----------------------------------------------------------------------

int AiMovementPathFollow::getPathLength ( void ) const
{
	return m_path->size();
}

// ----------------------------------------------------------------------

AiPath const * AiMovementPathFollow::getPath ( void ) const
{
	return m_path;
}

// ----------------------------------------------------------------------

void AiMovementPathFollow::swapPath ( AiPath * newPath )
{
	if(newPath == nullptr) return;

	AiPath::iterator it;

#ifdef _DEBUG
	for(it = m_path->begin(); it != m_path->end(); it++)
	{
		sendEraseWaypoint(*it);
	}
#endif

	m_path->clear();

	m_path->swap(*newPath);

	for(it = m_path->begin(); it != m_path->end(); it++)
	{
		(*it).detach();

#ifdef _DEBUG
		sendWaypointInfo(*it);
#endif
	}
}

// ----------------------------------------------------------------------

void AiMovementPathFollow::clearPath ( void )
{
#ifdef _DEBUG

	if(m_path && !m_path->empty())
	{
		AiPath::iterator it;

		for(it = m_path->begin(); it != m_path->end(); it++)
		{
			sendEraseWaypoint(*it);
		}
	}

#endif

	m_path->clear();
}

// ----------------------------------------------------------------------

AiStateResult AiMovementPathFollow::cutPath()
{
	CreatureObject * const creatureOwner = m_controller->getCreature();
	Vector const & creaturePosition_p = m_controller->getCreaturePosition_p();
	CellProperty const * creatureCell = m_controller->getCreatureCell();

	// See if we can move straight to the final path location

	if (m_path->size() > 1)
	{
		AiLocation const & finalLocation = m_path->back();
		Vector const finalPosition_p = finalLocation.getPosition_p(creatureCell);
		Vector delta = creaturePosition_p - finalPosition_p;

		if (delta.magnitudeSquared() < sqr(s_maxCutDistance))
		{
			float const moveRadius = creatureOwner->getRadius();
			bool const checkY = true;
			bool const checkFlora = true; // statics
			bool const checkFauna = false; // dynamics

			if (CollisionWorld::canMove(creatureOwner, finalPosition_p, moveRadius, checkY, checkFlora, checkFauna) == CMR_MoveOK)
			{
				// Since we can move to the final path location, remove
				// all path locations except the final one

				while (m_path->size() > 1)
				{
					popNodeFront();
				}
			}
		}
	}

	// Try to skip the next waypoint

	if (m_path->size() >= 2)
	{
		AiPath::iterator iterPath = m_path->begin();
		Vector delta = creaturePosition_p - iterPath->getPosition_p();

		if (delta.magnitudeSquared() <= sqr(s_maxCutDistance)) 
		{
			// Advance to the next path location

			++iterPath;

			if (iterPath->isCuttable())
			{
				Vector const finalPosition_p = iterPath->getPosition_p(m_controller->getCreatureCell());
				float const moveRadius = creatureOwner->getRadius();
				bool const checkY = true;
				bool const checkFlora = true; // statics
				bool const checkFauna = false; // dynamics

				if (CollisionWorld::canMove(creatureOwner, finalPosition_p, moveRadius, checkY, checkFlora, checkFauna) == CMR_MoveOK)
				{
					popNodeFront();
				}
				else
				{
					iterPath->setCuttable(false);
				}
			}
		}
	}

	return ASR_Continue;
}

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, AiPath & target)
	{
		int size;
		AiLocation location;
		get(source, size);
		for (int i = 0; i < size; ++i)
		{
			AiLocation location(source);
			target.push_back(location);
			location.clear();
		}
	}

	void put(ByteStream & target, const AiPath & source)
	{
		put(target, static_cast<int>(source.size()));
		for (AiPath::const_iterator i = source.begin(); i != source.end(); ++i)
		{
			(*i).pack(target);
		}
	}
};

// ======================================================================
