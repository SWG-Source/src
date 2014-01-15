// ======================================================================
//
// MessageQueueTeleportObject.cpp
// Copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/MessageQueueTeleportObject.h"

#include <string>
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueTeleportObject, CM_teleportObject);

//===================================================================

MessageQueueTeleportObject::MessageQueueTeleportObject(Vector const &position_w, NetworkId const &targetContainerId, std::string const &targetCellName, Vector const &position_p, std::string const &scriptCallback) :
	m_position_w(position_w),
	m_targetContainerId(targetContainerId),
	m_targetCellName(targetCellName),
	m_position_p(position_p),
	m_scriptCallback(scriptCallback)
{
}	

// ----------------------------------------------------------------------

MessageQueueTeleportObject::~MessageQueueTeleportObject()
{
}

// ----------------------------------------------------------------------

void MessageQueueTeleportObject::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueTeleportObject* const msg = safe_cast<const MessageQueueTeleportObject*> (data);
	if (msg)
	{
		Archive::put(target, msg->m_position_w);
		Archive::put(target, msg->m_targetContainerId);
		Archive::put(target, msg->m_targetCellName);
		Archive::put(target, msg->m_position_p);
		Archive::put(target, msg->m_scriptCallback);
	}
}

// ----------------------------------------------------------------------

MessageQueue::Data* MessageQueueTeleportObject::unpack(Archive::ReadIterator & source)
{
	Vector position_w;
	NetworkId targetContainerId;
	std::string targetCellName;
	Vector position_p;
	std::string scriptCallback;

	Archive::get(source, position_w);
	Archive::get(source, targetContainerId);
	Archive::get(source, targetCellName);
	Archive::get(source, position_p);
	Archive::get(source, scriptCallback);
	
	return new MessageQueueTeleportObject(position_w, targetContainerId, targetCellName, position_p, scriptCallback);
}

// ======================================================================

