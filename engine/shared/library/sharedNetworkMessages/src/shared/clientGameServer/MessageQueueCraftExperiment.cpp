//========================================================================
//
// MessageQueueCraftExperiment.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCraftExperiment.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueCraftExperiment, CM_experimentMessage);

//===================================================================


/**
 * Class destructor.
 */
MessageQueueCraftExperiment::~MessageQueueCraftExperiment()
{
}	// MessageQueueCraftExperiment::~MessageQueueCraftExperiment
//----------------------------------------------------------------------
void MessageQueueCraftExperiment::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueCraftExperiment* const msg = safe_cast<const MessageQueueCraftExperiment*> (data);
	if (msg)
	{
		Archive::put (target, msg->getSequenceId());
		
		const std::vector<MessageQueueCraftExperiment::ExperimentInfo> & experiments =
			msg->getExperiments();
		int count = experiments.size();
		Archive::put (target, count);
		std::vector<MessageQueueCraftExperiment::ExperimentInfo>::const_iterator iter;
		for (iter = experiments.begin(); iter != experiments.end(); ++iter)
		{
			Archive::put (target, (*iter).attributeIndex);
			Archive::put (target, (*iter).experimentPoints);
		}

		Archive::put(target, msg->getCoreLevel());
	}
}

//-----------------------------------------------------------------------
MessageQueue::Data* MessageQueueCraftExperiment::unpack(Archive::ReadIterator & source)
{
	uint8 sequenceId;
	Archive::get(source, sequenceId);
	MessageQueueCraftExperiment* const msg = new MessageQueueCraftExperiment(sequenceId);
	
	int count;
	int attribute, points;
	int corelevel;
	Archive::get(source, count);
	for (int i = 0; i < count; ++i)
	{
		Archive::get(source, attribute);
		Archive::get(source, points);
		msg->addExperiment(attribute, points);
	}

	Archive::get(source, corelevel);
	msg->setCoreLevel(corelevel);
	
	return msg;
}

//----------------------------------------------------------------------

