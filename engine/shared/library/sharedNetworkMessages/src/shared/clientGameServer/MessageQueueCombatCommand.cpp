//======================================================================
//
// MessageQueueCombatCommand.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCombatCommand.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"

//===================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(MessageQueueCombatCommand, true, 0, 0, 0);

//===================================================================


//-----------------------------------------------------------------------

MessageQueueCombatCommand::MessageQueueCombatCommand (const std::string & str, uint32 sequenceId) :
Data (),
m_string (str),
m_sequenceId (sequenceId)
{
}

//-----------------------------------------------------------------------

MessageQueueCombatCommand::~MessageQueueCombatCommand()
{
}

//-----------------------------------------------------------------------
