// ======================================================================
//
// SpaceVisibilityManager.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SpaceVisibilityManager_H
#define INCLUDED_SpaceVisibilityManager_H

// ======================================================================

class ServerObject;
class Client;
class Vector;

// ======================================================================

class SpaceVisibilityManager
{
  public:
	static void addClient           (Client & client, ServerObject & observer);
	static void removeClient        (Client & client);

	static void addObject           (ServerObject & object, int updateRadius);
	static void moveObject          (const ServerObject & object);
	static void removeObject        (ServerObject & object);

	static void getObjectsVisibleFromLocation (const Vector & location, std::vector<ServerObject*> &results);
	static void getClientsInRange (const Vector &location, float radius, std::set<Client*> &results);
};


// ======================================================================

#endif
