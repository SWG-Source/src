// ======================================================================
//
// AdjustAccountFeatureIdResponse.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AdjustAccountFeatureIdResponse_H
#define INCLUDED_AdjustAccountFeatureIdResponse_H

// ======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class AdjustAccountFeatureIdResponse : public GameNetworkMessage
{
public:
	AdjustAccountFeatureIdResponse  (NetworkId const & requestingPlayer, uint32 gameServer, NetworkId const & targetPlayer, std::string const & targetPlayerDescription, StationId targetStationId, NetworkId const & targetItem, std::string const & targetItemDescription, uint32 gameCode, uint32 featureId, int oldValue, int newValue, unsigned int resultCode, bool resultCameFromSession, const char * sessionResultString = nullptr, const char * sessionResultText = nullptr);
	AdjustAccountFeatureIdResponse  (Archive::ReadIterator & source);
	virtual ~AdjustAccountFeatureIdResponse ();

	NetworkId const & getRequestingPlayer() const;
	uint32 getGameServer() const;
	NetworkId const & getTargetPlayer() const;
	std::string const & getTargetPlayerDescription() const;
	StationId getTargetStationId() const;
	NetworkId const & getTargetItem() const;
	std::string const & getTargetItemDescription() const;
	uint32 getGameCode() const;
	uint32 getFeatureId() const;
	int getOldValue() const;
	int getNewValue() const;
	unsigned int getResultCode() const; // 0 = SUCCESS
	bool getResultCameFromSession() const;
	std::string const & getSessionResultString() const;
	std::string const & getSessionResultText() const;
	void setSessionResultCode(unsigned int resultCode, const char * sessionResultString = nullptr, const char * sessionResultText = nullptr);

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
	Archive::AutoVariable<int> m_oldValue;
	Archive::AutoVariable<int> m_newValue;
	Archive::AutoVariable<unsigned int> m_resultCode; // 0 = SUCCESS
	Archive::AutoVariable<bool> m_resultCameFromSession;
	Archive::AutoVariable<std::string> m_sessionResultString;
	Archive::AutoVariable<std::string> m_sessionResultText;

private:
	AdjustAccountFeatureIdResponse();
	AdjustAccountFeatureIdResponse(const AdjustAccountFeatureIdResponse&);
	AdjustAccountFeatureIdResponse& operator= (const AdjustAccountFeatureIdResponse&);
};

// ----------------------------------------------------------------------

inline NetworkId const & AdjustAccountFeatureIdResponse::getRequestingPlayer() const
{
	return m_requestingPlayer.get();
}

// ----------------------------------------------------------------------

inline uint32 AdjustAccountFeatureIdResponse::getGameServer() const
{
	return m_gameServer.get();
}

// ----------------------------------------------------------------------

inline NetworkId const & AdjustAccountFeatureIdResponse::getTargetPlayer() const
{
	return m_targetPlayer.get();
}

// ----------------------------------------------------------------------

inline std::string const & AdjustAccountFeatureIdResponse::getTargetPlayerDescription() const
{
	return m_targetPlayerDescription.get();
}

// ----------------------------------------------------------------------

inline StationId AdjustAccountFeatureIdResponse::getTargetStationId() const
{
	return m_targetStationId.get();
}

// ----------------------------------------------------------------------

inline NetworkId const & AdjustAccountFeatureIdResponse::getTargetItem() const
{
	return m_targetItem.get();
}

// ----------------------------------------------------------------------

inline std::string const & AdjustAccountFeatureIdResponse::getTargetItemDescription() const
{
	return m_targetItemDescription.get();
}

// ----------------------------------------------------------------------

inline uint32 AdjustAccountFeatureIdResponse::getGameCode() const
{
	return m_gameCode.get();
}

// ----------------------------------------------------------------------

inline uint32 AdjustAccountFeatureIdResponse::getFeatureId() const
{
	return m_featureId.get();
}

// ----------------------------------------------------------------------

inline int AdjustAccountFeatureIdResponse::getOldValue() const
{
	return m_oldValue.get();
}

// ----------------------------------------------------------------------

inline int AdjustAccountFeatureIdResponse::getNewValue() const
{
	return m_newValue.get();
}

// ----------------------------------------------------------------------

inline unsigned int AdjustAccountFeatureIdResponse::getResultCode() const
{
	return m_resultCode.get();
}

// ----------------------------------------------------------------------

inline bool AdjustAccountFeatureIdResponse::getResultCameFromSession() const
{
	return m_resultCameFromSession.get();
}

// ----------------------------------------------------------------------

inline std::string const & AdjustAccountFeatureIdResponse::getSessionResultString() const
{
	return m_sessionResultString.get();
}

// ----------------------------------------------------------------------

inline std::string const & AdjustAccountFeatureIdResponse::getSessionResultText() const
{
	return m_sessionResultText.get();
}

// ----------------------------------------------------------------------

inline void AdjustAccountFeatureIdResponse::setSessionResultCode(unsigned int resultCode, const char * sessionResultString /*= nullptr*/, const char * sessionResultText /*= nullptr*/)
{
	m_resultCode.set(resultCode);

	if (sessionResultString)
		m_sessionResultString.set(std::string(sessionResultString));
	else
		m_sessionResultString.set(std::string());

	if (sessionResultText)
		m_sessionResultText.set(std::string(sessionResultText));
	else
		m_sessionResultText.set(std::string());
}

// ======================================================================

#endif
