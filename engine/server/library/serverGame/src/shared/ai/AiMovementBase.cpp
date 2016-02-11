// ======================================================================
//
// AiMovementBase.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementBase.h"

#include "serverGame/AiMovementIdle.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/AiLocation.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/Footprint.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/AiDebugString.h"
#include "sharedLog/Log.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"

#include "UnicodeUtils.h"


// ----------------------------------------------------------------------

AiMovementBase::AiMovementBase( AICreatureController * controller )
:	m_controller( controller ),
	m_stateFunction( nullptr ),
	m_stateName(),
	m_pendingFunction( nullptr ),
	m_pendingName()
{
}

// ----------

AiMovementBase::AiMovementBase( AICreatureController * controller, Archive::ReadIterator & source )
:	m_controller( controller ),
	m_stateFunction( nullptr ),
	m_stateName(),
	m_pendingFunction( nullptr ),
	m_pendingName()
{
	// !!!
	// note that it's up to the derived class to set the state/pending state 
	// based on the names we get
	// !!!
	Archive::get(source, m_stateName);
	Archive::get(source, m_pendingName);
}

// ----------

AiMovementBase::~AiMovementBase()
{
}

// ----------------------------------------------------------------------

void AiMovementBase::pack( Archive::ByteStream & target ) const
{
	Archive::put(target, m_stateName);
	Archive::put(target, m_pendingName);
}

// ----------------------------------------------------------------------

void AiMovementBase::alter ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementBase::alter");
	applyStateChange();

	int cycleCount = 0;

	while(m_stateFunction)
	{
		AiStateResult result = (this->*m_stateFunction)(time);

		if(result == ASR_Done)
		{
			break;
		}
		else
		{
			cycleCount++;

			if(cycleCount > 5)
			{
				DEBUG_WARNING(ConfigServerGame::getReportAiWarnings(),("AiMovementBase::alter - Behavior has gone through more than 5 state changes in a single frame, the behavior may be in an infinite loop\n"));
				break;
			}

			applyStateChange();
		}
	}
}

// ----------------------------------------------------------------------

float AiMovementBase::getDefaultAlterTime() const
{
	return AlterResult::cms_alterQuickly;
}

// ----------------------------------------------------------------------

bool AiMovementBase::getHibernateOk ( void ) const
{
	return true;
}

// ----------------------------------------------------------------------

void AiMovementBase::getDebugInfo ( std::string & outString ) const
{
	Vector const & position_w = m_controller->getCreaturePosition_w();

	outString += FormattedString<1024>().sprintf("AiMovementBase: location(%.2f, %.2f, %.2f) state(%s)", position_w.x, position_w.y, position_w.z, m_stateName.c_str());
}


// ----------------------------------------------------------------------

void AiMovementBase::endBehavior ( void )
{
	clear();

	if(!m_controller->hasPendingMovement())
	{
		AiMovementBasePtr movement(new AiMovementIdle(m_controller));
		m_controller->changeMovement(movement);
	}
}

// ----------------------------------------------------------------------

void AiMovementBase::clear ( void )
{
}

// ----------------------------------------------------------------------

void AiMovementBase::refresh( void )
{
}

// ----------------------------------------------------------------------

void AiMovementBase::setState ( StateAlterFunction newState, char const * newStateName )
{
	if(newState != m_stateFunction)
	{
		m_pendingFunction = newState;
		m_pendingName = newStateName;
	}
}

// ----------

void AiMovementBase::applyStateChange ( void )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiBehaviorBase::applyStateChange");

	// This is a while instead of an if because onStateChanged may decide to
	// force a change to a different state (if the desired state can't be entered, for
	// example)

	while(m_pendingFunction)
	{
		//LOGC(ConfigServerGame::isAiLoggingEnabled(), "debug_ai", ("AiMovementBase::setState - going to state %s\n",m_pendingName.c_str()));

		m_stateFunction = m_pendingFunction;
		m_stateName = m_pendingName;

		m_pendingFunction = nullptr;
		m_pendingName.clear();
	}
}

// ----------------------------------------------------------------------

void AiMovementBase::debugSpeak ( char const * string ) const
{
	static char buffer[256];

	int processId = GameServer::getInstance().getProcessId();

	sprintf(buffer,"%d: %s",processId,string);

	static Unicode::String wideString;

	Unicode::narrowToWide(buffer,wideString);

	ServerObject * object = m_controller->getCreature();

	object->speakText(NetworkId::cms_invalid, 0, 0, 0, wideString, 0, Unicode::emptyString);
}

// ----------------------------------------------------------------------

bool AiMovementBase::reachedLocation ( AiLocation const & location ) const
{
	return m_controller->reachedPoint( location.getCell(), location.getPosition_p(), location.getRadius() );
}

// ----------------------------------------------------------------------

void AiMovementBase::warpToLocation ( AiLocation const & location )
{
	m_controller->warpTo( location.getCell(), location.getPosition_p() );
}

#ifdef _DEBUG
// ----------------------------------------------------------------------

void AiMovementBase::addDebug(AiDebugString & aiDebugString)
{
	aiDebugString.addLineToPosition(CellProperty::getPosition_w(m_controller->getHomeLocation()), PackedRgb::solidGreen);
}
#endif // _DEBUG

// ----------------------------------------------------------------------

char const * AiMovementBase::getMovementString(AiMovementType const aiMovementType)
{
	switch (aiMovementType)
	{
		case AMT_idle: { return "IDLE"; }
		case AMT_loiter: { return "LOITER"; }
		case AMT_wander: { return "WANDER"; }
		case AMT_follow: { return "FOLLOW"; }
		case AMT_flee: { return "FLEE"; }
		case AMT_move: { return "MOVE_TO"; }
		case AMT_face: { return "FACE"; }
		case AMT_swarm: { return "SWARM"; }
		case AMT_patrol: { return "PATROL"; }
		default:
		case AMT_invalid: { break; }
	}

	return "INVALID";
}

// ----------------------------------------------------------------------
AiMovementSwarm * AiMovementBase::asAiMovementSwarm()
{
	return nullptr;
}

// ----------------------------------------------------------------------
AiMovementFace * AiMovementBase::asAiMovementFace()
{
	return nullptr;
}

// ----------------------------------------------------------------------
AiMovementFlee * AiMovementBase::asAiMovementFlee()
{
	return nullptr;
}

// ----------------------------------------------------------------------
AiMovementFollow * AiMovementBase::asAiMovementFollow()
{
	return nullptr;
}

// ----------------------------------------------------------------------
AiMovementLoiter * AiMovementBase::asAiMovementLoiter()
{
	return nullptr;
}

// ----------------------------------------------------------------------
AiMovementMove * AiMovementBase::asAiMovementMove()
{
	return nullptr;
}

// ----------------------------------------------------------------------
AiMovementPatrol * AiMovementBase::asAiMovementPatrol()
{
	return nullptr;
}

// ----------------------------------------------------------------------
AiMovementWander * AiMovementBase::asAiMovementWander()
{
	return nullptr;
}

// ======================================================================
