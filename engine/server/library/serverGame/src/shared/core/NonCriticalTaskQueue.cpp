// ======================================================================
//
// NonCriticalTaskQueue.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/NonCriticalTaskQueue.h"

#include "serverGame/ConfigServerGame.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedLog/Log.h"

#include <list>

// ======================================================================

NonCriticalTaskQueue::NonCriticalTaskQueue() :
		m_tasks(new std::list<TaskRequest*>)
{
	ExitChain::add(&remove, "NonCriticalTaskQueue::remove");
}

// ----------------------------------------------------------------------

NonCriticalTaskQueue::~NonCriticalTaskQueue()
{
	for (TaskListType::const_iterator i=m_tasks->begin(); i!=m_tasks->end(); ++i)
	{
		delete *i;
	}
	delete m_tasks;
	m_tasks=0;
}

// ----------------------------------------------------------------------

void NonCriticalTaskQueue::addTask(TaskRequest *newTask)
{
	m_tasks->push_back(newTask);
	//LOG("NonCritialTaskQueue",("Added a task to the queue."));
}

// ----------------------------------------------------------------------

void NonCriticalTaskQueue::update()
{
	int taskLimit=ConfigServerGame::getNonCriticalTasksPerFrame();
	if (taskLimit <= 0)
		taskLimit = m_tasks->size();
	
	if (static_cast<int>(m_tasks->size()) > taskLimit)
		LOG("NonCritialTaskQueue",("Backlog:  %i tasks in the queue, %i will be done this frame.",m_tasks->size(),taskLimit));

	while ((taskLimit-- > 0) && !(m_tasks->empty()))
	{
		TaskRequest *task = m_tasks->front();
		m_tasks->pop_front();

		if (task->run())
		{
			delete task;
			task=0;
//			LOG("NonCritialTaskQueue",("Deleting completed task."));
		}
		else
		{
			addTask(task);
//			LOG("NonCritialTaskQueue",("Re-queueing non-completed task."));
		}
	}
}

// ======================================================================

NonCriticalTaskQueue::TaskRequest::~TaskRequest()
{
}

// ======================================================================
