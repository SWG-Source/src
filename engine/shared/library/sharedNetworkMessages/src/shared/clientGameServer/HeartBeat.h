// HeartBeat.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_HeartBeat_H
#define	_INCLUDED_HeartBeat_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h" 

//-----------------------------------------------------------------------

class HeartBeat : public GameNetworkMessage
{
public:
	HeartBeat();
	explicit HeartBeat(Archive::ReadIterator & source);
	~HeartBeat();

private:
	HeartBeat & operator = (const HeartBeat & rhs);
	HeartBeat(const HeartBeat & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_HeartBeat_H
