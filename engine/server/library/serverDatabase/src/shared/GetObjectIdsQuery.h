// ======================================================================
//
// GetOIDsQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GetOIDsQuery_H
#define INCLUDED_GetOIDsQuery_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"

// ======================================================================

namespace DBQuery
{

/**
 * A query to get a block of object ID's.
 */
	class GetOIDsQuery : public DB::Query
	{
	  public:
		GetOIDsQuery();
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

		void      setHowMany(int howMany);
		int       getHowMany() const;
		NetworkId getStartId() const;
		NetworkId getEndId() const;
		
	  protected:
		virtual QueryMode getExecutionMode() const;
		
	  private:   
		GetOIDsQuery(const GetOIDsQuery&);
		GetOIDsQuery& operator=(const GetOIDsQuery&);

		DB::BindableLong        min_count;
		DB::BindableNetworkId   start_id;
		DB::BindableNetworkId   end_id;
	};

}

// ----------------------------------------------------------------------

inline NetworkId DBQuery::GetOIDsQuery::getStartId() const
{
	FATAL(start_id.isNull(),("The database has run out of object ids.\n"));
	return start_id.getValue();
}

// ----------------------------------------------------------------------

inline NetworkId DBQuery::GetOIDsQuery::getEndId() const
{
	if (end_id.isNull())  	//TODO:  remove when done with Postgres
		return NetworkId(start_id.getValue().getValue() + min_count.getValue() - 1); // for Postgres, query doesn't return the end id, so we assume we got the number of id's we wanted.
	return end_id.getValue();
}

// ----------------------------------------------------------------------

inline int DBQuery::GetOIDsQuery::getHowMany() const
{
	return min_count.getValue();
}

// ----------------------------------------------------------------------

inline void DBQuery::GetOIDsQuery::setHowMany(int howMany)
{
	min_count = howMany;
}

// ======================================================================

#endif
