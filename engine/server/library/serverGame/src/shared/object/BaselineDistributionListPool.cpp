// ======================================================================
//
// BaselineDistributionListPool.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/BaselineDistributionListPool.h"
#include "serverGame/Client.h"

// ======================================================================

static std::map<ServerObject const *, std::vector<Watcher<Client> > > s_pools;

// ======================================================================

std::vector<Watcher<Client> > *BaselineDistributionListPool::get(ServerObject const &obj, bool allocIfNeeded) // static
{
	if (allocIfNeeded)
		return &(s_pools[&obj]);
	std::map<ServerObject const *, std::vector<Watcher<Client> > >::iterator i = s_pools.find(&obj);
	if (i != s_pools.end())
		return &((*i).second);
	return 0;
}

// ----------------------------------------------------------------------

void BaselineDistributionListPool::free(ServerObject const &obj) // static
{
	s_pools.erase(&obj);
}

// ======================================================================

