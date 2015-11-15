// ======================================================================
//
// UpdateContainmentMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_UpdateContainmentMessage_H_
#define	_UpdateContainmentMessage_H_

// ======================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class UpdateContainmentMessage: public GameNetworkMessage
{
public:
	UpdateContainmentMessage(NetworkId const &networkId, NetworkId const &containerId, int slotArrangement);
	explicit UpdateContainmentMessage(Archive::ReadIterator &source);

public:

	NetworkId const &getNetworkId() const;
	NetworkId const &getContainerId() const;
	int getSlotArrangement() const;

private: 
	Archive::AutoVariable<NetworkId> m_networkId;
	Archive::AutoVariable<NetworkId> m_containerId;
	Archive::AutoVariable<int> m_slotArrangement;
};

// ----------------------------------------------------------------------

inline NetworkId const &UpdateContainmentMessage::getNetworkId() const
{
	return m_networkId.get();
}

// ----------------------------------------------------------------------

inline NetworkId const &UpdateContainmentMessage::getContainerId() const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

inline int UpdateContainmentMessage::getSlotArrangement() const
{
	return m_slotArrangement.get();
}

// ======================================================================

#endif // _UpdateContainmentMessage_H_

