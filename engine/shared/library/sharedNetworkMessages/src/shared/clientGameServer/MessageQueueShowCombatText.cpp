//======================================================================
//
// MessageQueueShowCombatText.cpp
// Copyright (c) 2005 Sony Online Entertainment, Inc.
// All rights reserved.
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueShowCombatText.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueShowCombatTextArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_WITH_ARCHIVE(MessageQueueShowCombatText, CM_showCombatText);

//===================================================================

MessageQueueShowCombatText::MessageQueueShowCombatText (const NetworkId &defenderId, const NetworkId &attackerId, const StringId &outputTextId, float scale, int r, int g, int b, int flags) :
	m_defenderId   (defenderId),
	m_attackerId   (attackerId),
	m_outputTextId (outputTextId),
	m_outputTextOOB (),
	m_scale        (scale),
	m_r            (r),
	m_g            (g),
	m_b            (b),
	m_flags        (flags)
{
}

//======================================================================

	MessageQueueShowCombatText::MessageQueueShowCombatText (const NetworkId &defenderId, const NetworkId &attackerId, const Unicode::String &outputTextOOB, float scale, int r, int g, int b, int flags) :
	m_defenderId   (defenderId),
	m_attackerId   (attackerId),
	m_outputTextId (),
	m_outputTextOOB (outputTextOOB),
	m_scale        (scale),
	m_r            (r),
	m_g            (g),
	m_b            (b),
	m_flags        (flags)
{
}

//======================================================================
