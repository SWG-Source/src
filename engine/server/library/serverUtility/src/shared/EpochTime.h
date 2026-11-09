// EpochTime.h
// Calendar (Unix epoch) time helpers for server code.

#ifndef INCLUDED_EpochTime_H
#define INCLUDED_EpochTime_H

//-----------------------------------------------------------------------

#include <cstdint>
#include <ctime>
#include <limits>

//-----------------------------------------------------------------------

// Server code keeps calendar time as signed 64-bit seconds since the Unix
// epoch: in memory, in server-to-server messages and in database binds.
// Signed 32-bit seconds overflow on 2038-01-19 03:14:08 UTC.
//
// The stock game client still uses signed 32-bit calendar times. Convert at
// the client boundary with toWire32(), which wraps modulo 2^32 exactly as the
// 32-bit server build always did, so the bytes sent to the client do not
// change. Java scripts use the same wrapped int convention (see
// getCalendarTime); fromWire32() recovers the full value near a reference.

namespace EpochTime
{
	// Now, as 64-bit epoch seconds. Use this instead of
	// static_cast<int>(::time(nullptr)).
	inline int64_t now()
	{
		return static_cast<int64_t>(::time(nullptr));
	}

	// Two's-complement wrap to the legacy signed 32-bit form.
	inline int32_t toWire32(int64_t const value)
	{
		uint32_t const low = static_cast<uint32_t>(static_cast<uint64_t>(value));
		return (low <= static_cast<uint32_t>((std::numeric_limits<int32_t>::max)()))
			? static_cast<int32_t>(low)
			: static_cast<int32_t>(static_cast<int64_t>(low) - (static_cast<int64_t>(1) << 32));
	}

	// The 64-bit time whose wrapped form is wire32 and which lies within
	// about 68 years of reference (normally now()).
	inline int64_t fromWire32(int32_t const wire32, int64_t const reference)
	{
		int32_t const delta = toWire32(static_cast<int64_t>(wire32) - reference);
		return reference + delta;
	}

	// base + seconds, saturating instead of overflowing.
	inline int64_t add(int64_t const base, int64_t const seconds)
	{
		if (seconds > 0 && base > (std::numeric_limits<int64_t>::max)() - seconds)
			return (std::numeric_limits<int64_t>::max)();
		if (seconds < 0 && base < (std::numeric_limits<int64_t>::min)() - seconds)
			return (std::numeric_limits<int64_t>::min)();
		return base + seconds;
	}

	// a - b clamped to the int range; for remaining/elapsed second counts
	// that are handed to int fields.
	inline int clampedDifference(int64_t const a, int64_t const b)
	{
		int64_t const d = add(a, -b);
		if (d > (std::numeric_limits<int>::max)())
			return (std::numeric_limits<int>::max)();
		if (d < (std::numeric_limits<int>::min)())
			return (std::numeric_limits<int>::min)();
		return static_cast<int>(d);
	}
}

//-----------------------------------------------------------------------

#endif
