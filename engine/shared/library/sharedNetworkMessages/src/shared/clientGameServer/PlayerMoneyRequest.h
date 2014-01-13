//======================================================================
//
// PlayerMoneyRequest.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_PlayerMoneyRequest_H
#define INCLUDED_PlayerMoneyRequest_H

//======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class PlayerMoneyRequest : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	PlayerMoneyRequest  ();
	PlayerMoneyRequest  (Archive::ReadIterator &);
	~PlayerMoneyRequest ();

private:
	PlayerMoneyRequest & operator = (const PlayerMoneyRequest & rhs);
	PlayerMoneyRequest (const PlayerMoneyRequest & source);
};

//======================================================================

#endif
