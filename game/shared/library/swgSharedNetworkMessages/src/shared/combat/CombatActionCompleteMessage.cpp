// ======================================================================
//
// CombatActionCompleteMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#include "swgSharedNetworkMessages/FirstSwgSharedNetworkMessages.h"
#include "swgSharedNetworkMessages/CombatActionCompleteMessage.h"

//-----------------------------------------------------------------------

const char * const CombatActionCompleteMessage::MESSAGE_TYPE = "CombatActionCompleteMessage::MESSAGE_TYPE";

//-----------------------------------------------------------------

CombatActionCompleteMessage::CombatActionCompleteMessage (uint32 sequenceId, bool nack) :
GameNetworkMessage (MESSAGE_TYPE),
m_sequenceId (sequenceId & ~(0x80000000))
{
	addVariable (m_sequenceId);

	//-- set the high bit if this is a nack
	if (nack)
		m_sequenceId.set (m_sequenceId.get () | 0x80000000);
}

//-----------------------------------------------------------------------

CombatActionCompleteMessage::CombatActionCompleteMessage(Archive::ReadIterator & source) :
GameNetworkMessage (MESSAGE_TYPE),
m_sequenceId ()
{
	addVariable (m_sequenceId);
	unpack (source);
}

//----------------------------------------------------------------------

CombatActionCompleteMessage::~CombatActionCompleteMessage()
{
}

//-----------------------------------------------------------------------
