// ======================================================================
//
// LoginCreateCharacterAckMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LoginCreateCharacterAckMessage_H
#define INCLUDED_LoginCreateCharacterAckMessage_H

// ======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Sent From:  LoginServer
 * Sent To:    Central
 * Action:     Acknowledge that the character has been saved to the login
 *             server database.
 */

class LoginCreateCharacterAckMessage : public GameNetworkMessage
{
  public:
    LoginCreateCharacterAckMessage (const StationId &stationId, const NetworkId &characterNetworkId);
    LoginCreateCharacterAckMessage (Archive::ReadIterator & source);
    virtual ~LoginCreateCharacterAckMessage ();

  public:
	StationId           getStationId() const;
	const NetworkId &   getCharacterNetworkId() const;
	
  private:
    Archive::AutoVariable<StationId> m_stationId;
	Archive::AutoVariable<NetworkId> m_characterNetworkId;
  
  private: // disable:
    LoginCreateCharacterAckMessage();
    LoginCreateCharacterAckMessage(const LoginCreateCharacterAckMessage&);
    LoginCreateCharacterAckMessage& operator= (const LoginCreateCharacterAckMessage&);

};

// ======================================================================

inline StationId LoginCreateCharacterAckMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline const NetworkId &LoginCreateCharacterAckMessage::getCharacterNetworkId() const
{
	return m_characterNetworkId.get();
}

// ======================================================================

#endif
