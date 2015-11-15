// ======================================================================
//
// PlanetLoadCharacterMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PlanetLoadCharacterMessage_H
#define INCLUDED_PlanetLoadCharacterMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Request that a character be loaded on a gameserver.
 *
 * Sent from:  PlanetServer 
 * Sent to:  DatabaseProcess
 * Action:  Load the specified character and send it to the specified
 * game server.
 */
class PlanetLoadCharacterMessage : public GameNetworkMessage
{
  public:
	PlanetLoadCharacterMessage  (const NetworkId &characterId, uint32 gameServerId);
	PlanetLoadCharacterMessage  (Archive::ReadIterator & source);
	~PlanetLoadCharacterMessage ();

	const NetworkId &getCharacterId() const;
	uint32 getGameServerId() const;

  private:
	Archive::AutoVariable<NetworkId> m_characterId;
	Archive::AutoVariable<uint32> m_gameServerId;

	PlanetLoadCharacterMessage();
	PlanetLoadCharacterMessage(const PlanetLoadCharacterMessage&);
	PlanetLoadCharacterMessage& operator= (const PlanetLoadCharacterMessage&);
};

// ----------------------------------------------------------------------

inline const NetworkId &PlanetLoadCharacterMessage::getCharacterId() const
{
	return m_characterId.get();
}

// ----------------------------------------------------------------------

inline uint32 PlanetLoadCharacterMessage::getGameServerId() const
{
	return m_gameServerId.get();
}

// ======================================================================

#endif
