//========================================================================
//
// MessageQueueCreateSaga.cpp
//
// copyright 2009 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCreateSaga.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueCreateSaga, CM_createSaga);

//===================================================================


/**
* Class destructor.
*/
MessageQueueCreateSaga::~MessageQueueCreateSaga()
{
}	// MessageQueueCreateSaga::~MessageQueueCreateSaga
//----------------------------------------------------------------------
void MessageQueueCreateSaga::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueCreateSaga* const msg = safe_cast<const MessageQueueCreateSaga*> (data);
	if (msg)
	{
		Archive::put(target, msg->getQuestName());
		Archive::put(target, msg->getQuestDescription());

		const std::vector<MessageQueueCreateSaga::TaskInfo> & tasks =
			msg->getTasks();
		int count = static_cast<int>(tasks.size());
		Archive::put (target, count);
		std::vector<MessageQueueCreateSaga::TaskInfo>::const_iterator iter;
		for (iter = tasks.begin(); iter != tasks.end(); ++iter)
		{
			Archive::put (target, (*iter).taskType);
			Archive::put (target, (*iter).numParameters);
			for(int i = 0; i < (*iter).numParameters; ++i)
				Archive::put (target, (*iter).parameters[i]);
		}

		Archive::put(target, msg->getRewards());
		Archive::put(target, msg->isRecipe());
		Archive::put(target, msg->getRecipeOID());
		Archive::put(target, msg->getShareAmount());
	}
}

//-----------------------------------------------------------------------
MessageQueue::Data* MessageQueueCreateSaga::unpack(Archive::ReadIterator & source)
{
	MessageQueueCreateSaga* const msg = new MessageQueueCreateSaga();
	
	std::string questName;
	std::string questDescription;

	Archive::get(source, questName);

	msg->setQuestName(questName);

	Archive::get(source, questDescription);

	msg->setQuestDescription(questDescription);

	int numTasks;
	int taskType, numParameters;
	Archive::get(source, numTasks);
	msg->setTotalTasks(numTasks);
	for (int i = 0; i < numTasks; ++i)
	{
		Archive::get(source, taskType);
		Archive::get(source, numParameters);
		std::string tempParam;
		std::vector<std::string> paramVector;
		for(int j = 0; j < numParameters; ++j)
		{
			Archive::get(source, tempParam);
			paramVector.push_back(tempParam);
		}

		msg->addTask(taskType, paramVector);
	}

	std::string rewards;
	Archive::get(source, rewards);
	msg->setRewards(rewards);
	
	bool recipe;
	Archive::get(source, recipe);
	msg->setRecipe(recipe);

	NetworkId recipeOID;
	Archive::get(source, recipeOID);
	msg->setRecipeOID(recipeOID);

	int shareAmount;
	Archive::get(source, shareAmount);
	msg->setShareAmount(shareAmount);

	return msg;
}

void MessageQueueCreateSaga::addTask(int taskType, std::vector<std::string> & parameters)
{
	m_tasks.push_back(TaskInfo(taskType, parameters));
}

//----------------------------------------------------------------------

