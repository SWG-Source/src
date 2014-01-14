//======================================================================
//
// SuiPageDataArchive.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SuiPageDataArchive.h"

#include "sharedGame/SuiPageData.h"

//======================================================================

namespace Archive
{
	void get(ReadIterator & source, SuiPageData & target)
	{
		target.get(source);
	}


	void put(ByteStream & target, const SuiPageData & source)
	{
		source.put(target);
	}
}

//======================================================================
