// ======================================================================
//
// FeatureIdTransactionRequest.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FeatureIdTransactionRequest_H
#define INCLUDED_FeatureIdTransactionRequest_H

// ======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class FeatureIdTransactionRequest : public GameNetworkMessage
{
  public:
	FeatureIdTransactionRequest  (uint32 gameServer, StationId stationId, NetworkId const & player);
	FeatureIdTransactionRequest  (Archive::ReadIterator & source);
	virtual ~FeatureIdTransactionRequest ();

	uint32 getGameServer() const;
	StationId getStationId() const;
	NetworkId const & getPlayer() const;

  private:
	  Archive::AutoVariable<uint32> m_gameServer;
	  Archive::AutoVariable<StationId> m_stationId;
	  Archive::AutoVariable<NetworkId> m_player;

  private:
	FeatureIdTransactionRequest();
	FeatureIdTransactionRequest(const FeatureIdTransactionRequest&);
	FeatureIdTransactionRequest& operator= (const FeatureIdTransactionRequest&);
};

// ----------------------------------------------------------------------

inline uint32 FeatureIdTransactionRequest::getGameServer() const
{
	return m_gameServer.get();
}

// ----------------------------------------------------------------------

inline StationId FeatureIdTransactionRequest::getStationId() const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline NetworkId const & FeatureIdTransactionRequest::getPlayer() const
{
	return m_player.get();
}

// ======================================================================

#endif
