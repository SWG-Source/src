// ======================================================================
//
// GetVersionNumberQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GetVersionNumberQuery_H
#define INCLUDED_GetVersionNumberQuery_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbQuery.h"

// ======================================================================

namespace DBQuery
{
	
	class GetVersionNumberQuery : public DB::Query
	{
	  public:
		GetVersionNumberQuery();
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

		int getVersionNumber();
		int getMinVersionNumber();
		
	  private:
		DB::BindableLong current_version_number;
		DB::BindableLong min_version_number;

	  private:
		GetVersionNumberQuery              (const GetVersionNumberQuery &); // disable
		GetVersionNumberQuery & operator=  (const GetVersionNumberQuery &); // disable
	};

}

// ======================================================================

inline int DBQuery::GetVersionNumberQuery::getVersionNumber()
{
	return current_version_number.getValue();
}
	
// ----------------------------------------------------------------------

inline int DBQuery::GetVersionNumberQuery::getMinVersionNumber()
{
	return min_version_number.getValue();
}

// ======================================================================

#endif
