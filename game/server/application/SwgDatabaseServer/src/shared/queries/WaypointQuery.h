// ======================================================================
//
// WaypointQuery.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_WaypointQuery_H
#define INCLUDED_WaypointQuery_H

// ======================================================================

#include "sharedDatabaseInterface/DbModeQuery.h"
#include "SwgDatabaseServer/Schema.h"
#include <vector>

// ======================================================================

namespace DBQuery
{
	
	class WaypointQuery : public DB::ModeQuery
	{
		WaypointQuery(const WaypointQuery&); // disable
		WaypointQuery& operator=(const WaypointQuery&); // disable
	  public:
		WaypointQuery();
		
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
	};

// ----------------------------------------------------------------------

	class GetAllWaypoints : public DB::Query
	{
	  public:
		GetAllWaypoints(const std::string &schema);
		
		const std::vector<DBSchema::WaypointRow> &getData() const;
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
		virtual QueryMode getExecutionMode() const;
	
	  private:
		GetAllWaypoints(const GetAllWaypoints&); // disable
		GetAllWaypoints& operator=(const GetAllWaypoints&); // disable

	  private:
		std::vector<DBSchema::WaypointRow> m_data;
		const std::string m_schema;
	};
}

// ======================================================================

inline const std::vector<DBSchema::WaypointRow> & DBQuery::GetAllWaypoints::getData() const
{
	return m_data;
}

// ======================================================================

#endif
