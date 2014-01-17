// ======================================================================
//
// TaskGetLocationList.h
// copyright (c) 2004 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================

#ifndef INCLUDED_TaskGetLocationList_H
#define INCLUDED_TaskGetLocationList_H

// ======================================================================

#include "serverDatabase/dBLocationRecord.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbRow.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"
#include <vector>

// ======================================================================

namespace DBSchema
{
	struct AuctionLocationRow : public DB::Row
	{
		DB::BindableNetworkId        locationId;
		DB::BindableNetworkId        ownerId;
		DB::BindableString<256>      locationString;
		DB::BindableLong             salesTax;
		DB::BindableNetworkId        salesTaxBankId;
		DB::BindableLong             emptyDate;
		DB::BindableLong             lastAccessDate;
		DB::BindableLong             inactiveDate;
		DB::BindableLong             status;
		DB::BindableBool             searchEnabled;
		DB::BindableLong             entranceCharge;

		virtual void copy(const DB::Row &rhs)
		{
			*this = dynamic_cast<const AuctionLocationRow&>(rhs);
		}
	};
}

// ======================================================================

class TaskGetLocationList : public DB::TaskRequest
{
  public:
	explicit TaskGetLocationList ();
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:

	class GetLocationsQuery : public DB::Query
	{
	public:
		std::vector<DBSchema::AuctionLocationRow> m_data;

		GetLocationsQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	private:  //disable
		GetLocationsQuery(const GetLocationsQuery&);
		GetLocationsQuery& operator=(const GetLocationsQuery&);
	};

	int count;
};

// ======================================================================

#endif
