//======================================================================
//
// PlayerMoneyRequest.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/PlayerMoneyRequest.h"

//-----------------------------------------------------------------------

const char * const PlayerMoneyRequest::MessageType = "PlayerMoneyRequest";

//----------------------------------------------------------------------

PlayerMoneyRequest::PlayerMoneyRequest() :
GameNetworkMessage (MessageType)
{
}

//-----------------------------------------------------------------------

PlayerMoneyRequest::PlayerMoneyRequest(Archive::ReadIterator & source) :
GameNetworkMessage (MessageType)
{
	unpack(source);
}

//-----------------------------------------------------------------------

PlayerMoneyRequest::~PlayerMoneyRequest()
{
}

//======================================================================
