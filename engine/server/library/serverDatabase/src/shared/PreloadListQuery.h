// ======================================================================
//
// PreloadListQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PreloadListQuery_H
#define INCLUDED_PreloadListQuery_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"

class NetworkId;

// ======================================================================

namespace DBQuery
{
	
	class PreloadListQuery : public DB::Query
	{
	  public:
		PreloadListQuery(const std::string &sceneId);
		virtual ~PreloadListQuery();

	  public:
		DB::BindableString<50> scene_id;
		DB::BindableNetworkId  object_id;
		DB::BindableLong       chunk_x;
		DB::BindableLong       chunk_z;
		DB::BindableLong       preload_range;
		DB::BindableLong       city_server_id;
		DB::BindableLong       wilderness_server_id;
		
	  public:
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);

	  private:
		virtual QueryMode getExecutionMode() const;
	
	  private:
		PreloadListQuery(const PreloadListQuery&); // disable
		PreloadListQuery& operator=(const PreloadListQuery&); // disable
	};
}

// ======================================================================

#endif
