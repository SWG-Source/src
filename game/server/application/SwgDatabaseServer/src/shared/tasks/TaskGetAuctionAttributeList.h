// ======================================================================
//
// TaskGetAuctionAttributeList.h
// copyright (c) 2004 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================

#ifndef INCLUDED_TaskGetAuctionAttributeList_H
#define INCLUDED_TaskGetAuctionAttributeList_H

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
	struct AuctionAttributeRow : public DB::Row
	{
		DB::BindableNetworkId     item_id;
		DB::BindableString<1000>  attribute_name;
		DB::BindableUnicode<1000> attribute_value;

		virtual void copy(const DB::Row &rhs)
		{
			*this = dynamic_cast<const AuctionAttributeRow&>(rhs);
		}
	};
}

// ======================================================================

class TaskGetAuctionAttributeList : public DB::TaskRequest
{
  public:
	TaskGetAuctionAttributeList ();
	
  public:
	virtual bool process    (DB::Session *session);
	virtual void onComplete ();

  private:

	class GetAuctionAttributesQuery : public DB::Query
	{
	public:
		std::vector<DBSchema::AuctionAttributeRow> m_data;

		GetAuctionAttributesQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	private:  //disable
		GetAuctionAttributesQuery(const GetAuctionAttributesQuery&);
		GetAuctionAttributesQuery& operator=(const GetAuctionAttributesQuery&);
	};
	
	int count;
};

// ======================================================================

#endif
