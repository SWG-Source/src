//======================================================================
//
// PlayerMoneyResponse.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/PlayerMoneyResponse.h"

//-----------------------------------------------------------------------

const char * const PlayerMoneyResponse::MessageType = "PlayerMoneyResponse";

//----------------------------------------------------------------------

PlayerMoneyResponse::PlayerMoneyResponse() :
GameNetworkMessage   (MessageType),
m_balanceCash        (0),
m_balanceBank        (0)
{
	addVariable (m_balanceCash);
	addVariable (m_balanceBank);
}

//-----------------------------------------------------------------------

PlayerMoneyResponse::PlayerMoneyResponse(Archive::ReadIterator & source) :
GameNetworkMessage   (MessageType),
m_balanceCash        (0),
m_balanceBank        (0)
{
	addVariable (m_balanceCash);
	addVariable (m_balanceBank);

	unpack(source);
}

//-----------------------------------------------------------------------

PlayerMoneyResponse::~PlayerMoneyResponse()
{
}

//======================================================================
