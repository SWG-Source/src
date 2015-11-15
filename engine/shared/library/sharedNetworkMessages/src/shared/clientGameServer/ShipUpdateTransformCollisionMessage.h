// ======================================================================
//
// ShipUpdateTransformCollisionMessage.h
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDED_ShipUpdateTransformCollisionMessage_H
#define	INCLUDED_ShipUpdateTransformCollisionMessage_H

// ======================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Transform.h"

// ======================================================================

class ShipUpdateTransformCollisionMessage: public GameNetworkMessage
{
public:
	ShipUpdateTransformCollisionMessage(NetworkId const &networkId, Transform const &transform, Vector const &velocity, uint32 syncStampLong);
	explicit ShipUpdateTransformCollisionMessage(Archive::ReadIterator &source);

public:
	NetworkId const & getNetworkId() const;
	Transform const & getTransform() const;
	Vector const & getVelocity() const;
	uint32 getSyncStampLong() const;

private:
	ShipUpdateTransformCollisionMessage();
	ShipUpdateTransformCollisionMessage(ShipUpdateTransformCollisionMessage const &);
	ShipUpdateTransformCollisionMessage & operator=(ShipUpdateTransformCollisionMessage const &);

private:
	Archive::AutoVariable<NetworkId> m_networkId;
	Archive::AutoVariable<Transform> m_transform;
	Archive::AutoVariable<Vector> m_velocity;
	Archive::AutoVariable<uint32> m_syncStampLong;
};

// ======================================================================

#endif
