// ======================================================================
//
// ValidateAccountMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ValidateAccountMessage_H
#define INCLUDED_ValidateAccountMessage_H

// ======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Sent From:  Connection Server
 * Sent To:    Central
 * Action:     Do whatever validation Central can, then forward to LoginServer.
 *
 * Sent From:  Central
 * Sent To:    LoginServer
 * Action:     Check the permissions for the account (can create characters, etc.)
 *             Reply with ValidateAccountReplyMessage
 */

class ValidateAccountMessage : public GameNetworkMessage
{
public:
	ValidateAccountMessage (StationId stationId, unsigned int track, uint32 subscriptionBits);
	ValidateAccountMessage (Archive::ReadIterator & source);
	virtual ~ValidateAccountMessage ();
	
public:
	StationId     getStationId() const;
	unsigned int  getTrack() const;
	uint32        getSubscriptionBits() const;

private:
	Archive::AutoVariable<StationId>     m_stationId;
	Archive::AutoVariable<unsigned int>  m_track;
	Archive::AutoVariable<uint32>        m_subscriptionBits;

private: // disable:
	ValidateAccountMessage();
	ValidateAccountMessage(const ValidateAccountMessage&);
	ValidateAccountMessage& operator= (const ValidateAccountMessage&);

};

// ======================================================================

inline StationId ValidateAccountMessage::getStationId() const
{
	return m_stationId.get();
}

// ======================================================================

#endif
