//======================================================================
//
// MessageQueueSitOnObject.cpp
// Copyright (c) 2002 Sony Online Entertainment, Inc.
// All rights reserved.
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueSitOnObject.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueSitOnObjectArchive.h"

// ======================================================================

float const MessageQueueSitOnObject::cs_maximumChairRange        = 2.0f;
float const MessageQueueSitOnObject::cs_maximumChairRangeSquared = cs_maximumChairRange * cs_maximumChairRange;

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_WITH_ARCHIVE(MessageQueueSitOnObject, CM_sitOnObject);

//===================================================================

MessageQueueSitOnObject::MessageQueueSitOnObject(NetworkId const &chairCellId, Vector const &chairPosition_p) :
	MessageQueue::Data (),
	m_chairCellId(chairCellId),
	m_chairPosition_p(chairPosition_p)
{
}

//======================================================================
