// ======================================================================
//
// LocationQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LocationQuery_H
#define INCLUDED_LocationQuery_H

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include <vector>

class LocationData;

// ======================================================================

namespace DBQuery
{
	class SaveLocationQuery : public DB::Query
	{
	  public:
		SaveLocationQuery();
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
		virtual QueryMode getExecutionMode() const;

		void setData(const NetworkId &objectId, size_t listId, int action, int index, const LocationData &location);
		
	  public:
		DB::BindableNetworkId object_id;
		DB::BindableLong list_id;
		DB::BindableLong index;
		DB::BindableLong action;
		DB::BindableUnicode<255> name;
		DB::BindableString<50> scene;
		DB::BindableDouble x;
		DB::BindableDouble y;
		DB::BindableDouble z;
		DB::BindableDouble radius;

	  private:
		SaveLocationQuery(const SaveLocationQuery&);            //disable
		SaveLocationQuery &operator=(const SaveLocationQuery&); //disable
	};

// ======================================================================

	class LoadLocationQuery : public DB::Query
	{
	  public:
		struct LocationRow
		{
			DB::BindableNetworkId object_id;
			DB::BindableLong list_id;
			DB::BindableLong index;
			DB::BindableUnicode<255> name;
			DB::BindableString<50> scene;
			DB::BindableDouble x;
			DB::BindableDouble y;
			DB::BindableDouble z;
			DB::BindableDouble radius;
		};
 
	  public:
		LoadLocationQuery(const std::string &schema);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
		virtual QueryMode getExecutionMode() const;

		const std::vector<LocationRow> & getData() const;

	  private:
		std::vector<LocationRow> m_data;
		const std::string m_schema;
		
	  private:
		LoadLocationQuery(const LoadLocationQuery&);            //disable
		LoadLocationQuery &operator=(const LoadLocationQuery&); //disable
	};

} //namespace

// ======================================================================

inline const std::vector<DBQuery::LoadLocationQuery::LocationRow> & DBQuery::LoadLocationQuery::getData() const
{
	return m_data;
}

// ======================================================================

#endif
