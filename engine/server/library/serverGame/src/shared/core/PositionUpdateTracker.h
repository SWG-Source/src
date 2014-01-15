// ======================================================================
//
// PositionUpdateTracker.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _PositionUpdateTracker_H_
#define _PositionUpdateTracker_H_

// ======================================================================

class Scheduler;
class ServerObject;

// ======================================================================

class PositionUpdateTracker
{
public:
	static void install();
	static void remove();

	static void positionChanged(ServerObject &obj);
	static void flushPositionUpdate(ServerObject &obj);
	static void sendPositionUpdate(ServerObject &obj);
	static void serverObjDestroyed(ServerObject &obj);
	static Scheduler &getScheduler();

private:
	static bool shouldSendPositionUpdate(ServerObject const &obj);
	static void handlePositionUpdateLongDelayCallback(void const *context);
	static void handlePositionUpdateShortDelayCallback(void const *context);

private:
	PositionUpdateTracker();
	PositionUpdateTracker(PositionUpdateTracker const &);
	PositionUpdateTracker &operator=(PositionUpdateTracker const &);
};

// ======================================================================

#endif // _PositionUpdateTracker_H_

