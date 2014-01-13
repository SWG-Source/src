// ======================================================================
//
// CreateMissileMessage.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CreateMissileMessage_H
#define INCLUDED_CreateMissileMessage_H

// ======================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class CreateMissileMessage : public GameNetworkMessage
{
  public:
	CreateMissileMessage(int missileId, const NetworkId &source, const NetworkId &target, const Vector & sourceLocation, const Vector & targetLocation, int impactTime, int missileTypeId, int weaponId, int targetComponent);
	explicit CreateMissileMessage(Archive::ReadIterator &source);

  public:
	int getMissileId() const;
	const NetworkId & getSource() const;
	const NetworkId & getTarget() const;
	const Vector & getSourceLocation() const;
	const Vector & getTargetLocation() const;
	int getImpactTime() const;
	int getMissileTypeId() const;
	int getWeaponId() const;
	int getTargetComponent() const;
	
  private:
	Archive::AutoVariable<int> m_missileId;
	Archive::AutoVariable<NetworkId> m_source;
	Archive::AutoVariable<NetworkId> m_target;
	Archive::AutoVariable<Vector> m_sourceLocation;
	Archive::AutoVariable<Vector> m_targetLocation;
	Archive::AutoVariable<int> m_impactTime;
	Archive::AutoVariable<int> m_missileTypeId;
	Archive::AutoVariable<int> m_weaponId;
	Archive::AutoVariable<int> m_targetComponent;
};

// ======================================================================

inline int CreateMissileMessage::getMissileId() const
{
	return m_missileId.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & CreateMissileMessage::getSource() const
{
	return m_source.get();
}

// ----------------------------------------------------------------------

inline const NetworkId & CreateMissileMessage::getTarget() const
{
	return m_target.get();
}

// ----------------------------------------------------------------------

inline const Vector & CreateMissileMessage::getSourceLocation() const
{
	return m_sourceLocation.get();
}

// ----------------------------------------------------------------------

inline const Vector & CreateMissileMessage::getTargetLocation() const
{
	return m_targetLocation.get();
}

// ----------------------------------------------------------------------

inline int CreateMissileMessage::getImpactTime() const
{
	return m_impactTime.get();
}

// ----------------------------------------------------------------------

inline int CreateMissileMessage::getMissileTypeId() const
{
	return m_missileTypeId.get();
}

// ----------------------------------------------------------------------

inline int CreateMissileMessage::getWeaponId() const
{
	return m_weaponId.get();
}
	
// ----------------------------------------------------------------------

inline int CreateMissileMessage::getTargetComponent() const
{
	return m_targetComponent.get();
}
	
// ======================================================================

#endif
