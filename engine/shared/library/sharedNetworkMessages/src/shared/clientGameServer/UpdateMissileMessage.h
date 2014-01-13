// ======================================================================
//
// UpdateMissileMessage.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UpdateMissileMessage_H
#define INCLUDED_UpdateMissileMessage_H

// ======================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * A network message to tell the client about a change to a missile.
 * Depending on the type of update, some fields in this message may not
 * be relevant.
 */
class UpdateMissileMessage : public GameNetworkMessage
{
  public:
	enum UpdateType {UT_miss, UT_hit, UT_countermeasured, UT_countermeasureFailed};
	
  public:
	UpdateMissileMessage(int missileId, NetworkId const & shipId, int countermeasureType, UpdateType updateType);
	explicit UpdateMissileMessage(Archive::ReadIterator &source);

  public:
	int getMissileId() const;
	NetworkId const & getShipId() const;
	int getCountermeasureType() const;
	UpdateType getUpdateType() const;
	
  private:
	Archive::AutoVariable<int> m_missileId;
	Archive::AutoVariable<NetworkId> m_shipId;
	Archive::AutoVariable<int> m_countermeasureType;
	Archive::AutoVariable<int> m_updateType;
};

// ======================================================================

inline int UpdateMissileMessage::getMissileId() const
{
	return m_missileId.get();
}

// ----------------------------------------------------------------------

inline UpdateMissileMessage::UpdateType UpdateMissileMessage::getUpdateType() const
{
	return static_cast<UpdateType>(m_updateType.get());
}

// ----------------------------------------------------------------------

inline NetworkId const & UpdateMissileMessage::getShipId() const
{
	return m_shipId.get();
}

// ----------------------------------------------------------------------

inline int UpdateMissileMessage::getCountermeasureType() const
{
	return m_countermeasureType.get();
}

// ======================================================================

#endif
