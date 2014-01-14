//======================================================================
//
// AuctionManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/AuctionManager.h"

#include "UnicodeUtils.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include <vector>

//======================================================================

bool AuctionManager::separateLocation          (const std::string & location, std::string & planet, std::string & region, std::string & name, NetworkId & id, int & x, int & z)
{
	const char * const whitespace = ".";
	size_t endpos = 0;
	std::string token;

	typedef std::vector<std::string> StringVector;
	static StringVector sv;
	sv.clear ();

	std::string first;
	std::string vectorString;

	const size_t hashPos = location.rfind ('#');
	if (hashPos != std::string::npos)
	{
		first        = location.substr (0, hashPos);
		vectorString = location.substr (hashPos + 1);
	}

	while (endpos != Unicode::String::npos && Unicode::getFirstToken (first, endpos, endpos, token, whitespace))
	{
		sv.push_back (token);
		if (endpos != Unicode::String::npos)
			++endpos;
	}

	if (sv.size () != 4)
	{
		WARNING (true, ("AuctionManagerClient invalid location [%s]", location.c_str ()));
		return false;
	}

	planet = sv [0];
	region = sv [1];
	name   = sv [2];
	id = NetworkId (sv [3]);
	
	x = 0;
	z = 0;
	
	if (!vectorString.empty ())
	{
		const size_t commaPos = vectorString.rfind (',');
		if (commaPos != std::string::npos)
		{
			x = atoi (vectorString.c_str ());
			z = atoi (vectorString.c_str () + commaPos + 1);
		}
	}

	return true;
}

//----------------------------------------------------------------------

bool AuctionManager::separateLocation          (const std::string & location, std::string & planet, std::string & region, std::string & name)
{
	NetworkId id;
	int x = 0;
	int z = 0;
	return separateLocation (location, planet, region, name, id, x, z);
}

//======================================================================
