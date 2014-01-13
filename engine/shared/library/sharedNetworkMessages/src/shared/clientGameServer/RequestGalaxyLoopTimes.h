// RequestGalaxyLoopTimes.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_RequestGalaxyLoopTimes_H
#define	_INCLUDED_RequestGalaxyLoopTimes_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class RequestGalaxyLoopTimes : public GameNetworkMessage
{
public:
	          RequestGalaxyLoopTimes   ();
	explicit  RequestGalaxyLoopTimes   (Archive::ReadIterator & source);
	          ~RequestGalaxyLoopTimes  ();

private:
	RequestGalaxyLoopTimes & operator = (const RequestGalaxyLoopTimes & rhs);
	RequestGalaxyLoopTimes(const RequestGalaxyLoopTimes & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_RequestGalaxyLoopTimes_H
