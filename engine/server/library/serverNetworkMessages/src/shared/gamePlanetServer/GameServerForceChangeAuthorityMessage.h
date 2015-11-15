//========================================================================
//
// GameServerForceChangeAuthorityMessage.h - tells the PlanetServer a game server has
// forced an object to change it's authoritative server.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_GameServerForceChangeAuthorityMessage_H
#define	_INCLUDED_GameServerForceChangeAuthorityMessage_H

//-----------------------------------------------------------------------

#include <vector>
#include "sharedMath/Transform.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Game Server
 * Sent to:    Planet Server
 * Action:     Tells the PlanetServer a game server has forced an object to change 
 *             it's authoritative server.
 */
class GameServerForceChangeAuthorityMessage : public GameNetworkMessage
{
public:
	typedef std::vector<uint32> ProcessList;

	GameServerForceChangeAuthorityMessage  (const NetworkId& id, uint32 fromProcess, uint32 toProcess);
	GameServerForceChangeAuthorityMessage  (Archive::ReadIterator & source);
	~GameServerForceChangeAuthorityMessage ();

	const NetworkId&            getId(void) const;
	uint32                      getFromProcess(void) const;
	uint32                      getToProcess(void) const;
	
private:
	Archive::AutoVariable<NetworkId>  m_id;			    // id of object being made authoritative
	Archive::AutoVariable<uint32>     m_fromProcess;	// process that the object was authoritative on
	Archive::AutoVariable<uint32>     m_toProcess;      // process that the object is being made authoritative on

	GameServerForceChangeAuthorityMessage();
	GameServerForceChangeAuthorityMessage(const GameServerForceChangeAuthorityMessage&);
	GameServerForceChangeAuthorityMessage& operator= (const GameServerForceChangeAuthorityMessage&);
};


//-----------------------------------------------------------------------

inline const NetworkId& GameServerForceChangeAuthorityMessage::getId(void) const
{
	return m_id.get();
}

inline uint32 GameServerForceChangeAuthorityMessage::getFromProcess(void) const
{
	return m_fromProcess.get();
}

inline uint32 GameServerForceChangeAuthorityMessage::getToProcess(void) const
{
	return m_toProcess.get();
}

#endif	// _INCLUDED_GameServerForceChangeAuthorityMessage_H
