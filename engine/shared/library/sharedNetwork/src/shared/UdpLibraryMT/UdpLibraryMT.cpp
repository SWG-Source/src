// ======================================================================
//
// UdpLibraryMT.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetwork/FirstSharedNetwork.h"
#include "UdpLibraryMT.h"
#include "Events.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/ConfigSharedNetwork.h"
#include "sharedSynchronization/Guard.h"
#include "sharedThread/RunThread.h"
#include <set>
#include <vector>

// ======================================================================

bool UdpLibraryMT::ms_useNetworkThread;

// ======================================================================

static std::set<UdpManager *> s_udpManagers;
static std::vector<UdpManager *> s_udpManagerPendingRemoves;
static bool s_updating;
static volatile bool s_threadRunning;
static volatile bool s_threadShutdown;

// ======================================================================

static void networkThreadFunc()
{
	Thread::getCurrentThread()->setPriority(static_cast<Thread::ePriority>(ConfigSharedNetwork::getNetworkThreadPriority()));
	while (!s_threadShutdown)
	{
		Os::sleep(ConfigSharedNetwork::getNetworkThreadSleepTimeMs());
		UdpLibraryMT::networkThreadUpdate();
	}
	s_threadRunning = false;
}

// ----------------------------------------------------------------------

static void startNetworkThread()
{
	s_threadShutdown = false;
	if (!s_threadRunning)
	{
		s_threadRunning = true;
		runThread(networkThreadFunc);
	}
}

// ----------------------------------------------------------------------

static void stopNetworkThread()
{
	s_threadShutdown = true;
	while (s_threadRunning)
		Os::sleep(1);
	s_threadShutdown = false;
}

// ======================================================================

void UdpLibraryMT::install()
{
	Events::install();
	ms_useNetworkThread = ConfigSharedNetwork::getUseNetworkThread();
	if (ms_useNetworkThread)
		startNetworkThread();
}

// ----------------------------------------------------------------------

void UdpLibraryMT::remove()
{
	if (s_threadRunning)
		stopNetworkThread();

	{
		Guard lock(getMutex());
		Events::remove();
	}
}

// ----------------------------------------------------------------------

RecursiveMutex &UdpLibraryMT::getMutex()
{
	static RecursiveMutex mutex;
	return mutex;
}

// ----------------------------------------------------------------------

void UdpLibraryMT::registerUdpManager(UdpManager *udpManager)
{
	s_udpManagers.insert(udpManager);
}

// ----------------------------------------------------------------------

void UdpLibraryMT::unregisterUdpManager(UdpManager *udpManager)
{
	if (s_updating)
	{
		udpManager->AddRef();
		s_udpManagerPendingRemoves.push_back(udpManager);
	}
	else
	{
		std::set<UdpManager*>::iterator i = s_udpManagers.find(udpManager);
		if (i != s_udpManagers.end())
			s_udpManagers.erase(i);
	}
}

// ----------------------------------------------------------------------

void UdpLibraryMT::mainThreadUpdate()
{
	// update from main thread - process incoming events

	Guard lock(getMutex());

	Events::processIncoming();
}

// ----------------------------------------------------------------------

void UdpLibraryMT::networkThreadUpdate()
{
	// update from network thread - process outgoing events, then give time to the UdpManagers
#ifdef _DEBUG
	unsigned long const lockStart = Clock::timeMs();
#endif

	Guard lock(getMutex());

#ifdef _DEBUG
	unsigned long const lockStop = Clock::timeMs();
#endif

	Events::processOutgoing();

	s_updating = true;

	{
		for (std::set<UdpManager *>::iterator i = s_udpManagers.begin(); i != s_udpManagers.end(); ++i)
			(*i)->GiveTime();
	}

	s_updating = false;

	{
		for (std::vector<UdpManager *>::iterator i = s_udpManagerPendingRemoves.begin(); i != s_udpManagerPendingRemoves.end(); ++i)
		{
			unregisterUdpManager(*i);
			(*i)->Release();
		}
		s_udpManagerPendingRemoves.clear();
	}

#ifdef _DEBUG
	unsigned long const updateStop = Clock::timeMs();

	if (updateStop - lockStart > 500)
	{
		static const Os::OsPID_t id = Os::getProcessId();
		fprintf(stderr, "Network:PID %d:ThreadUpdate:guard lock time=%lu, processing time = %lu\n", id, lockStop - lockStart, updateStop - lockStop);
	}
#endif
}

// ======================================================================

