// ======================================================================
//
// ServerDeleteCharacterMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerDeleteCharacterMessage_H
#define INCLUDED_ServerDeleteCharacterMessage_H

// ======================================================================

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Sent From:  Login Server
 * Sent To:    Central, which forwards to DBProcess
 * Action:     Verify that the specified character belongs to the specified
 *             account, then delete the character
 */

class ServerDeleteCharacterMessage : public GameNetworkMessage
{
  public:
    ServerDeleteCharacterMessage (StationId stationId, const NetworkId &characterId, uint32 loginServerId);
    ServerDeleteCharacterMessage (Archive::ReadIterator & source);
    virtual ~ServerDeleteCharacterMessage ();

  public:
	void                    setLoginServerId (uint32 loginServerId);
	StationId               getStationId     () const;
    const NetworkId &       getCharacterId   () const;
	uint32                  getLoginServerId () const;

  private:
    Archive::AutoVariable<StationId> m_stationId;
    Archive::AutoVariable<NetworkId> m_characterId;
	Archive::AutoVariable<uint32>    m_loginServerId;

  private: // disable:
    ServerDeleteCharacterMessage();
    ServerDeleteCharacterMessage(const ServerDeleteCharacterMessage&);
    ServerDeleteCharacterMessage& operator= (const ServerDeleteCharacterMessage&);

};

// ======================================================================

inline StationId ServerDeleteCharacterMessage::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & ServerDeleteCharacterMessage::getCharacterId() const
{
	return m_characterId.get();
}

// ----------------------------------------------------------------------

inline uint32 ServerDeleteCharacterMessage::getLoginServerId () const
{
	DEBUG_FATAL(m_loginServerId.get() == 0,("Programmer bug:  Login Server Id was not set in ServerDeleteCharacterMessage.  (It should be set when Central forwards the message.)\n"));
	return m_loginServerId.get();
}

// ======================================================================

#endif
