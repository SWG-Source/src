//======================================================================
//
// SuiCommandArchive.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SuiCommandArchive_H
#define INCLUDED_SuiCommandArchive_H

//======================================================================

class SuiCommand;

//----------------------------------------------------------------------

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, SuiCommand & target);
	void put(ByteStream & target, const SuiCommand & source);
}

//======================================================================

#endif
