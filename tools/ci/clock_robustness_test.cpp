// Regression test: server clocks survive host clock steps.
// Built by run-clocks.py once per clock implementation, selected by one of:
//  TEST_SERVER_CLOCK  ServerClock game time never goes backwards; setting it is exact.
//  TEST_UDP_CLOCK     UdpMisc::Clock is monotonic, far from 0, and keeps the
//                     sync-stamp bits equal to CLOCK_MONOTONIC milliseconds.
//  TEST_TCP_CLOCK     TcpLibrary Clock actually reads a clock.
// Prints PASS/FAIL lines and ALL PASS.

#include <cstdio>
#include <ctime>
#include <time.h>
#include <unistd.h>

namespace
{
	int failures = 0;

	void check(bool ok, char const *what)
	{
		std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
		if (!ok)
			++failures;
	}

	long long monotonicMs()
	{
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return static_cast<long long>(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
	}
}

#if defined(TEST_SERVER_CLOCK)

#include "sharedFoundation/FirstSharedFoundation.h"
#include "serverUtility/ServerClock.h"
#include "sharedLog/LogManager.h"
#include <string>

// The host clock as ServerClock sees it: time() is replaced for this process.
static time_t s_hostTime = 1000000;
extern "C" time_t time(time_t *t) __THROW
{
	if (t)
		*t = s_hostTime;
	return s_hostTime;
}

// Stand-ins for the log calls ServerClock makes; they count lines.
static int s_logLines = 0;
void LogManager::setArgs(std::string const &) {}
void LogManager::log(char const *, ...) { ++s_logLines; }

static void frame(time_t hostTime)
{
	s_hostTime = hostTime;
	ServerClock::getInstance().incrementServerFrame();
}

static void testServerClock()
{
	ServerClock &clock = ServerClock::getInstance();
	clock.setGameTimeSeconds(500);
	check(clock.getGameTimeSeconds() == 500, "ServerClock: game time is exactly the loaded value");

	frame(1000010);
	check(clock.getGameTimeSeconds() == 510, "ServerClock: game time follows the host clock");

	int const logsBefore = s_logLines;
	frame(1000010 - 3600);
	check(clock.getGameTimeSeconds() == 510, "ServerClock: host clock one hour back does not move game time back");
	check(s_logLines == logsBefore + 1, "ServerClock: the backward step is logged once");
	frame(1000010 - 3500);
	check(clock.getGameTimeSeconds() == 510, "ServerClock: game time holds while the host clock catches up");
	check(s_logLines == logsBefore + 1, "ServerClock: no log line per held frame");
	frame(1000011);
	check(clock.getGameTimeSeconds() == 511, "ServerClock: game time advances again once the host clock catches up");

	frame(1000011 + 7200);
	check(clock.getGameTimeSeconds() == 511 + 7200, "ServerClock: a forward step is kept");

	// Loading the clock while held back must still give exactly the loaded value.
	frame(1000011);
	clock.setGameTimeSeconds(90000);
	check(clock.getGameTimeSeconds() == 90000, "ServerClock: setGameTimeSeconds is exact while game time is held");
	frame(1000011 + 7200 + 5);
	check(clock.getGameTimeSeconds() == 90005, "ServerClock: and advances from there");
}

#elif defined(TEST_UDP_CLOCK)

#include "UdpLibrary.hpp"

static void testUdpClock()
{
	check(UdpMisc::ClockElapsed(0) == 2000000000, "UdpMisc::Clock: a stamp of 0 reads as long ago");

	long long const before = monotonicMs();
	UdpMisc::ClockStamp const stamp = UdpMisc::Clock();
	long long const after = monotonicMs();
	unsigned const low = static_cast<unsigned>(stamp & 0xffffffff);
	check(static_cast<unsigned>(low - static_cast<unsigned>(before)) <= static_cast<unsigned>(after - before),
		"UdpMisc::Clock: sync-stamp bits are CLOCK_MONOTONIC milliseconds");

	UdpMisc::ClockStamp last = UdpMisc::Clock();
	bool monotonic = true;
	for (int i = 0; i < 100000; ++i)
	{
		UdpMisc::ClockStamp const now = UdpMisc::Clock();
		monotonic = monotonic && now >= last;
		last = now;
	}
	check(monotonic, "UdpMisc::Clock: never decreases");

	UdpMisc::ClockStamp const start = UdpMisc::Clock();
	usleep(30000);
	int const elapsed = UdpMisc::ClockElapsed(start);
	check(elapsed >= 29 && elapsed < 5000, "UdpMisc::ClockElapsed: measures a 30 ms sleep");
}

#elif defined(TEST_TCP_CLOCK)

#include "Clock.h"

static void testTcpClock()
{
	Clock clock;
	clock.start();
	check(!clock.isDone(60000), "TcpLibrary Clock: not done a minute early");
	usleep(30000);
	check(clock.isDone(20), "TcpLibrary Clock: done after a 30 ms sleep for a 20 ms timer");
	check(!clock.isDone(60000), "TcpLibrary Clock: still not done for a one-minute timer");
}

#endif

int main()
{
#if defined(TEST_SERVER_CLOCK)
	testServerClock();
#elif defined(TEST_UDP_CLOCK)
	testUdpClock();
#elif defined(TEST_TCP_CLOCK)
	testTcpClock();
#endif
	(void)monotonicMs;
	std::printf(failures ? "FAILED %d\n" : "ALL PASS\n", failures);
	return failures ? 1 : 0;
}
