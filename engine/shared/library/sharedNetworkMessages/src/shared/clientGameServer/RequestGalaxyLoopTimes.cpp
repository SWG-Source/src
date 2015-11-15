// RequestGalaxyLoopTimes.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/RequestGalaxyLoopTimes.h"

//-----------------------------------------------------------------------

RequestGalaxyLoopTimes::RequestGalaxyLoopTimes() :
GameNetworkMessage("RequestGalaxyLoopTimes")
{
}

//-----------------------------------------------------------------------

RequestGalaxyLoopTimes::RequestGalaxyLoopTimes(Archive::ReadIterator & source) :
GameNetworkMessage("RequestGalaxyLoopTimes")
{
	unpack(source);
}

//-----------------------------------------------------------------------

RequestGalaxyLoopTimes::~RequestGalaxyLoopTimes()
{
}

//-----------------------------------------------------------------------

