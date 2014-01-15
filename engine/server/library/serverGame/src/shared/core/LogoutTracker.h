// ======================================================================
//
// LogoutTracker.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _LogoutTracker_H_
#define _LogoutTracker_H_

// ======================================================================

class NetworkId;
class Scheduler;
class ServerObject;

// ======================================================================

class LogoutTracker
{
public:
	static void install();
	static void remove();

	static void add(NetworkId const &networkId);
	static void handleLogoutCallback(void const *context);
	static bool isInLogoutCallbackList(NetworkId const &networkId);
	static Scheduler &getScheduler();
	static void addPendingSave(ServerObject *character);
	static void onPersisted(const NetworkId &character);
	static ServerObject *findPendingCharacterSave(const NetworkId &character);
	
private:
	
	LogoutTracker();
	LogoutTracker(LogoutTracker const &);
};

// ======================================================================

#endif // _LogoutTracker_H_

