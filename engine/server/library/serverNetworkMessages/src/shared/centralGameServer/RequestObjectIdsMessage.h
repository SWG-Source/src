//========================================================================
//
// RequestOIDsMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_RequestOIDsMessage_H
#define	_INCLUDED_RequestOIDsMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class RequestOIDsMessage : public GameNetworkMessage
{
public:
	RequestOIDsMessage  (uint32 serverId, uint32 howMany, bool logRequest);
	RequestOIDsMessage  (Archive::ReadIterator & source);
	~RequestOIDsMessage ();

	uint32              getHowMany(void) const;
	uint32              getServerId(void) const;
	bool                getLogRequest(void) const;

private:
	Archive::AutoVariable<uint32> m_serverId;
	Archive::AutoVariable<uint32> m_howMany;
	Archive::AutoVariable<bool> m_logRequest;

	RequestOIDsMessage();
	RequestOIDsMessage(const RequestOIDsMessage&);
	RequestOIDsMessage& operator= (const RequestOIDsMessage&);
};


//-----------------------------------------------------------------------

inline uint32 RequestOIDsMessage::getHowMany(void) const
{
	return m_howMany.get();
}

inline uint32 RequestOIDsMessage::getServerId(void) const
{
	return m_serverId.get();
}

inline bool RequestOIDsMessage::getLogRequest(void) const
{
	return m_logRequest.get();
}

#endif	// _INCLUDED_RequestOIDsMessage_H

