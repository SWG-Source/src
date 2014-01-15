// ======================================================================
//
// AdjustAccountFeatureIdRequest.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AdjustAccountFeatureIdRequest_H
#define INCLUDED_AdjustAccountFeatureIdRequest_H

// ======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class AdjustAccountFeatureIdRequest : public GameNetworkMessage
{
  public:
	AdjustAccountFeatureIdRequest  (NetworkId const & requestingPlayer, uint32 gameServer, NetworkId const & targetPlayer, std::string const & targetPlayerDescription, StationId targetStationId, NetworkId const & targetItem, std::string const & targetItemDescription, uint32 gameCode, uint32 featureId, int adjustment);
	AdjustAccountFeatureIdRequest  (Archive::ReadIterator & source);
	virtual ~AdjustAccountFeatureIdRequest ();

	NetworkId const & getRequestingPlayer() const;
	uint32 getGameServer() const;
	NetworkId const & getTargetPlayer() const;
	std::string const & getTargetPlayerDescription() const;
	StationId getTargetStationId() const;
	NetworkId const & getTargetItem() const;
	std::string const & getTargetItemDescription() const;
	uint32 getGameCode() const;
	uint32 getFeatureId() const;
	int getAdjustment() const;

  private:
	Archive::AutoVariable<NetworkId> m_requestingPlayer;
	Archive::AutoVariable<uint32> m_gameServer;
	Archive::AutoVariable<NetworkId> m_targetPlayer;
	Archive::AutoVariable<std::string> m_targetPlayerDescription;
	Archive::AutoVariable<StationId> m_targetStationId;
	Archive::AutoVariable<NetworkId> m_targetItem;
	Archive::AutoVariable<std::string> m_targetItemDescription;
	Archive::AutoVariable<uint32> m_gameCode;
	Archive::AutoVariable<uint32> m_featureId;
	Archive::AutoVariable<int> m_adjustment;

  private:
	AdjustAccountFeatureIdRequest();
	AdjustAccountFeatureIdRequest(const AdjustAccountFeatureIdRequest&);
	AdjustAccountFeatureIdRequest& operator= (const AdjustAccountFeatureIdRequest&);
};

// ----------------------------------------------------------------------

inline NetworkId const & AdjustAccountFeatureIdRequest::getRequestingPlayer() const
{
	return m_requestingPlayer.get();
}

// ----------------------------------------------------------------------

inline uint32 AdjustAccountFeatureIdRequest::getGameServer() const
{
	return m_gameServer.get();
}

// ----------------------------------------------------------------------

inline NetworkId const & AdjustAccountFeatureIdRequest::getTargetPlayer() const
{
	return m_targetPlayer.get();
}

// ----------------------------------------------------------------------

inline std::string const & AdjustAccountFeatureIdRequest::getTargetPlayerDescription() const
{
	return m_targetPlayerDescription.get();
}

// ----------------------------------------------------------------------

inline StationId AdjustAccountFeatureIdRequest::getTargetStationId() const
{
	return m_targetStationId.get();
}

// ----------------------------------------------------------------------

inline NetworkId const & AdjustAccountFeatureIdRequest::getTargetItem() const
{
	return m_targetItem.get();
}

// ----------------------------------------------------------------------

inline std::string const & AdjustAccountFeatureIdRequest::getTargetItemDescription() const
{
	return m_targetItemDescription.get();
}

// ----------------------------------------------------------------------

inline uint32 AdjustAccountFeatureIdRequest::getGameCode() const
{
	return m_gameCode.get();
}

// ----------------------------------------------------------------------

inline uint32 AdjustAccountFeatureIdRequest::getFeatureId() const
{
	return m_featureId.get();
}

// ----------------------------------------------------------------------

inline int AdjustAccountFeatureIdRequest::getAdjustment() const
{
	return m_adjustment.get();
}

// ======================================================================

#endif
