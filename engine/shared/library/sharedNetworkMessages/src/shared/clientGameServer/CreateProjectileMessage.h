// ======================================================================
//
// CreateProjectileMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef	_CreateProjectileMessage_H_
#define	_CreateProjectileMessage_H_

// ======================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMath/Transform.h"
#include "sharedUtility/PackedPosition.h"

// ======================================================================

class CreateProjectileMessage: public GameNetworkMessage
{
public:
	CreateProjectileMessage(uint16 shipId, int weaponIndex, int projectileIndex, int targetedComponent, Transform const &transform_p, uint32 syncStampLong);
	explicit CreateProjectileMessage(Archive::ReadIterator &source);

public:
	uint16 getShipId() const;
	int getWeaponIndex() const;
	int getProjectileIndex() const;
	int getTargetedComponent() const;
	Transform const getTransform_p() const;
	uint32 getSyncStampLong() const;

private: 
	Archive::AutoVariable<uint16> m_shipId;
	Archive::AutoVariable<int8> m_weaponIndex;
	Archive::AutoVariable<int8> m_projectileIndex;
	Archive::AutoVariable<int8> m_targetedComponent;
	Archive::AutoVariable<PackedPosition> m_startPosition_p;
	Archive::AutoVariable<PackedPosition> m_direction_p;
	Archive::AutoVariable<uint32> m_syncStampLong;
};

// ======================================================================

#endif // _CreateProjectileMessage_H_

