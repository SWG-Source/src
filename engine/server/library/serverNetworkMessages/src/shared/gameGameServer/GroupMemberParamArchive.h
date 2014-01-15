//======================================================================
//
// GroupMemberParamArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_GroupMemberParamArchive_H
#define INCLUDED_GroupMemberParamArchive_H

//======================================================================

class GroupMemberParam;

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get (Archive::ReadIterator & source, GroupMemberParam & target);
	void put (ByteStream & target, const GroupMemberParam & source);
};

//======================================================================

#endif
