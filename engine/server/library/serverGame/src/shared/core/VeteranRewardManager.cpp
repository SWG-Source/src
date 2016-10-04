// ======================================================================
//
// VeteranRewardManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/VeteranRewardManager.h"

#include "StringId.h"
#include "UnicodeUtils.h"
#include "serverGame/Chat.h"
#include "serverGame/Client.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/AccountFeatureIdRequest.h"
#include "serverNetworkMessages/AdjustAccountFeatureIdRequest.h"
#include "serverNetworkMessages/AdjustAccountFeatureIdResponse.h"
#include "serverNetworkMessages/ClaimRewardsMessage.h"
#include "serverNetworkMessages/ClaimRewardsReplyMessage.h"
#include "serverNetworkMessages/FeatureIdTransactionSyncUpdate.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptParameters.h"
#include "Session/CommonAPI/CommonAPI.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedGame/ProsePackage.h"
#include "sharedLog/Log.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include <algorithm>
#include <limits>

// ======================================================================

namespace VeteranRewardManagerNamespace
{
	std::string const ms_rewardItemsTableName("datatables/veteran_rewards/items.iff");
	std::string const ms_rewardEventsTableName("datatables/veteran_rewards/events.iff");

	std::string const ms_consumedEventsObjvar("veteran_rewards.events_consumed");
	std::string const ms_claimedItemsObjvar("veteran_rewards.items_claimed");
	std::string const ms_featureIdTransactionObjvar("feature_id_transaction");
	std::string const ms_tcgRedemptionInProgressObjvar("tcgRedemptionInProgress");

	std::string const ms_tradeInInProgressObjvar("tradeInInProgress");

	std::string const TRADE_IN_SCRIPT("systems.veteran_reward.trade_in");

	bool ms_installed = false;

	class RewardItem
	{
	public:
		RewardItem(DataTable const & dataTable, int row);

		std::string const & getId() const;
		StringId const & getDescription() const;
		std::string const & getObjectTemplateName() const;
		bool getOncePerAccount() const;
		bool getOncePerAccountFeatureId() const;
		bool getOncePerCharacter() const;
		bool getCanTradeIn() const;
		bool grantToPlayer(CreatureObject & playerCreature, uint32 accountFeatureId, int previousAccountFeatureIdCount, int currentAccountFeatureIdCount, bool regrant = false) const;
		int getMoney() const;
		
	private:
		std::string const m_id;
		StringId const m_description;
		std::string const m_objectTemplateName;
		int const m_money;
		bool const m_oncePerAccount;
		bool const m_oncePerAccountFeatureId;
		bool const m_oncePerCharacter;
		bool const m_canTradeIn;
		//TODO: replacable items

	private:  //disable:
		RewardItem();  
		RewardItem(RewardItem const &);
		RewardItem & operator=(RewardItem const &);
	};

	class RewardEvent
	{
	public:
		RewardEvent(DataTable const & dataTable, int row);

	public:
		std::string const & getId() const;
		StringId const & getAnnouncement() const;
		StringId const & getDescription() const;
		std::string const & getUrl() const;
		std::vector<std::pair<RewardItem const *, RewardEvent const *> > const & getRewardItems() const;
		int getCategory() const;
		int getFeatureBitRewardExclusionMask() const;
		bool hasPlayerTriggered(CreatureObject const & playerCreature) const;
		bool getOncePerAccount() const;
		uint32 getAccountFeatureId() const;
		bool getConsumeAccountFeatureId() const;
			
	private:
		int const m_rowId;
		std::string const m_id;
		std::vector<std::string> const m_specificItems;
		std::vector<std::string> const m_includeItemsFrom;
		std::vector<std::pair<RewardItem const *, RewardEvent const *> > mutable * m_allItems;
		int const m_category;
		int const m_featureBitRewardExclusionMask;
		uint32 const m_accountFlags;
		uint32 const m_accountFeatureId;
		bool const m_consumeAccountFeatureId;
		unsigned int const m_minAccountAge;
		int const m_minCharacterAge;
		std::string const m_characterObjvar;
		bool const m_oncePerAccount;
		StringId const m_annoucement;
		StringId const m_description;
		std::string const m_url;
		time_t const m_startDate;
		time_t const m_endDate;
		int const m_minBuddyPoints;
		int const m_featureIdBlockedBySubscriptionBit;

	private:  //disable:
		RewardEvent();  
		RewardEvent(RewardEvent const &); 
		RewardEvent & operator=(RewardEvent const &);		
	};

	typedef std::map<std::string, RewardItem *> RewardItemsType;
	typedef std::map<std::string, RewardEvent *> RewardEventsType;
	typedef std::vector<RewardEvent *> OrderedRewardEventsType;
	typedef std::map<uint32, std::pair<std::string, uint32> > CanTradeInRewardTemplateItemType;
	typedef std::map<std::string, std::pair<std::string, uint32> > CanTradeInRewardStaticItemType;
	typedef std::set<uint32> CanTradeInRewardFeatureIdType;
	typedef std::map<uint32, uint32> FeatureIdBlockedBySubscriptionBitMap;

	RewardItemsType ms_rewardItems;
	RewardEventsType ms_rewardEvents;
	OrderedRewardEventsType ms_orderedRewardEvents;
	CanTradeInRewardTemplateItemType ms_canTradeInRewardTemplateItem;
	CanTradeInRewardStaticItemType ms_canTradeInRewardStaticItem;
	CanTradeInRewardFeatureIdType ms_canTradeInRewardFeatureId;
	FeatureIdBlockedBySubscriptionBitMap ms_featureIdBlockedBySubscriptionBits;
	int ms_overrideAccountAge=0;

	RewardEvent * getRewardEventByName(std::string const & eventName);
	RewardItem  * getRewardItemByName(std::string const & itemName);
	
	time_t yyyymmddToTime(int const yyyy, int const mm, int const dd);
	std::vector<std::string> buildVectorFromString(std::string packedString);
}

using namespace VeteranRewardManagerNamespace;

// ======================================================================

void VeteranRewardManager::install()
{
	DEBUG_FATAL(ms_installed,("Programmer bug:  VeteranRewardManager::install() called twice"));
	ms_installed = true;
	ExitChain::add(VeteranRewardManager::remove, "VeteranRewardManager::remove");

	if (ConfigServerGame::getEnableNewVeteranRewards())
	{
		{
			DataTable * data = NON_NULL(DataTableManager::getTable(ms_rewardItemsTableName, true));
			int const numRows = data->getNumRows();
			for (int row=0; row<numRows; ++row)
			{
				RewardItem * newItem = new RewardItem(*data,row);		
				std::pair<RewardItemsType::iterator, bool> result = ms_rewardItems.insert(std::make_pair(newItem->getId(), newItem));
				FATAL(!result.second,("Data bug:  Reward item %s was specified more than once in data table %s",newItem->getId().c_str(), ms_rewardItemsTableName.c_str()));		
			}

			for (RewardItemsType::const_iterator iterRewardItem = ms_rewardItems.begin(); iterRewardItem != ms_rewardItems.end(); ++iterRewardItem)
			{
				std::string const & objectTemplateName = iterRewardItem->second->getObjectTemplateName();
				uint32 templateItemCrc = 0;
				if (objectTemplateName.find(".iff") != std::string::npos)
					templateItemCrc = CrcLowerString::calculateCrc(objectTemplateName.c_str());

				if (templateItemCrc > 0)
				{
					CanTradeInRewardTemplateItemType::const_iterator const iterFind = ms_canTradeInRewardTemplateItem.find(templateItemCrc);
					if (iterFind != ms_canTradeInRewardTemplateItem.end())
						FATAL((iterFind->second.first != iterRewardItem->second->getId()), ("Data bug:  \"can trade in\" reward object template (%s) can only be defined as the reward object for 1 reward item", objectTemplateName.c_str()));
				}
				else
				{
					CanTradeInRewardStaticItemType::const_iterator const iterFind = ms_canTradeInRewardStaticItem.find(objectTemplateName);
					if (iterFind != ms_canTradeInRewardStaticItem.end())
						FATAL((iterFind->second.first != iterRewardItem->second->getId()), ("Data bug:  \"can trade in\" reward object template (%s) can only be defined as the reward object for 1 reward item", objectTemplateName.c_str()));
				}
			}
		}

		{
			DataTable * data = NON_NULL(DataTableManager::getTable(ms_rewardEventsTableName, true));
			int const numRows = data->getNumRows();
			for (int row=0; row<numRows; ++row)
			{
				RewardEvent * newEvent = new RewardEvent(*data,row);
				std::pair<RewardEventsType::iterator, bool> result = ms_rewardEvents.insert(std::make_pair(newEvent->getId(), newEvent));
				FATAL(!result.second,("Data bug:  Reward event %s was specified more than once in data table %s",newEvent->getId().c_str(), ms_rewardEventsTableName.c_str()));
				ms_orderedRewardEvents.push_back(newEvent);
			}
		}

		// sanity check
		for (OrderedRewardEventsType::const_iterator iterEvent = ms_orderedRewardEvents.begin(); iterEvent != ms_orderedRewardEvents.end(); ++iterEvent)
		{
			std::vector<std::pair<RewardItem const *, RewardEvent const *> > const & items = (*iterEvent)->getRewardItems();
			for (std::vector<std::pair<RewardItem const *, RewardEvent const *> >::const_iterator item = items.begin(); item != items.end(); ++item)
			{
				FATAL((item->first->getOncePerAccount() && ((*iterEvent)->getAccountFeatureId() > 0) && (*iterEvent)->getConsumeAccountFeatureId()),("Data bug:  veteran reward event name \"%s\" is a \"consuming\" feature Id event, but includes \"once per account\" item \"%s\" which is not allowed.", (*iterEvent)->getId().c_str(), item->first->getId().c_str()));
				FATAL(((item->first->getMoney() > 0) && ((*iterEvent)->getAccountFeatureId() > 0) && (*iterEvent)->getConsumeAccountFeatureId()),("Data bug:  veteran reward event name \"%s\" is a \"consuming\" feature Id event, but includes item \"%s\" which grants credits, which is not allowed.", (*iterEvent)->getId().c_str(), item->first->getId().c_str()));

				std::string const & objectTemplateName = item->first->getObjectTemplateName();
				uint32 templateItemCrc = 0;
				if (objectTemplateName.find(".iff") != std::string::npos)
					templateItemCrc = CrcLowerString::calculateCrc(objectTemplateName.c_str());

				if (templateItemCrc > 0)
				{
					CanTradeInRewardTemplateItemType::iterator const iterFind = ms_canTradeInRewardTemplateItem.find(templateItemCrc);
					if (iterFind != ms_canTradeInRewardTemplateItem.end())
					{
						FATAL((iterFind->second.second > 0), ("Data bug:  \"can trade in\" reward object template (%s) can only be defined as the reward object for 1 reward event", objectTemplateName.c_str()));
						FATAL((*iterEvent)->getOncePerAccount(), ("Data bug:  \"can trade in\" reward object template (%s) can not be defined as the reward object for a \"once per account\" reward event", objectTemplateName.c_str()));
						FATAL(((*iterEvent)->getAccountFeatureId() == 0), ("Data bug:  \"can trade in\" reward object template (%s) can only be defined as the reward object for a reward event that requires an account feature id", objectTemplateName.c_str()));
						FATAL(!(*iterEvent)->getConsumeAccountFeatureId(), ("Data bug:  \"can trade in\" reward object template (%s) can only be defined as the reward object for a reward event that consumes an account feature id", objectTemplateName.c_str()));

						iterFind->second.second = (*iterEvent)->getAccountFeatureId();
					}
				}
				else
				{
					CanTradeInRewardStaticItemType::iterator const iterFind = ms_canTradeInRewardStaticItem.find(objectTemplateName);
					if (iterFind != ms_canTradeInRewardStaticItem.end())
					{
						FATAL((iterFind->second.second > 0), ("Data bug:  \"can trade in\" reward object template (%s) can only be defined as the reward object for 1 reward event", objectTemplateName.c_str()));
						FATAL((*iterEvent)->getOncePerAccount(), ("Data bug:  \"can trade in\" reward object template (%s) can not be defined as the reward object for a \"once per account\" reward event", objectTemplateName.c_str()));
						FATAL(((*iterEvent)->getAccountFeatureId() == 0), ("Data bug:  \"can trade in\" reward object template (%s) can only be defined as the reward object for a reward event that requires an account feature id", objectTemplateName.c_str()));
						FATAL(!(*iterEvent)->getConsumeAccountFeatureId(), ("Data bug:  \"can trade in\" reward object template (%s) can only be defined as the reward object for a reward event that consumes an account feature id", objectTemplateName.c_str()));

						iterFind->second.second = (*iterEvent)->getAccountFeatureId();
					}
				}
			}
		}

		// more sanity check
		{
			for (CanTradeInRewardTemplateItemType::const_iterator iter = ms_canTradeInRewardTemplateItem.begin(); iter != ms_canTradeInRewardTemplateItem.end(); ++iter)
			{
				FATAL((iter->second.second == 0), ("Data bug:  \"can trade in\" reward item (%s) is not associated with any reward event", iter->second.first.c_str()));
				ms_canTradeInRewardFeatureId.insert(iter->second.second);
			}
		}

		{
			for (CanTradeInRewardStaticItemType::const_iterator iter = ms_canTradeInRewardStaticItem.begin(); iter != ms_canTradeInRewardStaticItem.end(); ++iter)
			{
				FATAL((iter->second.second == 0), ("Data bug:  \"can trade in\" reward item (%s) is not associated with any reward event", iter->second.first.c_str()));
				ms_canTradeInRewardFeatureId.insert(iter->second.second);
			}
		}
	}
}

// ----------------------------------------------------------------------

void VeteranRewardManager::remove()
{
	for (RewardItemsType::iterator i=ms_rewardItems.begin(); i!=ms_rewardItems.end(); ++i)
		delete i->second;
	for (RewardEventsType::iterator j=ms_rewardEvents.begin(); j!=ms_rewardEvents.end(); ++j)
		delete j->second;

	ms_rewardItems.clear();
	ms_rewardEvents.clear();
	ms_orderedRewardEvents.clear();
	
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));
	ms_installed = false;
}

// ----------------------------------------------------------------------

RewardEvent * VeteranRewardManagerNamespace::getRewardEventByName(std::string const & eventName)
{
	RewardEventsType::iterator i=ms_rewardEvents.find(eventName);
	if (i!=ms_rewardEvents.end())
		return i->second;
	else
		return nullptr;
}

// ----------------------------------------------------------------------

RewardItem * VeteranRewardManagerNamespace::getRewardItemByName(std::string const & itemName)
{
	RewardItemsType::iterator i=ms_rewardItems.find(itemName);
	if (i!=ms_rewardItems.end())
		return i->second;
	else
		return nullptr;
}

// ----------------------------------------------------------------------

/**
 * Returns the IDs of all the reward events that the player qualifies for;
 * for multiple reward events in the same category, only return the first ID,
 * preceded by * to indicate that there are more rewards in that category
 * that the player qualifies for
 */
void VeteranRewardManager::getTriggeredEventsIds(CreatureObject const & playerCreature, std::vector<std::string> & eventsIds)
{
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));

	eventsIds.clear();

	if (!ConfigServerGame::getEnableNewVeteranRewards())
		return;

	// Get account information
	Client const * const client = playerCreature.getClient();
	if (!playerCreature.isAuthoritative() || !client)
	{
		DEBUG_FATAL(true,("Programmer bug:  called getTriggeredEventsIds() with object %s, which isn't authoritative or doesn't have a client.", playerCreature.getNetworkId().getValueString().c_str()));
		return;
	}

	if (client->isUsingAdminLogin())
		return;

	// Get list of reward events that have already been consumed or are being ignored
	std::vector<std::string> consumedEventsString;
	getConsumedEvents(playerCreature,consumedEventsString);
	std::set<RewardEvent const *> consumedEvents;
	{
		for (std::vector<std::string>::const_iterator i=consumedEventsString.begin(); i!=consumedEventsString.end(); ++i)
		{
			RewardEvent const * const event = getRewardEventByName(*i);
			if (event)
				IGNORE_RETURN(consumedEvents.insert(event));
		}
	}

	// Check for reward events -- assumes reward events are ordered in the order we want to present them to the player
	// (Probably least valuable to most valuable is the best order, so that players don't use a more valuable event to
	// claim a less valuable prize, not realizing that they also had achieved a less valuable event they could use for that prize.)
	std::map<int, int> triggeredEventCategories;
	std::string triggeredEvents;
	{
		for (OrderedRewardEventsType::const_iterator i=ms_orderedRewardEvents.begin(); i!=ms_orderedRewardEvents.end(); ++i)
		{
			if (consumedEvents.find(*i)==consumedEvents.end())
			{
				if ((*i)->hasPlayerTriggered(playerCreature))
				{
					// an event that consumes an account feature Id is only
					// triggered if there is an item in the event that can
					// be claimed by this character; for example, if all the
					// items in the event are once per character, and the
					// character has already claimed all the items, then the
					// event should not be triggered, since there is no item
					// in the event that this character can claim
					if (((*i)->getAccountFeatureId() > 0) && (*i)->getConsumeAccountFeatureId())
					{
						std::vector<std::string> rewardItems;
						getAvailableRewardItemsForEvent(playerCreature, (*i)->getId(), rewardItems, nullptr);

						if (rewardItems.empty())
							continue;
					}

					if (!triggeredEvents.empty())
						triggeredEvents += ", ";

					triggeredEvents += (*i)->getId();

					int const eventCategory = (*i)->getCategory();
					if (eventCategory != 0)
					{
						std::map<int, int>::const_iterator iterFind = triggeredEventCategories.find(eventCategory);
						if (iterFind == triggeredEventCategories.end())
						{
							triggeredEventCategories[eventCategory] = eventsIds.size();
							eventsIds.push_back((*i)->getId());
						}
						else if (eventsIds[iterFind->second][0] != '*')
						{
							eventsIds[iterFind->second] = std::string("*") + eventsIds[iterFind->second];
						}
					}
					else
					{
						eventsIds.push_back((*i)->getId());
					}
				}
			}
		}
	}

	if (!triggeredEvents.empty())
		LOG("CustomerService",("VeteranRewards:  %s has triggered reward events (%s)", PlayerObject::getAccountDescription(&playerCreature).c_str(),triggeredEvents.c_str()));
}

// ----------------------------------------------------------------------

/**
 * Get the list of reward items a player can claim using the specified event.
 * Checks for unique items, etc.
 * Returns a vector of the IDs of the possible items and a vector of the
 * StringIds of the descriptions of these items.
 */

void VeteranRewardManager::getAvailableRewardItemsForEvent(CreatureObject const & playerCreature, std::string const & eventName, std::vector<std::string> & rewardItems, std::vector<std::string> * rewardDescriptions)
{
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));

	RewardEvent const * const event = getRewardEventByName(eventName);
	if (!event)
		return;
	
	// Get list of reward items that have already been claimed
	std::vector<std::string> claimedItemsString;
	getClaimedItems(playerCreature,claimedItemsString);
	std::set<RewardItem const *> claimedItems;
	for (std::vector<std::string>::const_iterator i=claimedItemsString.begin(); i!=claimedItemsString.end(); ++i)
	{
		RewardItem const * const item = getRewardItemByName(*i);
		// it's possible that an item might be changed from once-per-account/cluster to not being once-per-account/cluster, so re-check the flags on the items here
		if (item && (item->getOncePerAccount() || item->getOncePerCharacter()))
			IGNORE_RETURN(claimedItems.insert(item));
	}

	std::vector<std::pair<RewardItem const *, RewardEvent const *> > const & items = event->getRewardItems();
	for (std::vector<std::pair<RewardItem const *, RewardEvent const *> >::const_iterator item = items.begin(); item != items.end(); ++item)
	{
		if (claimedItems.find(item->first)==claimedItems.end())
		{
			rewardItems.push_back(item->first->getId());
			if (rewardDescriptions)
			{
				std::string description = std::string("@");
				description += item->first->getDescription().getCanonicalRepresentation();
				if (item->second && (item->second != event))
				{
					description += " @veteran_new:color_lime_green - from @";
					description += item->second->getAnnouncement().getCanonicalRepresentation();
				}

				rewardDescriptions->push_back(description);
			}
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Start the process of claiming a list of rewards
 * @return true if the request is OK, will be sent to login server for verification if necessary
 * false if there is a problem with the request
 */
bool VeteranRewardManager::claimRewards(CreatureObject const & playerCreature, std::string const & rewardEventName, std::string const & rewardItem, std::string * debugMessage)
{
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));

	if (!ConfigServerGame::getEnableNewVeteranRewards())
		return false;

	Client * const client = playerCreature.getClient();
	if (!client)
		return false;
	RewardEvent const * const event = getRewardEventByName(rewardEventName);
	if (!event)
	{
		if (debugMessage)
			(*debugMessage) = "No such event";
		return false;
	}

	// check to make sure that event can be claimed
	if (!event->hasPlayerTriggered(playerCreature))
	{
		if (debugMessage)
			(*debugMessage) = "Event not triggered";
		return false;
	}

	std::vector<std::string> possibleRewardItems;
	getAvailableRewardItemsForEvent(playerCreature, rewardEventName, possibleRewardItems, nullptr);
	if (possibleRewardItems.empty())
	{
		if (debugMessage)
			(*debugMessage) = "There isn't any possible reward item left for this event";
		return false;
	}

	// Check that the requested item can be claimed with this event
	bool accountUniqueItem = false;
	bool found = false;

	for (std::vector<std::string>::const_iterator j=possibleRewardItems.begin(); j!=possibleRewardItems.end(); ++j)
	{
		if (rewardItem == *j)
		{
			found = true;
			RewardItem const * const item = getRewardItemByName(rewardItem);
			if (!item)
			{
				LOG("CustomerService",("VeteranRewards:  %s has requested item %s (claimed as part of event %s), but that item could not be found.", PlayerObject::getAccountDescription(&playerCreature).c_str(), rewardItem.c_str(), rewardEventName.c_str()));

				if (debugMessage)
					(*debugMessage) = "No such item:  " + rewardItem;

				return false;
			}
			LOG("CustomerService",("VeteranRewards:  %s has requested item %s (claimed as part of event %s)", PlayerObject::getAccountDescription(&playerCreature).c_str(), rewardItem.c_str(), rewardEventName.c_str()));

			if (item->getOncePerAccount())
				accountUniqueItem = true;

			break;
		}
	}

	if (!found)
	{
		LOG("CustomerService",("VeteranRewards:  %s has requested item %s (claimed as part of event %s), but that item was not a legitimate choice for this event.", PlayerObject::getAccountDescription(&playerCreature).c_str(), rewardItem.c_str(), rewardEventName.c_str()));
		if (debugMessage)
			(*debugMessage) = "Item is not a legitimate choice for this event (or has already been claimed):  " + rewardItem;
		return false;
	}

	if (event->getOncePerAccount() || accountUniqueItem || (event->getAccountFeatureId() > 0))
	{
		ClaimRewardsMessage const msg(GameServer::getInstance().getProcessId(), client->getStationId(), playerCreature.getNetworkId(), rewardEventName, event->getOncePerAccount(), rewardItem, accountUniqueItem, event->getAccountFeatureId(), event->getConsumeAccountFeatureId());

		// reward requiring account feature id needs to go to the ConnectionServer for validation
		// with session/Platform that the account has the required account feature id
		if (event->getAccountFeatureId() > 0)
		{
			client->sendToConnectionServer(msg);
		}
		else
		{
			GameServer::getInstance().sendToCentralServer(msg);
		}
	}
	else
	{
		//skip sending message to login, because it isn't needed.  Just grant the items right away.
		//(Because no items are unique, stationId does't matter, pass 0 for it.)
		handleClaimRewardsReply(0, playerCreature.getNetworkId(), rewardEventName, rewardItem, 0, false, 0, 0, true, false);
	}
	return true;
}

// ----------------------------------------------------------------------

void VeteranRewardManager::handleClaimRewardsReply(StationId stationId, NetworkId const & player, std::string const & rewardEvent, std::string const & rewardItem, uint32 accountFeatureId, bool consumeAccountFeatureId, int previousAccountFeatureIdCount, int currentAccountFeatureIdCount, bool result, bool verify)
{
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));

	static std::vector<int8> const emptyMessageData;
		 
	ServerObject * const so = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(player));
	CreatureObject * const playerCreature = so ? so->asCreatureObject() : nullptr;
	if (!playerCreature)
		return;	// player has vanished -- reward claim will be handled by the recovery code at the next login
	if (!playerCreature->isAuthoritative())
	{
		uint32 authServer = playerCreature->getAuthServerProcessId();
		ClaimRewardsReplyMessage msg(authServer, stationId, player, rewardEvent, rewardItem, accountFeatureId, consumeAccountFeatureId, previousAccountFeatureIdCount, currentAccountFeatureIdCount, result);
		ServerMessageForwarding::begin(authServer);
		ServerMessageForwarding::send(msg);
		ServerMessageForwarding::end();
		return;
	}

	if (result)
	{
		// one final check to make sure player is qualified to receive the item; don't
		// need to do it if it's a feature id reward, since the verification has
		// already been done by the LoginServer, which has access to up-to-date feature
		// id information, whereas we only have a cache of the feature id information
		if (verify && (accountFeatureId == 0))
		{
			RewardEvent const * const event = getRewardEventByName(rewardEvent);
			if (!event)
			{
				LOG("CustomerService",("VeteranRewards:  %s's reward claim failed - could not find event", PlayerObject::getAccountDescription(playerCreature).c_str()));
				MessageToQueue::getInstance().sendMessageToJava(player, "veteranItemGrantFailed",emptyMessageData,0,false);
				return;
			}

			// check to make sure that event can be claimed
			if (!event->hasPlayerTriggered(*playerCreature))
			{
				LOG("CustomerService",("VeteranRewards:  %s's reward claim failed - event not triggered", PlayerObject::getAccountDescription(playerCreature).c_str()));
				MessageToQueue::getInstance().sendMessageToJava(player, "veteranItemGrantFailed",emptyMessageData,0,false);
				return;
			}

			// Check that the requested item can be claimed with this event
			std::vector<std::string> possibleRewardItems;
			getAvailableRewardItemsForEvent(*playerCreature, rewardEvent, possibleRewardItems, nullptr);
			bool found = false;

			for (std::vector<std::string>::const_iterator j=possibleRewardItems.begin(); j!=possibleRewardItems.end(); ++j)
			{
				if (rewardItem == *j)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				LOG("CustomerService",("VeteranRewards:  %s's reward claim failed - reward item not available for event", PlayerObject::getAccountDescription(playerCreature).c_str()));
				MessageToQueue::getInstance().sendMessageToJava(player, "veteranItemGrantFailed",emptyMessageData,0,false);
				return;
			}
		}

		// if the claim consumed an account feature id, send request for updated account feature id info
		if ((accountFeatureId > 0) && consumeAccountFeatureId)
		{
			Client * const client = playerCreature->getClient();
			if (client)
			{
				// until we receive the updated account feature id info,
				// locally decrement the count on the account feature id
				// so that it is in sync with what's on the account
				Client::AccountFeatureIdList const & accountFeatureIds = client->getAccountFeatureIds();
				Client::AccountFeatureIdList::const_iterator iterFeatureID = accountFeatureIds.find(accountFeatureId);
				if (iterFeatureID != accountFeatureIds.end())
					client->setAccountFeatureId(accountFeatureId, std::max(0, currentAccountFeatureIdCount));

				AccountFeatureIdRequest const req(NetworkId::cms_invalid, GameServer::getInstance().getProcessId(), player, stationId, PlatformGameCode::SWG, AccountFeatureIdRequest::RR_Reload);
				client->sendToConnectionServer(req);
			}
		}

		// Attempt to create the item
		bool itemCreationSuccess = false;
		RewardItem const * const item = getRewardItemByName(rewardItem);
		if (item)
		{
			itemCreationSuccess = item->grantToPlayer(*playerCreature, accountFeatureId, previousAccountFeatureIdCount, currentAccountFeatureIdCount);
		}

		if (itemCreationSuccess)
		{
			// Record that the event has been used, except for event that uses "non-consuming"
			// account Feature Id, as those events can be claimed multiple times
			if ((accountFeatureId == 0) || !consumeAccountFeatureId)
			{
				IGNORE_RETURN(playerCreature->setObjVarItem(ms_consumedEventsObjvar + "." + rewardEvent, static_cast<int>(1)));
			}
			else
			{
				// record the transaction, which will be compared with the transaction
				// recorded in the LoginServer DB, so that if the cluster crashes, and there
				// is a rollback, there will be a discrepancy, and we can re-grant the item
				std::string const featureIdTransactionObjvar = ms_featureIdTransactionObjvar + "." + rewardItem;
				int count;
				if (playerCreature->getObjVars().getItem(featureIdTransactionObjvar, count))
					++count;
				else
					count = 1;

				playerCreature->setObjVarItem(featureIdTransactionObjvar, count);
			}

			LOG("CustomerService",("VeteranRewards:  %s's reward claim completed", PlayerObject::getAccountDescription(playerCreature).c_str()));

			ScriptParams params;
			params.addParam(item->getCanTradeIn(), "canTradeIn");
			ScriptDictionaryPtr dictionary;
			GameScriptObject::makeScriptDictionary(params, dictionary);
			if (dictionary.get() != nullptr)
			{
				dictionary->serialize();
				MessageToQueue::getInstance().sendMessageToJava(player, "veteranItemGrantSucceeded",dictionary->getSerializedData(),0,false);
			}
			else
			{
				MessageToQueue::getInstance().sendMessageToJava(player, "veteranItemGrantSucceeded",emptyMessageData,0,false);
			}
		}
		else
		{
			LOG("CustomerService",("VeteranRewards:  %s's reward claim failed", PlayerObject::getAccountDescription(playerCreature).c_str()));
			MessageToQueue::getInstance().sendMessageToJava(player, "veteranItemGrantFailed",emptyMessageData,0,false);
		}
	}
	else
	{
		LOG("CustomerService",("VeteranRewards:  %s's reward claim was denied by the login server", PlayerObject::getAccountDescription(playerCreature).c_str()));
		MessageToQueue::getInstance().sendMessageToJava(player, "veteranItemGrantFailed",emptyMessageData,0,false);
	}
}

// ----------------------------------------------------------------------

/**
 * Combine the data sent from the login server, listing rewards claimed on
 * other clusters, with the local data.  Write the data to objvars on the
 * player, so that other functions can use this data.
 */
void VeteranRewardManager::writeAccountDataToObjvars(CreatureObject & playerCreature)
{
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));

	if (!ConfigServerGame::getEnableNewVeteranRewards())
		return;

	if (!playerCreature.isAuthoritative())
		return;	
	Client * const client = playerCreature.getClient();
	if (!client)
		return;
	
	{
		std::string consumedEventObjvar;
		std::vector<std::pair<NetworkId, std::string> > const & accountConsumedEvents = client->getConsumedRewardEvents();
		for (std::vector<std::pair<NetworkId, std::string> >::const_iterator i=accountConsumedEvents.begin(); i!=accountConsumedEvents.end(); ++i)
		{
			consumedEventObjvar = ms_consumedEventsObjvar + "." + i->second;

			if (i->first == playerCreature.getNetworkId())
			{
				if (!playerCreature.getObjVars().hasItem(consumedEventObjvar))
				{
					// Login server thinks event was claimed by this character, but we have no record of it.  Allow it to be re-claimed by not inserting
					// it into the list in the objvar.
					if (getRewardEventByName(i->second))
						LOG("CustomerService",("VeteranRewards:  %s was flagged as claiming event %s in the login database, but the local cluster has no record of it.  Reward can be re-claimed.", PlayerObject::getAccountDescription(&playerCreature).c_str(), i->second.c_str()));
				}
			}
			else if (!playerCreature.getObjVars().hasItem(consumedEventObjvar))
			{
				// Add events claimed on other clusters to the list, so that we don't ask the player about them, etc.
				IGNORE_RETURN(playerCreature.setObjVarItem(consumedEventObjvar, static_cast<int>(1)));
			}
		}
	}

	{
		std::string claimedItemObjvar;
		std::vector<std::pair<NetworkId, std::string> > const & accountClaimedItems = client->getClaimedRewardItems();
		for (std::vector<std::pair<NetworkId, std::string> >::const_iterator i=accountClaimedItems.begin(); i!=accountClaimedItems.end(); ++i)
		{
			claimedItemObjvar = ms_claimedItemsObjvar + "." + i->second;

			if (i->first==playerCreature.getNetworkId())
			{
				if (!playerCreature.getObjVars().hasItem(claimedItemObjvar))
				{
					// Login server thinks item was claimed by this character, but we have no record of it.  Allow it to be re-claimed.
					if (getRewardItemByName(i->second))
						LOG("CustomerService",("VeteranRewards:  %s was flagged as claiming item %s in the login database, but the local cluster has no record of it.  Reward can be re-claimed.", PlayerObject::getAccountDescription(&playerCreature).c_str(), i->second.c_str()));
				}
			}
			else if (!playerCreature.getObjVars().hasItem(claimedItemObjvar))
			{
				// Add items claimed on other clusters to the list, so that we don't ask the player about them, etc.
				IGNORE_RETURN(playerCreature.setObjVarItem(claimedItemObjvar, static_cast<int>(1)));
			}
		}
	}
}

// ----------------------------------------------------------------------

time_t VeteranRewardManagerNamespace::yyyymmddToTime(int const yyyy, int const mm, int const dd)
{
	if ((yyyy == 0) && (mm == 0) && (dd == 0))
		return 0;

	FATAL(((yyyy < 1975) || (yyyy > 2035)),("Data bug:  Reward event date (%d / %d / %d) - year %d specified for a reward event must be between 1975 - 2035.",mm,dd,yyyy,yyyy));
	FATAL(((mm < 1) || (mm > 12)),("Data bug:  Reward event date (%d / %d / %d) - month %d specified for a reward event must be between 1 - 12.",mm,dd,yyyy,mm));
	FATAL(((dd < 1) || (dd > 31)),("Data bug:  Reward event date (%d / %d / %d) - day %d specified for a reward event must be between 1 - 31.",mm,dd,yyyy,dd));

	time_t const rawtime = ::time(nullptr);
	struct tm * const timeinfo = ::localtime(&rawtime);
	if (!timeinfo)
	{
		FATAL(true,(":localtime() returns nullptr"));
		return 0;
	}

	timeinfo->tm_year = yyyy - 1900;
	timeinfo->tm_mon = mm - 1;
	timeinfo->tm_mday = dd;
	timeinfo->tm_hour = 0;
	timeinfo->tm_min = 0;
	timeinfo->tm_sec = 0;

	time_t const retval = ::mktime(timeinfo);

	FATAL(retval==-1,("Could not convert tm to time_t"));
	return retval;
}

// ----------------------------------------------------------------------

void VeteranRewardManager::getRewardChoicesDescriptions(CreatureObject const & playerCreature, std::string const & eventName, std::vector<std::string> & rewardDescriptions)
{
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));
	
	std::vector<std::string> rewardItems;
	rewardDescriptions.clear();
	getAvailableRewardItemsForEvent(playerCreature, eventName, rewardItems, &rewardDescriptions);
}

// ----------------------------------------------------------------------

void VeteranRewardManager::getRewardChoicesTags(CreatureObject const & playerCreature, std::string const & eventName, std::vector<Unicode::String> & rewardTagsUnicode)
{
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));

	std::vector<std::string> rewardItems;

	getAvailableRewardItemsForEvent(playerCreature, eventName, rewardItems, nullptr);
	for (std::vector<std::string>::const_iterator i=rewardItems.begin(); i!=rewardItems.end(); ++i)
	{
		rewardTagsUnicode.push_back(Unicode::narrowToWide(*i));
	}
}

// ----------------------------------------------------------------------

StringId const * VeteranRewardManager::getEventAnnouncement(std::string const & eventName)
{
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));
	
	RewardEvent const * const event = getRewardEventByName(eventName);
	if (event)
	{
		return &(event->getAnnouncement());
	}
	return nullptr;
}

// ----------------------------------------------------------------------

StringId const * VeteranRewardManager::getEventDescription(std::string const & eventName)
{
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));
	
	RewardEvent const * const event = getRewardEventByName(eventName);
	if (event)
	{
		return &(event->getDescription());
	}
	return nullptr;
}

// ----------------------------------------------------------------------

std::string const * VeteranRewardManager::getEventUrl(std::string const & eventName)
{
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));
	
	RewardEvent const * const event = getRewardEventByName(eventName);
	if (event)
	{
		return &(event->getUrl());
	}
	return nullptr;
}

// ----------------------------------------------------------------------

bool VeteranRewardManager::isEventAccountUnique(std::string const & eventName)
{
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));
	
	RewardEvent const * const event = getRewardEventByName(eventName);
	if (event)
	{
		return (event->getOncePerAccount());
	}
	return false;
}

// ----------------------------------------------------------------------

bool VeteranRewardManager::isItemAccountUnique(std::string const & itemName)
{
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));
	
	RewardItem const * const item = getRewardItemByName(itemName);
	if (item)
	{
		return (item->getOncePerAccount());
	}
	return false;
}

// ----------------------------------------------------------------------

bool VeteranRewardManager::isItemAccountUniqueFeatureId(std::string const & itemName)
{
	DEBUG_FATAL(!ms_installed,("Programmer bug:  VeteranRewardManager was not installed"));

	RewardItem const * const item = getRewardItemByName(itemName);
	if (item)
	{
		return (item->getOncePerAccountFeatureId());
	}
	return false;
}

// ----------------------------------------------------------------------

void VeteranRewardManager::verifyFeatureIdTransactions(CreatureObject & playerCreature, std::map<std::string, int> const & transactionsFromLoginServer)
{
	// get the transaction list from the character
	std::map<std::string, int> transactionsFromCharacter;
	DynamicVariableList::NestedList const children(playerCreature.getObjVars(), ms_featureIdTransactionObjvar);
	for (DynamicVariableList::NestedList::const_iterator iterChildren = children.begin(); iterChildren != children.end(); ++iterChildren)
	{
		if (iterChildren.getType() == DynamicVariable::INT)
		{
			int count = 0;
			iterChildren.getValue(count);

			transactionsFromCharacter[iterChildren.getName()] = count;
		}
	}

	// if the transaction value on the character is > the transaction value on
	// the LoginServer, then update the LoginServer transaction value to match
	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(&playerCreature);
	if (playerObject)
	{
		for (std::map<std::string, int>::const_iterator iterTransactionCharacter = transactionsFromCharacter.begin(); iterTransactionCharacter != transactionsFromCharacter.end(); ++iterTransactionCharacter)
		{
			int transactionCountFromLoginServer = 0;
			std::map<std::string, int>::const_iterator iterTransactionLoginServer = transactionsFromLoginServer.find(iterTransactionCharacter->first);
			if (iterTransactionLoginServer != transactionsFromLoginServer.end())
				transactionCountFromLoginServer = iterTransactionLoginServer->second;

			if (iterTransactionCharacter->second > transactionCountFromLoginServer)
			{
				FeatureIdTransactionSyncUpdate const fitsu(playerObject->getStationId(), playerCreature.getNetworkId(), iterTransactionCharacter->first, (iterTransactionCharacter->second - transactionCountFromLoginServer));
				GameServer::getInstance().sendToCentralServer(fitsu);

				LOG("CustomerService",("FeatureIdTransaction: syncing LoginServer DB feature Id transaction for item (%s) from (%d) to (%d) for (%s)", iterTransactionCharacter->first.c_str(), transactionCountFromLoginServer, iterTransactionCharacter->second, playerObject->getAccountDescription().c_str()));
			}
		}
	}

	// if the transaction value on the character is < the transaction value on
	// the LoginServer, then grant the player the missing item(s) until the
	// transaction value match
	for (std::map<std::string, int>::const_iterator iterTransactionLoginServer = transactionsFromLoginServer.begin(); iterTransactionLoginServer != transactionsFromLoginServer.end(); ++iterTransactionLoginServer)
	{
		int transactionCountFromCharacter = 0;
		std::map<std::string, int>::const_iterator iterTransactionCharacter = transactionsFromCharacter.find(iterTransactionLoginServer->first);
		if (iterTransactionCharacter != transactionsFromCharacter.end())
			transactionCountFromCharacter = iterTransactionCharacter->second;

		if (iterTransactionLoginServer->second > transactionCountFromCharacter)
		{
			RewardItem const * const item = getRewardItemByName(iterTransactionLoginServer->first);
			if (item)
			{
				std::string const featureIdTransactionObjvar = ms_featureIdTransactionObjvar + "." + iterTransactionLoginServer->first;
				int adjustedCount = transactionCountFromCharacter;
				for (int i = transactionCountFromCharacter + 1; i <= iterTransactionLoginServer->second; ++i)
				{
					if (item->grantToPlayer(playerCreature, 0, 0, 0, true))
					{
						++adjustedCount;

						// record the transaction, which will be compared with the transaction
						// recorded in the LoginServer DB, so that if the cluster crashes, and there
						// is a rollback, there will be a discrepancy, and we can re-grant the item
						int count;
						if (playerCreature.getObjVars().getItem(featureIdTransactionObjvar, count))
							++count;
						else
							count = 1;

						playerCreature.setObjVarItem(featureIdTransactionObjvar, count);
					}
				}

				if (adjustedCount != transactionCountFromCharacter)
					LOG("CustomerService",("FeatureIdTransaction: syncing character feature Id transaction for item (%s) from (%d) to (%d) for (%s)", iterTransactionLoginServer->first.c_str(), transactionCountFromCharacter, adjustedCount, PlayerObject::getAccountDescription(&playerCreature).c_str()));
			}
		}
	}
}

// ----------------------------------------------------------------------

void VeteranRewardManager::migrateVeteranRewardsClaimInformation(CreatureObject & playerCreature)
{
	static std::string const oldConsumedEventsObjvar("veteran_rewards.consumed_events");
	if ((playerCreature.getObjVars().hasItem(oldConsumedEventsObjvar)) && (playerCreature.getObjVars().getType(oldConsumedEventsObjvar) == DynamicVariable::STRING_ARRAY))
	{
		std::vector<Unicode::String> consumedEventsUnicode; // objvars only support Unicode vectors, or else would use std::string here
		IGNORE_RETURN(playerCreature.getObjVars().getItem(oldConsumedEventsObjvar, consumedEventsUnicode));

		std::string consumedEventsString;
		if (!consumedEventsUnicode.empty())
		{
			std::string eventString;
			for (std::vector<Unicode::String>::const_iterator i = consumedEventsUnicode.begin(); i != consumedEventsUnicode.end(); ++i)
			{
				eventString = Unicode::wideToNarrow(*i);

				if (!consumedEventsString.empty())
					consumedEventsString += ", ";

				consumedEventsString += eventString;

				IGNORE_RETURN(playerCreature.setObjVarItem(ms_consumedEventsObjvar + "." + eventString, static_cast<int>(1)));
			}
		}

		LOG("CustomerService",("VeteranRewards: migrated %s objvar (%s) for %s.", oldConsumedEventsObjvar.c_str(), consumedEventsString.c_str(), PlayerObject::getAccountDescription(&playerCreature).c_str()));
		playerCreature.removeObjVarItem(oldConsumedEventsObjvar);
	}

	static std::string const oldClaimedItemsObjvar("veteran_rewards.claimed_items");
	if ((playerCreature.getObjVars().hasItem(oldClaimedItemsObjvar)) && (playerCreature.getObjVars().getType(oldClaimedItemsObjvar) == DynamicVariable::STRING_ARRAY))
	{
		std::vector<Unicode::String> claimedItemsUnicode; // objvars only support Unicode vectors, or else would use std::string here
		IGNORE_RETURN(playerCreature.getObjVars().getItem(oldClaimedItemsObjvar, claimedItemsUnicode));

		std::string claimedItemsString;
		if (!claimedItemsUnicode.empty())
		{
			std::string itemString;
			for (std::vector<Unicode::String>::const_iterator i = claimedItemsUnicode.begin(); i != claimedItemsUnicode.end(); ++i)
			{
				itemString = Unicode::wideToNarrow(*i);

				if (!claimedItemsString.empty())
					claimedItemsString += ", ";

				claimedItemsString += itemString;

				IGNORE_RETURN(playerCreature.setObjVarItem(ms_claimedItemsObjvar + "." + itemString, static_cast<int>(1)));
			}
		}

		LOG("CustomerService",("VeteranRewards: migrated %s objvar (%s) for %s.", oldClaimedItemsObjvar.c_str(), claimedItemsString.c_str(), PlayerObject::getAccountDescription(&playerCreature).c_str()));
		playerCreature.removeObjVarItem(oldClaimedItemsObjvar);
	}
}

// ----------------------------------------------------------------------

void VeteranRewardManager::getConsumedEvents(ServerObject const & playerCreature, std::vector<std::string> & consumedEvents)
{
	consumedEvents.clear();

	DynamicVariableList::NestedList const children(playerCreature.getObjVars(), ms_consumedEventsObjvar);
	for (DynamicVariableList::NestedList::const_iterator iterChildren = children.begin(); iterChildren != children.end(); ++iterChildren)
	{
		if (iterChildren.getType() == DynamicVariable::INT)
			consumedEvents.push_back(iterChildren.getName());
	}
}

// ----------------------------------------------------------------------

void VeteranRewardManager::getClaimedItems(ServerObject const & playerCreature, std::vector<std::string> & claimedItems)
{
	claimedItems.clear();

	DynamicVariableList::NestedList const children(playerCreature.getObjVars(), ms_claimedItemsObjvar);
	for (DynamicVariableList::NestedList::const_iterator iterChildren = children.begin(); iterChildren != children.end(); ++iterChildren)
	{
		if (iterChildren.getType() == DynamicVariable::INT)
			claimedItems.push_back(iterChildren.getName());
	}
}

// ----------------------------------------------------------------------

void VeteranRewardManager::tcgRedemption(CreatureObject const & playerCreature, ServerObject & item, int featureId, int adjustment)
{
	if (!playerCreature.isAuthoritative())
		return;

	PlayerObject const * const playerPlayer = PlayerCreatureController::getPlayerObject(&playerCreature);
	if (!playerPlayer)
		return;

	Client * const client = playerCreature.getClient();
	if (!client)
		return;

	if (!item.isAuthoritative())
		return;

	// item to be claimed must be in the player's top level inventory
	ServerObject const * const playerInventory = playerCreature.getInventory();
	if (!playerInventory)
		return;

	if (static_cast<void const *>(playerInventory) != static_cast<void const *>(ContainerInterface::getContainedByObject(item)))
	{
		Chat::sendSystemMessage(playerCreature, StringId("veteran_new", "redeem_tcg_card_not_in_inventory"), Unicode::emptyString);
		return;
	}

	// check to see if the object is already in the middle of a redemption
	if (!checkForTcgRedemptionInProgress(item))
		return;

	// set a "timed" objvar on the object which will be used to prevent the object
	// from being destroyed or transferred to a different container while we do
	// the redemption
	int const redemptionTimeout = 300; // 5 minutes;
	item.removeObjVarItem(ms_tcgRedemptionInProgressObjvar);
	item.setObjVarItem(ms_tcgRedemptionInProgressObjvar, static_cast<int>(::time(nullptr) + redemptionTimeout)); // 5 minutes

	// send off request to adjust the account feature Id
	AdjustAccountFeatureIdRequest const msg(NetworkId::cms_invalid, GameServer::getInstance().getProcessId(), playerCreature.getNetworkId(), PlayerObject::getAccountDescription(&playerCreature), playerPlayer->getStationId(), item.getNetworkId(), ServerObject::getLogDescription(&item), PlatformGameCode::SWGTCG, static_cast<uint32>(featureId), adjustment);
	client->sendToConnectionServer(msg);

	// let player know the redemption request has been sent
	ProsePackage prosePackage;
	prosePackage.stringId = StringId("veteran_new", "redeem_tcg_card_redemption_request_sent");
	prosePackage.target.str = Unicode::narrowToWide(CalendarTime::convertSecondsToMS(static_cast<unsigned int>(redemptionTimeout)));
	Chat::sendSystemMessage(playerCreature, prosePackage);
}

// ----------------------------------------------------------------------

bool VeteranRewardManager::checkForTcgRedemptionInProgress(ServerObject const & item)
{
	if (item.getObjVars().hasItem(ms_tcgRedemptionInProgressObjvar) && (item.getObjVars().getType(ms_tcgRedemptionInProgressObjvar) == DynamicVariable::INT))
	{
		int redemptionTimeout;
		IGNORE_RETURN(item.getObjVars().getItem(ms_tcgRedemptionInProgressObjvar, redemptionTimeout));

		time_t const currentTime = ::time(nullptr);
		if (redemptionTimeout > currentTime)
		{
			// item should be in top level inventory; get the player object the item is in
			ServerObject const * containingPlayer = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(item)); // this gets the player's inventory
			if (containingPlayer)
				containingPlayer = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*containingPlayer)); // this gets the player

			if (containingPlayer && containingPlayer->getClient())
			{
				ProsePackage prosePackage;
				prosePackage.stringId = StringId("veteran_new", "redeem_tcg_card_redemption_already_in_progress");
				prosePackage.target.str = Unicode::narrowToWide(CalendarTime::convertSecondsToMS(static_cast<unsigned int>(redemptionTimeout - currentTime)));
				Chat::sendSystemMessage(*containingPlayer, prosePackage);
			}

			// item is currently being redeemed
			return false;
		}
	}

	// item is not currently being redeemed
	return true;
}

// ----------------------------------------------------------------------

void VeteranRewardManager::handleTcgRedemptionResponse(ServerObject & item, AdjustAccountFeatureIdResponse const & response)
{
	if (response.getResultCode() == RESULT_SUCCESS)
	{
		// card has been successfully redeemed, destroy it
		item.permanentlyDestroy(DeleteReasons::Consumed);
	}
	else
	{
		// card has NOT been successfully redeemed, remove the
		// redemption lock on it, so the player can try again
		item.removeObjVarItem(ms_tcgRedemptionInProgressObjvar);
	}
}

// ----------------------------------------------------------------------

void VeteranRewardManager::attachTradeInScriptToItem(ServerObject & item)
{
	GameScriptObject * const script = item.getScriptObject();
	if (!script)
		return;

	std::string const & staticItemName = item.getStaticItemName();
	if (staticItemName.empty())
	{
		if ((ms_canTradeInRewardTemplateItem.count(item.getTemplateCrc()) > 0) && (!script->hasScript(TRADE_IN_SCRIPT)))
			script->attachScript(TRADE_IN_SCRIPT, false);
	}
	else if ((ms_canTradeInRewardStaticItem.count(staticItemName) > 0) && (!script->hasScript(TRADE_IN_SCRIPT)))
	{
		script->attachScript(TRADE_IN_SCRIPT, false);
	}
}

// ----------------------------------------------------------------------

bool VeteranRewardManager::tradeInReward(CreatureObject const & playerCreature, ServerObject & item, bool validateOnly)
{
	if (!playerCreature.isAuthoritative())
		return false;

	PlayerObject const * const playerPlayer = PlayerCreatureController::getPlayerObject(&playerCreature);
	if (!playerPlayer)
		return false;

	Client * const client = playerCreature.getClient();
	if (!client)
		return false;

	if (!item.isAuthoritative())
		return false;

	// verify that the item can be traded in
	uint32 featureId = 0;
	std::string const & staticItemName = item.getStaticItemName();
	if (staticItemName.empty())
	{
		CanTradeInRewardTemplateItemType::const_iterator const iterFind = ms_canTradeInRewardTemplateItem.find(item.getTemplateCrc());
		if (iterFind == ms_canTradeInRewardTemplateItem.end())
			return false;
		else
			featureId = iterFind->second.second;
	}
	else
	{
		CanTradeInRewardStaticItemType::const_iterator const iterFind = ms_canTradeInRewardStaticItem.find(staticItemName);
		if (iterFind == ms_canTradeInRewardStaticItem.end())
			return false;
		else
			featureId = iterFind->second.second;
	}

	if (featureId == 0)
		return false;

	// item to be traded in must be in the player's top level inventory
	ServerObject const * const playerInventory = playerCreature.getInventory();
	if (!playerInventory)
		return false;

	if (static_cast<void const *>(playerInventory) != static_cast<void const *>(ContainerInterface::getContainedByObject(item)))
	{
		Chat::sendSystemMessage(playerCreature, StringId("veteran_new", "trade_in_item_not_in_inventory"), Unicode::emptyString);
		return false;
	}

	// check to see if the object is already in the middle of a trade in
	if (!checkForTradeInInProgress(item))
		return false;

	// there is a wait period after the item is claimed before it can be traded in
	time_t itemClaimTime = 0;
	DynamicVariableList const & itemObjvar = item.getObjVars();
	if (itemObjvar.hasItem("rewardGrantInfo") && (itemObjvar.getType("rewardGrantInfo") == DynamicVariable::STRING_ARRAY))
	{
		std::vector<Unicode::String> rewardGrantInfo;
		if (itemObjvar.getItem("rewardGrantInfo", rewardGrantInfo) && (rewardGrantInfo.size() == 4))
			itemClaimTime = static_cast<time_t>(::atol(Unicode::wideToNarrow(rewardGrantInfo[0]).c_str()));
	}

	// it may be the Sarlacc trash can which has its born date stored on it
	if ((itemClaimTime <= 0) && (staticItemName == std::string("item_cts_sarlacc_mini_game")) && itemObjvar.hasItem("sarlacc_minigame.born_time") && (itemObjvar.getType("sarlacc_minigame.born_time") == DynamicVariable::INT))
	{
		int sarlaccBornTime = 0;
		if (itemObjvar.getItem("sarlacc_minigame.born_time", sarlaccBornTime))
			itemClaimTime = static_cast<time_t>(sarlaccBornTime);
	}

	if (itemClaimTime > 0)
	{
		time_t timeRedeem = itemClaimTime + static_cast<time_t>(ConfigServerGame::getVeteranRewardTradeInWaitPeriodSeconds());
		time_t const timeNow = ::time(nullptr);

		// see if the item has its own trade-in wait period
		if (itemObjvar.hasItem("rewardTradeInWaitPeriod") && (itemObjvar.getType("rewardTradeInWaitPeriod") == DynamicVariable::INT))
		{
			int itemTradeInWaitPeriodSeconds = 0;
			if (itemObjvar.getItem("rewardTradeInWaitPeriod", itemTradeInWaitPeriodSeconds))
				timeRedeem = itemClaimTime + std::max(itemTradeInWaitPeriodSeconds, 0);
		}

		if (timeRedeem > timeNow)
		{
			ProsePackage prosePackage;
			prosePackage.stringId = StringId("veteran_new", "trade_in_too_soon");
			prosePackage.target.str = Unicode::narrowToWide(CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(timeRedeem - timeNow)));
			Chat::sendSystemMessage(playerCreature, prosePackage);

			return false;
		}
	}

	if (validateOnly)
		return true;

	// set a "timed" objvar on the object which will be used to prevent the object
	// from being destroyed or transferred to a different container while we do
	// the trade in
	int const redemptionTimeout = 300; // 5 minutes;
	item.removeObjVarItem(ms_tradeInInProgressObjvar);
	item.setObjVarItem(ms_tradeInInProgressObjvar, static_cast<int>(::time(nullptr) + redemptionTimeout)); // 5 minutes

	// send off request to adjust the account feature Id
	AdjustAccountFeatureIdRequest const msg(NetworkId::cms_invalid, GameServer::getInstance().getProcessId(), playerCreature.getNetworkId(), PlayerObject::getAccountDescription(&playerCreature), playerPlayer->getStationId(), item.getNetworkId(), ServerObject::getLogDescription(&item), PlatformGameCode::SWG, static_cast<uint32>(featureId), 1);
	client->sendToConnectionServer(msg);

	// let player know the trade in request has been sent
	ProsePackage prosePackage;
	prosePackage.stringId = StringId("veteran_new", "trade_in_request_sent");
	prosePackage.target.str = Unicode::narrowToWide(CalendarTime::convertSecondsToMS(static_cast<unsigned int>(redemptionTimeout)));
	Chat::sendSystemMessage(playerCreature, prosePackage);

	return true;
}

// ----------------------------------------------------------------------

bool VeteranRewardManager::checkForTradeInInProgress(ServerObject const & item)
{
	if (item.getObjVars().hasItem(ms_tradeInInProgressObjvar) && (item.getObjVars().getType(ms_tradeInInProgressObjvar) == DynamicVariable::INT))
	{
		int redemptionTimeout;
		IGNORE_RETURN(item.getObjVars().getItem(ms_tradeInInProgressObjvar, redemptionTimeout));

		time_t const currentTime = ::time(nullptr);
		if (redemptionTimeout > currentTime)
		{
			// item should be in top level inventory; get the player object the item is in
			ServerObject const * containingPlayer = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(item)); // this gets the player's inventory
			if (containingPlayer)
				containingPlayer = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*containingPlayer)); // this gets the player

			if (containingPlayer && containingPlayer->getClient())
			{
				ProsePackage prosePackage;
				prosePackage.stringId = StringId("veteran_new", "trade_in_already_in_progress");
				prosePackage.target.str = Unicode::narrowToWide(CalendarTime::convertSecondsToMS(static_cast<unsigned int>(redemptionTimeout - currentTime)));
				Chat::sendSystemMessage(*containingPlayer, prosePackage);
			}

			// item is currently being redeemed
			return false;
		}
	}

	// item is not currently being redeemed
	return true;
}

// ----------------------------------------------------------------------

void VeteranRewardManager::handleTradeInResponse(ServerObject & item, AdjustAccountFeatureIdResponse const & response)
{
	if (response.getResultCode() == RESULT_SUCCESS)
	{
		// item has been successfully traded in, destroy it
		item.permanentlyDestroy(DeleteReasons::Consumed);
	}
	else
	{
		// item has NOT been successfully traded in, remove the
		// trade in lock on it, so the player can try again
		item.removeObjVarItem(ms_tradeInInProgressObjvar);
	}
}

// ----------------------------------------------------------------------

bool VeteranRewardManager::isTradeInRewardFeatureId(uint32 featureId)
{
	return (ms_canTradeInRewardFeatureId.count(featureId) > 0);
}

// ----------------------------------------------------------------------

void VeteranRewardManager::announceMonthlyBonusBoosterPack(CreatureObject const & playerCreature)
{
	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(&playerCreature);
	if (!playerObject)
		return;

	Client * const client = playerCreature.getClient();
	if (!client)
		return;

	if (client->isUsingAdminLogin())
		return;

	if (ConfigServerGame::getAccountFeatureIdForMonthlyBoosterPack() <= 0)
		return;

	Client::AccountFeatureIdList const & accountFeatureIds = client->getAccountFeatureIds();
	Client::AccountFeatureIdList::const_iterator iterFind = accountFeatureIds.find(static_cast<uint32>(ConfigServerGame::getAccountFeatureIdForMonthlyBoosterPack()));
	if ((iterFind == accountFeatureIds.end()) || (iterFind->second <= 0))
		return;

	// reset the announcement feature id
	AdjustAccountFeatureIdRequest const msg(NetworkId::cms_invalid, GameServer::getInstance().getProcessId(), playerCreature.getNetworkId(), std::string(), playerObject->getStationId(), NetworkId::cms_invalid, std::string(), PlatformGameCode::SWG, static_cast<uint32>(ConfigServerGame::getAccountFeatureIdForMonthlyBoosterPack()), (std::numeric_limits<int>::min() / 2)); // adjusting by a large negative value will force it to 0
	client->sendToConnectionServer(msg);

	// display announcement
	LOG("CustomerService",("VeteranRewards: displaying announcement for monthly free 5 pack of boosters for %s (account feature Id %lu, %d -> 0)", PlayerObject::getAccountDescription(&playerCreature).c_str(), static_cast<uint32>(ConfigServerGame::getAccountFeatureIdForMonthlyBoosterPack()), iterFind->second));
	Chat::sendSystemMessage(playerCreature, StringId("veteran_new", "tcg_available_monthly_bonus_booster_pack"), Unicode::emptyString);
}

// ----------------------------------------------------------------------

void VeteranRewardManager::announceSwgTcgBeta(CreatureObject const & playerCreature)
{
	if (ConfigServerGame::getAccountFeatureIdForTcgBetaAnnouncement() <= 0)
		return;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(&playerCreature);
	if (!playerObject)
		return;

	Client * const client = playerCreature.getClient();
	if (!client)
		return;

	if (client->isUsingAdminLogin())
		return;

	Client::AccountFeatureIdList const & accountFeatureIds = client->getAccountFeatureIds();
	Client::AccountFeatureIdList::const_iterator iterFind = accountFeatureIds.find(static_cast<uint32>(ConfigServerGame::getAccountFeatureIdForTcgBetaAnnouncement()));
	if ((iterFind == accountFeatureIds.end()) || (iterFind->second <= 0))
		return;

	// reset the announcement feature id
	AdjustAccountFeatureIdRequest const msg(NetworkId::cms_invalid, GameServer::getInstance().getProcessId(), playerCreature.getNetworkId(), std::string(), playerObject->getStationId(), NetworkId::cms_invalid, std::string(), PlatformGameCode::SWG, static_cast<uint32>(ConfigServerGame::getAccountFeatureIdForTcgBetaAnnouncement()), (std::numeric_limits<int>::min() / 2)); // adjusting by a large negative value will force it to 0
	client->sendToConnectionServer(msg);

	// display and send mail announcement
	LOG("CustomerService",("VeteranRewards: displaying TCG beta announcement for %s (account feature Id %lu, %d -> 0)", PlayerObject::getAccountDescription(&playerCreature).c_str(), static_cast<uint32>(ConfigServerGame::getAccountFeatureIdForTcgBetaAnnouncement()), iterFind->second));
	Chat::sendSystemMessage(playerCreature, StringId("veteran_new", "tcg_beta_announcement"), Unicode::emptyString);

	Unicode::String const mailSubject = Unicode::narrowToWide("@" + StringId("veteran_new", "tcg_beta_announcement_subject").getCanonicalRepresentation());

	ProsePackage pp;
	pp.stringId = StringId("veteran_new", "tcg_beta_announcement");

	Unicode::String oob;
	OutOfBandPackager::pack(pp, -1, oob);

	Chat::sendPersistentMessage("Star Wars Galaxies", Unicode::wideToNarrow(playerCreature.getAssignedObjectFirstName()), mailSubject, Unicode::emptyString, oob);
}

// ----------------------------------------------------------------------

std::vector<std::string> VeteranRewardManagerNamespace::buildVectorFromString(std::string packedString)
{
	std::vector<std::string> results;

	if (packedString.empty())
		return results;
	
	packedString +=','; // makes parsing easier
	char temp[200];
	int tempPos = 0;
	for (std::string::const_iterator i=packedString.begin(); i!=packedString.end(); ++i)
	{
		if (*i==',')
		{
			temp[tempPos]='\0';
			results.push_back(std::string(temp));
			tempPos = 0;
		}
		else
			temp[tempPos++]=*i;
		FATAL(tempPos==200,("Data bug:  VeteranRewardManager could not parse list %s",packedString.c_str()));
	}

	return results;
}

// ----------------------------------------------------------------------

void VeteranRewardManager::setOverrideAccountAge(int accountAge)
{
	if (ConfigServerGame::getVeteranDebugEnableOverrideAccountAge())
		ms_overrideAccountAge=accountAge;
}

// ----------------------------------------------------------------------

bool VeteranRewardManager::isFeatureIdBlockedBySubscriptionBit(CreatureObject const & playerCreature, uint32 featureId)
{
	FeatureIdBlockedBySubscriptionBitMap::const_iterator iter = ms_featureIdBlockedBySubscriptionBits.find(featureId);

	if (iter == ms_featureIdBlockedBySubscriptionBits.end())
		return false;

	Client const * const client = playerCreature.getClient();

	if (!client)
		return false;

	uint32 const subscriptionFeatures = client->getSubscriptionFeatures();

	return (subscriptionFeatures & iter->second) != 0;
}

// ======================================================================

RewardItem::RewardItem(DataTable const & dataTable, int row) :
		m_id(dataTable.getStringValue("id",row)),
		m_description(StringId(dataTable.getStringValue("Description",row))),
		m_objectTemplateName(dataTable.getStringValue("Object Template",row)),
		m_money(dataTable.getIntValue("Money",row)),
		m_oncePerAccount(dataTable.getIntValue("Once Per Account",row) != 0),
		m_oncePerAccountFeatureId(dataTable.getIntValue("Once Per Account Feature Id",row) != 0),
		m_oncePerCharacter(m_oncePerAccount || dataTable.getIntValue("Once Per Character",row) != 0),
		m_canTradeIn(dataTable.getIntValue("Can Trade In",row) != 0)
{
	FATAL(m_id.length() > 25,("Data bug:  veteran reward item name \"%s\" is too long.  The limit is 25 characters.", m_id.c_str()));
	FATAL(!m_objectTemplateName.empty() && m_money != 0,("Data bug:  veteran reward %s specified an object template and a money amount.  A reward should be one or the other (to avoid a complicated exploit in which players can claim the item multiple times by preventing the money transfer from going through)",m_id.c_str()));

	// only one of Once Per Account/Once Per Account Feature Id/Once Per Character can be set
	FATAL(m_oncePerAccount && m_oncePerAccountFeatureId,("Data bug:  veteran reward %s cannot be both \"Once Per Account\" and \"Once Per Account Feature Id\"",m_id.c_str()));
	FATAL(m_oncePerAccount && (dataTable.getIntValue("Once Per Character",row) != 0),("Data bug:  veteran reward %s cannot be both \"Once Per Account\" and \"Once Per Character\"",m_id.c_str()));
	FATAL(m_oncePerAccountFeatureId && (dataTable.getIntValue("Once Per Character",row) != 0),("Data bug:  veteran reward %s cannot be both \"Once Per Character\" and \"Once Per Account Feature Id\"",m_id.c_str()));

	uint32 templateItemCrc = 0;
	if (m_objectTemplateName.find(".iff") != std::string::npos)
		templateItemCrc = CrcLowerString::calculateCrc(m_objectTemplateName.c_str());

	FATAL(((templateItemCrc > 0) && (ms_canTradeInRewardTemplateItem.count(templateItemCrc) > 0)), ("Data bug:  \"can trade in\" reward object template (%s) can only be defined as the reward object for 1 reward item", m_objectTemplateName.c_str()));
	FATAL(((templateItemCrc == 0) && (ms_canTradeInRewardStaticItem.count(m_objectTemplateName) > 0)), ("Data bug:  \"can trade in\" reward object template (%s) can only be defined as the reward object for 1 reward item", m_objectTemplateName.c_str()));

	if (m_canTradeIn)
	{
		FATAL(m_oncePerAccountFeatureId,("Data bug:  \"can trade in\" veteran reward %s cannot be \"Once Per Account Feature Id\"",m_id.c_str()));
		FATAL(m_oncePerAccount || (dataTable.getIntValue("Once Per Character",row) != 0),("Data bug:  \"can trade in\" veteran reward %s cannot be \"Once Per Account\" or \"Once Per Character\"",m_id.c_str()));

		if (templateItemCrc > 0)
			ms_canTradeInRewardTemplateItem[templateItemCrc] = std::make_pair(m_id, 0);
		else
			ms_canTradeInRewardStaticItem[m_objectTemplateName] = std::make_pair(m_id, 0);
	}
}

// ----------------------------------------------------------------------

std::string const & RewardItem::getId() const
{
	return m_id;
}

// ----------------------------------------------------------------------

StringId const & RewardItem::getDescription() const
{
	return m_description;
}

// ----------------------------------------------------------------------

std::string const & RewardItem::getObjectTemplateName() const
{
	return m_objectTemplateName;
}

// ----------------------------------------------------------------------

bool RewardItem::getOncePerAccount() const
{
	return m_oncePerAccount;
}

// ----------------------------------------------------------------------

bool RewardItem::getOncePerAccountFeatureId() const
{
	return m_oncePerAccountFeatureId;
}

// ----------------------------------------------------------------------

bool RewardItem::getOncePerCharacter() const
{
	return m_oncePerCharacter;
}

// ----------------------------------------------------------------------

bool RewardItem::getCanTradeIn() const
{
	return m_canTradeIn;
}

// ----------------------------------------------------------------------

int RewardItem::getMoney() const
{
	return m_money;
}

// ----------------------------------------------------------------------

bool RewardItem::grantToPlayer(CreatureObject & playerCreature, uint32 accountFeatureId, int previousAccountFeatureIdCount, int currentAccountFeatureIdCount, bool regrant /*= false*/) const
{
	if (!m_objectTemplateName.empty())
	{
		NetworkId newObject;

		std::vector<const char *> rewardItem;
		rewardItem.push_back(m_objectTemplateName.c_str());

		ScriptParams params;
		params.addParam(rewardItem);

		GameScriptObject * gso = playerCreature.getScriptObject();
		if (gso && gso->hasTrigger(Scripting::TRIG_CREATE_VETERAN_REWARD))
		{
			// tell script to create the reward item
			IGNORE_RETURN(gso->trigAllScripts(Scripting::TRIG_CREATE_VETERAN_REWARD, params));

			// get the id of the created item
			std::vector<const char *> const & triggerResult = params.getStringArrayParam(0);
			if (!triggerResult.empty() && triggerResult[0] && (std::string(triggerResult[0]) != m_objectTemplateName))
			{
				newObject = NetworkId(std::string(triggerResult[0]));
			}
		}

		if (!newObject.isValid())
		{
			if (!regrant)
			{
				if (accountFeatureId > 0)
				{
					LOG("CustomerService",("VeteranRewards:  %s tried to claim item %s, but it could not be created (account feature Id %lu, %d -> %d)", PlayerObject::getAccountDescription(&playerCreature).c_str(), getId().c_str(), accountFeatureId, previousAccountFeatureIdCount, currentAccountFeatureIdCount));
				}
				else
				{
					LOG("CustomerService",("VeteranRewards:  %s tried to claim item %s, but it could not be created", PlayerObject::getAccountDescription(&playerCreature).c_str(), getId().c_str()));
				}
			}
			else
			{
				LOG("CustomerService",("VeteranRewards:  failed to regrant (item could not be created) item %s to %s", getId().c_str(), PlayerObject::getAccountDescription(&playerCreature).c_str()));
			}

			return false;
		}
		else
		{
			if (!regrant)
			{
				if (accountFeatureId > 0)
				{
					LOG("CustomerService",("VeteranRewards:  Granted item %s (%s) to player %s (account feature Id %lu, %d -> %d)", newObject.getValueString().c_str(), getId().c_str(), PlayerObject::getAccountDescription(&playerCreature).c_str(), accountFeatureId, previousAccountFeatureIdCount, currentAccountFeatureIdCount));
				}
				else
				{
					LOG("CustomerService",("VeteranRewards:  Granted item %s (%s) to player %s", newObject.getValueString().c_str(), getId().c_str(), PlayerObject::getAccountDescription(&playerCreature).c_str()));
				}
			}
			else
			{
				LOG("CustomerService",("VeteranRewards:  Regranted item %s (%s) to player %s", newObject.getValueString().c_str(), getId().c_str(), PlayerObject::getAccountDescription(&playerCreature).c_str()));
			}

			// if the reward item supports trade in, attach the trade in script to it to provide the trade in functionality
			ServerObject * const newObjectServerObject = ServerWorld::findObjectByNetworkId(newObject);
			if (newObjectServerObject && newObjectServerObject->isAuthoritative() && newObjectServerObject->isInitialized())
				VeteranRewardManager::attachTradeInScriptToItem(*newObjectServerObject);
		}
	}

	if (getMoney() > 0)
	{
		if (!playerCreature.transferBankCreditsFrom("VeteranRewards", getMoney()))
		{
			if (!regrant)
				LOG("CustomerService",("VeteranRewards:  %s tried to claim item %s, but the money could not be transfered", PlayerObject::getAccountDescription(&playerCreature).c_str(), getId().c_str()));
			else
				LOG("CustomerService",("VeteranRewards:  failed to regrant (money could not be transferred) item %s to %s", getId().c_str(), PlayerObject::getAccountDescription(&playerCreature).c_str()));

			return false;
		}
		else
		{
			if (!regrant)
				LOG("CustomerService",("VeteranRewards:  Granted %d credits (%s) to player %s", getMoney(), getId().c_str(), PlayerObject::getAccountDescription(&playerCreature).c_str()));
			else
				LOG("CustomerService",("VeteranRewards:  Regranted %d credits (%s) to player %s", getMoney(), getId().c_str(), PlayerObject::getAccountDescription(&playerCreature).c_str()));
		}
	}

	if (getOncePerCharacter() || getOncePerAccount())
	{
		IGNORE_RETURN(playerCreature.setObjVarItem(ms_claimedItemsObjvar + "." + getId(), static_cast<int>(1)));
	}

	return true;
}

// ======================================================================

RewardEvent::RewardEvent(DataTable const & dataTable, int row) :
		m_rowId(row),
		m_id(dataTable.getStringValue("id",row)),
		m_specificItems(buildVectorFromString(dataTable.getStringValue("Items",row))),
		m_includeItemsFrom(buildVectorFromString(dataTable.getStringValue("Include Items From",row))),
		m_allItems(nullptr),
		m_category(dataTable.getIntValue("Category", row)),
		m_featureBitRewardExclusionMask(dataTable.getIntValue("Feature Bit Reward Exclusion Mask", row)),
		m_accountFlags(static_cast<uint32>(dataTable.getIntValue("Account Flags",row))),
		m_accountFeatureId(static_cast<uint32>(dataTable.getIntValue("Account Feature Id",row))),
		m_consumeAccountFeatureId((m_accountFeatureId > 0) ? (dataTable.getIntValue("Consume Account Feature Id",row) != 0) : false), // "Consume Account Feature Id" is only meaningful if "Account Feature Id" is > 0
		m_minAccountAge(static_cast<unsigned int>(dataTable.getIntValue("Account Age Days",row))),
		m_minCharacterAge(dataTable.getIntValue("Character Age Days",row)),
		m_characterObjvar(dataTable.getStringValue("Character Objvar",row)),
		m_oncePerAccount(dataTable.getIntValue("Once Per Account",row) != 0),
		m_annoucement(dataTable.getStringValue("Announcement Message",row)),
		m_description(dataTable.getStringValue("Description",row)),
		m_url(dataTable.getStringValue("URL",row)),
		m_startDate(yyyymmddToTime(dataTable.getIntValue("Start Date yyyy",row), dataTable.getIntValue("Start Date mm",row), dataTable.getIntValue("Start Date dd",row))),
		m_endDate(yyyymmddToTime(dataTable.getIntValue("End Date yyyy",row), dataTable.getIntValue("End Date mm",row), dataTable.getIntValue("End Date dd",row))),
		m_minBuddyPoints(dataTable.getIntValue("Buddy Points",row)),
		m_featureIdBlockedBySubscriptionBit(dataTable.getIntValue("Feature Id Blocked by Subscription Bit", row))
{
	FATAL(m_id.empty(),("Data bug:  veteran reward event name at row %d is empty.", (row+3)));
	FATAL(m_id[0] == '*',("Data bug:  veteran reward event name \"%s\" cannot start with *", m_id.c_str()));
	FATAL(m_id.length() > 20,("Data bug:  veteran reward event name \"%s\" is too long.  The limit is 20 characters.", m_id.c_str()));
	FATAL((m_oncePerAccount && (m_accountFeatureId > 0) && m_consumeAccountFeatureId),("Data bug:  veteran reward event name \"%s\" cannot be both a \"once per account\" and a \"consuming\" feature Id event.", m_id.c_str()));
	FATAL(!m_accountFeatureId && m_featureIdBlockedBySubscriptionBit,("Data bug:  veteran reward event name \"%s\" has a Feature Id Blocked by Subscription Bit but the Feature Id is zero.", m_id.c_str()));

	if (m_featureIdBlockedBySubscriptionBit && m_accountFeatureId)
		ms_featureIdBlockedBySubscriptionBits[m_accountFeatureId] = m_featureIdBlockedBySubscriptionBit;
}

// ----------------------------------------------------------------------

int RewardEvent::getCategory() const
{
	return m_category;
}

// ----------------------------------------------------------------------

int RewardEvent::getFeatureBitRewardExclusionMask() const
{
	return m_featureBitRewardExclusionMask;
}

// ----------------------------------------------------------------------

bool RewardEvent::getOncePerAccount() const
{
	return m_oncePerAccount;
}

// ----------------------------------------------------------------------

uint32 RewardEvent::getAccountFeatureId() const
{
	return m_accountFeatureId;
}

// ----------------------------------------------------------------------

bool RewardEvent::getConsumeAccountFeatureId() const
{
	return m_consumeAccountFeatureId;
}

// ----------------------------------------------------------------------

bool RewardEvent::hasPlayerTriggered(CreatureObject const & playerCreature) const
{
	if (!ConfigServerGame::getEnableNewVeteranRewards())
		return false;

	if (!playerCreature.isAuthoritative())
		return false;

	Client const * const client = playerCreature.getClient();
	if (!client)
		return false;

	if (client->isUsingAdminLogin())
		return false; // Admin logins don't trigger rewards

	if (ConfigServerGame::getVeteranDebugTriggerAll())
		return true;

	unsigned int entitledTime = client->getEntitlementEntitledTime();
	uint32 const gameFeatures = client->getGameFeatures();
	int const buddyPoints = client->getBuddyPoints();
	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(&playerCreature);
	int const characterAge = playerObject ? playerObject->getAgeInDays() : 0;
	uint32 const subscriptionFeatures = client->getSubscriptionFeatures();

	entitledTime += static_cast<unsigned int>(static_cast<float>(buddyPoints) * ConfigServerGame::getBuddyPointTimeBonus());

	if (ms_overrideAccountAge!=0)
		entitledTime = ms_overrideAccountAge;

	if (m_startDate != 0 || m_endDate != 0)
	{
		time_t currentTime = time(nullptr);
		if ((m_startDate != 0 && currentTime < m_startDate)
			|| (m_endDate != 0 && currentTime > m_endDate))
			return false;
	}
	
	bool result = ((entitledTime >= m_minAccountAge) &&
			(characterAge >= m_minCharacterAge) &&
			((gameFeatures & m_accountFlags) == m_accountFlags) &&
			(buddyPoints >= m_minBuddyPoints));

	// check objvar requirement
	if (result && !m_characterObjvar.empty())
	{
		if (!playerCreature.getObjVars().hasItem(m_characterObjvar))
			result = false;
	}

	// check Feature Id requirement
	if (result && (m_accountFeatureId > 0))
	{
		if ((m_featureIdBlockedBySubscriptionBit & subscriptionFeatures) != 0)
			return false;

		Client::AccountFeatureIdList const & accountFeatureIds = client->getAccountFeatureIds();
		Client::AccountFeatureIdList::const_iterator iterFind = accountFeatureIds.find(m_accountFeatureId);
		if ((iterFind == accountFeatureIds.end()) || (iterFind->second <= 0))
			result = false;
	}

	// reward exclusion handling
	//   this mask is used to exclude the player from receiving a reward based on a feature bit setting
	//   for example, FreeTrial2 blocks the BARC speeder reward from ROWretail
	if (result && (gameFeatures & getFeatureBitRewardExclusionMask()) != 0)
		result = false;

	return result;
}

// ----------------------------------------------------------------------

std::string const & RewardEvent::getId() const
{
	return m_id;
}

// ----------------------------------------------------------------------

StringId const & RewardEvent::getAnnouncement() const
{
	return m_annoucement;
}

// ----------------------------------------------------------------------

StringId const & RewardEvent::getDescription() const
{
	return m_description;
}

// ----------------------------------------------------------------------

std::string const & RewardEvent::getUrl() const
{
	return m_url;
}

// ----------------------------------------------------------------------

/**
 * Return the list of reward items, including those indirectly included from
 * other events.
 *
 * (Circular includes are allowed.)
 */
std::vector<std::pair<RewardItem const *, RewardEvent const *> > const & RewardEvent::getRewardItems() const
{
	if (!m_allItems)
	{
		// order this event and other events that this event can get
		// items from  by the order they are listed in the rewards table;
		// this is so that we can order the items by the order of the 
		// event that they appear in in the rewards table 
		std::map<int, RewardEvent const *> events;

		{
			events[m_rowId] = this;

			for (std::vector<std::string>::const_iterator i=m_includeItemsFrom.begin(); i!=m_includeItemsFrom.end(); ++i)
			{
				RewardEvent const * const event = getRewardEventByName(*i);
				WARNING_DEBUG_FATAL(!event,("Data bug:  Reward Event %s specified including items from Reward Event %s, which could not be found.  It is probably not in the events data table.", m_id.c_str(), i->c_str()));
				if (event) //lint !e774 always true in debug builds
				{
					events[event->m_rowId] = event;
				}
			}
		}

		// get items for each of the event
		std::list<std::pair<RewardItem const *, RewardEvent const *> > tempAllItems;
		{
			std::set<RewardItem const *> uniqueItems;
			for (std::map<int, RewardEvent const *>::const_iterator iterEvent=events.begin(); iterEvent!=events.end(); ++iterEvent)
			{
				// get items specified in the event
				std::list<std::pair<RewardItem const *, RewardEvent const *> > tempEventItems;
				for (std::vector<std::string>::const_iterator i=iterEvent->second->m_specificItems.begin(); i!=iterEvent->second->m_specificItems.end(); ++i)
				{
					RewardItem const * const item = getRewardItemByName(*i);
					WARNING_DEBUG_FATAL(!item,("Data bug:  Reward Event %s specified item %s, which could not be found.  It is probably not in the items data table.", iterEvent->second->m_id.c_str(), i->c_str()));
					if (item) //lint !e774 always true in debug builds
					{
						if (uniqueItems.insert(item).second)
							tempEventItems.push_front(std::make_pair(item, iterEvent->second));
					}
				}

				// add items specified in the event to the master events list
				for (std::list<std::pair<RewardItem const *, RewardEvent const *> >::const_iterator j = tempEventItems.begin(); j != tempEventItems.end(); ++j)
					tempAllItems.push_front(*j);
			}
		}

		// save the master events list
		m_allItems = new std::vector<std::pair<RewardItem const *, RewardEvent const *> >;
		
		for (std::list<std::pair<RewardItem const *, RewardEvent const *> >::const_iterator iterItem = tempAllItems.begin(); iterItem != tempAllItems.end(); ++iterItem)
			m_allItems->push_back(*iterItem);
	}
	
	return *m_allItems;
}

// ======================================================================
