// ======================================================================
//
// AuctionMarket.cpp (refactor of original Commodities Market code)
// copyright (c) 2004 Sony Online Entertainment
//
// Original Author: Matt Severenson
// Refactored by  : Doug Mellencamp
//
// ======================================================================

#include "FirstCommodityServer.h"
#include "CommodityServer.h"
#include "AuctionMarket.h"
#include "GameServerConnection.h"
#include "ConfigCommodityServer.h"
#include "DatabaseServerConnection.h"
#include "StringId.h"
#include "sharedFoundation/Crc.h"
#include "sharedLog/Log.h"
#include "serverNetworkMessages/AuctionBase.h"
#include "serverNetworkMessages/CMCreateAuctionMessage.h"
#include "serverNetworkMessages/CMCreateAuctionBidMessage.h"
#include "serverNetworkMessages/CMCreateLocationMessage.h"
#include "serverNetworkMessages/CMDeleteAuctionMessage.h"
#include "serverNetworkMessages/CMDeleteLocationMessage.h"
#include "serverNetworkMessages/CMUpdateAuctionMessage.h"
#include "serverNetworkMessages/CMUpdateLocationMessage.h"
#include "sharedNetworkMessages/DeleteCharacterMessage.h"
#include "serverNetworkMessages/LoadCommoditiesMessage.h"
#include "serverNetworkMessages/OnAcceptHighBidMessage.h"
#include "serverNetworkMessages/OnAddAuctionMessage.h"
#include "serverNetworkMessages/OnAddBidMessage.h"
#include "serverNetworkMessages/OnAuctionExpiredMessage.h"
#include "serverNetworkMessages/OnCancelAuctionMessage.h"
#include "serverNetworkMessages/OnCleanupInvalidItemRetrievalMessage.h"
#include "serverNetworkMessages/OnCreateVendorMarketMessage.h"
#include "serverNetworkMessages/OnGetItemDetailsMessage.h"
#include "serverNetworkMessages/OnGetItemMessage.h"
#include "serverNetworkMessages/OnGetPlayerVendorCountMessage.h"
#include "serverNetworkMessages/OnGetVendorOwnerMessage.h"
#include "serverNetworkMessages/OnGetVendorValueMessage.h"
#include "serverNetworkMessages/OnItemExpiredMessage.h"
#include "serverNetworkMessages/OnPermanentAuctionPurchasedMessage.h"
#include "serverNetworkMessages/OnQueryAuctionHeadersMessage.h"
#include "serverNetworkMessages/OnVendorRefuseItemMessage.h"
#include "serverNetworkMessages/OnUpdateVendorSearchOptionMessage.h"
#include "serverNetworkMessages/QueryVendorItemCountMessage.h"
#include "serverNetworkMessages/OnQueryVendorItemCountReplyMessage.h"
#include "serverNetworkMessages/UpdateVendorStatusMessage.h"
#include "serverNetworkMessages/VendorStatusChangeMessage.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedGame/CommoditiesAdvancedSearchAttribute.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

#include "utf8.h"
#include "UnicodeUtils.h"

namespace AuctionMarketNamespace
{
	std::string const  g_dummyName = "INVALID_CHARACTER";
	NetworkId const zeroNetworkId(static_cast<NetworkId::NetworkIdType>(0));
	NetworkId const negOneNetworkId(static_cast<NetworkId::NetworkIdType>(-1));
	char const * vendorStatus[] = {"Active", "Empty", "Unaccessed", "EmptyAndUnaccessed", "Endangered", "Removed"};

	// ======================================================================

	void encodeOOB(const Unicode::String & str, std::string & HexString )
	{
		char buffer [5];
		Unicode::String localCopy(str);
		Unicode::String::iterator z = localCopy.begin();
		while (z != localCopy.end())
		{
			if (sprintf (buffer, "%04X", *z) < 0)
			{
				WARNING(true, ("[Commodities Server ] : Writing converted OOB hex value to buffer failed in: TaskCreateAuction::encodeOOB.\n"));
			}
			else
			{
				HexString += buffer;
			}
			z++;
		}

		DEBUG_REPORT_LOG(ConfigCommodityServer::getShowAllDebugInfo(), ("[Commodities Server ][HEX OOB IN] : %s \n", HexString.c_str()));

	}

	// ----------------------------------------------------------------------

	void GetHighBidInfo(
		Auction const * const auction,
		int & highBidOut,
		NetworkId & highBidderIdOut
	)
	{
		if (auction->IsImmediate())
		{
			highBidOut = auction->GetBuyNowPriceWithSalesTax();
			if (auction->GetHighBid())
			{
        		highBidderIdOut = auction->GetHighBid()->GetBidderId();
			}
		}
		else if (auction->GetHighBid())
		{
			highBidOut = auction->GetHighBid()->GetBid();
			highBidderIdOut = auction->GetHighBid()->GetBidderId();
		}
		else
		{
			highBidOut = auction->GetMinBid();
		}
	}

	// ----------------------------------------------------------------------

	AuctionDataHeader *MakeDataHeader(
		NetworkId const & playerId,
		Auction const * const auction,
		int const type,
		int const entranceCharge,
		AuctionBid const * const playerBid = nullptr
	)
	{
		AuctionDataHeader *header = new AuctionDataHeader;

		header->type = type;
		header->auctionId = auction->GetItem().GetItemId();
		header->itemId = auction->GetItem().GetItemId();
		header->itemNameLength = auction->GetItem().GetNameLength();
		header->itemName = auction->GetItem().GetName();
		header->minBid = auction->GetMinBid();
		header->entranceCharge = entranceCharge;

		time_t currentTime = time(0);
		if (auction->IsActive())
		{
			if (auction->GetAuctionTimer() < currentTime)
			{
				header->timer = 0;
			}
			else
			{
				header->timer = auction->GetAuctionTimer() - currentTime;
			}
		}
		else
		{
			if (auction->GetItem().GetItemTimer() < currentTime)
			{
				header->timer = 0;
			}
			else
			{
				header->timer = auction->GetItem().GetItemTimer() - currentTime;
			}
		}
		header->buyNowPrice = auction->GetBuyNowPrice();
		header->location  = auction->GetLocation().GetLocationString();
		header->ownerId = auction->GetCreatorId();

		// Determine the high bid info
		GetHighBidInfo(auction, header->highBid, header->highBidderId);

		if (type == AST_ByPlayerBids)
		{
			if (playerBid)
			{
				header->maxProxyBid = playerBid->GetMaxProxyBid();
				header->myBid = playerBid->GetBid();
			}
		}
		header->itemType =  auction->GetItem().GetCategory();
		header->resourceContainerClassCrc = auction->GetItem().GetResourceContainerClassCrc();
		header->flags = auction->GetFlags();
		if (auction->IsActive())
		{
			header->flags |= AUCTION_ACTIVE;
		}

		return header;
	}

	// ----------------------------------------------------------------------

	bool IsTextFilterMatch(
		Unicode::String const & itemName,
		Unicode::UnicodeStringVector const & textFilterAllTokens,
		Unicode::UnicodeStringVector const & textFilterAnyTokens)
	{
		bool isTextMatch = true;

		// We are doing a case insensitive search
		Unicode::String const lowerCaseName = Unicode::toLower(itemName);

		if (!textFilterAllTokens.empty())
		{
			// See if all of the tokens for the text filter can be found in the name
			Unicode::UnicodeStringVector::const_iterator i;
			for (i = textFilterAllTokens.begin(); i != textFilterAllTokens.end(); ++i)
			{
				if (lowerCaseName.find(*i) == Unicode::String::npos)
				{
					// We could not find a token so we do not match
					isTextMatch = false;
					break;
				}
			}
		}

		// If the name has all the required tokens, make sure it has
		// at least one of the "any" tokens
		if (isTextMatch)
		{
			if (!textFilterAnyTokens.empty())
			{
				// Assume we won't find a match
				isTextMatch = false;

				// See if any of the tokens for the text filter can be found in the name
				Unicode::UnicodeStringVector::const_iterator i;
				for (i = textFilterAnyTokens.begin(); i != textFilterAnyTokens.end(); ++i)
				{
					if (lowerCaseName.find(*i) != Unicode::String::npos)
					{
						// We found a token so we match
						isTextMatch = true;
						break;
					}
				}
			}
		}

		return isTextMatch;
	}

	// ----------------------------------------------------------------------

	bool IsPriceFilterMatch(
		Auction const * const auction,
		int const entranceCharge,
		int const priceFilterMin,
		int const priceFilterMax,
		bool const isPriceFilterToIncludeFee)
	{
		bool isPriceMatch = true;
		if ((priceFilterMin != 0) || (priceFilterMax != 0))
		{
			// Get the high bid info for the auction
			int       highBid;
			NetworkId highBidderId;
			GetHighBidInfo(auction, highBid, highBidderId);

			// Include the entrance fee for the price if necessary
			if (isPriceFilterToIncludeFee)
			{
				highBid += entranceCharge;
			}

			// Is this price less than the min specified?
			if ((priceFilterMin != 0) && (highBid < priceFilterMin))
			{
				isPriceMatch = false;
			}
			else
			{
				// Is this price greater than the max specified?
				if ((priceFilterMax != 0) && (highBid > priceFilterMax))
				{
					isPriceMatch = false;
				}
			}
		}

		return isPriceMatch;
	}

	// ----------------------------------------------------------------------

	struct GetItemAttributeDataRequest
	{
		std::string action;
		int requestingGameServerId;
		NetworkId requester;
		int gameObjectType;
		bool exactGameObjectTypeMatch;
		int throttleNumberItemsPerFrame;
		int numberItemsProcessed;
		NetworkId lastItemProcessed;

		// for GetItemAttributeData
		std::string outputFileName;
		std::map<int, std::pair<int, std::map<std::string, std::pair<int, std::string> > > > allItemAttributes;
		bool ignoreSearchableAttribute;

		// for GetItemAttributeDataValues
		std::string attributeName;
		int numberItemsMatchGameObjectType;
		std::map<std::string, int> attributeValue;
	};

	GetItemAttributeDataRequest * getItemAttributeDataRequest = nullptr;
	
	void processItemAttributeData(std::map<NetworkId, Auction *> const & auctions);

	// ----------------------------------------------------------------------

	void skipField(std::string const &source, unsigned int &pos, char const separator)
	{
		while (source[pos] && source[pos] != separator)
			++pos;
		if (source[pos] == separator)
			++pos;
	}

	// ----------------------------------------------------------------------

	void nextInt(std::string const &source, unsigned int &pos, int &ret, char const separator)
	{
		ret = atoi(source.c_str()+pos);
		skipField(source, pos, separator);
	}

	// ----------------------------------------------------------------------

	void nextString(std::string const &source, unsigned int &pos, std::string &ret, char const separator)
	{
		unsigned int oldPos = pos;
		while (source[pos] && source[pos] != separator)
			++pos;
		ret = source.substr(oldPos, pos-oldPos);
		if (source[pos] == separator)
			++pos;
	}

	// ----------------------------------------------------------------------

	void nextOid(std::string const &source, unsigned int &pos, NetworkId &ret, char const separator)
	{
		unsigned int oldPos = pos;
		skipField(source, pos, separator);
		ret = NetworkId(source.substr(oldPos, pos-oldPos));
	}

	// ----------------------------------------------------------------------

	void removeAuctionLocationFromList(std::map<NetworkId, AuctionLocation *> & auctionLocationList, const AuctionLocation * auctionLocation)
	{
		if (auctionLocation)
		{
			std::map<NetworkId, AuctionLocation *>::iterator const iterFind = auctionLocationList.find(auctionLocation->GetLocationId());
			if ((iterFind != auctionLocationList.end()) && (iterFind->second == auctionLocation))
			{
				auctionLocationList.erase(iterFind);
			}
		}
	}

};  // namespace AuctionMarketNamespace

// ======================================================================

void AuctionMarketNamespace::processItemAttributeData(std::map<NetworkId, Auction *> const & auctions)
{
	if (!getItemAttributeDataRequest)
		return;

	int count = 0;
	std::map<NetworkId, Auction *>::const_iterator iterAuction = auctions.upper_bound(getItemAttributeDataRequest->lastItemProcessed);
	for (; iterAuction != auctions.end(); ++iterAuction)
	{
		if (getItemAttributeDataRequest->action == "GetItemAttributeData")
		{
			bool proceed = true;
			if (getItemAttributeDataRequest->gameObjectType > 0)
			{
				if (getItemAttributeDataRequest->exactGameObjectTypeMatch)
				{
					if (iterAuction->second->GetItem().GetCategory() != getItemAttributeDataRequest->gameObjectType)
					{
						proceed = false;
					}
				}
				else
				{
					if (!GameObjectTypes::isTypeOf(iterAuction->second->GetItem().GetCategory(), getItemAttributeDataRequest->gameObjectType))
					{
						proceed = false;
					}
				}
			}

			if (proceed)
			{
				std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> const * skipAttribute = nullptr;
				std::map<std::string, std::string> const * skipAttributeAlias = nullptr;
				if (getItemAttributeDataRequest->ignoreSearchableAttribute)
				{
					std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> const & sa = CommoditiesAdvancedSearchAttribute::getSearchAttributeForGameObjectType(iterAuction->second->GetItem().GetCategory());
					if (!sa.empty())
					{
						skipAttribute = &sa;
					}

					if (skipAttribute)
					{
						std::map<std::string, std::string> const & saAlias = CommoditiesAdvancedSearchAttribute::getSearchAttributeNameAliasesForGameObjectType(iterAuction->second->GetItem().GetCategory());
						if (!saAlias.empty())
						{
							skipAttributeAlias = &saAlias;
						}
					}
				}

				std::pair<int, std::map<std::string, std::pair<int, std::string> > > & itemAttributeData = getItemAttributeDataRequest->allItemAttributes[iterAuction->second->GetItem().GetCategory()];
				++itemAttributeData.first;

				std::map<std::string, std::pair<int, std::string> > & itemAttributes = itemAttributeData.second;

				std::pair<int, std::map<std::string, std::pair<int, std::string> > > & parentItemAttributeData = getItemAttributeDataRequest->allItemAttributes[GameObjectTypes::getMaskedType(iterAuction->second->GetItem().GetCategory())];
				if (&itemAttributeData.first != &parentItemAttributeData.first)
					++parentItemAttributeData.first;

				std::map<std::string, std::pair<int, std::string> > & parentItemAttributes = parentItemAttributeData.second;

				std::vector<std::pair<std::string, Unicode::String> > const & attributes = iterAuction->second->GetAttributes();
				for (std::vector<std::pair<std::string, Unicode::String> >::const_iterator iterAttribute = attributes.begin(); iterAttribute != attributes.end(); ++iterAttribute)
				{
					if (skipAttribute && (skipAttribute->count(iterAttribute->first) >= 1))
					{
						continue;
					}

					if (skipAttributeAlias && (skipAttributeAlias->count(iterAttribute->first) >= 1))
					{
						continue;
					}

					std::map<std::string, std::pair<int, std::string> >::iterator const iterFindAttribute = itemAttributes.find(iterAttribute->first);
					if (iterFindAttribute != itemAttributes.end())
					{
						++iterFindAttribute->second.first;

						if (iterFindAttribute->second.second.empty())
							iterFindAttribute->second.second = Unicode::wideToNarrow(iterAttribute->second);
					}
					else
					{
						itemAttributes.insert(std::make_pair(iterAttribute->first, std::make_pair(1, Unicode::wideToNarrow(iterAttribute->second))));
					}

					if (&itemAttributeData.first != &parentItemAttributeData.first)
					{
						std::map<std::string, std::pair<int, std::string> >::iterator const iterFindParentAttribute = parentItemAttributes.find(iterAttribute->first);
						if (iterFindParentAttribute != parentItemAttributes.end())
						{
							++iterFindParentAttribute->second.first;

							if (iterFindParentAttribute->second.second.empty())
								iterFindParentAttribute->second.second = Unicode::wideToNarrow(iterAttribute->second);
						}
						else
						{
							parentItemAttributes.insert(std::make_pair(iterAttribute->first, std::make_pair(1, Unicode::wideToNarrow(iterAttribute->second))));
						}
					}
				}
			}
		}
		else if (getItemAttributeDataRequest->action == "GetItemAttributeDataValues")
		{
			bool proceed = true;
			if (getItemAttributeDataRequest->gameObjectType > 0)
			{
				if (getItemAttributeDataRequest->exactGameObjectTypeMatch)
				{
					if (iterAuction->second->GetItem().GetCategory() != getItemAttributeDataRequest->gameObjectType)
					{
						proceed = false;
					}
				}
				else
				{
					if (!GameObjectTypes::isTypeOf(iterAuction->second->GetItem().GetCategory(), getItemAttributeDataRequest->gameObjectType))
					{
						proceed = false;
					}
				}
			}

			if (proceed)
			{
				++getItemAttributeDataRequest->numberItemsMatchGameObjectType;

				std::vector<std::pair<std::string, Unicode::String> > const & attributes = iterAuction->second->GetAttributes();
				for (std::vector<std::pair<std::string, Unicode::String> >::const_iterator iterAttribute = attributes.begin(); iterAttribute != attributes.end(); ++iterAttribute)
				{
					if (iterAttribute->first == getItemAttributeDataRequest->attributeName)
					{
						std::string const value = Unicode::wideToNarrow(iterAttribute->second);
						std::map<std::string, int>::iterator const iterValue = getItemAttributeDataRequest->attributeValue.find(value);
						if (iterValue != getItemAttributeDataRequest->attributeValue.end())
						{
							++iterValue->second;
						}
						else
						{
							getItemAttributeDataRequest->attributeValue.insert(std::make_pair(value, 1));
						}
					}
				}
			}
		}

		++getItemAttributeDataRequest->numberItemsProcessed;

		++count;
		if (count >= getItemAttributeDataRequest->throttleNumberItemsPerFrame)
		{
			getItemAttributeDataRequest->lastItemProcessed = iterAuction->first;
			break;
		}
	}

	if (iterAuction == auctions.end())
	{
		std::string output;
		char buffer[2048];

		if (getItemAttributeDataRequest->action == "GetItemAttributeData")
		{
			for (std::map<int, std::pair<int, std::map<std::string, std::pair<int, std::string> > > >::const_iterator iterOutputGot = getItemAttributeDataRequest->allItemAttributes.begin(); iterOutputGot != getItemAttributeDataRequest->allItemAttributes.end(); ++iterOutputGot)
			{
				snprintf(buffer, sizeof(buffer)-1, "%s (count=%d)", GameObjectTypes::getCanonicalName(iterOutputGot->first).c_str(), iterOutputGot->second.first);
				buffer[sizeof(buffer)-1] = '\0';

				output += buffer;
				output += "\r\n";

				for (std::map<std::string, std::pair<int, std::string> >::const_iterator iterOutputAttribute = iterOutputGot->second.second.begin(); iterOutputAttribute != iterOutputGot->second.second.end(); ++iterOutputAttribute)
				{
					snprintf(buffer, sizeof(buffer)-1, "    %s (%s) (count=%d)", iterOutputAttribute->first.c_str(), iterOutputAttribute->second.second.c_str(), iterOutputAttribute->second.first);
					buffer[sizeof(buffer)-1] = '\0';

					output += buffer;
					output += "\r\n";
				}

				output += "\r\n";
			}

			if (output.empty())
			{
				output = "No Data";
			}
		}
		else if (getItemAttributeDataRequest->action == "GetItemAttributeDataValues")
		{
			snprintf(buffer, sizeof(buffer)-1, "%s (count=%d) attribute name=(%s)", GameObjectTypes::getCanonicalName(getItemAttributeDataRequest->gameObjectType).c_str(), getItemAttributeDataRequest->numberItemsMatchGameObjectType, getItemAttributeDataRequest->attributeName.c_str());
			buffer[sizeof(buffer)-1] = '\0';

			output = buffer;
			output += "\r\n";

			for (std::map<std::string, int>::const_iterator iter = getItemAttributeDataRequest->attributeValue.begin(); iter != getItemAttributeDataRequest->attributeValue.end(); ++iter)
			{
				snprintf(buffer, sizeof(buffer)-1, "    (%s) (count=%d)", iter->first.c_str(), iter->second);
				buffer[sizeof(buffer)-1] = '\0';

				output += buffer;
				output += "\r\n";
			}
		}

		GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(getItemAttributeDataRequest->requestingGameServerId);
		if (gameServerConn)
		{
			if (getItemAttributeDataRequest->action == "GetItemAttributeData")
			{
				GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::string>, std::string> > const msg("GetItemAttributeDataRsp", std::make_pair(std::make_pair(getItemAttributeDataRequest->requester, getItemAttributeDataRequest->outputFileName), output));
				gameServerConn->send(msg, true);
			}
			else if (getItemAttributeDataRequest->action == "GetItemAttributeDataValues")
			{
				GenericValueTypeMessage<std::pair<NetworkId, std::string> > const msg("DisplayStringForPlayer", std::make_pair(getItemAttributeDataRequest->requester, output));
				gameServerConn->send(msg, true);
			}
		}

		delete getItemAttributeDataRequest;
		getItemAttributeDataRequest = nullptr;
	}
}

// ======================================================================

using namespace AuctionMarketNamespace;

// ======================================================================

AuctionMarket::AuctionMarket()  :
Singleton<AuctionMarket>(),
m_locationIdMap(),
m_playerVendorListMap(),
m_allBazaar(),
m_bazaarByPlanet(),
m_bazaarByRegion(),
m_allSearchableVendor(),
m_searchableVendorByPlanet(),
m_searchableVendorByRegion(),
m_auctions(),
m_auctionsCountByGameObjectType(),
m_auctionsCountByGameObjectTypeChanged(),
m_auctionCountMap(),
m_resourceTreeHierarchy(),
m_itemTypeMap(),
m_itemTypeMapVersionNumber(0),
m_resourceTypeMap(),
m_resourceTypeMapVersionNumber(0),
m_gameTime(0),
m_showAllDebugInfo(ConfigCommodityServer::getShowAllDebugInfo()),
m_completedAuctions(),
m_priorityQueueAuctionTimer(),
m_priorityQueueItemTimer()
{
	//
	// DO NOT DO ANYTHING HERE.  DO IT IN SingletonInialize()
	//
}

// ----------------------------------------------------------------------

void AuctionMarket::SingletonInialize()
{
	// ideally, these should be in the constructor, but they will result
	// in calling the Singleton's getInstance() method while the
	// Singleton is in the process of creating the Singleton, so
	// it tries to create another Singleton leading to an infinite loop;
	// we'll call this after the Singleton has been created
	GetLocation(std::string("PUBLIC_LOCATION"));

	// initialize list of auctions count by game object type
	GameObjectTypes::TypeStringMap const & gameObjectTypes = GameObjectTypes::getTypeStringMap();
	for (GameObjectTypes::TypeStringMap::const_iterator iter = gameObjectTypes.begin(); iter != gameObjectTypes.end(); ++iter)
	{
		m_auctionsCountByGameObjectType[iter->second] = 0;
		m_auctionsCountByGameObjectTypeChanged.insert(iter->second);
	}
}

// ----------------------------------------------------------------------

AuctionMarket::~AuctionMarket()
{
	//We need to delete the auction and auction location objects
	for (std::map<NetworkId, Auction *>::iterator i = m_auctions.begin(); i != m_auctions.end(); ++i)
	{
		delete (*i).second;
	}
	for (std::map<NetworkId, AuctionLocation *>::iterator i = m_locationIdMap.begin(); i != m_locationIdMap.end(); ++i)
	{
		delete (*i).second;
	}
}

// ----------------------------------------------------------------------

AuctionLocation *AuctionMarket::CreateLocation(const NetworkId & ownerId, const std::string & locationString, int entranceCharge)
{
	NetworkId loc_id = GetLocationId(locationString);
	AuctionLocation *auctionLocation = new AuctionLocation(loc_id, locationString, ownerId, 0, zeroNetworkId, 0, m_gameTime, 0, ACTIVE, true, entranceCharge);

	m_locationIdMap.insert(std::make_pair(loc_id, auctionLocation));
	if (ownerId != zeroNetworkId)
	{
		AddPlayerVendor(ownerId, loc_id, auctionLocation);
	}

	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Creating new CreateLocationMessage.\n"));

	CMCreateLocationMessage msg(
		auctionLocation->GetLocationId(),
		auctionLocation->GetLocationString(),
		auctionLocation->GetOwnerId(),
		auctionLocation->GetSalesTax(),
		auctionLocation->GetSalesTaxBankId(),
		0,
		m_gameTime,
		0,
		ACTIVE,
		true,
		entranceCharge
	);
	DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
	if (dbServer)
		dbServer->send(msg, true);

	return auctionLocation;
}

// ----------------------------------------------------------------------

void AuctionMarket::InitializeFromDB()
{
	LoadCommoditiesMessage msg(1);
	DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
	LOG("CommoditiesServer", ("send LoadCommoditiesMessage to SwgDatabaseServer"));
	if (dbServer)
		dbServer->send(msg, true);
	else
		DEBUG_REPORT_LOG(true, ("[Commodities Server] : Cannot find dbServer to send LoadCommodities message.\n"));
}

// ----------------------------------------------------------------------

AuctionLocation &AuctionMarket::GetLocation(const NetworkId &locationId)
{
	std::map<NetworkId, AuctionLocation *>::const_iterator i =
		m_locationIdMap.find(locationId);
	if (i != m_locationIdMap.end())
	{
		return (* ((*i).second) );
	}

	DEBUG_REPORT_LOG(true, ("[Commodities Server] : Couldnot find location_id %s from auction_locations.\n", locationId.getValueString().c_str()));

	AuctionLocation *location = CreateLocation(zeroNetworkId, "PUBLIC_LOCATION", 0);
	return *location;
}

// ----------------------------------------------------------------------

AuctionLocation &AuctionMarket::GetLocation(const std::string &locationString)
{
	NetworkId locationId(GetLocationId(locationString));
	std::map<NetworkId, AuctionLocation *>::const_iterator i =
		m_locationIdMap.find(locationId);
	if (i != m_locationIdMap.end())
	{
		return (* ((*i).second) );
	}

	DEBUG_REPORT_LOG(true, ("[Commodities Server] : Couldnot find location_id %s from auction_locations.\n", locationId.getValueString().c_str()));

	AuctionLocation *location = CreateLocation(zeroNetworkId, locationString, 0);
	return *location;
}

// ----------------------------------------------------------------------

void AuctionMarket::ModifyAuctionCount(const NetworkId &  playerId, int delta)
{
	std::map<NetworkId, int>::iterator f = m_auctionCountMap.find(playerId);
	int currentAmount = 0;
	if (f != m_auctionCountMap.end())
	{
		currentAmount = (*f).second;
	}

	m_auctionCountMap[playerId] = (currentAmount + delta);
}

// ----------------------------------------------------------------------

bool AuctionMarket::HasOpenAuctionSlots(const NetworkId &  playerId)
{
	std::map<NetworkId, int>::iterator f = m_auctionCountMap.find(playerId);
	if (f != m_auctionCountMap.end())
	{
		if ((*f).second >= ConfigCommodityServer::getMaxAuctionsPerPlayer())
		{
			return false;
		}
	}
	return true;
}

// ----------------------------------------------------------------------

void AuctionMarket::AddPlayerVendor(const NetworkId & playerId, const NetworkId & vendorId, AuctionLocation * auctionLocation)
{
	(m_playerVendorListMap[playerId])[vendorId] = auctionLocation;
}

// ----------------------------------------------------------------------

void AuctionMarket::RemovePlayerVendor(const NetworkId & playerId, const NetworkId & vendorId)
{
	std::map<NetworkId, std::map<NetworkId, AuctionLocation *> >::iterator const iterPlayer = m_playerVendorListMap.find(playerId);
	if (iterPlayer != m_playerVendorListMap.end())
	{
		iterPlayer->second.erase(vendorId);
		if (iterPlayer->second.empty())
		{
			m_playerVendorListMap.erase(iterPlayer);
		}
	}
}

// ----------------------------------------------------------------------

int AuctionMarket::GetVendorCount(const NetworkId & playerId)
{
	std::map<NetworkId, std::map<NetworkId, AuctionLocation *> >::const_iterator const iterPlayer = m_playerVendorListMap.find(playerId);
	if (iterPlayer != m_playerVendorListMap.end())
	{
		return iterPlayer->second.size();
	}

	return 0;
}

// ----------------------------------------------------------------------

int AuctionMarket::GetItemCount(const NetworkId & playerId)
{
	int itemCount = 0;

	std::map<NetworkId, std::map<NetworkId, AuctionLocation *> >::const_iterator const iterPlayer = m_playerVendorListMap.find(playerId);
	if (iterPlayer != m_playerVendorListMap.end())
	{
		for (std::map<NetworkId, AuctionLocation *>::const_iterator iterAuctionLocation = iterPlayer->second.begin(); iterAuctionLocation != iterPlayer->second.end(); ++iterAuctionLocation)
		{
			itemCount += iterAuctionLocation->second->GetAuctionItemCount();
		}
	}

	return itemCount;
}

// ----------------------------------------------------------------------

void AuctionMarket::AddAuctionToPriorityQueue(const Auction & auction)
{
	if (!(auction.GetFlags() & AUCTION_ALWAYS_PRESENT))
	{
		if (auction.IsActive() && (auction.GetAuctionTimer() > 0))
			m_priorityQueueAuctionTimer.insert(std::make_pair(auction.GetAuctionTimer(), auction.GetItem().GetItemId()));

		if (auction.GetItem().GetItemTimer() > 0)
			m_priorityQueueItemTimer.insert(std::make_pair(auction.GetItem().GetItemTimer(), auction.GetItem().GetItemId()));
	}
}

// ----------------------------------------------------------------------

void AuctionMarket::AddAuctionLocationToPriorityQueue(const AuctionLocation & auctionLocation)
{
	m_priorityQueueAuctionLocation.insert(std::make_pair(auctionLocation.GetNextUpdateTime(), auctionLocation.GetLocationId()));
}

// ----------------------------------------------------------------------

void AuctionMarket::RemoveAuctionLocationFromPriorityQueue(const AuctionLocation & auctionLocation)
{
	m_priorityQueueAuctionLocation.erase(std::make_pair(auctionLocation.GetNextUpdateTime(), auctionLocation.GetLocationId()));
}

// ----------------------------------------------------------------------

void AuctionMarket::AddAuction(Auction *auction)
{
	assert(auction != nullptr);

	AuctionLocation &location = auction->GetLocation();
	if (!location.IsOwner(auction->GetItem().GetOwnerId()))
	{
		ModifyAuctionCount(auction->GetItem().GetOwnerId(), 1);
	}

	if ((auction->GetItem().GetItemTemplateId() != 0) && (auction->GetItem().GetCategory() != 0))
	{
		bool newItemType = false;

		std::set<int> & itemTypeList = m_itemTypeMap[auction->GetItem().GetCategory()];
		if (itemTypeList.count(auction->GetItem().GetItemTemplateId()) == 0)
		{
			newItemType = true;
			IGNORE_RETURN(itemTypeList.insert(auction->GetItem().GetItemTemplateId()));
		}

		if (newItemType)
		{
			++m_itemTypeMapVersionNumber;
			GenericValueTypeMessage<std::pair<int, std::pair<int, int> > > msg("CommoditiesItemTypeAdded", std::make_pair(m_itemTypeMapVersionNumber, std::make_pair(auction->GetItem().GetCategory(), auction->GetItem().GetItemTemplateId())));
			CommodityServer::getInstance().sendToAllGameServers(msg);
		}
	}

	m_auctions.insert(std::make_pair(auction->GetItem().GetItemId(), auction));

	std::string const & gameObjectType = GameObjectTypes::getCanonicalName(auction->GetItem().GetCategory());
	std::map<std::string, int>::iterator iterAuctionsCountByGameObjectType = m_auctionsCountByGameObjectType.find(gameObjectType);
	if (iterAuctionsCountByGameObjectType != m_auctionsCountByGameObjectType.end())
	{
		++iterAuctionsCountByGameObjectType->second;
		m_auctionsCountByGameObjectTypeChanged.insert(gameObjectType);

		if (GameObjectTypes::isSubType(auction->GetItem().GetCategory()))
		{
			std::string const & gameParentObjectType = GameObjectTypes::getCanonicalName(GameObjectTypes::getMaskedType(auction->GetItem().GetCategory()));
			std::map<std::string, int>::iterator iterAuctionsCountByParentGameObjectType = m_auctionsCountByGameObjectType.find(gameParentObjectType);
			if (iterAuctionsCountByParentGameObjectType != m_auctionsCountByGameObjectType.end())
			{
				++iterAuctionsCountByParentGameObjectType->second;
				m_auctionsCountByGameObjectTypeChanged.insert(gameParentObjectType);
			}
		}
	}

	location.AddAuction(auction);
	AddAuctionToPriorityQueue(*auction);

	if (auction->IsActive() && auction->IsSold())
		AddAuctionToCompletedAuctionsList(*auction);

	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Creating new CreateAuctionMessage.\n"));
	LOG("CommoditiesServer", ("Player %s put item %s for sale at location %s for %d",
		auction->GetCreatorId().getValueString().c_str(),
		auction->GetItem().GetItemId().getValueString().c_str(),
		auction->GetLocation().GetLocationId().getValueString().c_str(),
		auction->GetBuyNowPrice() + auction->GetMinBid()));
	LOG("CustomerService", ("Auction: Player %s put item %s for sale at location %s for %d",
		auction->GetCreatorId().getValueString().c_str(),
		auction->GetItem().GetItemId().getValueString().c_str(),
		auction->GetLocation().GetLocationId().getValueString().c_str(),
		auction->GetBuyNowPrice() + auction->GetMinBid()));
	Unicode::String itemName(auction->GetItem().GetName());
	if (itemName.size() >= 1024)
	{
		itemName.assign(itemName, 0, 1023);
	}
	Unicode::String userDesc(auction->GetUserDescription());
	if (userDesc.size() >= 1024)
	{
		userDesc.assign(userDesc, 0, 1023);
	}
	CMCreateAuctionMessage msg(
		auction->GetCreatorId(),
		auction->GetMinBid(),
		auction->GetAuctionTimer(),
		auction->GetBuyNowPrice(),
		userDesc.size(),
		userDesc,
		auction->GetAttributes(),
		auction->GetLocation().GetLocationId(),
		auction->GetItem().GetItemId(),
		auction->GetItem().GetCategory(),
		auction->GetItem().GetItemTemplateId(),
		auction->GetItem().GetItemTimer(),
		itemName.size(),
		itemName,
		auction->GetItem().GetOwnerId(),
		(1 | auction->GetFlags()),
		auction->GetItem().GetSize());

	DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
	if (dbServer)
		dbServer->send(msg, true);
}

// ----------------------------------------------------------------------

void AuctionMarket::DestroyAuction(std::map<NetworkId, Auction *>::iterator &i)
{
	if (i != m_auctions.end())
	{
		Auction *auction = (*i).second;
		const NetworkId itemId = auction->GetItem().GetItemId();
		m_auctions.erase(i);

		std::string const & gameObjectType = GameObjectTypes::getCanonicalName(auction->GetItem().GetCategory());
		std::map<std::string, int>::iterator iterAuctionsCountByGameObjectType = m_auctionsCountByGameObjectType.find(gameObjectType);
		if ((iterAuctionsCountByGameObjectType != m_auctionsCountByGameObjectType.end()) && (iterAuctionsCountByGameObjectType->second > 0))
		{
			--iterAuctionsCountByGameObjectType->second;
			m_auctionsCountByGameObjectTypeChanged.insert(gameObjectType);

			if (GameObjectTypes::isSubType(auction->GetItem().GetCategory()))
			{
				std::string const & gameParentObjectType = GameObjectTypes::getCanonicalName(GameObjectTypes::getMaskedType(auction->GetItem().GetCategory()));
				std::map<std::string, int>::iterator iterAuctionsCountByParentGameObjectType = m_auctionsCountByGameObjectType.find(gameParentObjectType);
				if ((iterAuctionsCountByParentGameObjectType != m_auctionsCountByGameObjectType.end()) && (iterAuctionsCountByParentGameObjectType->second > 0))
				{
					--iterAuctionsCountByParentGameObjectType->second;
					m_auctionsCountByGameObjectTypeChanged.insert(gameParentObjectType);
				}
			}
		}

		AuctionLocation &location = auction->GetLocation();
		if (!location.IsOwner(auction->GetItem().GetOwnerId()))
		{
			ModifyAuctionCount(auction->GetItem().GetOwnerId(), -1);
		}
		location.RemoveAuction(auction);
		m_priorityQueueAuctionTimer.erase(std::make_pair(auction->GetAuctionTimer(), itemId));
		m_priorityQueueItemTimer.erase(std::make_pair(auction->GetItem().GetItemTimer(), itemId));

		delete auction;

		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Creating new DeleteAuctionMessage.\n"));
		CMDeleteAuctionMessage msg(itemId);
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Item ItemId : %s.\n", msg.GetItemId().getValueString().c_str()));

		DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
		if (dbServer)
			dbServer->send(msg, true);
	}
	else
	{
		printf("Yo this is whacked\n");
	}
}

// ----------------------------------------------------------------------

void AuctionMarket::UpdateLiveAuctions(int gameTime)
{
	if (getItemAttributeDataRequest)
		processItemAttributeData(m_auctions);

	// build the list of auctions that needs to be processed
	std::set<NetworkId> auctionsToBeProcessed;

	// auctions that have completed but not yet processed
	for (std::vector<NetworkId>::const_iterator iterCompletedAuctions = m_completedAuctions.begin(); iterCompletedAuctions != m_completedAuctions.end(); ++iterCompletedAuctions)
		auctionsToBeProcessed.insert(*iterCompletedAuctions);

	m_completedAuctions.clear();

	// auctions whose auction timer has expired
	for (std::set<std::pair<int, NetworkId> >::iterator iterAuctionTimer = m_priorityQueueAuctionTimer.begin(); iterAuctionTimer != m_priorityQueueAuctionTimer.end();)
	{
		if (gameTime >= (*iterAuctionTimer).first)
		{
			auctionsToBeProcessed.insert((*iterAuctionTimer).second);
			m_priorityQueueAuctionTimer.erase(iterAuctionTimer++);
		}
		else
		{
			break;
		}
	}

	// auctions whose item timer has expired
	for (std::set<std::pair<int, NetworkId> >::iterator iterItemTimer = m_priorityQueueItemTimer.begin(); iterItemTimer != m_priorityQueueItemTimer.end();)
	{
		if (gameTime > (*iterItemTimer).first)
		{
			auctionsToBeProcessed.insert((*iterItemTimer).second);
			m_priorityQueueItemTimer.erase(iterItemTimer++);
		}
		else
		{
			break;
		}
	}

	// process auctions needing to be processed
	std::set<NetworkId>::const_iterator iterAuctionsToBeProcessed;
	std::vector<std::map<NetworkId, Auction *>::iterator> completedAuctions;
	std::map<NetworkId, Auction *>::iterator auctionsIterator;

	for (iterAuctionsToBeProcessed = auctionsToBeProcessed.begin(); iterAuctionsToBeProcessed != auctionsToBeProcessed.end(); ++iterAuctionsToBeProcessed)
	{
		auctionsIterator = m_auctions.find(*iterAuctionsToBeProcessed);
		if (auctionsIterator != m_auctions.end())
		{
			if ((*auctionsIterator).second->Update(gameTime) == false)
			{
				//item is finished.
				const AuctionItem &item = (*auctionsIterator).second->GetItem();
				const AuctionLocation &location = (*auctionsIterator).second->GetLocation();
				OnItemExpired(item.GetOwnerId(), item.GetItemId(),
					item.GetNameLength(), item.GetName(),
					location.GetLocationString(), location.GetLocationId());
				completedAuctions.push_back(auctionsIterator);
			}
		}
	}

	std::vector<std::map<NetworkId, Auction *>::iterator>::iterator ri =
		completedAuctions.begin();
	while (ri != completedAuctions.end())
	{
		auctionsIterator = (*ri);
		LOG("CommoditiesServer", ("Auction: Auction for item %s being deleted by the Commodities Server because auction has expired (gametime: %d, itemTimer: %d).", (*auctionsIterator).second->GetItem().GetItemId().getValueString().c_str(), gameTime, (*auctionsIterator).second->GetItem().GetItemTimer()));
		LOG("CustomerService", ("Auction: Auction for item %s being deleted by the Commodities Server because auction has expired (gametime: %d, itemTimer: %d).", (*auctionsIterator).second->GetItem().GetItemId().getValueString().c_str(), gameTime, (*auctionsIterator).second->GetItem().GetItemTimer()));
		DestroyAuction(auctionsIterator);
		++ri;
	}

	std::list<AuctionLocation *> auctionLocationsToBeProcessed;
	{
		for (std::set<std::pair<int, NetworkId> >::iterator iterAuctionLocationToBeProcessed = m_priorityQueueAuctionLocation.begin(); iterAuctionLocationToBeProcessed != m_priorityQueueAuctionLocation.end();)
		{
			if (gameTime > (*iterAuctionLocationToBeProcessed).first)
			{
				std::map<NetworkId, AuctionLocation *>::iterator iterFind = m_locationIdMap.find((*iterAuctionLocationToBeProcessed).second);
				if (iterFind != m_locationIdMap.end())
				{
					auctionLocationsToBeProcessed.push_back(iterFind->second);
				}

				m_priorityQueueAuctionLocation.erase(iterAuctionLocationToBeProcessed++);
			}
			else
			{
				break;
			}
		}
	}

	for (std::list<AuctionLocation *>::iterator locationIterator = auctionLocationsToBeProcessed.begin(); locationIterator != auctionLocationsToBeProcessed.end(); ++locationIterator)
	{
		if ((*locationIterator)->IsVendorMarket())
		{
			VendorStatusCode status = (VendorStatusCode) (*locationIterator)->GetStatus();
			VendorStatusCode newStatus = status;
			int emptyDate = (*locationIterator)->GetEmptyDate();
			int lastAccessDate = (*locationIterator)->GetLastAccessDate();
			int inactiveDate = (*locationIterator)->GetInactiveDate();
			int newEmptyDate = -1, newLastAccessDate = -1, newInactiveDate = -1;
			switch (status) {
				case ACTIVE:
					if ((*locationIterator)->GetAuctionItemCount() == 0)
					{
						newStatus = EMPTY;
						newEmptyDate = m_gameTime;
					}
					else
					{
						int const unaccessedTime = lastAccessDate + (ConfigCommodityServer::getMinutesActiveToUnaccessed() * 60);
						if (m_gameTime >= unaccessedTime)
						{
							newStatus = UNACCESSED;
						}
						else
						{
							(*locationIterator)->SetNextUpdateTime(unaccessedTime);
						}
					}
					break;
				case EMPTY:
					{
						int const endangeredTime = emptyDate + (ConfigCommodityServer::getMinutesEmptyToEndangered() * 60);
						int const emptyAndUnaccessedTime = lastAccessDate + (ConfigCommodityServer::getMinutesUnaccessedToEndangered() * 60);
						if (m_gameTime >= endangeredTime)
						{
							newStatus = ENDANGERED;
							newInactiveDate = m_gameTime;
						}
						else if (m_gameTime >= emptyAndUnaccessedTime)
						{
							newStatus = EMPTY_AND_UNACCESSED;
						}
						else
						{
							(*locationIterator)->SetNextUpdateTime((endangeredTime < emptyAndUnaccessedTime) ? endangeredTime : emptyAndUnaccessedTime);
						}
					}
					break;
				case UNACCESSED:
					{
						int const endangeredTime = lastAccessDate + ((ConfigCommodityServer::getMinutesActiveToUnaccessed() + ConfigCommodityServer::getMinutesUnaccessedToEndangered()) * 60);
						if (m_gameTime >= endangeredTime)
						{
							newStatus = ENDANGERED;
							newInactiveDate = m_gameTime;
						}
						else if (emptyDate > 0)
						{
							int const emptyAndUnaccessedTime = emptyDate + (ConfigCommodityServer::getMinutesEmptyToEndangered() * 60);
							if (m_gameTime >= emptyAndUnaccessedTime)
							{
								newStatus = EMPTY_AND_UNACCESSED;
							}
							else
							{
								(*locationIterator)->SetNextUpdateTime((endangeredTime < emptyAndUnaccessedTime) ? endangeredTime : emptyAndUnaccessedTime);
							}
						}
						else
						{
							(*locationIterator)->SetNextUpdateTime(endangeredTime);
						}
					}
					break;
				case EMPTY_AND_UNACCESSED:
					{
						int const endangeredTime1 = emptyDate + (ConfigCommodityServer::getMinutesEmptyToEndangered() * 60);
						int const endangeredTime2 = lastAccessDate + ((ConfigCommodityServer::getMinutesActiveToUnaccessed() + ConfigCommodityServer::getMinutesUnaccessedToEndangered()) * 60);
						if ((m_gameTime >= endangeredTime1) || (m_gameTime >= endangeredTime2))
						{
							newStatus = ENDANGERED;
							newInactiveDate = m_gameTime;
						}
						else
						{
							(*locationIterator)->SetNextUpdateTime((endangeredTime1 < endangeredTime2) ? endangeredTime1 : endangeredTime2);
						}
					}
					break;
				case ENDANGERED:
					{
						int const removedTime = inactiveDate + (ConfigCommodityServer::getMinutesEndangeredToRemoved() * 60);
						if (m_gameTime >= removedTime)
						{
							newStatus = REMOVED;
						}
						else
						{
							(*locationIterator)->SetNextUpdateTime(removedTime);
						}
					}
					break;
				case REMOVED:
					newStatus = REMOVED;
			}
			if (status != newStatus)
			{
				(*locationIterator)->Update(newEmptyDate, newLastAccessDate, newInactiveDate, newStatus);
				LOG("CustomerService", ("Vendor: Vendor %s (owner %s) status changed from %s to %s", (*locationIterator)->GetLocationId().getValueString().c_str(), (*locationIterator)->GetOwnerId().getValueString().c_str(), vendorStatus[status], vendorStatus[newStatus]));
				LOG("CommoditiesServer", ("Vendor %s (owner %s) status changed from %s to %s", (*locationIterator)->GetLocationId().getValueString().c_str(), (*locationIterator)->GetOwnerId().getValueString().c_str(), vendorStatus[status], vendorStatus[newStatus]));
				AuctionResultCode arc;
				switch (newStatus) {
					case EMPTY:
						arc = ARC_VendorStatusEmpty;
						break;
					case UNACCESSED:
						arc = ARC_VendorUnaccessed;
						break;
					case EMPTY_AND_UNACCESSED:
						if (status == EMPTY)
							arc = ARC_VendorUnaccessed;
						else
							arc = ARC_VendorStatusEmpty;
						break;
					case ENDANGERED:
						arc = ARC_VendorEndangered;
						LOG("CommoditiesServer", ("Vendor %s becomes endangered being removed from game.",
							(*locationIterator)->GetLocationId().getValueString().c_str()));
						LOG("CuxtomerService", ("Vendor: Vendor %s becomes endangered being removed from game.",
							(*locationIterator)->GetLocationId().getValueString().c_str()));
						break;
					case REMOVED:
						arc = ARC_VendorRemoved;
						LOG("CommoditiesServer", ("Vendor %s will be removed from game due to empty or inactivity.",
							(*locationIterator)->GetLocationId().getValueString().c_str()));
						LOG("CustomerService", ("Vendor: Vendor %s will be removed from game due to empty or inactivity.",
							(*locationIterator)->GetLocationId().getValueString().c_str()));
						break;
					default:
						// newStatus is ACTIVE, which is impossible.
						// code added to avoid error in release build
						arc = ARC_Success;
				}
				OnVendorStatusChange(-1, (*locationIterator)->GetLocationId(), arc);
			}
		}
	}

	// if a processed auction is still valid (i.e. not yet deleted),
	// add it back to the priority queue so it can be processed
	// at its next timer expiration time
	for (iterAuctionsToBeProcessed = auctionsToBeProcessed.begin(); iterAuctionsToBeProcessed != auctionsToBeProcessed.end(); ++iterAuctionsToBeProcessed)
	{
		auctionsIterator = m_auctions.find(*iterAuctionsToBeProcessed);

		if (auctionsIterator != m_auctions.end())
		{
			AddAuctionToPriorityQueue(*(auctionsIterator->second));

			if (auctionsIterator->second->IsActive() && auctionsIterator->second->IsSold())
				AddAuctionToCompletedAuctionsList(*(auctionsIterator->second));
		}
	}
}

// ----------------------------------------------------------------------

void AuctionMarket::DestroyExpiredItems(int gameTime)
{
}

// ----------------------------------------------------------------------

void AuctionMarket::Update(int gameTime)
{
	m_gameTime = time(0);
	DestroyExpiredItems(gameTime);
	UpdateLiveAuctions(gameTime);
}

// ======================================================================
//
// AuctionMarket Network Message Handlers
//
// ======================================================================

void AuctionMarket::AddAuction(const AddAuctionMessage &message)
{
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : OwnerId : %s.\n", message.GetOwnerId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : OwnerName : %s.\n", message.GetOwnerName().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : MinimumBid : %d.\n", message.GetMinimumBid()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : AuctionTimer : %d.\n", message.GetAuctionTimer()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : ItemId : %s.\n", message.GetItemId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : ItemNameLength : %d.\n", message.GetItemNameLength()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : ItemName : %s.\n", Unicode::wideToNarrow(message.GetItemName()).c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : ItemType : %d.\n", message.GetItemType()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : ItemTemplateId : %d.\n", message.GetItemTemplateId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : ExpireTimer : %d.\n", message.GetExpireTimer()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : LocationId : %s.\n", message.GetLocationId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : Location : %s.\n", message.GetLocation().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : Flags : %d.\n", message.GetFlags()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : UserDescriptionLength : %d.\n", message.GetUserDescriptionLength()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : UserDescription : %s.\n", Unicode::wideToNarrow(message.GetUserDescription()).c_str()));
//	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : OobLength : %d.\n", message.GetOobLength()));
//	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : OobActualLength : %d.\n", message.GetOobData().size()));
//	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : OobData : %s.\n", Unicode::wideToNarrow(message.GetOobData()).c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : ItemSize : %d.\n", message.GetItemSize()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : Vendor Limit : %d.\n", message.GetVendorLimit()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : Vendor Item Limit : %d.\n", message.GetVendorItemLimit()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddAuctionMessage] : TrackId : %d.\n", message.GetTrackId()));

	std::map<NetworkId, Auction *>::iterator exists = m_auctions.find(
		message.GetItemId());
	AuctionLocation &auctionLocation = GetLocation(std::string(message.GetLocation()));
	if (exists != m_auctions.end())
	{
		OnAddAuction(message.GetTrackId(), ARC_AuctionAlreadyExists,
			message.GetResponseId(),
			message.GetItemId(), message.GetOwnerId(), message.GetOwnerName(),
			auctionLocation.GetOwnerId(),
			auctionLocation.GetLocationString());
		return;
	}
	if (!auctionLocation.IsOwner(message.GetOwnerId()) && !HasOpenAuctionSlots(message.GetOwnerId()))
	{
		OnAddAuction(message.GetTrackId(), ARC_TooManyAuctions,
			message.GetResponseId(),
			message.GetItemId(), message.GetOwnerId(), message.GetOwnerName(),
			auctionLocation.GetOwnerId(),
			auctionLocation.GetLocationString());
		return;
	}

	if (auctionLocation.IsVendorMarket())
	{
		OnAddAuction(message.GetTrackId(), ARC_AuctionAtVendorMarket,
			message.GetResponseId(),
			message.GetItemId(), message.GetOwnerId(), message.GetOwnerName(),
			auctionLocation.GetOwnerId(),
			auctionLocation.GetLocationString());
		return;
	}
	int itemSize = (message.GetItemSize() <2) ? 1 : (message.GetItemSize()-1);

	Auction *auction = new Auction(
		message.GetOwnerId(),
		message.GetMinimumBid(),
		message.GetAuctionTimer(),
		message.GetItemId(),
		message.GetItemNameLength(),
		message.GetItemName(),
		message.GetItemType(),
		message.GetItemTemplateId(),
		message.GetExpireTimer(),
		itemSize,
		auctionLocation,
		message.GetFlags(),
		0,
		message.GetUserDescriptionLength(),
		message.GetUserDescription(),
		0,
		Unicode::emptyString,
		message.GetAttributes());

	AddAuction(auction);

	OnAddAuction(message.GetTrackId(), ARC_Success,
		message.GetResponseId(),
		message.GetItemId(), message.GetOwnerId(), message.GetOwnerName(),
		auctionLocation.GetOwnerId(),
		auctionLocation.GetLocationString());

}

// ----------------------------------------------------------------------

void AuctionMarket::AddImmediateAuction(const AddImmediateAuctionMessage &message)
{
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : OwnerId : %s.\n", message.GetOwnerId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : OwnerName : %s.\n", message.GetOwnerName().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : AuctionTimer : %d.\n", message.GetAuctionTimer()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : GetPrice : %d.\n", message.GetPrice()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : ItemId : %s.\n", message.GetItemId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : ItemNameLength : %d.\n", message.GetItemNameLength()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : ItemName : %s.\n", Unicode::wideToNarrow(message.GetItemName()).c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : ItemType : %d.\n", message.GetItemType()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : ItemTemplateId : %d.\n", message.GetItemTemplateId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : ExpireTimer : %d.\n", message.GetExpireTimer()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : LocationId : %s.\n", message.GetLocationId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : Location : %s.\n", message.GetLocation().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : Flags : %d.\n", message.GetFlags()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : UserDescriptionLength : %d.\n", message.GetUserDescriptionLength()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : UserDescription : %s.\n", Unicode::wideToNarrow(message.GetUserDescription()).c_str()));
//	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : OobLength : %d.\n", message.GetOobLength()));
//	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : OobActualLength : %d.\n", message.GetOobData().size()));
//	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : OobData : %s.\n", Unicode::wideToNarrow(message.GetOobData()).c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : ItemSize : %d.\n", message.GetItemSize()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : Vendor Limit : %d.\n", message.GetVendorLimit()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : Vendor Item Limit : %d.\n", message.GetVendorItemLimit()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : TrackId : %d.\n", message.GetTrackId()));

	std::map<NetworkId, Auction *>::iterator exists = m_auctions.find(
		message.GetItemId());

	// force a 30 day stock/restock timer for player vendors.
	int auctionTimer = (ConfigCommodityServer::getMinutesVendorAuctionTimer() * 60) + time(0);
	// force the expire timer to be 30 days after the auction timer has finished
	// The expire timer should be large to allow for time to sit in the stockroom before it is deleted.
	int expireTimer = auctionTimer + (ConfigCommodityServer::getMinutesVendorItemTimer() * 60);
	// bazaar timers
    int bazaarAuctionTimer = (ConfigCommodityServer::getMinutesBazaarAuctionTimer() * 60) + time(0);
    int bazaarItemTimer = bazaarAuctionTimer + (ConfigCommodityServer::getMinutesBazaarItemTimer() * 60);

	if (message.GetFlags() & AUCTION_VENDOR_TRANSFER)
	{
		if (exists == m_auctions.end())
		{
			OnAddAuction(message.GetTrackId(), ARC_AuctionDoesNotExist,
				message.GetResponseId(),
				message.GetItemId(), message.GetOwnerId(), message.GetOwnerName(),
				zeroNetworkId,
				std::string());
			return;
		}
		Auction *oldAuction = (*exists).second;
		if (oldAuction->GetItem().GetOwnerId() != message.GetOwnerId())
		{
			OnAddAuction(message.GetTrackId(), ARC_NotItemOwner,
				message.GetResponseId(),
				message.GetItemId(), message.GetOwnerId(), message.GetOwnerName(),
				zeroNetworkId,
				std::string());
			return;
		}
		AuctionLocation &location = oldAuction->GetLocation();

		// This catches the restock path
		if( location.IsVendorMarket() )
		{
			// check the player's vendor limit
			if (GetVendorCount(message.GetOwnerId()) > message.GetVendorLimit())
			{
				OnAddAuction(message.GetTrackId(),
					ARC_AuctionVendorLimitExceeded,
					message.GetResponseId(),
					message.GetItemId(),
					message.GetOwnerId(),
					message.GetOwnerName(),
					zeroNetworkId,
					std::string());
				DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : Vendor Limit Exceeded for Player %s. (Current: %d, Limit: %d)\n", message.GetOwnerId().getValueString().c_str(), GetVendorCount(message.GetOwnerId()), message.GetVendorLimit()));
				return;
			}
			if (GetItemCount(message.GetOwnerId()) >= message.GetVendorItemLimit())
			{
				OnAddAuction(message.GetTrackId(),
					ARC_AuctionVendorItemLimitExceeded,
					message.GetResponseId(),
					message.GetItemId(),
					message.GetOwnerId(),
					message.GetOwnerName(),
					zeroNetworkId,
					std::string());
				DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : Vendor Item Limit Exceeded for Player %s. (Current: %d, Limit: %d)\n", message.GetOwnerId().getValueString().c_str(), GetItemCount(message.GetOwnerId()), message.GetVendorItemLimit()));
				return;
			}
		}
        // If this is not a player vendor, use bazaar settings for auction and expiration time
		else
		{
		    auctionTimer = bazaarAuctionTimer;
		    expireTimer = bazaarItemTimer;
		}

		// cannot use "relist" on an item bought as a vendor offer;
		// must use "sell" and enter a new selling price for the item
		if ((oldAuction->GetFlags() & AUCTION_OFFERED_ITEM) && (message.GetPrice() < 0))
		{
			OnAddAuction(message.GetTrackId(),
				ARC_InvalidBid,
				message.GetResponseId(),
				message.GetItemId(),
				message.GetOwnerId(),
				message.GetOwnerName(),
				zeroNetworkId,
				std::string());
			return;
		}

		// make sure price is valid
		int const price = ((message.GetPrice() >= 0) ? message.GetPrice() : oldAuction->GetBuyNowPrice());
		if (price < 0)
		{
			OnAddAuction(message.GetTrackId(),
				ARC_InvalidBid,
				message.GetResponseId(),
				message.GetItemId(),
				message.GetOwnerId(),
				message.GetOwnerName(),
				zeroNetworkId,
				std::string());
			return;
		}

		Auction *auction = new Auction(
			message.GetOwnerId(),
			0, auctionTimer,
			message.GetItemId(),
			((message.GetItemNameLength() > 0) ? message.GetItemNameLength() : oldAuction->GetItem().GetNameLength()),
			((message.GetItemNameLength() > 0) ? message.GetItemName() : oldAuction->GetItem().GetName()),
			oldAuction->GetItem().GetCategory(),
			oldAuction->GetItem().GetItemTemplateId(),
			expireTimer,
			oldAuction->GetItem().GetSize(),
			oldAuction->GetLocation(),
			(oldAuction->GetFlags() & (~AUCTION_OFFERED_ITEM)),
			price,
			((message.GetPrice() >= 0) ? message.GetUserDescriptionLength() : oldAuction->GetUserDescriptionLength()),
			((message.GetPrice() >= 0) ? message.GetUserDescription() : oldAuction->GetUserDescription()),
			oldAuction->GetOobLength(),
			oldAuction->GetOobData(),
			oldAuction->GetAttributes());

		LOG("CommoditiesServer", ("Auction: Auction for item %s being deleted by the Commodities Server because auction is about to be relisted.", (*exists).second->GetItem().GetItemId().getValueString().c_str()));
		LOG("CustomerService", ("Auction: Auction for item %s being deleted by the Commodities Server because auction is about to be relisted.", (*exists).second->GetItem().GetItemId().getValueString().c_str()));

		DestroyAuction(exists);

		AddAuction(auction);

		OnAddAuction(message.GetTrackId(), ARC_Success,
			message.GetResponseId(),
			message.GetItemId(), message.GetOwnerId(), message.GetOwnerName(),
			location.GetOwnerId(),
			location.GetLocationString());
		if (location.IsVendorMarket() && location.IsOwner(message.GetOwnerId()))
		{
			if (location.GetStatus() != ACTIVE)
			{
				LOG("CustomerService", ("Vendor: Vendor %s (owner %s) status changed from %s to Active", location.GetLocationId().getValueString().c_str(), location.GetOwnerId().getValueString().c_str(), vendorStatus[location.GetStatus()]));
				LOG("CommoditiesServer", ("Vendor %s (owner %s) status changed from %s to Active", location.GetLocationId().getValueString().c_str(), location.GetOwnerId().getValueString().c_str(), vendorStatus[location.GetStatus()]));
			}
			location.Update(0, m_gameTime, 0, ACTIVE);
		}
		return;
	}

	int itemSize = (message.GetItemSize() <2) ? 1 : (message.GetItemSize()-1);
	AuctionLocation &auctionLocation = GetLocation(std::string(message.GetLocation()));
	if (exists != m_auctions.end())
	{
		static Unicode::String const emptyUnicodeString;
		if (message.GetOwnerName() == "Restoring object")
		{
			DestroyAuction(exists);
			Auction *auction = new Auction(
				message.GetOwnerId(),
				0,
				auctionTimer,
				message.GetItemId(),
				message.GetItemNameLength(),
				message.GetItemName(),
				message.GetItemType(),
				message.GetItemTemplateId(),
				expireTimer,
				itemSize,
				auctionLocation,
				message.GetFlags(),
				message.GetPrice(),
				message.GetUserDescriptionLength(),
				message.GetUserDescription(),
				0,
				emptyUnicodeString,
				message.GetAttributes());
			AddAuction(auction);
		}
		OnAddAuction(message.GetTrackId(), ARC_AuctionAlreadyExists,
			message.GetResponseId(),
			message.GetItemId(), message.GetOwnerId(), message.GetOwnerName(),
			auctionLocation.GetOwnerId(),
			auctionLocation.GetLocationString());
		return;
	}

	if( auctionLocation.IsVendorMarket()  && auctionLocation.IsOwner(message.GetOwnerId()))
	{
		// check the player's vendor limit
		if (GetVendorCount(message.GetOwnerId()) > message.GetVendorLimit())
		{
			OnAddAuction(message.GetTrackId(),
				ARC_AuctionVendorLimitExceeded,
				message.GetResponseId(),
				message.GetItemId(),
				message.GetOwnerId(),
				message.GetOwnerName(),
				zeroNetworkId,
				std::string());
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : Vendor Limit Exceeded for Player %s. (Current: %d, Limit: %d)\n", message.GetOwnerId().getValueString().c_str(), GetVendorCount(message.GetOwnerId()), message.GetVendorLimit()));
			return;
		}
		if (GetItemCount(message.GetOwnerId()) >= message.GetVendorItemLimit())
		{
			OnAddAuction(message.GetTrackId(),
				ARC_AuctionVendorItemLimitExceeded,
				message.GetResponseId(),
				message.GetItemId(),
				message.GetOwnerId(),
				message.GetOwnerName(),
				zeroNetworkId,
				std::string());
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddImmediateAuctionMessage] : Vendor Item Limit Exceeded for Player %s. (Current: %d, Limit: %d)\n", message.GetOwnerId().getValueString().c_str(), GetItemCount(message.GetOwnerId()), message.GetVendorItemLimit()));
			return;
		}
	}
	else if (!HasOpenAuctionSlots(message.GetOwnerId()))
	{
		OnAddAuction(message.GetTrackId(), ARC_TooManyAuctions,
			message.GetResponseId(),
			message.GetItemId(), message.GetOwnerId(), message.GetOwnerName(),
			auctionLocation.GetOwnerId(),
			auctionLocation.GetLocationString());
		return;
	}

    // If this is not a player vendor, use bazaar settings for auction and expiration time
	if( !auctionLocation.IsVendorMarket() )
	{
        auctionTimer = bazaarAuctionTimer;
        expireTimer = bazaarItemTimer;
	}

	static Unicode::String const emptyUnicodeString;
	Auction *auction = new Auction(
		message.GetOwnerId(),
		0, auctionTimer,
		message.GetItemId(),
		message.GetItemNameLength(),
		message.GetItemName(),
		message.GetItemType(),
		message.GetItemTemplateId(),
		expireTimer,
		itemSize,
		auctionLocation,
		message.GetFlags(),
		message.GetPrice(),
		message.GetUserDescriptionLength(),
		message.GetUserDescription(),
		0,
		emptyUnicodeString,
		message.GetAttributes());

	AddAuction(auction);

	OnAddAuction(message.GetTrackId(), ARC_Success,
		message.GetResponseId(),
		message.GetItemId(), message.GetOwnerId(), message.GetOwnerName(),
		auctionLocation.GetOwnerId(),
		auctionLocation.GetLocationString());
	if (auctionLocation.IsVendorMarket() && auctionLocation.IsOwner(message.GetOwnerId()))
	{
		if (auctionLocation.GetStatus() != ACTIVE)
		{
			LOG("CustomerService", ("Vendor: Vendor %s (owner %s) status changed from %s to Active", auctionLocation.GetLocationId().getValueString().c_str(), auctionLocation.GetOwnerId().getValueString().c_str(), vendorStatus[auctionLocation.GetStatus()]));
			LOG("CommoditiesServer", ("Vendor %s (owner %s) status changed from %s to Active", auctionLocation.GetLocationId().getValueString().c_str(), auctionLocation.GetOwnerId().getValueString().c_str(), vendorStatus[auctionLocation.GetStatus()]));
		}
		auctionLocation.Update(0, m_gameTime, 0, ACTIVE);
	}
}

// ----------------------------------------------------------------------

void AuctionMarket::AddBid(const AddBidMessage &message)
{
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddBidMessage] : GetAuctionId : %s.\n", message.GetAuctionId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddBidMessage] : GetPlayerId : %s.\n", message.GetPlayerId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddBidMessage] : GetBid : %d.\n", message.GetBid()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddBidMessage] : GetMaxProxyBid : %d.\n", message.GetMaxProxyBid()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddBidMessage] : GetPlayerName	: %s.\n", message.GetPlayerName().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddBidMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AddBidMessage] : TrackId : %d.\n", message.GetTrackId()));

	std::map<NetworkId, Auction *>::iterator iter = m_auctions.find(
		message.GetAuctionId());
	if (iter == m_auctions.end())
	{
		OnAddBid(message.GetTrackId(), ARC_AuctionDoesNotExist,
			message.GetResponseId(), zeroNetworkId, message.GetAuctionId(),
			message.GetPlayerId(), zeroNetworkId, message.GetBid(), 0,
			message.GetMaxProxyBid(), "PUBLIC_VENDOR",
			0, Unicode::String(), 0, zeroNetworkId);
		return;
	}
	if (!(*iter).second->IsActive())
	{
		OnAddBid(message.GetTrackId(), ARC_AuctionDoesNotExist,
			message.GetResponseId(), zeroNetworkId, message.GetAuctionId(),
			message.GetPlayerId(), zeroNetworkId, message.GetBid(), 0,
			message.GetMaxProxyBid(), "PUBLIC_VENDOR",
			0, Unicode::String(), 0, zeroNetworkId);
		return;
	}

	Auction *auction = (*iter).second;

	// if it's an immediate auction, the bid value must be
	// exactly the same as the buy now price + sales tax;
	// the client won't let you enter a different bid price
	// for an immediate auction, but a player could use a
	// hacked client to change the bid price, and we need
	// to catch that, and generate a customer service log
	// if the bid price is not the same as the buy now
	// price + sales tax; note that we could also get here
	// legitimately if the sales tax was changed but the
	// client had not refreshed the auction to reflect
	// the price with the new sales tax
	if (auction->IsImmediate() && ((message.GetBid() != auction->GetBuyNowPriceWithSalesTax()) || (message.GetMaxProxyBid() != auction->GetBuyNowPriceWithSalesTax())))
	{
		std::string itemName = Unicode::wideToNarrow(auction->GetItem().GetName());

		LOG("CustomerService", ("Auction: Player %s (%s) sent an invalid bid (bid=%d, max proxy bid=%d) for immediate auction %s (%s) (buy now price=%d, buy now price with sales tax=%d) at location (%s)",
			message.GetPlayerName().c_str(), message.GetPlayerId().getValueString().c_str(),
			message.GetBid(), message.GetMaxProxyBid(),
			auction->GetItem().GetItemId().getValueString().c_str(), itemName.c_str(),
			auction->GetBuyNowPrice(), auction->GetBuyNowPriceWithSalesTax(),
			auction->GetLocation().GetLocationString().c_str()));

		OnAddBid(message.GetTrackId(), ARC_InvalidBid,
			message.GetResponseId(), zeroNetworkId, message.GetAuctionId(),
			message.GetPlayerId(), zeroNetworkId, message.GetBid(), 0,
			message.GetMaxProxyBid(), "PUBLIC_VENDOR",
			0, Unicode::String(), 0, zeroNetworkId);

		return;
	}

	int	oldHighBidAmount = auction->GetHighBidAmount();

	AuctionResultCode result = auction->AddBid(
		message.GetPlayerId(), message.GetBid(), message.GetMaxProxyBid());

	if (result == ARC_SuccessPermanentAuction)
	{
		//printf("AddBid is PermanentAuction.\n");
		OnPermanentAuctionPurchased(
			message.GetTrackId(),
			auction->GetCreatorId(), message.GetPlayerId(),
			message.GetBid(), message.GetAuctionId(),
			auction->GetLocation().GetLocationString(),
			auction->GetItem().GetNameLength(),
			auction->GetItem().GetName(),
			auction->GetAttributes());

		OnAddBid(message.GetTrackId(), result,
			message.GetResponseId(), auction->GetCreatorId(),
			message.GetAuctionId(),
			message.GetPlayerId(), zeroNetworkId, message.GetBid(),
			0, message.GetMaxProxyBid(),
			auction->GetLocation().GetLocationString(),
			auction->GetItem().GetNameLength(), auction->GetItem().GetName(), 0, zeroNetworkId);

		return;
	}

	const AuctionBid *highBid = auction->GetHighBid();

	if ((highBid) &&
		((result == ARC_Success) || (oldHighBidAmount < highBid->GetBid())))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Creating new CreateAuctionBidMessage.\n"));

		CMCreateAuctionBidMessage msg(
			auction->GetItem().GetItemId(),
			highBid->GetBidderId(),
			highBid->GetBid(),
			highBid->GetMaxProxyBid()
		);
		DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
		if (dbServer)
			dbServer->send(msg, true);
	}

	const AuctionBid *previousBid = auction->GetPreviousBid();
	NetworkId previousBidder = zeroNetworkId;
	int previousBidAmount = 0;
	int maxProxyBid = message.GetMaxProxyBid();
	int salesTaxAmount = 0;
	NetworkId bankId = zeroNetworkId;

	if (previousBid)
	{
		previousBidder = previousBid->GetBidderId();
		previousBidAmount = previousBid->GetMaxProxyBid();
		//printf("AddBid is Auction.\n");
	}
	if (auction->GetBuyNowPrice() != 0)
	{
		//its an instant auction, set the previous bidder to -1 to signify
		previousBidder = negOneNetworkId;
		maxProxyBid = auction->GetBuyNowPrice();
		salesTaxAmount = auction->GetBuyNowPriceWithSalesTax() - auction->GetBuyNowPrice();
		bankId = auction->GetLocation().GetSalesTaxBankId();
		//printf("AddBid is Immediate.\n");
	}

	OnAddBid(message.GetTrackId(), result,
		message.GetResponseId(), auction->GetCreatorId(),
		message.GetAuctionId(),
		message.GetPlayerId(), previousBidder, message.GetBid(),
		previousBidAmount, maxProxyBid,
		auction->GetLocation().GetLocationString(),
		auction->GetItem().GetNameLength(), auction->GetItem().GetName(),
		salesTaxAmount, bankId);
	LOG("CommoditiesServer", ("Player %s put a bid %d on item %s",
		message.GetPlayerId().getValueString().c_str(),
		message.GetBid(),
		message.GetAuctionId().getValueString().c_str()));
	LOG("CustomerService", ("Auction: Player %s put a bid %d on item %s",
		message.GetPlayerId().getValueString().c_str(),
		message.GetBid(),
		message.GetAuctionId().getValueString().c_str()));

	AuctionLocation &loc = auction->GetLocation();
	if (loc.IsVendorMarket() && (!loc.IsOwner(message.GetPlayerId())))
	{
		if (loc.GetStatus() != ACTIVE)
		{
			LOG("CustomerService", ("Vendor: Vendor %s (owner %s) status changed from %s to Active", loc.GetLocationId().getValueString().c_str(), loc.GetOwnerId().getValueString().c_str(), vendorStatus[loc.GetStatus()]));
			LOG("CommoditiesServer", ("Vendor %s (owner %s) status changed from %s to Active", loc.GetLocationId().getValueString().c_str(), loc.GetOwnerId().getValueString().c_str(), vendorStatus[loc.GetStatus()]));
		}
		loc.Update(0, m_gameTime, 0, ACTIVE);
	}

	// if an item got bought off a vendor, we must send the auction
	// complete/expire message to the game server where the item is
	// bought, because that game server is where some additional
	// processing/validation needs to be done for this transaction;
	// we store the game server id in Auction, and in Auction::Update(),
	// it will send the auction complete/expire message to the game server
	if (loc.IsVendorMarket() && auction->IsImmediate() && auction->IsSold())
		auction->SetTrackId(message.GetTrackId());
}

// ----------------------------------------------------------------------

void AuctionMarket::CancelAuction(const CancelAuctionMessage &message)
{
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CancelAuctionMessage] : PlayerId : %s.\n", message.GetPlayerId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CancelAuctionMessage] : AuctionId : %s.\n", message.GetAuctionId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CancelAuctionMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CancelAuctionMessage] : TrackId : %d.\n", message.GetTrackId()));

	std::map<NetworkId, Auction *>::iterator iter = m_auctions.find(message.GetAuctionId());
	if (iter == m_auctions.end())
	{
		OnCancelAuction(message.GetTrackId(), ARC_AuctionDoesNotExist,
			message.GetResponseId(), message.GetAuctionId(),
			message.GetPlayerId(), zeroNetworkId, 0, "PUBLIC_LOCATION");
		return;
	}

	Auction *auction = (*iter).second;
	LOG("CommoditiesServer", ("Player %s canceled auction for item %s at location %s",
		auction->GetItem().GetOwnerId().getValueString().c_str(),
		auction->GetItem().GetItemId().getValueString().c_str(),
		auction->GetLocation().GetLocationId().getValueString().c_str()));
	LOG("CustomerService", ("Auction: Player %s canceled auction for item %s at location %s",
		auction->GetItem().GetOwnerId().getValueString().c_str(),
		auction->GetItem().GetItemId().getValueString().c_str(),
		auction->GetLocation().GetLocationId().getValueString().c_str()));

	if (auction->GetLocation().IsVendorMarket() &&  auction->GetLocation().IsOwner(message.GetPlayerId()))
	{
		if (auction->GetItem().GetOwnerId() == message.GetPlayerId())
		{
			if (auction->IsActive())
			{
				//The vendor owner is cancelling his own sale
				OnCancelAuction(message.GetTrackId(), ARC_Success,
								message.GetResponseId(), message.GetAuctionId(),
								message.GetPlayerId(), zeroNetworkId, 0,
								auction->GetLocation().GetLocationString());
				auction->Expire(false, false, message.GetTrackId());
				if (auction->GetLocation().GetStatus() != ACTIVE)
				{
					LOG("CustomerService", ("Vendor: Vendor %s (owner %s) status changed from %s to Active", auction->GetLocation().GetLocationId().getValueString().c_str(), auction->GetLocation().GetOwnerId().getValueString().c_str(), vendorStatus[auction->GetLocation().GetStatus()]));
					LOG("CommoditiesServer", ("Vendor %s (owner %s) status changed from %s to Active", auction->GetLocation().GetLocationId().getValueString().c_str(), auction->GetLocation().GetOwnerId().getValueString().c_str(), vendorStatus[auction->GetLocation().GetStatus()]));
				}
				auction->GetLocation().Update(0, m_gameTime, 0, ACTIVE);
			}
			else
			{
				OnCancelAuction(message.GetTrackId(), ARC_VendorOwnerCanceledCompletedAuction,
								message.GetResponseId(), message.GetAuctionId(),
								message.GetPlayerId(), zeroNetworkId, 0,
								auction->GetLocation().GetLocationString());
				return;

			}
		}
		// make sure the sale is still active before deciding it's the owner refusing to buy an offer.
		// Fixes a crediting duping exploit where the owner
		// of the vendor withdraws a sale after someone buys it.
		else if( auction->GetItem().GetOwnerId() != message.GetPlayerId() && !auction->IsActive() )
		{
			OnCancelAuction(message.GetTrackId(), ARC_VendorOwnerCanceledCompletedAuction,
				message.GetResponseId(), message.GetAuctionId(),
				message.GetPlayerId(), zeroNetworkId, 0,
				auction->GetLocation().GetLocationString());
			return;
		}
		// owner of the vendor is refusing an offer to his vendor
		else
		{
			//The vendor owner doesn't want your crap!
			OnVendorRefuseItem(message.GetTrackId(), ARC_Success,
				message.GetResponseId(), message.GetAuctionId(),
				message.GetPlayerId(), auction->GetCreatorId());
			auction->Expire(false, false, message.GetTrackId());
		}
		return;
	}
	else if (message.GetPlayerId() != auction->GetCreatorId())
	{
		OnCancelAuction(message.GetTrackId(), ARC_NotItemOwner,
			message.GetResponseId(), message.GetAuctionId(),
			message.GetPlayerId(), zeroNetworkId, 0,
			auction->GetLocation().GetLocationString());
		return;
	}
	else if ((message.GetPlayerId() == auction->GetCreatorId()) &&
			  !auction->IsActive())
	{
		OnCancelAuction(message.GetTrackId(), ARC_AuctionAlreadyCompleted,
			message.GetResponseId(), message.GetAuctionId(),
			message.GetPlayerId(), zeroNetworkId, 0,
			auction->GetLocation().GetLocationString());
		return;
	}

	auction->Expire(false, false, message.GetTrackId());

	const AuctionBid *highBid = auction->GetHighBid();
	NetworkId highBidderId = zeroNetworkId;
	int highBidAmount = 0;
	if (highBid)
	{
		highBidderId = highBid->GetBidderId();
		highBidAmount = highBid->GetMaxProxyBid();
	}

	OnCancelAuction(message.GetTrackId(), ARC_Success,
		message.GetResponseId(), message.GetAuctionId(),
		message.GetPlayerId(), highBidderId, highBidAmount,
		auction->GetLocation().GetLocationString());
}

// ----------------------------------------------------------------------

void AuctionMarket::AcceptHighBid(const AcceptHighBidMessage &message)
{
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AcceptHighBidMessage] : PlayerId : %s.\n", message.GetPlayerId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AcceptHighBidMessage] : AuctionId : %s.\n", message.GetAuctionId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AcceptHighBidMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server AcceptHighBidMessage] : TrackId : %d.\n", message.GetTrackId()));

	Auction *auction = nullptr;
	AuctionResultCode result = ARC_Success;
	std::map<NetworkId, Auction *>::iterator iter = m_auctions.find(
		message.GetAuctionId());
	if (iter == m_auctions.end())
	{
		result = ARC_AuctionDoesNotExist;
	}
	else
	{
		auction = (*iter).second;
		if (message.GetPlayerId() != auction->GetCreatorId())
		{
			result = ARC_NotItemOwner;
		}
		else if (auction->GetHighBid() == nullptr)
		{
			result = ARC_NoBids;
		}
		else if (!auction->IsActive())
		{
			result = ARC_AuctionAlreadyCompleted;
		}
	}

	if (result == ARC_Success)
	{
		auction->Expire(true, false, message.GetTrackId());
	}

	OnAcceptHighBid(message.GetTrackId(), result,
		message.GetResponseId(), message.GetAuctionId(),
		message.GetPlayerId());

}

// ----------------------------------------------------------------------

void AuctionMarket::QueryAuctionHeaders(
	const QueryAuctionHeadersMessage &message
)
{
	// Break the text filter string for matching ALL tokens into lower-case tokens
	Unicode::UnicodeStringVector textFilterAllTokens;
	IGNORE_RETURN(Unicode::tokenize(Unicode::toLower(message.GetTextFilterAll()), textFilterAllTokens));

	// Break the text filter string for matching ANY tokens into lower-case tokens
	Unicode::UnicodeStringVector textFilterAnyTokens;
	IGNORE_RETURN(Unicode::tokenize(Unicode::toLower(message.GetTextFilterAny()), textFilterAnyTokens));

	std::string debugOutput;

#ifdef _DEBUG
	if (m_showAllDebugInfo || message.GetOverrideVendorSearchFlag())
#else
	if (message.GetOverrideVendorSearchFlag())
#endif
	{
		debugOutput = "Commodities Server QueryAuctionHeadersMessage:\r\n";

		char buffer[4096];

		snprintf(buffer, sizeof(buffer)-1, "ResponseId = (%d)", message.GetResponseId());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "TrackId = (%d)", message.GetTrackId());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "PlayerId = (%s)", message.GetPlayerId().getValueString().c_str());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "VendorId = (%s)", message.GetVendorId().getValueString().c_str());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		if (message.GetQueryType() == AST_ByCategory)
		{
			snprintf(buffer, sizeof(buffer)-1, "QueryType = (%d, AST_ByCategory)", message.GetQueryType());
		}
		else if (message.GetQueryType() == AST_ByLocation)
		{
			snprintf(buffer, sizeof(buffer)-1, "QueryType = (%d, AST_ByLocation)", message.GetQueryType());
		}
		else if (message.GetQueryType() == AST_ByAll)
		{
			snprintf(buffer, sizeof(buffer)-1, "QueryType = (%d, AST_ByAll)", message.GetQueryType());
		}
		else if (message.GetQueryType() == AST_ByPlayerSales)
		{
			snprintf(buffer, sizeof(buffer)-1, "QueryType = (%d, AST_ByPlayerSales)", message.GetQueryType());
		}
		else if (message.GetQueryType() == AST_ByPlayerBids)
		{
			snprintf(buffer, sizeof(buffer)-1, "QueryType = (%d, AST_ByPlayerBids)", message.GetQueryType());
		}
		else if (message.GetQueryType() == AST_ByPlayerStockroom)
		{
			snprintf(buffer, sizeof(buffer)-1, "QueryType = (%d, AST_ByPlayerStockroom)", message.GetQueryType());
		}
		else if (message.GetQueryType() == AST_ByVendorOffers)
		{
			snprintf(buffer, sizeof(buffer)-1, "QueryType = (%d, AST_ByVendorOffers)", message.GetQueryType());
		}
		else if (message.GetQueryType() == AST_ByVendorSelling)
		{
			snprintf(buffer, sizeof(buffer)-1, "QueryType = (%d, AST_ByVendorSelling)", message.GetQueryType());
		}
		else if (message.GetQueryType() == AST_ByVendorStockroom)
		{
			snprintf(buffer, sizeof(buffer)-1, "QueryType = (%d, AST_ByVendorStockroom)", message.GetQueryType());
		}
		else if (message.GetQueryType() == AST_ByPlayerOffersToVendor)
		{
			snprintf(buffer, sizeof(buffer)-1, "QueryType = (%d, AST_ByPlayerOffersToVendor)", message.GetQueryType());
		}
		else
		{
			snprintf(buffer, sizeof(buffer)-1, "QueryType = (%d)", message.GetQueryType());
		}

		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "ItemType = (%d, %s)", message.GetItemType(), GameObjectTypes::getCanonicalName(message.GetItemType()).c_str());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "ItemTypeExactMatch = (%s)", (message.GetItemTypeExactMatch() ? "yes" : "no"));
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "ItemTemplate = (%d)", message.GetItemTemplateId());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "TextFilterAll = (%s)", Unicode::wideToNarrow(message.GetTextFilterAll()).c_str());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		for (Unicode::UnicodeStringVector::const_iterator iterTextFilterAllTokens = textFilterAllTokens.begin(); iterTextFilterAllTokens != textFilterAllTokens.end(); ++iterTextFilterAllTokens)
		{
			snprintf(buffer, sizeof(buffer)-1, "TextFilterAll = (%s)", Unicode::wideToNarrow(*iterTextFilterAllTokens).c_str());
			buffer[sizeof(buffer)-1] = '\0';
			debugOutput += buffer;
			debugOutput += "\r\n";
#ifdef _DEBUG
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif
		}

		snprintf(buffer, sizeof(buffer)-1, "TextFilterAny = (%s)", Unicode::wideToNarrow(message.GetTextFilterAny()).c_str());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		for (Unicode::UnicodeStringVector::const_iterator iterTextFilterAnyTokens = textFilterAnyTokens.begin(); iterTextFilterAnyTokens != textFilterAnyTokens.end(); ++iterTextFilterAnyTokens)
		{
			snprintf(buffer, sizeof(buffer)-1, "TextFilterAny = (%s)", Unicode::wideToNarrow(*iterTextFilterAnyTokens).c_str());
			buffer[sizeof(buffer)-1] = '\0';
			debugOutput += buffer;
			debugOutput += "\r\n";
#ifdef _DEBUG
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif
		}

		snprintf(buffer, sizeof(buffer)-1, "MinPrice = (%d)", message.GetPriceFilterMin());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "MaxPrice = (%d)", message.GetPriceFilterMax());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "PriceIncludesEntranceFee = (%s)", (message.GetPriceFilterIncludesFee() ? "yes" : "no"));
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "LocationPlanet = (%s)", message.GetSearchStringPlanet().c_str());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "LocationRegion = (%s)", message.GetSearchStringRegion().c_str());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "LocationAuctionLocationId = (%s)", message.GetSearchAuctionLocationId().getValueString().c_str());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "SearchMyVendorsOnly = (%s)", (message.GetSearchMyVendorsOnly() ? "yes" : "no"));
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "OverrideVendorSearchFlag = (%s)", (message.GetOverrideVendorSearchFlag() ? "yes" : "no"));
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		snprintf(buffer, sizeof(buffer)-1, "QueryOffset = (%u)", message.GetQueryOffset());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		if (!message.getAdvancedSearch().empty())
		{
			if (message.getAdvancedSearchMatchAllAny() == AuctionQueryHeadersMessage::ASMAA_match_all)
			{
				snprintf(buffer, sizeof(buffer)-1, "AdvancedSearch = ASMAA_match_all");
			}
			else if (message.getAdvancedSearchMatchAllAny() == AuctionQueryHeadersMessage::ASMAA_match_any)
			{
				snprintf(buffer, sizeof(buffer)-1, "AdvancedSearch = ASMAA_match_any");
			}
			else if (message.getAdvancedSearchMatchAllAny() == AuctionQueryHeadersMessage::ASMAA_not_match_all)
			{
				snprintf(buffer, sizeof(buffer)-1, "AdvancedSearch = ASMAA_not_match_all");
			}
			else if (message.getAdvancedSearchMatchAllAny() == AuctionQueryHeadersMessage::ASMAA_not_match_any)
			{
				snprintf(buffer, sizeof(buffer)-1, "AdvancedSearch = ASMAA_not_match_any");
			}
			else
			{
				snprintf(buffer, sizeof(buffer)-1, "AdvancedSearch = UNKNOWN (%d)", static_cast<int>(message.getAdvancedSearchMatchAllAny()));
			}

			buffer[sizeof(buffer)-1] = '\0';
			debugOutput += buffer;
			debugOutput += "\r\n";
#ifdef _DEBUG
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

			std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> const & sa = CommoditiesAdvancedSearchAttribute::getSearchAttributeForGameObjectType(message.GetItemType());
			std::string attributeName;

			for (std::list<AuctionQueryHeadersMessage::SearchCondition>::const_iterator iter = message.getAdvancedSearch().begin(); iter != message.getAdvancedSearch().end(); ++iter)
			{
				attributeName.clear();
				if (!sa.empty())
				{
					for (std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *>::const_iterator iterSa = sa.begin(); iterSa != sa.end(); ++iterSa)
					{
						if (iterSa->second->attributeNameCrc == iter->attributeNameCrc)
						{
							attributeName = iterSa->second->attributeName;
							break;
						}
					}
				}

				if (iter->comparison == AuctionQueryHeadersMessage::SCC_int)
				{
					snprintf(buffer, sizeof(buffer)-1, "AdvancedSearch = (%s, %lu) SCC_int (%d, %d) required (%s)", attributeName.c_str(), iter->attributeNameCrc, iter->intMin, iter->intMax, (iter->requiredAttribute ? "yes" : "no"));
				}
				else if (iter->comparison == AuctionQueryHeadersMessage::SCC_float)
				{
					snprintf(buffer, sizeof(buffer)-1, "AdvancedSearch = (%s, %lu) SCC_float (%.2f, %.2f) required (%s)", attributeName.c_str(), iter->attributeNameCrc, iter->floatMin, iter->floatMax, (iter->requiredAttribute ? "yes" : "no"));
				}
				else if (iter->comparison == AuctionQueryHeadersMessage::SCC_string_equal)
				{
					snprintf(buffer, sizeof(buffer)-1, "AdvancedSearch = (%s, %lu) SCC_string_equal (%s) required (%s)", attributeName.c_str(), iter->attributeNameCrc, iter->stringValue.c_str(), (iter->requiredAttribute ? "yes" : "no"));
				}
				else if (iter->comparison == AuctionQueryHeadersMessage::SCC_string_not_equal)
				{
					snprintf(buffer, sizeof(buffer)-1, "AdvancedSearch = (%s, %lu) SCC_string_not_equal (%s) required (%s)", attributeName.c_str(), iter->attributeNameCrc, iter->stringValue.c_str(), (iter->requiredAttribute ? "yes" : "no"));
				}
				else if (iter->comparison == AuctionQueryHeadersMessage::SCC_string_contain)
				{
					snprintf(buffer, sizeof(buffer)-1, "AdvancedSearch = (%s, %lu) SCC_string_contain (%s) required (%s)", attributeName.c_str(), iter->attributeNameCrc, iter->stringValue.c_str(), (iter->requiredAttribute ? "yes" : "no"));
				}
				else if (iter->comparison == AuctionQueryHeadersMessage::SCC_string_not_contain)
				{
					snprintf(buffer, sizeof(buffer)-1, "AdvancedSearch = (%s, %lu) SCC_string_not_contain (%s) required (%s)", attributeName.c_str(), iter->attributeNameCrc, iter->stringValue.c_str(), (iter->requiredAttribute ? "yes" : "no"));
				}
				else
				{
					snprintf(buffer, sizeof(buffer)-1, "AdvancedSearch = (%s, %lu) UNKNOWN COMPARISON (%d) (%d, %d) (%.2f, %.2f) (%s) required (%s)", attributeName.c_str(), iter->attributeNameCrc, static_cast<int>(iter->comparison), iter->intMin, iter->intMax, iter->floatMin, iter->floatMax, iter->stringValue.c_str(), (iter->requiredAttribute ? "yes" : "no"));
				}

				buffer[sizeof(buffer)-1] = '\0';
				debugOutput += buffer;
				debugOutput += "\r\n";
#ifdef _DEBUG
				DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif
			}
		}
		else
		{
			snprintf(buffer, sizeof(buffer)-1, "AdvancedSearch = (none)");
			buffer[sizeof(buffer)-1] = '\0';
			debugOutput += buffer;
			debugOutput += "\r\n";
#ifdef _DEBUG
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif
		}
	}

	// The item template we are looking for
	int const itemTemplateId = message.GetItemTemplateId();

	// The item category we are looking for
	int const itemCategory = message.GetItemType();
	bool const itemCategoryMatchExact = (message.GetItemTypeExactMatch() && (itemCategory != 0) && !GameObjectTypes::isSubType(itemCategory) && (itemTemplateId == 0));

	// the combination of values of itemCategory and
	// itemTemplateId indicate what type of a search it is
	//
	// if itemCategory is 0xffffffff, then it is a search
	// for a resource container with a specific name.
	// itemTemplateId contains the crc value of the resource name
	//
	// else if itemCategory is GOT type GOT_resource_container
	// and itemTemplateId is != 0, then it is a search
	// for a resource container of a particular resource
	// class (or derives from the particular resource class).
	// itemTemplateId contains the crc value
	// of the name of the resource class
	//
	// else if itemTemplateId is == 0, then it is a
	// search for items belonging to the GOT type specified
	// by itemCategory
	//
	// else if itemTemplateId is != 0, then it is a
	// search for a specific item type. itemCategory
	// specifies the GOT and itemTemplateId is the
	// crc value of the object template

	// The min and max price of the items we are looking for
	int const priceFilterMin = message.GetPriceFilterMin();
	int const priceFilterMax = message.GetPriceFilterMax();

	// Advanced "attribute value" search
	std::list<AuctionQueryHeadersMessage::SearchCondition> const & advancedSearch = message.getAdvancedSearch();
	bool const useAdvancedSearch = !advancedSearch.empty();

	// For the price filter, do we want to include the entrance fee?
	bool const priceFilterIncludesFee = message.GetPriceFilterIncludesFee();

	// Indicates searching for resource containers
	bool const searchForResourceContainer = (itemCategory != 0) && ((static_cast<unsigned int>(itemCategory) == 0xffffffff) || (AuctionItem::IsCategoryResourceContainer(itemCategory)));

	bool hasMorePages = false;
	uint queryOffset = message.GetQueryOffset();
	uint numOffset = 0;
	int numAuctions = 0;
	std::vector<AuctionDataHeader> headers;

	bool allowSearchVendors = true;

	if (message.GetQueryType() != AST_ByPlayerOffersToVendor &&
		message.GetQueryType() != AST_ByVendorSelling        &&
		message.GetQueryType() != AST_ByPlayerStockroom      &&
		message.GetQueryType() != AST_ByVendorOffers         &&
		message.GetQueryType() != AST_ByVendorStockroom)
	{
		allowSearchVendors = false;
	}

	bool checkLocationInfo = false;
	static std::map<NetworkId, AuctionLocation *> emptyAuctionLocationList;
	std::map<NetworkId, AuctionLocation *>::iterator locationIter = emptyAuctionLocationList.begin();;
	std::map<NetworkId, AuctionLocation *>::iterator locationIterEnd = emptyAuctionLocationList.end();

	// specific vendor is specified
	// covers all cases of a non vendor owner accessing a vendor and all cases
	// where "this vendor" radio button is selected from the location filter
	if (message.GetSearchAuctionLocationId().isValid())
	{
		locationIter = m_locationIdMap.find(message.GetSearchAuctionLocationId());
		locationIterEnd = locationIter;

		if (locationIterEnd != m_locationIdMap.end())
		{
			++locationIterEnd;
		}
	}
	// only search vendors owned by the player
	// covers all cases of a vendor owner accessing a vendor he owns
	else if (message.GetSearchMyVendorsOnly())
	{
		checkLocationInfo = true;
		std::map<NetworkId, std::map<NetworkId, AuctionLocation *> >::iterator iterFind = m_playerVendorListMap.find(message.GetPlayerId());
		if (iterFind != m_playerVendorListMap.end())
		{
			locationIter = iterFind->second.begin();
			locationIterEnd = iterFind->second.end();
		}
	}
	// the rest of the cases just need to cover the various cases of browsing the bazaar
	// bazaar "all auctions"
	else if (message.GetQueryType() == AST_ByAll)
	{
		if (message.GetSearchStringPlanet().empty())
		{
			locationIter = m_allBazaar.begin();
			locationIterEnd =  m_allBazaar.end();
		}
		else if (message.GetSearchStringRegion().empty())
		{
			std::map<std::string, std::map<NetworkId, AuctionLocation *> >::iterator iterFind = m_bazaarByPlanet.find(message.GetSearchStringPlanet());
			if (iterFind != m_bazaarByPlanet.end())
			{
				locationIter = iterFind->second.begin();
				locationIterEnd = iterFind->second.end();
			}
		}
		else
		{
			std::map<std::pair<std::string, std::string>, std::map<NetworkId, AuctionLocation *> >::iterator iterFind = m_bazaarByRegion.find(std::make_pair(message.GetSearchStringPlanet(), message.GetSearchStringRegion()));
			if (iterFind != m_bazaarByRegion.end())
			{
				locationIter = iterFind->second.begin();
				locationIterEnd = iterFind->second.end();
			}
		}
	}
	// bazaar "my bids" and "my sales"
	else if ((message.GetQueryType() == AST_ByPlayerBids) || (message.GetQueryType() == AST_ByPlayerSales))
	{
		locationIter = m_allBazaar.begin();
		locationIterEnd =  m_allBazaar.end();
	}
	// bazaar "available items"
	else if (message.GetQueryType() == AST_ByPlayerStockroom)
	{
		locationIter = m_locationIdMap.begin();
		locationIterEnd = m_locationIdMap.end();
	}
	// bazaar "vendor location"
	else if (message.GetQueryType() == AST_ByVendorSelling)
	{
		if (message.GetOverrideVendorSearchFlag())
		{
			checkLocationInfo = true;
			locationIter = m_locationIdMap.begin();
			locationIterEnd = m_locationIdMap.end();
		}
		else if (message.GetSearchStringPlanet().empty())
		{
			locationIter = m_allSearchableVendor.begin();
			locationIterEnd = m_allSearchableVendor.end();
		}
		else if (message.GetSearchStringRegion().empty())
		{
			std::map<std::string, std::map<NetworkId, AuctionLocation *> >::iterator iterFind = m_searchableVendorByPlanet.find(message.GetSearchStringPlanet());
			if (iterFind != m_searchableVendorByPlanet.end())
			{
				locationIter = iterFind->second.begin();
				locationIterEnd = iterFind->second.end();
			}
		}
		else
		{
			std::map<std::pair<std::string, std::string>, std::map<NetworkId, AuctionLocation *> >::iterator iterFind = m_searchableVendorByRegion.find(std::make_pair(message.GetSearchStringPlanet(), message.GetSearchStringRegion()));
			if (iterFind != m_searchableVendorByRegion.end())
			{
				locationIter = iterFind->second.begin();
				locationIterEnd = iterFind->second.end();
			}
		}
	}

	int debugNumberLocationsTested = 0;
	int debugNumberLocationsMatched = 0;
	int debugNumberAuctionsTested = 0;

	std::map<NetworkId, Auction *> *auctionsPtr = nullptr;
	int entranceCharge = 0;
	AuctionLocation *locationPtr = nullptr;
	Auction *auctionPtr = nullptr;
	std::map<NetworkId, Auction *>::const_iterator auctionIterator;
	AuctionDataHeader *header = nullptr;
	bool checkItemTemplate;
	while (locationIter != locationIterEnd)
	{
		++debugNumberLocationsTested;

		checkItemTemplate = false;
		auctionsPtr = nullptr;
		entranceCharge = 0;
		locationPtr = (*locationIter).second;

		if (locationPtr->MatchLocation(message.GetSearchStringPlanet(), message.GetSearchStringRegion(), message.GetSearchAuctionLocationId(), checkLocationInfo, message.GetSearchMyVendorsOnly(), message.GetOverrideVendorSearchFlag(), message.GetPlayerId(), allowSearchVendors, message.GetVendorId(), message.GetQueryType()))
		{
			++debugNumberLocationsMatched;

			entranceCharge = locationPtr->GetEntranceCharge();

			if (message.GetQueryType() == AST_ByVendorOffers ||
				message.GetQueryType() == AST_ByPlayerOffersToVendor ||
				message.GetQueryType() == AST_ByVendorStockroom ||
				(message.GetQueryType() == AST_ByPlayerStockroom &&
				 locationPtr->IsVendorMarket()))
			{
				// we are using a list of all items, so we will need to check
				// to see if the player specified a template and/or item type,
				// and if so, ignore any items that doesn't match the template
				// and/or item type
				//
				// in all other cases, we use the template and/or item type
				// that the player specified, and only look at the item list
				// of that particular template and/or item type, so we know that
				// every item in that list matches the specified template and/or
				// item type
				checkItemTemplate = true;

				auctionsPtr = &(locationPtr->GetVendorOffers());
			}
			else
			{
				if (searchForResourceContainer)
				{
					auctionsPtr = &(locationPtr->GetAuctionsResourceContainer());
				}
				else if (itemCategory != 0)
				{
					if (itemTemplateId == 0)
					{
						if (itemCategoryMatchExact)
						{
							auctionsPtr = &(locationPtr->GetAuctionsByParentTypeExactMatch(itemCategory));
						}
						else
						{
							auctionsPtr = &(locationPtr->GetAuctionsByType(itemCategory));
						}
					}
					else
					{
						auctionsPtr = &(locationPtr->GetAuctionsByTemplate(itemCategory, itemTemplateId));
					}
				}
				else
				{
					auctionsPtr = &(locationPtr->GetAuctions());
				}
			}
		}

		if (auctionsPtr)
		{
			for (auctionIterator = auctionsPtr->begin(); auctionIterator != auctionsPtr->end(); ++auctionIterator)
			{
				++debugNumberAuctionsTested;

				auctionPtr = (*auctionIterator).second;
				const AuctionItem &item = auctionPtr->GetItem();
				header = nullptr;

				// Check to see if the item template matches
				if (searchForResourceContainer && (itemTemplateId != 0))
				{
					if (checkItemTemplate && !AuctionItem::IsCategoryResourceContainer(item.GetCategory()))
					{
						continue;
					}

					if (static_cast<unsigned int>(itemCategory) == 0xffffffff)
					{
						if (item.GetResourceNameCrc() != itemTemplateId)
						{
							continue;
						}
					}
					else if (!IsResourceClassDerivedFrom(item.GetResourceContainerClassCrc(), itemTemplateId))
					{
						continue;
					}
				}
				else if (checkItemTemplate && (itemCategory != 0))
				{
					if (itemCategoryMatchExact)
					{
						if (itemCategory != item.GetCategory())
						{
							continue;
						}
					}
					else if (!GameObjectTypes::isTypeOf(item.GetCategory(), itemCategory))
					{
						continue;
					}

					if ((itemTemplateId != 0) && (itemTemplateId != item.GetItemTemplateId()))
					{
						continue;
					}
				}

				// Check to see if the text matches
				if (!((textFilterAllTokens.empty() && textFilterAnyTokens.empty()) ||
					IsTextFilterMatch(item.GetName(), textFilterAllTokens, textFilterAnyTokens)))
				{
					continue;
				}

				// Check to see if the price matches
				if (!(((priceFilterMin == 0) && (priceFilterMax == 0)) ||
					IsPriceFilterMatch(auctionPtr, entranceCharge, priceFilterMin, priceFilterMax, priceFilterIncludesFee)))
				{
					continue;
				}

				// Check to see if the advanced search matches
				if (!(!useAdvancedSearch || auctionPtr->IsAdvancedFilterMatch(advancedSearch, static_cast<int>(message.getAdvancedSearchMatchAllAny()))))
				{
					continue;
				}

				switch(message.GetQueryType())
				{
					case AST_ByLocation:
					case AST_ByAll:
					case AST_ByVendorOffers:
					case AST_ByVendorSelling:
						if (auctionPtr->IsActive())
						{
							if (numOffset >= queryOffset)
							{
								header = MakeDataHeader(
									message.GetPlayerId(), auctionPtr,
									static_cast<int>(message.GetQueryType()), entranceCharge);
							}
							else
							{
								++numOffset;
							}
						}
						break;

					case AST_ByPlayerSales:
						if (auctionPtr->IsActive() &&
						    auctionPtr->GetCreatorId() == message.GetPlayerId())
						{
							if (numOffset >= queryOffset)
							{
								header = MakeDataHeader(
									message.GetPlayerId(), auctionPtr,
									static_cast<int>(message.GetQueryType()), entranceCharge);
							}
							else
							{
								++numOffset;
							}
						}
						break;

					case AST_ByPlayerStockroom:
					case AST_ByVendorStockroom:
						if (!auctionPtr->IsActive() &&
						    auctionPtr->GetItem().GetOwnerId() == message.GetPlayerId())
						{
							if (numOffset >= queryOffset)
							{
								header = MakeDataHeader(
									message.GetPlayerId(), auctionPtr,
									static_cast<int>(message.GetQueryType()), entranceCharge);
							}
							else
							{
								++numOffset;
							}
						}
						break;

					case AST_ByPlayerOffersToVendor:
						if (auctionPtr->IsActive() &&
						    auctionPtr->GetCreatorId() == message.GetPlayerId())
						{
							if (numOffset >= queryOffset)
							{
								header = MakeDataHeader(
									message.GetPlayerId(), auctionPtr,
									static_cast<int>(message.GetQueryType()), entranceCharge);
							}
							else
							{
								++numOffset;
							}
						}
						break;

					case AST_ByPlayerBids:
					{
						if (!auctionPtr->IsActive())
						{
							break;
						}
						const AuctionBid *bid = auctionPtr->GetPlayerBid(
							message.GetPlayerId());
						if (bid)
						{
							if (numOffset >= queryOffset)
							{
								header = MakeDataHeader(
									message.GetPlayerId(), auctionPtr,
									static_cast<int>(message.GetQueryType()), entranceCharge,
									bid);
							}
							else
							{
								++numOffset;
							}
						}
						break;
					}

					case AST_ByCategory:
						if (auctionPtr->IsActive())
						{
							if (numOffset >= queryOffset)
							{
								header = MakeDataHeader(
									message.GetPlayerId(), auctionPtr,
									static_cast<int>(message.GetQueryType()), entranceCharge);
							}
							else
							{
								++numOffset;
							}
						}
						break;
				}

				if (header)
				{
					headers.push_back(*header);
					delete header;

					++numAuctions;
					if (numAuctions >= ConfigCommodityServer::getMaxAuctionsPerPage())
					{
						hasMorePages = true;
						break;
					}
				}
			}
		}

		// If we found enough auctions, stop looking
		if (numAuctions >= ConfigCommodityServer::getMaxAuctionsPerPage())
		{
			break;
		}

		++locationIter;
	}

#ifdef _DEBUG
	if (m_showAllDebugInfo || message.GetOverrideVendorSearchFlag())
#else
	if (message.GetOverrideVendorSearchFlag())
#endif
	{
		char buffer[4096];

		snprintf(buffer, sizeof(buffer)-1, "(%d / %d / %d) locations tested, (%d / %d) auctions tested", debugNumberLocationsMatched, debugNumberLocationsTested, m_locationIdMap.size(), debugNumberAuctionsTested, m_auctions.size());
		buffer[sizeof(buffer)-1] = '\0';
		debugOutput += buffer;
		debugOutput += "\r\n";
#ifdef _DEBUG
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryAuctionHeadersMessage] : %s\n", buffer));
#endif

		if (message.GetOverrideVendorSearchFlag())
		{
			GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(message.GetTrackId());
			if (gameServerConn)
			{
				GenericValueTypeMessage<std::pair<NetworkId, std::string> > const msg("DisplayStringForPlayer", std::make_pair(message.GetPlayerId(), debugOutput));
				gameServerConn->send(msg, true);
			}
		}
	}

	OnQueryAuctionHeaders(message.GetTrackId(), ARC_Success,
		message.GetResponseId(), message.GetPlayerId(),
		message.GetQueryType(), headers, queryOffset, hasMorePages);
}

// ----------------------------------------------------------------------

void AuctionMarket::SetGameTime(const SetGameTimeMessage &message)
{
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server SetGameTimeMessage] : GameTime : %d.\n", message.GetGameTime()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server SetGameTimeMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server SetGameTimeMessage] : TrackId : %d.\n", message.GetTrackId()));

	m_gameTime = message.GetGameTime();
}

// ----------------------------------------------------------------------

void AuctionMarket::GetItemDetails(const GetItemDetailsMessage &message)
{

	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetItemDetailsMessage] : PlayerId : %s.\n", message.GetPlayerId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetItemDetailsMessage] : AuctionId : %s.\n", message.GetAuctionId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetItemDetailsMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetItemDetailsMessage] : TrackId : %d.\n", message.GetTrackId()));

	std::map<NetworkId, Auction *>::iterator iter = m_auctions.find(
		message.GetAuctionId());
	if (iter == m_auctions.end())
	{
		static const std::vector<std::pair<std::string, Unicode::String> > emptyAttributes;
		OnGetItemDetails(message.GetTrackId(), ARC_AuctionDoesNotExist,
			message.GetResponseId(), message.GetAuctionId(),
			message.GetPlayerId(), 0, Unicode::emptyString, 0, Unicode::emptyString, emptyAttributes);
		return;
	}

	Auction *auction = (*iter).second;
	OnGetItemDetails(message.GetTrackId(), ARC_Success,
					 message.GetResponseId(), message.GetAuctionId(),
					 message.GetPlayerId(),
					 auction->GetUserDescriptionLength(), auction->GetUserDescription(),
					 auction->GetOobLength(), auction->GetOobData(),
					 auction->GetAttributes());
}

// ----------------------------------------------------------------------

void AuctionMarket::GetItem(const GetItemMessage &message)
{

	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetItemMessage] : PlayerId : %s.\n", message.GetPlayerId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetItemMessage] : ItemId : %s.\n", message.GetItemId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetItemMessage] : Location : %s.\n", message.GetLocation().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetItemMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetItemMessage] : TrackId : %d.\n", message.GetTrackId()));

	AuctionLocation &location = GetLocation(std::string(message.GetLocation()));
	Auction *auction = location.GetAuction(message.GetItemId());
	if (!auction)
	{
		OnGetItem(message.GetTrackId(), ARC_AuctionDoesNotExist,
			message.GetResponseId(), message.GetItemId(),
			message.GetPlayerId(), message.GetLocation());
		return;
	}
	if (auction->GetItem().GetOwnerId() != message.GetPlayerId())
	{
		OnGetItem(message.GetTrackId(), ARC_NotItemOwner,
			message.GetResponseId(), message.GetItemId(),
			message.GetPlayerId(), message.GetLocation());
		return;
	}

	LOG("CommoditiesServer", ("Player %s retrived item %s at location %s",
		auction->GetItem().GetOwnerId().getValueString().c_str(),
		auction->GetItem().GetItemId().getValueString().c_str(),
		auction->GetLocation().GetLocationId().getValueString().c_str()));
	LOG("CustomerService", ("Auction: Player %s retrived item %s at location %s",
		auction->GetItem().GetOwnerId().getValueString().c_str(),
		auction->GetItem().GetItemId().getValueString().c_str(),
		auction->GetLocation().GetLocationId().getValueString().c_str()));
	std::map<NetworkId, Auction *>::iterator i = m_auctions.find(auction->GetItem().GetItemId());
	if (i != m_auctions.end())
	{
		LOG("CommoditiesServer", ("Auction: Auction for item %s being deleted by the Commodities Server because item is being retrieved.", (*i).second->GetItem().GetItemId().getValueString().c_str()));
		LOG("CustomerService", ("Auction: Auction for item %s being deleted by the Commodities Server because item is being retrieved.", (*i).second->GetItem().GetItemId().getValueString().c_str()));
		DestroyAuction(i);
	}

	OnGetItem(message.GetTrackId(), ARC_Success,
		message.GetResponseId(), message.GetItemId(),
		message.GetPlayerId(), message.GetLocation());
}

// ----------------------------------------------------------------------

void AuctionMarket::CleanupInvalidItemRetrieval( const CleanupInvalidItemRetrievalMessage &message)
{

	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CleanupInvalidItemRetrievalMessage] : ItemId : %s.\n", message.GetItemId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CleanupInvalidItemRetrievalMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CleanupInvalidItemRetrievalMessage] : TrackId : %d.\n", message.GetTrackId()));

	//printf( "Received command to cleanup an invalid item.\n");
        std::map<NetworkId, Auction *>::iterator itemIterator = m_auctions.find( message.GetItemId());
	if( itemIterator != m_auctions.end() )
	{
		LOG("CommoditiesServer", ("CleanupInvalidItemRetrieval for item %s.",
			message.GetItemId().getValueString().c_str()));
		LOG("CustomerService", ("Auction: CleanupInvalidItemRetrieval for item %s.",
			message.GetItemId().getValueString().c_str()));
		int reimburseAmt = 0;
		Auction *item = (*itemIterator).second;
		if( item->GetCreatorId() != item->GetItem().GetOwnerId() )
		{
			//printf("Player needs reimbursed for item = %d.\n", item->GetItem().GetItemId() );

			// figure out the price.
			// For instant sales, the buy now price
			if( item->IsImmediate() )
			{
				reimburseAmt = item->GetBuyNowPriceWithSalesTax();
				//printf( "Reimbursing player %d credits for invalid instant auction.\n", reimburseAmt );
			}
			else
			{
				reimburseAmt = item->GetHighBid()->GetBid();
				//printf( "Reimbursing player %d credits for invalid bid auction.\n", reimburseAmt );
			}
			// For bid items, the highest bid
		}
		OnCleanupInvalidItemRetrieval(message.GetTrackId(), message.GetResponseId(), item->GetItem().GetItemId(), item->GetItem().GetOwnerId(), item->GetCreatorId(), reimburseAmt);
		LOG("CommoditiesServer", ("Auction: Auction for item %s being deleted by the Commodities Server because of cleanup of invalied item retieval.", (*itemIterator).second->GetItem().GetItemId().getValueString().c_str()));
		LOG("CustomerService", ("Auction: Auction for item %s being deleted by the Commodities Server because of cleanup of invalied item retieval.", (*itemIterator).second->GetItem().GetItemId().getValueString().c_str()));

		//printf( "Deleting auction item = %d\n", item->GetItem().GetItemId() );
		DestroyAuction(itemIterator);
	}
}

// ----------------------------------------------------------------------

void AuctionMarket::DestroyVendorMarket(const DestroyVendorMarketMessage &message)
{

	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server DestroyVendorMarketMessage] : OwnerId : %s.\n", message.GetOwnerId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server DestroyVendorMarketMessage] : OwnerName : %s.\n", message.GetOwnerName().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server DestroyVendorMarketMessage] : Location : %s.\n", message.GetLocation().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server DestroyVendorMarketMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server DestroyVendorMarketMessage] : TrackId : %d.\n", message.GetTrackId()));

	std::map<NetworkId, AuctionLocation *>::iterator i = m_locationIdMap.find(GetLocationId(message.GetLocation()));
	if( i != m_locationIdMap.end() )
	{
		LOG("CommoditiesServer", ("Remove location %s owned by %s (%s) from Commodities Server",
			GetLocationId(message.GetLocation()).getValueString().c_str(),
			message.GetOwnerName().c_str(),
			message.GetOwnerId().getValueString().c_str()));
		LOG("CustomerService", ("Vendor: Remove location %s owned by %s (%s) from Commodities Server",
			GetLocationId(message.GetLocation()).getValueString().c_str(),
			message.GetOwnerName().c_str(),
			message.GetOwnerId().getValueString().c_str()));
		std::vector<std::map<NetworkId, Auction *>::iterator> destroyedAuctions;
		std::map<NetworkId, Auction *>::iterator item;
		// get all the auctions for this location and get an iterator from the real auctions map
		for( std::map<NetworkId, Auction*>::iterator j = (*i).second->GetAuctions().begin(); j != (*i).second->GetAuctions().end(); j++ )
		{
			item = m_auctions.find((*j).first);
			if( item != m_auctions.end() )
			{
				destroyedAuctions.push_back(item);
			}
		}

		// do it again for all vendor offers
		for( std::map<NetworkId, Auction*>::iterator k = (*i).second->GetVendorOffers().begin(); k != (*i).second->GetVendorOffers().end(); k++ )
		{
			item = m_auctions.find((*k).first);
			if( item != m_auctions.end() )
			{
				destroyedAuctions.push_back(item);
			}
		}


		// destroy the auctions
		std::vector<std::map<NetworkId, Auction *>::iterator>::iterator ri = destroyedAuctions.begin();
		std::map<NetworkId, Auction *>::iterator auctionsIterator;
		while (ri != destroyedAuctions.end())
		{
			auctionsIterator = *ri;
			//(*auctionsIterator).second->Expire(false, false);
			LOG("CommoditiesServer", ("Auction: Auction for item %s being deleted by the Commodities Server because vendor is being destroyed by the owner.", (*auctionsIterator).second->GetItem().GetItemId().getValueString().c_str()));
			LOG("CustomerService", ("Auction: Auction for item %s being deleted by the Commodities Server because vendor is being destroyed by the owner.", (*auctionsIterator).second->GetItem().GetItemId().getValueString().c_str()));
			DestroyAuction(auctionsIterator);
			++ri;
		}

		// get rid of the location from m_locationMap
		AuctionLocation *al = (*i).second;
		if (al->GetOwnerId() != zeroNetworkId)
		{
			RemovePlayerVendor(al->GetOwnerId(), al->GetLocationId());
		}
		m_locationIdMap.erase(i);

		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Creating new DeleteLocationMessage.\n"));

		CMDeleteLocationMessage msg(al->GetLocationId());
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Location Id : %s.\n", msg.GetLocationId().getValueString().c_str()));

		DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
		if (dbServer)
			dbServer->send(msg, true);

		delete al;
	}
}

// ----------------------------------------------------------------------

void AuctionMarket::DeleteAuctionLocation(const DeleteAuctionLocationMessage &message)
{
	std::string replyMessage;

	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server DeleteAuctionLocationMessage] : LocationId : %s.\n", message.GetLocationId().getValueString().c_str()));

	std::map<NetworkId, AuctionLocation *>::iterator i = m_locationIdMap.find(message.GetLocationId());
	if( i != m_locationIdMap.end() )
	{
		LOG("CommoditiesServer", ("Remove location %s owned by %s from Commodities Server",
			message.GetLocationId().getValueString().c_str(),
			(*i).second->GetOwnerId().getValueString().c_str()));

		LOG("CustomerService", ("Vendor: Remove location %s owned by %s from Commodities Server",
			message.GetLocationId().getValueString().c_str(),
			(*i).second->GetOwnerId().getValueString().c_str()));
		std::vector<std::map<NetworkId, Auction *>::iterator> destroyedAuctions;
		std::map<NetworkId, Auction *>::iterator item;
		// get all the auctions for this location and get an iterator from the real auctions map
		for( std::map<NetworkId, Auction*>::iterator j = (*i).second->GetAuctions().begin(); j != (*i).second->GetAuctions().end(); j++ )
		{
			item = m_auctions.find((*j).first);
			if( item != m_auctions.end() )
			{
				destroyedAuctions.push_back(item);
			}
		}

		// do it again for all vendor offers
		for( std::map<NetworkId, Auction*>::iterator k = (*i).second->GetVendorOffers().begin(); k != (*i).second->GetVendorOffers().end(); k++ )
		{
			item = m_auctions.find((*k).first);
			if( item != m_auctions.end() )
			{
				destroyedAuctions.push_back(item);
			}
		}


		// destroy the auctions
		std::vector<std::map<NetworkId, Auction *>::iterator>::iterator ri = destroyedAuctions.begin();
		std::map<NetworkId, Auction *>::iterator auctionsIterator;
		while (ri != destroyedAuctions.end())
		{
			auctionsIterator = *ri;
			//(*auctionsIterator).second->Expire(false, false);
			LOG("CommoditiesServer", ("Auction: Auction for item %s being deleted by the Commodities Server because auction location is being deleted from DeleteAuctionLocationMessage request.", (*auctionsIterator).second->GetItem().GetItemId().getValueString().c_str()));
			LOG("CustomerService", ("Auction: Auction for item %s being deleted by the Commodities Server because auction location is being deleted from DeleteAuctionLocationMessage request.", (*auctionsIterator).second->GetItem().GetItemId().getValueString().c_str()));
			DestroyAuction(auctionsIterator);
			++ri;
		}

		// get rid of the location from m_locationMap
		AuctionLocation *al = (*i).second;
		if (al->GetOwnerId() != zeroNetworkId)
		{
			RemovePlayerVendor(al->GetOwnerId(), al->GetLocationId());
		}
		m_locationIdMap.erase(i);

		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Creating new DeleteLocationMessage.\n"));

		replyMessage = std::string("Location ") + message.GetLocationId().getValueString().c_str() + " owned by " + al->GetOwnerId().getValueString().c_str() + " is deleted.";
		CMDeleteLocationMessage msg(al->GetLocationId());
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Location Id : %s.\n", msg.GetLocationId().getValueString().c_str()));

		DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
		if (dbServer)
			dbServer->send(msg, true);

		delete al;
	}
	else
		replyMessage = std::string("Location ") + message.GetLocationId().getValueString().c_str() + " does not exist.";

	GenericValueTypeMessage<std::pair<std::string, std::string> >
		reply("OnCommodityReplyMessage",
		std::make_pair(message.GetWhoRequested(), replyMessage));
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(message.GetTrackId());
	if (gameServerConn)
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnCommodityReplyMessage callback to GamerServer ID: %d.\n", message.GetTrackId()));
		gameServerConn->send(reply, true);
	}
	else
	{
		WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnAddAuctionMessage.\n", message.GetTrackId()));
	}

}

// ----------------------------------------------------------------------

void AuctionMarket::UpdateVendorStatus(const UpdateVendorStatusMessage &message)
{
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server UpdateVendorStatus] : VendorId : %s.\n", message.GetVendorId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server UpdateVendorStatus] : Location : %s.\n", message.GetLocation().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server UpdateVendorStatus] : Status : 0x%0X.\n", message.GetStatus()));

	std::map<NetworkId, AuctionLocation *>::const_iterator i = m_locationIdMap.find(GetLocationId(message.GetLocation()));

	if (i != m_locationIdMap.end())
	{
		(*i).second->SetLocationString(message.GetLocation());
		(*i).second->SetFullStatus(message.GetStatus(), true);

		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server UpdateVendorStatus] : LocationId : %s.\n", (*i).second->GetLocationId().getValueString().c_str()));
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server UpdateVendorStatus] : Full Status : 0x%0X.\n", (*i).second->GetFullStatus()));

		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Creating new UpdateLocationMessage.\n"));
		CMUpdateLocationMessage msg(
			(*i).second->GetLocationId(),
			(*i).second->GetOwnerId(),
			(*i).second->GetLocationString(),
			(*i).second->GetSalesTax(),
			(*i).second->GetSalesTaxBankId(),
			(*i).second->GetEmptyDate(),
			(*i).second->GetLastAccessDate(),
			(*i).second->GetInactiveDate(),
			(*i).second->GetFullStatus(),
			(*i).second->GetSearchEnabled(),
			(*i).second->GetEntranceCharge()
			);

		DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
		if (dbServer)
			dbServer->send(msg, true);
	}
	else
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server UpdateVendorStatus] : Update failed - Vendor not found!\n"));
}

// ----------------------------------------------------------------------

void AuctionMarket::UpdateVendorLocation(const NetworkId &locationId, const std::string &locationString)
{
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server UpdateVendorLocation] : VendorId : %s.\n", locationId.getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server UpdateVendorLocation] : Location : %s.\n", locationString.c_str()));

	std::map<NetworkId, AuctionLocation *>::const_iterator i = m_locationIdMap.find(locationId);

	if (i != m_locationIdMap.end())
	{
		if  ((*i).second->GetLocationString() != locationString)
		{
			LOG("CustomerService", ("Vendor: Fixing up vendor %s (owner %s) location from (%s) to (%s)", (*i).second->GetLocationId().getValueString().c_str(), (*i).second->GetOwnerId().getValueString().c_str(), (*i).second->GetLocationString().c_str(), locationString.c_str()));

			(*i).second->SetLocationString(locationString);

			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Creating new UpdateLocationMessage.\n"));
			CMUpdateLocationMessage msg(
				(*i).second->GetLocationId(),
				(*i).second->GetOwnerId(),
				(*i).second->GetLocationString(),
				(*i).second->GetSalesTax(),
				(*i).second->GetSalesTaxBankId(),
				(*i).second->GetEmptyDate(),
				(*i).second->GetLastAccessDate(),
				(*i).second->GetInactiveDate(),
				(*i).second->GetFullStatus(),
				(*i).second->GetSearchEnabled(),
				(*i).second->GetEntranceCharge()
				);

			DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
			if (dbServer)
				dbServer->send(msg, true);
		}
	}
	else
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server UpdateVendorLocation] : Update failed - Vendor not found!\n"));
}

// ----------------------------------------------------------------------

void AuctionMarket::CreateVendorMarket(const CreateVendorMarketMessage &message)
{

	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CreateVendorMarketMessage] : OwnerId : %s.\n", message.GetOwnerId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CreateVendorMarketMessage] : Location : %s.\n", message.GetLocation().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CreateVendorMarketMessage] : Vendor Limit : %d.\n", message.GetVendorLimit()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CreateVendorMarketMessage] : Entrance Charge : %d.\n", message.GetEntranceCharge()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CreateVendorMarketMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CreateVendorMarketMessage] : TrackId : %d.\n", message.GetTrackId()));

	std::map<NetworkId, AuctionLocation *>::const_iterator i =
		m_locationIdMap.find(GetLocationId(message.GetLocation()));
	if (i != m_locationIdMap.end())
	{
		// check the current owner and location string - update if needed
		if (((*i).second->GetOwnerId() != message.GetOwnerId()) || ((*i).second->GetLocationString() != message.GetLocation()))
		{
			// check the player's vendor limit
			if (GetVendorCount(message.GetOwnerId()) >= message.GetVendorLimit())
			{
				OnCreateVendorMarket(message.GetTrackId(),
					ARC_LocationVendorLimitExceeded,
					message.GetResponseId(),
					message.GetOwnerId(),
					message.GetLocation());
				DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CreateVendorMarketMessage] : Vendor Limit Exceeded for Player %s. (Current: %d, Limit: %d)\n", message.GetOwnerId().getValueString().c_str(), GetVendorCount(message.GetOwnerId()), message.GetVendorLimit()));
				return;
			}

			RemovePlayerVendor((*i).second->GetOwnerId(), (*i).second->GetLocationId());
			AddPlayerVendor(message.GetOwnerId(), (*i).second->GetLocationId(), (*i).second);
			(*i).second->SetOwnerId(message.GetOwnerId());
			(*i).second->SetLocationString(message.GetLocation());
			(*i).second->SetEntranceCharge(message.GetEntranceCharge());
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Creating new UpdateLocationMessage.\n"));
			CMUpdateLocationMessage msg(
				(*i).second->GetLocationId(),
				(*i).second->GetOwnerId(),
				(*i).second->GetLocationString(),
				(*i).second->GetSalesTax(),
				(*i).second->GetSalesTaxBankId(),
				(*i).second->GetEmptyDate(),
				(*i).second->GetLastAccessDate(),
				(*i).second->GetInactiveDate(),
				(*i).second->GetFullStatus(),
				(*i).second->GetSearchEnabled(),
				(*i).second->GetEntranceCharge()
			);
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Location Id : %s.\n", msg.GetLocationId().getValueString().c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Location OwnerId : %s.\n", msg.GetOwnerId().getValueString().c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Location Name : %s.\n", msg.GetLocationString().c_str()));

			DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
			if (dbServer)
				dbServer->send(msg, true);
		}
		// This should only be for the reinitialize case
		else if (GetVendorCount(message.GetOwnerId()) > message.GetVendorLimit())
		{
			OnCreateVendorMarket(message.GetTrackId(),
				ARC_LocationVendorLimitExceeded,
				message.GetResponseId(),
				message.GetOwnerId(),
				message.GetLocation());
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CreateVendorMarketMessage] : Vendor Limit Exceeded for Player %s. (Current: %d, Limit: %d)\n", message.GetOwnerId().getValueString().c_str(), GetVendorCount(message.GetOwnerId()), message.GetVendorLimit()));
			return;
		}

		OnCreateVendorMarket(message.GetTrackId(),
			ARC_LocationAlreadyExists,
			message.GetResponseId(),
			message.GetOwnerId(),
			message.GetLocation());
			return;
	}

	// check the player's vendor limit
	if (GetVendorCount(message.GetOwnerId()) >= message.GetVendorLimit())
	{
		OnCreateVendorMarket(message.GetTrackId(),
			ARC_LocationVendorLimitExceeded,
			message.GetResponseId(),
			message.GetOwnerId(),
			message.GetLocation());
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server CreateVendorMarketMessage] : Vendor Limit Exceeded for Player %s. (Current: %d, Limit: %d)\n", message.GetOwnerId().getValueString().c_str(), GetVendorCount(message.GetOwnerId()), message.GetVendorLimit()));
		return;
	}

	CreateLocation(message.GetOwnerId(), message.GetLocation(), message.GetEntranceCharge());

	OnCreateVendorMarket(message.GetTrackId(),
		ARC_Success,
		message.GetResponseId(),
		message.GetOwnerId(),
		message.GetLocation());
}

// ----------------------------------------------------------------------

// returns the containerId as a string given a location string
bool AuctionMarket::GetContainerIdString( const std::string &loc, std::string &output )
{
	// format of location string is as follows:
	// planetname.regionname.vendorname.containerId#xLoc,zLoc
	const std::string delims1(".");
	const std::string delims2("#");
	bool retval = false;

	std::string::size_type begIdx, endIdx;

	begIdx = loc.find_first_not_of(delims1);
	// skip over first 3 '.'s
	for( int i=0; i<3; i++ )
	{
		begIdx = loc.find_first_of(delims1, begIdx);
		if( begIdx != std::string::npos )
		{
			begIdx = loc.find_first_not_of(delims1, begIdx);
		}
	}

	if( begIdx != std::string::npos )
	{
		endIdx = loc.find_first_of(delims2, begIdx);
		if( endIdx != std::string::npos )
		{
			output = loc.substr(begIdx, endIdx-begIdx);
			retval =  true;
		}
	}

	return retval;
}

// ----------------------------------------------------------------------

// returns the locationId as a number given a location string
NetworkId AuctionMarket::GetLocationId(const std::string &loc)
{
	// format of location string is as follows:
	// planetname.regionname.vendorname.containerId#xLoc,zLoc
	const std::string delims1(".");
	const std::string delims2("#");
	std::string output;
	bool found = false;

	std::string::size_type begIdx, endIdx;

	begIdx = loc.find_first_not_of(delims1);
	// skip over first 3 '.'s
	for( int i=0; i<3; i++ )
	{
		begIdx = loc.find_first_of(delims1, begIdx);
		if( begIdx != std::string::npos )
		{
			begIdx = loc.find_first_not_of(delims1, begIdx);
		}
	}

	if( begIdx != std::string::npos )
	{
		endIdx = loc.find_first_of(delims2, begIdx);
		if( endIdx != std::string::npos )
		{
			output = loc.substr(begIdx, endIdx-begIdx);
			found = true;
		}
	}

	if (found)
	{
		NetworkId retval(output);
		return retval;
	}
	else
	{
		NetworkId retval(int64(0));
		return retval;
	}
}

// ----------------------------------------------------------------------

// This function checks to see if the embedded containerId matches with any existing
// vendor location string.  If it does, it updates the location string for that match
//
// returns true if it fixed something, false otherwise
bool AuctionMarket::FixVendorLocation( const std::string &loc )
{
	std::map<NetworkId, AuctionLocation *>::iterator i = m_locationIdMap.begin();

	std::string containerIdString;
	if( !GetContainerIdString(loc, containerIdString) )
	{
		return false;
	}

	while( i != m_locationIdMap.end() )
	{
		AuctionLocation *al = (*i).second;
		if( al )
		{
			std::string oldStrLoc(al->GetLocationString());
			//printf( "Old strloc: %s\n", oldStrLoc);
			std::string checkId;
			//printf( "checking %s with %s\n", checkId, containerIdString);

			if( GetContainerIdString( oldStrLoc, checkId ) && checkId == containerIdString )
			{
				// update the location string
				al->SetLocationString( loc );

				DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Creating new UpdateLocationMessage.\n"));
				CMUpdateLocationMessage msg(
					al->GetLocationId(),
					al->GetOwnerId(),
					loc,
					al->GetSalesTax(),
					al->GetSalesTaxBankId(),
					al->GetEmptyDate(),
					al->GetLastAccessDate(),
					al->GetInactiveDate(),
					al->GetFullStatus(),
					al->GetSearchEnabled(),
					al->GetEntranceCharge()
				);
				DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Location Id : %s.\n", msg.GetLocationId().getValueString().c_str()));
				DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Location Name : %s.\n", msg.GetLocationString().c_str()));
				DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Location Sales Tax : %d.\n", msg.GetSalesTax()));
				DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Location Bank Id : %s.\n", msg.GetBankId().getValueString().c_str()));

				DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
				if (dbServer)
					dbServer->send(msg, true);

				return true;
			}
		}

		i++;
	}

	return false;
}

// ----------------------------------------------------------------------

void AuctionMarket::GetVendorOwner(const GetVendorOwnerMessage &message)
{

	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetVendorOwnerMessage] : OwnerId : %s.\n", message.GetOwnerId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetVendorOwnerMessage] : Location : %s.\n", message.GetLocation().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetVendorOwnerMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetVendorOwnerMessage] : TrackId : %d.\n", message.GetTrackId()));

	std::map<NetworkId, AuctionLocation *>::const_iterator i =
		m_locationIdMap.find(GetLocationId(message.GetLocation()));
	if (i == m_locationIdMap.end())
	{
		// if the region name changes, we lose track of an existing vendor
		// this function will fix up those vendors.
		if( FixVendorLocation(message.GetLocation()) )
		{
			// try again now...
			i = m_locationIdMap.find(GetLocationId(message.GetLocation()));
			if( i == m_locationIdMap.end() )
			{
				// still couldn't find the vendor
				OnGetVendorOwner(message.GetTrackId(), ARC_LocationAlreadyExists,
					message.GetResponseId(), zeroNetworkId,
					message.GetOwnerId(), message.GetLocation());
				return;
			}
		}
		else
		{	// couldn't find the vendor
			OnGetVendorOwner(message.GetTrackId(), ARC_LocationAlreadyExists,
				message.GetResponseId(), zeroNetworkId,
				message.GetOwnerId(), message.GetLocation());
			return;
		}
	}
	AuctionLocation *auctionLocation = (*i).second;

	OnGetVendorOwner(message.GetTrackId(), ARC_LocationAlreadyExists,
		message.GetResponseId(), auctionLocation->GetOwnerId(),
		message.GetOwnerId(), message.GetLocation());
}

// ----------------------------------------------------------------------

void AuctionMarket::GetVendorValue(const GetVendorValueMessage &message)
{

	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetVendorValueMessage] : Location : %s.\n", message.GetLocation().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetVendorValueMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetVendorValueMessage] : TrackId : %d.\n", message.GetTrackId()));

	std::map<NetworkId, AuctionLocation *>::const_iterator i =
		m_locationIdMap.find(GetLocationId(message.GetLocation()));
	if (i == m_locationIdMap.end())
	{
		OnGetVendorValue(message.GetTrackId(), message.GetResponseId(),
			message.GetLocation(), 0);
		return;
	}
	AuctionLocation *auctionLocation = (*i).second;

	OnGetVendorValue(message.GetTrackId(), message.GetResponseId(),
		message.GetLocation(), auctionLocation->GetValue());
}

// ----------------------------------------------------------------------

void AuctionMarket::SetSalesTax(const SetSalesTaxMessage &message)
{

	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server SetSalesTaxMessage] : GetBankId : %s.\n", message.GetBankId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server SetSalesTaxMessage] : GetSalesTax : %d.\n", message.GetSalesTax()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server SetSalesTaxMessage] : Location : %s.\n", message.GetLocation().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server SetSalesTaxMessage] : ResponseId : %d.\n", message.GetResponseId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server SetSalesTaxMessage] : TrackId : %d.\n", message.GetTrackId()));

	// printf("Received SetSalesTax request: sales tax=%ld, bankId=%Ld, location='%s'\n", message.GetSalesTax(), message.GetBankId().getValue(), message.GetLocation() );

	AuctionLocation &al = GetLocation(message.GetLocation());
	// make sure we got a real location
	if( al.GetLocationString() == message.GetLocation() )
	{
		if ((al.GetSalesTax() != message.GetSalesTax()) || (al.GetSalesTaxBankId() != message.GetBankId()))
		{
			al.SetSalesTax(message.GetSalesTax(), message.GetBankId());
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Creating new UpdateLocationMessage.\n"));
			CMUpdateLocationMessage msg(
				al.GetLocationId(),
				al.GetOwnerId(),
				al.GetLocationString(),
				al.GetSalesTax(),
				al.GetSalesTaxBankId(),
				al.GetEmptyDate(),
				al.GetLastAccessDate(),
				al.GetInactiveDate(),
				al.GetFullStatus(),
				al.GetSearchEnabled(),
				al.GetEntranceCharge()
			);
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Location Id : %s.\n", msg.GetLocationId().getValueString().c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Location Name : %s.\n", msg.GetLocationString().c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Location Sales Tax : %d.\n", msg.GetSalesTax()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Location Bank Id : %s.\n", msg.GetBankId().getValueString().c_str()));

			DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
			if (dbServer)
				dbServer->send(msg, true);
		}
	}
}

// ----------------------------------------------------------------------

void AuctionMarket::QueryVendorItemCount(const QueryVendorItemCountMessage &message)
{
	AuctionLocation &loc = GetLocation(message.GetVendorId());
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryVendorItemCountMessage] : TrackId : %d\n", message.GetTrackId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryVendorItemCountMessage] : VendorId : %s\n", message.GetVendorId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server QueryVendorItemCountMessage] : ItemCount : %d\n", loc.GetAuctionItemCount()));

	OnQueryVendorItemCount(message.GetResponseId(), message.GetTrackId(), message.GetVendorId(), loc.GetAuctionItemCount(), loc.GetSearchEnabled());
}

// ----------------------------------------------------------------------

void AuctionMarket::GetPlayerVendorCount(const GetPlayerVendorCountMessage &message)
{
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetPlayerVendorCountMessage] : TrackId : %d\n", message.GetTrackId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetPlayerVendorCountMessage] : VendorId : %s\n", message.GetPlayerId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server GetPlayerVendorCountMessage] : ItemCount : %d\n", GetVendorCount(message.GetPlayerId())));

	std::vector<NetworkId> vendorList;
	std::map<NetworkId, std::map<NetworkId, AuctionLocation *> >::const_iterator const iterPlayer = m_playerVendorListMap.find(message.GetPlayerId());
	if (iterPlayer != m_playerVendorListMap.end())
	{
		for (std::map<NetworkId, AuctionLocation *>::const_iterator iterAuctionLocation = iterPlayer->second.begin(); iterAuctionLocation != iterPlayer->second.end(); ++iterAuctionLocation)
		{
			vendorList.push_back(iterAuctionLocation->first);
		}
	}

	OnGetPlayerVendorCount(message.GetResponseId(), message.GetTrackId(), message.GetPlayerId(), vendorList.size(), vendorList);
}

// ----------------------------------------------------------------------

void AuctionMarket::UpdateVendorSearchOption(const UpdateVendorSearchOptionMessage &message)
{

	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server UpdateVendorSearchOptionMessage] : TrackId : %d\n", message.GetTrackId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server UpdateVendorSearchOptionMessage] : VendorId : %s\n", message.GetVendorId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server UpdateVendorSearchOptionMessage] : Enabled : %d\n", message.GetEnabled()));

	AuctionLocation &loc = GetLocation(message.GetVendorId());
	loc.SetSearchedEnabled(message.GetEnabled());
	CMUpdateLocationMessage msg(
		loc.GetLocationId(),
		loc.GetOwnerId(),
		loc.GetLocationString(),
		loc.GetSalesTax(),
		loc.GetSalesTaxBankId(),
		loc.GetEmptyDate(),
		loc.GetLastAccessDate(),
		loc.GetInactiveDate(),
		loc.GetFullStatus(),
		message.GetEnabled(),
		loc.GetEntranceCharge()
	);

	DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
	if (dbServer)
		dbServer->send(msg, true);

	OnUpdateVendorSearchOptionMessage reply(message.GetPlayerId(), message.GetEnabled());

	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(message.GetTrackId());
	if (gameServerConn)
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnUpdateVendorSearchOptionMessage callback to GamerServer ID: %d\n", message.GetTrackId()));
		gameServerConn->send(reply, true);
	}
	else
	{
		WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnAddAuctionMessage.\n", message.GetTrackId()));
	}
}

// ----------------------------------------------------------------------

void AuctionMarket::SetEntranceCharge(const SetEntranceChargeMessage &message)
{

	bool tempv = m_showAllDebugInfo;
	m_showAllDebugInfo = true;
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server SetEntranceChargeMessage] : TrackId : %d\n", message.GetTrackId()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server SetEntranceChargeMessage] : VendorId : %s\n", message.GetVendorId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server SetEntranceChargeMessage] : EntranceCharge : %d\n", message.GetEntranceCharge()));
	m_showAllDebugInfo = tempv;

	AuctionLocation &loc = GetLocation(message.GetVendorId());
	loc.SetEntranceCharge(message.GetEntranceCharge());
	CMUpdateLocationMessage msg(
		loc.GetLocationId(),
		loc.GetOwnerId(),
		loc.GetLocationString(),
		loc.GetSalesTax(),
		loc.GetSalesTaxBankId(),
		loc.GetEmptyDate(),
		loc.GetLastAccessDate(),
		loc.GetInactiveDate(),
		loc.GetFullStatus(),
		loc.GetSearchEnabled(),
		loc.GetEntranceCharge()
	);

	DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
	if (dbServer)
		dbServer->send(msg, true);

}

// ----------------------------------------------------------------------

void AuctionMarket::DeleteCharacter(const DeleteCharacterMessage &message)
{
	std::map<NetworkId, Auction *>::iterator auctionsIterator = m_auctions.begin();
	std::vector<std::map<NetworkId, Auction *>::iterator> auctionsToDelete;

	while (auctionsIterator != m_auctions.end())
	{
		if ((*auctionsIterator).second->GetItem().GetOwnerId() == message.getCharacterId())
			auctionsToDelete.push_back(auctionsIterator);
		++auctionsIterator;
	}

	std::vector<std::map<NetworkId, Auction *>::iterator>::iterator iter = auctionsToDelete.begin();
	while (iter != auctionsToDelete.end())
	{
		auctionsIterator = (*iter);
		LOG("CommoditiesServer", ("Auction: Auction for item %s being deleted by the Commodities Server because character %s is deleted.", (*auctionsIterator).second->GetItem().GetItemId().getValueString().c_str(), message.getCharacterId().getValueString().c_str()));
		LOG("CustomerService", ("Auction: Auction for item %s being deleted by the Commodities Server because character %s is deleted.", (*auctionsIterator).second->GetItem().GetItemId().getValueString().c_str(), message.getCharacterId().getValueString().c_str()));
		DestroyAuction(auctionsIterator);
		++iter;
	}

}

// ----------------------------------------------------------------------

void AuctionMarket::SendItemTypeMap(GameServerConnection &gameServerConnection)
{
	GenericValueTypeMessage<std::pair<int, std::map<int, std::set<int> > > > msg("CommoditiesItemTypeMap", std::make_pair(m_itemTypeMapVersionNumber, m_itemTypeMap));
	gameServerConnection.send(msg, true);
}

// ----------------------------------------------------------------------

void AuctionMarket::AddResourceType(int resourceClassCrc, const std::string & resourceName)
{
	std::set<std::string> & resourceNameList = m_resourceTypeMap[resourceClassCrc];
	if (resourceNameList.count(resourceName) == 0)
	{
		++m_resourceTypeMapVersionNumber;
		IGNORE_RETURN(resourceNameList.insert(resourceName));

		GenericValueTypeMessage<std::pair<int, std::pair<int, std::string> > > msg("CommoditiesResourceTypeAdded", std::make_pair(m_resourceTypeMapVersionNumber, std::make_pair(resourceClassCrc, resourceName)));
		CommodityServer::getInstance().sendToAllGameServers(msg);	
	}
}

// ----------------------------------------------------------------------

void AuctionMarket::SendResourceTypeMap(GameServerConnection &gameServerConnection)
{
	GenericValueTypeMessage<std::pair<int, std::map<int, std::set<std::string> > > > msg("CommoditiesResourceTypeMap", std::make_pair(m_resourceTypeMapVersionNumber, m_resourceTypeMap));
	gameServerConnection.send(msg, true);
}

// ----------------------------------------------------------------------

void AuctionMarket::RemoveFromAuctionTimerPriorityQueue(int timer, const NetworkId & item)
{
	m_priorityQueueAuctionTimer.erase(std::make_pair(timer, item));
}

// ----------------------------------------------------------------------

void AuctionMarket::AddAuctionToCompletedAuctionsList(const Auction & auction)
{
	m_completedAuctions.push_back(auction.GetItem().GetItemId());
}

// ======================================================================
//
// AuctionMarket Message Callback Handlers
//
// ======================================================================

void AuctionMarket::OnAddAuction(
	int trackId,
	int result,
	int responseId,
	const NetworkId &  itemId,
	const NetworkId &  ownerId,
	const std::string & ownerName,
	const NetworkId &  vendorId,
	const std::string & location
)
{
	OnAddAuctionMessage message(responseId, result, itemId, ownerId, ownerName, vendorId, location);
	message.SetTrackId(trackId);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnAddAuctionMessage callback to GamerServer ID: %d.\n", trackId));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnAddAuctionMessage.\n", trackId));
		}


}

void AuctionMarket::OnAddBid(
	int trackId,
	int result,
	int responseId,
	const NetworkId &  ownerId,
	const NetworkId &  itemId,
	const NetworkId &  bidderId,
	const NetworkId &  previousBidderId,
	int bidAmount,
	int previousBidAmount,
	int maxProxyBid,
	const std::string & location,
	int itemNameLength,
	const Unicode::String & itemName,
	int salesTaxAmount,
	const NetworkId &  salesTaxBankId
)
{
	OnAddBidMessage message(responseId, result, itemId, ownerId, bidderId, previousBidderId, bidAmount, previousBidAmount, maxProxyBid, location, itemNameLength, itemName, salesTaxAmount, salesTaxBankId);
	message.SetTrackId(trackId);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnAddBidMessage callback to GamerServer ID: %d.\n", trackId));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnAddBidMessage.\n", trackId));
		}

}

void AuctionMarket::OnCancelAuction(
	int trackId,
	int result,
	int responseId,
	const NetworkId &  itemId,
	const NetworkId &  playerId,
	const NetworkId &  highBidderId,
	int highBid,
	const std::string & location
)
{
	OnCancelAuctionMessage message(responseId, result, itemId, playerId, highBidderId, highBid, location);
	message.SetTrackId(trackId);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnCancelAuctionMessage callback to GamerServer ID: %d.\n", trackId));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnCancelAuctionMessage.\n", trackId));
		}

}

void AuctionMarket::OnAcceptHighBid(
	int trackId,
	int result,
	int responseId,
	const NetworkId &  itemId,
	const NetworkId &  playerId
)
{
	OnAcceptHighBidMessage message(responseId, result, itemId, playerId);
	message.SetTrackId(trackId);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnAcceptHighBidMessage callback to GamerServer ID: %d.\n", trackId));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnAcceptHighBidMessage.\n", trackId));
		}


}

void AuctionMarket::OnQueryAuctionHeaders(
	int trackId,
	int result,
	int responseId,
	const NetworkId &  playerId,
	int queryType,
	std::vector<AuctionDataHeader> & auctions,
	unsigned int queryOffset,
	bool hasMorePages
)
{
	OnQueryAuctionHeadersMessage message(responseId, result, playerId, queryType, auctions, queryOffset, hasMorePages);
	message.SetTrackId(trackId);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnQueryAuctionHeadersMessage callback to GamerServer ID: %d.\n\n", trackId));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnQueryAuctionHeadersMessage] : PlayerId : %s.\n", message.GetPlayerId().getValueString().c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnQueryAuctionHeadersMessage] : QueryType : %d.\n", message.GetQueryType()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnQueryAuctionHeadersMessage] : NumAuctions : %d.\n", message.GetNumAuctions()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnQueryAuctionHeadersMessage] : GetResultCode : %d.\n", message.GetResultCode()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnQueryAuctionHeadersMessage] : QueryOffset : %u.\n", message.GetQueryOffset()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnQueryAuctionHeadersMessage] : HasMorePages : %d.\n", message.HasMorePages()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnQueryAuctionHeadersMessage] : ResponseId : %d.\n", message.GetResponseId()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnQueryAuctionHeadersMessage] : TrackId : %d.\n", message.GetTrackId()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnQueryAuctionHeadersMessage] : Auctions : %d.\n", message.GetAuctionData().size()));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnQueryAuctionHeadersMessage.\n", trackId));
		}

}

void AuctionMarket::OnGetItemDetails(
	int trackId,
	int result,
	int responseId,
	const NetworkId &  itemId,
	const NetworkId &  playerId,
	int userDescriptionLength,
	const Unicode::String & userDescription,
	int oobDataLength,
	const Unicode::String & oobData,
	std::vector<std::pair<std::string, Unicode::String> > const & attributes
)
{
	OnGetItemDetailsMessage message(responseId, result, itemId, playerId, userDescriptionLength, userDescription, oobDataLength, oobData, attributes);
	message.SetTrackId(trackId);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnGetItemDetailsMessage callback to GamerServer ID: %d.\n", trackId));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnGetItemDetailsMessage] : Result : %d.\n", message.GetResultCode()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnGetItemDetailsMessage] : ItemId : %s.\n", message.GetItemId().getValueString().c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnGetItemDetailsMessage] : PlayerId : %s.\n", message.GetPlayerId().getValueString().c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnGetItemDetailsMessage] : UserDescriptionLength : %d.\n", message.GetUserDescriptionLength()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnGetItemDetailsMessage] : UserDescription : %s.\n", Unicode::wideToNarrow(message.GetUserDescription()).c_str()));
//			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnGetItemDetailsMessage] : OobLength : %d.\n", message.GetOobLength()));
//			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnGetItemDetailsMessage] : OobActualLength : %d.\n", message.GetOobData().size()));
//			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnGetItemDetailsMessage] : OobData 	: "));
//			if (m_showAllDebugInfo)
//			{
//				Unicode::String temp = message.GetOobData();
//				Unicode::String::iterator i = temp.begin();
//				while (i != temp.end())
//				{	printf(" %X", (*i));
//					++i;
//				}
//				printf("\n");
//			}
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnGetItemDetailsMessage] : ResponseId : %d.\n", message.GetResponseId()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnGetItemDetailsMessage] : TrackId 	: %d.\n", message.GetTrackId()));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnGetItemDetailsMessage.\n", trackId));
		}


}

void AuctionMarket::OnAuctionExpired(
	const NetworkId &  ownerId,
	bool sold, const int flags,
	const NetworkId &  buyerId,
	int highBidAmount,
	const NetworkId &  itemId,
	int highBidMaxProxy,
	const std::string & location,
	bool immediate,
	int itemNameLength,
	const Unicode::String & itemName,
	const NetworkId &  itemOwnerId,
	int track_id,
	bool sendSellerMail
)
{

	OnAuctionExpiredMessage message(ownerId, sold, buyerId, highBidAmount, itemId, highBidMaxProxy, location, immediate, itemNameLength, itemName, sendSellerMail);
	message.SetTrackId(track_id);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(track_id);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnAuctionExpiredMessage callback to GamerServer ID: %d.\n\n", track_id));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnAuctionExpiredMessage] : ItemId : %s.\n", message.GetItemId().getValueString().c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnAuctionExpiredMessage] : OwnerId : %s.\n", message.GetOwnerId().getValueString().c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnAuctionExpiredMessage] : BuyerId : %s.\n", message.GetBuyerId().getValueString().c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnAuctionExpiredMessage] : Bid : %d.\n", message.GetBid()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnAuctionExpiredMessage] : MaxProxyBid : %d.\n", message.GetMaxProxyBid()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnAuctionExpiredMessage] : Location : %s.\n", message.GetLocation().c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnAuctionExpiredMessage] : IsSold : %d.\n", message.IsSold()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnAuctionExpiredMessage] : IsImmediate : %d.\n", message.IsImmediate()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnAuctionExpiredMessage] : ItemName : %s.\n", Unicode::wideToNarrow(message.GetItemName()).c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnAuctionExpiredMessage] : ResponseId : %d.\n", message.GetResponseId()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnAuctionExpiredMessage] : TrackId : %d.\n", message.GetTrackId()));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnAuctionExpiredMessage.\n", track_id));
		}

	if (sold)
	{
		AuctionLocation &auctionLocation = GetLocation(location);
		if (!auctionLocation.IsOwner(ownerId))
		{
			ModifyAuctionCount(ownerId, -1);
		}
		if (!auctionLocation.IsOwner(buyerId))
		{
			ModifyAuctionCount(buyerId, 1);
		}
		const_cast<NetworkId &>(ownerId) = buyerId;
	}

	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Creating new UpdateAuctionMessage.\n"));
	CMUpdateAuctionMessage msg(
		itemId,
		itemOwnerId,
		(flags & (~3))
	);
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Id : %s.\n", msg.GetItemId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Item Owner : %s.\n", msg.GetOwnerId().getValueString().c_str()));
	DEBUG_REPORT_LOG(m_showAllDebugInfo, ("Auction Flags : %d.\n", msg.GetFlags()));

	DatabaseServerConnection* dbServer = CommodityServer::getInstance().getDatabaseServer();
	if (dbServer)
		dbServer->send(msg, true);
}

void AuctionMarket::OnItemExpired(
	const NetworkId &  ownerId,
	const NetworkId &  itemId,
	int itemNameLength,
	const Unicode::String & itemName,
	const std::string & locationName,
	const NetworkId & locationId
)
{
	OnItemExpiredMessage message(ownerId, itemId, itemNameLength, itemName, locationName, locationId);
	message.SetTrackId(-1);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(-1);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnItemExpiredMessage callback to GamerServer ID: %d.\n\n", -1));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnItemExpiredMessage] : ItemId : %s.\n", message.GetItemId().getValueString().c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnItemExpiredMessage] : OwnerId : %s.\n", message.GetOwnerId().getValueString().c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnItemExpiredMessage] : ItemName : %s.\n", Unicode::wideToNarrow(message.GetItemName()).c_str()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnItemExpiredMessage] : ResponseId : %d.\n", message.GetResponseId()));
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server OnItemExpiredMessage] : TrackId : %d.\n", message.GetTrackId()));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnItemExpiredMessage.\n", -1));
		}


}

void AuctionMarket::OnGetItem(
	int trackId,
	int result,
	int responseId,
	const NetworkId &  itemId,
	const NetworkId &  playerId,
	const std::string & location
)
{
	OnGetItemMessage message(responseId, result, itemId, playerId, location);
	message.SetTrackId(trackId);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnGetItemMessage callback to GamerServer ID: %d.\n", trackId));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnGetItemMessage.\n", trackId));
		}


}

void AuctionMarket::OnCreateVendorMarket(
	int trackId,
	int result,
	int responseId,
	const NetworkId &  ownerId,
	const std::string & location
)
{
	OnCreateVendorMarketMessage message(responseId, result, ownerId, location);
	message.SetTrackId(trackId);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnCreateVendorMarketMessage callback to GamerServer ID: %d.\n", trackId));
		gameServerConn->send(message, true);
	}
	else
	{
		WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnCreateVendorMarketMessage.\n", trackId));
	}


}

void AuctionMarket::OnVendorRefuseItem(
	int trackId,
	int result,
	int responseId,
	const NetworkId &  itemId,
	const NetworkId &  playerId,
	const NetworkId &  creatorId
)
{
	OnVendorRefuseItemMessage message(responseId, result, itemId, playerId, creatorId);
	message.SetTrackId(trackId);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnVendorRefuseItemMessage callback to GamerServer ID: %d.\n", trackId));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnVendorRefuseItemMessage.\n", trackId));
		}

}

void AuctionMarket::OnGetVendorOwner(
	int trackId,
	int result,
	int responseId,
	const NetworkId &  ownerId,
	const NetworkId &  requesterId,
	const std::string & location
)
{
	OnGetVendorOwnerMessage message(responseId, result, ownerId, requesterId, location);
	message.SetTrackId(trackId);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnGetVendorOwnerMessage callback to GamerServer ID: %d.\n", trackId));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnGetVendorOwnerMessage.\n", trackId));
		}
}

void AuctionMarket::OnPermanentAuctionPurchased(
	int trackId,
	const NetworkId &  ownerId,
	const NetworkId &  buyerId,
	int price,
	const NetworkId &  itemId,
	const std::string & location,
	int itemNameLength,
	const Unicode::String & itemName,
	std::vector<std::pair<std::string, Unicode::String> > const & attributes
	)
{
	OnPermanentAuctionPurchasedMessage message(ownerId, buyerId, price, itemId, location, itemNameLength, itemName, attributes);
	message.SetTrackId(trackId);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnPermanentAuctionPurchasedMessage callback to GamerServer ID: %d.\n", trackId));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnPermanentAuctionPurchasedMessage.\n", trackId));
		}

}

void AuctionMarket::OnGetVendorValue(
	int trackId,
	int responseId,
	const std::string & location,
	int value)
{
	OnGetVendorValueMessage message(responseId, location, value);
	message.SetTrackId(trackId);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnGetVendorValueMessage callback to GamerServer ID: %d.\n", trackId));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnGetVendorValueMessage.\n", trackId));
		}

}

void AuctionMarket::OnCleanupInvalidItemRetrieval(
	int trackId,
	int responseId,
	const NetworkId &  itemId,
	const NetworkId &  playerId,
	const NetworkId &  creatorId,
	int reimburseAmt)
{
	OnCleanupInvalidItemRetrievalMessage message(responseId, itemId, playerId, creatorId, reimburseAmt);
	message.SetTrackId(trackId);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
		{
			DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnCleanupInvalidItemRetrievalMessage callback to GamerServer ID: %d.\n", trackId));
			gameServerConn->send(message, true);
		}
	else
		{
			WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnCleanupInvalidItemRetrievalMessage.\n", trackId));
		}

}

void    AuctionMarket::OnQueryVendorItemCount (
	const int responseId,
	const int trackId,
	const NetworkId &vendorId,
	const int itemCount,
	const bool searchEnabled)
{
	OnQueryVendorItemCountReplyMessage message(responseId, vendorId, itemCount, searchEnabled);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnQueryVendorItemCountMessage callback to GamerServer ID: %d.\n", trackId));
		gameServerConn->send(message, true);
	}
	else
	{
		WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnQueryVendorItemCountMessage.\n", trackId));
	}
}

void AuctionMarket::OnGetPlayerVendorCount (
	const int responseId,
	const int trackId,
	const NetworkId &playerId,
	const int vendorCount,
	std::vector<NetworkId> vendorList)
{
	OnGetPlayerVendorCountMessage message(responseId, playerId, vendorCount, vendorList);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending OnGetPlayerVendorCountMessage callback to GamerServer ID: %d.\n", trackId));
		gameServerConn->send(message, true);
	}
	else
	{
		WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send OnGetPlayerVendorCountMessage.\n", trackId));
	}
}

void    AuctionMarket::OnVendorStatusChange (
	int trackId,
	const NetworkId &vendorId,
	int newStatus)
{
	if (newStatus == ARC_VendorRemoved)
	{
		RemovePlayerVendor(GetLocation(vendorId).GetOwnerId(), vendorId);
	}
	VendorStatusChangeMessage message(vendorId, newStatus);
	GameServerConnection*  gameServerConn;
	gameServerConn = CommodityServer::getInstance().getGameServer(trackId);
	if (gameServerConn)
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Sending VendorStatusChangeMessage callback to GamerServer ID: %d, vendorId = %s, newStatus = %d.\n", trackId, vendorId.getValueString().c_str(), newStatus));
		gameServerConn->send(message, true);
	}
	else
	{
		WARNING(true, ("[Commodities Server] : No Gameserver connection at ID: %d to send VendorStatusChangeMessage.\n", trackId));
	}
}

void static decodeOOB(const std::string & UTF8String, Unicode::String & UniString)
{
	std::string::const_iterator c = UTF8String.begin();
	std::string tempstring;
	unsigned short unicharvalue;
	char * pEnd;

	while (c != UTF8String.end())
	{
		tempstring.push_back(*c);
		if (tempstring.length() == 4)
		{
			unicharvalue = static_cast<unsigned short>(strtoul(tempstring.c_str(), &pEnd, 16));
			if (pEnd)
			{
				// WARNING(true, ("[Commodities Server ] : Conversion of string hex values to unsigned short failed in: TaskGetAuctionList::decodeOOB.\n"));
			}
			UniString.push_back(unicharvalue);
			tempstring.clear();
		}
		c++;
	}
	if (tempstring.length() != 0)
	{
		WARNING(true, ("[Commodities Server ] : Unconverted OOB data in - TaskGetAuctionList::decodeOOB.\n"));
	}
}

void	AuctionMarket::onReceiveAuctionLocations     (const NetworkId &locationId, const std::string &locationName, const NetworkId &ownerId, const int salesTax, const NetworkId &salesTaxBankId, const int emptyDate, const int lastAccessDate, const int inactiveDate, const int status, const bool searchEnabled, const int entranceCharge)
{

    	AuctionLocation *auctionLocation = new AuctionLocation(locationId, locationName, ownerId, salesTax, salesTaxBankId, emptyDate, lastAccessDate, inactiveDate, status, searchEnabled, entranceCharge);

	m_locationIdMap.insert(std::make_pair(auctionLocation->GetLocationId(), auctionLocation));
	if ((ownerId != zeroNetworkId) && (auctionLocation->GetStatus() < REMOVED))
	{
		AddPlayerVendor(ownerId, locationId, auctionLocation);
	}
}

void AuctionMarket::onReceiveMarketAuctions (const NetworkId &itemId, const NetworkId &ownerId, const NetworkId &creatorId, const NetworkId &locationId, const int minBid, const int buyNowPrice, const int auctionTimer, const std::string &oob, const Unicode::String &userDescription, const int category, const int itemTemplateId, const Unicode::String &itemName, const int itemTimer, const int active, const int itemSize)
{
	AuctionLocation &location = GetLocation(locationId);

	Unicode::String oobData;
	decodeOOB(oob, oobData);

	std::vector<std::pair<std::string, Unicode::String> > attributes;

	Auction *auction = new Auction(
				creatorId,
				minBid,
				auctionTimer,
				itemId,
				itemName.size(),
				itemName,
				category,
				itemTemplateId,
				itemTimer,
				itemSize,
				location,
				active,
				buyNowPrice,
				userDescription.size(),
				userDescription,
				oobData.size(),
				oobData,
				attributes,
				(active & 1),
				(active & 2));

	auction->GetItem().SetOwnerId(ownerId);

	if ((auction->GetItem().GetItemTemplateId() != 0) && (auction->GetItem().GetCategory() != 0))
	{
		std::set<int> & itemTypeList = m_itemTypeMap[auction->GetItem().GetCategory()];
		if (itemTypeList.count(auction->GetItem().GetItemTemplateId()) == 0)
		{
			++m_itemTypeMapVersionNumber;
			IGNORE_RETURN(itemTypeList.insert(auction->GetItem().GetItemTemplateId()));
		}
	}

	m_auctions.insert(std::make_pair(auction->GetItem().GetItemId(), auction));

	std::string const & gameObjectType = GameObjectTypes::getCanonicalName(auction->GetItem().GetCategory());
	std::map<std::string, int>::iterator iterAuctionsCountByGameObjectType = m_auctionsCountByGameObjectType.find(gameObjectType);
	if (iterAuctionsCountByGameObjectType != m_auctionsCountByGameObjectType.end())
	{
		++iterAuctionsCountByGameObjectType->second;
		m_auctionsCountByGameObjectTypeChanged.insert(gameObjectType);

		if (GameObjectTypes::isSubType(auction->GetItem().GetCategory()))
		{
			std::string const & gameParentObjectType = GameObjectTypes::getCanonicalName(GameObjectTypes::getMaskedType(auction->GetItem().GetCategory()));
			std::map<std::string, int>::iterator iterAuctionsCountByParentGameObjectType = m_auctionsCountByGameObjectType.find(gameParentObjectType);
			if (iterAuctionsCountByParentGameObjectType != m_auctionsCountByGameObjectType.end())
			{
				++iterAuctionsCountByParentGameObjectType->second;
				m_auctionsCountByGameObjectTypeChanged.insert(gameParentObjectType);
			}
		}
	}

	if (!location.IsOwner(auction->GetItem().GetOwnerId()))
	{
		ModifyAuctionCount(auction->GetItem().GetOwnerId(), 1);
	}

	location.AddAuction(auction);
	AddAuctionToPriorityQueue(*auction);

	if (auction->IsActive() && auction->IsSold())
		AddAuctionToCompletedAuctionsList(*auction);
}

void AuctionMarket::onReceiveMarketAuctionAttributes(const NetworkId &itemId, const std::string &attributeName, const Unicode::String &attributeValue)
{
	std::map<NetworkId, Auction *>::iterator iter = m_auctions.find(itemId);

	if (iter != m_auctions.end())
	{
		Auction *auction = (*iter).second;

		auction->AddAttributes(attributeName, attributeValue);
	}
	else
	{
		WARNING(true,("[Commodities Server] : There are orphaned auction attributes records for item %s.", itemId.getValueString().c_str()));
	}
}

void	AuctionMarket::onReceiveMarketAuctionBids    (const NetworkId &itemId, const NetworkId &bidderId, const int bid, const int maxProxyBid)
{
	std::map<NetworkId, Auction *>::iterator iter = m_auctions.find(itemId);

	if (iter != m_auctions.end())
	{
		Auction *auction = (*iter).second;

		auction->AddLoadedBid(bidderId, bid, maxProxyBid);
	}
	else
	{
		WARNING(true,("[Commodities Server] : There are orphaned bid records for item %s.", itemId.getValueString().c_str()));
	}
}

void	AuctionMarket::printAuctionTables()
{

	DEBUG_REPORT_LOG(true, ("  Print AUCTION_LOCATIONS data:\n"));
	std::map<NetworkId, AuctionLocation *>::iterator auctionLocationIterator =
		m_locationIdMap.begin();
	while (auctionLocationIterator != m_locationIdMap.end())
	{
		DEBUG_REPORT_LOG(true, ("    LocId = %s, LocName = %s\n      OwnerId = %s, SalesTax = %d, SalesTaxBankId = %s, EntranceCharge = %d\n", auctionLocationIterator->second->GetLocationId().getValueString().c_str(), auctionLocationIterator->second->GetLocationString().c_str(), auctionLocationIterator->second->GetOwnerId().getValueString().c_str(), auctionLocationIterator->second->GetSalesTax(), auctionLocationIterator->second->GetSalesTaxBankId().getValueString().c_str(), auctionLocationIterator->second->GetEntranceCharge()));
		++auctionLocationIterator;
	}

	DEBUG_REPORT_LOG(true, ("  Print MARKET_AUCTIONS data:\n"));
	std::map<NetworkId, Auction *>::iterator marketAuctionsIterator = m_auctions.begin();
	while (marketAuctionsIterator != m_auctions.end())
	{
		Auction *auction = marketAuctionsIterator->second;
		UNREF(auction);
		DEBUG_REPORT_LOG(true, ("    ItemId = %s, OwnerId = %s, CreatorId = %s, LocId = %s\n      MinBid = %d, BuyNowPrice = %d, AuctionTimer = %d, Category = %d\n",
		auction->GetItem().GetItemId().getValueString().c_str(),
		auction->GetItem().GetOwnerId().getValueString().c_str(),
		auction->GetCreatorId().getValueString().c_str(),
		auction->GetLocation().GetLocationId().getValueString().c_str(),
		auction->GetMinBid(),
		auction->GetBuyNowPrice(),
		auction->GetAuctionTimer(),
		auction->GetFlags()));
		++marketAuctionsIterator;
	}
}

void AuctionMarket::VerifyExcludedGotTypes(std::map<int, std::string> const & excludedGotTypes)
{
	std::set<int> processedGotTypes;

	int itemCategory, itemGeneralCategory;
	std::map<int, std::string>::const_iterator iterFind;
	std::map<NetworkId, Auction *>::const_iterator marketAuctionsIterator = m_auctions.begin();
	while (marketAuctionsIterator != m_auctions.end())
	{
		AuctionItem const & auctionItem = marketAuctionsIterator->second->GetItem();
		itemCategory = auctionItem.GetCategory();
		if (processedGotTypes.count(itemCategory) == 0)
		{
			IGNORE_RETURN(processedGotTypes.insert(itemCategory));

			iterFind = excludedGotTypes.find(itemCategory);
			if (iterFind != excludedGotTypes.end())
			{
				LOG("CustomerService", ("CommoditiesMissingCategory:Item %s (%s) is in excluded category %d (%s)",
					auctionItem.GetItemId().getValueString().c_str(),
					Unicode::wideToNarrow(auctionItem.GetName()).c_str(),
					iterFind->first,
					iterFind->second.c_str()));
			}

			itemGeneralCategory = itemCategory & 0xffffff00;
			if (itemCategory != itemGeneralCategory)
			{
				// general category
				iterFind = excludedGotTypes.find(itemGeneralCategory);
				if (iterFind != excludedGotTypes.end())
				{
					LOG("CustomerService", ("CommoditiesMissingCategory:Item %s (%s) is in excluded general category %d (%s)",
						auctionItem.GetItemId().getValueString().c_str(),
						Unicode::wideToNarrow(auctionItem.GetName()).c_str(),
						iterFind->first,
						iterFind->second.c_str()));
				}
			}
		}

		++marketAuctionsIterator;
	}
}

void AuctionMarket::VerifyExcludedResourceClasses(std::set<std::string> const & excludedResourceClasses)
{
	int excludedClassCrc;
	for (std::set<std::string>::const_iterator iterExcluded = excludedResourceClasses.begin(); iterExcluded != excludedResourceClasses.end(); ++iterExcluded)
	{
		excludedClassCrc = static_cast<int>(Crc::calculate(iterExcluded->c_str()));

		for (std::map<NetworkId, AuctionLocation *>::iterator locationIter = m_locationIdMap.begin(); locationIter != m_locationIdMap.end(); ++locationIter)		
		{
			std::map<NetworkId, Auction *> const & auctionsResourceContainer = locationIter->second->GetAuctionsResourceContainer();
			for (std::map<NetworkId, Auction *>::const_iterator auctionIter = auctionsResourceContainer.begin(); auctionIter != auctionsResourceContainer.end(); ++auctionIter)
			{
				AuctionItem const & auctionItem = auctionIter->second->GetItem();
				if (IsResourceClassDerivedFrom(auctionItem.GetResourceContainerClassCrc(), excludedClassCrc))
				{
					LOG("CustomerService", ("CommoditiesMissingResourceClass:Item %s (%s) is in excluded resource class (%s)",
						auctionItem.GetItemId().getValueString().c_str(),
						Unicode::wideToNarrow(auctionItem.GetName()).c_str(),
						iterExcluded->c_str()));
				}
			}
		}
	}
}

bool AuctionMarket::IsResourceClassDerivedFrom(int resourceClassCrc, int parentResourceClassCrc)
{
	if (resourceClassCrc == parentResourceClassCrc)
		return true;

	std::map<int, std::set<int> >::const_iterator iter = m_resourceTreeHierarchy.find(resourceClassCrc);
	if (iter == m_resourceTreeHierarchy.end())
		return false;

	return (iter->second.count(parentResourceClassCrc) > 0);
}

void AuctionMarket::SetResourceTreeHierarchy(std::map<int, std::set<int> > const & resourceTreeHierarchy)
{
	m_resourceTreeHierarchy = resourceTreeHierarchy;
}

void AuctionMarket::getItemAttributeData(int requestingGameServerId, const NetworkId & requester, const std::string & outputFileName, int gameObjectType, bool exactGameObjectTypeMatch, bool ignoreSearchableAttribute, int throttle) const
{
	if (getItemAttributeDataRequest)
	{
		if (getItemAttributeDataRequest->requester != requester)
		{
			GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(requestingGameServerId);
			if (gameServerConn)
			{
				char buffer[2048];
				snprintf(buffer, sizeof(buffer)-1, "There is already a pending %s request by (%s).", getItemAttributeDataRequest->action.c_str(), getItemAttributeDataRequest->requester.getValueString().c_str());
				buffer[sizeof(buffer)-1] = '\0';

				GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::string>, std::string> > const msg("GetItemAttributeDataRsp", std::make_pair(std::make_pair(requester, std::string("(~!@#$%^&*)system_mesage(*&^%$#@!~)")), std::string(buffer)));
				gameServerConn->send(msg, true);
			}
		}
		else if (getItemAttributeDataRequest->action != "GetItemAttributeData")
		{
			GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(requestingGameServerId);
			if (gameServerConn)
			{
				char buffer[2048];
				snprintf(buffer, sizeof(buffer)-1, "There is already a pending %s request by (%s).", getItemAttributeDataRequest->action.c_str(), getItemAttributeDataRequest->requester.getValueString().c_str());
				buffer[sizeof(buffer)-1] = '\0';

				GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::string>, std::string> > const msg("GetItemAttributeDataRsp", std::make_pair(std::make_pair(requester, std::string("(~!@#$%^&*)system_mesage(*&^%$#@!~)")), std::string(buffer)));
				gameServerConn->send(msg, true);
			}
		}
		else
		{
			getItemAttributeDataRequest->requestingGameServerId = requestingGameServerId;
			getItemAttributeDataRequest->throttleNumberItemsPerFrame = throttle;

			getItemAttributeDataRequest->outputFileName = outputFileName;

			GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(requestingGameServerId);
			if (gameServerConn)
			{
				char buffer[2048];
				snprintf(buffer, sizeof(buffer)-1, "%s request modified.  outputFileName=(%s), gameObjectType=(%d, %s), exactGameObjectTypeMatch=(%s), ignoreSearchableAttribute=(%s), throttle=(%d), numberItemsProcessed=(%d), totalNumberOfItems=(%d).",
					getItemAttributeDataRequest->action.c_str(),
					getItemAttributeDataRequest->outputFileName.c_str(),
					getItemAttributeDataRequest->gameObjectType,
					GameObjectTypes::getCanonicalName(getItemAttributeDataRequest->gameObjectType).c_str(),
					(getItemAttributeDataRequest->exactGameObjectTypeMatch ? "yes" : "no" ),
					(getItemAttributeDataRequest->ignoreSearchableAttribute ? "yes" : "no"),
					getItemAttributeDataRequest->throttleNumberItemsPerFrame,
					getItemAttributeDataRequest->numberItemsProcessed,
					m_auctions.size());

				buffer[sizeof(buffer)-1] = '\0';

				GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::string>, std::string> > const msg("GetItemAttributeDataRsp", std::make_pair(std::make_pair(requester, std::string("(~!@#$%^&*)system_mesage(*&^%$#@!~)")), std::string(buffer)));
				gameServerConn->send(msg, true);
			}
		}
	}
	else
	{
		getItemAttributeDataRequest = new GetItemAttributeDataRequest();
		getItemAttributeDataRequest->action = "GetItemAttributeData";
		getItemAttributeDataRequest->requestingGameServerId = requestingGameServerId;
		getItemAttributeDataRequest->requester = requester;
		getItemAttributeDataRequest->gameObjectType = gameObjectType;
		getItemAttributeDataRequest->exactGameObjectTypeMatch = exactGameObjectTypeMatch;
		getItemAttributeDataRequest->ignoreSearchableAttribute = ignoreSearchableAttribute;
		getItemAttributeDataRequest->throttleNumberItemsPerFrame = throttle;
		getItemAttributeDataRequest->numberItemsProcessed = 0;
		getItemAttributeDataRequest->lastItemProcessed = NetworkId::cms_invalid;

		getItemAttributeDataRequest->outputFileName = outputFileName;
		getItemAttributeDataRequest->allItemAttributes.clear();

		GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(requestingGameServerId);
		if (gameServerConn)
		{
			char buffer[2048];
			snprintf(buffer, sizeof(buffer)-1, "%s request accepted.  outputFileName=(%s), gameObjectType=(%d, %s), exactGameObjectTypeMatch=(%s), ignoreSearchableAttribute=(%s), throttle=(%d), totalNumberOfItems=(%d).",
				getItemAttributeDataRequest->action.c_str(),
				getItemAttributeDataRequest->outputFileName.c_str(),
				getItemAttributeDataRequest->gameObjectType,
				GameObjectTypes::getCanonicalName(getItemAttributeDataRequest->gameObjectType).c_str(),
				(getItemAttributeDataRequest->exactGameObjectTypeMatch ? "yes" : "no" ),
				(getItemAttributeDataRequest->ignoreSearchableAttribute ? "yes" : "no"),
				getItemAttributeDataRequest->throttleNumberItemsPerFrame,
				m_auctions.size());

			buffer[sizeof(buffer)-1] = '\0';

			GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::string>, std::string> > const msg("GetItemAttributeDataRsp", std::make_pair(std::make_pair(requester, std::string("(~!@#$%^&*)system_mesage(*&^%$#@!~)")), std::string(buffer)));
			gameServerConn->send(msg, true);
		}
	}
}

void AuctionMarket::getItemAttributeDataValues(int requestingGameServerId, const NetworkId & requester, int gameObjectType, bool exactGameObjectTypeMatch, const std::string & attributeName, int throttle) const
{
	if (getItemAttributeDataRequest)
	{
		if (getItemAttributeDataRequest->requester != requester)
		{
			GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(requestingGameServerId);
			if (gameServerConn)
			{
				char buffer[2048];
				snprintf(buffer, sizeof(buffer)-1, "There is already a pending %s request by (%s).", getItemAttributeDataRequest->action.c_str(), getItemAttributeDataRequest->requester.getValueString().c_str());
				buffer[sizeof(buffer)-1] = '\0';

				GenericValueTypeMessage<std::pair<NetworkId, std::string> > const msg("DisplayStringForPlayer", std::make_pair(requester, std::string(buffer)));
				gameServerConn->send(msg, true);
			}
		}
		else if (getItemAttributeDataRequest->action != "GetItemAttributeDataValues")
		{
			GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(requestingGameServerId);
			if (gameServerConn)
			{
				char buffer[2048];
				snprintf(buffer, sizeof(buffer)-1, "There is already a pending %s request by (%s).", getItemAttributeDataRequest->action.c_str(), getItemAttributeDataRequest->requester.getValueString().c_str());
				buffer[sizeof(buffer)-1] = '\0';

				GenericValueTypeMessage<std::pair<NetworkId, std::string> > const msg("DisplayStringForPlayer", std::make_pair(requester, std::string(buffer)));
				gameServerConn->send(msg, true);
			}
		}
		else
		{
			getItemAttributeDataRequest->requestingGameServerId = requestingGameServerId;
			getItemAttributeDataRequest->throttleNumberItemsPerFrame = throttle;

			GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(requestingGameServerId);
			if (gameServerConn)
			{
				char buffer[2048];
				snprintf(buffer, sizeof(buffer)-1, "%s request modified.  game object type=(%d, %s), exactGameObjectTypeMatch=(%s), attribute name=(%s), throttle=(%d), numberItemsProcessed=(%d), totalNumberOfItems=(%d).",
					getItemAttributeDataRequest->action.c_str(),
					getItemAttributeDataRequest->gameObjectType,
					GameObjectTypes::getCanonicalName(getItemAttributeDataRequest->gameObjectType).c_str(),
					(getItemAttributeDataRequest->exactGameObjectTypeMatch ? "yes" : "no" ),
					getItemAttributeDataRequest->attributeName.c_str(),
					getItemAttributeDataRequest->throttleNumberItemsPerFrame,
					getItemAttributeDataRequest->numberItemsProcessed,
					m_auctions.size());
				buffer[sizeof(buffer)-1] = '\0';

				GenericValueTypeMessage<std::pair<NetworkId, std::string> > const msg("DisplayStringForPlayer", std::make_pair(requester, std::string(buffer)));
				gameServerConn->send(msg, true);
			}
		}
	}
	else
	{
		getItemAttributeDataRequest = new GetItemAttributeDataRequest();
		getItemAttributeDataRequest->action = "GetItemAttributeDataValues";
		getItemAttributeDataRequest->requestingGameServerId = requestingGameServerId;
		getItemAttributeDataRequest->requester = requester;
		getItemAttributeDataRequest->throttleNumberItemsPerFrame = throttle;
		getItemAttributeDataRequest->numberItemsProcessed = 0;
		getItemAttributeDataRequest->lastItemProcessed = NetworkId::cms_invalid;

		getItemAttributeDataRequest->gameObjectType = gameObjectType;
		getItemAttributeDataRequest->exactGameObjectTypeMatch = exactGameObjectTypeMatch;
		getItemAttributeDataRequest->attributeName = attributeName;
		getItemAttributeDataRequest->numberItemsMatchGameObjectType = 0;
		getItemAttributeDataRequest->attributeValue.clear();

		GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(requestingGameServerId);
		if (gameServerConn)
		{
			char buffer[2048];
			snprintf(buffer, sizeof(buffer)-1, "%s request accepted.  game object type=(%d, %s), exactGameObjectTypeMatch=(%s), attribute name=(%s), throttle=(%d), totalNumberOfItems=(%d).",
				getItemAttributeDataRequest->action.c_str(),
				getItemAttributeDataRequest->gameObjectType,
				GameObjectTypes::getCanonicalName(getItemAttributeDataRequest->gameObjectType).c_str(),
				(getItemAttributeDataRequest->exactGameObjectTypeMatch ? "yes" : "no" ),
				getItemAttributeDataRequest->attributeName.c_str(),
				getItemAttributeDataRequest->throttleNumberItemsPerFrame,
				m_auctions.size());
			buffer[sizeof(buffer)-1] = '\0';

			GenericValueTypeMessage<std::pair<NetworkId, std::string> > const msg("DisplayStringForPlayer", std::make_pair(requester, std::string(buffer)));
			gameServerConn->send(msg, true);
		}
	}
}

void AuctionMarket::getItemAttribute(int requestingGameServerId, const NetworkId & requester, const NetworkId & item) const
{
	std::map<NetworkId, Auction *>::const_iterator iterAuction = m_auctions.find(item);
	if (iterAuction != m_auctions.end())
	{
		GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(requestingGameServerId);
		if (gameServerConn)
		{
			std::string output;
			iterAuction->second->GetAttributes(output);

			GenericValueTypeMessage<std::pair<NetworkId, std::string> > const msg("DisplayStringForPlayer", std::make_pair(requester, output));
			gameServerConn->send(msg, true);
		}
	}
	else
	{
		GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(requestingGameServerId);
		if (gameServerConn)
		{
			char buffer[2048];
			snprintf(buffer, sizeof(buffer)-1, "item (%s) is not in the commodities system", item.getValueString().c_str());
			buffer[sizeof(buffer)-1] = '\0';

			GenericValueTypeMessage<std::pair<NetworkId, std::string> > const msg("DisplayStringForPlayer", std::make_pair(requester, std::string(buffer)));
			gameServerConn->send(msg, true);
		}
	}
}

void AuctionMarket::getVendorInfoForPlayer(int requestingGameServerId, const NetworkId & requester, const NetworkId & player, bool godMode)
{
	// if vendor is specified for player, replace it with the vendor's owner
	NetworkId vendorOwner = player;
	if (vendorOwner != requester)
	{
		std::map<NetworkId, AuctionLocation *>::iterator const iterFind = m_locationIdMap.find(vendorOwner);
		if (iterFind != m_locationIdMap.end())
		{
			vendorOwner = iterFind->second->GetOwnerId();
		}
	}

	std::map<std::string, std::vector<std::string> > vendorInfo;
	int totalOfferCount = 0;
	int totalStockRoomCount = 0;
	std::map<NetworkId, std::map<NetworkId, AuctionLocation *> >::const_iterator const iterPlayer = m_playerVendorListMap.find(vendorOwner);
	if (iterPlayer != m_playerVendorListMap.end())
	{
		char buffer[64];
		std::string planet;
		std::string region;
		std::string name;
		NetworkId id;
		int x, z;
		int offerCount, stockRoomCount;

		unsigned int pos = 0;

		for (std::map<NetworkId, AuctionLocation *>::const_iterator iterAuctionLocation = iterPlayer->second.begin(); iterAuctionLocation != iterPlayer->second.end(); ++iterAuctionLocation)
		{
			pos = 0;
			nextString(iterAuctionLocation->second->GetLocationString(), pos, planet, '.');
			nextString(iterAuctionLocation->second->GetLocationString(), pos, region, '.');
			nextString(iterAuctionLocation->second->GetLocationString(), pos, name, '.');
			nextOid(iterAuctionLocation->second->GetLocationString(), pos, id, '#');
			nextInt(iterAuctionLocation->second->GetLocationString(), pos, x, ',');
			nextInt(iterAuctionLocation->second->GetLocationString(), pos, z, ',');

			if ((name.find("Vendor: ") == 0) && (name.size() > 8))
			{
				name = name.substr(8);
			}

			vendorInfo["name"].push_back(name);

			if (iterAuctionLocation->second->IsPacked())
			{
				planet = "Datapad";
			}
			else
			{
				planet = Unicode::wideToNarrow(StringId("planet_n", planet).localize());

				snprintf(buffer, sizeof(buffer)-1, " (%d, %d)", x, z);
				buffer[sizeof(buffer)-1] = '\0';

				planet += buffer;
			}

			if (godMode)
			{
				planet += " (";
				planet += iterAuctionLocation->second->GetLocationString();
				planet += ")";
			}

			vendorInfo["location"].push_back(planet);

			snprintf(buffer, sizeof(buffer)-1, "%d", (iterAuctionLocation->second->GetSalesTax() / 100));
			buffer[sizeof(buffer)-1] = '\0';
			vendorInfo["tax"].push_back(buffer);

			vendorInfo["taxCity"].push_back(iterAuctionLocation->second->GetSalesTaxBankId().getValueString());

			if (iterAuctionLocation->second->GetEmptyDate() > 0)
			{
				vendorInfo["emptyDate"].push_back(CalendarTime::convertEpochToTimeStringLocal_YYYYMMDDHHMMSS(static_cast<time_t>(iterAuctionLocation->second->GetEmptyDate())));
			}
			else
			{
				vendorInfo["emptyDate"].push_back("");
			}

			if (iterAuctionLocation->second->GetLastAccessDate() > 0)
			{
				vendorInfo["lastAccessDate"].push_back(CalendarTime::convertEpochToTimeStringLocal_YYYYMMDDHHMMSS(static_cast<time_t>(iterAuctionLocation->second->GetLastAccessDate())));
			}
			else
			{
				vendorInfo["lastAccessDate"].push_back("");
			}

			if (iterAuctionLocation->second->GetInactiveDate() > 0)
			{
				vendorInfo["inactiveDate"].push_back(CalendarTime::convertEpochToTimeStringLocal_YYYYMMDDHHMMSS(static_cast<time_t>(iterAuctionLocation->second->GetInactiveDate())));
			}
			else
			{
				vendorInfo["inactiveDate"].push_back("");
			}

			vendorInfo["status"].push_back(vendorStatus[iterAuctionLocation->second->GetStatus()]);

			if (iterAuctionLocation->second->IsPacked())
			{
				vendorInfo["searchable"].push_back("no");
			}
			else
			{
				vendorInfo["searchable"].push_back(iterAuctionLocation->second->GetSearchEnabled() ? "yes" : "no");
			}

			snprintf(buffer, sizeof(buffer)-1, "%d", iterAuctionLocation->second->GetEntranceCharge());
			buffer[sizeof(buffer)-1] = '\0';
			vendorInfo["entranceCharge"].push_back(buffer);

			snprintf(buffer, sizeof(buffer)-1, "%d", iterAuctionLocation->second->GetAuctionItemCount());
			buffer[sizeof(buffer)-1] = '\0';
			vendorInfo["itemCount"].push_back(buffer);

			offerCount = 0;
			stockRoomCount = 0;
			std::map<NetworkId, Auction *> const & offers = iterAuctionLocation->second->GetVendorOffers();
			for (std::map<NetworkId, Auction *>::const_iterator iterOffer = offers.begin(); iterOffer != offers.end(); ++iterOffer)
			{
				if (iterOffer->second->GetItem().GetOwnerId() == vendorOwner)
				{
					stockRoomCount += iterOffer->second->GetItem().GetSize();
				}
				else if (iterOffer->second->IsActive())
				{
					offerCount += iterOffer->second->GetItem().GetSize();
				}
			}

			snprintf(buffer, sizeof(buffer)-1, "%d", offerCount);
			buffer[sizeof(buffer)-1] = '\0';
			vendorInfo["offerCount"].push_back(buffer);

			snprintf(buffer, sizeof(buffer)-1, "%d", stockRoomCount);
			buffer[sizeof(buffer)-1] = '\0';
			vendorInfo["stockRoomCount"].push_back(buffer);

			totalOfferCount += offerCount;
			totalStockRoomCount += stockRoomCount;
		}
	}

	GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(requestingGameServerId);
	if (gameServerConn)
	{
		char buffer[2048];

		if (requester == vendorOwner)
		{
			snprintf(buffer, sizeof(buffer)-1, "You currently have %d initialized vendors containing %d active items, %d stockroom items, and %d offered items.", GetVendorCount(vendorOwner), GetItemCount(vendorOwner), totalStockRoomCount, totalOfferCount);
			buffer[sizeof(buffer)-1] = '\0';
		}
		else
		{
			snprintf(buffer, sizeof(buffer)-1, "Player %s currently has %d initialized vendors containing %d active items, %d stockroom items, and %d offered items.", vendorOwner.getValueString().c_str(), GetVendorCount(vendorOwner), GetItemCount(vendorOwner), totalStockRoomCount, totalOfferCount);
			buffer[sizeof(buffer)-1] = '\0';
		}

		GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::string>, std::map<std::string, std::vector<std::string> > > > const msg("GetVendorInfoForPlayerRsp", std::make_pair(std::make_pair(requester, std::string(buffer)), vendorInfo));
		gameServerConn->send(msg, true);
	}
}

void AuctionMarket::getAuctionLocationPriorityQueue(int requestingGameServerId, const NetworkId & requester, int count) const
{
	std::string output;
	char buffer[2048];
	int const timeNow = static_cast<int>(::time(nullptr));
	for (std::set<std::pair<int, NetworkId> >::const_iterator iterPQ = m_priorityQueueAuctionLocation.begin(); iterPQ != m_priorityQueueAuctionLocation.end(); ++iterPQ)
	{
		if (count <= 0)
			break;

		std::map<NetworkId, AuctionLocation *>::const_iterator iterAuctionLocation = m_locationIdMap.find((*iterPQ).second);
		if (iterAuctionLocation != m_locationIdMap.end())
		{
			if ((*iterPQ).first >= timeNow)
			{
				snprintf(buffer, sizeof(buffer)-1, "next update in (%d, %s) for auction location (%s, %s)", (*iterPQ).first, CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>((*iterPQ).first - timeNow)).c_str(), (*iterPQ).second.getValueString().c_str(), iterAuctionLocation->second->GetLocationString().c_str());
				buffer[sizeof(buffer)-1] = '\0';
			}
			else
			{
				snprintf(buffer, sizeof(buffer)-1, "next update in (%d, -%s) for auction location (%s, %s)", (*iterPQ).first, CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(timeNow - (*iterPQ).first)).c_str(), (*iterPQ).second.getValueString().c_str(), iterAuctionLocation->second->GetLocationString().c_str());
				buffer[sizeof(buffer)-1] = '\0';
			}
		}
		else
		{
			if ((*iterPQ).first >= timeNow)
			{
				snprintf(buffer, sizeof(buffer)-1, "next update in (%d, %s) for auction location (%s)", (*iterPQ).first, CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>((*iterPQ).first - timeNow)).c_str(), (*iterPQ).second.getValueString().c_str());
				buffer[sizeof(buffer)-1] = '\0';
			}
			else
			{
				snprintf(buffer, sizeof(buffer)-1, "next update in (%d, -%s) for auction location (%s)", (*iterPQ).first, CalendarTime::convertSecondsToDHMS(static_cast<unsigned int>(timeNow - (*iterPQ).first)).c_str(), (*iterPQ).second.getValueString().c_str());
				buffer[sizeof(buffer)-1] = '\0';
			}
		}

		output += buffer;
		output += "\r\n";

		--count;
	}

	GameServerConnection * const gameServerConn = CommodityServer::getInstance().getGameServer(requestingGameServerId);
	if (gameServerConn)
	{
		GenericValueTypeMessage<std::pair<NetworkId, std::string> > const msg("DisplayStringForPlayer", std::make_pair(requester, output));
		gameServerConn->send(msg, true);
	}
}

void AuctionMarket::BuildAuctionsSearchableAttributeList()
{
	for (std::map<NetworkId, Auction *>::iterator i = m_auctions.begin(); i != m_auctions.end(); ++i)
		i->second->BuildSearchableAttributeList();
}

void AuctionMarket::addAuctionLocationToLocationIndex(const AuctionLocation * auctionLocation)
{
	if (!auctionLocation)
		return;

	if (!auctionLocation->IsVendorMarket())
	{
		m_allBazaar[auctionLocation->GetLocationId()] = const_cast<AuctionLocation *>(auctionLocation);
		(m_bazaarByPlanet[auctionLocation->GetLocationPlanet()])[auctionLocation->GetLocationId()] = const_cast<AuctionLocation *>(auctionLocation);
		(m_bazaarByRegion[std::make_pair(auctionLocation->GetLocationPlanet(), auctionLocation->GetLocationRegion())])[auctionLocation->GetLocationId()] = const_cast<AuctionLocation *>(auctionLocation);
	}
	else if (auctionLocation->GetSearchEnabled() && !auctionLocation->IsPacked())
	{
		m_allSearchableVendor[auctionLocation->GetLocationId()] = const_cast<AuctionLocation *>(auctionLocation);
		(m_searchableVendorByPlanet[auctionLocation->GetLocationPlanet()])[auctionLocation->GetLocationId()] = const_cast<AuctionLocation *>(auctionLocation);
		(m_searchableVendorByRegion[std::make_pair(auctionLocation->GetLocationPlanet(), auctionLocation->GetLocationRegion())])[auctionLocation->GetLocationId()] = const_cast<AuctionLocation *>(auctionLocation);
	}
}

void AuctionMarket::removeAuctionLocationFromLocationIndex(const AuctionLocation * auctionLocation)
{
	if (!auctionLocation)
		return;

	{
		m_allBazaar.erase(auctionLocation->GetLocationId());
	}

	{
		std::map<std::string, std::map<NetworkId, AuctionLocation *> >::iterator iterFind = m_bazaarByPlanet.find(auctionLocation->GetLocationPlanet());
		if (iterFind != m_bazaarByPlanet.end())
		{
			iterFind->second.erase(auctionLocation->GetLocationId());
			if (iterFind->second.empty())
			{
				m_bazaarByPlanet.erase(iterFind);
			}
		}
	}

	{
		std::map<std::pair<std::string, std::string>, std::map<NetworkId, AuctionLocation *> >::iterator iterFind = m_bazaarByRegion.find(std::make_pair(auctionLocation->GetLocationPlanet(), auctionLocation->GetLocationRegion()));
		if (iterFind != m_bazaarByRegion.end())
		{
			iterFind->second.erase(auctionLocation->GetLocationId());
			if (iterFind->second.empty())
			{
				m_bazaarByRegion.erase(iterFind);
			}
		}
	}

	{
		m_allSearchableVendor.erase(auctionLocation->GetLocationId());
	}

	{
		std::map<std::string, std::map<NetworkId, AuctionLocation *> >::iterator iterFind = m_searchableVendorByPlanet.find(auctionLocation->GetLocationPlanet());
		if (iterFind != m_searchableVendorByPlanet.end())
		{
			iterFind->second.erase(auctionLocation->GetLocationId());
			if (iterFind->second.empty())
			{
				m_searchableVendorByPlanet.erase(iterFind);
			}
		}
	}

	{
		std::map<std::pair<std::string, std::string>, std::map<NetworkId, AuctionLocation *> >::iterator iterFind = m_searchableVendorByRegion.find(std::make_pair(auctionLocation->GetLocationPlanet(), auctionLocation->GetLocationRegion()));
		if (iterFind != m_searchableVendorByRegion.end())
		{
			iterFind->second.erase(auctionLocation->GetLocationId());
			if (iterFind->second.empty())
			{
				m_searchableVendorByRegion.erase(iterFind);
			}
		}
	}
}

void AuctionMarket::sanityCheckAuctionLocationBeingDestroyed(const AuctionLocation * auctionLocation)
{
	if (!auctionLocation)
		return;

	{
		removeAuctionLocationFromList(m_locationIdMap, auctionLocation);
	}

	{
		for (std::map<NetworkId, std::map<NetworkId, AuctionLocation *> >::iterator iter = m_playerVendorListMap.begin(); iter != m_playerVendorListMap.end(); ++iter)
		{
			removeAuctionLocationFromList(iter->second, auctionLocation);
		}
	}

	{
		removeAuctionLocationFromList(m_allBazaar, auctionLocation);
	}

	{
		for (std::map<std::string, std::map<NetworkId, AuctionLocation *> >::iterator iter = m_bazaarByPlanet.begin(); iter != m_bazaarByPlanet.end(); ++iter)
		{
			removeAuctionLocationFromList(iter->second, auctionLocation);
		}
	}

	{
		for (std::map<std::pair<std::string, std::string>, std::map<NetworkId, AuctionLocation *> >::iterator iter = m_bazaarByRegion.begin(); iter != m_bazaarByRegion.end(); ++iter)
		{
			removeAuctionLocationFromList(iter->second, auctionLocation);
		}
	}

	{
		removeAuctionLocationFromList(m_allSearchableVendor, auctionLocation);
	}

	{
		for (std::map<std::string, std::map<NetworkId, AuctionLocation *> >::iterator iter = m_searchableVendorByPlanet.begin(); iter != m_searchableVendorByPlanet.end(); ++iter)
		{
			removeAuctionLocationFromList(iter->second, auctionLocation);
		}
	}

	{
		for (std::map<std::pair<std::string, std::string>, std::map<NetworkId, AuctionLocation *> >::iterator iter = m_searchableVendorByRegion.begin(); iter != m_searchableVendorByRegion.end(); ++iter)
		{
			removeAuctionLocationFromList(iter->second, auctionLocation);
		}
	}
}

void AuctionMarket::getPlanetAndRegionFromLocationString(const std::string &locationName, std::string &planet, std::string &region)
{
	unsigned int pos = 0;
	nextString(locationName, pos, planet, '.');
	nextString(locationName, pos, region, '.');
}
