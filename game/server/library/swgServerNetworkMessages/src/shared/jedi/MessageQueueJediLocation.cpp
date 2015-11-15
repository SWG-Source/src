// ======================================================================
//
// MessageQueueJediLocation.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgServerNetworkMessages/FirstSwgServerNetworkMessages.h"
#include "swgServerNetworkMessages/MessageQueueJediLocation.h"

#include <string>
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueJediLocation, CM_updateJediLocation);


//===================================================================


// ----------------------------------------------------------------------

MessageQueueJediLocation::MessageQueueJediLocation(const NetworkId & id, 
	const Vector & location, const std::string & scene) :
	m_id(id),
	m_location(location),
	m_scene(scene)
{
}	

// ----------------------------------------------------------------------

MessageQueueJediLocation::~MessageQueueJediLocation()
{
	
}

void MessageQueueJediLocation::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueJediLocation* const msg = safe_cast<const MessageQueueJediLocation*> (data);
	if (msg)
	{
		Archive::put(target, msg->m_id);
		Archive::put(target, msg->m_location);
		Archive::put(target, msg->m_scene);
	}
}

// ----------------------------------------------------------------------

MessageQueue::Data* MessageQueueJediLocation::unpack(Archive::ReadIterator & source)
{
	NetworkId id;
	Vector location;
	std::string scene;

	Archive::get(source, id);
	Archive::get(source, location);
	Archive::get(source, scene);
	
	return new MessageQueueJediLocation(id, location, scene);
}


// ======================================================================
