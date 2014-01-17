// ======================================================================
//
// GetUniverseQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GetUniverseQuery_H
#define INCLUDED_GetUniverseQuery_H

// ======================================================================

#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"

// ======================================================================

namespace DBQuery {

	/** A query to get the list of universe objects.
	 */
	class GetUniverseQuery : public DB::Query
	{
	  public:
		NetworkId getObjectId();
		
	  public:
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	  public:
		GetUniverseQuery(const std::string &schema);
		
	  private:
		GetUniverseQuery(const GetUniverseQuery&); // disable
		GetUniverseQuery& operator=(const GetUniverseQuery&); //disable

	  private:
		DB::BindableNetworkId object_id;
		std::string m_schema;
	};

// ----------------------------------------------------------------------

	inline NetworkId GetUniverseQuery::getObjectId()
	{
		return object_id.getValue();
	}
	
}

// ======================================================================

#endif
