// ======================================================================
//
// AutoDeltaPackedMap.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "FirstArchive.h"
#include "AutoDeltaPackedMap.h"

#include <stdio.h>

// ======================================================================

namespace Archive
{
	int countCharacter(const std::string &str, char c)
	{
		int result=0;
		for (std::string::const_iterator i=str.begin(); i!=str.end(); ++i)
		{
			if (*i==c)
				++result;
		}

		return result;
	}

	
} //namespace

// ======================================================================
