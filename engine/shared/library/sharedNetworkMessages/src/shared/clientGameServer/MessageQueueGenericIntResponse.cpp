//======================================================================
//
// MessageQueueGenericIntResponse.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueGenericIntResponse.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueGenericIntResponseArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION_WITH_ARCHIVE_NO_INSTALL(MessageQueueGenericIntResponse);

void MessageQueueGenericIntResponse::install() 
{ 
	installMemoryBlockManager();
	ControllerMessageFactory::registerControllerMessageHandler(CM_craftingResult, pack, unpack);
	ControllerMessageFactory::registerControllerMessageHandler(CM_nextCraftingStageResult, pack, unpack);
	ControllerMessageFactory::registerControllerMessageHandler(CM_experimentResult, pack, unpack);
	ExitChain::add(&remove, "MessageQueueGenericIntResponse::remove"); 
} 


//======================================================================

MessageQueueGenericIntResponse::~MessageQueueGenericIntResponse ()
{
}

//======================================================================
