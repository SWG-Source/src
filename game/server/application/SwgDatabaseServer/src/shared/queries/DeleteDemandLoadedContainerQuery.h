// ======================================================================
//
// DeleteDemandLoadedContainerQuery.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DeleteDemandLoadedContainerQuery_H
#define INCLUDED_DeleteDemandLoadedContainerQuery_H

// ======================================================================

#include "sharedDatabaseInterface/DbBindableVarray.h"
#include "sharedDatabaseInterface/DbQuery.h"

// ======================================================================

namespace DBQuery
{

	class DeleteDemandLoadedContainerQuery : public DB::Query
	{
	  public:
		DeleteDemandLoadedContainerQuery();
		~DeleteDemandLoadedContainerQuery();

		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
		virtual QueryMode getExecutionMode() const;

		bool addData(const NetworkId &networkId, int reason);
		bool setupData(DB::Session &session);
		void clearData();
		void freeData();
		int  getCount() const;
			
	  private:
		DB::BindableVarrayString object_ids;
		DB::BindableVarrayNumber reasons;
		DB::BindableLong         num_items;
		
	  private:
		DeleteDemandLoadedContainerQuery(const DeleteDemandLoadedContainerQuery&);
		DeleteDemandLoadedContainerQuery& operator=(const DeleteDemandLoadedContainerQuery&);
	};
}

// ======================================================================

#endif
