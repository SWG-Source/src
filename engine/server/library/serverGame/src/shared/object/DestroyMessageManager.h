// ======================================================================
//
// DestroyMessageManager.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _DestroyMessageManager_H_
#define _DestroyMessageManager_H_

// ======================================================================

class Client;
class NetworkId;

// ======================================================================

class DestroyMessageManager
{
public:
	static void add(Client *client, NetworkId const &destroyId, bool hyperspace);
	static bool remove(Client const *client, NetworkId const &destroyId);
	static void update();
	
private:
	DestroyMessageManager();
	~DestroyMessageManager();
	DestroyMessageManager(DestroyMessageManager const &);
	DestroyMessageManager &operator=(DestroyMessageManager const &);
};

// ======================================================================

#endif // _DestroyMessageManager_H_

