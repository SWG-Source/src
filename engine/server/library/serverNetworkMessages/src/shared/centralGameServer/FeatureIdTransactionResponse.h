// ======================================================================
//
// FeatureIdTransactionResponse.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FeatureIdTransactionResponse_H
#define INCLUDED_FeatureIdTransactionResponse_H

// ======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class FeatureIdTransactionResponse : public GameNetworkMessage
{
  public:
	FeatureIdTransactionResponse  (uint32 gameServer, NetworkId const & player, std::map<std::string, int> const & transactions);
	FeatureIdTransactionResponse  (Archive::ReadIterator & source);
	virtual ~FeatureIdTransactionResponse ();

	uint32 getGameServer() const;
	NetworkId const & getPlayer() const;
	std::map<std::string, int> const & getTransactions() const;

  private:
	  Archive::AutoVariable<uint32> m_gameServer;
	  Archive::AutoVariable<NetworkId> m_player;
	  Archive::AutoVariable<std::map<std::string, int> > m_transactions;

  private:
	FeatureIdTransactionResponse();
	FeatureIdTransactionResponse(const FeatureIdTransactionResponse&);
	FeatureIdTransactionResponse& operator= (const FeatureIdTransactionResponse&);
};

// ----------------------------------------------------------------------

inline uint32 FeatureIdTransactionResponse::getGameServer() const
{
	return m_gameServer.get();
}

// ----------------------------------------------------------------------

inline NetworkId const & FeatureIdTransactionResponse::getPlayer() const
{
	return m_player.get();
}

// ----------------------------------------------------------------------

inline std::map<std::string, int> const & FeatureIdTransactionResponse::getTransactions() const
{
	return m_transactions.get();
}

// ======================================================================

#endif
