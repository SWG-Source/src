// ======================================================================
//
// CreateProjectileMessage.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CreateProjectileMessage.h"

#include "sharedGame/ShipChassisSlotType.h"

#include "sharedMathArchive/TransformArchive.h"

// ======================================================================

CreateProjectileMessage::CreateProjectileMessage(uint16 shipId, int weaponIndex, int projectileIndex, int targetedComponent, Transform const &transform_p, uint32 syncStampLong) :
	GameNetworkMessage("CreateProjectileMessage"),
	m_shipId(shipId),
	m_weaponIndex(static_cast<int8>(weaponIndex)),
	m_projectileIndex(static_cast<int8>(projectileIndex)),
	m_targetedComponent(static_cast<int8>(targetedComponent)),
	m_startPosition_p(transform_p.getPosition_p()),
	m_direction_p(transform_p.getLocalFrameK_p()*7800.f),
	m_syncStampLong(syncStampLong)
{
	addVariable(m_shipId);
	addVariable(m_weaponIndex);
	addVariable(m_projectileIndex);
	addVariable(m_targetedComponent);
	addVariable(m_startPosition_p);
	addVariable(m_direction_p);
	addVariable(m_syncStampLong);
}

// ----------------------------------------------------------------------

CreateProjectileMessage::CreateProjectileMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("CreateProjectileMessage"),
	m_shipId(),
	m_weaponIndex(0),
	m_projectileIndex(0),
	m_targetedComponent(static_cast<int8>(ShipChassisSlotType::SCST_num_types)),
	m_startPosition_p(),
	m_direction_p(),
	m_syncStampLong(0)
{
	addVariable(m_shipId);
	addVariable(m_weaponIndex);
	addVariable(m_projectileIndex);
	addVariable(m_targetedComponent);
	addVariable(m_startPosition_p);
	addVariable(m_direction_p);
	addVariable(m_syncStampLong);
	unpack(source);
}

// ----------------------------------------------------------------------

uint16 CreateProjectileMessage::getShipId() const
{
	return m_shipId.get();
}

// ----------------------------------------------------------------------

int CreateProjectileMessage::getWeaponIndex() const
{
	return m_weaponIndex.get();
}

// ----------------------------------------------------------------------

int CreateProjectileMessage::getProjectileIndex() const
{
	return m_projectileIndex.get();
}

// ----------------------------------------------------------------------

int CreateProjectileMessage::getTargetedComponent() const
{
	return m_targetedComponent.get();
}

// ----------------------------------------------------------------------

Transform const CreateProjectileMessage::getTransform_p() const
{
	Transform ret(Transform::IF_none);
	Vector const &startPosition_p = m_startPosition_p.get().get();
	Vector direction_p = m_direction_p.get().get();
	direction_p.normalize();
	ret.setLocalFrameKJ_p(direction_p, Vector::perpendicular(direction_p));
	ret.setPosition_p(startPosition_p);
	return ret;
}

// ----------------------------------------------------------------------

uint32 CreateProjectileMessage::getSyncStampLong() const
{
	return m_syncStampLong.get();
}

// ======================================================================

