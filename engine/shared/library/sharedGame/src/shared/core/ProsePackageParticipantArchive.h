//======================================================================
//
// ProsePackageParticipantArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ProsePackageParticipantArchive_H
#define INCLUDED_ProsePackageParticipantArchive_H

//======================================================================

class ProsePackageParticipant;

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get (Archive::ReadIterator & source, ProsePackageParticipant & target);
	void put (ByteStream & target, const ProsePackageParticipant & source);
};

//======================================================================

#endif
