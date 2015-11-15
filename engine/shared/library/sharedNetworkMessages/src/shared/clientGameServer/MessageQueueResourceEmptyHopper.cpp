//======================================================================
//
// MessageQueueResourceEmptyHopper.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueResourceEmptyHopper.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueResourceEmptyHopperArchive.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION_WITH_ARCHIVE(MessageQueueResourceEmptyHopper, CM_clientResourceHarvesterEmptyHopper);

//======================================================================

MessageQueueResourceEmptyHopper::MessageQueueResourceEmptyHopper (const NetworkId & playerId, const NetworkId & harvesterId, const NetworkId & resourceId, int amount, bool discard, uint8 sequenceId) :
MessageQueue::Data (),
m_playerId (playerId),
m_harvesterId (harvesterId),
m_resourceId (resourceId),
m_amount (amount),
m_discard (discard),
m_sequenceId (sequenceId)
{
	
}

//----------------------------------------------------------------------


MessageQueueResourceEmptyHopper::MessageQueueResourceEmptyHopper () :
MessageQueue::Data (),
m_playerId     (),
m_harvesterId  (),
m_resourceId   (),
m_amount       (0),
m_discard      (false),
m_sequenceId   (0)
{
	
}

//----------------------------------------------------------------------

MessageQueueResourceEmptyHopper::~MessageQueueResourceEmptyHopper ()
{
}


//======================================================================
