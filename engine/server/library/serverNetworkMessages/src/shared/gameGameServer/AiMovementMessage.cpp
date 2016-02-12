// ======================================================================
//
// AiMovementMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/AiMovementMessage.h"

#include "serverNetworkMessages/GameServerMessageInterface.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"


//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(AiMovementMessage, CM_aiSetMovement);


//=======================================================================

AiMovementMessage::AiMovementMessage() : 
	m_objectId(),
	m_movement()
{
}

//-----------------------------------------------------------------------

AiMovementMessage::AiMovementMessage(const NetworkId & objectId, AiMovementBasePtr behavior) : 
	m_objectId(objectId),
	m_movement(behavior)
{
}

//-----------------------------------------------------------------------

AiMovementMessage::AiMovementMessage(const NetworkId & objectId) : 
	m_objectId(objectId),
	m_movement()
{
}

//-----------------------------------------------------------------------

AiMovementMessage::~AiMovementMessage()
{
	m_movement = AiMovementBaseNullPtr;
}

//-----------------------------------------------------------------------

void AiMovementMessage::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const AiMovementMessage * const msg = safe_cast<const AiMovementMessage *> (data);
	if (msg)
	{
		if (GameServerMessageInterface::getInstance() != nullptr)
			GameServerMessageInterface::getInstance()->pack(target, *msg);
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* AiMovementMessage::unpack(Archive::ReadIterator & source)
{
	AiMovementMessage * msg = new AiMovementMessage();

	if (GameServerMessageInterface::getInstance() != nullptr)
		GameServerMessageInterface::getInstance()->unpack(source, *msg);

	return msg;
}

//-----------------------------------------------------------------------
AiMovementType AiMovementMessage::getMovementType() const
{
	AiMovementType result = AMT_idle;

	if (m_movement != AiMovementBaseNullPtr)
	{
		result = m_movement->getType();
	}

	return result;
}

//=======================================================================

