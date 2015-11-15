//======================================================================
//
// MessageQueueHarvesterResourceData.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueHarvesterResourceData.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedUtility/InstallationResourceDataArchive.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueHarvesterResourceData, CM_clientResourceHarvesterResourceData);

//===================================================================


//-----------------------------------------------------------------------

MessageQueueHarvesterResourceData::MessageQueueHarvesterResourceData (const NetworkId & harvesterId, const DataVector & data) :
Data (),
m_harvesterId (harvesterId),
m_data (data)
{
}

//-----------------------------------------------------------------------

MessageQueueHarvesterResourceData::~MessageQueueHarvesterResourceData()
{
}

//-----------------------------------------------------------------------

void MessageQueueHarvesterResourceData::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueHarvesterResourceData* const msg = safe_cast<const MessageQueueHarvesterResourceData*> (data);
	if (msg)
	{
		Archive::put (target, msg->getHarvesterId ());
		Archive::AutoArray<InstallationResourceData> array;
		array.set (msg->getDataVector ());
		Archive::put (target, array);
	}
}

//----------------------------------------------------------------------


MessageQueue::Data* MessageQueueHarvesterResourceData::unpack(Archive::ReadIterator & source)
{
	NetworkId id;
	Archive::get(source, id);
	
	Archive::AutoArray<InstallationResourceData> array;
	Archive::get(source, array);

	MessageQueue::Data * result = new MessageQueueHarvesterResourceData (id, array.get ());
	return result;
}

//----------------------------------------------------------------------

