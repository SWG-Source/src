// ======================================================================
//
// UdpLibraryMT.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _UdpLibraryMT_H_
#define _UdpLibraryMT_H_

// ======================================================================

#include "UdpLibrary.h"

#include "UdpConnectionMT.h"
#include "UdpManagerMT.h"
#include "UdpMiscMT.h"

// ======================================================================

class EventBase;
class RecursiveMutex;

// ======================================================================

class UdpLibraryMT
{
public:
	static void install();
	static void remove();

	static RecursiveMutex &getMutex();
	static bool getUseNetworkThread();
	static void registerUdpManager(UdpManager *udpManager);
	static void unregisterUdpManager(UdpManager *udpManager);
	static void pushOutgoingEvent(EventBase *event);
	static void pushIncomingEvent(EventBase *event);

	static void mainThreadUpdate();
	static void networkThreadUpdate();

private:
	UdpLibraryMT();
	UdpLibraryMT(UdpLibraryMT const &);
	UdpLibraryMT& operator=(UdpLibraryMT const &);

	static bool ms_useNetworkThread;
};

// ----------------------------------------------------------------------

inline bool UdpLibraryMT::getUseNetworkThread()
{
	return ms_useNetworkThread;
}

// ======================================================================

#endif // _UdpLibraryMT_H_

