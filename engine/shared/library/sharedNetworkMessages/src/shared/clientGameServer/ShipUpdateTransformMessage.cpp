// ======================================================================
//
// ShipUpdateTransformMessage.cpp
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ShipUpdateTransformMessage.h"

#include "sharedFoundation/ConstCharCrcString.h"

// ======================================================================

static ConstCharCrcString s_shipUpdateTransformMessageName("ShipUpdateTransformMessage");

// ======================================================================

ShipUpdateTransformMessage::ShipUpdateTransformMessage(uint16 shipId, Transform const &transform, Vector const &velocity, float yawRate, float pitchRate, float rollRate, uint32 syncStampLong) :
	GameNetworkMessage(s_shipUpdateTransformMessageName),
	m_shipId(shipId),
	m_transform(transform),
	m_velocity(velocity),
	m_yawRate(yawRate),
	m_pitchRate(pitchRate),
	m_rollRate(rollRate),
	m_syncStampLong(syncStampLong)
{
	addVariable(m_shipId);
	addVariable(m_transform);
	addVariable(m_velocity);
	addVariable(m_yawRate);
	addVariable(m_pitchRate);
	addVariable(m_rollRate);
	addVariable(m_syncStampLong);

#ifdef _DEBUG
	IGNORE_RETURN(transform.validate());
#endif
}

// ----------------------------------------------------------------------

ShipUpdateTransformMessage::ShipUpdateTransformMessage(Archive::ReadIterator &source) :
	GameNetworkMessage(s_shipUpdateTransformMessageName),
	m_shipId(),
	m_transform(),
	m_velocity(),
	m_yawRate(),
	m_pitchRate(),
	m_rollRate(),
	m_syncStampLong()
{
	addVariable(m_shipId);
	addVariable(m_transform);
	addVariable(m_velocity);
	addVariable(m_yawRate);
	addVariable(m_pitchRate);
	addVariable(m_rollRate);
	addVariable(m_syncStampLong);

	unpack(source);

#ifdef _DEBUG
	IGNORE_RETURN(m_transform.get().get().validate());
#endif
}

// ----------------------------------------------------------------------

uint16 ShipUpdateTransformMessage::getShipId() const
{
	return m_shipId.get();
}

// ----------------------------------------------------------------------

Transform const ShipUpdateTransformMessage::getTransform() const
{
	return m_transform.get().get();
}

// ----------------------------------------------------------------------

Vector const ShipUpdateTransformMessage::getVelocity() const
{
	return m_velocity.get().get();
}

// ----------------------------------------------------------------------

float ShipUpdateTransformMessage::getYawRate() const
{
	return m_yawRate.get().get();
}

// ----------------------------------------------------------------------

float ShipUpdateTransformMessage::getPitchRate() const
{
	return m_pitchRate.get().get();
}

// ----------------------------------------------------------------------

float ShipUpdateTransformMessage::getRollRate() const
{
	return m_rollRate.get().get();
}

// ----------------------------------------------------------------------

uint32 ShipUpdateTransformMessage::getSyncStampLong() const
{
	return m_syncStampLong.get();
}

// ======================================================================

