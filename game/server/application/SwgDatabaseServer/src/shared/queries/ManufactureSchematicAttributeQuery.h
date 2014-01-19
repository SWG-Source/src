// ======================================================================
//
// ManufactureSchematicAttributeQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ManufactureSchematicAttributeQuery_H
#define INCLUDED_ManufactureSchematicAttributeQuery_H

// ======================================================================

#include "sharedDatabaseInterface/DbModeQuery.h"
#include "SwgDatabaseServer/Schema.h"
#include <vector>

// ======================================================================

namespace DBSchema
{
	struct ManufactureSchematicAttributeRow : public DB::Row
	{
		DB::BindableNetworkId object_id;
		DB::BindableLong         action;
		DB::BindableString<500>  attribute_type;
		DB::BindableDouble       value;

		virtual void copy(const DB::Row &rhs)
			{
				*this = dynamic_cast<const ManufactureSchematicAttributeRow&>(rhs);
			}
	};
	
}

// ----------------------------------------------------------------------

namespace DBQuery
{
	
	class ManufactureSchematicAttributeQuery : public DB::ModeQuery
	{
		ManufactureSchematicAttributeQuery(const ManufactureSchematicAttributeQuery&); // disable
		ManufactureSchematicAttributeQuery& operator=(const ManufactureSchematicAttributeQuery&); // disable
	  public:
		ManufactureSchematicAttributeQuery();
		
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
	};

// ----------------------------------------------------------------------

	/**
	 * A query that gets all the attributes for an object.
	 */
	
	class GetAllManufactureSchematicAttributes : public DB::Query
	{
	  public:
		GetAllManufactureSchematicAttributes(const std::string &schema);
		
		const std::vector<DBSchema::ManufactureSchematicAttributeRow> &getData() const;
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
		virtual QueryMode getExecutionMode() const;

	  private:
		GetAllManufactureSchematicAttributes(const GetAllManufactureSchematicAttributes&); // disable
		GetAllManufactureSchematicAttributes& operator=(const GetAllManufactureSchematicAttributes&); // disable

	  private:
		std::vector<DBSchema::ManufactureSchematicAttributeRow> m_data;
		const std::string m_schema;
	};
}

// ======================================================================

inline const std::vector<DBSchema::ManufactureSchematicAttributeRow> & DBQuery::GetAllManufactureSchematicAttributes::getData() const
{
	return m_data;
}

// ======================================================================

#endif
