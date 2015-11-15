//======================================================================
//
// MessageQueueShowFlyText.cpp
// Copyright (c) 2002 Sony Online Entertainment, Inc.
// All rights reserved.
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueShowFlyText.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueShowFlyTextArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_WITH_ARCHIVE(MessageQueueShowFlyText, CM_showFlyText);

//===================================================================

MessageQueueShowFlyText::MessageQueueShowFlyText (const NetworkId &emitterId, const StringId &outputTextId, float scale, int r, int g, int b, int flags) :
	m_emitterId    (emitterId),
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

MessageQueueShowFlyText::MessageQueueShowFlyText (const NetworkId &emitterId, const Unicode::String &outputTextOOB, float scale, int r, int g, int b, int flags) :
	m_emitterId    (emitterId),
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
