//======================================================================
//
// MessageQueueGenericResponse.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueGenericResponse.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueGenericResponseArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_WITH_ARCHIVE_NO_INSTALL(MessageQueueGenericResponse);

//===================================================================
void MessageQueueGenericResponse::install() 
{ 
	installMemoryBlockManager();
	ControllerMessageFactory::registerControllerMessageHandler(CM_nextStageReady, pack, unpack); 
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientResourceHarvesterEmptyHopperResponse, pack, unpack); 
	ExitChain::add(&remove, "MessageQueueGenericResponse::remove"); 
} 


//======================================================================

MessageQueueGenericResponse::~MessageQueueGenericResponse ()
{
}

//======================================================================
