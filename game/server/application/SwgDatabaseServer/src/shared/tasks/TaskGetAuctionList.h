// ======================================================================
//
// TaskGetAuctionList.h
// copyright (c) 2004 Sony Online Entertainment
// Author: Doug Mellencamp
//
// This task has a special decoding function for loading out of band
// data(OOB) from the database. This is the only place OOB is loaded.
//
// ======================================================================

#ifndef INCLUDED_TaskGetAuctionList_H
#define INCLUDED_TaskGetAuctionList_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbRow.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"
#include <vector>

// ======================================================================

namespace DBSchema
{
	struct AuctionRow : public DB::Row
	{
		DB::BindableNetworkId      creatorId;
		DB::BindableInt32           minBid;
		DB::BindableInt32           auctionTimer;
		DB::BindableInt32           buyNowPrice;
		DB::BindableUnicode<1024>  userDescription;
		DB::BindableString<4000>   oob;
		DB::BindableNetworkId      locationId;
		DB::BindableNetworkId      itemId;
		DB::BindableInt32           category;
		DB::BindableInt32           itemTimer;
		DB::BindableUnicode<1024>  itemName;
		DB::BindableNetworkId      ownerId;
		DB::BindableInt32           active;
		DB::BindableInt32           itemSize;
		DB::BindableInt32           itemTemplateId;

		virtual void copy(const DB::Row &rhs)
		{
			*this = dynamic_cast<const AuctionRow&>(rhs);
		}
	};
}

// ======================================================================

class TaskGetAuctionList : public DB::TaskRequest
{
  public:
	TaskGetAuctionList ();
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:

	class GetAuctionsQuery : public DB::Query
	{
	public:
		std::vector<DBSchema::AuctionRow> m_data;

		GetAuctionsQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	private:  //disable
		GetAuctionsQuery(const GetAuctionsQuery&);
		GetAuctionsQuery& operator=(const GetAuctionsQuery&);
	};

	int count;
};

// ======================================================================

#endif
