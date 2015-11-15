//========================================================================
//
// RequestUnloadObjectMessage.h - tells CentralServer we want to unload an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_RequestUnloadObjectMessage_H
#define	_INCLUDED_RequestUnloadObjectMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class RequestUnloadObjectMessage : public GameNetworkMessage
{
public:
	RequestUnloadObjectMessage  (const NetworkId& id, uint32 process);
	RequestUnloadObjectMessage  (Archive::ReadIterator & source);
	~RequestUnloadObjectMessage ();

	const NetworkId&          getId(void) const;
	uint32              getProcess(void) const;

private:
	Archive::AutoVariable<NetworkId>     m_id;			// id of object we want to unload
	Archive::AutoVariable<uint32>  m_process;		// process id of Gameserver that wants to unload the object

	RequestUnloadObjectMessage();
	RequestUnloadObjectMessage(const RequestUnloadObjectMessage&);
	RequestUnloadObjectMessage& operator= (const RequestUnloadObjectMessage&);
};


//-----------------------------------------------------------------------

inline const NetworkId& RequestUnloadObjectMessage::getId(void) const
{
	return m_id.get();
}	// RequestUnloadObjectMessage::getId

// ----------------------------------------------------------------------

inline uint32 RequestUnloadObjectMessage::getProcess(void) const
{
	return m_process.get();
}	// RequestUnloadObjectMessage::getProcess

// ----------------------------------------------------------------------


#endif	// _INCLUDED_RequestUnloadObjectMessage_H
