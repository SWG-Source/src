//======================================================================
//
// CustomerServiceSearchResultArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CustomerServiceSearchResultArchive.h"

#include "sharedNetworkMessages/CustomerServiceSearchResult.h"

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, CustomerServiceSearchResult & target)
	{
		get (source, target.m_title);	
		get (source, target.m_id);	
		get (source, target.m_matchPercent);	
	}

	void put (ByteStream & target, const CustomerServiceSearchResult & source)
	{
		put (target, source.m_title);
		put (target, source.m_id);
		put (target, source.m_matchPercent);
	}
}

//======================================================================
