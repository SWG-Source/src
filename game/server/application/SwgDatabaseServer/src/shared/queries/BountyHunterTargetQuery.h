// ======================================================================
//
// BountyHunterTargetQuery,h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BountyHunterTargetQuery_H
#define INCLUDED_BountyHunterTargetQuery_H

// ======================================================================

#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/DatabaseProcessQuery.h"
#include "sharedDatabaseInterface/DbBindableVarray.h"

#include <vector>

// ======================================================================

namespace DBQuery
{
	class BountyHunterTargetQuery : public DB::Query
	{
		BountyHunterTargetQuery(const BountyHunterTargetQuery&);
		BountyHunterTargetQuery& operator= (const BountyHunterTargetQuery&);
	  public:
		BountyHunterTargetQuery();

		virtual bool bindParameters ();
		virtual bool bindColumns    ();
		virtual void getSQL(std::string &sql);

		bool setupData(DB::Session *session);
		bool addData(DBSchema::BountyHunterTargetRow const & data);
		void clearData();
		void freeData();

		int getNumItems() const;

	  private:
		DB::BindableVarrayString    m_object_ids;
		DB::BindableVarrayString    m_target_ids;
		DB::BindableLong            m_numItems;
	};

	
	class BountyHunterTargetQuerySelect : public DB::Query
	{
	  public:
		BountyHunterTargetQuerySelect(const std::string &schema);

		virtual bool bindParameters ();
		virtual bool bindColumns    ();
		virtual void getSQL         (std::string &sql);

		const std::vector<DBSchema::BountyHunterTargetRow> & getData() const;

	  protected:
		virtual QueryMode getExecutionMode() const;

	  private:
		std::vector<DBSchema::BountyHunterTargetRow> m_data;
		const std::string m_schema;

		BountyHunterTargetQuerySelect             (const BountyHunterTargetQuerySelect&);
		BountyHunterTargetQuerySelect& operator= (const BountyHunterTargetQuerySelect&);
	};

}

// ======================================================================

#endif
