// ======================================================================
//
// SkillQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SkillQuery_H
#define INCLUDED_SkillQuery_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbModeQuery.h"

// ======================================================================

namespace DBSchema
{
	struct SkillQueryData : public DB::Row
	{
		enum OperationType
		{
			ot_INSERT,
			ot_DELETE,
			ot_SET
		};
		
		DB::BindableNetworkId object_id;
		DB::BindableLong index;
		DB::BindableLong operation;
		DB::BindableString<500> skill; //TODO:  size

		virtual void copy(const DB::Row &rhs)
		{
			*this = dynamic_cast<const SkillQueryData&>(rhs);
		}
	};
}

// ======================================================================

namespace DBQuery
{
	
	class SkillQuery : public DB::ModeQuery
	{
	  public:
		SkillQuery();
		
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);

	  private:
		SkillQuery(const SkillQuery&);            // disable
		SkillQuery& operator=(const SkillQuery&); // disable
	};

// ----------------------------------------------------------------------

	/**
	 * A query that gets all the attributes for an object.
	 */
	
	class GetAllSkills : public DB::Query
	{
	  public:
		  GetAllSkills();
		
		const DBSchema::SkillQueryData &getData() const;
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
		virtual QueryMode getExecutionMode() const;

	  private:
		GetAllSkills(const GetAllSkills&); // disable
		GetAllSkills& operator=(const GetAllSkills&); // disable

	  private:
		DBSchema::SkillQueryData m_data;
	};
}

// ======================================================================

inline const DBSchema::SkillQueryData &DBQuery::GetAllSkills::getData() const
{
	return m_data;
}

// ----------------------------------------------------------------------

inline DBQuery::GetAllSkills::GetAllSkills() : Query(), m_data()
{
}

// ======================================================================

#endif
