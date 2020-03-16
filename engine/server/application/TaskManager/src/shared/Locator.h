// ======================================================================
//
// Locator.h
//
// Copyright 2000-04 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_Locator_H
#define	_INCLUDED_Locator_H

// ======================================================================

#include <string>

class GameNetworkMessage;
class ManagerConnection;

// ======================================================================

class Locator
{
public:
	static void install();
	static void closed(std::string const &label, ManagerConnection const *oldConnection);
	static ManagerConnection *getBestServer(std::string const &processName, std::string const &options, float cost);
	static bool isMasterNodePreferred(std::string const &processName, std::string const &options, float cost);	
	static float getMyLoad();
	static float getMyMaximumLoad();
	static float getServerLoad(std::string const &label);
	static float getServerMaximumLoad(std::string const &label);
	static ManagerConnection *getServer(std::string const &label);
	static void incrementMyLoad(float amount);
	static void incrementServerLoad(std::string const &label, float amount);
	static void decrementMyLoad(float amount);
	static void decrementServerLoad(std::string const &label, float amount);
	static void opened(std::string const &label, ManagerConnection *newConnection);
	static void sendToAllTaskManagers(GameNetworkMessage const &msg);
	static void updateServerLoad(std::string const &label, float load);
	static void updateAllLoads(float delta);
	static std::map<std::string, std::string> const & getClosedConnections();
};

// ======================================================================

#endif	// _INCLUDED_Locator_H

