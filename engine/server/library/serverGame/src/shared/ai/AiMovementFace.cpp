// ======================================================================
//
// AiMovementFace.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementFace.h"

#include "serverGame/AiMovementArchive.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWorld.h"

#include "sharedFoundation/DynamicVariableLocationData.h"

#include "sharedDebug/Profiler.h"

#include "sharedObject/CellProperty.h"

// ----------------------------------------------------------------------

AiMovementFace::AiMovementFace( AICreatureController * controller )
: AiMovementBase(controller),
  m_target()
{
}

AiMovementFace::AiMovementFace( AICreatureController * controller, CellProperty const * targetCell, Vector const & target_p )
: AiMovementBase(controller),
  m_target(targetCell,target_p)
{
	CHANGE_STATE( AiMovementFace::stateWaiting );
}

// ----------

AiMovementFace::AiMovementFace( AICreatureController * controller, ServerObject const * target )
:	AiMovementBase(controller),
	m_target(target)
{
	CHANGE_STATE( AiMovementFace::stateWaiting );
}

// ----------

AiMovementFace::AiMovementFace( AICreatureController * controller, Archive::ReadIterator & source )
:	AiMovementBase(controller, source),
	m_target(source)
{
	SETUP_SYNCRONIZED_STATE( AiMovementFace::stateWaiting );
}

// ----------

AiMovementFace::~AiMovementFace()
{
}

// ----------------------------------------------------------------------

void AiMovementFace::pack( Archive::ByteStream & target ) const
{
	AiMovementBase::pack(target);
	m_target.pack(target);
}

// ----------------------------------------------------------------------

void AiMovementFace::alter ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementFace::alter");

	AiMovementBase::alter(time);
}

// ----------------------------------------------------------------------

AiStateResult AiMovementFace::stateWaiting ( float time ) 
{
	UNREF(time);

	m_target.update();

	m_controller->turnToward( m_target.getCell(), m_target.getPosition_p() );

	return ASR_Done;
}

// ----------------------------------------------------------------------

void AiMovementFace::getDebugInfo ( std::string & outString ) const
{
	AiMovementBase::getDebugInfo(outString);

	outString += "\n";

	outString += "AiMovementFace:\n";

	char buffer[256];

	Vector pos = m_target.getPosition_w();

	sprintf(buffer,"Target : (%f,%f,%f)\n",pos.x,pos.y,pos.z);

	outString += buffer;
}

// ----------------------------------------------------------------------

AiMovementType AiMovementFace::getType() const
{
	return AMT_face;
}

// ----------------------------------------------------------------------

AiMovementFace * AiMovementFace::asAiMovementFace()
{
	return this;
}

// ----------------------------------------------------------------------

AiLocation const & AiMovementFace::getTarget() const
{
	return m_target;
}

// ======================================================================
