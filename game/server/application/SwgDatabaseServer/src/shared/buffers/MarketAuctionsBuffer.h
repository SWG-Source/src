// ======================================================================
//
// MarketAuctionsBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MarketAuctionsBuffer_H
#define INCLUDED_MarketAuctionsBuffer_H

// ======================================================================

#include "SwgDatabaseServer/CommoditiesQuery.h"
#include "SwgDatabaseServer/CommoditiesSchema.h"
#include "serverDatabase/AbstractTableBuffer.h"
#include <unordered_map>
#include <string>

// ======================================================================

namespace MarketAuctionsTableBufferNamespace
{
	const int ms_maxItemsPerExec = 10000;
}

using namespace MarketAuctionsTableBufferNamespace;

// ======================================================================

class MarketAuctionsBufferCreate : public AbstractTableBuffer
{
public:
	                          MarketAuctionsBufferCreate();
	virtual                   ~MarketAuctionsBufferCreate();

	virtual bool              load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual bool              save(DB::Session *session);

	DBSchema::MarketAuctionsRow * findRowByIndex     (const NetworkId &itemId);

	void                      setMarketAuctions      (const NetworkId &itemId, const NetworkId &ownerId, const int active);
	void                      setMarketAuctions      (const NetworkId &itemId, const NetworkId &ownerId, const NetworkId &creatorId, const NetworkId &locationId, const int minBid, const int buyNowPrice, const int auctionTimer, std::vector<std::pair<std::string, Unicode::String> > const & attributes, const Unicode::String &userDescription, const int category, const int itemTemplateId, const Unicode::String &itemName, const int itemTimer, const int active, const int itemSize);
	void                      removeMarketAuctions   (const NetworkId &itemId);
	virtual void              removeObject           (const NetworkId &object);

private:
	void                      addRowToIndex          (const NetworkId &itemId, DBSchema::MarketAuctionsRow *row);

private:
	typedef std::unordered_map<NetworkId, DBSchema::MarketAuctionsRow*> IndexType;
	IndexType m_rows;

	typedef std::unordered_map<NetworkId, std::vector<std::pair<std::string, Unicode::String> > > AttributesType;
	AttributesType m_attributes;

private:
	MarketAuctionsBufferCreate(const MarketAuctionsBufferCreate&); //disable
	MarketAuctionsBufferCreate & operator=(const MarketAuctionsBufferCreate&); //disable
}; //lint !e1712 // IndexKey has no default constructor

// ======================================================================

class MarketAuctionsBufferDelete : public AbstractTableBuffer
{
public:
	explicit                  MarketAuctionsBufferDelete();
	virtual                   ~MarketAuctionsBufferDelete();

	virtual bool              load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual bool              save(DB::Session *session);

	DBSchema::MarketAuctionsRowDelete * findRowByIndex(const NetworkId &itemId);

	void                      setMarketAuctions      (const NetworkId &itemId);
	void                      removeMarketAuctions   (const NetworkId &itemId);
	virtual void              removeObject           (const NetworkId &object);

private:
	void                      addRowToIndex          (const NetworkId &itemId, DBSchema::MarketAuctionsRowDelete *row);

private:
	typedef std::unordered_map<NetworkId, DBSchema::MarketAuctionsRowDelete*> IndexType;
	IndexType m_rows;

private:
	MarketAuctionsBufferDelete(const MarketAuctionsBufferDelete&); //disable
	MarketAuctionsBufferDelete & operator=(const MarketAuctionsBufferDelete&); //disable
}; //lint !e1712 // IndexKey has no default constructor

// ======================================================================

class MarketAuctionsBufferUpdate : public AbstractTableBuffer
{
public:
	explicit                  MarketAuctionsBufferUpdate();
	virtual                   ~MarketAuctionsBufferUpdate();

	virtual bool              load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual bool              save(DB::Session *session);

	DBSchema::MarketAuctionsRowUpdate * findRowByIndex(const NetworkId &itemId);

	void                      setMarketAuctions      (const NetworkId &itemId, const NetworkId &ownerId, const int active);
	void                      removeMarketAuctions   (const NetworkId &itemId);
	virtual void              removeObject           (const NetworkId &object);

private:
	void                      addRowToIndex          (const NetworkId &itemId, DBSchema::MarketAuctionsRowUpdate *row);

private:
	typedef std::unordered_map<NetworkId, DBSchema::MarketAuctionsRowUpdate*> IndexType;
	IndexType m_rows;

private:
	MarketAuctionsBufferUpdate(const MarketAuctionsBufferUpdate&); //disable
	MarketAuctionsBufferUpdate & operator=(const MarketAuctionsBufferUpdate&); //disable
}; //lint !e1712 // IndexKey has no default constructor
   
// ======================================================================

#endif
