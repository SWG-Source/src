// ======================================================================
//
// ValidateCharacterForLoginMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ValidateCharacterForLoginMessage_H
#define INCLUDED_ValidateCharacterForLoginMessage_H

// ======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Sent from:  Connection Server
 * Sent to:  Central, which forwards to DatabaseProcess
 * Action:  A player has requested to play a particular character.
 *          Check whether the character is valid for that account, and if it is figure
 *          out where it is located in the world.
 *          Respond with ValidateCharacterForLoginReplyMessage
 */
class ValidateCharacterForLoginMessage : public GameNetworkMessage
{
  public:
	ValidateCharacterForLoginMessage(StationId suid, const NetworkId &characterId);
	ValidateCharacterForLoginMessage(Archive::ReadIterator & source);

  public:
	StationId getSuid() const;
	const NetworkId & getCharacterId() const;
	
  private:
	Archive::AutoVariable<StationId> m_suid;
	Archive::AutoVariable<NetworkId> m_characterId;
};

// ======================================================================

inline StationId ValidateCharacterForLoginMessage::getSuid() const
{
	return m_suid.get();
}

// ----------------------------------------------------------------------

inline const NetworkId &ValidateCharacterForLoginMessage::getCharacterId() const
{
	return m_characterId.get();
}

// ======================================================================

#endif
 
