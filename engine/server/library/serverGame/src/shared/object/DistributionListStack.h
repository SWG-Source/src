// ======================================================================
//
// DistributionListStack.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _DistributionListStack_H_
#define _DistributionListStack_H_

// ======================================================================

#include "sharedFoundation/NetworkId.h"

// ======================================================================

class ConnectionServerConnection;

// ======================================================================

class DistributionListStack
{
public:
	static std::map<ConnectionServerConnection *, std::vector<NetworkId>> &alloc();
	static void release();

private:
	DistributionListStack();
	DistributionListStack(DistributionListStack const &);
	DistributionListStack &operator=(DistributionListStack const &);
};

// ======================================================================

#endif // _DistributionListStack_H_

