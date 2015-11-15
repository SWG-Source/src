//========================================================================
//
// AttribModArchive.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef _INCLUDED_AttribModArchive_H
#define _INCLUDED_AttribModArchive_H

#include "Archive/AutoByteStream.h"

namespace AttribMod
{
struct AttribMod;
};


namespace Archive
{
	void put(Archive::ByteStream &target, const AttribMod::AttribMod &mod);
	void get(Archive::ReadIterator &source, AttribMod::AttribMod &mod);
}

#endif	// _INCLUDED_AttribModArchive_H

