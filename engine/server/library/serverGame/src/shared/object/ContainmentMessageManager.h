// ======================================================================
//
// ContainmentMessageManager.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _ContainmentMessageManager_H_
#define _ContainmentMessageManager_H_

// ======================================================================

#include "serverGame/Client.h"

// ======================================================================

class ContainmentMessageManager
{
public:
	static void addContainmentMessage(ServerObject &object);
	static void addFrameBaselines(NetworkId const &networkId, std::vector<Watcher<Client> > const &distributionList);
	static void update();
	
private:
	ContainmentMessageManager();
	~ContainmentMessageManager();
	ContainmentMessageManager(ContainmentMessageManager const &);
	ContainmentMessageManager &operator=(ContainmentMessageManager const &);
};

// ======================================================================

#endif // _ContainmentMessageManager_H_

