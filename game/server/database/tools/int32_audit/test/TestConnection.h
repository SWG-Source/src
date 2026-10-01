// ======================================================================
//
// TestConnection.h
//
// The scratch schema the 32-bit binding tests connect to. It comes only
// from the environment, never from a built-in default, so a test cannot
// reach a live schema by accident:
//
//   SWG_INT32_TEST_DSN       e.g. //dbhost/service
//   SWG_INT32_TEST_USER      the scratch schema's owner
//   SWG_INT32_TEST_PASSWORD
//
// The tests write only inside a transaction they roll back, plus scratch
// objects named int32test_*, but they still belong on a scratch schema.
//
// ======================================================================

#ifndef INCLUDED_Int32TestConnection_H
#define INCLUDED_Int32TestConnection_H

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace Int32TestConnection
{
	struct Parameters
	{
		std::string dsn;
		std::string user;
		std::string password;

		// The owner of the scratch schema's types (VAOFNUMBER, ...).
		std::string schemaOwner() const
		{
			std::string owner(user);
			for (std::string::iterator i = owner.begin(); i != owner.end(); ++i)
				*i = static_cast<char>(toupper(static_cast<unsigned char>(*i)));
			return owner;
		}
	};

	inline Parameters fromEnvironment()
	{
		char const * const dsn = getenv("SWG_INT32_TEST_DSN");
		char const * const user = getenv("SWG_INT32_TEST_USER");
		char const * const password = getenv("SWG_INT32_TEST_PASSWORD");
		if (!dsn || !*dsn || !user || !*user || !password)
		{
			fprintf(stderr, "Set SWG_INT32_TEST_DSN, SWG_INT32_TEST_USER and SWG_INT32_TEST_PASSWORD to a scratch schema (see README.md).\n");
			exit(2);
		}
		Parameters p;
		p.dsn = dsn;
		p.user = user;
		p.password = password;
		return p;
	}
}

#endif
