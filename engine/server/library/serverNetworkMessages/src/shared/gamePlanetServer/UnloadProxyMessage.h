// ======================================================================
//
// UnloadProxyMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UnloadProxyMessage_H
#define INCLUDED_UnloadProxyMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/** 
 * Tells the authoritative game server to remove a proxy for an object
 *
 * Sent from:  Planet Server 
 * Sent to:  Game Server
 * Action:  Remove the specified proxy
 */
class UnloadProxyMessage : public GameNetworkMessage
{
  public:
	UnloadProxyMessage  (NetworkId objectId, uint32 proxyGameServerId);
	UnloadProxyMessage  (Archive::ReadIterator & source);
	~UnloadProxyMessage ();

	NetworkId getObjectId() const;
	uint32 getProxyGameServerId() const;

  private:
	Archive::AutoVariable<NetworkId> m_objectId;
	Archive::AutoVariable<uint32> m_proxyGameServerId;

	UnloadProxyMessage();
	UnloadProxyMessage(const UnloadProxyMessage&);
	UnloadProxyMessage& operator= (const UnloadProxyMessage&);
};

// ----------------------------------------------------------------------

inline NetworkId UnloadProxyMessage::getObjectId() const
{
	return m_objectId.get();
}

// ----------------------------------------------------------------------

inline uint32 UnloadProxyMessage::getProxyGameServerId() const
{
	return m_proxyGameServerId.get();
}

// ======================================================================

#endif
