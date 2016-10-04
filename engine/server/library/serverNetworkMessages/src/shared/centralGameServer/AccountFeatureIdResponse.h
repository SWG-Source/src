// ======================================================================
//
// AccountFeatureIdResponse.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AccountFeatureIdResponse_H
#define INCLUDED_AccountFeatureIdResponse_H

// ======================================================================

#include "serverNetworkMessages/AccountFeatureIdRequest.h"
#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class AccountFeatureIdResponse : public GameNetworkMessage
{
  public:
	AccountFeatureIdResponse  (NetworkId const & requester, uint32 gameServer, NetworkId const & target, StationId targetStationId, uint32 gameCode, AccountFeatureIdRequest::RequestReason requestReason, unsigned int resultCode, bool resultCameFromSession, std::map<uint32, int> const & featureIds, std::map<uint32, std::string> const & sessionFeatureIdsData, const char * sessionResultString = nullptr, const char * sessionResultText = nullptr);
	AccountFeatureIdResponse  (Archive::ReadIterator & source);
	virtual ~AccountFeatureIdResponse ();

	NetworkId const & getRequester() const;
	uint32 getGameServer() const;
	NetworkId const & getTarget() const;
	StationId getTargetStationId() const;
	uint32 getGameCode() const;
	AccountFeatureIdRequest::RequestReason getRequestReason() const;
	unsigned int getResultCode() const; // 0 = SUCCESS
	bool getResultCameFromSession() const;
	std::map<uint32, int> const & getFeatureIds() const;
	std::map<uint32, std::string> const & getSessionFeatureIdsData() const;
	std::string const & getSessionResultString() const;
	std::string const & getSessionResultText() const;

  private:
	Archive::AutoVariable<NetworkId> m_requester;
	Archive::AutoVariable<uint32> m_gameServer;
	Archive::AutoVariable<NetworkId> m_target;
	Archive::AutoVariable<StationId> m_targetStationId;
	Archive::AutoVariable<uint32> m_gameCode;
	Archive::AutoVariable<int8> m_requestReason;
	Archive::AutoVariable<unsigned int> m_resultCode; // 0 = SUCCESS
	Archive::AutoVariable<bool> m_resultCameFromSession;
	Archive::AutoVariable<std::map<uint32, int> > m_featureIds;
	Archive::AutoVariable<std::map<uint32, std::string> > m_sessionFeatureIdsData;
	Archive::AutoVariable<std::string> m_sessionResultString;
	Archive::AutoVariable<std::string> m_sessionResultText;

  private:
	AccountFeatureIdResponse();
	AccountFeatureIdResponse(const AccountFeatureIdResponse&);
	AccountFeatureIdResponse& operator= (const AccountFeatureIdResponse&);
};

// ----------------------------------------------------------------------

inline NetworkId const & AccountFeatureIdResponse::getRequester() const
{
	return m_requester.get();
}

// ----------------------------------------------------------------------

inline uint32 AccountFeatureIdResponse::getGameServer() const
{
	return m_gameServer.get();
}

// ----------------------------------------------------------------------

inline NetworkId const & AccountFeatureIdResponse::getTarget() const
{
	return m_target.get();
}

// ----------------------------------------------------------------------

inline StationId AccountFeatureIdResponse::getTargetStationId() const
{
	return m_targetStationId.get();
}

// ----------------------------------------------------------------------

inline uint32 AccountFeatureIdResponse::getGameCode() const
{
	return m_gameCode.get();
}

// ----------------------------------------------------------------------

inline AccountFeatureIdRequest::RequestReason AccountFeatureIdResponse::getRequestReason() const
{
	return static_cast<AccountFeatureIdRequest::RequestReason>(m_requestReason.get());
}

// ----------------------------------------------------------------------

inline unsigned int AccountFeatureIdResponse::getResultCode() const
{
	return m_resultCode.get();
}

// ----------------------------------------------------------------------

inline bool AccountFeatureIdResponse::getResultCameFromSession() const
{
	return m_resultCameFromSession.get();
}

// ----------------------------------------------------------------------

inline std::map<uint32, int> const & AccountFeatureIdResponse::getFeatureIds() const
{
	return m_featureIds.get();
}

// ----------------------------------------------------------------------

inline std::map<uint32, std::string> const & AccountFeatureIdResponse::getSessionFeatureIdsData() const
{
	return m_sessionFeatureIdsData.get();
}

// ----------------------------------------------------------------------

inline std::string const & AccountFeatureIdResponse::getSessionResultString() const
{
	return m_sessionResultString.get();
}

// ----------------------------------------------------------------------

inline std::string const & AccountFeatureIdResponse::getSessionResultText() const
{
	return m_sessionResultText.get();
}

// ======================================================================

#endif
