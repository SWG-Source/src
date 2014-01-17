// ======================================================================
//
// ExperienceQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ExperienceQuery_H
#define INCLUDED_ExperienceQuery_H

// ======================================================================

#include "sharedDatabaseInterface/DbModeQuery.h"
#include "SwgDatabaseServer/Schema.h"
#include <vector>

// ======================================================================

namespace DBQuery
{
	
	class ExperienceQuery : public DB::ModeQuery
	{
		ExperienceQuery(const ExperienceQuery&); // disable
		ExperienceQuery& operator=(const ExperienceQuery&); // disable
	  public:
		ExperienceQuery();
		
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
	};

// ----------------------------------------------------------------------

	class GetAllExperience : public DB::Query
	{
	  public:
		GetAllExperience(const std::string &schema);
		
		const std::vector<DBSchema::ExperiencePointRow> &getData() const;
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
		virtual QueryMode getExecutionMode() const;

	  private:
		GetAllExperience(const GetAllExperience&); // disable
		GetAllExperience& operator=(const GetAllExperience&); // disable

	  private:
		std::vector<DBSchema::ExperiencePointRow> m_data;
		const std::string m_schema;
	};
}

// ======================================================================

inline const std::vector<DBSchema::ExperiencePointRow> & DBQuery::GetAllExperience::getData() const
{
	return m_data;
}

// ======================================================================

#endif
