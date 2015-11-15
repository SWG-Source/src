// ======================================================================
//
// AccountFeatureIdRequest.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AccountFeatureIdRequest_H
#define INCLUDED_AccountFeatureIdRequest_H

// ======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class AccountFeatureIdRequest : public GameNetworkMessage
{
  public:
	enum RequestReason {RR_Reload, RR_ConsoleCommandReloadRequest, RR_ReloadRewardCheck};

	AccountFeatureIdRequest  (NetworkId const & requester, uint32 gameServer, NetworkId const & target, StationId targetStationId, uint32 gameCode, RequestReason requestReason);
	AccountFeatureIdRequest  (Archive::ReadIterator & source);
	virtual ~AccountFeatureIdRequest ();

	NetworkId const & getRequester() const;
	uint32 getGameServer() const;
	NetworkId const & getTarget() const;
	StationId getTargetStationId() const;
	uint32 getGameCode() const;
	RequestReason getRequestReason() const;

  private:
	Archive::AutoVariable<NetworkId> m_requester;
	Archive::AutoVariable<uint32> m_gameServer;
	Archive::AutoVariable<NetworkId> m_target;
	Archive::AutoVariable<StationId> m_targetStationId;
	Archive::AutoVariable<uint32> m_gameCode;
	Archive::AutoVariable<int8> m_requestReason;

  private:
	AccountFeatureIdRequest();
	AccountFeatureIdRequest(const AccountFeatureIdRequest&);
	AccountFeatureIdRequest& operator= (const AccountFeatureIdRequest&);
};

// ----------------------------------------------------------------------

inline NetworkId const & AccountFeatureIdRequest::getRequester() const
{
	return m_requester.get();
}

// ----------------------------------------------------------------------

inline uint32 AccountFeatureIdRequest::getGameServer() const
{
	return m_gameServer.get();
}

// ----------------------------------------------------------------------

inline NetworkId const & AccountFeatureIdRequest::getTarget() const
{
	return m_target.get();
}

// ----------------------------------------------------------------------

inline StationId AccountFeatureIdRequest::getTargetStationId() const
{
	return m_targetStationId.get();
}

// ----------------------------------------------------------------------

inline uint32 AccountFeatureIdRequest::getGameCode() const
{
	return m_gameCode.get();
}

// ----------------------------------------------------------------------

inline AccountFeatureIdRequest::RequestReason AccountFeatureIdRequest::getRequestReason() const
{
	return static_cast<RequestReason>(m_requestReason.get());
}

// ======================================================================

#endif
