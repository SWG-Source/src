// ======================================================================
//
// CMLoader.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "CMLoader.h"

#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverNetworkMessages/CommoditiesLoadDoneMessage.h"
#include "serverNetworkMessages/LoadCommoditiesMessage.h"
#include "serverNetworkMessages/GetAuctionLocationsMessage.h"
#include "serverNetworkMessages/GetMarketAuctionAttributesMessage.h"
#include "serverNetworkMessages/GetMarketAuctionBidsMessage.h"
#include "serverNetworkMessages/GetMarketAuctionsMessage.h"
#include "sharedDatabaseInterface/DbTaskQueue.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "TaskGetAuctionAttributeList.h"
#include "TaskGetAuctionList.h"
#include "TaskGetLocationList.h"
#include "TaskGetBidList.h"

CMLoader *CMLoader::ms_instance = nullptr;

// ======================================================================

void CMLoader::install()
{
	DEBUG_FATAL(ms_instance, ("Installed CMLoader twice.\n"));
	ms_instance = new CMLoader;
	ExitChain::add(&remove, "Loader::remove");
}

// ----------------------------------------------------------------------

void CMLoader::remove()
{
	NOT_NULL(ms_instance);
	delete ms_instance;
	ms_instance = nullptr;
}

// ----------------------------------------------------------------------

CMLoader::CMLoader() :
	m_tablesLoaded(0),
	m_auctionLocationsCount(0),
	m_marketAuctionsCount(0),
	m_marketAuctionAttributesCount(0),
	m_marketAuctionBidsCount(0),
	m_mutex(),
	m_taskQ(new DB::TaskQueue(static_cast<unsigned int>(ConfigServerDatabase::getLoaderThreads()),DatabaseProcess::getInstance().getDBServer(),0)),
	m_locationList(),
	m_auctionList(),
	m_auctionAttributeList(),
	m_bidList(),
	m_timeLoadStarted(0), // 0 means load hasn't started yet
	m_loadTime(-1) // -1 means load hasn't completed yet
{
	;
}

// ----------------------------------------------------------------------

CMLoader:: ~CMLoader()
{
}

// ----------------------------------------------------------------------
// It is important that commodity tables are loaded in the following order:
// AUCTION_LOCATIONS, MARKET_AUCTIONS, MARKET_AUCTION_ATTRIBUTES, and MARKET_AUCTION_BIDS.
// To make sure commodity tables are loaded in this order, TaskGetLocationList is put
// into taskQ first.  When onLoadComplete for AUCTION_LOCATIONS finishes,
// TaskGetAuctionList is put into taskQ, and so on.

void CMLoader::loadCommoditiesData()
{
	m_timeLoadStarted = time(0); // mark when the load started
	m_loadTime = -1; // -1 means load hasn't completed yet
	m_tablesLoaded = 0;
	m_taskQ->asyncRequest(new TaskGetLocationList);
	LOG("SwgDatabaseServer", ("start CommoditiesLoad"));
}

void CMLoader::onLoadComplete(const std::string & tableName, const int count)
{
	m_tablesLoaded++;
	if (tableName == "AUCTION_LOCATIONS")
	{
		m_auctionLocationsCount = count;
		m_taskQ->asyncRequest(new TaskGetAuctionList);
		LOG("SwgDatabaseServer", ("finish loading AUCTION_LOCATIONS, start loading MARKET_AUCTIONS"));
	}
	else if (tableName == "MARKET_AUCTIONS")
	{
		m_marketAuctionsCount = count;
		m_taskQ->asyncRequest(new TaskGetAuctionAttributeList);
		LOG("SwgDatabaseServer", ("finish loading MARKET_AUCTIONS, start loading MARKET_AUCTION_ATTRIBUTES"));
	}
	else if (tableName == "MARKET_AUCTION_ATTRIBUTES")
	{
		m_marketAuctionAttributesCount = count;
		m_taskQ->asyncRequest(new TaskGetBidList);
		LOG("SwgDatabaseServer", ("finish loading MARKET_AUCTION_ATTRIBUTES, start loading MARKET_AUCTION_BIDS"));
	}
	else if (tableName == "MARKET_AUCTION_BIDS")
	{
		m_marketAuctionBidsCount = count;
		LOG("SwgDatabaseServer", ("finish loading MARKET_AUCTION_BIDS"));
	}
	else
		FATAL(true, ("Unknown table %s loaded for commodities.\n", tableName.c_str()));
}

// ----------------------------------------------------------------------

void CMLoader::update()
{
	volatile int i;
	NOT_NULL(m_taskQ);
	m_taskQ->update(0);
	m_mutex.enter();
	if (! m_locationList.empty())
	{
		GetAuctionLocationsMessage msg;
		int numLocations = 0;
		std::list<LocationRecord>::iterator locationIterator = m_locationList.begin();
		while (locationIterator != m_locationList.end())
		{
			msg.addAuctionLocation(
				locationIterator->m_locationId, 
				locationIterator->m_locationString.length(), 
				locationIterator->m_locationString, 
				locationIterator->m_ownerId, 
				locationIterator->m_salesTax, 
				locationIterator->m_salesTaxBankId,
				locationIterator->m_emptyDate,
				locationIterator->m_lastAccessDate,
				locationIterator->m_inactiveDate,
				locationIterator->m_status,
				locationIterator->m_searchEnabled,
				locationIterator->m_entranceCharge
			);

			if (++numLocations >= ConfigServerDatabase::getAuctionLocationLoadBatchSize())
			{
				DatabaseProcess::getInstance().sendToCommoditiesServer(msg, true);
				numLocations = 0;
				msg.clearAllAuctionLocations();
			}
			++locationIterator;
		}

		if (!msg.getAuctionLocations().empty())
			DatabaseProcess::getInstance().sendToCommoditiesServer(msg, true);

		m_locationList.clear();
	}
	if (! m_auctionList.empty())
	{
		GetMarketAuctionsMessage msg;
		int numAuctions = 0;
		std::list<AuctionRecord>::iterator auctionIterator = m_auctionList.begin();
		while (auctionIterator != m_auctionList.end())
		{
			msg.addAuction(auctionIterator->m_itemId, auctionIterator->m_ownerId, auctionIterator->m_creatorId, auctionIterator->m_locationId, auctionIterator->m_minBid, auctionIterator->m_buyNowPrice, auctionIterator->m_auctionTimer, auctionIterator->m_oob.length(), auctionIterator->m_oob, auctionIterator->m_userDescription.length(), auctionIterator->m_userDescription, auctionIterator->m_category, auctionIterator->m_itemTemplateId, auctionIterator->m_itemName.length(), auctionIterator->m_itemName, auctionIterator->m_itemTimer, auctionIterator->m_active, auctionIterator->m_itemSize);
			if (++numAuctions >= ConfigServerDatabase::getAuctionLoadBatchSize())
			{
				DatabaseProcess::getInstance().sendToCommoditiesServer(msg, true);
				numAuctions = 0;
				msg.clearAllAuctions();
			}
			++auctionIterator;
		}

		if (!msg.getAuctions().empty())
			DatabaseProcess::getInstance().sendToCommoditiesServer(msg, true);

		m_auctionList.clear();
	}
	if (! m_auctionAttributeList.empty())
	{
		GetMarketAuctionAttributesMessage msg;
		int numAttributes = 0;
		std::list<std::pair<NetworkId, std::pair<std::string, Unicode::String> > >::iterator auctionAttributeIterator = m_auctionAttributeList.begin();
		while (auctionAttributeIterator != m_auctionAttributeList.end())
		{
			msg.addAttribute(auctionAttributeIterator->first, auctionAttributeIterator->second.first, auctionAttributeIterator->second.second);
			if (++numAttributes >= ConfigServerDatabase::getAuctionAttributeLoadBatchSize())
			{
				DatabaseProcess::getInstance().sendToCommoditiesServer(msg, true);
				numAttributes = 0;
				msg.clearAllAttributes();
			}
			++auctionAttributeIterator;
		}

		if (!msg.getAttributes().empty())
			DatabaseProcess::getInstance().sendToCommoditiesServer(msg, true);

		m_auctionAttributeList.clear();
	}
	if (! m_bidList.empty())
	{
		GetMarketAuctionBidsMessage msg;
		int numBids = 0;
		std::list<BidRecord>::iterator bidIterator = m_bidList.begin();
		while (bidIterator != m_bidList.end())
		{
			msg.addMarketAuctionBid(bidIterator->m_itemId, bidIterator->m_bidderId, bidIterator->m_bid, bidIterator->m_maxProxyBid);
			if (++numBids >= ConfigServerDatabase::getAuctionBidLoadBatchSize())
			{
				DatabaseProcess::getInstance().sendToCommoditiesServer(msg, true);
				numBids = 0;
				msg.clearAllMarketAuctionBids();
			}
			++bidIterator;
		}

		if (!msg.getMarketAuctionBids().empty())
			DatabaseProcess::getInstance().sendToCommoditiesServer(msg, true);

		m_bidList.clear();
	}
	i = m_tablesLoaded;
	if (i == 4)
	{
		m_tablesLoaded = 0;

		// record how long it took to load
		if ((m_loadTime == -1) && (m_timeLoadStarted != 0))
			m_loadTime = int((time(0) - m_timeLoadStarted) / 60);

		// commodity loading finished, send message back to CommoditiesServer
		DEBUG_REPORT_LOG(true, ("Sending CommoditiesLoadDoneMessage to CommoditesServer. Auction_Locations = %d, Market_Auctions = %d, Market_Auction_Attributes = %d, Market_Auction_Bids = %d\n",
			m_auctionLocationsCount, m_marketAuctionsCount, m_marketAuctionAttributesCount, m_marketAuctionBidsCount));
		LOG("SwgDatabaseServer", ("Sending CommoditiesLoadDoneMessage to CommoditesServer. Auction_Locations = %d, Market_Auctions = %d, Market_Auction_Attributes = %d, Market_Auction_Bids = %d\n",
			m_auctionLocationsCount, m_marketAuctionsCount, m_marketAuctionAttributesCount, m_marketAuctionBidsCount));
		CommoditiesLoadDoneMessage msg(m_auctionLocationsCount, m_marketAuctionsCount, m_marketAuctionAttributesCount, m_marketAuctionBidsCount);
		DatabaseProcess::getInstance().sendToCommoditiesServer(msg, true);
	}
	m_mutex.leave();
}

// ----------------------------------------------------------------------

int CMLoader::getLoadTime() const
{
	if (m_timeLoadStarted == 0)
	{
		// load hasn't started yet
		return 0;
	}
	else
	{
		if (m_loadTime != -1)
			return m_loadTime;
		else
			// return how long we have been loading
			return int((time(0) - m_timeLoadStarted) / 60);
	}
}

// ======================================================================
