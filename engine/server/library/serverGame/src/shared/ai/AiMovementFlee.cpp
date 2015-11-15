// ======================================================================
//
// AiMovementPathFlee.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementFlee.h"

#include "serverGame/AiMovementArchive.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedObject/CellProperty.h"

using namespace Scripting;

// ======================================================================

AiMovementFlee::AiMovementFlee( AICreatureController * controller )
:   AiMovementWaypoint(controller),
    m_fleeFrom(),
	m_fleeTo(),
	m_minDistance(0.0f),
	m_maxDistance(0.0f)
{
}

AiMovementFlee::AiMovementFlee( AICreatureController * controller, ServerObject const * target, float minDistance, float maxDistance )
:	AiMovementWaypoint(controller),
	m_fleeFrom(target),
	m_fleeTo(),
	m_minDistance(minDistance),
	m_maxDistance(maxDistance)
{
}

AiMovementFlee::AiMovementFlee( AICreatureController * controller, Archive::ReadIterator & source )
:   AiMovementWaypoint(controller, source),
    m_fleeFrom(source),
	m_fleeTo(source),
	m_minDistance(0.0f),
	m_maxDistance(0.0f)
{
	Archive::get(source, m_minDistance);
	Archive::get(source, m_maxDistance);

	SETUP_SYNCRONIZED_STATE( AiMovementFlee::stateWaiting );
	SETUP_SYNCRONIZED_STATE( AiMovementFlee::stateThinking );
	SETUP_SYNCRONIZED_STATE( AiMovementFlee::stateMoving );
}

// ----------

AiMovementFlee::~AiMovementFlee()
{
}

// ----------------------------------------------------------------------

void AiMovementFlee::pack( Archive::ByteStream & target ) const
{
	AiMovementWaypoint::pack(target);
	m_fleeFrom.pack(target);
	m_fleeTo.pack(target);
	Archive::put(target, m_minDistance);
	Archive::put(target, m_maxDistance);
}

// ----------------------------------------------------------------------

void AiMovementFlee::alter ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementFlee::alter");

	AiMovementWaypoint::alter(time);
}

// ----------------------------------------------------------------------

void AiMovementFlee::getDebugInfo ( std::string & outString ) const
{
	AiMovementWaypoint::getDebugInfo(outString);

	Vector const & flee_w = m_fleeFrom.getPosition_w();
	outString += FormattedString<1024>().sprintf("AiMovementFlee: fleeingFrom(%d) offset(%.2f, %.2f, %.2f) distance[%.2f...%.2f]", m_fleeFrom.getObjectId().getHashValue(), flee_w.x, flee_w.y, flee_w.z, m_minDistance, m_maxDistance);
}

// ----------------------------------------------------------------------

AiStateResult AiMovementFlee::stateWaiting ( float time )
{
	UNREF(time);

	if(!m_fleeFrom.isValid())
		return triggerTargetLost();

	return triggerThinking();
}

// ----------

AiStateResult AiMovementFlee::stateThinking ( float time )
{
	UNREF(time);

	if(!m_fleeFrom.isValid())
		return triggerTargetLost();

	if(generateWaypoint())
		return triggerMoving();

	return ASR_Done;
}

// ----------

AiStateResult AiMovementFlee::stateMoving ( float time )
{
	if(m_controller->getStuckCounter() > 5)
		return triggerThinking();

	return AiMovementWaypoint::stateMoving(time);
}

// ----------------------------------------------------------------------

AiStateResult AiMovementFlee::triggerThinking ( void )
{
	CHANGE_STATE( AiMovementFlee::stateThinking );

	return ASR_Continue;
}

// ----------

AiStateResult AiMovementFlee::triggerWaypoint ( void )
{
	m_controller->triggerScriptsWithTarget(TRIG_FLEE_WAYPOINT,m_fleeFrom.getObjectId());

	AiMovementWaypoint::triggerWaypoint();

	return ASR_Continue;
}

// ----------

AiStateResult AiMovementFlee::triggerTargetLost ( void )
{
	m_controller->triggerScriptsWithTarget(TRIG_FLEE_TARGET_LOST,m_fleeFrom.getObjectId());

	AiMovementWaypoint::triggerTargetLost();

	endBehavior();

	return ASR_Done;
}

// ----------

AiStateResult AiMovementFlee::triggerPathNotFound ( void )
{
	m_controller->triggerScriptsWithTarget(TRIG_FLEE_PATH_NOT_FOUND,m_fleeFrom.getObjectId());

	endBehavior();

	return ASR_Done;
}

// ----------------------------------------------------------------------

bool AiMovementFlee::hasWaypoint ( void ) const
{
	return m_fleeTo.isValid();
}

// ----------

AiLocation const & AiMovementFlee::getWaypoint ( void ) const
{
	return m_fleeTo;
}

// ----------

bool AiMovementFlee::updateWaypoint ( void )
{
	m_fleeFrom.update();
	m_fleeTo.update();

	if(m_fleeFrom.hasChanged())
	{
		generateWaypoint();

		m_fleeFrom.setChanged(false);
	}

	return true;
}

// ----------

bool AiMovementFlee::generateWaypoint ( void )
{
	Vector current  = m_controller->getCreaturePosition_w();
	Vector start = m_fleeFrom.getPosition_w();
	
	Vector delta = current - start;
	delta.x += Random::randomReal(0.0f,0.1f);
	delta.y = 0;
	delta.z += Random::randomReal(0.0f,0.1f);
	delta.normalize();

	float fleeDistance = Random::randomReal(m_minDistance, m_maxDistance);
	delta *= fleeDistance;

	Vector fleePoint = current + delta;

	m_fleeTo = AiLocation(CellProperty::getWorldCellProperty(),fleePoint);

	return true;
}

// ----------------------------------------------------------------------

bool AiMovementFlee::getDecelerate ( void ) const
{
	return false;
}

// ----------------------------------------------------------------------

AiMovementType AiMovementFlee::getType() const
{
	return AMT_flee;
}

// ----------------------------------------------------------------------

AiMovementFlee * AiMovementFlee::asAiMovementFlee()
{
	return this;
}

// ======================================================================
