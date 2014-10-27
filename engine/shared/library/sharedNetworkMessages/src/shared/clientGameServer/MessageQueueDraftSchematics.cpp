//========================================================================
//
// MessageQueueDraftSchematics.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueDraftSchematics.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueDraftSchematics, CM_draftSchematicsMessage);

//===================================================================


/**
* Class destructor.
*/
MessageQueueDraftSchematics::~MessageQueueDraftSchematics()
{
}	// MessageQueueDraftSchematics::~MessageQueueDraftSchematics

//----------------------------------------------------------------------

MessageQueueDraftSchematics::MessageQueueDraftSchematics(const NetworkId & toolId,
	const NetworkId & stationId) :
m_schematics (),
m_toolId (toolId),
m_stationId (stationId)
{
}	// MessageQueueDraftSchematics::MessageQueueDraftSchematics()

//----------------------------------------------------------------------

void MessageQueueDraftSchematics::addSchematic(const std::pair<uint32, uint32> & crc, 
	int category)
{
	m_schematics.push_back(SchematicData(crc.first, crc.second, category));
}	// MessageQueueDraftSchematics::addSchematic

//----------------------------------------------------------------------

int MessageQueueDraftSchematics::getSchematicCount(void) const
{
	return m_schematics.size();
}	// MessageQueueDraftSchematics::getSchematicCount

//----------------------------------------------------------------------

const MessageQueueDraftSchematics::SchematicData & MessageQueueDraftSchematics::getSchematic(int index) const
{
	DEBUG_FATAL(index < 0 || index >= static_cast<int>(m_schematics.size()), 
		("MessageQueueDraftSchematics::getSchematic index out of range"));
	return m_schematics.at(index);
}	// MessageQueueDraftSchematics::getSchematic

//----------------------------------------------------------------------

void MessageQueueDraftSchematics::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueDraftSchematics* const msg = safe_cast<const MessageQueueDraftSchematics*> (data);
	if (msg)
	{
		Archive::put(target, msg->getToolId ());
		Archive::put(target, msg->getStationId ());
		
		int count = msg->getSchematicCount();
		Archive::put(target, count);
		for (int i = 0; i < count; ++i)
		{
			const MessageQueueDraftSchematics::SchematicData & data = msg->getSchematic(i);
			Archive::put(target, data.serverCrc);
			Archive::put(target, data.sharedCrc);
			Archive::put(target, data.category);
		}
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueDraftSchematics::unpack(Archive::ReadIterator & source)
{
	NetworkId toolId, stationId;
	Archive::get(source, toolId);
	Archive::get(source, stationId);
	
	MessageQueueDraftSchematics* const msg = new MessageQueueDraftSchematics(toolId, stationId);
	
	int count, category;
	uint32 serverCrc, sharedCrc;
	Archive::get(source, count);
	for (int i = 0; i < count; ++i)
	{
		Archive::get(source, serverCrc);
		Archive::get(source, sharedCrc);
		Archive::get(source, category);
		msg->addSchematic(std::make_pair(serverCrc, sharedCrc), category);
	}
	
	return msg;
}

//----------------------------------------------------------------------
