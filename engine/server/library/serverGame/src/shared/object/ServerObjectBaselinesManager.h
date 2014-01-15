// ======================================================================
//
// ServerObjectBaselinesManager.h
//
// Copyright 2000-2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerObjectBaselinesManager_H
#define INCLUDED_ServerObjectBaselinesManager_H

// ======================================================================

class Client;
class NetworkId;
class ServerObject;

// ======================================================================

class ServerObjectBaselinesManager
{
public:
	static void add(ServerObject const &object);
	static void addOpenedContainer(Client &client, int sequence, NetworkId const &containerId, std::string const &slotName);
	static void update();
	static void serverObjDestroyed(ServerObject const &obj);
	static void onContainerChanged(ServerObject const &object, ServerObject const *oldContainer, ServerObject const *newContainer);
	
private:
	ServerObjectBaselinesManager();
	~ServerObjectBaselinesManager();
	ServerObjectBaselinesManager(ServerObjectBaselinesManager const &);
	ServerObjectBaselinesManager &operator=(ServerObjectBaselinesManager const &);
};

// ======================================================================

#endif // INCLUDED_ServerObjectBaselinesManager_H

