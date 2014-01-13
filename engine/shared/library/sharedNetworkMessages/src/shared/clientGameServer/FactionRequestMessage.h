// ======================================================================
//
// FactionRequestMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FactionRequestMessage_H
#define INCLUDED_FactionRequestMessage_H

// ======================================================================

//#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Client
 * Sent to:    Connection Server
 * Action:     Requests the state of the player's stomach(s).
 */
class FactionRequestMessage : public GameNetworkMessage
{
 public:
	FactionRequestMessage();
	explicit FactionRequestMessage(Archive::ReadIterator & source);
	virtual ~FactionRequestMessage();

  private:
	FactionRequestMessage(const FactionRequestMessage&);
	FactionRequestMessage& operator= (const FactionRequestMessage&);
};

// ======================================================================

#endif	// _INCLUDED_ClientCentralMessages_H
