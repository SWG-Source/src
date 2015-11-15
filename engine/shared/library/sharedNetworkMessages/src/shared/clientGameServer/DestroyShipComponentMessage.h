// ======================================================================
//
// DestroyShipComponentMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_DestroyShipComponentMessage_H_
#define	_DestroyShipComponentMessage_H_

// ======================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class DestroyShipComponentMessage: public GameNetworkMessage
{
public:

	static char const * const MessageType;

	DestroyShipComponentMessage(NetworkId const &shipId, int chassisSlot, float severity);
	explicit DestroyShipComponentMessage(Archive::ReadIterator &source);

public:
	NetworkId const &getShipId() const;
	int getChassisSlot() const;
	float getSeverity() const;

private: 
	Archive::AutoVariable<NetworkId> m_shipId;
	Archive::AutoVariable<int> m_chassisSlot;
	Archive::AutoVariable<float> m_severity;
};

// ======================================================================

#endif // _DestroyShipComponentMessage_H_

