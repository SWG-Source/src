// ======================================================================
// 
// SpacePathManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/SpacePathManager.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/SpacePath.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedLog/Log.h"

#include <algorithm>
#include <deque>
#include <set>

// ======================================================================
//
// SpacePathManagerNamespace
//
// ======================================================================

namespace SpacePathManagerNamespace
{
	typedef std::set<SpacePath *> PathList;
	PathList s_pathList;

	typedef std::deque<SpacePath *> PathQueue;
	PathQueue s_pathQueue;
}

using namespace SpacePathManagerNamespace;

// ======================================================================
//
// SpacePathManager
//
// ======================================================================

// ----------------------------------------------------------------------
void SpacePathManager::install()
{
	ExitChain::add(&remove, "SpacePathManager::remove");
	LOGC(ConfigServerGame::getSpaceAiUsePathRefinement(), "space_debug_ai", ("SpacePathManager::install() Per frame path refinement is disabled!"));
}

// ----------------------------------------------------------------------
void SpacePathManager::remove()
{
	if (!s_pathList.empty())
	{
		WARNING(true, ("SpacePathManager::remove() The path list is not empty(%u), this is a sign of a reference counting problem.", s_pathList.size()));
	}
}

// ----------------------------------------------------------------------
SpacePath * SpacePathManager::fetch(SpacePath * const path, void const * const object, float const objectRadius)
{
	SpacePath * result = nullptr;

	if (path == nullptr)
	{
		// This is a new path, add it to the list
	
		result = new SpacePath;
		IGNORE_RETURN(s_pathList.insert(result));
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpacePathManager::fetch() NEW PATH path(0x%p) referenceCount(%d) totalPathCount(%u)", result, result->getReferenceCount(), s_pathList.size()));
	}
	else
	{
		PathList::iterator iterPathList = s_pathList.find(path);
	
		if (iterPathList != s_pathList.end())
		{
			// This path already exists, increase the reference count
	
			result = *iterPathList;
	
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpacePathManager::fetch() path(0x%p) referenceCount(%d) totalPathCount(%u)", path, (*iterPathList)->getReferenceCount(), s_pathList.size()));
		}
		else
		{
			FATAL(true, ("SpacePathManager::fetch() ERROR: Unable to fetch the path(0x%p) totalPathCount(%u)", path, s_pathList.size()));
		}
	}

	result->addReference(object, objectRadius);

	// Queue up a request to process this path.
	if(ConfigServerGame::getSpaceAiUsePathRefinement())
	{
		if (!result->getUpdateFlag())
		{
			result->setUpdateFlag(true);
			s_pathQueue.push_back(result);
		}
	}

	return result;
}

// ----------------------------------------------------------------------
void SpacePathManager::release(SpacePath * const path, void const * const object)
{
	if (path == nullptr)
	{
		return;
	}

	PathList::iterator iterPathList = s_pathList.find(path);
	
	if (iterPathList != s_pathList.end())
	{
		path->releaseReference(object);

		if (path->getReferenceCount() <= 0)
		{
			removeFromRefineQueue(path);

			s_pathList.erase(iterPathList);

			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpacePathManager::release() Removing 0 ref count path(0x%p) totalPathCount(%u)", path, s_pathList.size()));

			delete path;
		}
		else
		{
			LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpacePathManager::release() path(0x%p) referenceCount(%d) totalPathCount(%u)", path, (*iterPathList)->getReferenceCount(), s_pathList.size()));
		}
	}
	else
	{
		LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("SpacePathManager::release() Unable to find the specified path(0x%p) totalPathCount(%u)", path, s_pathList.size()));
	}
}


// ----------------------------------------------------------------------
void SpacePathManager::alter(float const /*deltaTime*/)
{
	if(ConfigServerGame::getSpaceAiUsePathRefinement())
	{
		int remainingAvoidanceChecks = ConfigServerGame::getSpaceAiAvoidanceChecksPerFrame();
		
		// Pop some updates off the queue.
		if (!s_pathQueue.empty())
		{
			while(!s_pathQueue.empty() && remainingAvoidanceChecks > 0)
			{
				// Shortcut to the object.
				SpacePath * const spacePath = s_pathQueue.front();
				
				// Get rid of this entry.
				s_pathQueue.pop_front();
				
				// Refine the path if it exists.
				bool notCompleted = !spacePath->refine(remainingAvoidanceChecks);
				
				// If we are not done, request another update.
				spacePath->setUpdateFlag(notCompleted);
				
				// Now, add to the queue if not completed.
				if (notCompleted)
				{
					// Schedule for update next frame.
					s_pathQueue.push_back(spacePath);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------
void SpacePathManager::removeFromRefineQueue(SpacePath *path)
{
	IGNORE_RETURN(s_pathQueue.erase(std::remove(s_pathQueue.begin(), s_pathQueue.end(), path), s_pathQueue.end()));
}


// ======================================================================
