// ======================================================================
//
// GetTimestampQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GetTimestampQuery_H
#define INCLUDED_GetTimestampQuery_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbQuery.h"

// ======================================================================

namespace DBQuery
{
	
	class GetTimestampQuery : public DB::Query
	{
	  public:
		GetTimestampQuery();
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

		int getTimestamp();
		
	  private:
		DB::BindableLong timestamp;

	  private:
		GetTimestampQuery(const GetTimestampQuery &); // disable
		GetTimestampQuery & operator=(const GetTimestampQuery &); //disable
	};

}

// ======================================================================

inline int DBQuery::GetTimestampQuery::getTimestamp()
{
	return timestamp.getValue();
}
	
// ======================================================================

#endif
