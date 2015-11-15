// ======================================================================
//
// ClaimRewardsReplyMessage.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClaimRewardsReplyMessage_H
#define INCLUDED_ClaimRewardsReplyMessage_H

// ======================================================================

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Request claiming a set of veteran rewards
 * Sent from:  Game Server
 * Sent to:  Login Server (via Central)
 * Action:  Check that these rewards can still be claimed on this account.
 */
class ClaimRewardsReplyMessage : public GameNetworkMessage
{
  public:
	ClaimRewardsReplyMessage  (uint32 gameServer, StationId stationId, NetworkId const & player, std::string const & rewardEvent, std::string const & rewardItem, uint32 accountFeatureId, bool consumeAccountFeatureId, int previousAccountFeatureIdCount, int currentAccountFeatureIdCount, bool result);
	ClaimRewardsReplyMessage  (Archive::ReadIterator & source);
	~ClaimRewardsReplyMessage ();

	uint32 getGameServer() const;
	StationId getStationId() const;
	NetworkId const & getPlayer() const;
	std::string const & getRewardEvent() const;
	std::string const & getRewardItem() const;
	uint32 getAccountFeatureId() const;
	bool getConsumeAccountFeatureId() const;
	int getPreviousAccountFeatureIdCount() const;
	int getCurrentAccountFeatureIdCount() const;
	bool getResult() const;
	
  private:
	Archive::AutoVariable<uint32> m_gameServer;
	Archive::AutoVariable<StationId> m_stationId;
	Archive::AutoVariable<NetworkId> m_player;
	Archive::AutoVariable<std::string> m_rewardEvent;
	Archive::AutoArray<std::string> m_accountUniqueItems; // once-per-account items that should be registered in the login database
	Archive::AutoArray<std::string> m_additionalItems;    // rewards the player is claiming which are not once-per-account and don't need to be registered in the login database
	Archive::AutoVariable<uint32> m_accountFeatureId; // if non-zero, reward required that the account has this feature id, and reward has consumed 1 count of this feature id
	Archive::AutoVariable<bool> m_consumeAccountFeatureId; // for account feature Id reward, indicates whether to consume the account feature Id
	Archive::AutoVariable<int> m_previousAccountFeatureIdCount;
	Archive::AutoVariable<int> m_currentAccountFeatureIdCount;
	Archive::AutoVariable<bool> m_result;
	
  private:
	ClaimRewardsReplyMessage();
	ClaimRewardsReplyMessage(const ClaimRewardsReplyMessage&);
	ClaimRewardsReplyMessage& operator= (const ClaimRewardsReplyMessage&);
};

// ======================================================================

#endif
