//======================================================================
//
// SuiPageDataServerArchive.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SuiPageDataServerArchive_H
#define INCLUDED_SuiPageDataServerArchive_H

//======================================================================

class SuiPageDataServer;

//----------------------------------------------------------------------

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, SuiPageDataServer & target);
	void put(ByteStream & target, const SuiPageDataServer & source);
}

//======================================================================

#endif
