// ======================================================================
//
// TaskGetBidList.h
// copyright (c) 2004 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================

#ifndef INCLUDED_TaskGetBidList_H
#define INCLUDED_TaskGetBidList_H

// ======================================================================

#include "serverDatabase/dBBidRecord.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbRow.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"
#include <vector>

// ======================================================================

namespace DBSchema
{
	struct AuctionBidRow : public DB::Row
	{
		DB::BindableNetworkId       itemId;
		DB::BindableNetworkId       bidderId;
		DB::BindableLong            bid;
		DB::BindableLong            maxProxyBid;

		virtual void copy(const DB::Row &rhs)
		{
			*this = dynamic_cast<const AuctionBidRow&>(rhs);
		}
	};
}

// ======================================================================

class TaskGetBidList : public DB::TaskRequest
{
  public:
	TaskGetBidList ();
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:

	class GetBidsQuery : public DB::Query
	{
	  public:
		std::vector<DBSchema::AuctionBidRow> m_data;

		GetBidsQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	  private:  //disable
		GetBidsQuery(const GetBidsQuery&);
		GetBidsQuery& operator=(const GetBidsQuery&);
	};

	int count;
};

// ======================================================================

#endif
