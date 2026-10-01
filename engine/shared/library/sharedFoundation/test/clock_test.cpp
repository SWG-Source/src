// Regression test: Clock milliseconds past 2^32 (about 49.7 days of uptime).
//
// Clock::timeMs() is CLOCK_MONOTONIC milliseconds. It used to return unsigned
// long, which is 32 bits on ILP32, and its values were stored in a mix of
// 32-bit and unsigned long variables. This test checks the contract on the
// real code, linked against this tree's sharedFoundation:
//  - timeMs() and getFrameStartTimeMs() are 64-bit on every ABI;
//  - Scheduler keeps ordering callbacks correctly across the 32-bit wrap;
//  - Clock::durationMs<T> narrows a duration by saturating, never wrapping.
// Build and run with clock-test.sh; it must print ALL PASS on -m32 and -m64.

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/PerThreadData.h"
#include "sharedFoundation/Scheduler.h"
#include "sharedFoundation/StaticCallbackEntry.h"

#include <cstdio>

namespace
{
	int failures = 0;

	void check(bool ok, char const *what)
	{
		std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
		if (!ok)
			++failures;
	}

	int fired = 0;
	void onTimer(const void *) { ++fired; }

	uint64_t const cs_2to32 = static_cast<uint64_t>(1) << 32;
}

int main()
{
	PerThreadData::install();
	StaticCallbackEntry::install();
	std::printf("sizeof(long)=%u\n", unsigned(sizeof(long)));

	// --- Clock values are 64-bit on every ABI
	check(sizeof(Clock::timeMs()) == 8, "Clock::timeMs() returns a 64-bit value");
	check(sizeof(Clock::getFrameStartTimeMs()) == 8, "Clock::getFrameStartTimeMs() returns a 64-bit value");

	// --- Scheduler: a callback set 100 ms before the 32-bit wrap, due 1 s later
	{
		Scheduler scheduler;
		scheduler.update(cs_2to32 - 100);
		scheduler.setCallback(onTimer, 0, 1000);
		scheduler.update(cs_2to32 - 50);
		check(fired == 0, "Scheduler: callback due after the 2^32 ms wrap does not fire 50 ms after it was set");
		scheduler.update(cs_2to32 + 800);
		check(fired == 0, "Scheduler: callback does not fire 900 ms after it was set");
		scheduler.update(cs_2to32 + 900);
		check(fired == 1, "Scheduler: callback fires exactly when due, past the wrap");
		check(scheduler.getCurrentCount() == cs_2to32 + 900, "Scheduler: current count keeps all 64 bits");
	}

#ifdef CLOCK_HAS_DURATION_MS
	// --- durationMs narrows a duration by saturating
	uint64_t const base = cs_2to32 + 5000;
	check(Clock::durationMs<int>(base, base + 1234) == 1234, "durationMs<int>: plain duration past 2^32 ms");
	check(Clock::durationMs<int>(base + 5, base) == 0, "durationMs: 0 when end precedes start");
	check(Clock::durationMs<uint32_t>(0, static_cast<uint64_t>(1) << 40) == 0xFFFFFFFFu, "durationMs<uint32_t> saturates");
	check(Clock::durationMs<int>(0, static_cast<uint64_t>(1) << 40) == 0x7FFFFFFF, "durationMs<int> saturates");
#else
	check(false, "Clock::durationMs is declared in Clock.h");
#endif

	std::printf(failures ? "FAILED %d\n" : "ALL PASS\n", failures);
	return failures ? 1 : 0;
}
