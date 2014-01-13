//======================================================================
//
// CustomerServiceSearchResultArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CustomerServiceSearchResultArchive_H
#define INCLUDED_CustomerServiceSearchResultArchive_H

//======================================================================

#include "Archive/ByteStream.h"

class CustomerServiceSearchResult;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, CustomerServiceSearchResult & target);
	void put (ByteStream & target, const CustomerServiceSearchResult & source);
}

//======================================================================

#endif
