#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "CommoditiesQuery.h"
#include "serverDatabase/DatabaseProcess.h"
#include "SwgDatabaseServer/CommoditiesSchema.h"

using namespace DBQuery;
using namespace DBSchema;


// Static instance counters
volatile int AuctionLocationsRow::m_sAuctionRowsCreated = 0;
volatile int AuctionLocationsRow::m_sAuctionRowsDeleted = 0;

volatile int MarketAuctionsRow::m_sAuctionRowsCreated = 0;
volatile int MarketAuctionsRow::m_sAuctionRowsDeleted = 0;

volatile int MarketAuctionsRowDelete::m_sAuctionRowsCreated = 0;
volatile int MarketAuctionsRowDelete::m_sAuctionRowsDeleted = 0;

volatile int MarketAuctionsRowUpdate::m_sAuctionRowsCreated = 0;
volatile int MarketAuctionsRowUpdate::m_sAuctionRowsDeleted = 0;

volatile int MarketAuctionBidsRow::m_sAuctionRowsCreated = 0;
volatile int MarketAuctionBidsRow::m_sAuctionRowsDeleted = 0;



// ======================================================================

namespace CommoditiesQueryNamespace
{
	const int ms_fetchBatchSize = 1000;
}
using namespace CommoditiesQueryNamespace;

// ======================================================================

AuctionLocationsQuery::AuctionLocationsQuery() :
	DatabaseProcessQuery(new AuctionLocationsRow),
	m_numItems(0)
{
}

bool AuctionLocationsQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
			if (!m_location_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_location_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_owner_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_sales_taxes.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_sales_tax_bank_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_empty_date.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_last_access_date.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_inactive_date.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_status.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_search_enabled.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_entrance_charge.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			break;

		case mode_INSERT:
			if (!m_location_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_location_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_owner_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_sales_taxes.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_sales_tax_bank_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_empty_date.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_last_access_date.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_inactive_date.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_status.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_search_enabled.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_entrance_charge.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			if (!m_location_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
	return true;
}

bool AuctionLocationsQuery::addData(const DB::Row *_data)
{
	const AuctionLocationsRow *myData=dynamic_cast<const AuctionLocationsRow*>(_data);
	FATAL(myData == nullptr, ("Adding nullptr data into AuctionLocations"));
	switch(mode)
	{
		case mode_UPDATE:
			if (!m_location_ids.push_back(myData->location_id.getValueASCII())) return false;
			if (!m_location_names.push_back(myData->location_name.getValueASCII())) return false;
			if (!m_owner_ids.push_back(myData->owner_id.isNull(), myData->owner_id.getValueASCII())) return false;
			if (!m_sales_taxes.push_back(myData->sales_tax.isNull(), myData->sales_tax.getValue())) return false;
			if (!m_sales_tax_bank_ids.push_back(myData->sales_tax_bank_id.isNull(), myData->sales_tax_bank_id.getValue())) return false;
			if (!m_empty_date.push_back(myData->empty_date.isNull(), myData->empty_date.getValue())) return false;
			if (!m_last_access_date.push_back(myData->last_access_date.isNull(), myData->last_access_date.getValue())) return false;
			if (!m_inactive_date.push_back(myData->inactive_date.isNull(), myData->inactive_date.getValue())) return false;
			if (!m_status.push_back(myData->status.isNull(), myData->status.getValue())) return false;
			if (!m_search_enabled.push_back(myData->search_enabled.isNull(), myData->search_enabled.getValue())) return false;
			if (!m_entrance_charge.push_back(myData->entrance_charge.isNull(), myData->entrance_charge.getValue())) return false;
			break;

		case mode_INSERT:
			if (!m_location_ids.push_back(myData->location_id.getValueASCII())) return false;
			if (!m_location_names.push_back(myData->location_name.getValueASCII())) return false;
			if (!m_owner_ids.push_back(myData->owner_id.isNull(), myData->owner_id.getValueASCII())) return false;
			if (!m_sales_taxes.push_back(myData->sales_tax.isNull(), myData->sales_tax.getValue())) return false;
			if (!m_sales_tax_bank_ids.push_back(myData->sales_tax_bank_id.isNull(), myData->sales_tax_bank_id.getValue())) return false;
			if (!m_empty_date.push_back(myData->empty_date.isNull(), myData->empty_date.getValue())) return false;
			if (!m_last_access_date.push_back(myData->last_access_date.isNull(), myData->last_access_date.getValue())) return false;
			if (!m_inactive_date.push_back(myData->inactive_date.isNull(), myData->inactive_date.getValue())) return false;
			if (!m_status.push_back(myData->status.isNull(), myData->status.getValue())) return false;
			if (!m_search_enabled.push_back(myData->search_enabled.isNull(), myData->search_enabled.getValue())) return false;
			if (!m_entrance_charge.push_back(myData->entrance_charge.isNull(), myData->entrance_charge.getValue())) return false;
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			if (!m_location_ids.push_back(myData->location_id.getValueASCII())) return false;
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

	m_numItems=m_numItems.getValue() + 1;
	return true;
}

int AuctionLocationsQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void AuctionLocationsQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
			m_location_ids.clear();
			m_location_names.clear();
			m_owner_ids.clear();
			m_sales_taxes.clear();
			m_sales_tax_bank_ids.clear();
			m_empty_date.clear();
			m_last_access_date.clear();
			m_inactive_date.clear();
			m_status.clear();
			m_search_enabled.clear();
			m_entrance_charge.clear();
			break;

		case mode_INSERT:
			m_location_ids.clear();
			m_location_names.clear();
			m_owner_ids.clear();
			m_sales_taxes.clear();
			m_sales_tax_bank_ids.clear();
			m_empty_date.clear();
			m_last_access_date.clear();
			m_inactive_date.clear();
			m_status.clear();
			m_search_enabled.clear();
			m_entrance_charge.clear();
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			m_location_ids.clear();
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

	m_numItems=0;
}

void AuctionLocationsQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
			m_location_ids.free();
			m_location_names.free();
			m_owner_ids.free();
			m_sales_taxes.free();
			m_sales_tax_bank_ids.free();
			m_empty_date.free();
			m_last_access_date.free();
			m_inactive_date.free();
			m_status.free();
			m_search_enabled.free();
			m_entrance_charge.free();
			break;

		case mode_INSERT:
			m_location_ids.free();
			m_location_names.free();
			m_owner_ids.free();
			m_sales_taxes.free();
			m_sales_tax_bank_ids.free();
			m_empty_date.free();
			m_last_access_date.free();
			m_inactive_date.free();
			m_status.free();
			m_search_enabled.free();
			m_entrance_charge.free();
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			m_location_ids.free();
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

bool AuctionLocationsQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
			if (!bindParameter(m_location_ids)) return false;
			if (!bindParameter(m_location_names)) return false;
			if (!bindParameter(m_owner_ids)) return false;
			if (!bindParameter(m_sales_taxes)) return false;
			if (!bindParameter(m_sales_tax_bank_ids)) return false;
			if (!bindParameter(m_empty_date)) return false;
			if (!bindParameter(m_last_access_date)) return false;
			if (!bindParameter(m_inactive_date)) return false;
			if (!bindParameter(m_status)) return false;
			if (!bindParameter(m_search_enabled)) return false;
			if (!bindParameter(m_entrance_charge)) return false;
			if (!bindParameter(m_numItems)) return false;
			break;

		case mode_INSERT:
			if (!bindParameter(m_location_ids)) return false;
			if (!bindParameter(m_location_names)) return false;
			if (!bindParameter(m_owner_ids)) return false;
			if (!bindParameter(m_sales_taxes)) return false;
			if (!bindParameter(m_sales_tax_bank_ids)) return false;
			if (!bindParameter(m_empty_date)) return false;
			if (!bindParameter(m_last_access_date)) return false;
			if (!bindParameter(m_inactive_date)) return false;
			if (!bindParameter(m_status)) return false;
			if (!bindParameter(m_search_enabled)) return false;
			if (!bindParameter(m_entrance_charge)) return false;
			if (!bindParameter(m_numItems)) return false;
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			if (!bindParameter(m_location_ids)) return false;
			if (!bindParameter(m_numItems)) return false;
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
return true;

}

bool AuctionLocationsQuery::bindColumns()
{
	return true;
}

void AuctionLocationsQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() + "cm_persister.update_auction_locations (:location_id, :location_name, :owner_id, :sales_tax, :sales_tax_bank_id, :empty_date, :last_access_date, :inactive_date, :status, :search_enabled, :entrance_charge, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() + "cm_persister.insert_auction_locations (:location_id, :location_name, :owner_id, :sales_tax, :sales_tax_bank_id, :empty_date, :last_access_date, :inactive_date, :status, :search_enabled, :entrance_charge, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() + "cm_persister.delete_auction_locations (:location_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

AuctionLocationsQuerySelect::AuctionLocationsQuerySelect(const std::string &schema) :
		m_data(ms_fetchBatchSize),
		m_schema(schema)
{
}

bool AuctionLocationsQuerySelect::bindParameters ()
{
	return true;
}

bool AuctionLocationsQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].location_id)) return false;
	if (!bindCol(m_data[0].location_name)) return false;
	if (!bindCol(m_data[0].owner_id)) return false;
	if (!bindCol(m_data[0].sales_tax)) return false;
	if (!bindCol(m_data[0].sales_tax_bank_id)) return false;
	if (!bindCol(m_data[0].empty_date)) return false;
	if (!bindCol(m_data[0].last_access_date)) return false;
	if (!bindCol(m_data[0].inactive_date)) return false;
	if (!bindCol(m_data[0].status)) return false;
	if (!bindCol(m_data[0].search_enabled)) return false;
	if (!bindCol(m_data[0].entrance_charge)) return false;
	return true;
}

const std::vector<AuctionLocationsRow> & AuctionLocationsQuerySelect::getData() const
{
	return m_data;
}

void AuctionLocationsQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"cm_loader.get_location_list(); end;";
}

DB::Query::QueryMode AuctionLocationsQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================

MarketAuctionsQuery::MarketAuctionsQuery() :
	DatabaseProcessQuery(new MarketAuctionsRow),
	m_numItems(0)
{
}

bool MarketAuctionsQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
			if (!m_item_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_owner_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_actives.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			break;

		case mode_INSERT:
			if (!m_item_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_owner_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_creator_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_location_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_min_bids.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_buy_now_prices.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_auction_timers.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_oobs.create(session, "VAOFLONGSTRING", DatabaseProcess::getInstance().getSchema(),4000)) return false;
			if (!m_user_descriptions.create(session, "VAOFLONGSTRING", DatabaseProcess::getInstance().getSchema(),4000)) return false;
			if (!m_categories.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_item_names.create(session, "VAOFLONGSTRING", DatabaseProcess::getInstance().getSchema(),4000)) return false;
			if (!m_item_timers.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_actives.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_item_sizes.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_object_template_ids.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			if (!m_item_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
	return true;
}

bool MarketAuctionsQuery::addData(const DB::Row *_data)
{
	switch(mode)
	{
		case mode_UPDATE:
			{
				const MarketAuctionsRowUpdate *myData=dynamic_cast<const MarketAuctionsRowUpdate*>(_data);
				FATAL(myData == nullptr, ("Adding nullptr data into MarketAuctions for mode_UPDATE"));

				if (!m_item_ids.push_back(myData->item_id.getValueASCII())) return false;
				if (!m_owner_ids.push_back(myData->owner_id.getValueASCII())) return false;
				if (!m_actives.push_back(myData->active.isNull(), myData->active.getValue())) return false;
				break;
			}

		case mode_INSERT:
			{
				const MarketAuctionsRow *myData=dynamic_cast<const MarketAuctionsRow*>(_data);
				FATAL(myData == nullptr, ("Adding nullptr data into MarketAuctions for mode_INSERT"));

				if (!m_item_ids.push_back(myData->item_id.getValueASCII())) return false;
				if (!m_owner_ids.push_back(myData->owner_id.getValueASCII())) return false;
				if (!m_creator_ids.push_back(myData->creator_id.isNull(), myData->creator_id.getValueASCII())) return false;
				if (!m_location_ids.push_back(myData->location_id.getValueASCII())) return false;
				if (!m_min_bids.push_back(myData->min_bid.isNull(), myData->min_bid.getValue())) return false;
				if (!m_buy_now_prices.push_back(myData->buy_now_price.isNull(), myData->buy_now_price.getValue())) return false;
				if (!m_auction_timers.push_back(myData->auction_timer.isNull(), myData->auction_timer.getValue())) return false;
				if (!m_oobs.push_back(myData->oob.isNull(), myData->oob.getValueASCII())) return false;
				if (!m_user_descriptions.push_back(myData->user_description.isNull(), myData->user_description.getValue())) return false;
				if (!m_categories.push_back(myData->category.isNull(), myData->category.getValue())) return false;
				if (!m_item_names.push_back(myData->item_name.isNull(), myData->item_name.getValue())) return false;
				if (!m_item_timers.push_back(myData->item_timer.isNull(), myData->item_timer.getValue())) return false;
				if (!m_actives.push_back(myData->active.isNull(), myData->active.getValue())) return false;
				if (!m_item_sizes.push_back(myData->item_size.isNull(), myData->item_size.getValue())) return false;
				if (!m_object_template_ids.push_back(myData->object_template_id.isNull(), myData->object_template_id.getValue())) return false;
				break;
			}

		case mode_SELECT:
			break;

		case mode_DELETE:
			{
				const MarketAuctionsRowDelete *myData=dynamic_cast<const MarketAuctionsRowDelete*>(_data);
				FATAL(myData == nullptr, ("Adding nullptr data into MarketAuctions for mode_DELETE"));

				if (!m_item_ids.push_back(myData->item_id.getValueASCII())) return false;
				break;
			}

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

	m_numItems=m_numItems.getValue() + 1;
	return true;
}

int MarketAuctionsQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void MarketAuctionsQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
			m_item_ids.clear();
			m_owner_ids.clear();
			m_actives.clear();
			break;

		case mode_INSERT:
			m_item_ids.clear();
			m_owner_ids.clear();
			m_creator_ids.clear();
			m_location_ids.clear();
			m_min_bids.clear();
			m_buy_now_prices.clear();
			m_auction_timers.clear();
			m_oobs.clear();
			m_user_descriptions.clear();
			m_categories.clear();
			m_item_names.clear();
			m_item_timers.clear();
			m_actives.clear();
			m_item_sizes.clear();
			m_object_template_ids.clear();
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			m_item_ids.clear();
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

	m_numItems=0;
}

void MarketAuctionsQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
			m_item_ids.free();
			m_owner_ids.free();
			m_actives.free();
			break;

		case mode_INSERT:
			m_item_ids.free();
			m_owner_ids.free();
			m_creator_ids.free();
			m_location_ids.free();
			m_min_bids.free();
			m_buy_now_prices.free();
			m_auction_timers.free();
			m_oobs.free();
			m_user_descriptions.free();
			m_categories.free();
			m_item_names.free();
			m_item_timers.free();
			m_actives.free();
			m_item_sizes.free();
			m_object_template_ids.free();
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			m_item_ids.free();
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

bool MarketAuctionsQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
			if (!bindParameter(m_item_ids)) return false;
			if (!bindParameter(m_owner_ids)) return false;
			if (!bindParameter(m_actives)) return false;
			if (!bindParameter(m_numItems)) return false;
			break;
		
		case mode_INSERT:
			if (!bindParameter(m_item_ids)) return false;
			if (!bindParameter(m_owner_ids)) return false;
			if (!bindParameter(m_creator_ids)) return false;
			if (!bindParameter(m_location_ids)) return false;
			if (!bindParameter(m_min_bids)) return false;
			if (!bindParameter(m_buy_now_prices)) return false;
			if (!bindParameter(m_auction_timers)) return false;
			if (!bindParameter(m_oobs)) return false;
			if (!bindParameter(m_user_descriptions)) return false;
			if (!bindParameter(m_categories)) return false;
			if (!bindParameter(m_item_names)) return false;
			if (!bindParameter(m_item_timers)) return false;
			if (!bindParameter(m_actives)) return false;
			if (!bindParameter(m_item_sizes)) return false;
			if (!bindParameter(m_object_template_ids)) return false;
			if (!bindParameter(m_numItems)) return false;
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			if (!bindParameter(m_item_ids)) return false;
			if (!bindParameter(m_numItems)) return false;
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
return true;

}

bool MarketAuctionsQuery::bindColumns()
{
	return true;
}

void MarketAuctionsQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
			sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() +
			    "cm_persister.update_market_auctions (:item_id, :owner_id, :active, :chunk_size); end;";
			break;

		case mode_INSERT:
			sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() +
			    "cm_persister.insert_market_auctions (:item_id, :owner_id, :creator_id, :location_id, :min_bid, :buy_now_price, :auction_timer, :oob, :user_description, :category, :item_name, :item_timer, :active, :item_size, :object_template_id, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() +
			    "cm_persister.delete_market_auctions (:item_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

MarketAuctionsQuerySelect::MarketAuctionsQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool MarketAuctionsQuerySelect::bindParameters ()
{
	return true;
}

bool MarketAuctionsQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].item_id)) return false;
	if (!bindCol(m_data[0].owner_id)) return false;
	if (!bindCol(m_data[0].creator_id)) return false;
	if (!bindCol(m_data[0].location_id)) return false;
	if (!bindCol(m_data[0].min_bid)) return false;
	if (!bindCol(m_data[0].buy_now_price)) return false;
	if (!bindCol(m_data[0].auction_timer)) return false;
	if (!bindCol(m_data[0].oob)) return false;
	if (!bindCol(m_data[0].user_description)) return false;
	if (!bindCol(m_data[0].category)) return false;
	if (!bindCol(m_data[0].item_name)) return false;
	if (!bindCol(m_data[0].item_timer)) return false;
	if (!bindCol(m_data[0].active)) return false;
	if (!bindCol(m_data[0].item_size)) return false;
	return true;
}

const std::vector<MarketAuctionsRow> & MarketAuctionsQuerySelect::getData() const
{
	return m_data;
}

void MarketAuctionsQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"cm_loader.get_auction_list(); end;";
}

DB::Query::QueryMode MarketAuctionsQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================

MarketAuctionBidsQuery::MarketAuctionBidsQuery() :
	DatabaseProcessQuery(new MarketAuctionBidsRow),
	m_numItems(0)
{
}

bool MarketAuctionBidsQuery::setupData(DB::Session *session)
{

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_item_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_bidder_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_bids.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			if (!m_max_proxy_bids.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			if (!m_item_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
	return true;	
}

bool MarketAuctionBidsQuery::addData(const DB::Row *_data)
{
	const MarketAuctionBidsRow *myData=dynamic_cast<const MarketAuctionBidsRow*>(_data);
	
	FATAL(myData == nullptr, ("Adding nullptr data into MarketAuctionBids"));

	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!m_item_ids.push_back(myData->item_id.getValueASCII())) return false;
			if (!m_bidder_ids.push_back(myData->bidder_id.getValueASCII())) return false;
			if (!m_bids.push_back(myData->bid.getValue())) return false;
			if (!m_max_proxy_bids.push_back(myData->max_proxy_bid.isNull(), myData->max_proxy_bid.getValue())) return false;
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			if (!m_item_ids.push_back(myData->item_id.getValueASCII())) return false;
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

	m_numItems=m_numItems.getValue() + 1;
	return true;
}

int MarketAuctionBidsQuery::getNumItems() const
{
	return m_numItems.getValue();
}

void MarketAuctionBidsQuery::clearData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_item_ids.clear();
			m_bidder_ids.clear();
			m_bids.clear();
			m_max_proxy_bids.clear();
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			m_item_ids.clear();
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

	m_numItems=0;
}

void MarketAuctionBidsQuery::freeData()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			m_item_ids.free();
			m_bidder_ids.free();
			m_bids.free();
			m_max_proxy_bids.free();
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			m_item_ids.free();
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

bool MarketAuctionBidsQuery::bindParameters()
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			if (!bindParameter(m_item_ids)) return false;
			if (!bindParameter(m_bidder_ids)) return false;
			if (!bindParameter(m_bids)) return false;
			if (!bindParameter(m_max_proxy_bids)) return false;
			if (!bindParameter(m_numItems)) return false;
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			if (!bindParameter(m_item_ids)) return false;
			if (!bindParameter(m_numItems)) return false;
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
return true;

}

bool MarketAuctionBidsQuery::bindColumns()
{
	return true;
}

void MarketAuctionBidsQuery::getSQL(std::string &sql)
{
	switch(mode)
	{
		case mode_UPDATE:
		case mode_INSERT:
			sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() +
			    "cm_persister.insert_market_auction_bids (:item_id, :bidder_id, :bid, :max_proxy_bid, :chunk_size); end;";
			break;

		case mode_SELECT:
			break;

		case mode_DELETE:
			sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() +
			    "cm_persister.delete_market_auction_bids (:item_id, :chunk_size ); end;";
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}

}

MarketAuctionBidsQuerySelect::MarketAuctionBidsQuerySelect(const std::string &schema) :
	m_data(ms_fetchBatchSize),
	m_schema(schema)
{
}

bool MarketAuctionBidsQuerySelect::bindParameters ()
{
	return true;
}

bool MarketAuctionBidsQuerySelect::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, ms_fetchBatchSize);

	if (!bindCol(m_data[0].item_id)) return false;
	if (!bindCol(m_data[0].bidder_id)) return false;
	if (!bindCol(m_data[0].bid)) return false;
	if (!bindCol(m_data[0].max_proxy_bid)) return false;
	return true;
}

const std::vector<MarketAuctionBidsRow> & MarketAuctionBidsQuerySelect::getData() const
{
	return m_data;
}

void MarketAuctionBidsQuerySelect::getSQL(std::string &sql)
{
			sql=std::string("begin :result := ")+m_schema+"cm_loader.get_bid_list(); end;";
}

DB::Query::QueryMode MarketAuctionBidsQuerySelect::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}
// ======================================================================

MarketAuctionsAttributesQuery::MarketAuctionsAttributesQuery() :
	DatabaseProcessQuery(new MarketAuctionsRow),
	m_numItems(0)
{
}

// ----------------------------------------------------------------------

bool MarketAuctionsAttributesQuery::setupData(DB::Session *session)
{
	switch(mode)
	{
		case mode_INSERT:
			if (!m_item_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_attribute_names.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			if (!m_attribute_values.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
			break;

		default:
			DEBUG_FATAL(true,("Bad query mode."));
	}
	return true;
}

// ----------------------------------------------------------------------

bool MarketAuctionsAttributesQuery::addData(NetworkId const & itemId, std::string const & attributeName, Unicode::String const & attributeValue)
{
	if (!m_item_ids.push_back(itemId)) return false;
	if (!m_attribute_names.push_back(attributeName)) return false;
	if (!m_attribute_values.push_back(attributeValue)) return false;

	m_numItems=m_numItems.getValue() + 1;
	return true;
}

// ----------------------------------------------------------------------

int MarketAuctionsAttributesQuery::getNumItems() const
{
	return m_numItems.getValue();
}

// ----------------------------------------------------------------------

void MarketAuctionsAttributesQuery::clearData()
{
	m_item_ids.clear();
	m_attribute_names.clear();
	m_attribute_values.clear();
		
	m_numItems=0;
}

// ----------------------------------------------------------------------

void MarketAuctionsAttributesQuery::freeData()
{
	m_item_ids.free();
	m_attribute_names.free();
	m_attribute_values.free();
}

// ----------------------------------------------------------------------

bool MarketAuctionsAttributesQuery::bindParameters()
{
	if (!bindParameter(m_item_ids)) return false;
	if (!bindParameter(m_attribute_names)) return false;
	if (!bindParameter(m_attribute_values)) return false;
	if (!bindParameter(m_numItems)) return false;
		
	return true;
}

// ----------------------------------------------------------------------

bool MarketAuctionsAttributesQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

void MarketAuctionsAttributesQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ") + DatabaseProcess::getInstance().getSchemaQualifier() +
		"cm_persister.insert_auction_attributes (:item_ids, :attribute_names, :attribute_values, :num_items); end;";
}

// ======================================================================
