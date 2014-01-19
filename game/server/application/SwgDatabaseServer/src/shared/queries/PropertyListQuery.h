// ======================================================================
//
// PropertyListQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyListQuery_H
#define INCLUDED_PropertyListQuery_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbBindableVarray.h"
#include "sharedDatabaseInterface/DbModeQuery.h"
#include <vector>

// ======================================================================

namespace DBSchema
{
	struct PropertyListQueryData : public DB::Row
	{
		enum OperationType
		{
			ot_DELETE,
			ot_INSERT,
			ot_CLEAR
		};
		
		DB::BindableNetworkId object_id;
		DB::BindableLong list_id;
		DB::BindableLong operation;
		DB::BindableString<500> value; //TODO:  size

		virtual void copy(const DB::Row &rhs)
		{
			*this = dynamic_cast<const PropertyListQueryData&>(rhs);
		}
	};
}

// ======================================================================

namespace DBQuery
{
	
	class PropertyListQuery : public DB::ModeQuery
	{
	  public:
		PropertyListQuery();
		
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);

		bool setupData(DB::Session *session);
		bool addData(const NetworkId &networkId, int listId, const std::string &value, int operation);
		void clearData();
		void freeData();

		int getNumItems() const;


	  private:
		DB::BindableVarrayString	m_object_ids;
		DB::BindableVarrayNumber	m_list_ids;
		DB::BindableVarrayString	m_values;
		DB::BindableLong			m_operations;
		DB::BindableLong			m_numItems;
		DB::BindableLong			m_enableDatabaseLogging;

		PropertyListQuery(const PropertyListQuery&);            // disable
		PropertyListQuery& operator=(const PropertyListQuery&); // disable
	};

// ----------------------------------------------------------------------

	/**
	 * A query that gets all the attributes for an object.
	 */
	
	class GetAllPropertyLists : public DB::Query
	{
	  public:
		GetAllPropertyLists(const std::string &schema);
		
		const std::vector<DBSchema::PropertyListQueryData> &getData() const;
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
		virtual QueryMode getExecutionMode() const;

	  private:
		GetAllPropertyLists(const GetAllPropertyLists&); // disable
		GetAllPropertyLists& operator=(const GetAllPropertyLists&); // disable

	  private:
		std::vector<DBSchema::PropertyListQueryData> m_data;
		const std::string m_schema;
	};
}

// ======================================================================

inline const std::vector<DBSchema::PropertyListQueryData> &DBQuery::GetAllPropertyLists::getData() const
{
	return m_data;
}

// ======================================================================

#endif

