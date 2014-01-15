//========================================================================
//
// ExchangeListCreditsMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_ExchangeListCreditsMessage_H
#define	_INCLUDED_ExchangeListCreditsMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Game Server
 * Sent to:  Central Server 
 * Action:  List credits on station exchange 
 */
class ExchangeListCreditsMessage : public GameNetworkMessage
{
public:
	ExchangeListCreditsMessage  (NetworkId const &actorId, uint32 credits, uint32 processId);
	ExchangeListCreditsMessage  (Archive::ReadIterator & source);
	~ExchangeListCreditsMessage ();

public:
	NetworkId const &  getActorId() const;
	uint32             getProcessId() const;
	uint32             getCredits() const;
	
private:
	Archive::AutoVariable<NetworkId>  m_actorId;
	Archive::AutoVariable<uint32>     m_credits;
	Archive::AutoVariable<uint32>     m_processId;

private:
	ExchangeListCreditsMessage(const ExchangeListCreditsMessage&);
	ExchangeListCreditsMessage& operator= (const ExchangeListCreditsMessage&);
};

// ======================================================================

inline NetworkId const &ExchangeListCreditsMessage::getActorId() const
{
	return m_actorId.get();
}

// ----------------------------------------------------------------------

inline uint32 ExchangeListCreditsMessage::getCredits() const
{
	return m_credits.get();
}

// ----------------------------------------------------------------------

inline uint32 ExchangeListCreditsMessage::getProcessId() const
{
	return m_processId.get();
}

// ======================================================================

#endif	// _INCLUDED_ExchangeListCreditsMessage_H
