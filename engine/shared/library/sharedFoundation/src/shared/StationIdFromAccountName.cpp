// ======================================================================
//
// StationIdFromAccountName.cpp
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/StationIdFromAccountName.h"

#include <cstdlib>
#include <cstring>

// ======================================================================

namespace StationIdFromAccountNameNamespace
{
	uint32 const cs_seed = 0xc70f6907u;

	// Little-endian loads: the x86 builds that created existing StationIds
	// read the input this way.
	uint32 load32(unsigned char const * p)
	{
		return static_cast<uint32>(p[0])
			| (static_cast<uint32>(p[1]) << 8)
			| (static_cast<uint32>(p[2]) << 16)
			| (static_cast<uint32>(p[3]) << 24);
	}

	uint64 load64(unsigned char const * p)
	{
		return static_cast<uint64>(load32(p)) | (static_cast<uint64>(load32(p + 4)) << 32);
	}

	uint64 shiftMix(uint64 v)
	{
		return v ^ (v >> 47);
	}
}

using namespace StationIdFromAccountNameNamespace;

// ======================================================================

uint32 StationIdFromAccountName::gcc32StringHash(char const * const data, size_t length)
{
	// libstdc++ libsupc++/hash_bytes.cc, __SIZEOF_SIZE_T__ == 4 branch, with
	// every intermediate held in a 32-bit type.
	uint32 const m = 0x5bd1e995u;
	uint32 hash = cs_seed ^ static_cast<uint32>(length);
	unsigned char const * buf = reinterpret_cast<unsigned char const *>(data);

	while (length >= 4)
	{
		uint32 k = load32(buf);
		k *= m;
		k ^= k >> 24;
		k *= m;
		hash *= m;
		hash ^= k;
		buf += 4;
		length -= 4;
	}

	switch (length)
	{
	case 3:
		hash ^= static_cast<uint32>(buf[2]) << 16;
		// fall through
	case 2:
		hash ^= static_cast<uint32>(buf[1]) << 8;
		// fall through
	case 1:
		hash ^= static_cast<uint32>(buf[0]);
		hash *= m;
		break;
	default:
		break;
	}

	hash ^= hash >> 13;
	hash *= m;
	hash ^= hash >> 15;
	return hash;
}

// ----------------------------------------------------------------------

uint32 StationIdFromAccountName::gcc64StringHash(char const * const data, size_t const length)
{
	// libstdc++ libsupc++/hash_bytes.cc, __SIZEOF_SIZE_T__ == 8 branch, with
	// every intermediate held in a 64-bit type.
	uint64 const mul = (static_cast<uint64>(0xc6a4a793u) << 32) + static_cast<uint64>(0x5bd1e995u);
	unsigned char const * const buf = reinterpret_cast<unsigned char const *>(data);
	uint64 const len = static_cast<uint64>(length);
	uint64 const lenAligned = len & ~static_cast<uint64>(0x7);
	uint64 hash = static_cast<uint64>(cs_seed) ^ (len * mul);

	for (uint64 i = 0; i != lenAligned; i += 8)
	{
		uint64 const d = shiftMix(load64(buf + i) * mul) * mul;
		hash ^= d;
		hash *= mul;
	}

	uint64 const tail = len & 0x7;
	if (tail != 0)
	{
		// load_bytes(): the remaining bytes, little-endian.
		uint64 d = 0;
		for (uint64 n = tail; n-- > 0; )
			d = (d << 8) + static_cast<uint64>(buf[lenAligned + n]);
		hash ^= d;
		hash *= mul;
	}

	hash = shiftMix(hash) * mul;
	hash = shiftMix(hash);
	return static_cast<uint32>(hash);
}

// ----------------------------------------------------------------------

bool StationIdFromAccountName::parseAlgorithm(char const * const name, Algorithm & algorithm)
{
	if (name && strcmp(name, "gcc32") == 0)
	{
		algorithm = Gcc32Murmur2;
		return true;
	}
	if (name && strcmp(name, "gcc64") == 0)
	{
		algorithm = Gcc64Murmur64A;
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------

char const * StationIdFromAccountName::getAlgorithmName(Algorithm const algorithm)
{
	return algorithm == Gcc64Murmur64A ? "gcc64" : "gcc32";
}

// ----------------------------------------------------------------------

StationId StationIdFromAccountName::derive(std::string const & accountName, Algorithm const algorithm)
{
	// A numeric name was atoi(name) converted to StationId. glibc's atoi is
	// (int)strtol(name), and strtol saturates at the range of long, which is
	// 32 bits on a 32-bit build and 64 bits on a 64-bit build; so reproduce
	// the saturation of the build whose ids the chosen algorithm matches,
	// explicitly, instead of depending on this build's long.
	char const * const s = accountName.c_str();
	long long v = strtoll(s, nullptr, 10);
	if (algorithm == Gcc32Murmur2)
	{
		if (v > 2147483647LL)
			v = 2147483647LL;
		else if (v < -2147483647LL - 1)
			v = -2147483647LL - 1;
	}
	StationId suid = static_cast<StationId>(static_cast<uint64>(v));
	if (suid == 0)
	{
		// The old code hashed std::string(accountName.c_str()), so the input
		// stops at the first NUL.
		size_t const n = strlen(s);
		suid = (algorithm == Gcc64Murmur64A) ? gcc64StringHash(s, n) : gcc32StringHash(s, n);
	}
	return suid;
}

// ======================================================================
