// ======================================================================
//
// StomachRequestMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_StomachRequestMessage_H
#define INCLUDED_StomachRequestMessage_H

// ======================================================================

//#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Client
 * Sent to:    Connection Server
 * Action:     Requests the state of the player's stomach(s).
 */
class StomachRequestMessage : public GameNetworkMessage
{
 public:
	StomachRequestMessage();
	explicit StomachRequestMessage(Archive::ReadIterator & source);
	virtual ~StomachRequestMessage();

  private:
	StomachRequestMessage(const StomachRequestMessage&);
	StomachRequestMessage& operator= (const StomachRequestMessage&);
};

// ======================================================================

#endif	// _INCLUDED_ClientCentralMessages_H
