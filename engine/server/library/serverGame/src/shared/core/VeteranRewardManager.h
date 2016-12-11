// ======================================================================
//
// VeteranRewardManager.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_VeteranRewardManager_H
#define INCLUDED_VeteranRewardManager_H

// ======================================================================

#include "StringId.h"
#include "sharedFoundation/StationId.h"

// ======================================================================

class AdjustAccountFeatureIdResponse;
class CreatureObject;
class NetworkId;
class ServerObject;

// ======================================================================

class VeteranRewardManager
{
  public:
	static void install();

	static void setOverrideAccountAge(int accountAge);
	
	static bool claimRewards(CreatureObject const & playerCreature, std::string const & rewardEventName, std::string const & rewardItem, std::string * debugMessage);
	static void handleClaimRewardsReply(StationId stationId, NetworkId const & player, std::string const & rewardEvent, std::string const & rewardItem, uint32 accountFeatureId, bool consumeAccountFeatureId, int previousAccountFeatureIdCount, int currentAccountFeatureIdCount, bool result, bool verify);

	static void writeAccountDataToObjvars(CreatureObject & playerCreature);
	
	static void getTriggeredEventsIds(CreatureObject const & playerCreature, std::vector<std::string> & eventsIds);
	static void getAvailableRewardItemsForEvent(CreatureObject const & playerCreature, std::string const & rewardEventName, std::vector<std::string> & rewardItems, std::vector<std::string> * rewardDescriptions);
	static void getRewardChoicesDescriptions(CreatureObject const & playerCreature, std::string const & eventName, std::vector<std::string> & rewardDescriptions);
	static void getRewardChoicesTags(CreatureObject const & playerCreature, std::string const & eventName, std::vector<Unicode::String> & rewardTagsUnicode);
	static StringId const * getEventAnnouncement(std::string const & eventName);
	static StringId const * getEventDescription(std::string const & eventName);
	static std::string const * getEventUrl(std::string const & eventName);
	static bool isEventAccountUnique(std::string const & eventName);
	static bool isItemAccountUnique(std::string const & itemName);
	static bool isItemAccountUniqueFeatureId(std::string const & itemName);
	static void verifyFeatureIdTransactions(CreatureObject & playerCreature, std::map<std::string, int> const & transactionsFromLoginServer);
	static void migrateVeteranRewardsClaimInformation(CreatureObject & playerCreature);
	static void getConsumedEvents(ServerObject const & playerCreature, std::vector<std::string> & consumedEvents);
	static void getClaimedItems(ServerObject const & playerCreature, std::vector<std::string> & claimedItems);
	static void tcgRedemption(CreatureObject const & playerCreature, ServerObject & item, int featureId, int adjustment);
	static bool checkForTcgRedemptionInProgress(ServerObject const & item);
	static void handleTcgRedemptionResponse(ServerObject & item, AdjustAccountFeatureIdResponse const & response);
	static void attachTradeInScriptToItem(ServerObject & item);
	static bool tradeInReward(CreatureObject const & playerCreature, ServerObject & item, bool validateOnly);
	static bool checkForTradeInInProgress(ServerObject const & item);
	static void handleTradeInResponse(ServerObject & item, AdjustAccountFeatureIdResponse const & response);
	static bool isTradeInRewardFeatureId(uint32 featureId);
	static void announceMonthlyBonusBoosterPack(CreatureObject const & playerCreature);
	static void announceSwgTcgBeta(CreatureObject const & playerCreature);
	static bool isFeatureIdBlockedBySubscriptionBit(CreatureObject const & playerCreature, uint32 featureId);

  private:
	static void remove();
};

// ======================================================================

#endif
