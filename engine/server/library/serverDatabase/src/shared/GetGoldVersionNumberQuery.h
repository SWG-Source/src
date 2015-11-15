// ======================================================================
//
// GetGoldVersionNumberQuery.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GetGoldVersionNumberQuery_H
#define INCLUDED_GetGoldVersionNumberQuery_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbQuery.h"

// ======================================================================

namespace DBQuery
{
	
	class GetGoldVersionNumberQuery : public DB::Query
	{
	  public:
		GetGoldVersionNumberQuery();
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

		int getVersionNumber();
		int getMinVersionNumber();
		
	  private:
		DB::BindableLong current_version_number;
		DB::BindableLong min_version_number;

	  private:
		GetGoldVersionNumberQuery              (const GetGoldVersionNumberQuery &); // disable
		GetGoldVersionNumberQuery & operator=  (const GetGoldVersionNumberQuery &); // disable
	};

}

// ======================================================================

inline int DBQuery::GetGoldVersionNumberQuery::getVersionNumber()
{
	return current_version_number.getValue();
}
	
// ----------------------------------------------------------------------

inline int DBQuery::GetGoldVersionNumberQuery::getMinVersionNumber()
{
	return min_version_number.getValue();
}

// ======================================================================

#endif
