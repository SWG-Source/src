// ======================================================================
//
// TransferCharacterDataArchive.h
// copyright (c) 2001-2004 Sony Online Entertainment
//
// ======================================================================

#ifndef _TransferCharacterDataArchive_H
#define _TransferCharacterDataArchive_H

//-----------------------------------------------------------------------

class TransferCharacterData;

namespace Archive
{
	class ByteStream;
	class ReadIterator;
}

//-----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, TransferCharacterData & target);
	void put (ByteStream & target, const TransferCharacterData & source);
}

//-----------------------------------------------------------------------

#endif//_TransferCharacterDataArchive_H
