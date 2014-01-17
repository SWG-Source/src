// ======================================================================
//
// MarketAuctionBidsBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MarketAuctionBidsBuffer_H
#define INCLUDED_MarketAuctionBidsBuffer_H

// ======================================================================

#include "SwgDatabaseServer/CommoditiesQuery.h"
#include "SwgDatabaseServer/CommoditiesSchema.h"
#include "serverDatabase/AbstractTableBuffer.h"
#include <map>
#include <string>

// ======================================================================

namespace MarketAuctionBidsTableBufferNamespace
{
	const int ms_maxItemsPerExec = 10000;
}

using namespace MarketAuctionBidsTableBufferNamespace;

// ======================================================================

class MarketAuctionBidsBuffer : public AbstractTableBuffer
{
public:
	explicit                  MarketAuctionBidsBuffer       (DB::ModeQuery::Mode mode);
	virtual                   ~MarketAuctionBidsBuffer      ();
	
	virtual bool              load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual bool              save(DB::Session *session);
		
	DBSchema::MarketAuctionBidsRow * findRowByIndex  (const NetworkId &itemId);

	void                      setMarketAuctionBids   (const NetworkId &itemId, const NetworkId &bidderId, const int bid, const int maxProxyBid);
	void                      removeMarketAuctionBids(const NetworkId &itemId);
	virtual void             removeObject           (const NetworkId &object);
	
  private:
	void                      addRowToIndex          (const NetworkId &itemId, DBSchema::MarketAuctionBidsRow *row);

  private:

	typedef std::map<NetworkId, DBSchema::MarketAuctionBidsRow*> IndexType;

	DB::ModeQuery::Mode m_mode;
	IndexType m_rows;

 private:
	MarketAuctionBidsBuffer(); //disable
	MarketAuctionBidsBuffer(const MarketAuctionBidsBuffer&); //disable
	MarketAuctionBidsBuffer & operator=(const MarketAuctionBidsBuffer&); //disable
}; //lint !e1712 // IndexKey has no default constructor

// ======================================================================

#endif
