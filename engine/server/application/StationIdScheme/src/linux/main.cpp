// ======================================================================
//
// main.cpp (stationIdScheme)
//
// Labels a login database with the rule its station ids were derived
// with (table station_id_scheme). The LoginServer will not start until the
// label exists. See game/server/database/tools/station_id_scheme/README.md.
//
//   stationIdScheme classify               < ids.tsv > label.sql
//   stationIdScheme assert gcc32|gcc64     < ids.tsv > label.sql
//
// ids.tsv is the combined output of export.sql from every schema that holds
// login or cluster tables. A report goes to stderr; the SQL that records
// the label goes to stdout only when a label may be recorded.
//
// Each witness (the account name the game recorded for one of an id's
// characters) is judged on its own: it reproduces its id under gcc32 only,
// under gcc64 only, under both (numeric names), or under neither. Only an
// exclusive witness is evidence for a scheme, and a witness that fits both
// can never outweigh one that fits only one.
//
// classify records a label only from complete evidence: every station id
// has an exclusive witness for that scheme, no witness is exclusive to the
// other scheme, and no witness is unexplained. assert records the
// operator's statement as such, and is refused if any witness is exclusive
// to the other scheme.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/StationIdFromAccountName.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <string>

// ======================================================================

namespace
{
	using StationIdFromAccountName::Algorithm;
	using StationIdFromAccountName::Gcc32Murmur2;
	using StationIdFromAccountName::Gcc64Murmur64A;

	struct Id
	{
		std::set<std::string> tables;
		std::set<std::string> witnesses;
		bool proven32 = false; // some witness reproduces the id under gcc32 only
		bool proven64 = false; // some witness reproduces the id under gcc64 only
	};

	int usage()
	{
		fprintf(stderr, "usage: stationIdScheme classify < ids.tsv > label.sql\n"
		                "       stationIdScheme assert gcc32|gcc64 < ids.tsv > label.sql\n");
		return 2;
	}

	// A station id is stored as the signed 32-bit value with the same bits;
	// accept that form or the unsigned one.
	bool parseStationId(std::string const & text, uint32 & id)
	{
		char * end = nullptr;
		errno = 0;
		long long const v = strtoll(text.c_str(), &end, 10);
		if (errno == ERANGE || end == text.c_str() || *end != '\0' || v < -2147483648LL || v > 4294967295LL)
			return false;
		id = static_cast<uint32>(static_cast<uint64>(v));
		return true;
	}

	void printLabelSql(char const * scheme, char const * basis)
	{
		printf("whenever sqlerror exit failure rollback\n"
		       "insert into station_id_scheme (id, scheme, basis) values (1, '%s', '%s');\n"
		       "commit;\n"
		       "exit\n", scheme, basis);
	}
}

// ======================================================================

int main(int argc, char ** argv)
{
	if (argc < 2)
		return usage();
	std::string const command = argv[1];
	Algorithm asserted = Gcc32Murmur2;
	if (command == "assert")
	{
		if (argc != 3 || !StationIdFromAccountName::parseAlgorithm(argv[2], asserted))
			return usage();
	}
	else if (command != "classify" || argc != 2)
		return usage();

	std::map<uint32, Id> ids;
	std::set<std::string> unexplained; // witness names matching neither scheme
	int badLines = 0;

	std::string line;
	int lineNumber = 0;
	while (std::getline(std::cin, line))
	{
		++lineNumber;
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty())
			continue;
		size_t const tab1 = line.find('\t');
		size_t const tab2 = (tab1 == std::string::npos) ? std::string::npos : line.find('\t', tab1 + 1);
		uint32 id = 0;
		if (tab2 == std::string::npos || !parseStationId(line.substr(0, tab1), id))
		{
			fprintf(stderr, "line %d is not <station id> TAB <name> TAB <table>: %s\n", lineNumber, line.c_str());
			++badLines;
			continue;
		}
		Id & entry = ids[id];
		entry.tables.insert(line.substr(tab2 + 1));
		std::string name = line.substr(tab1 + 1, tab2 - tab1 - 1);
		if (name.empty())
			continue;
		// The LoginServer derives ids from the trimmed, lower-cased name.
		std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return static_cast<char>(tolower(c)); });
		entry.witnesses.insert(name);
		bool const is32 = StationIdFromAccountName::derive(name, Gcc32Murmur2) == id;
		bool const is64 = StationIdFromAccountName::derive(name, Gcc64Murmur64A) == id;
		if (is32 && !is64)
			entry.proven32 = true;
		if (is64 && !is32)
			entry.proven64 = true;
		if (!is32 && !is64)
			unexplained.insert(name + " (" + std::to_string(id) + ")");
	}

	if (badLines > 0)
	{
		fprintf(stderr, "%d unreadable lines; nothing recorded\n", badLines);
		return 1;
	}

	int unwitnessed = 0, proven32 = 0, proven64 = 0, contradicted = 0, unproven = 0;
	for (auto const & i : ids)
	{
		Id const & e = i.second;
		if (e.witnesses.empty())
			++unwitnessed;
		else if (e.proven32 && e.proven64)
			++contradicted;
		else if (e.proven32)
			++proven32;
		else if (e.proven64)
			++proven64;
		else
			++unproven; // witnessed only by names that fit both schemes
	}
	bool const any32 = (proven32 + contradicted) > 0;
	bool const any64 = (proven64 + contradicted) > 0;

	fprintf(stderr, "station ids: %zu\n", ids.size());
	fprintf(stderr, "  proven gcc32 (a witness fits gcc32 only): %d\n", proven32);
	fprintf(stderr, "  proven gcc64 (a witness fits gcc64 only): %d\n", proven64);
	fprintf(stderr, "  proven both ways (witnesses disagree): %d\n", contradicted);
	fprintf(stderr, "  witnessed only by names that fit both (numeric): %d\n", unproven);
	fprintf(stderr, "  without a witness: %d\n", unwitnessed);
	fprintf(stderr, "witness names reproducing neither: %zu\n", unexplained.size());
	for (auto const & u : unexplained)
		fprintf(stderr, "  %s\n", u.c_str());
	for (auto const & i : ids)
		if (i.second.witnesses.empty())
		{
			std::string tables;
			for (auto const & t : i.second.tables)
				tables += (tables.empty() ? "" : ", ") + t;
			fprintf(stderr, "  no witness: %u (signed %d) in %s\n", i.first, static_cast<int32>(i.first), tables.c_str());
		}

	if (command == "classify")
	{
		if (ids.empty())
		{
			fprintf(stderr, "no station ids: recording gcc32 (new). If this database has accounts, the export came from the wrong schema; do not apply label.sql.\n");
			printLabelSql("gcc32", "new");
			return 0;
		}
		if (any32 && any64)
		{
			fprintf(stderr, "the evidence is contradictory: both schemes are proven, so the accounts were created under both rules; nothing recorded. One set of accounts must be re-keyed before the database can be labelled.\n");
			return 1;
		}
		bool const complete = unwitnessed == 0 && unproven == 0 && unexplained.empty();
		if (complete && any32)
		{
			fprintf(stderr, "complete evidence for gcc32\n");
			printLabelSql("gcc32", "evidence");
			return 0;
		}
		if (complete && any64)
		{
			fprintf(stderr, "complete evidence for gcc64\n");
			printLabelSql("gcc64", "evidence");
			return 0;
		}
		fprintf(stderr, "the evidence is incomplete; nothing recorded. If you know which rule created this database's accounts, use 'assert'.\n");
		return 1;
	}

	// assert
	char const * const scheme = StationIdFromAccountName::getAlgorithmName(asserted);
	bool const provenOther = (asserted == Gcc32Murmur2) ? any64 : any32;
	if (provenOther)
	{
		fprintf(stderr, "refused: %d station ids are proven under the other scheme; %s cannot be asserted\n", ((asserted == Gcc32Murmur2) ? proven64 : proven32) + contradicted, scheme);
		return 1;
	}
	fprintf(stderr, "recording %s as asserted by the operator\n", scheme);
	printLabelSql(scheme, "asserted");
	return 0;
}

// ======================================================================
