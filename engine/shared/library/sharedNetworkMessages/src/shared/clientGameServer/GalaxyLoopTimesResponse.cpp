// GalaxyLoopTimesResponse.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GalaxyLoopTimesResponse.h"

//-----------------------------------------------------------------------

GalaxyLoopTimesResponse::GalaxyLoopTimesResponse(const unsigned long l, const unsigned long c) :
GameNetworkMessage("GalaxyLoopTimesResponse"),
currentFrameMilliseconds(c),
lastFrameMilliseconds(l)
{
	addVariable(currentFrameMilliseconds);
	addVariable(lastFrameMilliseconds);
}

//-----------------------------------------------------------------------

GalaxyLoopTimesResponse::GalaxyLoopTimesResponse(Archive::ReadIterator & source) :
GameNetworkMessage("GalaxyLoopTimesResponse"),
currentFrameMilliseconds(),
lastFrameMilliseconds()
{
	addVariable(currentFrameMilliseconds);
	addVariable(lastFrameMilliseconds);
	unpack(source);
}

//-----------------------------------------------------------------------

GalaxyLoopTimesResponse::~GalaxyLoopTimesResponse()
{
}

//-----------------------------------------------------------------------

