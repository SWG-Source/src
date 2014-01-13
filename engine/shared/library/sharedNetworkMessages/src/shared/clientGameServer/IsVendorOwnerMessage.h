// ======================================================================
//
// IsVendorOwnerMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_IsVendorOwnerMessage_H
#define	_IsVendorOwnerMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Tag.h"

//-----------------------------------------------------------------------

class IsVendorOwnerMessage : public GameNetworkMessage
{
public:

	IsVendorOwnerMessage(NetworkId containerId);
	explicit IsVendorOwnerMessage(Archive::ReadIterator &source);

	~IsVendorOwnerMessage();

public: // methods

	NetworkId                        getContainerId() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId> m_containerId;
};

// ----------------------------------------------------------------------

inline NetworkId IsVendorOwnerMessage::getContainerId() const
{
	return m_containerId.get();
}

// ----------------------------------------------------------------------

#endif // _IsVendorOwnerMessage_H

