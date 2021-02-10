// ======================================================================
//
// CommoditiesMarket.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CommoditiesMarket.h"

#include "CommoditiesServerConnection.h"
#include "serverGame/AuctionInfo.h"
#include "serverGame/AuctionTokenServer.h"
#include "serverGame/Chat.h"
#include "serverGame/CommoditiesMarketListener.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "sharedGame/GameObjectTypes.h"
#include "serverGame/GameServer.h"
#include "serverGame/IntangibleObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NameManager.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/Region.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerTangibleObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/AuctionBase.h"
#include "serverNetworkMessages/AuctionData.h"
#include "serverNetworkMessages/SetEntranceChargeMessage.h"
#include "serverNetworkMessages/FlagObjectForDeleteMessage.h"
#include "serverNetworkMessages/QueryVendorItemCountMessage.h"
#include "serverNetworkMessages/UpdateVendorSearchOptionMessage.h"
#include "serverNetworkMessages/UpdateVendorStatusMessage.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/AuctionManager.h"
#include "sharedGame/AuctionToken.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/Location.h"
#include "sharedGame/OutOfBand.h"
#include "sharedGame/OutOfBandBase.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/WaypointData.h"
#include "sharedLog/Log.h"
#include "sharedMath/Vector2d.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/AcceptAuctionResponseMessage.h"
#include "sharedNetworkMessages/AuctionData.h"
#include "sharedNetworkMessages/AuctionErrorCodes.h"
#include "sharedNetworkMessages/AuctionQueryHeadersMessage.h"
#include "sharedNetworkMessages/AuctionQueryHeadersResponseMessage.h"
#include "sharedNetworkMessages/AuctionQueryResponseMessage.h"
#include "sharedNetworkMessages/BidAuctionResponseMessage.h"
#include "sharedNetworkMessages/CancelLiveAuctionResponseMessage.h"
#include "sharedNetworkMessages/CreateAuctionResponseMessage.h"
#include "sharedNetworkMessages/GetAuctionDetailsResponse.h"
#include "sharedNetworkMessages/IsVendorOwnerResponseMessage.h"
#include "sharedNetworkMessages/RetrieveAuctionItemResponseMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/SceneChannelMessages.h"
#include "sharedObject/Container.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"

//----------------------------------------------------------------------

#define ADD_AUCTION_COST 0

// this must be kept in sync in the following files
// src/engine/client/library/clientGame/src/shared/core/AuctionManagerClient.cpp (s_maxBid)
// src/engine/server/application/CommoditiesServer/src/shared/Auction.cpp (MAX_BID)
// src/engine/server/library/serverGame/src/shared/commoditiesMarket/CommoditiesMarket.cpp (MAX_BID)
#define MAX_BID 10000000
#define MAX_VENDOR_PRICE 833333333

//----------------------------------------------------------------------
const std::string CommoditiesMarket::OBJVAR_VENDOR_REINITIALIZING("vendor_reinit");

static const std::string OBJVAR_VENDOR_VALUE("vendor_value");
static const std::string OBJVAR_VENDOR_INITIALIZED("vendor_initialized");
static const std::string OBJVAR_VENDOR_DEACTIVATED("vendor_deactivated");


static const NetworkId zeroNetworkId(static_cast<NetworkId::NetworkIdType>(0));
static const std::string std_blank = std::string("");

namespace CommoditiesMarketNamespace
{

	namespace AuctionStringIds
	{
#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

		const StringId buyerAuctionSuccess("auction", "buyer_success");	
		const StringId sellerAuctionFail("auction", "seller_fail");	
		const StringId buyerAuctionFail("auction", "buyer_canceled");	
		const StringId bidderOutbid("auction", "bidder_outbid");	
		const StringId itemExpired("auction", "item_expired");	
		const StringId subjectVendorSeller("auction", "subject_vendor_seller");
		const StringId subjectInstantSeller("auction", "subject_instant_seller");
		const StringId subjectAuctionSeller("auction", "subject_auction_seller");
		
		const StringId subjectVendorBuyer("auction", "subject_vendor_buyer");
		const StringId subjectInstantBuyer("auction", "subject_instant_buyer");
		const StringId subjectAuctionBuyer("auction", "subject_auction_buyer");
		const StringId subjectAuctionUnsuccessful("auction", "subject_auction_unsuccessful");
		const StringId subjectAuctionCancelled("auction", "subject_auction_cancelled");
		const StringId subjectAuctionOutbid("auction", "subject_auction_outbid");
		const StringId subjectAuctionItemExpired("auction", "subject_auction_item_expired");
			
		MAKE_STRING_ID (auction, seller_success);
		MAKE_STRING_ID (auction, seller_success_location);
		MAKE_STRING_ID (auction, seller_success_vendor);
		MAKE_STRING_ID (auction, buyer_success_location);

#undef MAKE_STRING_ID
	}

	bool isReinitializing(const ServerObject& vendor)
 	{
 		return vendor.getObjVars().hasItem(CommoditiesMarket::OBJVAR_VENDOR_REINITIALIZING);
 	}

	struct SalesTaxInfo
 	{
 		NetworkId itemId;
 		NetworkId buyerId;
 		int salesTaxAmount;
 		NetworkId salesTaxBankId;
 	};
   	
	std::map<NetworkId, SalesTaxInfo*> s_pendingSalesTax;

	struct ItemRestriction
	{
		std::set<int>         inclusionGameObjectType;
		std::set<uint32>      inclusionTemplateItemType;
		std::set<std::string> inclusionStaticItemType;

		std::set<int>         exclusionGameObjectType;
		std::set<uint32>      exclusionTemplateItemType;
		std::set<std::string> exclusionStaticItemType;

		std::string           rejectionMessage;
	};
	bool isVendorItemRestriction(ServerObject const & item, std::string const & itemRestrictionFile, bool checkInInclusionList, bool checkInExclusionList, std::string & itemRestrictedRejectionMessage);

	bool canAuction(ServerObject& item, CreatureObject& owner, ServerObject& auctionContainer, int minBid, time_t timer, int auctionFee, AuctionResult& errorCode, std::string& itemRestrictedRejectionMessage)
	{
		errorCode = ar_OK;
		itemRestrictedRejectionMessage.clear();

		if (&owner != ContainerInterface::getFirstParentInWorld(item))
		{
			errorCode = ar_NOT_ITEM_OWNER;
			return false;
		}
		
		// ================================================
		// Exploit Fix
		// This block fixes an exploit that allows item duplication and excess credits
		// by selling an item to a junk dealer while simultaneously selling it on bazaar
		// ================================================

		errorCode = ar_NOT_ITEM_OWNER; //set this, we'll reset back to ar_OK inside the while loop
		const ServerObject* inventory = owner.getInventory();
		if (inventory == NULL)
		{
			return false; //couldn't find our inventory? bail
		}
		const Object* container = ContainerInterface::getContainedByObject(item);

		while (container != NULL)
		{
			if (inventory->getNetworkId() == container->getNetworkId())
			{
				//success
				errorCode = ar_OK;
				break;
			}
			// go up a container level
			container = ContainerInterface::getContainedByObject(*container);
		}
		if (errorCode != ar_OK)
		{
			LOG("CustomerService", ("ExploitBazaarSales: %s attempted to sell item %s that wasn't in their inventory or an accessible container.", owner.getNetworkId().getValueString().c_str(), item.getNetworkId().getValueString().c_str()));
			return false; //we weren't OK, return
		}

		// ================================================
		// == End Exploit Fix =============================
		// ================================================

		if (minBid < 0)
		{
			errorCode = ar_INVALID_MINIMUM_BID;
		}

		if (timer <= 0)
		{
			errorCode = ar_INVALID_AUCTION_LENGTH;
		}

		if (auctionFee > owner.getTotalMoney())
		{
			errorCode = ar_NOT_ENOUGH_MONEY;
		}

		int const maxBid = (auctionContainer.isVendor() ? MAX_VENDOR_PRICE : MAX_BID);
		if (minBid > maxBid)
		{
			errorCode = (auctionContainer.isVendor() ? ar_PRICE_TOO_HIGH : ar_BID_TOO_HIGH);
		}

		if (auctionContainer.getObjVars().hasItem(OBJVAR_VENDOR_DEACTIVATED))
		{
			errorCode = ar_VENDOR_DEACTIVATED;
		}
		
		Container::ContainerErrorCode tmp = Container::CEC_Success;
		ServerObject *bazaarContainer = auctionContainer.getBazaarContainer();
		if (!bazaarContainer || !ContainerInterface::canTransferTo(bazaarContainer, item, nullptr, tmp))
		{
			errorCode = ar_INVALID_ITEM_ID;
		}
		
		//-- can't sell the special items inside crates
		const ServerObject * const itemContainedBy = safe_cast<const ServerObject *>(ContainerInterface::getContainedByObject (item));
		if (itemContainedBy && itemContainedBy->getGameObjectType () == SharedObjectTemplate::GOT_misc_factory_crate)
		{
			errorCode = ar_IN_CRATE;
		}
		
		// disallow putting an item up for auction that is already in secure trade
		if( item.isInSecureTrade() )
		{
			LOG("CustomerService", ("Auction:item %s was attempted to be placed for sale by %s but object is secure trade.",
									ServerObject::getLogDescription(item.getNetworkId()).c_str(), PlayerObject::getAccountDescription(owner.getNetworkId()).c_str()));
			errorCode = ar_IN_TRADE;
		}
		// make sure we have an authoritative item
		if( !item.isAuthoritative() )
		{
			LOG("CustomerService", ("Auction:item %s was attempted to be placed for sale by %s but object is not authoritative on this server.",
									ServerObject::getLogDescription(item.getNetworkId()).c_str(), PlayerObject::getAccountDescription(owner.getNetworkId()).c_str()));
			errorCode = ar_INVALID_ITEM_ID;
		}
	
		//can't sell no trade items
		if (!item.canTradeRecursive(false))
		{
			errorCode = ar_NOT_ALLOWED;
		}

		// can't sell bio-linked items
		if (item.isBioLinkedRecursive())
		{
			errorCode = ar_IS_BIOLINKED;
		}

		// can't sell equipped items
		if (owner.isItemEquipped (item))
		{
			errorCode = ar_ITEM_EQUIPPED;
		}

		if ((errorCode == ar_OK) && auctionContainer.isVendor() && !auctionContainer.isBazaarTerminal())
		{
			// check to make sure the vendor isn't restricted from accepting this item
			std::string itemRestrictionFile;
			if (!auctionContainer.getObjVars().getItem("access_file", itemRestrictionFile))
				itemRestrictionFile = ConfigServerGame::getDefaultVendorItemRestrictionFile();

			if (isVendorItemRestriction(item, itemRestrictionFile, true, true, itemRestrictedRejectionMessage))
			{
				errorCode = ar_ITEM_RESTRICTED;
			}
		}

		return errorCode == ar_OK;
	}

	int getItemObjectTemplateId(const ServerObject &item)
	{
		return (item.getObjectTemplate() ? static_cast<int>(item.getObjectTemplate()->getCrcName().getCrc()) : 0);
	}

	int getCrateContainedItemObjectTemplateId(const ServerObject &crate)
	{
		if (crate.getGameObjectType() != SharedObjectTemplate::GOT_misc_factory_crate)
			return 0;

		const Container* c = ContainerInterface::getContainer(crate);
		if (!c)
			return 0;

		for (ContainerConstIterator i = c->begin(); i != c->end(); ++i)
		{
			if ((*i).getObject() && (*i).getObject()->getObjectTemplate())
				return static_cast<int>((*i).getObject()->getObjectTemplate()->getCrcName().getCrc());
		}

		return 0;
	}

	Unicode::String convertSeparatorToNull(Unicode::String const & sourceString)
	{
		Unicode::String result;
		for (Unicode::String::const_iterator i=sourceString.begin(); i!=sourceString.end(); ++i)
			if (*i==uint16('\t'))
				result += uint16(0);
			else
				result += *i;

		return result;
	}

	std::map<int, std::map<int, std::pair<int, StringId> > > s_itemTypeMap;
	int s_itemTypeMapVersionNumber = 0;

	std::map<int, std::set<std::string> > s_resourceTypeMap;
	int s_resourceTypeMapVersionNumber = 0;
}

using namespace CommoditiesMarketNamespace;

// ======================================================================

bool CommoditiesMarketNamespace::isVendorItemRestriction(ServerObject const & item, std::string const & itemRestrictionFile, bool checkInInclusionList, bool checkInExclusionList, std::string & itemRestrictedRejectionMessage)
{
	static std::map<std::string, ItemRestriction> itemRestrictionList;

	if (!checkInInclusionList && !checkInExclusionList)
	{
		return false;
	}

	ItemRestriction const * itemRestriction = nullptr;

	std::map<std::string, ItemRestriction>::const_iterator const iterFind = itemRestrictionList.find(itemRestrictionFile);
	if (iterFind != itemRestrictionList.end())
	{
		itemRestriction = &(iterFind->second);
	}
	else
	{
		ItemRestriction & ir = itemRestrictionList[itemRestrictionFile];
		itemRestriction = &ir;
		ir.inclusionGameObjectType.clear();
		ir.inclusionTemplateItemType.clear();
		ir.inclusionStaticItemType.clear();
		ir.exclusionGameObjectType.clear();
		ir.exclusionTemplateItemType.clear();
		ir.exclusionStaticItemType.clear();
		ir.rejectionMessage = ConfigServerGame::getDefaultVendorItemRestrictionRejectionMessage();

		DataTable * const table = DataTableManager::getTable(itemRestrictionFile, true);
		if (table)
		{
			int const columnInclusion = table->findColumnNumber("Inclusion");
			int const columnExclusion = table->findColumnNumber("Exclusion");
			int const columnRejectionMessage = table->findColumnNumber("Rejection Message");

			if ((columnInclusion >= 0) && (columnExclusion >= 0) && (columnRejectionMessage >= 0))
			{
				std::string inclusion;
				std::string exclusion;
				std::string rejectionMessage;
				int got;

				for (int i = 0, numRows = table->getNumRows(); i < numRows; ++i)
				{
					inclusion = table->getStringValue(columnInclusion, i);
					exclusion = table->getStringValue(columnExclusion, i);
					rejectionMessage = table->getStringValue(columnRejectionMessage, i);

					if (!inclusion.empty())
					{
						if (inclusion.find(".iff") != std::string::npos)
						{
							ir.inclusionTemplateItemType.insert(CrcLowerString::calculateCrc(inclusion.c_str()));
						}
						else
						{
							got = GameObjectTypes::getGameObjectType(inclusion);
							if (got > 0)
							{
								ir.inclusionGameObjectType.insert(got);
							}
							else
							{
								ir.inclusionStaticItemType.insert(inclusion);
							}
						}
					}

					if (!exclusion.empty())
					{
						if (exclusion.find(".iff") != std::string::npos)
						{
							ir.exclusionTemplateItemType.insert(CrcLowerString::calculateCrc(exclusion.c_str()));
						}
						else
						{
							got = GameObjectTypes::getGameObjectType(exclusion);
							if (got > 0)
							{
								ir.exclusionGameObjectType.insert(got);
							}
							else
							{
								ir.exclusionStaticItemType.insert(exclusion);
							}
						}
					}

					if (!rejectionMessage.empty())
					{
						ir.rejectionMessage = rejectionMessage;
					}
				}
			}

			DataTableManager::close(itemRestrictionFile);
		}
	}

	itemRestrictedRejectionMessage.clear();
	if (!itemRestriction)
	{
		return false;
	}

	if (itemRestriction->inclusionGameObjectType.empty() &&
		itemRestriction->inclusionTemplateItemType.empty() &&
		itemRestriction->inclusionStaticItemType.empty() &&
		itemRestriction->exclusionGameObjectType.empty() &&
		itemRestriction->exclusionTemplateItemType.empty() &&
		itemRestriction->exclusionStaticItemType.empty())
	{
		return false;
	}

	// check for restriction because in exclusion list
	if (checkInExclusionList)
	{
		if (!itemRestriction->exclusionGameObjectType.empty())
		{
			if (itemRestriction->exclusionGameObjectType.count(item.getGameObjectType()) >= 1)
			{
				itemRestrictedRejectionMessage = itemRestriction->rejectionMessage;
				return true;
			}

			if (itemRestriction->exclusionGameObjectType.count(GameObjectTypes::getMaskedType(item.getGameObjectType())) >= 1)
			{
				itemRestrictedRejectionMessage = itemRestriction->rejectionMessage;
				return true;
			}
		}

		if (!itemRestriction->exclusionTemplateItemType.empty())
		{
			if (itemRestriction->exclusionTemplateItemType.count(item.getTemplateCrc()) >= 1)
			{
				itemRestrictedRejectionMessage = itemRestriction->rejectionMessage;
				return true;
			}
		}

		if (!itemRestriction->exclusionStaticItemType.empty())
		{
			std::string const & staticItemName = item.getStaticItemName();
			if (!staticItemName.empty())
			{
				if (itemRestriction->exclusionStaticItemType.count(staticItemName) >= 1)
				{
					itemRestrictedRejectionMessage = itemRestriction->rejectionMessage;
					return true;
				}
			}
		}
	}

	// check for restriction because *NOT* in inclusion list
	if (checkInInclusionList)
	{
		if (!itemRestriction->inclusionGameObjectType.empty() || !itemRestriction->inclusionTemplateItemType.empty() || !itemRestriction->inclusionStaticItemType.empty())
		{
			// restricted until proved otherwise by being in one of the inclusion lists
			bool restricted = true;

			if (restricted && !itemRestriction->inclusionGameObjectType.empty())
			{
				if (itemRestriction->inclusionGameObjectType.count(item.getGameObjectType()) >= 1)
				{
					restricted = false;
				}
				else if (itemRestriction->inclusionGameObjectType.count(GameObjectTypes::getMaskedType(item.getGameObjectType())) >= 1)
				{
					restricted = false;
				}
			}

			if (restricted && !itemRestriction->inclusionTemplateItemType.empty())
			{
				if (itemRestriction->inclusionTemplateItemType.count(item.getTemplateCrc()) >= 1)
				{
					restricted = false;
				}
			}

			if (restricted && !itemRestriction->inclusionStaticItemType.empty())
			{
				std::string const & staticItemName = item.getStaticItemName();
				if (!staticItemName.empty())
				{
					if (itemRestriction->inclusionStaticItemType.count(staticItemName) >= 1)
					{
						restricted = false;
					}
				}
			}

			if (restricted)
			{
				itemRestrictedRejectionMessage = itemRestriction->rejectionMessage;
				return true;
			}
		}
	}

	// check to see if any of the contained item is restricted
	const Container * const container = ContainerInterface::getContainer(item);
	if (container)
	{
		for (ContainerConstIterator i = container->begin(); i != container->end(); ++i)
		{
			const ServerObject * const content = safe_cast<const ServerObject *>((*i).getObject());
			if (content)
			{
				// for items inside the container, just check them against the exclusion list
				// as we assume that if a container is accepted, then all the items inside the
				// container should not be excluded by the inclusion list, but still need to
				// be excluded through the exclusion list
				if (isVendorItemRestriction(*content, itemRestrictionFile, false, true, itemRestrictedRejectionMessage))
					return true;
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------

#ifdef _WIN32
#define atoll(a) _atoi64(a)
#endif

// ======================================================================

struct ItemLoadPair
{
	ItemLoadPair(const NetworkId &o, const std::string &l) : ownerId(o), location(l) {} 
	NetworkId ownerId;
	std::string location;
};

struct QueuedQuery
{
	// time that the previous query was sent off
	unsigned long previousQueryTime;

	// the most recent query that came in while
	// the previous query was being processed
	int requestId;
	int type;
	int category;
	bool categoryExactMatch;
	int itemTemplateId;
	Unicode::String textFilterAll;
	Unicode::String textFilterAny;
	int priceFilterMin;
	int priceFilterMax;
	bool priceFilterIncludesFee;
	std::list<AuctionQueryHeadersMessage::SearchCondition> advancedSearch;
	AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny;
	NetworkId container;
	int locationSearchType;
	bool searchMyVendorsOnly;
	uint16 queryOffset;
};

//----------------------------------------------------------------------

static CommoditiesServerConnection *                   s_market;
static time_t                                          s_timeMarketConnectionCreated = 0;
static std::map<int, std::pair<CachedNetworkId, int> > s_queries;
static std::map<NetworkId, QueuedQuery *>              s_queryTime;
static int                                             s_nextRequestId;
static std::unordered_map<NetworkId, ItemLoadPair, NetworkId::Hash>                  s_pendingLoads;
static std::unordered_map<int, int>                         s_pendingAdds; //sequence to cost map

// all money transactions transfer money out of or to the bank of the player and never to or from the vendor/bazaar location except bazaar sale fees
static const std::string COMMODITIES_NAMED_ACCOUNT("commodities_named_escrow_account");

// ======================================================================

// this function transfers money from a player to a named account.  It requires that the object exist on the this server to succeed.
static bool transferFromBankToCommoditiesNamedAccount(const NetworkId &target, int32 amount)
{
	ServerObject *obj = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(target));
	if( obj )
	{
		if( obj->transferBankCreditsTo(COMMODITIES_NAMED_ACCOUNT, amount))
		{
			LOG("CustomerService", ("Auction: transfered bid amount of %d from bank to commodities named account for player '%s' (%Ld)",
				amount, PlayerObject::getAccountDescription(target).c_str(), target.getValue()));
			return true;
		}
	}

	LOG("CustomerService", ("Auction: failed commodities named account bank transfer for player '%s' (%Ld) in the bid amount of %d",
		PlayerObject::getAccountDescription(target).c_str(), target.getValue(), amount));
	return false;
}

// this function transfers money from a named account to the player.
static void transferToBankFromCommoditiesNamedAccount(const NetworkId &target, const int32 amount, const std::string &reason)
{
	ServerObject *obj = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(target));
	if( obj && obj->isAuthoritative() )
	{
		if( obj->transferBankCreditsFrom(COMMODITIES_NAMED_ACCOUNT, amount) )
		{
			LOG("CustomerService", ("Auction: successfully transfered %d credits to player '%s' (%Ld).  Transfer Reason = '%s'",
				amount, PlayerObject::getAccountDescription(target).c_str(), target.getValue(), reason.c_str()));
		}
		else
		{
			LOG("CustomerService", ("Auction: failed to transfer %d credits to player '%s' (%Ld) because ServerObject::transferBankCreditsFrom() returned false.  Transfer Reason = '%s'",
				amount, PlayerObject::getAccountDescription(target).c_str(), target.getValue(), reason.c_str()));
		}
	}
	else
	{	
		LOG("CustomerService", ("Auction: Transferring %d credits to player '%s' (%Ld) using C++FromNamedAccountToBank message method.  Transfer Reason = '%s'",
			amount, PlayerObject::getAccountDescription(target).c_str(), target.getValue(), reason.c_str()));
		char buf[100];
		snprintf(buf, sizeof(buf), "%s %d", COMMODITIES_NAMED_ACCOUNT.c_str(), (int)(amount));
		MessageToQueue::getInstance().sendMessageToC(target, "C++FromNamedAccountToBank", buf, 0, true);
	}
}

static void transferBank(const NetworkId &source, const NetworkId &dest, int32 amount)
{
	ServerObject *obj = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(source));
	if (obj)
	{
		obj->transferBankCreditsTo(dest, amount);
	}
	else
	{
//		printf("Transfering %i bank from %Ld to %Ld", (int)amount, source.getValue(), dest.getValue());
		char buf[100];
		sprintf(buf, "%Ld %Ld", (int64)dest.getValue(), (int64)amount);
		MessageToQueue::getInstance().sendMessageToC(source, "C++ModifyBank", buf, 0, true);
	}
}

// ======================================================================

static std::string getLocationString(const ServerObject &container)
{
	ServerObject *auctionContainer = const_cast<ServerObject *>(&container);
	//std::string ret(auctionContainer->getNetworkId().getValueString());

	if (!auctionContainer)
	{
		return "junk";
	}
	ServerObject *bazaarContainer = auctionContainer->getBazaarContainer();
	ServerObject *parent = const_cast<ServerObject *>(dynamic_cast<const ServerObject *>(ContainerInterface::getFirstParentInWorld(container)));
	if (parent && parent->asCreatureObject())
	{
		if (parent->getBazaarContainer() == auctionContainer)
		{
			//We got passed the container itself on a vendor, but we want auctionContainer to be the parent.
			auctionContainer = parent;
			bazaarContainer = parent->getBazaarContainer();
		}
	}

	if (!bazaarContainer)
	{
		return "junk"; 
	}
	
	PlanetObject * const planet = ServerUniverse::getInstance().getPlanetByName(ServerWorld::getSceneId());
	if (!planet)
	{
		return std::string ("BADPLANET.@BADREGION.") + 
			Unicode::wideToNarrow (auctionContainer->getEncodedObjectName ()) + 
			"." + auctionContainer->getNetworkId().getValueString();
	}

	const Vector & pos_w = auctionContainer->findPosition_w();
	Region * const r = const_cast<Region *>(RegionMaster::getSmallestVisibleRegionAtPoint(planet->getName(), pos_w.x, pos_w.z));

	std::string regionName;

	if (r)
		regionName = Unicode::wideToNarrow (r->getName ());
	else
		regionName = std::string ("@planet_n:") + planet->getName ();

	char positionString[256];
	snprintf(positionString, sizeof (positionString), "#%i,%i", static_cast<int>(pos_w.x), static_cast<int>(pos_w.z));

	
	std::string ret = planet->getName() + ".";

	if (!regionName.empty () && regionName [0] != '@')
		ret.push_back ('@');

	ret += regionName;
	ret.push_back ('.');

	ret += Unicode::wideToNarrow (auctionContainer->getEncodedObjectName ()) + "." + bazaarContainer->getNetworkId().getValueString() + positionString;
	return ret;
}

// ======================================================================

static void getLocationSearchString(ServerObject *auctionContainer, int locationSearchType, std::string & planetStr, std::string & regionStr, NetworkId & auctionLocationId)
{
	planetStr.clear();
	regionStr.clear();
	auctionLocationId = NetworkId::cms_invalid;

	if (!auctionContainer)
	{
		return;
	}

	ServerObject *bazaarContainer = auctionContainer->getBazaarContainer();
	ServerObject *parent = const_cast<ServerObject *>(dynamic_cast<const ServerObject *>(ContainerInterface::getFirstParentInWorld(*auctionContainer)));
	if (parent && parent->asCreatureObject())
	{
		if (parent->getBazaarContainer() == auctionContainer)
		{
			//We got passed the container itself on a vendor, but we want auctionContainer to be the parent.
			auctionContainer = parent;
			bazaarContainer = parent->getBazaarContainer();
		}
	}

	if (!bazaarContainer)
	{
		return;
	}

	const PlanetObject * const planet = ServerUniverse::getInstance().getPlanetByName(ServerWorld::getSceneId());
	if (!planet)
	{
		return;
	}

	const Vector & pos_w = auctionContainer->findPosition_w();
	Region * const r = const_cast<Region *>(RegionMaster::getSmallestVisibleRegionAtPoint(planet->getName(), pos_w.x, pos_w.z));

	std::string regionName;

	if (r)
		regionName = Unicode::wideToNarrow (r->getName ());
	else
		regionName = std::string ("@planet_n:") + planet->getName ();

	switch (locationSearchType)
	{
	case AuctionQueryHeadersMessage::ALS_Galaxy:
		break;
		
	case AuctionQueryHeadersMessage::ALS_Planet:
		planetStr = planet->getName();
		break;
		
	case AuctionQueryHeadersMessage::ALS_Region:
		planetStr = planet->getName();
		regionStr = regionName;
		break;
		
	case AuctionQueryHeadersMessage::ALS_Market:
		auctionLocationId = bazaarContainer->getNetworkId();
		break;
	}

	return;
}

// ======================================================================

namespace Auction
{
	const uint16 maxPageSize = 100;

	static const Unicode::String at = Unicode::narrowToWide("@");
	static const std::string auctionSender("Auctioner");

	//----------------------------------------------------------------------

	void sendAuctionSuccessToSeller(
		const std::string &     seller, 
		const std::string &     buyer,
		const std::string &     location,
		const NetworkId &       buyerId,
		const Unicode::String & itemName,
		const NetworkId &       itemId,
		int                     purchaseAmount,
		bool                    immediate
	)
	{
		std::string planetName;
		std::string regionName;
		std::string marketName;

		AuctionManager::separateLocation (location, planetName, regionName, marketName);

		static const Unicode::String subject_auction = Unicode::narrowToWide("@" + AuctionStringIds::subjectAuctionSeller.getCanonicalRepresentation());
		static const Unicode::String subject_instant = Unicode::narrowToWide("@" + AuctionStringIds::subjectInstantSeller.getCanonicalRepresentation());
		static const Unicode::String subject_vendor = Unicode::narrowToWide("@" + AuctionStringIds::subjectVendorSeller.getCanonicalRepresentation());

		ProsePackage pp_location;
		pp_location.stringId   = AuctionStringIds::seller_success_location;
		pp_location.target.str = Unicode::narrowToWide (regionName);
		pp_location.other.str  = Unicode::narrowToWide ("@planet_n:") + Unicode::narrowToWide (planetName);

		//-- it is a vendor sale
		if (!_strnicmp (marketName.c_str (), "vendor:", 7))
		{
			ProsePackage pp;
			
			pp.stringId     = AuctionStringIds::seller_success_vendor;

			pp.actor.str    = Unicode::narrowToWide (marketName);
			pp.target.id    = buyerId;
			pp.target.str   = Unicode::narrowToWide(buyer);			
			pp.other.id     = itemId;
			pp.other.str    = itemName;
			pp.digitInteger = purchaseAmount; 
			
			Unicode::String oob;
			OutOfBandPackager::pack(pp, -1, oob);
			OutOfBandPackager::pack(pp_location, -1, oob);

			Chat::sendPersistentMessage(auctionSender, seller, subject_vendor, Unicode::emptyString, oob);
		}
		else
		{
			ProsePackage pp;
			
			pp.stringId     = AuctionStringIds::seller_success;
			pp.target.id    = buyerId;
			pp.target.str   = Unicode::narrowToWide(buyer);
			
			pp.other.id     = itemId;
			pp.other.str    = itemName;
			pp.digitInteger = purchaseAmount; 
			
			Unicode::String oob;
			OutOfBandPackager::pack(pp, -1, oob);
			OutOfBandPackager::pack(pp_location, -1, oob);

			Unicode::String subject;
			if (immediate)
				subject = subject_instant;
			else
				subject = subject_auction;

			Chat::sendPersistentMessage(auctionSender, seller, subject, Unicode::emptyString, oob); 
		}
	}

	//----------------------------------------------------------------------

	WaypointDataBase const getWaypointData(const std::string & locationString, const Unicode::String &itemName)
	{
		std::string planetName;
		std::string regionName;
		std::string marketName;
		NetworkId   marketId;
		int         marketX = 0;
		int         marketZ = 0;

		AuctionManager::separateLocation (locationString, planetName, regionName, marketName, marketId, marketX, marketZ);

		const Vector coords (static_cast<float>(marketX), 0.0f, static_cast<float>(marketZ));

		WaypointDataBase waypoint;
		waypoint.m_location = Location (coords, NetworkId::cms_invalid, Location::getCrcBySceneName (planetName));
		waypoint.setName(itemName);

		return waypoint;
	}

	//----------------------------------------------------------------------

	void sendAuctionSuccessToBuyer(
		const std::string &     seller, 
		const NetworkId &       sellerId,
		const std::string &     buyer,
		const std::string &     location,
		const Unicode::String & itemName,
		const NetworkId &       itemId,
		int                     purchaseAmount,
		bool                    immediate
	)
	{
		ProsePackage pp;
			
		pp.stringId     = AuctionStringIds::buyerAuctionSuccess;
		pp.target.id    = sellerId;
		pp.target.str   = Unicode::narrowToWide(seller);

		pp.other.id     = itemId;
		pp.other.str    = itemName;
		pp.digitInteger = purchaseAmount; 

		static const Unicode::String subject_auction = Unicode::narrowToWide("@" + AuctionStringIds::subjectAuctionBuyer.getCanonicalRepresentation());
		static const Unicode::String subject_instant = Unicode::narrowToWide("@" + AuctionStringIds::subjectInstantBuyer.getCanonicalRepresentation());
		static const Unicode::String subject_vendor = Unicode::narrowToWide("@" + AuctionStringIds::subjectVendorBuyer.getCanonicalRepresentation());
		
		Unicode::String subject;

		std::string planetName;
		std::string regionName;
		std::string marketName;

		AuctionManager::separateLocation (location, planetName, regionName, marketName);

		//-- it is a vendor sale
		if (!_strnicmp (marketName.c_str (), "vendor:", 7))
		{
			subject = subject_vendor;
		}
		else
		{
			if (immediate)
				subject = subject_instant;
			else
				subject = subject_auction;
		}

		Unicode::String oob;
		OutOfBandPackager::pack(pp, -1, oob);
		
		ProsePackage pp_location;
		pp_location.stringId   = AuctionStringIds::buyer_success_location;
		pp_location.target.str = Unicode::narrowToWide (regionName);
		pp_location.other.str  = Unicode::narrowToWide ("@planet_n:") + Unicode::narrowToWide (planetName);
		OutOfBandPackager::pack (pp_location, -1, oob);

		WaypointDataBase const &waypoint = getWaypointData(location, itemName);

		OutOfBandPackager::pack(waypoint, -3, oob);

		Chat::sendPersistentMessage(auctionSender, buyer, subject, Unicode::emptyString, oob); 
	}

	//----------------------------------------------------------------------

	void sendAuctionFailedToSeller(
		const std::string &     seller, 
		const std::string &     locationString,
		const Unicode::String & itemName,
		const NetworkId &       itemId
	)
	{
		UNREF (locationString);

		ProsePackage pp;
			
		pp.stringId  = AuctionStringIds::sellerAuctionFail;
		pp.other.id  = itemId;
		pp.other.str = itemName;

		static const Unicode::String subject = Unicode::narrowToWide("@" + AuctionStringIds::subjectAuctionUnsuccessful.getCanonicalRepresentation());
		Unicode::String oob;
		OutOfBandPackager::pack(pp, -1, oob);
		Chat::sendPersistentMessage(auctionSender, seller, subject, Unicode::emptyString, oob); 
	}

	//----------------------------------------------------------------------

	void sendAuctionFailedToBidder(
		const std::string &     seller, 
		const NetworkId &       sellerId,
		const std::string &     buyer,
		const std::string &     locationString,
		const Unicode::String & itemName,
		const NetworkId &       itemId
	)
	{
		UNREF (locationString);

		ProsePackage pp;
			
		pp.stringId   = AuctionStringIds::buyerAuctionFail;
		pp.target.id  = sellerId;
		pp.target.str = Unicode::narrowToWide(seller);

		pp.other.id   = itemId;
		pp.other.str  = itemName;

		static const Unicode::String subject = Unicode::narrowToWide("@" + AuctionStringIds::subjectAuctionCancelled.getCanonicalRepresentation());		
		Unicode::String oob;
		OutOfBandPackager::pack(pp, -1, oob);
		Chat::sendPersistentMessage(auctionSender, buyer, subject, Unicode::emptyString, oob); 
	}

	//----------------------------------------------------------------------

	void sendAuctionBidToPreviousBidder(
		const std::string &previousBidderName, 
		const Unicode::String &itemName,
		const NetworkId &itemId
	)
	{
		ProsePackage pp;
			
		pp.stringId  = AuctionStringIds::bidderOutbid;
		pp.other.id  = itemId;
		pp.other.str = itemName;

		static const Unicode::String subject = Unicode::narrowToWide("@" + AuctionStringIds::subjectAuctionOutbid.getCanonicalRepresentation());
		Unicode::String oob;
		OutOfBandPackager::pack(pp, -1, oob);
		Chat::sendPersistentMessage(auctionSender, previousBidderName, subject, Unicode::emptyString, oob); 
	}

	//----------------------------------------------------------------------

	void sendItemExpiredToOwner(
		const std::string &ownerName, 
		const Unicode::String &itemName,
		const NetworkId &itemId
	)
	{
		ProsePackage pp;
			
		pp.stringId  = AuctionStringIds::itemExpired;
		pp.other.id  = itemId;
		pp.other.str = itemName;

		static const Unicode::String subject = Unicode::narrowToWide("@" + AuctionStringIds::subjectAuctionItemExpired.getCanonicalRepresentation());
		Unicode::String oob;
		OutOfBandPackager::pack(pp, -1, oob);
		Chat::sendPersistentMessage(auctionSender, ownerName, subject, Unicode::emptyString, oob); 
	}

}
// ----------------------------------------------------------------------

// todo: might want to add pending message q here if connectivity problems are frequent
// by passing in message that failed. Also would add q send mechanism on reconnect here

void CommoditiesMarket::getCommoditiesServerConnection()
{
	// wait until the game server has received object ids before connecting
	// to the commodities server, because as soon as the game server connects
	// to the commodities server, the commodities server will start sending
	// messages which will cause the game server to request an object
	// id in the processing of the message, and we want to make sure
	// that we have object ids available when that happens, or the game server
	// will FATAL()
	if (ObjectIdManager::hasAvailableObjectId())
	{
		s_market = new CommoditiesServerConnection(ConfigServerGame::getCommoditiesServerServiceBindInterface(), static_cast<unsigned short>(ConfigServerGame::getCommoditiesServerServiceBindPort()));
		s_timeMarketConnectionCreated = ::time(nullptr);
	}
}

// ----------------------------------------------------------------------

int CommoditiesMarket::getCommoditiesServerConnectionAgeSeconds()
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return 0;

	if (s_timeMarketConnectionCreated <= 0)
		return 0;

	if (s_market == nullptr)
		return 0;

	return static_cast<int>(::time(nullptr) - s_timeMarketConnectionCreated);
}

// ----------------------------------------------------------------------

bool CommoditiesMarket::isCommoditiesServerAvailable()
{
	return (getCommoditiesServerConnectionAgeSeconds() > 30);
}

// ----------------------------------------------------------------------

void CommoditiesMarket::install()
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	FATAL(s_market, ("CommoditiesMarket already installed."));
	getCommoditiesServerConnection();
	time_t t = time(0);
	
	if (s_market)
	{
		s_market->SetGameTime(t);
	}
	else
	{
		//DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send SetGameTime."));
		
		getCommoditiesServerConnection(); //attempt to reconnect to commodities server
	}
	
	s_pendingSalesTax.clear();
}

// ----------------------------------------------------------------------

void CommoditiesMarket::remove()
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	//FATAL(!s_market, ("CommoditiesMarket not installed."));
	try
	{
		s_market = 0;
		s_timeMarketConnectionCreated = 0;

		for(std::map<NetworkId, SalesTaxInfo*>::iterator i = s_pendingSalesTax.begin(); i != s_pendingSalesTax.end(); ++i)
		{
			delete i->second;
		}
		s_pendingSalesTax.clear();
	}
	catch (...)
	{
		;
	}

	for(std::map<NetworkId, QueuedQuery *>::iterator iter = s_queryTime.begin(); iter != s_queryTime.end(); ++iter)
	{
		delete iter->second;
	}
	s_queryTime.clear();
}

// ----------------------------------------------------------------------

void CommoditiesMarket::closeCommoditiesServerConnection()
{
	if (s_market)
	{
		s_market->disconnect();
		s_market = 0;
		s_timeMarketConnectionCreated = 0;
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::giveTime()
{
	static unsigned long reconnectTime =0;
	
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	if (s_market)
	{
		s_market->GiveTime();
	}
	else
	{
		if (Clock::timeSeconds() - ConfigServerGame::getCommoditiesServerReconnectIntervalSec() > reconnectTime)
		{			
			getCommoditiesServerConnection(); //attempt to reconnect to commodities server
			reconnectTime = Clock::timeSeconds();
		}
	}
}

// ----------------------------------------------------------------------

namespace Auction
{
	Unicode::String getItemAuctionName(const ServerObject *item, const Unicode::String &itemLocalizedName)
	{
		if (!item)
		{
			return Unicode::emptyString;
		}
		const Unicode::String objectName = ((itemLocalizedName.size() > 0) ? itemLocalizedName : item->getEncodedObjectName());
		const ResourceContainerObject * const resourceContainer = dynamic_cast<const ResourceContainerObject *>(item);
		if (resourceContainer)
		{
			ResourceTypeObject const * const rto = resourceContainer->getResourceType();
			if (rto)
			{
				char buf[128];
				const size_t buf_size = sizeof (buf);
				snprintf(buf, buf_size, " (%s) %6d", rto->getResourceName().c_str(), resourceContainer->getQuantity());

				return (objectName + uint16(0) + Unicode::narrowToWide(buf));
			}

			return objectName;
		}
		else
		{

			const TangibleObject * const tangible = item->asTangibleObject ();
			if (tangible && tangible->getCount ())
			{
				char buf[32];
				const size_t buf_size = sizeof (buf);

				snprintf(buf, buf_size, " %6d", tangible->getCount ());				
				return (objectName + uint16(0) + Unicode::narrowToWide(buf));				
			}

			const IntangibleObject * const intangible = !tangible ? dynamic_cast<const IntangibleObject *>(item) : 0;
			if (intangible && intangible->getCount ())
			{
				char buf[32];
				const size_t buf_size = sizeof (buf);

				snprintf(buf, buf_size, " %6d", intangible->getCount ());				
				return (objectName + uint16(0) + Unicode::narrowToWide(buf));				
			}

			return objectName;
		}
		return Unicode::emptyString;
	}
}

// ----------------------------------------------------------------------


void CommoditiesMarket::auctionCreate(CreatureObject &actorCreature, NetworkId &itemId, const Unicode::String &itemLocalizedName, NetworkId &auctionContainerId, BidAmount minBid, time_t timer, const Unicode::String &userDescription, bool premium)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	AuctionResult errorCode = ar_OK;
	std::string itemRestrictedRejectionMessage;

	ServerObject * const itemTangible = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(itemId));

	ServerObject *auctionContainer = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(auctionContainerId));
	if (!auctionContainer)
	{
		errorCode = ar_INVALID_CONTAINER_ID;
	}
	else if (!itemTangible)
	{
		errorCode = ar_INVALID_ITEM_ID;
	}
	else
	{
		canAuction(*itemTangible, actorCreature, *auctionContainer, minBid, timer, 0, errorCode, itemRestrictedRejectionMessage);
	}

	if (errorCode == ar_OK)
	{
		auctionCreate(actorCreature, *itemTangible, itemLocalizedName, *auctionContainer, minBid, timer, userDescription, premium);
	}
	else
	{
		Client *client = actorCreature.getClient();
		if (client)
		{
			CreateAuctionResponseMessage msg(itemId, errorCode, itemRestrictedRejectionMessage);
			client->send(msg, true);
		}
	}	
}

// ----------------------------------------------------------------------

void CommoditiesMarket::onAddAuction(int sequence, int32 result, const NetworkId &auctionOwnerId, const std::string &ownerName, const NetworkId &itemId, const NetworkId &vendorId, const std::string &vendorName, const NetworkId &location)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	CreatureObject * const auctionCreator = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(auctionOwnerId));
	ServerObject * const item             = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(itemId));
	Client *client = nullptr;
	if (auctionCreator)
		client = auctionCreator->getClient();

	if (result == 0)
	{
		LOG("CustomerService", ("Auction:%s has put item %s for auction at location %Ld",
			PlayerObject::getAccountDescription(auctionOwnerId).c_str(), ServerObject::getLogDescription(itemId).c_str(), location.getValue()));
		if ((item) && (item->getVolume() > 1))
		{
			Container *container = ContainerInterface::getContainer(*item);
			if (container)
			{
				for (ContainerIterator containerIterator = container->begin(); containerIterator != container->end(); ++containerIterator)
				{
					ServerObject * containedObject = safe_cast<ServerObject *>((*containerIterator).getObject());
					if (containedObject)
					{
						LOG("CustomerService", ("Auction:%s has put item %s for auction at location %Ld inside container %s",
							PlayerObject::getAccountDescription(auctionOwnerId).c_str(), 
							ServerObject::getLogDescription(containedObject->getNetworkId()).c_str(), 
							location.getValue(),
							ServerObject::getLogDescription(itemId).c_str()
						));
					}
				}
			}
		}
		std::unordered_map<int, int>::iterator f = s_pendingAdds.find(sequence);
		int auctionFee = 0;
		if (f != s_pendingAdds.end())
		{
			auctionFee = (*f).second;
			s_pendingAdds.erase(f);
		}
		if (auctionCreator && auctionFee > 0)
		{
			//MLS transfer the bank credits
			if (auctionFee > auctionCreator->getBankBalance())
			{
				auctionCreator->depositCashToBank(auctionFee - auctionCreator->getBankBalance());
			}
			transferBank(auctionOwnerId, location, auctionFee);
		}
	
		// we unload the item and then transfer it to the auction container in the database
		if (item)
		{
			ServerObject *container = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(location));
			if (container)
			{
				Container::ContainerErrorCode tmp = Container::CEC_Success;
				bool result = ContainerInterface::transferItemToVolumeContainer(*container,*item, nullptr, tmp);
				if (!result)
				{
					LOG("CustomerService", ("Auction: Player %s transfer of item (%Ld) to auction container (%Ld) failed with code %d",
											PlayerObject::getAccountDescription(auctionOwnerId).c_str(),
											itemId.getValue(),
											location.getValue(),
											static_cast<int>(tmp)));
					WARNING(!result, ("Transfer of item (%Ld) to auction container (%Ld) failed", itemId.getValue(), location.getValue()));
				}
			}
			else
			{
				WARNING(true, ("Tried to transfer an item sold at a commodities terminal or vendor to the container, but the container was not found.  This is very bad since the object will simply stay in the player's inventory and the object will be for sale on the market."));	
			}
		}
	}
	if (client)
	{
		AuctionResult auctionResult = (AuctionResult)result;
		if (result == ARC_TooManyAuctions)
		{
			auctionResult = ar_TOO_MANY_AUCTIONS;
		}
		else if (result == ARC_AuctionVendorLimitExceeded)
		{
			auctionResult = ar_TOO_MANY_VENDORS;
		}
		else if (result == ARC_AuctionVendorItemLimitExceeded)
		{
			auctionResult = ar_TOO_MANY_VENDOR_ITEMS;
		}		
		else if (result == ARC_InvalidBid)
		{
			auctionResult = ar_INVALID_MINIMUM_BID;
		}
		else if (result == ARC_AuctionAlreadyExists)
		{
			auctionResult = ar_ITEM_ALREADY_AUCTIONED;
			LOG("CustomerService", ("Auction:%Ld has tried to auction the item %Ld, but it failed because the item was already for auction.  This comes about when the cluster comes down and the auction is written to the DB before the item transfer occurs.",  auctionOwnerId.getValue(), itemId.getValue()));
			if (s_market)
			{
				s_market->CancelAuction(itemId, auctionOwnerId);
			}	
			else
			{
				DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send CancelAuction."));
				getCommoditiesServerConnection(); //attempt to reconnect to commodities server
			}
		}
		CreateAuctionResponseMessage msg(NetworkId(itemId.getValue()),
			auctionResult, std::string());
		client->send(msg, true);
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::auctionCreate(CreatureObject &owner, ServerObject &item, const Unicode::String &itemLocalizedName, ServerObject &auctionContainer, BidAmount minBid, time_t timer, const Unicode::String &userDescription, bool premium)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	ScriptParams params;
	params.clear();
	params.addParam(item.getNetworkId());
	if (owner.getScriptObject()->trigAllScripts(Scripting::TRIG_AUCTION_ITEM, params) != SCRIPT_CONTINUE)
	{

		Client *client = owner.getClient();
		if (client)
		{
			AuctionResult errorCode = (AuctionResult)params.getIntParam(1);
			CreateAuctionResponseMessage msg(item.getNetworkId(), errorCode, std::string());
			client->send(msg, true);
		}
		return;	
	}

	params.clear();
	params.addParam(owner.getNetworkId());
	params.addParam(auctionContainer.getNetworkId());
	params.addParam(item.getNetworkId());
	params.addParam(premium);
	params.addParam(-1);
	owner.getScriptObject()->trigAllScripts(Scripting::TRIG_REQUEST_AUCTION_FEE, params);

	int auctionFee = params.getIntParam(4);

	if (auctionFee <= 0)
	{
		auctionFee = ADD_AUCTION_COST;
	}

	int flags = 0;
	if (premium)
	{
		flags |= AUCTION_PREMIUM_AUCTION;
	}
	const TangibleObject *tangibleObject = item.asTangibleObject();
	if (tangibleObject && tangibleObject->hasCondition(ServerTangibleObjectTemplate::C_magicItem))
	{
		flags |= AUCTION_MAGIC_ITEM;
	}

	AuctionResult errorCode = ar_OK;
	std::string itemRestrictedRejectionMessage;

	bool canSell = canAuction(item, owner, auctionContainer, minBid, timer, auctionFee, errorCode, itemRestrictedRejectionMessage);
	
	Client *client = owner.getClient();
	const NetworkId & itemId = item.getNetworkId();
	if (canSell && errorCode == ar_OK)
	{
		if (item.getOwnerId() != owner.getNetworkId())
		{
			item.setOwnerId(owner.getNetworkId());
		}
		
		timer = time(0) + timer;

		const Unicode::String objectName = Auction::getItemAuctionName(&item, itemLocalizedName);

		//--- Build attributes
		ServerObject::AttributeVector attributes;
		item.getAttributesForAuction(attributes);

		int itemSize = static_cast<ServerObject &>(item).getVolume();
		
		if (s_market)
		{	
			const int got = item.getGameObjectType();
			int itemObjectTemplateId = 0;

			if (got == SharedObjectTemplate::GOT_misc_factory_crate)
				itemObjectTemplateId = getCrateContainedItemObjectTemplateId(item);

			if (itemObjectTemplateId == 0)
				itemObjectTemplateId = getItemObjectTemplateId(item);

			int sequence = s_market->AddAuction(
				owner.getNetworkId(),
				Unicode::wideToNarrow(owner.getAssignedObjectName()),
				minBid,
				timer,
				itemId,
				objectName.size(), objectName.data(),
				got,
				itemObjectTemplateId,
				timer+ConfigServerGame::getUnclaimedAuctionItemDestroyTimeSec(),
				auctionContainer.getNetworkId(),
				getLocationString(auctionContainer),
				flags,
				userDescription.size(), userDescription.data(),
				attributes,
				itemSize,
				owner.getModValue("manage_vendor"),
				owner.getModValue("vendor_item_limit"));
			s_pendingAdds[sequence] = auctionFee;
		}
		else
		{
			DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send AddAuction."));
			
			getCommoditiesServerConnection(); //attempt to reconnect to commodities server
		}
		
	}
	else if (client)
	{
		CreateAuctionResponseMessage msg(itemId, errorCode, itemRestrictedRejectionMessage);
		client->send(msg, true);
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::transferVendorItemFromStockroom(CreatureObject &owner, NetworkId &itemId, const Unicode::String &itemLocalizedName, BidAmount price, time_t timer, const Unicode::String &userDescription)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	AuctionResult errorCode = ar_OK;

	// price of -1 means to use the current price and description of the item in the stockroom
	if ((price < 0) && (price != -1))
	{
		errorCode = ar_INVALID_MINIMUM_BID;
	}

	if (price > MAX_VENDOR_PRICE)
	{
		errorCode = ar_PRICE_TOO_HIGH;
	}

	if (timer <= 0)
	{
		errorCode = ar_INVALID_AUCTION_LENGTH;
	}

	Client *client = owner.getClient();

	if (errorCode == ar_OK)
	{
		timer = time(0) + timer;

		if (s_market)
		{
			static std::vector<std::pair<std::string, Unicode::String> > const emptyAttributes;
			
			s_market->AddImmediateAuction(
				owner.getNetworkId(),
				Unicode::wideToNarrow(owner.getAssignedObjectName()),
				price,
				timer,
				itemId,
				itemLocalizedName.size(),
				itemLocalizedName,
				0,
				0,
				timer+ConfigServerGame::getUnclaimedAuctionItemDestroyTimeSec(),
				zeroNetworkId,
				std_blank,
				AUCTION_VENDOR_TRANSFER,
				userDescription.size(),
				userDescription,
				emptyAttributes,
				0,  // will get set based on old itemSize value
				owner.getModValue("manage_vendor"),
				owner.getModValue("vendor_item_limit"));
		}
		else
		{

			DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send AddImmediateAuction."));

			getCommoditiesServerConnection(); //attempt to reconnect to commodities server
		}
			
	}
	else if (client)
	{
		CreateAuctionResponseMessage msg(itemId, errorCode, std::string());
		client->send(msg, true);
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::auctionCreateImmediate(CreatureObject &actorCreature, NetworkId &itemId, const Unicode::String &itemLocalizedName, NetworkId &auctionContainerId, BidAmount price, time_t timer, const Unicode::String &userDescription, bool premium)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	AuctionResult errorCode = ar_OK;
	std::string itemRestrictedRejectionMessage;

	ServerObject *itemTangible = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(itemId));
	ServerObject * const auctionContainer = safe_cast<ServerObject*> (NetworkIdManager::getObjectById(auctionContainerId));

	if (!auctionContainer)
	{
		errorCode = ar_INVALID_CONTAINER_ID;
	}
	else if (!itemTangible)
	{
		errorCode = ar_INVALID_ITEM_ID;
	}
	else
	{
		canAuction(*itemTangible, actorCreature, *auctionContainer, price, timer, 0, errorCode, itemRestrictedRejectionMessage);
	}

	if (errorCode == ar_OK)
	{
		auctionCreateImmediate(actorCreature, *itemTangible, itemLocalizedName, *auctionContainer, price, timer, userDescription, premium);
	}
	else
	{
		Client *client = actorCreature.getClient();
		if (client)
		{
			CreateAuctionResponseMessage msg(itemId, errorCode, itemRestrictedRejectionMessage);
			client->send(msg, true);
		}
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::auctionCreateImmediate(CreatureObject &owner, ServerObject &item, const Unicode::String &itemLocalizedName, ServerObject &auctionContainer, BidAmount price, time_t timer, const Unicode::String &userDescription, bool premium)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	AuctionResult errorCode = ar_OK;
	std::string itemRestrictedRejectionMessage;

	ScriptParams params;
	params.clear();
	params.addParam(item.getNetworkId());
	if (owner.getScriptObject()->trigAllScripts(Scripting::TRIG_AUCTION_ITEM, params) != SCRIPT_CONTINUE)
	{
		Client *client = owner.getClient();
		if (client)
		{
			AuctionResult errorCode = (AuctionResult)params.getIntParam(1);
			CreateAuctionResponseMessage msg(item.getNetworkId(), errorCode, std::string());
			client->send(msg, true);
		}
		return;	
	}

	params.clear();
	params.addParam(owner.getNetworkId());
	params.addParam(auctionContainer.getNetworkId());
	params.addParam(item.getNetworkId());
	params.addParam(premium);
	params.addParam(ADD_AUCTION_COST);
	owner.getScriptObject()->trigAllScripts(Scripting::TRIG_REQUEST_AUCTION_FEE, params);

	int auctionFee = params.getIntParam(4);
	if (auctionFee <= 0)
	{
		auctionFee = ADD_AUCTION_COST;
	}
	int flags = 0;
	if (premium)
	{
		flags |= AUCTION_PREMIUM_AUCTION;
	}

	const TangibleObject * const tangibleObject = item.asTangibleObject();
	if (tangibleObject && tangibleObject->hasCondition(ServerTangibleObjectTemplate::C_magicItem))
	{
		flags |= AUCTION_MAGIC_ITEM;
	}

	// auction is a vendor offer
	if (auctionContainer.isVendor())
	{
		NetworkId vendorOwnerId;
		if (!auctionContainer.getObjVars().getItem("vendor_owner", vendorOwnerId))
			vendorOwnerId = auctionContainer.getOwnerId();

		if (owner.getNetworkId() != vendorOwnerId)
		{
			flags |= AUCTION_OFFERED_ITEM;
		}
	}

	bool canSell = canAuction(item, owner, auctionContainer, price, timer, auctionFee, errorCode, itemRestrictedRejectionMessage);
	
	Client * const client = owner.getClient();
	NetworkId itemId(item.getNetworkId());
	if (canSell && errorCode == ar_OK)
	{
		// in order to auction an item, the auctioner must:
		//   be the owner of the item
		//   be the firstParentInWorld of the item
		
		if (item.getOwnerId() != owner.getNetworkId())
		{
			item.setOwnerId(owner.getNetworkId());
		}
		
		timer = time(0) + timer;

		const Unicode::String objectName = Auction::getItemAuctionName(&item, itemLocalizedName);

		ServerObject::AttributeVector attributes;
		item.getAttributesForAuction(attributes);
		
		int itemSize = static_cast<ServerObject &>(item).getVolume();
		
		if (s_market)
		{	
			const int got = item.getGameObjectType();
			int itemObjectTemplateId = 0;

			if (got == SharedObjectTemplate::GOT_misc_factory_crate)
				itemObjectTemplateId = getCrateContainedItemObjectTemplateId(item);

			if (itemObjectTemplateId == 0)
				itemObjectTemplateId = getItemObjectTemplateId(item);

			const int sequence = s_market->AddImmediateAuction(
				owner.getNetworkId(),
				Unicode::wideToNarrow(owner.getAssignedObjectName()),
				price,
				timer,
				itemId,
				objectName.size(), objectName,
				got,
				itemObjectTemplateId,
				timer+ConfigServerGame::getUnclaimedAuctionItemDestroyTimeSec(),
				auctionContainer.getNetworkId(),
				getLocationString(auctionContainer),
				flags,
				userDescription.size(), userDescription.data(),
				attributes,
				itemSize,
				owner.getModValue("manage_vendor"),
				owner.getModValue("vendor_item_limit"));
			s_pendingAdds[sequence] = auctionFee;
		}
		else
		{

			DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send AddImmediateAuction."));

			getCommoditiesServerConnection(); //attempt to reconnect to commodities server
		}
		
	} else if (client)
	{
		CreateAuctionResponseMessage msg(itemId, errorCode, itemRestrictedRejectionMessage);
		client->send(msg, true);
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::auctionCreatePermanent(const std::string &, const ServerObject &, const ServerObject &, BidAmount , const Unicode::String &, bool )
{
	DEBUG_WARNING(true, ("auctionCreatePermanent has been depricated and shouldn't be used.  If you see this WARNING, add a line to catch cheaters in the command in CommandCppFuncs.cpp"));

	return;
}

// ----------------------------------------------------------------------

void CommoditiesMarket::auctionBid(CreatureObject &bidder, AuctionId auctionId, BidAmount bidAmount, BidAmount maxProxyBid)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;
	
	AuctionResult errorCode = ar_OK;

	if (bidAmount <= 0 || maxProxyBid < bidAmount)
	{
		errorCode = ar_INVALID_BID;
	}
	else if (maxProxyBid > bidder.getTotalMoney())
	{
		//insufficient funds
		errorCode = ar_NOT_ENOUGH_MONEY;
	}
	else if (maxProxyBid > bidder.getBankBalance())
	{
		int additionalBankNeeded = maxProxyBid - bidder.getBankBalance();
		if ((additionalBankNeeded + bidder.getBankBalance()) > ConfigServerGame::getMaxMoney())
		{
			additionalBankNeeded = ConfigServerGame::getMaxMoney() - bidder.getBankBalance();
		}

		if (additionalBankNeeded > bidder.getCashBalance())
		{
			additionalBankNeeded = bidder.getCashBalance();
		}

		if ((additionalBankNeeded > 0) && ((additionalBankNeeded + bidder.getBankBalance()) >= maxProxyBid))
			bidder.depositCashToBank(additionalBankNeeded);
	}

	// transfer money from player to a named account to prevent them from moving it somewhere else during the transaction
	// Resolve bug where player can transfer from their bank during an auction transaction and get things for free
	// as well as cause the seller to not receive any money
	if( errorCode == ar_OK && !transferFromBankToCommoditiesNamedAccount(bidder.getNetworkId(), maxProxyBid) )
	{
		errorCode = ar_NOT_ENOUGH_MONEY;
	}

	if (errorCode == ar_OK)
	{
		if (s_market)
		{	
			s_market->AddBid(
			NetworkId(auctionId),
			bidder.getNetworkId(),
			Unicode::wideToNarrow(bidder.getAssignedObjectName()),
			bidAmount,
			maxProxyBid);
		}
		else
		{

			DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send AddBid."));

			getCommoditiesServerConnection(); //attempt to reconnect to commodities server
		}
		//response message will be sent in onAddBid
	}
	else
	{
		Client *client = bidder.getClient();
		if (client)
		{
			NetworkId auctionNetworkId(auctionId);
			BidAuctionResponseMessage msg(auctionNetworkId, errorCode); 	
			client->send(msg, true);
		}
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::auctionCancel(CreatureObject &who, AuctionId auctionId)
{
	auctionCancel(who.getNetworkId(), auctionId);
}

//----------------------------------------------------------------------

void CommoditiesMarket::auctionCancel(const NetworkId &playerId, AuctionId auctionId)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	if (s_market)
	{
		s_market->CancelAuction (NetworkId(auctionId), playerId);
	}
	else
	{

		DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send CancelAuction."));

		getCommoditiesServerConnection(); //attempt to reconnect to commodities server
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::auctionAccept(CreatureObject &who, AuctionId auctionId)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	UNREF (who);
	UNREF (auctionId);

/*
	if (s_market)
	{
		s_market->AcceptHighBid(
			auctionId,
			who.getNetworkId());
	}
	else
	{
		WARNING(true, ("[Commodities API] : No commodities server connection to send AcceptHighBid."));
		getCommoditiesServerConnection(); //attempt to reconnect to commodities server
	}
*/
}

// ----------------------------------------------------------------------

void CommoditiesMarket::auctionQueryHeaders(
	CreatureObject &who,
	int requestId,
	int type,
	int category,
	bool categoryExactMatch,
	int itemTemplateId,
	const Unicode::String &textFilterAll,
	const Unicode::String &textFilterAny,
	int priceFilterMin,
	int priceFilterMax,
	bool priceFilterIncludesFee,
	const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch,
	AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny,
	const NetworkId &container,
	int locationSearchType,
	bool searchMyVendorsOnly,
	uint16 queryOffset)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	// only allow a player to have one auction query header request at a time to prevent
	// overloading the commodities server with a lot of backlogged request from the
	// same player when the commodities server cannot process the request faster than
	// the players can click the GUI; the only exception to this is if the outstanding
	// request has exceeded a certain configurable time limit; this is so that if somehow
	// the request got lost, we don't lock the player out of the commodities server forever
	unsigned long const timeCurrent = Clock::timeSeconds();
	std::map<NetworkId, QueuedQuery *>::iterator iter = s_queryTime.find(who.getNetworkId());
	if (iter != s_queryTime.end())			
	{
		if ((iter->second->previousQueryTime + ConfigServerGame::getCommoditiesQueryTimeoutSec()) > timeCurrent)
		{
			if (ConfigServerGame::getCommoditiesShowAllDebugInfo())
				LOG("AuctionList", ("Queueing auctionQueryHeaders request %d for %s because there is already an outstanding request that is %lus old", requestId, who.getNetworkId().getValueString().c_str(), (timeCurrent - iter->second->previousQueryTime)));

			// hang on to the most recent request, because we want to service
			// the most recent request after the previous request has been serviced
			iter->second->requestId = requestId;
			iter->second->type = type;
			iter->second->category = category;
			iter->second->categoryExactMatch = categoryExactMatch;
			iter->second->itemTemplateId = itemTemplateId;
			iter->second->textFilterAll = textFilterAll;
			iter->second->textFilterAny = textFilterAny;
			iter->second->priceFilterMin = priceFilterMin;
			iter->second->priceFilterMax = priceFilterMax;
			iter->second->priceFilterIncludesFee = priceFilterIncludesFee;
			iter->second->advancedSearch = advancedSearch;
			iter->second->advancedSearchMatchAllAny = advancedSearchMatchAllAny;
			iter->second->container = container;
			iter->second->locationSearchType = locationSearchType;
			iter->second->searchMyVendorsOnly = searchMyVendorsOnly;
			iter->second->queryOffset = queryOffset;

			return;
		}
		else
		{
			// previous request has exceeded timeout, so send this request out
			if (ConfigServerGame::getCommoditiesShowAllDebugInfo())
				LOG("AuctionList", ("Sending auctionQueryHeaders request %d for %s even though there is already an outstanding request because outstanding request is %lus old which exceeds the request timeout threshold of %ds", requestId, who.getNetworkId().getValueString().c_str(), (timeCurrent - iter->second->previousQueryTime), ConfigServerGame::getCommoditiesQueryTimeoutSec()));

			// no longer need to track the request that timed out
			delete iter->second;
			s_queryTime.erase(iter);
		}
	}

	++s_nextRequestId;

	ServerObject * const auctionContainer = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(container));

	if (!auctionContainer)
	{
		Client * const client = who.getClient();
		if (client)
		{
			static std::vector<Auction::ItemDataHeader> auctionDataVector;
			const AuctionQueryHeadersResponseMessage message(requestId, type, auctionDataVector, queryOffset, false);
			client->send(message, true);	
		}
		return;
	}
	else
	{
		ScriptParams params;
		params.clear();
		params.addParam(who.getNetworkId());
		params.addParam(container);
		who.getScriptObject()->trigAllScripts(Scripting::TRIG_QUERY_AUCTIONS, params);
	}

	if (s_market)
	{	
		ServerObject* vendorObject = auctionContainer->getBazaarContainer();
		NetworkId vendorId;
		if (!vendorObject)
			vendorId = zeroNetworkId;
		else
			vendorId = vendorObject->getNetworkId();
		DEBUG_REPORT_LOG(true, ("  [AuctionQueryHeadersMessage]: container = %s, vendorId = %s", container.getValueString().c_str(), vendorId.getValueString().c_str()));

		std::string searchStringPlanet;
		std::string searchStringRegion;
		NetworkId searchAuctionLocationId;
		getLocationSearchString(auctionContainer, locationSearchType, searchStringPlanet, searchStringRegion, searchAuctionLocationId);

		s_nextRequestId = s_market->QueryAuctionHeaders(
		s_nextRequestId,
		who.getNetworkId(),
		vendorId,
		type,
		category,
		categoryExactMatch,
		itemTemplateId,
		textFilterAll,
		textFilterAny,
		priceFilterMin,
		priceFilterMax,
		priceFilterIncludesFee,
		advancedSearch,
		advancedSearchMatchAllAny,
		searchStringPlanet,
		searchStringRegion,
		searchAuctionLocationId,
		searchMyVendorsOnly,
		// if the player has the "commodities.overrideVendorSearchFlag" objvar and is in god mode,
		// set the following flag that will override the vendor specific "searchable" flag,
		// which then allows all vendors to be searched; this is useful for support/debugging purpose
		(who.getClient() && who.getClient()->isGod() && who.getObjVars().hasItem("commodities.overrideVendorSearchFlag")),
		queryOffset);
	}
	else
	{

		DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send QueryAuctionHeaders."));
		
		getCommoditiesServerConnection(); //attempt to reconnect to commodities server
	}

	// track the request
	QueuedQuery * queuedQuery = new QueuedQuery();
	queuedQuery->previousQueryTime = timeCurrent;

	s_queryTime[who.getNetworkId()] = queuedQuery;

	s_queries.insert(std::make_pair(s_nextRequestId, std::make_pair(CachedNetworkId(who), requestId)));
}

// ----------------------------------------------------------------------

void CommoditiesMarket::auctionRetrieve(CreatureObject &who, AuctionId auctionId, NetworkId const &itemId, ServerObject &auctionContainer)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	ServerObject * const item = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(itemId));
	LOG("AuctionRetrieval", ("CommoditiesMarket::attempt to load object %s from DB for retrieval", itemId.getValueString().c_str()));
	if (item)
	{
		s_pendingLoads.insert(std::make_pair(itemId,
			ItemLoadPair(who.getNetworkId(), getLocationString(auctionContainer))));
		checkPendingLoads(itemId);
	}
	else
	{
		auctionContainer.loadContainedObjectFromDB(itemId);		
		s_pendingLoads.insert(std::make_pair(itemId,
			ItemLoadPair(who.getNetworkId(), getLocationString(auctionContainer))));
		//Note, once it is loaded we'll check volume and then call GetItem.
	}

}

// ----------------------------------------------------------------------

void CommoditiesMarket::getAuctionDetails(CreatureObject &who, const NetworkId &itemId)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;
	if (s_market)
	{
		s_market->GetItemDetails(0, who.getNetworkId(), itemId);
	}
	else
	{

		DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send GetItemDetails."));

		getCommoditiesServerConnection(); //attempt to reconnect to commodities server
	}
}
// ----------------------------------------------------------------------

void CommoditiesMarket::getVendorValue(ServerObject &container)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;
	if (s_market)
	{
		s_market->GetVendorValue(getLocationString(container));
	}
	else
	{
		DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send GetVendorValue."));
		
		getCommoditiesServerConnection(); //attempt to reconnect to commodities server
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::createVendorMarket(const CreatureObject &who, ServerObject &container, int entranceCharge)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;
	
	if (s_market)
	{
		s_market->CreateVendorMarket(who.getNetworkId(), getLocationString(container), who.getModValue("manage_vendor"), entranceCharge);
	}
	else
	{
		DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send CreateVendorMarket."));

		getCommoditiesServerConnection(); //attempt to reconnect to commodities server
	}

	LOG("CustomerService", ("Auction:Player %s created vendor '%s'.", PlayerObject::getAccountDescription(who.getNetworkId()).c_str(), getLocationString(container).c_str()));
}

// ----------------------------------------------------------------------

void CommoditiesMarket::destroyVendorMarket(const NetworkId &playerId, ServerObject &container)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;

	LOG("CustomerService", ("Auction:Player %s deleted vendor '%s'.", PlayerObject::getAccountDescription(playerId).c_str(), getLocationString(container).c_str()));

	if (s_market)
	{
		s_market->DestroyVendorMarket(playerId, std_blank, getLocationString(container));
	}
	else
	{

		DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send DestroyVendorMarket."));

		getCommoditiesServerConnection(); //attempt to reconnect to commodities server
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::deleteAuctionLocation(const NetworkId& locationId, const std::string& whoRequested)
{

	LOG("CustomerService", ("Auction: Player %s deleted vendor '%s'.", whoRequested.c_str(), locationId.getValueString().c_str()));

	if (s_market)
	{
		s_market->deleteAuctionLocation(locationId, whoRequested);
	}
	else
	{
		DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send DestroyVendorMarket."));

		getCommoditiesServerConnection(); //attempt to reconnect to commodities server
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::isVendorOwner(CreatureObject &who, const NetworkId &containerId)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;
	ServerObject *container = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(containerId));
	if (container)
	{
		if (s_market)
		{
			s_market->GetVendorOwner(who.getNetworkId(), getLocationString(*container));
		}
		else
		{

			DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send GetVendorOwner."));

			onIsVendorOwner(who.getNetworkId(), zeroNetworkId, zeroNetworkId);
			getCommoditiesServerConnection(); //attempt to reconnect to commodities server
		}
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::onAddBid(int32 result, NetworkId const &itemId, NetworkId const &newBidPlayerId, const NetworkId &previousBidderId, BidAmount newBidMaxProxy, BidAmount previousBid, const NetworkId &location, bool success, const NetworkId &auctionOwnerId, const std::string & ownerName, const std::string & previousBidderName, int itemNameLength, const Unicode::String & itemName, int32 salesTaxAmount, NetworkId const &salesTaxBankId)
{
	AuctionResult errorCode = ar_OK;
	std::string errorReason;
	if (result == ARC_AuctionDoesNotExist)
	{
		success = 0;	
		errorCode = ar_INVALID_ITEM_ID;
		//transferToBankFromCommoditiesNamedAccount(newBidPlayerId, newBidMaxProxy, "INVALID_ITEM_ID errorCode in CommoditiesMarket::onAddBid()");
		errorReason = "INVALID_ITEM_ID errorCode in CommoditiesMarket::onAddBid()";
	}
	else if (result == ARC_BidTooHigh)
	{
		success = 0;
		errorCode = ar_BID_TOO_HIGH;
		//transferToBankFromCommoditiesNamedAccount(newBidPlayerId, newBidMaxProxy, "BID_TOO_HIGH errorCode in CommoditiesMarket::onAddBid()");
		errorReason = "BID_TOO_HIGH  errorCode in CommoditiesMarket::onAddBid()";
	}
	else if (result == ARC_BidOutbid)
	{
		success = 0;
		errorCode = ar_BID_OUTBID;
		errorReason = "Refund to current bidder because bid was outbid by proxy";
	}
	else if (result == ARC_InvalidBid)
	{
		success = 0;
		errorCode = ar_INVALID_BID;
		errorReason = "Refund to current bidder because bid was invalid";
	}
	else if( !success )
	{
		errorCode = ar_INVALID_BID;
		errorReason = "Refund to current bidder because bid did not succeed";
	}
	
	if (success)
	{
		if (previousBidderId.getValue() != -1)
		{
			//Not an instant auction, send it to the bazaar/vendor
			//Money was already transfered from the player to a named account
			//transferBank(newBidPlayerId, location, newBidMaxProxy);
			if (previousBidderId != NetworkId::cms_invalid)
			{
				transferToBankFromCommoditiesNamedAccount(previousBidderId, previousBid, "Previous bidder refund.");
				//transferBank(location, previousBidderId, previousBid);
			}
		}
		else
		{

			if (salesTaxAmount > 0)
 			{
 				SalesTaxInfo * s = new SalesTaxInfo();
 				s->itemId = itemId;
 				s->buyerId = newBidPlayerId;
 				s->salesTaxAmount = salesTaxAmount;
 				s->salesTaxBankId = salesTaxBankId;
 				s_pendingSalesTax[s->itemId] = s;
 			}

		}
		if (newBidPlayerId != previousBidderId && previousBidderName.size())
		{
			Auction::sendAuctionBidToPreviousBidder(
				previousBidderName,
				itemName,
				itemId);
		}
	}
	else
	{
		transferToBankFromCommoditiesNamedAccount(newBidPlayerId, newBidMaxProxy, errorReason);
	}

	CreatureObject *owner = dynamic_cast<CreatureObject*>
		(NetworkIdManager::getObjectById(newBidPlayerId));
	if (owner)
	{
		Client *client = owner->getClient();
		if (client)
		{
			BidAuctionResponseMessage msg(itemId, errorCode); 	
			client->send(msg, true);
		}
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::onAuctionComplete(NetworkId const &itemId, NetworkId const &sellerId, NetworkId const &buyerId, BidAmount purchaseAmount, BidAmount bidMaxProxy, const NetworkId &location, bool success, bool immediate, const std::string & locationString, const std::string & ownerName, const std::string & buyerName, int32 itemNameLength, const Unicode::String & itemName, const AuctionId& auctionId, bool sendSellerMail)
{
	int salesTax = 0;	
	if (success)
	{
		LOG("CustomerService", ("Auction:player %s (%Ld) has won the auction of %s from %s at location %Ld",
			PlayerObject::getAccountDescription(buyerId).c_str(), buyerId.getValue(), ServerObject::getLogDescription(itemId).c_str(), ServerObject::getLogDescription(sellerId).c_str(), location.getValue()));
		if (!immediate)
		{
			if (bidMaxProxy > purchaseAmount)
			{
				transferToBankFromCommoditiesNamedAccount(buyerId, bidMaxProxy - purchaseAmount, "Refund amount overpaid in bid proxy to winning bidder");
				//transferBank(location, buyerId, bidMaxProxy - purchaseAmount);
			}
			
		}
 		else
 		{
 			std::map<NetworkId, SalesTaxInfo*>::iterator i = s_pendingSalesTax.find(itemId);
 			if (i != s_pendingSalesTax.end())
 			{
 				SalesTaxInfo * s = i->second;
 				if (s->salesTaxAmount > 0)
 				{
 				 	salesTax = s->salesTaxAmount;
					transferToBankFromCommoditiesNamedAccount(s->salesTaxBankId, s->salesTaxAmount, "Credit the sales tax to the city for the sale");
 					LOG("CustomerService", ("Auction:Crediting sales tax of amount %d to bankId = %Ld from immediate sale of item %Ld to player %Ld",
 											s->salesTaxAmount, s->salesTaxBankId.getValue(), s->itemId.getValue(), buyerId.getValue() ));
 					if (buyerId != s->buyerId)
 					{
 						LOG("CustomerService", ("Auction: buyer id (%s) does not match stored sales tax id of %s.  Please contact the dev team.", buyerId.getValueString().c_str(), s->buyerId.getValueString().c_str()));
 					}
 				}
 				delete s;
 				s_pendingSalesTax.erase(i);
 			}
   		}

 		transferToBankFromCommoditiesNamedAccount(sellerId, purchaseAmount - salesTax, "Credit the auction owner for the amount of the winning bid");
		std::map<NetworkId, SalesTaxInfo*>::iterator i = s_pendingSalesTax.find(itemId);
		if (i != s_pendingSalesTax.end())
		{
			SalesTaxInfo * s = i->second;
			if (buyerId != s->buyerId)
			{
				LOG("CustomerService", ("Auction: Item cancellation! buyer id (%s) does not match stored sales tax id of %s.  Please contact the dev team.", buyerId.getValueString().c_str(), s->buyerId.getValueString().c_str()));
			}
			delete s;
			s_pendingSalesTax.erase(i);
		}

 		//transferBank(location, sellerId, purchaseAmount);


		if (sendSellerMail)
		{
			Auction::sendAuctionSuccessToSeller(
				ownerName,
				buyerName,
				locationString,
				buyerId,
				itemName,
				itemId,
				purchaseAmount,
				immediate);
		}

		Auction::sendAuctionSuccessToBuyer(
			ownerName,
			sellerId,
			buyerName,
			locationString,
			itemName,
			itemId,
			purchaseAmount,
			immediate);
		if (immediate)
		{
			CreatureObject *buyer = dynamic_cast<CreatureObject*>
				(NetworkIdManager::getObjectById(buyerId));
			ServerObject *auctionContainer = dynamic_cast<ServerObject*> (NetworkIdManager::getObjectById(location));
			if (buyer && auctionContainer)
			{
//				auctionRetrieve(*buyer, auctionId, itemId, *auctionContainer);
				// auctionRetrieve(CreatureObject &who, AuctionId auctionId, NetworkId const &itemId, ServerObject &auctionContainer)
			}
		}
	}
	else
	{
		if( bidMaxProxy > 0 && buyerId != NetworkId::cms_invalid && buyerId.getValue() != -1 )
		{
			transferToBankFromCommoditiesNamedAccount(buyerId, bidMaxProxy, "Refund amount to current highest bidder because of a canceled auction");
			//transferBank(location, buyerId, bidMaxProxy);	
		}

		CreatureObject *owner = dynamic_cast<CreatureObject*>
			(NetworkIdManager::getObjectById(sellerId));
		if (owner)
		{
			Client * const client = owner->getClient();
			if (client)
			{
				LOG("CustomerService", ("Auction:%s has canceled the auction of %s at location %Ld",
					PlayerObject::getAccountDescription(sellerId).c_str(), ServerObject::getLogDescription(itemId).c_str(), location.getValue()));
				CancelLiveAuctionResponseMessage msg(itemId, ar_OK); 	
				client->send(msg, true);
			}
		}

		if (sendSellerMail)
		{
			Auction::sendAuctionFailedToSeller(
				ownerName,
				locationString,
				itemName,
				itemId);
		}
		
		if (buyerName.size() && buyerName[0] != 0)
		{
			Auction::sendAuctionFailedToBidder(
				ownerName,
				sellerId,
				buyerName,
				locationString,
				itemName,
				itemId);
		}
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::onItemExpired(NetworkId const &itemOwnerId, NetworkId const &itemId, const std::string & ownerName, int itemNameLength, const Unicode::String & itemName, const std::string &locationName, const NetworkId &locationId)
{
	FlagObjectForDeleteMessage const dm(itemId, DeleteReasons::Decay, false, false, false);
	GameServer::getInstance().sendToDatabaseServer(dm);
	LOG("CustomerService", ("Auction: Auction %s is being removed from vendor: %s (%s) because its owner: %s(%s) did not retrive it from the stockroom",
		itemId.getValueString().c_str(),
		locationId.getValueString().c_str(),
		locationName.c_str(),
		ownerName.c_str(),
		itemOwnerId.getValueString().c_str()
		));
	Auction::sendItemExpiredToOwner(
		ownerName,
		itemName,
		itemId);
}

// ----------------------------------------------------------------------

void CommoditiesMarket::checkPendingLoads(const NetworkId &itemId)
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return;
	LOG("AuctionRetrieval", ("CommoditiesMarket::response from DB for loading object %s for retrieval", itemId.getValueString().c_str()));
	std::unordered_map<NetworkId, ItemLoadPair, NetworkId::Hash>::iterator f = s_pendingLoads.find(itemId);
	if (f != s_pendingLoads.end())
	{
		CreatureObject *player = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById((*f).second.ownerId));
		ServerObject *item   = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(itemId));
		if (player && item)
		{
			ServerObject *targetContainer = player->getInventory();
			if (dynamic_cast<IntangibleObject *>(item))
			{
				targetContainer = player->getDatapad();
			}
			if (targetContainer)
			{
				VolumeContainer *inventoryContainer = ContainerInterface::getVolumeContainer(*targetContainer);
				if (!inventoryContainer)
				{
					s_pendingLoads.erase(f);
					return;
				}	

				bool transferAllowed = true;
				Container::ContainerErrorCode error = Container::CEC_Success;
				transferAllowed = ContainerInterface::canTransferTo(targetContainer, *item, nullptr, error);

				if (!transferAllowed)
				{
					//item->unload();
					//error response
					RetrieveAuctionItemResponseMessage msg(itemId, (AuctionResult)ar_INVENTORY_FULL); 	
					Client *client = player->getClient();
					if (client)
					{
						client->send(msg, true);
					}
				}
				else
				{
					LOG("AuctionRetrieval", ("CommoditiesMarket::calling GetItem for loading object %s for retrieval", itemId.getValueString().c_str()));
					if (s_market)
					{
						s_market->GetItem(itemId, player->getNetworkId(), (*f).second.location);
					}
					else
					{
						DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send GetItem."));
				
						getCommoditiesServerConnection(); //attempt to reconnect to commodities server
					}
				}
			}
		}
		else
		{
			if( item )
			{
				WARNING(!player, ("After an auction purchased item was loaded, we tried to transfer it to the purchaser, but the purchaser was not found. itemId = %Ld", itemId.getValue()));
			}

			if (!item)
			{
				// signal the Commodities system to cleanup this item.
				// If this was a canceled auction sale retrieval, then the item will simply be removed from the commodities system.
				// If this was a purchased item retrieval, then the item will be removed and the player will be reimbursed.
				// We can't do reimbursement here because we don't know how the item got into the stockroom.  Only the CommoditiesServer knows that.
				
				if (s_market)
				{
					s_market->CleanupInvalidItemRetrieval(itemId);
				}
				else
				{
					DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send CleanupInvalidItemRetrieval."));
					getCommoditiesServerConnection(); //attempt to reconnect to commodities server
				}

				LOG("CustomerService", ("Auction:%Ld has tried to retrieve an auction item %Ld, but it failed because the item could not be loaded.  The system will automatically cleanup this item and reimburse money or adjust bazaar slots depending on if this was a purchased auction item or a canceled auction sale.",  (*f).second.ownerId.getValue(), itemId.getValue()));
			}
		}
		s_pendingLoads.erase(f);
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::onGetItemReply(int32 result, int32 requestId, NetworkId  itemOwnerId, NetworkId itemId, NetworkId location)
{
	LOG("AuctionRetrieval", ("CommoditiesMarket::received onGetItemReply for loading object %s for retrieval", itemId.getValueString().c_str()));
	AuctionResult auctionResult = ar_OK;
	CreatureObject *player = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(itemOwnerId));
	Client *client = player ? player->getClient() : nullptr;
	bool transactionFailed = false;

	ServerObject* vendor = nullptr;
	ServerObject* item = nullptr;
	
	if (result == ARC_AuctionDoesNotExist)
	{
		auctionResult = ar_ITEM_NOT_IN_CONTAINER;
	}
	else if (result == ARC_NotItemOwner)
	{
		auctionResult = ar_NOT_ITEM_OWNER;
	}
	if (result == ARC_Success)
	{
		if (player)
		{
			LOG("CustomerService", ("Auction: onGetItemReply - %s has retrieved auction item %s from location %Ld",
									PlayerObject::getAccountDescription(itemOwnerId).c_str(), ServerObject::getLogDescription(itemId).c_str(), location.getValue()));
			item = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(itemId));
			ServerObject *inventory = player->getInventory();
			if (item && dynamic_cast<IntangibleObject *>(item))
			{
				inventory = player->getDatapad();
			}
			if (inventory)
			{
				if (item)
				{
					Container::ContainerErrorCode tmp = Container::CEC_Success;
					vendor = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*item));
					if (vendor && vendor->getNetworkId() == location)
					{
						bool retval = ContainerInterface::transferItemToVolumeContainer(*inventory, *item, nullptr, tmp, false);
						if (retval)
						{

							ProxyList const & proxyList  = inventory->getExposedProxyList();
							std::string vendorProxies;
							std::string playerProxies;
							char buf[32];

							for (ProxyList::const_iterator j = proxyList.begin(); j != proxyList.end(); ++j)
							{
								_itoa(*j, buf, 10);
								playerProxies += buf;
								playerProxies += ",";
							}

							if (vendor)
							{
								ProxyList const &vendorProxyList = vendor->getExposedProxyList();
								for (ProxyList::const_iterator i = vendorProxyList.begin(); i != vendorProxyList.end(); ++i)
								{
									_itoa(*i, buf, 10);
									vendorProxies += buf;
									vendorProxies += ",";
								}
							}

							TangibleObject const * const itemAsTangible = item->asTangibleObject();
							int const itemCount = itemAsTangible ? itemAsTangible->getCount() : -1;

							Container const * const itemContainer = ContainerInterface::getContainer(*item);
							int const itemContents = itemContainer ? itemContainer->getNumberOfItems() : -1;
					
							LOG("CustomerService", ("Auction: onGetItemReply - %s successfully retrieved auction item %s (count=%d,contents=%d) from vendor %s. Vendor %d(%s). Player %d(%s). Item %d.",
													PlayerObject::getAccountDescription(itemOwnerId).c_str(),
													ServerObject::getLogDescription(itemId).c_str(),
													itemCount,
													itemContents,
													vendor ? vendor->getNetworkId().getValueString().c_str() : "unknown",
													vendor ? vendor->isAuthoritative() : 0,
													vendorProxies.c_str(),
													inventory->isAuthoritative(),
													playerProxies.c_str(),
													item->isAuthoritative()));
					
						}
						else
						{
							transactionFailed = true;
							LOG("CustomerService", ("Auction: onGetItemReply - %s failed to retrieve auction item %s from location %s with error code %d",
													PlayerObject::getAccountDescription(itemOwnerId).c_str(),
													ServerObject::getLogDescription(itemId).c_str(),
													location.getValueString().c_str(),
													static_cast<int>(tmp)));
						}

					}
					else
					{
						//someone else has purchaed the item.  Need to refund
						//do NOT set transaction failed.
						//and only refund the money if the owner is not the vendor owner and the owner if the item isn't the guy we're refunding.
						//ugh, that won't work.  To dupe you just buy the item, give it away, then 'buy' it again
						
						LOG("CustomerService", ("Auction: onGetItemReply - %s failed to retrieve auction item %s from location %Ld because someone else has it.",
												PlayerObject::getAccountDescription(itemOwnerId).c_str(),
												ServerObject::getLogDescription(itemId).c_str(),
												location.getValue()));
						
					}

				}
				else
				{
					transactionFailed = true;
					LOG("CustomerService", ("Auction: onGetItemReply - item not found for player %s, item %s, location %Ld",
											PlayerObject::getAccountDescription(itemOwnerId).c_str(),
											ServerObject::getLogDescription(itemId).c_str(),
											location.getValue()));
				}
			}	
			else
			{
				transactionFailed = true;
			
				LOG("CustomerService", ("Auction: onGetItemReply - inventory not found for player %s, item %s, location %Ld",
										PlayerObject::getAccountDescription(itemOwnerId).c_str(),
										ServerObject::getLogDescription(itemId).c_str(),
										location.getValue()));
			}
		}
		else
		{
			transactionFailed = true;
			LOG("CustomerService", ("Auction: onGetItemReply - player %s not found for item %s, location %Ld",
									result,
									itemOwnerId.getValueString().c_str(),
									ServerObject::getLogDescription(itemId).c_str(),
									location.getValue()));
			
		}
	}
	else
	{
		LOG("CustomerService", ("Auction: onGetItemReply - player (result %d) not found for player %s, item %s, location %Ld",
								result,
								PlayerObject::getAccountDescription(itemOwnerId).c_str(),
								ServerObject::getLogDescription(itemId).c_str(),
								location.getValue()));
	}

	if (transactionFailed && item && vendor)
	{
		TangibleObject * tangVendor = vendor->asTangibleObject();

		if (tangVendor)
		{
			TangibleObject * tangItem = item->asTangibleObject();
			if (tangItem)
				tangItem->setOwnerId(itemOwnerId);
			restoreItem(*item, *tangVendor);
		}
	}
	
	if (client)
	{
		RetrieveAuctionItemResponseMessage msg(itemId, auctionResult); 	
		client->send(msg, true);
	}
}

//----------------------------------------------------------------------

void CommoditiesMarket::onIsVendorOwner(const NetworkId & requesterId, const NetworkId & ownerId, const NetworkId &container)
{
	IsVendorOwnerResponseMessage::VendorOwnerResult result = IsVendorOwnerResponseMessage::vor_IsNotOwner;

	CreatureObject * player = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(requesterId));
	std::string marketName;

	ServerObject * const auctionContainer = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(container));
	NetworkId resultContainer = container;
	const TangibleObject * resultContainerObject = auctionContainer ? auctionContainer->asTangibleObject() : nullptr;
	if (auctionContainer)
	{
		marketName = getLocationString(*auctionContainer);
		ServerObject * const parent = const_cast<ServerObject *>(safe_cast<const ServerObject *>(ContainerInterface::getFirstParentInWorld(*auctionContainer)));
		if (parent && parent->asCreatureObject())
		{
			if (parent->getBazaarContainer() == auctionContainer)
			{
				//We got passed the container itself on a vendor, but we want auctionContainer to be the resultContainer.
				resultContainer = parent->getNetworkId();
				resultContainerObject = parent->asTangibleObject();
			}
		}
	}

	if (ownerId == requesterId)
 	{
 		result = IsVendorOwnerResponseMessage::vor_IsOwner;
 	}
 	else if (resultContainerObject && ownerId.getValue() == 0)
 	{
 		//if we get into this state, our vendor may be fubarr'd
 		if (resultContainerObject->hasCondition (TangibleObject::C_vendor) ||
 			resultContainerObject->getGameObjectType () == SharedObjectTemplate::GOT_vendor)
 		{
 			if (player && auctionContainer && resultContainerObject->getOwnerId() == player->getNetworkId())
 			{
 				//if it is a vendor that we own fix it up
 				reinitializeVendor(*auctionContainer, player->getNetworkId());
 			}
 			else
 			{
 				LOG("CustomerService", ("Auction: %s Could not reinitialize vendor %s (owner %s)",
 					PlayerObject::getAccountDescription(player).c_str(),
 					resultContainerObject->getNetworkId().getValueString().c_str(),
 					auctionContainer ? resultContainerObject->getOwnerId().getValueString().c_str() : "invalid auction container"));
 				
 			}
 		}
 		else
 		{
 			LOG("CustomerService", ("Auction %s Invalid non-vendor object during reinitialization.", PlayerObject::getAccountDescription(player).c_str()));
 		}

 		result = IsVendorOwnerResponseMessage::vor_HasNoOwner;
 	}

	AuctionResult resultCode = ar_OK;

	if (!resultContainerObject)
		resultCode = ar_INVALID_ITEM_ID;
	else
	{
		if (!resultContainerObject->hasCondition (TangibleObject::C_vendor) &&
			resultContainerObject->getGameObjectType () != SharedObjectTemplate::GOT_vendor &&
			resultContainerObject->getGameObjectType () != SharedObjectTemplate::GOT_terminal_bazaar)
 		{
 			resultCode = ar_INVALID_ITEM_ID;
 		}

	}

	const IsVendorOwnerResponseMessage message(resultContainer, marketName, result, resultCode, Auction::maxPageSize);

	if (player)
	{
		Client * const client = player->getClient();
		if (client)
			client->send(message, true);
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::onQueryHeadersReply(int32 result, int requestId, int typeFlag, int numAuctions, const std::vector<AuctionDataHeader> & auctionData, uint32 queryOffset, bool hasMorePages)
{
	std::map<int, std::pair<CachedNetworkId, int> >::iterator i = s_queries.find(requestId);
	if (i != s_queries.end())
	{
		std::pair<CachedNetworkId, int> &queryInfo = (*i).second;
		CreatureObject * const obj = safe_cast<CreatureObject*>(queryInfo.first.getObject());
		if (obj)
		{
			Client * const client = obj->getClient();
			if (client)
			{
				std::vector<Auction::ItemDataHeader> auctionDataVector;
				auctionDataVector.resize (numAuctions);
				for (int i = 0; i < numAuctions; ++i)
				{
					const AuctionDataHeader      & header  = auctionData [i];
					Auction::ItemDataHeader & data         = auctionDataVector [i];

					data.itemId         = header.itemId;
					data.itemName       = convertSeparatorToNull(header.itemName);
					data.highBid        = static_cast<int32>(header.highBid);
					data.timer          = header.timer;
					data.buyNowPrice    = header.buyNowPrice;
					data.location       = header.location;
					data.ownerId        = header.ownerId;
					data.ownerName      = NameManager::getInstance().getPlayerFullName(header.ownerId);
					data.highBidderId   = header.highBidderId;
					data.highBidderName = NameManager::getInstance().getPlayerFullName(header.highBidderId);
					data.maxProxyBid    = static_cast<int32>(header.maxProxyBid);
					data.myBid          = static_cast<int32>(header.myBid);
					data.itemType       = header.itemType;
					data.resourceContainerClassCrc = header.resourceContainerClassCrc;
					data.flags          = header.flags;
					data.entranceCharge = header.entranceCharge;
				}
				const AuctionQueryHeadersResponseMessage message(queryInfo.second, typeFlag, auctionDataVector, static_cast<uint16>(queryOffset), hasMorePages);
				client->send(message, true);
			}
		}

		std::map<NetworkId, QueuedQuery *>::iterator iter = s_queryTime.find(queryInfo.first);
		if (iter != s_queryTime.end())
		{
			if ((ConfigServerGame::getCommoditiesShowAllDebugInfo()) && (obj))
				LOG("AuctionList", ("Received response to auctionQueryHeaders request %d for %s in %lus", queryInfo.second, obj->getNetworkId().getValueString().c_str(), (Clock::timeSeconds() - iter->second->previousQueryTime)));

			QueuedQuery * queuedQuery = iter->second;

			// must be done before calling auctionQueryHeaders()
			s_queryTime.erase(iter);

			// if there is a queued request, send it out now
			if ((queuedQuery) && (queuedQuery->container.isValid()) && (obj))
				auctionQueryHeaders(*obj, queuedQuery->requestId, queuedQuery->type, queuedQuery->category, queuedQuery->categoryExactMatch, queuedQuery->itemTemplateId, queuedQuery->textFilterAll, queuedQuery->textFilterAny, queuedQuery->priceFilterMin, queuedQuery->priceFilterMax, queuedQuery->priceFilterIncludesFee, queuedQuery->advancedSearch, queuedQuery->advancedSearchMatchAllAny, queuedQuery->container, queuedQuery->locationSearchType, queuedQuery->searchMyVendorsOnly, queuedQuery->queryOffset);

			delete queuedQuery;
		}

		s_queries.erase(i);
	}
}

// ----------------------------------------------------------------------

void CommoditiesMarket::onGetItemDetailsReply(int32 result, int32 requestId, NetworkId playerId, NetworkId itemId, int userDescriptionLength, const Unicode::String & userDescription, int oobLength, const Unicode::String & oobData, std::vector<std::pair<std::string, Unicode::String> > const & attributes)
{
	CreatureObject *owner = dynamic_cast<CreatureObject*>
		(NetworkIdManager::getObjectById(playerId));
	if (owner)
	{
		Client *client = owner->getClient();
		if (client)
		{
			Auction::ItemDataDetails details;		
			details.itemId = itemId;
			details.userDescription = userDescription;
	
			if (oobLength!=0)
			{
				// Using the deprecated oobData system to pass the object's attributes
				
				Unicode::String oobDataString;
				oobDataString = oobData;

				OutOfBandPackager::OutOfBandBaseVector oobVector =
					OutOfBandPackager::unpack(oobDataString);

				OutOfBandPackager::OutOfBandBaseVector::const_iterator it = oobVector.begin();

				if (it == oobVector.end())
				{
					WARNING(true, ("Invalid Out of Band data for Auction Details"));
					return;
				}
				OutOfBandBase *base = *it;
				if (!base)
				{
					WARNING(true, ("nullptr OOB Base for Auction Data"));
					return;
				}
				if (base->getTypeId() == OutOfBandPackager::OT_auctionToken)
				{
					const OutOfBand<AuctionToken> * const oobat =
						safe_cast<const OutOfBand<AuctionToken> *>(base);	
					const AuctionToken *const token = oobat->getObject();
					details.templateName = token->sharedTemplateName;	
					details.appearanceString = token->customizationData;
				}
				else
				{
					WARNING(true, ("Improper data order for OOB Auction Data"));
					return;
				}

				//On to the vector of attributes
				++it;
				if (it == oobVector.end())
				{
					WARNING(true, ("Invalid Out of Band data for Auction Details"));
					return;
				}
				base = *it;
				if (!base)
				{
					WARNING(true, ("nullptr OOB Base for Auction Data"));
					return;
				}
				if (base->getTypeId() == OutOfBandPackager::OT_objectAttributes)
				{
					const OutOfBand<OutOfBandPackager::AttributeVector> * const oobav =
						safe_cast<const OutOfBand<OutOfBandPackager::AttributeVector> *>(base);	
					const OutOfBandPackager::AttributeVector *const attributes =
						oobav->getObject();

					details.propertyList = *attributes;	
				}
				else
				{
					WARNING(true,
							("Improper data order for OOB Auction Data"));
					return;
				}
			}
			else
			{
				// Using the new attributes vector system		   
				// Pull out two of the attributes to send separately:, pack the rest into details.propertyList
				for (std::vector<std::pair<std::string, Unicode::String> >::const_iterator i=attributes.begin(); i!=attributes.end(); ++i)
				{
					static const std::string attributeClientSharedTemplateName("ClientSharedTemplateName");
					static const std::string attributeAppearanceData("AppearanceData");

					DEBUG_REPORT_LOG(true,("Attribute:  %s, %s",i->first.c_str(),Unicode::wideToNarrow(i->second).c_str()));
					
					if (i->first==attributeClientSharedTemplateName)
						details.templateName = Unicode::wideToNarrow(i->second);
					else if (i->first==attributeAppearanceData)
						details.appearanceString = Unicode::wideToNarrow(i->second);
					else
						details.propertyList.push_back(*i);
				}
			}

			GetAuctionDetailsResponse msg(details);
			client->send(msg, true);
		}
	}
}

//----------------------------------------------------------------------

void CommoditiesMarket::onAcceptHighBid(int32 result, NetworkId itemId, NetworkId playerId)
{
	CreatureObject *player = dynamic_cast<CreatureObject*>
		(NetworkIdManager::getObjectById(playerId));
	if (player)
	{
		Client *client = player->getClient();
		if (result == ARC_NoBids)
		{
			result = (int)ar_INVALID_BID;
		}
		else if (result == ARC_AuctionAlreadyCompleted)
		{
			result = (int)ar_AUCTION_ALREADY_COMPLETED;
		}
		if (client)
		{
			AcceptAuctionResponseMessage msg(itemId, (AuctionResult)result); 	
			client->send(msg, true);
		}
	}
}

//----------------------------------------------------------------------

void CommoditiesMarket::onGetVendorValue(const NetworkId &location, int32 value)
{
	ServerObject *locationObject = dynamic_cast<ServerObject*>
		(NetworkIdManager::getObjectById(location));
	if (locationObject)
	{
		locationObject->setObjVarItem(OBJVAR_VENDOR_VALUE, static_cast<int>(value));
	}
}

//----------------------------------------------------------------------

void CommoditiesMarket::onVendorRefuseItem(int32 result, const NetworkId & itemId, const NetworkId & vendorId, const NetworkId & itemOwnerId)
{
	UNREF(itemOwnerId);
	CreatureObject *vendor = dynamic_cast<CreatureObject*>
		(NetworkIdManager::getObjectById(vendorId));
	if (vendor)
	{
		Client *client = vendor->getClient();
		if (client)
		{
			CancelLiveAuctionResponseMessage msg(itemId, static_cast<AuctionResult>(result), true); 	
			client->send(msg, true);
		}
	}
}

//----------------------------------------------------------------------

void CommoditiesMarket::onCancelAuction(int32 result, const NetworkId & playerId, const NetworkId & itemId)
{
	AuctionResult errorCode = ar_OK;
	if (result == 0)
	{
		return;
	}

	if (result == (int)ARC_AuctionAlreadyCompleted)
	{
		errorCode = ar_AUCTION_ALREADY_COMPLETED;
	}
	else if( result == (int) ARC_VendorOwnerCanceledCompletedAuction )
	{
		errorCode = ar_AUCTION_ALREADY_COMPLETED;
		LOG("CustomerService", ("VendorCreditDupeExploit:Player %s attempted to cancel vendor sale item %s which was already sold.", PlayerObject::getAccountDescription(playerId).c_str(), ServerObject::getLogDescription(itemId).c_str() ));

	}
	else
	{
		errorCode = (AuctionResult)result;
	}

	CreatureObject *owner = dynamic_cast<CreatureObject*>
		(NetworkIdManager::getObjectById(playerId));
	if (owner)
	{
		Client *client = owner->getClient();
		if (client)
		{
			CancelLiveAuctionResponseMessage msg(itemId, errorCode); 	
			client->send(msg, true);
		}
	}
}

//----------------------------------------------------------------------

void CommoditiesMarket::onCreateVendorMarket(int32 result, const NetworkId & playerId, const NetworkId & locationId)
{
	CreatureObject *owner = dynamic_cast<CreatureObject*> (NetworkIdManager::getObjectById(playerId));
	if ((result == ARC_Success) || (result == ARC_LocationAlreadyExists))
	{
		ServerObject *vendor = dynamic_cast<ServerObject*>
			(NetworkIdManager::getObjectById(locationId));
		ServerObject *resultContainer = vendor;
		if (!vendor)
		{
			return;
		}
		
		ServerObject *parent = const_cast<ServerObject *>(dynamic_cast<const ServerObject *>(ContainerInterface::getFirstParentInWorld(*vendor)));
		if (parent && parent->asCreatureObject())
		{
			if (parent->getBazaarContainer() == vendor)
			{
				resultContainer = parent;
			}
		}

		if (resultContainer)
		{
			resultContainer->setObjVarItem(OBJVAR_VENDOR_INITIALIZED, 1);
			
			if (owner)
			{
				owner->removeObjVarItem("vendor_not_initialized");
				StringId stringId = StringId("player_structure", "vendor_initialized");
				Chat::sendSystemMessage(*owner, stringId, Unicode::emptyString);
				LOG("CustomerService", ("CreateVendor response sent to client."));
			}
		}
		else
		{
			if (owner)
			{
				StringId stringId = StringId("ui_auc", "err_vendor_terminal_no_owner");
				Chat::sendSystemMessage(*owner, stringId, Unicode::emptyString);
				LOG("CustomerService", ("CreateVendor response sent to client. - Can't find resultContainer"));
			}
		}
	}
	else if (result == ARC_LocationVendorLimitExceeded)
	{
		if (owner)
		{
			StringId stringId = StringId("player_structure", "full_vendors");
			Chat::sendSystemMessage(*owner, stringId, Unicode::emptyString);
			LOG("CustomerService", ("CreateVendor response sent to client. - Over Vendor Limit"));	
		}
	}
	else
	{
		if (owner)
		{
			StringId stringId = StringId("ui_auc", "err_vendor_terminal_no_owner");
			Chat::sendSystemMessage(*owner, stringId, Unicode::emptyString);
			LOG("CustomerService", ("CreateVendor response sent to client. - Other Error"));
		}
	}
}

//----------------------------------------------------------------------

void CommoditiesMarket::onPermanentAuctionPurchased (
	NetworkId const &,
	NetworkId const &,
	BidAmount ,
	const NetworkId &,
	const NetworkId &,
	const std::string &,
	const std::string &,
	int32 ,
	const Unicode::String & ,
	std::vector<std::pair<std::string, Unicode::String> > const &
)
{
	FATAL(true,("onPermanentAuctionPurchased is deprecated."));
}
//----------------------------------------------------------------------

void CommoditiesMarket::onCleanupInvalidItemRetrieval(  const NetworkId &itemId, const NetworkId & playerId, const NetworkId &creatorId, int32 reimburseAmt )
{

	WARNING(true, ("Received OnCleanupInvalidItemRetrieval request.  item=%Ld, player=%Ld, creator=%Ld, reimburse=%d", itemId.getValue(), playerId.getValue(), creatorId.getValue(), reimburseAmt));
	CreatureObject *player = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById(playerId));
	if( reimburseAmt>0  && playerId.getValue()>0 )
	{
		if( player )
		{
			RetrieveAuctionItemResponseMessage msg(itemId, (AuctionResult)ar_INVALID_ITEM_REIMBURSAL); 	
			Client *client = player->getClient();
			if (client)
			{
				client->send(msg, true);
			}
		}
		char buffer[100];
		snprintf(buffer,100,"%i", static_cast<int>(reimburseAmt));
		if (reimburseAmt < ConfigServerGame::getMaxReimburseAmount())
		{
			MessageToQueue::getInstance().sendMessageToC(playerId, "C++FinishBankTransfer", buffer, 0, true);
			LOG("CustomerService", ("Auction:Player %s was reimbursed as a result of an invalid auction item %Ld cleanup for the amount of %d.", PlayerObject::getAccountDescription(playerId).c_str(), itemId.getValue(), reimburseAmt));
			LOG("CustomerService", ("Reimbursal:Player %s was reimbursed as a result of an invalid auction item %Ld cleanup for the amount of %d.", PlayerObject::getAccountDescription(playerId).c_str(), itemId.getValue(), reimburseAmt));
		}
		else
		{
			LOG("CustomerService", ("Reimbursal:Player %s may need to be reimbursed as a result of an invalid auction item %Ld cleanup for the amount of %d.  It was greater than the max allowed", PlayerObject::getAccountDescription(playerId).c_str(), itemId.getValue(), reimburseAmt));			
		}

	}

	if( itemId.getValue()>0 )
	{
		if( player )
		{
			RetrieveAuctionItemResponseMessage msg(itemId, (AuctionResult)ar_ITEM_NOLONGER_EXISTS); 	
			Client *client = player->getClient();
			if (client)
			{
				client->send(msg, true);
			}
		}
		LOG("CustomerService", ("Auction:Invalid auction item %Ld was cleaned from the system when attempted to be retrieved by player %Ld.", itemId.getValue(), playerId.getValue()));
	}
}

//----------------------------------------------------------------------

void CommoditiesMarket::setSalesTax( int32 salesTax, const NetworkId &bankId, ServerObject &auctionContainer )
{
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
	{
		return;
	}

	LOG("CustomerService", ("Auction: Setting sales tax=%d and bankId=%Ld for vendor at location '%s'",
				salesTax, bankId.getValue(), getLocationString(auctionContainer).c_str()));
	if (s_market)
	{
		s_market->SetSalesTax( salesTax, bankId, getLocationString(auctionContainer));
	}
	else
	{
		WARNING(true, ("[Commodities API] : No commodities server connection to send SetSalesTax."));
		getCommoditiesServerConnection(); //attempt to reconnect to commodities server
	}
}

//------------------------------------------------------------------------------------------

void CommoditiesMarket::reinitializeVendor(ServerObject &vendor, const NetworkId& owner)
{
 	if (isReinitializing(vendor))
 		return;

 	//set an objvar that the thing is re-initializing?
 	vendor.setObjVarItem(OBJVAR_VENDOR_REINITIALIZING, 1);
 	//load contents from db
 	if (!vendor.areContentsLoaded())
	{
		// support broken vendor unpack
		if  (vendor.getObjVars().hasItem("vendor.needs_old_reinitialize"))
			vendor.setLoadContents( false );

 		vendor.loadAllContentsFromDB();
	}
 	else
 		reinitializeVendorContents(vendor);
}
//------------------------------------------------------------------------------------------

void CommoditiesMarket::reinitializeVendorContents(ServerObject &vendor)
{
	//contents have loaded
	//add all contents
	//cancel all contents
	Container * container = ContainerInterface::getContainer(vendor);
	if (!container)
		return;
	
	TangibleObject * tang = vendor.asTangibleObject();
	if (!tang)
		return;

	CreatureObject * const owner = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(tang->getOwnerId()));
	if (!owner)
		return;
	
// 	destroyVendorMarket(owner->getNetworkId(), vendor);
	ServerObject * houseObj = ContainerInterface::getTopmostContainer(vendor)->asServerObject();
	if (! houseObj)
		return;
	BuildingObject * house = houseObj->asBuildingObject();

	if (! house)
		return;

	int entranceCharge = 0;
	if (houseObj->getObjVars().hasItem("turnstile.fee"))
		houseObj->getObjVars().getItem("trunstile.fee", entranceCharge);

	createVendorMarket(*owner, vendor, entranceCharge);
	for(ContainerIterator i = container->begin(); i != container->end(); ++i)
	{
		ServerObject * obj = safe_cast<ServerObject*>((*i).getObject());
		if (obj)
		{
			restoreItem(*obj, *tang);
		}
	}
	vendor.removeObjVarItem(OBJVAR_VENDOR_REINITIALIZING);
	vendor.removeObjVarItem("vendor.needs_old_reinitialize");
}

//----------------------------------------------------------------------

bool CommoditiesMarket::restoreItem(ServerObject& item, TangibleObject & vendor)
{
	//first create the auction
	if (!ConfigServerGame::getCommoditiesMarketEnabled())
		return false;

	TangibleObject* tang = dynamic_cast<TangibleObject*>(&item);
	const NetworkId& trueOwnerId = tang ? tang->getOwnerId() : vendor.getOwnerId();
	
	CreatureObject * const owner = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(trueOwnerId));
	
	bool const needsOldReinitialize = vendor.getObjVars().hasItem("vendor.needs_old_reinitialize");

	const Unicode::String objectName = needsOldReinitialize ?
		Auction::getItemAuctionName(&item, Unicode::narrowToWide("")) : Unicode::narrowToWide("Restored Item");

	Unicode::String userDescription = Unicode::narrowToWide("Restoring lost item");

	ServerObject::AttributeVector attributes;
	item.getAttributesForAuction(attributes);

	int itemSize = item.getVolume();
	
	if (s_market)
	{
		if (owner)
		{
			const int got = item.getGameObjectType();
			int itemObjectTemplateId = 0;

			if (got == SharedObjectTemplate::GOT_misc_factory_crate)
				itemObjectTemplateId = getCrateContainedItemObjectTemplateId(item);

			if (itemObjectTemplateId == 0)
				itemObjectTemplateId = getItemObjectTemplateId(item);

			const int sequence = s_market->AddImmediateAuction(
				trueOwnerId,
				std::string("Restoring object"),
				1000,
				1000,
				item.getNetworkId(),
				objectName.size(),
				objectName.data(),
				got,
				itemObjectTemplateId,
				time(0) + ConfigServerGame::getUnclaimedAuctionItemDestroyTimeSec(),
				vendor.getNetworkId(),
				getLocationString(vendor),
				0,
				userDescription.size(),
				userDescription.data(),
				attributes,
				itemSize,
				owner->getModValue("manage_vendor"),
				owner->getModValue("vendor_item_limit"));
	
			s_pendingAdds[sequence] = 0;
		}
		else
		{
			WARNING(true, ("[Commodities API] : Could not create Owner object in restore item."));
		}
	}
	else
	{
		DEBUG_WARNING(true, ("[Commodities API] : No commodities server connection to send AddImmediateAuction."));

		getCommoditiesServerConnection(); //attempt to reconnect to commodities server
		
	}
	
	
    //then cancel it
	auctionCancel(trueOwnerId, item.getNetworkId().getValue());
	return true;
}
 
//----------------------------------------------------------------------

void CommoditiesMarket::vendorStatusChange(const NetworkId& vendorId, int status)
{
	ServerObject* vendorContainer = ServerWorld::findObjectByNetworkId(vendorId);
	if (!vendorContainer)
	{
		char buf[255];
		snprintf(buf, sizeof(buf), "%d", status);
		buf[sizeof(buf) - 1] = '\0';
		MessageToQueue::getInstance().sendMessageToC(vendorId, "C++VendorStatusChange", buf, 1, true);
		return;
	}

	ServerObject * vendor = vendorContainer;

	// traverse the containment chain until we find the vendor object
	while (vendor && !vendor->isVendor())
		vendor = safe_cast<ServerObject*>(ContainerInterface::getContainedByObject(*vendor));

	if (vendor)
	{
		if (vendor->isAuthoritative())
		{
			ScriptParams params;
			params.clear();
			params.addParam(status);
			vendor->getScriptObject()->trigAllScripts(Scripting::TRIG_VENDOR_STATUS_CHANGE, params);
		}
		else
		{
			char buf[255];
			snprintf(buf, sizeof(buf), "%d", status);
			buf[sizeof(buf) - 1] = '\0';
			MessageToQueue::getInstance().sendMessageToC(vendorId, "C++VendorStatusChange", buf, 1, true);
		}
	}
}
//----------------------------------------------------------------------

void CommoditiesMarket::vendorItemCountReply(const NetworkId& vendorId, int count, bool searchEnabled)
{
	ServerObject* vendor = ServerWorld::findObjectByNetworkId(vendorId);
	if (vendor)
	{
		ServerObject * actualVendor = safe_cast<ServerObject*>(ContainerInterface::getFirstParentInWorld(*vendor));
		if (actualVendor)
		{
			ScriptParams params;
			params.clear();
			params.addParam(count);
			if (searchEnabled)
				params.addParam(1);
			else
				params.addParam(0);
			actualVendor->getScriptObject()->trigAllScripts(Scripting::TRIG_VENDOR_ITEM_COUNT_REPLY, params);
		}
	}

}
//----------------------------------------------------------------------

void CommoditiesMarket::requestVendorItemCount(const NetworkId& vendorId)
{
	ServerObject* vendorObject = ServerWorld::findObjectByNetworkId(vendorId);
	if (!vendorObject)
		return;

	ServerObject* auctionContainer = vendorObject->getBazaarContainer();
	if (!auctionContainer)
		return;
	
	if (s_market)
	{
		QueryVendorItemCountMessage m(0, 0, auctionContainer->getNetworkId());
		s_market->send(m, true);
	}
	else
	{
		//No commodities market.  Inform the script!
		ScriptParams params;
		params.clear();
		params.addParam(-1);
		params.addParam(-1);
		vendorObject->getScriptObject()->trigAllScripts(Scripting::TRIG_VENDOR_ITEM_COUNT_REPLY, params);
	}
	
}
//----------------------------------------------------------------------

void CommoditiesMarket::requestPlayerVendorCount(const NetworkId& playerId)
{
	if (s_market)
	{
		s_market->GetPlayerVendorCount(playerId);
	}
	else
	{
		ServerObject* playerObject = ServerWorld::findObjectByNetworkId(playerId);
		if (!playerObject)
			return;
		//No commodities market.  Inform the script!
		ScriptParams params;
		params.clear();
		params.addParam(-1);
		playerObject->getScriptObject()->trigAllScripts(Scripting::TRIG_PLAYER_VENDOR_COUNT_REPLY, params);
	}
}
//----------------------------------------------------------------------

void CommoditiesMarket::playerVendorCountReply(const NetworkId& playerId, int vendorCount, const std::vector<NetworkId> vendorList)
{
	ServerObject* player = ServerWorld::findObjectByNetworkId(playerId);
	if (player)
	{
		ScriptParams params;
		params.clear();
		params.addParam(vendorCount);
		player->getScriptObject()->trigAllScripts(Scripting::TRIG_PLAYER_VENDOR_COUNT_REPLY, params);
	}

}

//----------------------------------------------------------------------

void CommoditiesMarket::updateVendorSearchOption(const NetworkId& vendorId, const bool enabled)
{
	ServerObject* vendorObject = ServerWorld::findObjectByNetworkId(vendorId);
	if (!vendorObject)
		return;

	ServerObject* auctionContainer = vendorObject->getBazaarContainer();
	if (!auctionContainer)
		return;
	
	if (s_market)
	{
		UpdateVendorSearchOptionMessage m(0, 0, auctionContainer->getNetworkId(), enabled, vendorObject->getOwnerId());
		s_market->send(m, true);
	}
	else
	{
		//No commodities market.  Inform the script!
		ScriptParams params;
		params.clear();
		params.addParam(-1);
		params.addParam(-1);
		vendorObject->getScriptObject()->trigAllScripts(Scripting::TRIG_VENDOR_ITEM_COUNT_REPLY, params);
	}
	
}

//----------------------------------------------------------------------

void CommoditiesMarket::setEntranceCharge(const NetworkId& vendorId, const int entranceCharge)
{
	ServerObject* vendorObject = ServerWorld::findObjectByNetworkId(vendorId);
	if (!vendorObject)
		return;

	ServerObject* auctionContainer = vendorObject->getBazaarContainer();
	if (!auctionContainer)
		return;
	
	if (s_market)
	{
		SetEntranceChargeMessage m(0, 0, auctionContainer->getNetworkId(), entranceCharge);
		s_market->send(m, true);
	}
	else
	{
		//No commodities market.  Inform the script!
		ScriptParams params;
		params.clear();
		params.addParam(-1);
		params.addParam(-1);
		vendorObject->getScriptObject()->trigAllScripts(Scripting::TRIG_VENDOR_ITEM_COUNT_REPLY, params);
	}
	
}

//----------------------------------------------------------------------

void CommoditiesMarket::updateItemTypeMap(int itemTypeMapVersionNumber, const std::map<int, std::set<int> > & itemTypeMap)
{
	s_itemTypeMap.clear();

	for (std::map<int, std::set<int> >::const_iterator iter = itemTypeMap.begin(); iter != itemTypeMap.end(); ++iter)
	{
		for (std::set<int>::const_iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
		{
			updateItemTypeMap(itemTypeMapVersionNumber, iter->first, *iter2);
		}
	}
}

//----------------------------------------------------------------------

void CommoditiesMarket::updateItemTypeMap(int itemTypeMapVersionNumber, int itemType, int itemTemplateId)
{
	if ((itemType == 0) || (itemTemplateId == 0))
		return;

	StringId objectName;
	int gameObjectType = 0;

	// get the server template
	const ObjectTemplate * ot = ObjectTemplateList::fetch(static_cast<uint32>(itemTemplateId));
	if (ot != nullptr)
	{
		objectName = StringId(std::string(ot->getName()));

		// get the shared template 
		const ServerObjectTemplate * serverOt = ot->asServerObjectTemplate();
		if (serverOt != nullptr)
		{
			const std::string sharedTemplateName(serverOt->getSharedTemplate());
			serverOt->releaseReference();
			serverOt = nullptr;
			ot = ObjectTemplateList::fetch(sharedTemplateName);
			if (ot != nullptr)
			{
				const SharedObjectTemplate * sharedOt = ot->asSharedObjectTemplate();
				if (sharedOt != nullptr)
				{
					gameObjectType = static_cast<int>(sharedOt->getGameObjectType());
					StringId const tempObjectName(objectName);
					objectName = sharedOt->getObjectName();
					if (!objectName.isValid())
					{
						objectName = tempObjectName;
					}
					sharedOt->releaseReference();
					sharedOt = nullptr;
				}
				else
				{
					ot->releaseReference();
					ot = nullptr;
				}
			}
		}
		else
		{
			ot->releaseReference();
			ot = nullptr;
		}
	}
	else
	{
		char buffer[64];
		snprintf(buffer, sizeof(buffer)-1, "Unknown Item (%d)", itemTemplateId);
		buffer[sizeof(buffer)-1] = '\0';

		objectName = StringId(std::string(buffer));
	}

	(s_itemTypeMap[itemType])[itemTemplateId] = std::make_pair(gameObjectType, objectName);
	s_itemTypeMapVersionNumber = itemTypeMapVersionNumber;
}

//----------------------------------------------------------------------

const std::map<int, std::map<int, std::pair<int, StringId> > > & CommoditiesMarket::getItemTypeMap()
{
	return s_itemTypeMap;
}

//----------------------------------------------------------------------

int CommoditiesMarket::getItemTypeMapVersionNumber()
{
	return s_itemTypeMapVersionNumber;
}

//----------------------------------------------------------------------

void CommoditiesMarket::updateResourceTypeMap(int resourceTypeMapVersionNumber, const std::map<int, std::set<std::string> > & resourceTypeMap)
{
	s_resourceTypeMap.clear();
	s_resourceTypeMap = resourceTypeMap;
	s_resourceTypeMapVersionNumber = resourceTypeMapVersionNumber;
}

//----------------------------------------------------------------------

void CommoditiesMarket::updateResourceTypeMap(int resourceTypeMapVersionNumber, int resourceClassCrc, const std::string & resourceName)
{
	if ((resourceClassCrc == 0) || (resourceName.empty()))
		return;

	std::set<std::string> & resourceNameList = s_resourceTypeMap[resourceClassCrc];
	if (resourceNameList.count(resourceName) == 0)
	{
		IGNORE_RETURN(resourceNameList.insert(resourceName));
		s_resourceTypeMapVersionNumber = resourceTypeMapVersionNumber;
	}
}

//----------------------------------------------------------------------

const std::map<int, std::set<std::string> > & CommoditiesMarket::getResourceTypeMap()
{
	return s_resourceTypeMap;
}

//----------------------------------------------------------------------

int CommoditiesMarket::getResourceTypeMapVersionNumber()
{
	return s_resourceTypeMapVersionNumber;
}

//----------------------------------------------------------------------

void CommoditiesMarket::updateVendorStatus(NetworkId const & vendorId, int status)
{
	ServerObject const * const vendorObject = ServerWorld::findObjectByNetworkId(vendorId);
	if (!vendorObject)
		return;

	ServerObject const * const auctionContainer = vendorObject->getBazaarContainer();
	if (!auctionContainer)
		return;

	if (s_market)
	{
		UpdateVendorStatusMessage message(auctionContainer->getNetworkId(), getLocationString(*auctionContainer), status);
		s_market->send(message, true);
	}
}

//----------------------------------------------------------------------

void CommoditiesMarket::updateVendorLocation(const ServerObject & vendor)
{
	// only needs to be done for initialized, unpacked vendor
	if (!vendor.isInWorld())
		return;

	if (!vendor.getObjVars().hasItem(OBJVAR_VENDOR_INITIALIZED))
		return;

	const ServerObject * const parent = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(vendor));
	if (!parent)
		return;

	if (!parent->asCellObject())
		return;

	if (!parent->isInWorld())
		return;

	const ServerObject * const auctionContainer = vendor.getBazaarContainer();
	if (!auctionContainer)
		return;

	if (s_market)
	{
		GenericValueTypeMessage<std::pair<NetworkId, std::string> > const message("UpdateVendorLocationMessage", std::make_pair(auctionContainer->getNetworkId(), getLocationString(*auctionContainer)));
		s_market->send(message, true);
	}
}

//----------------------------------------------------------------------

void CommoditiesMarket::sendToCommoditiesServer(const GameNetworkMessage & message)
{
	if (s_market)
		s_market->send(message, true);
}

//----------------------------------------------------------------------

const std::string & CommoditiesMarket::getCommoditiesNamedEscrowAccount()
{
	return COMMODITIES_NAMED_ACCOUNT;
}
