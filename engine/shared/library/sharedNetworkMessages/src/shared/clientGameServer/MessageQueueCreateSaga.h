//========================================================================
//
// MessageQueueCreateSaga.h
//
// copyright 2009 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueCreateSaga_H
#define INCLUDED_MessageQueueCreateSaga_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

class MessageQueueCreateSaga : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	struct TaskInfo
	{
	public:
		TaskInfo()
		{

		}

		TaskInfo(int type, std::vector<std::string> params)
		{
			taskType = type;
			numParameters = params.size();
			for(int i = 0; i < numParameters; ++i)
				parameters.push_back(params[i]);
		}

		int				  taskType; // Saga Task Type.
		
		int						  numParameters;
		std::vector<std::string>  parameters;
	//private:
	//	TaskInfo & operator = (const TaskInfo &);
	};

public:

	MessageQueueCreateSaga();
	virtual ~MessageQueueCreateSaga();

	void							  setTotalTasks(int total);
	int								  getTotalTasks() const;
	
	void							  addTask(int taskType, std::vector<std::string> & parameters);
	const std::vector<TaskInfo> &     getTasks() const;

	void							  setQuestName(std::string name);
	std::string const &				  getQuestName() const;

	void							  setQuestDescription(std::string desc);
	std::string const &				  getQuestDescription() const;

	void							  setRewards(std::string rewards);
	std::string const &				  getRewards() const;

	void							  setRecipe(bool recipe);
	bool							  isRecipe() const;

	void							  setRecipeOID(NetworkId const & id);
	NetworkId const &				  getRecipeOID() const;

	void							  setShareAmount(int amount);
	int								  getShareAmount() const;

private:
	std::string					m_questName;
	std::string					m_questDescription;
	std::vector<TaskInfo>		m_tasks;
	int                         m_totalTasks;
	std::string					m_rewards;
	int							m_recipe;
	NetworkId					m_oldRecipeId;
	int							m_shareAmount;
};

inline MessageQueueCreateSaga::MessageQueueCreateSaga()
{

}

inline void MessageQueueCreateSaga::setRecipeOID(NetworkId const & id)
{
	m_oldRecipeId = id;
}

inline NetworkId const & MessageQueueCreateSaga::getRecipeOID() const
{
	return m_oldRecipeId;
}

inline void MessageQueueCreateSaga::setRecipe(bool recipe)
{
	m_recipe = recipe;
}

inline bool MessageQueueCreateSaga::isRecipe() const
{
	return m_recipe;
}

inline void MessageQueueCreateSaga::setTotalTasks(int total)
{
	m_totalTasks = total;
}

inline int MessageQueueCreateSaga::getTotalTasks() const
{
	return m_totalTasks;
}

inline void MessageQueueCreateSaga::setShareAmount(int amount)
{
	m_shareAmount = amount;
}

inline int MessageQueueCreateSaga::getShareAmount() const
{
	return m_shareAmount;
}

inline const std::vector<MessageQueueCreateSaga::TaskInfo> & MessageQueueCreateSaga::getTasks() const
{
	return m_tasks;
}

inline void MessageQueueCreateSaga::setQuestName(std::string name)
{
	m_questName = name;
}

inline std::string const & MessageQueueCreateSaga::getQuestName() const
{
	return m_questName;
}

inline void MessageQueueCreateSaga::setQuestDescription(std::string desc)
{
	m_questDescription = desc;
}

inline std::string const & MessageQueueCreateSaga::getQuestDescription() const
{
	return m_questDescription;
}

inline void MessageQueueCreateSaga::setRewards(std::string rewards)
{
	m_rewards = rewards;
}

inline std::string const & MessageQueueCreateSaga::getRewards() const
{
	return m_rewards;
}

#endif	// INCLUDED_MessageQueueCreateSaga_H
