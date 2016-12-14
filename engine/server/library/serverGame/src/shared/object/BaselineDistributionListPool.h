// ======================================================================
//
// BaselineDistributionListPool.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _BaselineDistributionListPool_H_
#define _BaselineDistributionListPool_H_

// ======================================================================

template <typename T> class Watcher;
class ServerObject;
class Client;

// ======================================================================

class BaselineDistributionListPool
{
public:
	static std::vector<Watcher<Client> > *get(ServerObject const &obj, bool allocIfNeeded);
	static void free(ServerObject const &obj);
};

// ======================================================================

#endif // _BaselineDistributionListPool_H_

