// ======================================================================
//
// SaveTimestampQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SaveTimestampQuery_H
#define INCLUDED_SaveTimestampQuery_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbQuery.h"

// ======================================================================

namespace DBQuery
{
	
	class SaveTimestampQuery : public DB::Query
	{
	  public:
		SaveTimestampQuery(int _timestamp);
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	  private:
		DB::BindableLong timestamp;

	  private:
		SaveTimestampQuery(const SaveTimestampQuery &); // disable
		SaveTimestampQuery & operator=(const SaveTimestampQuery &); //disable
	};

}

// ======================================================================

#endif
