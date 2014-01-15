// ScriptParamArchive.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ScriptParamArchive_H
#define	_INCLUDED_ScriptParamArchive_H

//-----------------------------------------------------------------------

struct Param;

//-----------------------------------------------------------------------

namespace Archive 
{
	class ReadIterator;
	class ByteStream;

	void get(Archive::ReadIterator & source, Param & target);
	void put(Archive::ByteStream & target, const Param & source);
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ScriptParamArchive_H
