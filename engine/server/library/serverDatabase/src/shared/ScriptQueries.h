// ======================================================================
//
// ScriptQueries.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ScriptQueries_H
#define INCLUDED_ScriptQueries_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include <vector>

// ======================================================================

namespace DBQuery
{
	class ScriptClearQuery : public DB::Query
	{
	  public:
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

		void setNetworkId(NetworkId objectID);

		ScriptClearQuery() {}

	  private:
		DB::BindableNetworkId object_id;
	
		ScriptClearQuery(const ScriptClearQuery&);
		ScriptClearQuery &operator=(const ScriptClearQuery&);
	};

// ----------------------------------------------------------------------

	class GetScripts : public DB::Query
	{
	  public:
		struct ScriptRow
		{
			DB::BindableNetworkId object_id;
			DB::BindableLong index;
			DB::BindableString<128> script_name;
		};

	  public:
		GetScripts(const std::string &schema);

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

		QueryMode getExecutionMode() const;

		const std::vector<ScriptRow> & getData() const;

	  private:
		std::vector<ScriptRow> m_data;
		const std::string m_schema;
		
		GetScripts(const GetScripts&);
		GetScripts &operator=(const GetScripts&);
	};
	
} // namespace

// ======================================================================

inline const std::vector<DBQuery::GetScripts::ScriptRow> & DBQuery::GetScripts::getData() const
{
	return m_data;
}

// ======================================================================

#endif
