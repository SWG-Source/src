//======================================================================
//
// SkillObjectArchive.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SkillObjectArchive_H
#define INCLUDED_SkillObjectArchive_H

//======================================================================
 
#if WIN32
#pragma warning (disable:4800) // forcing value to bool
#endif

#include "Archive/ByteStream.h"

class SkillObject;

namespace Archive
{
	void get(ReadIterator & source, const SkillObject *& target);
	void put(ByteStream & target, const SkillObject * const& source);
}

//======================================================================

#endif
