// ======================================================================
//
// CMLoader.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CMLoader_H
#define INCLUDED_CMLoader_H

#include "serverDatabase/dBAuctionRecord.h"
#include "serverDatabase/dBBidRecord.h"
#include "serverDatabase/dBLocationRecord.h"
#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedSynchronization/Mutex.h"
#include <list>

// ======================================================================

namespace DB
{
        class TaskQueue;
        class Server;
}

/**
 * Singleton that organizes and queues requests to load objects from the database.
 */
class CMLoader
{
  public:
	static void install();
	static CMLoader &getInstance();
	void update();
	void onLoadComplete(const std::string & tableName, const int count);
	void loadCommoditiesData();
	void addAuctionLocation(const LocationRecord & data)   {m_mutex.enter(); m_locationList.push_back(data); m_mutex.leave();};
	void addAuctionLocationNoLockMutex(const LocationRecord & data) {m_locationList.push_back(data);};
	void addMarketAuctions(const AuctionRecord & data)     {m_mutex.enter(); m_auctionList.push_back(data); m_mutex.leave();};
	void addMarketAuctionsNoLockMutex(const AuctionRecord & data) {m_auctionList.push_back(data);};
	void addMarketAuctionAttributes(const NetworkId & itemId, const std::string & attributeName, const Unicode::String & attributeValue) {m_mutex.enter(); m_auctionAttributeList.push_back(std::make_pair(itemId, std::make_pair(attributeName, attributeValue))); m_mutex.leave();};
	void addMarketAuctionAttributesNoLockMutex(const NetworkId & itemId, const std::string & attributeName, const Unicode::String & attributeValue) {m_auctionAttributeList.push_back(std::make_pair(itemId, std::make_pair(attributeName, attributeValue)));};
	void addMarketAuctionBid(const BidRecord & data)       {m_mutex.enter(); m_bidList.push_back(data); m_mutex.leave();};
	void addMarketAuctionBidNoLockMutex(const BidRecord & data) {m_bidList.push_back(data);};
	void lockMutex()                                       {m_mutex.enter();};
	void unlockMutex()                                     {m_mutex.leave();};
	int  getLoadTime() const;

  private:
	CMLoader();
	virtual ~CMLoader();
	CMLoader(const CMLoader &);
	CMLoader &  operator = (const CMLoader &);
	
	static void remove();
	static CMLoader *ms_instance;

	int m_tablesLoaded;
	int m_auctionLocationsCount;
	int m_marketAuctionsCount;
	int m_marketAuctionAttributesCount;
	int m_marketAuctionBidsCount;
	Mutex m_mutex;

	DB::TaskQueue *m_taskQ;
	std::list<LocationRecord>  m_locationList;
	std::list<AuctionRecord>   m_auctionList;
	std::list<std::pair<NetworkId, std::pair<std::string, Unicode::String> > > m_auctionAttributeList;
	std::list<BidRecord>       m_bidList;

	time_t m_timeLoadStarted;
	int m_loadTime;
};

// ----------------------------------------------------------------------

inline CMLoader &CMLoader::getInstance()
{
        NOT_NULL(ms_instance);
        return *ms_instance;
}

// ======================================================================

#endif
