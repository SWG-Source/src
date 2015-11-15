// ======================================================================
//
// DestroyShipMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_DestroyShipMessage_H_
#define	_DestroyShipMessage_H_

// ======================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class DestroyShipMessage: public GameNetworkMessage
{
public:

	static char const * const MessageType;

	DestroyShipMessage(NetworkId const &shipId, float severity);
	explicit DestroyShipMessage(Archive::ReadIterator &source);

public:
	NetworkId const &getShipId() const;
	float getSeverity() const;

private: 
	Archive::AutoVariable<NetworkId> m_shipId;
	Archive::AutoVariable<float> m_severity;
};

// ======================================================================

#endif // _DestroyShipMessage_H_

