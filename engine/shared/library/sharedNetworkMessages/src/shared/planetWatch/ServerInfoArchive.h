// ServerInfoArchive.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ServerInfoArchive_H
#define	_INCLUDED_ServerInfoArchive_H

//-----------------------------------------------------------------------

struct ServerInfo;

//-----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, ServerInfo & target);
	void put (ByteStream & target, const ServerInfo & source);
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ServerInfoArchive_H
