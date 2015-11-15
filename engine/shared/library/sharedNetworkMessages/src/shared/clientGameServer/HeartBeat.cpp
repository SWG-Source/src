// HeartBeat.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/HeartBeat.h"

//-----------------------------------------------------------------------

HeartBeat::HeartBeat() :
GameNetworkMessage("HeartBeat")
{
}

//-----------------------------------------------------------------------

HeartBeat::HeartBeat(Archive::ReadIterator & source) :
GameNetworkMessage("HeartBeat")
{
	unpack(source);
}

//-----------------------------------------------------------------------

HeartBeat::~HeartBeat()
{
}

//-----------------------------------------------------------------------

