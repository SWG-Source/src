//========================================================================
//
// RequestObjectMessage.h - tells Centralserver a Gameserver wants to create a
// proxy of an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_RequestObjectMessage_H
#define	_INCLUDED_RequestObjectMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class RequestObjectMessage : public GameNetworkMessage
{
public:
	RequestObjectMessage  (const NetworkId& id, uint32 process);
	RequestObjectMessage  (Archive::ReadIterator & source);
	~RequestObjectMessage ();

	const NetworkId&          getId(void) const;
	uint32              getProcess(void) const;

private:
	Archive::AutoVariable<NetworkId>    m_id;			// id of object we want to create
	Archive::AutoVariable<uint32> m_process;		// process requesting the object

	RequestObjectMessage();
	RequestObjectMessage(const RequestObjectMessage&);
	RequestObjectMessage& operator= (const RequestObjectMessage&);
};


//-----------------------------------------------------------------------

inline const NetworkId& RequestObjectMessage::getId(void) const
{
	return m_id.get();
}	// RequestObjectMessage::getId

inline uint32 RequestObjectMessage::getProcess(void) const
{
	return m_process.get();
}	// RequestObjectMessage::getProcess

#endif	// _INCLUDED_RequestObjectMessage_H
