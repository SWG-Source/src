// ======================================================================
//
// ClaimRewardsMessage.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClaimRewardsMessage_H
#define INCLUDED_ClaimRewardsMessage_H

// ======================================================================

#include "sharedFoundation/StationId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

/**
 * Request claiming a set of veteran rewards
 * Sent from:  Game Server
 * Sent to:  Login Server (via Central)
 * Action:  Check that these rewards can still be claimed on this account.
 */
class ClaimRewardsMessage : public GameNetworkMessage
{
  public:
	ClaimRewardsMessage  (uint32 gameServer, StationId stationId, NetworkId const & player, std::string const & rewardEvent, bool consumeEvent, std::string const & rewardItem, bool consumeItem, uint32 accountFeatureId, bool consumeAccountFeatureId);
	ClaimRewardsMessage  (Archive::ReadIterator & source);
	virtual ~ClaimRewardsMessage ();

	uint32 getGameServer() const;
	StationId getStationId() const;
	NetworkId const & getPlayer() const;
	std::string const & getRewardEvent() const;
	bool getConsumeEvent() const;
	std::string const & getRewardItem() const;
	bool getConsumeItem() const;
	uint32 getAccountFeatureId() const;
	bool getConsumeAccountFeatureId() const;
	std::string const & getAccountFeatureIdOldValue() const;
	void setAccountFeatureIdOldValue(std::string const & accountFeatureIdOldValue);
	std::string const & getAccountFeatureIdNewValue() const;
	void setAccountFeatureIdNewValue(std::string const & accountFeatureIdNewValue);

  private:
	Archive::AutoVariable<uint32> m_gameServer;
	Archive::AutoVariable<StationId> m_stationId;
	Archive::AutoVariable<NetworkId> m_player;
	Archive::AutoVariable<std::string> m_rewardEvent;
	Archive::AutoVariable<bool> m_consumeEvent;
	Archive::AutoArray<std::string> m_accountUniqueItems; // once-per-account items that should be registered in the login database
	Archive::AutoArray<std::string> m_additionalItems;    // rewards the player is claiming which are not once-per-account and don't need to be registered in the login database
	Archive::AutoVariable<uint32> m_accountFeatureId; // if non-zero, reward requires that the account has this feature id, and reward will consume 1 count of this feature id
	Archive::AutoVariable<bool> m_consumeAccountFeatureId; // for account feature Id reward, indicates whether to consume the account feature Id
	
	// for reward that requires an account feature id,
	// these fields will contain the old and new value
	// of the account feature id; these values are passed to
	// session/Platform to update the account feature id; only
	// on a successful update will the reward then be granted
	Archive::AutoVariable<std::string> m_accountFeatureIdOldValue;
	Archive::AutoVariable<std::string> m_accountFeatureIdNewValue;

  private:
	ClaimRewardsMessage();
	ClaimRewardsMessage(const ClaimRewardsMessage&);
	ClaimRewardsMessage& operator= (const ClaimRewardsMessage&);
};

// ======================================================================

#endif
