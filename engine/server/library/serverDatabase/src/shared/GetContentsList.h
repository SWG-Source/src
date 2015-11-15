// ======================================================================
//
// GetContentsList.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GetContentsList_H
#define INCLUDED_GetContentsList_H

// ======================================================================

#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"

class NetworkId;

// ======================================================================

namespace DBQuery
{
	
	class GetContentsList : public DB::Query
	{
	  public:
		GetContentsList();
		virtual ~GetContentsList();

	  public:
		void setContainerId(const NetworkId &id);
		NetworkId getContainedId() const;
		
	  public:
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);

	  private:
		virtual QueryMode getExecutionMode() const;
	
	  private:
		GetContentsList(const GetContentsList&); // disable
		GetContentsList& operator=(const GetContentsList&); // disable

	  private:
		DB::BindableNetworkId containerId;
		DB::BindableNetworkId containedId;
	};
}

// ======================================================================

#endif
