// ======================================================================
//
// ShipUpdateTransformCollisionMessage.cpp
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ShipUpdateTransformCollisionMessage.h"

#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedMathArchive/VectorArchive.h"

// ======================================================================

static ConstCharCrcString s_shipUpdateTransformCollisionMessageName("ShipUpdateTransformCollisionMessage");

// ======================================================================

ShipUpdateTransformCollisionMessage::ShipUpdateTransformCollisionMessage(NetworkId const &networkId, Transform const &transform, Vector const &velocity, uint32 syncStampLong) :
	GameNetworkMessage(s_shipUpdateTransformCollisionMessageName),
	m_networkId(networkId),
	m_transform(transform),
	m_velocity(velocity),
	m_syncStampLong(syncStampLong)
{
	addVariable(m_networkId);
	addVariable(m_transform);
	addVariable(m_velocity);
	addVariable(m_syncStampLong);

#ifdef _DEBUG
	IGNORE_RETURN(m_transform.get().validate());
#endif
}

// ----------------------------------------------------------------------

ShipUpdateTransformCollisionMessage::ShipUpdateTransformCollisionMessage(Archive::ReadIterator &source) :
	GameNetworkMessage(s_shipUpdateTransformCollisionMessageName),
	m_networkId(),
	m_transform(),
	m_velocity(),
	m_syncStampLong()
{
	addVariable(m_networkId);
	addVariable(m_transform);
	addVariable(m_velocity);
	addVariable(m_syncStampLong);

	unpack(source);

#ifdef _DEBUG
	IGNORE_RETURN(m_transform.get().validate());
#endif
}

// ----------------------------------------------------------------------

NetworkId const & ShipUpdateTransformCollisionMessage::getNetworkId() const
{
	return m_networkId.get();
}

// ----------------------------------------------------------------------

Transform const & ShipUpdateTransformCollisionMessage::getTransform() const
{
	return m_transform.get();
}

// ----------------------------------------------------------------------

Vector const & ShipUpdateTransformCollisionMessage::getVelocity() const
{
	return m_velocity.get();
}

// ----------------------------------------------------------------------

uint32 ShipUpdateTransformCollisionMessage::getSyncStampLong() const
{
	return m_syncStampLong.get();
}

// ======================================================================

