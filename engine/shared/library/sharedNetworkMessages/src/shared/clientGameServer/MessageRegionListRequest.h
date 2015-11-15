// MessageRegionListRequest.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_MessageRegionListRequest_H
#define	_INCLUDED_MessageRegionListRequest_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class MessageRegionListRequest : public GameNetworkMessage
{
public:
	MessageRegionListRequest();
	MessageRegionListRequest(Archive::ReadIterator & source);
	~MessageRegionListRequest();

private:
	MessageRegionListRequest & operator = (const MessageRegionListRequest & rhs);
	MessageRegionListRequest(const MessageRegionListRequest & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MessageRegionListRequest_H
