// ======================================================================
//
// FeatureIdTransactionSyncUpdate.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FeatureIdTransactionSyncUpdate_H
#define INCLUDED_FeatureIdTransactionSyncUpdate_H

// ======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class FeatureIdTransactionSyncUpdate : public GameNetworkMessage
{
  public:
	FeatureIdTransactionSyncUpdate  (StationId stationId, NetworkId const & player, std::string const & itemId, int adjustment);
	FeatureIdTransactionSyncUpdate  (Archive::ReadIterator & source);
	virtual ~FeatureIdTransactionSyncUpdate ();

	StationId getStationId() const;
	NetworkId const & getPlayer() const;
	std::string const & getItemId() const;
	int getAdjustment() const;

  private:
	  Archive::AutoVariable<StationId> m_stationId;
	  Archive::AutoVariable<NetworkId> m_player;
	  Archive::AutoVariable<std::string> m_itemId;
	  Archive::AutoVariable<int> m_adjustment;

  private:
	FeatureIdTransactionSyncUpdate();
	FeatureIdTransactionSyncUpdate(const FeatureIdTransactionSyncUpdate&);
	FeatureIdTransactionSyncUpdate& operator= (const FeatureIdTransactionSyncUpdate&);
};

// ----------------------------------------------------------------------

inline StationId FeatureIdTransactionSyncUpdate::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline NetworkId const & FeatureIdTransactionSyncUpdate::getPlayer() const
{
	return m_player.get();
}

// ----------------------------------------------------------------------

inline std::string const & FeatureIdTransactionSyncUpdate::getItemId() const
{
	return m_itemId.get();
}

// ----------------------------------------------------------------------

inline int FeatureIdTransactionSyncUpdate::getAdjustment() const
{
	return m_adjustment.get();
}

// ======================================================================

#endif
