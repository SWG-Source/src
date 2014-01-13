// ======================================================================
//
// ClientOpenContainerMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_ClientOpenContainerMessage_H
#define	_ClientOpenContainerMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class ClientOpenContainerMessage : public GameNetworkMessage
{
public:

	ClientOpenContainerMessage(NetworkId containerId, std::string slot );
	explicit ClientOpenContainerMessage(Archive::ReadIterator &source);

	~ClientOpenContainerMessage();

public: // methods

	NetworkId                        getContainerId() const;
	std::string                      getSlot() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId>             m_containerId;
	Archive::AutoVariable<std::string>           m_slot; 
};

// ----------------------------------------------------------------------

inline NetworkId ClientOpenContainerMessage::getContainerId() const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

inline std::string ClientOpenContainerMessage::getSlot() const
{
	return m_slot.get();
}

// ----------------------------------------------------------------------

#endif // _ClientOpenContainerMessage_H

