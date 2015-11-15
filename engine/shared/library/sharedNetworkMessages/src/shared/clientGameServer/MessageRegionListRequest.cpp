// MessageRegionListRequest.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "MessageRegionListRequest.h"

//-----------------------------------------------------------------------

MessageRegionListRequest::MessageRegionListRequest()
: GameNetworkMessage("MessageRegionListRequest")
{
}

//-----------------------------------------------------------------------

MessageRegionListRequest::MessageRegionListRequest(Archive::ReadIterator & source)
: GameNetworkMessage("MessageRegionListRequest")
{
	unpack(source);
}

//-----------------------------------------------------------------------

MessageRegionListRequest::~MessageRegionListRequest()
{
}

//-----------------------------------------------------------------------

