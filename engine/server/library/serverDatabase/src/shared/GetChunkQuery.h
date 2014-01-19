// ======================================================================
//
// GetChunkQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GetChunkQuery_H
#define INCLUDED_GetChunkQuery_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/DbQuery.h"

// ======================================================================

namespace DBQuery {

	/** A query to get the list of chunk objects.
	 */
	class GetChunkQuery : public DB::Query
	{
	  public:
		void setChunk(const std::string &sceneId, int nodeX, int nodeZ);
		
	  public:
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		int getObjectCount() const;

	  public:
		GetChunkQuery(const std::string &schema);

	  private:
		virtual QueryMode getExecutionMode() const;
		
	  private:
		GetChunkQuery(const GetChunkQuery&); // disable
		GetChunkQuery& operator=(const GetChunkQuery&); //disable

	  private:
		DB::BindableLong object_count;
		DB::BindableString<50> scene_id;
		DB::BindableLong node_x;
		DB::BindableLong node_z;

		std::string m_schema;
	};

}

// ----------------------------------------------------------------------

inline int DBQuery::GetChunkQuery::getObjectCount() const
{
	return object_count.getValue();
}

// ======================================================================

#endif
