// ======================================================================
//
// AiMovementMove.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementMove.h"

#include "serverGame/AiMovementArchive.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWorld.h"

#include "serverPathfinding/ServerPathBuilder.h"

#include "sharedFoundation/DynamicVariableLocationData.h"
#include "serverScript/ScriptFunctionTable.h"

#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorMesh.h"

#include "sharedDebug/PerformanceTimer.h"
#include "sharedFoundation/FormattedString.h"

#include "sharedObject/CellProperty.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/PortalPropertyTemplate.h"

#include "UnicodeUtils.h"

using namespace Scripting;


// ======================================================================

AiMovementMove::AiMovementMove( AICreatureController * controller )
: AiMovementTarget(controller),
  m_start(controller->getCreatureCell(),controller->getCreaturePosition_p()),
  m_targetName(),
  m_pathBuilder( new ServerPathBuilder() )
{
	CHANGE_STATE( AiMovementMove::stateWaiting );
}

AiMovementMove::AiMovementMove( AICreatureController * controller, CellProperty const * cell, Vector const & target, float radius )
: AiMovementTarget(controller, cell, target, radius),
  m_start(controller->getCreatureCell(),controller->getCreaturePosition_p()),
  m_targetName(),
  m_pathBuilder( new ServerPathBuilder() )
{
	CHANGE_STATE( AiMovementMove::stateWaiting );
}

// ----------

AiMovementMove::AiMovementMove( AICreatureController * controller, Unicode::String const & targetName )
: AiMovementTarget(controller),
  m_start(),
  m_targetName(targetName),
  m_pathBuilder( new ServerPathBuilder() )
{
	CHANGE_STATE( AiMovementMove::stateWaiting );
}

// ----------

AiMovementMove::AiMovementMove( AICreatureController * controller, Archive::ReadIterator & source )
: AiMovementTarget(controller, source),
  m_start(source),
  m_targetName(),
  m_pathBuilder( new ServerPathBuilder() )
{
	Archive::get(source, m_targetName);

	SETUP_SYNCRONIZED_STATE( AiMovementMove::stateWaiting );
	SETUP_SYNCRONIZED_STATE( AiMovementMove::statePathing );
}

// ----------

AiMovementMove::~AiMovementMove()
{
	delete m_pathBuilder;
	m_pathBuilder = nullptr;
}

// ----------------------------------------------------------------------

void AiMovementMove::pack( Archive::ByteStream & target ) const
{
	AiMovementTarget::pack(target);
	m_start.pack(target);
	Archive::put(target, m_targetName);
}

// ----------------------------------------------------------------------

void AiMovementMove::alter ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementMove::alter");

	AiMovementTarget::alter(time);
}

// ----------------------------------------------------------------------

void AiMovementMove::getDebugInfo ( std::string & outString ) const
{
	AiMovementTarget::getDebugInfo(outString);

	Vector const & start_w = m_start.getPosition_w();
	Vector const & goal_w = m_target.getPosition_w();

	outString += FormattedString<1024>().sprintf("AiMovementMove: start(%.2f, %.2f, %.2f) goal(%.2f, %.2f, %.2f)", start_w.x, start_w.y, start_w.z, goal_w.x, goal_w.y, goal_w.z);

	if (!m_targetName.empty())
	{
		outString += FormattedString<1024>().sprintf(" goalName(%s)", Unicode::wideToNarrow(m_targetName).c_str());
	}
	outString += "\n";
}

// ----------------------------------------------------------------------

void AiMovementMove::clear ( void )
{
	m_target = AiLocation();
	m_targetName.clear();
	AiMovementTarget::clear();
}

// ----------------------------------------------------------------------

/**
 * Recalculate our path.
 */
void AiMovementMove::refresh( void )
{
	// save our target, since it's going to get cleared in AiMovementMove::clear 
	// as a side effect of AiMovementTarget::refresh()
	const AiLocation target(m_target);
	const Unicode::String targetName(m_targetName);

	AiMovementTarget::refresh();
	
	// restore our target
	m_target = target;
	m_targetName = targetName;
	
	if (m_controller != nullptr)
		m_start = AiLocation(m_controller->getCreatureCell(), m_controller->getCreaturePosition_p());
	CHANGE_STATE( AiMovementMove::stateWaiting );
}

// ----------------------------------------------------------------------

AiStateResult AiMovementMove::stateWaiting ( float time )
{
	if(m_target.isValid() || !m_targetName.empty())
		return triggerPathing();

	return ASR_Done;
}

// ----------

AiStateResult AiMovementMove::statePathing ( float time )
{
	if(!m_pathBuilder->buildDone())
		return ASR_Done;

	if(m_pathBuilder->buildFailed())
		return triggerPathNotFound();

	// ----------
	// path search is done and succeeded - transition to moving state

	swapPath(m_pathBuilder->getPath());

	if(!m_targetName.empty())
	{
		if(hasPath())
		{
			m_target = getPath()->back();
		}
	}
	else
	{
		addPathNode(m_target);
	}

	return triggerMoving();
}

// ----------------------------------------------------------------------

AiStateResult AiMovementMove::triggerPathing ( void )
{
	if(setupPathSearch())
	{
		CHANGE_STATE( AiMovementMove::statePathing );

		return ASR_Continue;
	}
	else
	{
		return triggerPathNotFound();
	}
}

// ----------

AiStateResult AiMovementMove::triggerMoving ( void )
{
	m_controller->triggerScriptsSimple(TRIG_MOVE_MOVING);

	return AiMovementTarget::triggerMoving();
}

// ----------

AiStateResult AiMovementMove::triggerTargetLost ( void )
{
	endBehavior();

	return AiMovementTarget::triggerTargetLost();
}

// ----------

AiStateResult AiMovementMove::triggerPathNotFound ( void )
{
	m_controller->triggerScriptsSimple(TRIG_MOVE_PATH_NOT_FOUND);

	if (m_controller->isRetreating())
	{
		m_controller->setRetreating(false);
	}

	endBehavior();

	return ASR_Done;
}

// ----------

AiStateResult AiMovementMove::triggerPathComplete( void )
{
	if (m_controller->isRetreating())
	{
		m_controller->setRetreating(false);
	}

	m_controller->triggerScriptsSimple(TRIG_MOVE_PATH_COMPLETE);

	endBehavior();

	return AiMovementTarget::triggerPathComplete();
}

// ----------------------------------------------------------------------

bool AiMovementMove::setupPathSearch ( void )
{
	if(!m_targetName.empty())
	{
		return m_pathBuilder->buildPath_Async( m_controller->getCreature(), m_targetName );
	}

	// ----------

	if (m_target.getRadius() > 0)
	{
		// for the moment, adjust the target to the nearest point on the bounding 
		// sphere to the ai. we would like to make this more dynamic later (because 
		// the nearest point might not be reachable, or might change because of the
		// ai moving around to avoid obsticles).
		Vector v(m_start.getPosition_w() - m_target.getPosition_w());
		v.approximateNormalize();
		v *= m_target.getRadius();
		m_target.setPosition_w(v + m_target.getPosition_w());
		m_target.setRadius(0);
	}

	if(!CollisionWorld::testClear(m_target.getCell(),Sphere(m_target.getPosition_p(),m_target.getRadius())))
	{
		return false;
	}
	else
	{
		return m_pathBuilder->buildPath_Async( m_controller->getCreature(), m_target );
	}
}

// ----------------------------------------------------------------------

AiMovementType AiMovementMove::getType() const
{
	return AMT_move;
}

// ----------------------------------------------------------------------

AiMovementMove * AiMovementMove::asAiMovementMove()
{
	return this;
}

// ----------------------------------------------------------------------

Unicode::String const & AiMovementMove::getTargetName() const
{
	return m_targetName;
}

// ======================================================================
