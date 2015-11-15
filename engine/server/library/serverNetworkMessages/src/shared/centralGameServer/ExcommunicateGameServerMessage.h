// ======================================================================
//
// ExcommunicateGameServerMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ExcommunicateGameServerMessage_H
#define INCLUDED_ExcommunicateGameServerMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Sent from:  Central 
 * Sent to:  everybody
 * Action:  Drop connection to the specified game server, doing the same
 * things that would be done normally if the game server closed connection
 * (e.g. drop proxies, start recovery on the planet server, etc.)
 */
class ExcommunicateGameServerMessage : public GameNetworkMessage
{
public:
	ExcommunicateGameServerMessage  (uint32 serverId, uint32 pid, const std::string & hostName);
	explicit ExcommunicateGameServerMessage  (Archive::ReadIterator & source);
	~ExcommunicateGameServerMessage ();

	uint32               getServerId   () const;
	uint32               getProcessId  () const;
	const std::string &  getHostName   () const;

private:
	Archive::AutoVariable<uint32>         m_serverId;
	Archive::AutoVariable<uint32>         m_processId;	
	Archive::AutoVariable<std::string>    m_hostName;

	ExcommunicateGameServerMessage();
	ExcommunicateGameServerMessage(const ExcommunicateGameServerMessage&);
	ExcommunicateGameServerMessage& operator= (const ExcommunicateGameServerMessage&);
};

// ----------------------------------------------------------------------

inline uint32 ExcommunicateGameServerMessage::getServerId() const
{
	return m_serverId.get();
}

//-----------------------------------------------------------------------

inline uint32 ExcommunicateGameServerMessage::getProcessId() const
{
	return m_processId.get();
}

//-----------------------------------------------------------------------

inline const std::string & ExcommunicateGameServerMessage::getHostName() const
{
	return m_hostName.get();
}

// ======================================================================

#endif
