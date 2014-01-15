// ======================================================================
//
// AiMovementIdle.cpp
// copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementIdle.h"

#include "serverGame/AiMovementArchive.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/CreatureObject.h"
#include "sharedGame/AiDebugString.h"

// ----------------------------------------------------------------------
AiMovementIdle::AiMovementIdle(AICreatureController * controller)
 : AiMovementBase(controller)
{
}

// ----------------------------------------------------------------------
AiMovementIdle::AiMovementIdle(AICreatureController * controller, Archive::ReadIterator & source)
:	AiMovementBase(controller, source)
{
	SETUP_SYNCRONIZED_STATE(AiMovementIdle::stateWaiting);
}

// ----------------------------------------------------------------------
AiMovementIdle::~AiMovementIdle()
{
}

// ----------------------------------------------------------------------

void AiMovementIdle::pack( Archive::ByteStream & target ) const
{
	AiMovementBase::pack(target);
}

// ----------------------------------------------------------------------

void AiMovementIdle::alter(float time)
{
	AiMovementBase::alter(time);
}

// ----------------------------------------------------------------------
AiStateResult AiMovementIdle::stateWaiting(float /*time*/)
{
	return ASR_Done;
}

// ----------------------------------------------------------------------
AiMovementType AiMovementIdle::getType() const
{
	return AMT_idle;
}

// ----------------------------------------------------------------------

AiMovementIdle * AiMovementIdle::asAiMovementIdle()
{
	return this;
}

#ifdef _DEBUG
// ----------------------------------------------------------------------
void AiMovementIdle::addDebug(AiDebugString & aiDebugString)
{
	aiDebugString.addText("IDLE\n", PackedRgb::solidCyan);
}
#endif // _DEBUG

// ======================================================================
