//======================================================================
//
// SuiPageDataServerArchive.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/SuiPageDataServerArchive.h"

#include "serverGame/SuiPageDataServer.h"

//======================================================================

namespace Archive
{
	void get(ReadIterator & source, SuiPageDataServer & target)
	{
		target.get(source);
	}


	void put(ByteStream & target, const SuiPageDataServer & source)
	{
		source.put(target);
	}
}

//======================================================================
