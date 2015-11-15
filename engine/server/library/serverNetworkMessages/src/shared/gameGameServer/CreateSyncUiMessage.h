//========================================================================
//
// CreateSyncUiMessage.h - tells Gameserver to create a new object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_CreateSyncUiMessage_H
#define	_INCLUDED_CreateSyncUiMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaVector.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/Tag.h"


//-----------------------------------------------------------------------

class CreateSyncUiMessage : public GameNetworkMessage
{
public:
	CreateSyncUiMessage  (const NetworkId& id, const stdvector<NetworkId>::fwd & clients);
	CreateSyncUiMessage  (Archive::ReadIterator & source);
	~CreateSyncUiMessage ();

public:
	const NetworkId&                  getId(void) const;
	const stdvector<NetworkId>::fwd & getClients() const;
	
private:
	Archive::AutoVariable<NetworkId>         m_id;               // id of the object
	Archive::AutoDeltaVector<NetworkId>      m_clients;          // id of the object
	
	CreateSyncUiMessage();
	CreateSyncUiMessage(const CreateSyncUiMessage&);
	CreateSyncUiMessage& operator= (const CreateSyncUiMessage&);
};

// ======================================================================

inline const NetworkId& CreateSyncUiMessage::getId(void) const
{
	return m_id.get();
}

//-----------------------------------------------------------------------

inline const stdvector<NetworkId>::fwd & CreateSyncUiMessage::getClients() const
{
	return m_clients.get();
}

// ======================================================================

#endif	// _INCLUDED_CreateSyncUiMessage_H
