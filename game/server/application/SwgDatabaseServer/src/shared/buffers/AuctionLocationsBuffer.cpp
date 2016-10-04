// ======================================================================
//
// AuctionLocationsBuffer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/AuctionLocationsBuffer.h"
#include "SwgDatabaseServer/CommoditiesQuery.h"
#include "SwgDatabaseServer/CommoditiesSchema.h"
#include "serverGame/ServerPlayerObjectTemplate.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"
#include <vector>

// ======================================================================

AuctionLocationsBuffer::AuctionLocationsBuffer(DB::ModeQuery::Mode mode) :
		AbstractTableBuffer(),
		m_mode(mode),
		m_rows()
{
}

// ----------------------------------------------------------------------

AuctionLocationsBuffer::~AuctionLocationsBuffer(void)
{
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		delete i->second;
		i->second=nullptr;
	}
}

// ----------------------------------------------------------------------
	
bool AuctionLocationsBuffer::load(DB::Session *session, const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase)
{
	return true;
}
	
// ----------------------------------------------------------------------
	
void AuctionLocationsBuffer::removeObject(const NetworkId &object)
{
	removeAuctionLocations(object);
}

// ----------------------------------------------------------------------
	
void AuctionLocationsBuffer::removeAuctionLocations(const NetworkId &locationId)
{
	IndexType::iterator i=m_rows.find(locationId);
	if (i!=m_rows.end())
	{
		delete i->second;
		i->second=nullptr;
		m_rows.erase(i);
	}
}

// ----------------------------------------------------------------------

void AuctionLocationsBuffer::setAuctionLocations(const NetworkId &locationId)
{
	DBSchema::AuctionLocationsRow *row=findRowByIndex(locationId);
	if (!row)
	{
		row=new DBSchema::AuctionLocationsRow;
		addRowToIndex(locationId, row);
	}
	row->location_id = locationId;
}

// ----------------------------------------------------------------------

void AuctionLocationsBuffer::setAuctionLocations(const NetworkId &locationId, const std::string &locationName, const int salesTax, const NetworkId &salesTaxBankId, const int emptyDate, const int lastAccessDate, const int inactiveDate, const int status, const bool searchEnabled, const int entranceCharge)
{
	DBSchema::AuctionLocationsRow *row=findRowByIndex(locationId);
	if (!row)
	{
		row=new DBSchema::AuctionLocationsRow;
		addRowToIndex(locationId, row);
	}
	row->location_id = locationId;
	row->location_name = locationName;
	row->sales_tax = salesTax;
	row->sales_tax_bank_id = salesTaxBankId;
	row->empty_date = emptyDate;
	row->last_access_date = lastAccessDate;
	row->inactive_date = inactiveDate;
	row->status = status;
	row->search_enabled = searchEnabled;
	row->entrance_charge = entranceCharge;
}

// ----------------------------------------------------------------------

void AuctionLocationsBuffer::setAuctionLocations(const NetworkId &locationId, const std::string &locationName, const NetworkId &ownerId, const int salesTax, const NetworkId &salesTaxBankId, const int emptyDate, const int lastAccessDate, const int inactiveDate, const int status, const bool searchEnabled, const int entranceCharge)
{
	DBSchema::AuctionLocationsRow *row=findRowByIndex(locationId);
	if (!row)
	{
		row=new DBSchema::AuctionLocationsRow;
		addRowToIndex(locationId, row);
	}
	row->location_id = locationId;
	row->location_name = locationName;
	row->owner_id = ownerId;
	row->sales_tax = salesTax;
	row->sales_tax_bank_id = salesTaxBankId;
	row->empty_date = emptyDate;
	row->last_access_date = lastAccessDate;
	row->inactive_date = inactiveDate;
	row->status = status;
	row->search_enabled = searchEnabled;
	row->entrance_charge = entranceCharge;
}

// ----------------------------------------------------------------------

DBSchema::AuctionLocationsRow * AuctionLocationsBuffer::findRowByIndex(const NetworkId &locationId)
{
	IndexType::iterator i=m_rows.find(locationId);
	if (i==m_rows.end())
		return 0;
	else
		return (*i).second;
}

// ----------------------------------------------------------------------

void AuctionLocationsBuffer::addRowToIndex (const NetworkId &locationId, DBSchema::AuctionLocationsRow *row)
{
	m_rows[locationId]=row;
}

// ----------------------------------------------------------------------

bool AuctionLocationsBuffer::save(DB::Session *session)
{
	LOG("SaveCounts",("AuctionLocations:  %i saved to db",m_rows.size()));
	DEBUG_REPORT_LOG(true, ("Saving Auctions Locations Buffer. mode:%d rowcount:%d\n", m_mode, m_rows.size()));
	
	DBQuery::AuctionLocationsQuery qry;
	
	if (m_mode==DB::ModeQuery::mode_INSERT)
		qry.insertMode();
	else if (m_mode == DB::ModeQuery::mode_UPDATE)
		qry.updateMode();
	else
		qry.deleteMode();
	if (!qry.setupData(session))
		return false;
	
	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		if (!qry.addData(i->second))
			return false;
		if (qry.getNumItems() >= AuctionLocationsTableBufferNamespace::ms_maxItemsPerExec)
		{
			if (! (session->exec(&qry)))
				return false;
			qry.clearData();
		}
	}
	
	if (qry.getNumItems() != 0)
		if (! (session->exec(&qry)))
			return false;

	qry.done();
	qry.freeData();
	return true;

}

// ======================================================================
