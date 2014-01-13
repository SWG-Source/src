//======================================================================
//
// PlayerMoneyResponse.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_PlayerMoneyResponse_H
#define INCLUDED_PlayerMoneyResponse_H

//======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class PlayerMoneyResponse : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	PlayerMoneyResponse  ();
	PlayerMoneyResponse  (Archive::ReadIterator &);
	~PlayerMoneyResponse ();

	Archive::AutoVariable<int>                  m_balanceCash;
	Archive::AutoVariable<int>                  m_balanceBank;

private:
	PlayerMoneyResponse & operator = (const PlayerMoneyResponse & rhs);
	PlayerMoneyResponse(const PlayerMoneyResponse & source);
};

//======================================================================

#endif
