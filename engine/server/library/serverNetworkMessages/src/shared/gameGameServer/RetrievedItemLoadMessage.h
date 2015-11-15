//========================================================================
//
// RetrievedItemLoadMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_RetrievedItemLoadMessage_H
#define	_INCLUDED_RetrievedItemLoadMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  DBProcess
 * Sent to:  GameServer
 * Action:  Notifies game servers that an auction item has been retrieved and may be loaded
 */
class RetrievedItemLoadMessage : public GameNetworkMessage
{
public:
	RetrievedItemLoadMessage  (NetworkId const &ownerId, NetworkId const &itemId);
	RetrievedItemLoadMessage  (Archive::ReadIterator & source);
	~RetrievedItemLoadMessage ();

public:
	NetworkId const &    getOwnerId() const;
	NetworkId const &    getItemId() const;
	
private:
	Archive::AutoVariable<NetworkId>    m_ownerId;
	Archive::AutoVariable<NetworkId>    m_itemId;

private:
	RetrievedItemLoadMessage(const RetrievedItemLoadMessage&);
	RetrievedItemLoadMessage& operator= (const RetrievedItemLoadMessage&);
};

// ======================================================================

inline NetworkId const &RetrievedItemLoadMessage::getOwnerId() const
{
	return m_ownerId.get();
}

// ----------------------------------------------------------------------

inline NetworkId const &RetrievedItemLoadMessage::getItemId() const
{
	return m_itemId.get();
}

// ======================================================================

#endif	// _INCLUDED_RetrievedItemLoadMessage_H
