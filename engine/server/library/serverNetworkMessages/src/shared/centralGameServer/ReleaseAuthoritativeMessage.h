//========================================================================
//
// ReleaseAuthoritativeMessage.h - tells Centralserver an authoritative object
// wants to release it's authority to a proxy.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_ReleaseAuthoritativeMessage_H
#define	_INCLUDED_ReleaseAuthoritativeMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ReleaseAuthoritativeMessage : public GameNetworkMessage
{
public:
	explicit ReleaseAuthoritativeMessage  (const NetworkId& id, uint32 process = 0);
	ReleaseAuthoritativeMessage           (Archive::ReadIterator & source);
	~ReleaseAuthoritativeMessage ();

	const NetworkId&          getId(void) const;
	uint32              getProcess(void) const;

private:
	Archive::AutoVariable<NetworkId>    m_id;			// id of object that is releasing it's authority
	Archive::AutoVariable<uint32> m_process;		// process we would like to release the authority to (-1 = any)

	ReleaseAuthoritativeMessage();
	ReleaseAuthoritativeMessage(const ReleaseAuthoritativeMessage&);
	ReleaseAuthoritativeMessage& operator= (const ReleaseAuthoritativeMessage&);
};


//-----------------------------------------------------------------------

inline const NetworkId& ReleaseAuthoritativeMessage::getId(void) const
{
	return m_id.get();
}	// ReleaseAuthoritativeMessage::getId

inline uint32 ReleaseAuthoritativeMessage::getProcess(void) const
{
	return m_process.get();
}	// ReleaseAuthoritativeMessage::getProcess


#endif	// _INCLUDED_ReleaseAuthoritativeMessage_H
