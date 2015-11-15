// ======================================================================
//
// ServerBuildoutManager.h
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerBuildoutManager_H
#define INCLUDED_ServerBuildoutManager_H

// ======================================================================

class Client;

// ======================================================================

class ServerBuildoutManager
{
public:
	static void install();
	static void onChunkComplete(int nodeX, int nodeZ);
	static void saveArea(std::string const &serverFileName, std::string const &clientFileName, float x1, float z1, float x2, float z2);
	static void clientSaveArea(Client &client, std::string const &areaName, float x1, float z1, float x2, float z2);
	static void destroyPersistedDuplicates();
	static void editBuildoutArea(std::string const &areaName);
	static void onEventStarted(std::string const & eventName);
	static void onEventStopped(std::string const & eventName);
};

// ======================================================================

#endif // INCLUDED_ServerBuildoutManager_H

