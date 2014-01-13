//======================================================================
//
// CustomerServiceCommentArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CustomerServiceCommentArchive_H
#define INCLUDED_CustomerServiceCommentArchive_H

//======================================================================

#include "Archive/ByteStream.h"

class CustomerServiceComment;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, CustomerServiceComment & target);
	void put (ByteStream & target, const CustomerServiceComment & source);
}

//======================================================================

#endif
