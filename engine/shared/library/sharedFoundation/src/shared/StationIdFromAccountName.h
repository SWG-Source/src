// ======================================================================
//
// StationIdFromAccountName.h
//
// Derives the StationId used for accounts that log in without an
// external authentication service.
//
// Historically this was std::hash<std::string>, whose result is
// implementation-defined: the standard only requires it to be stable
// within one execution, and in practice it differs between compilers,
// library versions and between 32-bit and 64-bit builds. Every account's
// characters are keyed by this value, so the rule is a property of the
// data: the login database records it (table station_id_scheme) and the
// LoginServer uses what the database records. The functions below are
// explicit, portable implementations of the rules GCC builds have used:
//
//   Gcc32Murmur2   - "gcc32": what every 32-bit GCC (libstdc++ >= 4.6)
//                    server computed; used by all new databases.
//   Gcc64Murmur64A - "gcc64": what 64-bit GCC builds computed (truncated to
//                    32 bits) before the rule was made explicit. Legacy:
//                    kept for databases created by those builds.
//
// ======================================================================

#ifndef INCLUDED_StationIdFromAccountName_H
#define INCLUDED_StationIdFromAccountName_H

// ======================================================================

#include "sharedFoundation/StationId.h"

#include <string>

// ======================================================================

namespace StationIdFromAccountName
{
	enum Algorithm
	{
		Gcc32Murmur2,
		Gcc64Murmur64A
	};

	// libstdc++ _Hash_bytes for a 32-bit size_t (MurmurHash2, seed 0xc70f6907).
	uint32 gcc32StringHash(char const * data, size_t length);

	// libstdc++ _Hash_bytes for a 64-bit size_t (MurmurHash64A variant,
	// seed 0xc70f6907), truncated to 32 bits as assigning it to a
	// StationId always did.
	uint32 gcc64StringHash(char const * data, size_t length);

	// Parses "gcc32" / "gcc64"; returns false for anything else.
	bool parseAlgorithm(char const * name, Algorithm & algorithm);
	char const * getAlgorithmName(Algorithm algorithm);

	// A numeric account name is its own StationId (as with atoi); any other
	// name is hashed with the chosen algorithm. accountName is the trimmed,
	// lower-cased account name.
	StationId derive(std::string const & accountName, Algorithm algorithm);
}

// ======================================================================

#endif
