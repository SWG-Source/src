//======================================================================
//
// SuiCommandArchive.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SuiCommandArchive.h"

#include "sharedGame/SuiCommand.h"

//======================================================================

namespace Archive
{
	void get(ReadIterator & source, SuiCommand & target)
	{
		target.get(source);
	}


	void put(ByteStream & target, const SuiCommand & source)
	{
		source.put(target);
	}
}

//======================================================================
