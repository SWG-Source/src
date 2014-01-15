//========================================================================
//
// RequestBiographyMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_RequestBiographyMessage_H
#define	_INCLUDED_RequestBiographyMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Game Server
 * Sent to:  DBProcess
 * Action:  Retrieve the biography from the database and respond with
 *          BiographyMessage
 */
class RequestBiographyMessage : public GameNetworkMessage
{
  public:
	RequestBiographyMessage  (const NetworkId &owner);
	RequestBiographyMessage  (Archive::ReadIterator & source);
	~RequestBiographyMessage ();

	const NetworkId &getOwner() const;
	
  private:
	Archive::AutoVariable<NetworkId> m_owner;

  private:
	RequestBiographyMessage(const RequestBiographyMessage&);
	RequestBiographyMessage& operator= (const RequestBiographyMessage&);
};

// ======================================================================

inline const NetworkId &RequestBiographyMessage::getOwner() const
{
	return m_owner.get();
}

// ======================================================================

#endif	// _INCLUDED_RequestBiographyMessage_H
