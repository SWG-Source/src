// ======================================================================
//
// ShipUpdateTransformMessage.h
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDED_ShipUpdateTransformMessage_H
#define	INCLUDED_ShipUpdateTransformMessage_H

// ======================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMath/Transform.h"
#include "sharedUtility/PackedTransform.h"
#include "sharedUtility/PackedVelocity.h"
#include "sharedUtility/PackedRotationRate.h"

// ======================================================================

class ShipUpdateTransformMessage: public GameNetworkMessage
{
public:
	ShipUpdateTransformMessage(uint16 shipId, Transform const &transform, Vector const &velocity, float yawRate, float pitchRate, float rollRate, uint32 syncStampLong);
	explicit ShipUpdateTransformMessage(Archive::ReadIterator &source);

public:
	uint16 getShipId() const;
	Transform const getTransform() const;
	Vector const getVelocity() const;
	float getYawRate() const;
	float getPitchRate() const;
	float getRollRate() const;
	uint32 getSyncStampLong() const;

private:
	ShipUpdateTransformMessage();
	ShipUpdateTransformMessage(ShipUpdateTransformMessage const &);
	ShipUpdateTransformMessage & operator=(ShipUpdateTransformMessage const &);

private:
	Archive::AutoVariable<uint16> m_shipId;
	Archive::AutoVariable<PackedTransform> m_transform;
	Archive::AutoVariable<PackedVelocity> m_velocity;
	Archive::AutoVariable<PackedRotationRate> m_yawRate;
	Archive::AutoVariable<PackedRotationRate> m_pitchRate;
	Archive::AutoVariable<PackedRotationRate> m_rollRate;
	Archive::AutoVariable<uint32> m_syncStampLong;
};

// ======================================================================

#endif

