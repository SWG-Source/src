//========================================================================
//
// FailedToLoadObjectMessage.h - tells Centralserver an object couldn't be loaded.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_FailedToLoadObjectMessage_H
#define	_INCLUDED_FailedToLoadObjectMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class FailedToLoadObjectMessage : public GameNetworkMessage
{
public:
	FailedToLoadObjectMessage  (const NetworkId& id, uint32 requestingProcess);
	FailedToLoadObjectMessage  (Archive::ReadIterator & source);
	~FailedToLoadObjectMessage ();

	const NetworkId&          getId(void) const;
	uint32              getRequestingProcess(void) const;

private:
	Archive::AutoVariable<NetworkId>    m_id;				// id of object we didn't load
	Archive::AutoVariable<uint32> m_process;			// process that originally requested the load

	FailedToLoadObjectMessage();
	FailedToLoadObjectMessage(const FailedToLoadObjectMessage&);
	FailedToLoadObjectMessage& operator= (const FailedToLoadObjectMessage&);
};


//-----------------------------------------------------------------------

inline const NetworkId& FailedToLoadObjectMessage::getId(void) const
{
	return m_id.get();
}	// FailedToLoadObjectMessage::getId

inline uint32 FailedToLoadObjectMessage::getRequestingProcess(void) const
{
	return m_process.get();
}	// FailedToLoadObjectMessage::getRequestingProcess


#endif	// _INCLUDED_FailedToLoadObjectMessage_H
