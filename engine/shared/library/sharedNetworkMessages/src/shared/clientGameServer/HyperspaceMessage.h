// ======================================================================
//
// HyperspaceMessage.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef	_HyperspaceMessage_H_
#define	_HyperspaceMessage_H_

// ======================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Transform.h"

// ======================================================================

class HyperspaceMessage: public GameNetworkMessage
{
public:
	HyperspaceMessage(NetworkId const & ownerId);
	explicit HyperspaceMessage(Archive::ReadIterator & source);

public:
	NetworkId const & getOwnerId() const;

private: 
	Archive::AutoVariable<NetworkId> m_ownerId;
};

// ======================================================================

#endif // _HyperspaceMessage_H_

