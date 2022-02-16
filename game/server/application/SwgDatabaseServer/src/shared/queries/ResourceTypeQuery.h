// ======================================================================
//
// ResourceTypeQuery,h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ResourceTypeQuery_H
#define INCLUDED_ResourceTypeQuery_H

// ======================================================================

#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/DatabaseProcessQuery.h"
#include "sharedDatabaseInterface/DbBindableVarray.h"

#include <vector>

// ======================================================================

namespace DBQuery
{
	class ResourceTypeQuery : public DB::Query
	{
		ResourceTypeQuery(const ResourceTypeQuery&);
		ResourceTypeQuery& operator= (const ResourceTypeQuery&);
	  public:
		ResourceTypeQuery();

		virtual bool bindParameters ();
		virtual bool bindColumns    ();
		virtual void getSQL(std::string &sql);

		bool setupData(DB::Session *session);
		bool addData(DBSchema::ResourceTypeRow const & data);
		void clearData();
		void freeData();

		int getNumItems() const;

	  private:
		DB::BindableVarrayString    m_resource_ids;
		DB::BindableVarrayString    m_resource_names;
		DB::BindableVarrayString    m_resource_classes;
		DB::BindableVarrayString    m_attributes;
		DB::BindableVarrayString    m_fractal_seeds;
		DB::BindableVarrayNumber    m_depleted_timestamps;
		DB::BindableLong            m_numItems;
	};

	
	class ResourceTypeQuerySelect : public DB::Query
	{
	  public:
		ResourceTypeQuerySelect(const std::string &schema);

		virtual bool bindParameters ();
		virtual bool bindColumns    ();
		virtual void getSQL         (std::string &sql);

		const std::vector<DBSchema::ResourceTypeRow> & getData() const;

	  protected:
		virtual QueryMode getExecutionMode() const;

	  private:
		std::vector<DBSchema::ResourceTypeRow> m_data;
		const std::string m_schema;

		ResourceTypeQuerySelect            (const ResourceTypeQuerySelect&);
		ResourceTypeQuerySelect& operator= (const ResourceTypeQuerySelect&);
	};

}

// ======================================================================

#endif
