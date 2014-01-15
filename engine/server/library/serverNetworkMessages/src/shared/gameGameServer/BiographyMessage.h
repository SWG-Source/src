//========================================================================
//
// BiographyMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_BiographyMessage_H
#define	_INCLUDED_BiographyMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Game Server
 * Sent to:  DBProcess
 * Action:  Sets the biography for a player
 *
 * Sent from:  DBProcess
 * Sent to:  Game Server
 * Action:  Gives the game server the biography for a player
 */
class BiographyMessage : public GameNetworkMessage
{
  public:
	BiographyMessage  (const NetworkId &owner, const Unicode::String &bio);
	BiographyMessage  (Archive::ReadIterator & source);
	~BiographyMessage ();

  public:
	const NetworkId &getOwner() const;
	const Unicode::String &getBio() const;
	
  private:
	Archive::AutoVariable<NetworkId> m_owner;
	Archive::AutoVariable<Unicode::String> m_bio;

  private:
	BiographyMessage(const BiographyMessage&);
	BiographyMessage& operator= (const BiographyMessage&);
};

// ======================================================================

inline const NetworkId &BiographyMessage::getOwner() const
{
	return m_owner.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String &BiographyMessage::getBio() const
{
	return m_bio.get();
}

// ======================================================================

#endif	// _INCLUDED_BiographyMessage_H
