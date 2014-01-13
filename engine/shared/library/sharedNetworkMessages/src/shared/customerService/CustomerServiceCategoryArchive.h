//======================================================================
//
// CustomerServiceCategoryArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CustomerServiceCategoryArchive_H
#define INCLUDED_CustomerServiceCategoryArchive_H

//======================================================================

#include "Archive/ByteStream.h"

class CustomerServiceCategory;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, CustomerServiceCategory & target);
	void put (ByteStream & target, const CustomerServiceCategory & source);
}

//======================================================================

#endif
