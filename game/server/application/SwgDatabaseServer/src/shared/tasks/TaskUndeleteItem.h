// ======================================================================
//
// TaskUndeleteItem.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskUndeleteItem_H
#define INCLUDED_TaskUndeleteItem_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"

//-----------------------------------------------------------------------

class TaskUndeleteItem:public DB::TaskRequest
{
  public:
	TaskUndeleteItem(const NetworkId &itemId, const std::string &whoRequested);
	virtual ~TaskUndeleteItem();
	
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

  private:
	class UndeleteItemQuery : public DB::Query
	{
	  public:
		UndeleteItemQuery();

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;
		
	  public:
		// input:
		DB::BindableNetworkId   item_id;
		
		//output:
		DB::BindableLong        result;
		
	  private:
		UndeleteItemQuery(const UndeleteItemQuery&);
		UndeleteItemQuery& operator=(const UndeleteItemQuery&);
	};

  private:
	NetworkId m_itemId;
	std::string m_whoRequested;
	int       m_result;
	
  private:
	//Disabled:
	TaskUndeleteItem(const TaskUndeleteItem&);
	TaskUndeleteItem& operator=(const TaskUndeleteItem& rhs);
};

//-----------------------------------------------------------------------

#endif
