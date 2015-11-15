// ======================================================================
//
// CentralPingMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CentralPingMessage_H
#define INCLUDED_CentralPingMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Sent from:  Central / Game Server
 * Sent to:  Game Server / Central
 * Action:  Verfiy that we have two-way communications and that the game
 * server is responding.
 */
class CentralPingMessage : public GameNetworkMessage
{
  public:
	CentralPingMessage  ();
	CentralPingMessage  (Archive::ReadIterator & source);
	~CentralPingMessage ();

	uint32 getPingNumber() const;
	
  private:

	CentralPingMessage(const CentralPingMessage&);
	CentralPingMessage& operator= (const CentralPingMessage&);
};

// ======================================================================

#endif
