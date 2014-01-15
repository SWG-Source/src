//========================================================================
//
// AddOIDBlockMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_AddOIDBlockMessage_H
#define	_INCLUDED_AddOIDBlockMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class AddOIDBlockMessage : public GameNetworkMessage
{
public:
	AddOIDBlockMessage  (uint32 serverId, const NetworkId &start, const NetworkId &end, bool logRequest);
	AddOIDBlockMessage  (Archive::ReadIterator & source);
	~AddOIDBlockMessage ();

	uint32            getServerId(void) const;
	const NetworkId  &getStart(void) const;
	const NetworkId  &getEnd(void) const;
	bool              getLogRequest(void) const;

private:
	Archive::AutoVariable<uint32> m_serverId;
	Archive::AutoVariable<NetworkId> m_start;
	Archive::AutoVariable<NetworkId> m_end;
	Archive::AutoVariable<bool> m_logRequest;
	
	AddOIDBlockMessage();
	AddOIDBlockMessage(const AddOIDBlockMessage&);
	AddOIDBlockMessage& operator= (const AddOIDBlockMessage&);
};

//-----------------------------------------------------------------------

inline uint32 AddOIDBlockMessage::getServerId(void) const
{
	return m_serverId.get();
}

//-----------------------------------------------------------------------

inline const NetworkId &AddOIDBlockMessage::getStart(void) const
{
	return m_start.get();
}

//-----------------------------------------------------------------------

inline const NetworkId &AddOIDBlockMessage::getEnd(void) const
{
	return m_end.get();
}

//-----------------------------------------------------------------------

inline bool AddOIDBlockMessage::getLogRequest(void) const
{
	return m_logRequest.get();
}

// ======================================================================

#endif	// _INCLUDED_AddOIDBlockMessage_H

