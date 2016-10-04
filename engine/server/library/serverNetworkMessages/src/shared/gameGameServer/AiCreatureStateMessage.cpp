// ======================================================================
//
// AiCreatureStateMessage.cpp
// copyright 2005 Sony Online Entertainment
//
// ======================================================================

// ----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/AiCreatureStateMessage.h"

#include "serverNetworkMessages/GameServerMessageInterface.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

CONTROLLER_MESSAGE_IMPLEMENTATION(AiCreatureStateMessage, CM_aiCreatureState);

// ======================================================================
//
// AiCreatureStateMessage
//
// ======================================================================

// ----------------------------------------------------------------------
AiCreatureStateMessage::AiCreatureStateMessage()
 : m_networkId()
 , m_movement()
{
}

// ----------------------------------------------------------------------

void AiCreatureStateMessage::pack(MessageQueue::Data const * const data, Archive::ByteStream & target)
{
	AiCreatureStateMessage const * const msg = safe_cast<const AiCreatureStateMessage *>(data);

	if (msg != nullptr)
	{
		if (GameServerMessageInterface::getInstance() != nullptr)
		{
			GameServerMessageInterface::getInstance()->pack(target, *msg);
		}
	}
}

// ----------------------------------------------------------------------

MessageQueue::Data * AiCreatureStateMessage::unpack(Archive::ReadIterator & source)
{
	AiCreatureStateMessage * msg = new AiCreatureStateMessage();

	if (GameServerMessageInterface::getInstance() != nullptr)
	{
		GameServerMessageInterface::getInstance()->unpack(source, *msg);
	}

	return msg;
}

// ======================================================================
