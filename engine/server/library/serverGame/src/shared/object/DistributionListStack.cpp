// ======================================================================
//
// DistributionListStack.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/DistributionListStack.h"
#include <map>
#include <vector>

// ======================================================================

namespace DistributionListStackNamespace
{
	const unsigned int cms_maxDistributionLists = 16;
	std::map<ConnectionServerConnection *, std::vector<NetworkId> > s_distributionLists[cms_maxDistributionLists];
	unsigned int s_distributionListCount;
}
using namespace DistributionListStackNamespace;

// ======================================================================

std::map<ConnectionServerConnection *, std::vector<NetworkId> > &DistributionListStack::alloc()
{
	FATAL(s_distributionListCount == cms_maxDistributionLists, ("DistributionListStack::alloc too many entries on stack"));
	return s_distributionLists[s_distributionListCount++];
}

// ----------------------------------------------------------------------

void DistributionListStack::release()
{
	FATAL(s_distributionListCount == 0, ("DistributionListStack::release with empty stack"));
	s_distributionLists[--s_distributionListCount].clear();
}

// ======================================================================

