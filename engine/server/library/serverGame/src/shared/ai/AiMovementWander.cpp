// ======================================================================
//
// AiMovementWander.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementWander.h"

#include "serverGame/AiCreatureController.h"
#include "serverGame/CreatureObject.h"

#include "serverScript/ScriptFunctionTable.h"

using namespace Scripting;

// ======================================================================

AiMovementWander::AiMovementWander( AICreatureController * controller )
:	AiMovementWaypoint(controller),
	m_target(),
	m_sleepTime(0.0f),
	m_minDistance(0.0f),
	m_maxDistance(0.0f),
	m_minAngle(0.0f),
	m_maxAngle(0.0f),
	m_minDelay(0.0f),
	m_maxDelay(0.0f)
{
}

AiMovementWander::AiMovementWander( AICreatureController * controller, float minDistance, float maxDistance, float minAngle, float maxAngle, float minDelay, float maxDelay )
:	AiMovementWaypoint(controller),
	m_target(),
	m_sleepTime(0.0f),
	m_minDistance(minDistance),
	m_maxDistance(maxDistance),
	m_minAngle(minAngle),
	m_maxAngle(maxAngle),
	m_minDelay(minDelay),
	m_maxDelay(maxDelay)
{
	updateWaypoint();
}

// ----------

AiMovementWander::AiMovementWander( AICreatureController * controller, Archive::ReadIterator & source )
:	AiMovementWaypoint(controller, source),
	m_target(),
	m_sleepTime(0.0f),
	m_minDistance(0.0f),
	m_maxDistance(0.0f),
	m_minAngle(0.0f),
	m_maxAngle(0.0f),
	m_minDelay(0.0f),
	m_maxDelay(0.0f)
{
	Archive::get(source, m_minDistance);
	Archive::get(source, m_maxDistance);
	Archive::get(source, m_minAngle);
	Archive::get(source, m_maxAngle);
	Archive::get(source, m_minDelay);
	Archive::get(source, m_maxDelay);

	SETUP_SYNCRONIZED_STATE( AiMovementWander::stateWaiting );
	SETUP_SYNCRONIZED_STATE( AiMovementWander::stateThinking );
	SETUP_SYNCRONIZED_STATE( AiMovementWander::stateMoving );
}

// ----------

AiMovementWander::~AiMovementWander()
{
}

// ----------------------------------------------------------------------

void AiMovementWander::pack( Archive::ByteStream & target ) const
{
	AiMovementWaypoint::pack(target);
	Archive::put(target, m_minDistance);
	Archive::put(target, m_maxDistance);
	Archive::put(target, m_minAngle);
	Archive::put(target, m_maxAngle);
	Archive::put(target, m_minDelay);
	Archive::put(target, m_maxDelay);
}

// ----------------------------------------------------------------------

void AiMovementWander::alter ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementWander::alter");

	AiMovementWaypoint::alter(time);
}

// ----------------------------------------------------------------------

void AiMovementWander::getDebugInfo ( std::string & outString ) const
{
	AiMovementWaypoint::getDebugInfo(outString);

	outString += "\n";

	outString += "AiMovementWander:\n";

	char buffer[256];

	Vector goal = m_target.getPosition_w();
	sprintf(buffer,"Wandering to : (%f,%f,%f)\n",goal.x,goal.y,goal.z);
	outString += buffer;

	sprintf(buffer,"Wander distance : %f to %f\n",m_minDistance,m_maxDistance);
	outString += buffer;

	sprintf(buffer,"Wander angle : %f to %f\n",m_minAngle,m_maxAngle);
	outString += buffer;

	sprintf(buffer,"Wander delay : %f to %f\n",m_minDelay,m_maxDelay);
	outString += buffer;
}

// ----------------------------------------------------------------------

AiStateResult AiMovementWander::stateWaiting ( float time )
{
	UNREF(time);

	if(m_sleepTime > 0.0f)
	{
		m_controller->sleep(m_sleepTime);
		m_sleepTime = 0.0f;

		return ASR_Done;
	}
	else
	{
		triggerThinking();

		return ASR_Continue;
	}
}

// ----------

AiStateResult AiMovementWander::stateThinking ( float time )
{
	UNREF(time);

	if(generateWaypoint())
	{
		triggerMoving();

		return ASR_Continue;
	}
	else
	{
		// failed to generate a waypoint, sleep half a second before we try again.

		m_controller->sleep(0.5f);

		return ASR_Done;
	}
}

// ----------

AiStateResult AiMovementWander::stateMoving ( float time )
{
	if(m_controller->getStuckCounter() > 5)
	{
		triggerThinking();

		return ASR_Continue;
	}

	return AiMovementWaypoint::stateMoving(time);
}


// ----------------------------------------------------------------------

AiStateResult AiMovementWander::triggerWaiting ( void )
{
	m_controller->triggerScriptsWithParam(TRIG_WANDER_WAITING,m_sleepTime);

	return AiMovementWaypoint::triggerWaiting();
}

// ----------

AiStateResult AiMovementWander::triggerThinking ( void )
{
	CHANGE_STATE( AiMovementWander::stateThinking );

	return ASR_Continue;
}

// ----------

AiStateResult AiMovementWander::triggerMoving ( void )
{
	m_controller->triggerScriptsSimple(TRIG_WANDER_MOVING);

	return AiMovementWaypoint::triggerMoving();
}

// ----------

AiStateResult AiMovementWander::triggerWaypoint ( void )
{
	m_controller->triggerScriptsSimple(TRIG_WANDER_WAYPOINT);

	return AiMovementWaypoint::triggerWaypoint();
}

// ----------------------------------------------------------------------

bool AiMovementWander::hasWaypoint ( void ) const
{
	return true;
}

// ----------

AiLocation const & AiMovementWander::getWaypoint ( void ) const
{
	return m_target;
}

// ----------

bool AiMovementWander::updateWaypoint ( void )
{
	m_target.update();

	return true;
}

// ----------

void AiMovementWander::clearWaypoint ( void )
{
	m_target = AiLocation();
}

// ----------

bool AiMovementWander::generateWaypoint ( void )
{
	Vector center = m_controller->getCreaturePosition_p();
	Vector facing = m_controller->getOwner()->getObjectFrameK_w();

	float angle = atan2(facing.z, facing.x);
	float delta = Random::randomReal(m_minAngle, m_maxAngle);

	if (Random::random() & 16)
	{
		angle -= delta;
	}
	else
	{
		angle += delta;
	}

	float distance = Random::randomReal(m_minDistance, m_maxDistance);

	Vector point = center;

	point.x += distance * cos(angle);
	point.z += distance * sin(angle);

	m_target = AiLocation(m_controller->getCreatureCell(),point);

	m_sleepTime = Random::randomReal(m_minDelay,m_maxDelay);

	return true;
}

// ----------------------------------------------------------------------
AiMovementType AiMovementWander::getType() const
{
	return AMT_wander;
}

// ----------------------------------------------------------------------
AiMovementWander * AiMovementWander::asAiMovementWander()
{
	return this;
}

// ======================================================================
