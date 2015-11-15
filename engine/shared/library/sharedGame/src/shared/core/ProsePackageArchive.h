//======================================================================
//
// ProsePackageArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ProsePackageArchive_H
#define INCLUDED_ProsePackageArchive_H

//======================================================================

class ProsePackage;

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get (Archive::ReadIterator & source, ProsePackage & target);
	void put (ByteStream & target, const ProsePackage & source);
};

//======================================================================

#endif
