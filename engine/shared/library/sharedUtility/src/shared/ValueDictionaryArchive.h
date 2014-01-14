// ======================================================================
//
// ValueDictionaryArchive.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ValueDictionaryArchive_H
#define INCLUDED_ValueDictionaryArchive_H

//======================================================================

class ValueDictionary;

//----------------------------------------------------------------------

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get (ReadIterator & source, ValueDictionary & target);
	void put (ByteStream & target, const ValueDictionary & source);
}

//======================================================================
#endif
