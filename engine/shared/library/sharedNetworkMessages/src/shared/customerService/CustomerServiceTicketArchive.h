//======================================================================
//
// CustomerServiceTicketArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CustomerServiceTicketArchive_H
#define INCLUDED_CustomerServiceTicketArchive_H

//======================================================================

#include "Archive/ByteStream.h"

class CustomerServiceTicket;

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, CustomerServiceTicket & target);
	void put (ByteStream & target, const CustomerServiceTicket & source);
}

//======================================================================

#endif
