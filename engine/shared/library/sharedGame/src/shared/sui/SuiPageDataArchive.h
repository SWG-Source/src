//======================================================================
//
// SuiPageDataArchive.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SuiPageDataArchive_H
#define INCLUDED_SuiPageDataArchive_H

//======================================================================

class SuiPageData;

//----------------------------------------------------------------------

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, SuiPageData & target);
	void put(ByteStream & target, const SuiPageData & source);
}

//======================================================================

#endif
