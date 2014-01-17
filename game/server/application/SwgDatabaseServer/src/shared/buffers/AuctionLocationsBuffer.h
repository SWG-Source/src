// ======================================================================
//
// AuctionLocationsBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AuctionLocationsBuffer_H
#define INCLUDED_AuctionLocationsBuffer_H

// ======================================================================

#include "SwgDatabaseServer/CommoditiesQuery.h"
#include "SwgDatabaseServer/CommoditiesSchema.h"
#include "serverDatabase/AbstractTableBuffer.h"
#include <map>
#include <string>

// ======================================================================

namespace AuctionLocationsTableBufferNamespace
{
	const int ms_maxItemsPerExec = 10000;
}

using namespace AuctionLocationsTableBufferNamespace;

// ======================================================================

class AuctionLocationsBuffer : public AbstractTableBuffer
{
public:
	explicit                  AuctionLocationsBuffer       (DB::ModeQuery::Mode mode);
	virtual                   ~AuctionLocationsBuffer      ();
	
	virtual bool              load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual bool              save(DB::Session *session);
		
	DBSchema::AuctionLocationsRow * findRowByIndex   (const NetworkId &locationId);

	void                      setAuctionLocations    (const NetworkId &locationId);
	void                      setAuctionLocations    (const NetworkId &locationId, const std::string &locationName, const int salesTax, const NetworkId &salesTaxBankId, const int emptyDate, const int lastAccessDate, const int inactiveDate, const int status, const bool searchEnabled, const int entranceCharge);
	void                      setAuctionLocations    (const NetworkId &locationId, const std::string &locationName, const NetworkId &ownerId, const int salesTax, const NetworkId &salesTaxBankId, const int emptyDate, const int lastAccessDate, const int inactiveDate, const int status, const bool searchEnabled, const int entranceCharge);
	void                      removeAuctionLocations (const NetworkId &locationId);
	virtual void             removeObject           (const NetworkId &object);
	
  private:
	void                      addRowToIndex          (const NetworkId &locationId, DBSchema::AuctionLocationsRow *row);

  private:

	typedef std::map<NetworkId, DBSchema::AuctionLocationsRow*> IndexType;

	DB::ModeQuery::Mode m_mode;
	IndexType m_rows;

 private:
	AuctionLocationsBuffer(); //disable
	AuctionLocationsBuffer(const AuctionLocationsBuffer&); //disable
	AuctionLocationsBuffer & operator=(const AuctionLocationsBuffer&); //disable
}; //lint !e1712 // IndexKey has no default constructor

// ======================================================================

#endif
