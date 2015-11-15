// ======================================================================
//
// TransferAccountDataArchive.h
// copyright (c) 2001-2004 Sony Online Entertainment
//
// ======================================================================

#ifndef _TransferAccountDataArchive_H
#define _TransferAccountDataArchive_H

//-----------------------------------------------------------------------

class TransferAccountData;

namespace Archive
{
	class ByteStream;
	class ReadIterator;
}

//-----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, TransferAccountData & target);
	void put (ByteStream & target, const TransferAccountData & source);
}

//-----------------------------------------------------------------------

#endif//_TransferAccountDataArchive_H
