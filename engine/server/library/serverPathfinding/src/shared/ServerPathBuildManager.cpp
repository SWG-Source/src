// ======================================================================
//
// ServerPathBuildManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverPathfinding/FirstServerPathfinding.h"
#include "serverPathfinding/ServerPathBuildManager.h"

#include "serverPathfinding/ServerPathBuilder.h"

#include "sharedDebug/PerformanceTimer.h"

#include <algorithm>
#include <list>

typedef std::list< ServerPathBuilder * > BuildQueue;

BuildQueue gs_lowQueue;
BuildQueue gs_highQueue;

// ----------------------------------------------------------------------

void ServerPathBuildManager::install ( void )
{
}

void ServerPathBuildManager::remove ( void )
{
}

// ----------------------------------------------------------------------

void updateQueue ( BuildQueue * queue, PerformanceTimer const & timer, float timeBudget )
{
	BuildQueue::iterator it = queue->begin();

	while(it != queue->end())
	{
		if(timer.getSplitTime() >= timeBudget) return;

		ServerPathBuilder * currentBuilder = (*it);

		currentBuilder->update();

		BuildQueue::iterator old = it;
		it++;

		if(currentBuilder->buildDone())
		{
			currentBuilder->setQueued(false);
			queue->erase(old);
		}
	}
}

// ----------

void ServerPathBuildManager::update ( float timeBudget )
{
	PerformanceTimer timer;

	timer.start();

	updateQueue( &gs_highQueue, timer, timeBudget );

	updateQueue( &gs_lowQueue, timer, timeBudget );
}

// ----------------------------------------------------------------------

bool ServerPathBuildManager::queue ( ServerPathBuilder * builder, bool highPriority )
{
	if(builder->getQueued()) return false;

	if(highPriority)
	{
		gs_highQueue.push_back(builder);
	}
	else
	{
		gs_lowQueue.push_back(builder);
	}

	builder->setQueued(true);

	return true;
}

bool ServerPathBuildManager::unqueue ( ServerPathBuilder * builder )
{
	if(!builder->getQueued()) return false;

	BuildQueue::iterator it;
	
	it = std::find(gs_highQueue.begin(),gs_highQueue.end(),builder);

	if(it != gs_highQueue.end()) gs_highQueue.erase(it);

	it = std::find(gs_lowQueue.begin(),gs_lowQueue.end(),builder);

	if(it != gs_lowQueue.end()) gs_lowQueue.erase(it);

	builder->setQueued(false);

	return true;
}

